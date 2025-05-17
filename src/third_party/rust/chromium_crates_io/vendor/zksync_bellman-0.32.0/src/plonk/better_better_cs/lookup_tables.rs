use crate::bit_vec::BitVec;
use crate::pairing::ff::{Field, PrimeField};
use crate::pairing::Engine;

use crate::SynthesisError;
use std::marker::PhantomData;

use crate::plonk::domains::*;
use crate::plonk::polynomials::*;
use crate::worker::Worker;

use super::cs::*;
use super::data_structures::*;
use crate::plonk::better_cs::utils::*;
pub use crate::plonk::cs::variable::*;

pub const RANGE_CHECK_SINGLE_APPLICATION_TABLE_NAME: &'static str = "Range check table for a single column";

pub trait LookupTableInternal<E: Engine>: Send + Sync + 'static + std::any::Any + std::fmt::Debug {
    fn name(&self) -> &'static str;
    fn table_size(&self) -> usize;
    fn num_keys(&self) -> usize;
    fn num_values(&self) -> usize;
    fn allows_combining(&self) -> bool;
    fn is_valid_entry(&self, keys: &[E::Fr], values: &[E::Fr]) -> bool;
    fn query(&self, keys: &[E::Fr]) -> Result<Vec<E::Fr>, SynthesisError>;
    fn get_table_values_for_polys(&self) -> Vec<Vec<E::Fr>>;
    fn table_id(&self) -> E::Fr;
    fn sort(&self, values: &[E::Fr], column: usize) -> Result<Vec<E::Fr>, SynthesisError>;
    fn box_clone(&self) -> Box<dyn LookupTableInternal<E>>;
    fn column_is_trivial(&self, column_num: usize) -> bool;
}

impl<E: Engine> std::hash::Hash for dyn LookupTableInternal<E> {
    fn hash<H>(&self, state: &mut H)
    where
        H: std::hash::Hasher,
    {
        self.type_id().hash(state);
        self.name().hash(state);
        self.table_size().hash(state);
        self.num_keys().hash(state);
        self.num_values().hash(state);
    }
}

impl<E: Engine> PartialEq for dyn LookupTableInternal<E> {
    fn eq(&self, other: &Self) -> bool {
        self.type_id() == other.type_id() && self.name() == other.name() && self.table_size() == other.table_size() && self.num_keys() == other.num_keys() && self.num_values() == other.num_values()
    }
}

impl<E: Engine> Eq for dyn LookupTableInternal<E> {}

/// Applies a single lookup table to a specific set of columns
#[derive(serde::Serialize, serde::Deserialize)]
#[serde(bound(serialize = "dyn LookupTableInternal<E>: serde::Serialize", deserialize = "dyn LookupTableInternal<E>: serde::de::DeserializeOwned"))]
pub struct LookupTableApplication<E: Engine> {
    name: &'static str,
    apply_over: Vec<PolyIdentifier>,
    table_to_apply: Box<dyn LookupTableInternal<E>>,
    #[serde(skip)]
    name_generator: Option<Box<dyn (Fn() -> String) + 'static + Send + Sync>>,
    can_be_combined: bool,
}

impl<E: Engine> std::fmt::Debug for LookupTableApplication<E> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("LookupTableApplication")
            .field("name", &self.name)
            .field("apply_over", &self.apply_over)
            .field("table_to_apply", &self.table_to_apply)
            .field("can_be_combined", &self.can_be_combined)
            .finish()
    }
}

impl<E: Engine> PartialEq for LookupTableApplication<E> {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name && self.apply_over == other.apply_over && &self.table_to_apply == &other.table_to_apply && self.can_be_combined == other.can_be_combined
    }
}

impl<E: Engine> Eq for LookupTableApplication<E> {}

impl<E: Engine> LookupTableApplication<E> {
    pub fn new<L: LookupTableInternal<E>>(
        name: &'static str,
        table: L,
        apply_over: Vec<PolyIdentifier>,
        name_generator: Option<Box<dyn (Fn() -> String) + 'static + Send + Sync>>,
        can_be_combined: bool,
    ) -> Self {
        Self {
            name,
            apply_over,
            table_to_apply: Box::from(table) as Box<dyn LookupTableInternal<E>>,
            name_generator,
            can_be_combined,
        }
    }

    pub fn new_range_table_of_width_3(width: usize, over: Vec<PolyIdentifier>) -> Result<Self, SynthesisError> {
        let table = RangeCheckTableOverOneColumnOfWidth3::new(width);

        let name = RANGE_CHECK_SINGLE_APPLICATION_TABLE_NAME;

        Ok(Self {
            name: name,
            apply_over: over,
            table_to_apply: table.box_clone(),
            name_generator: None,
            can_be_combined: true,
        })
    }

    pub fn new_xor_table(bit_width: usize, over: Vec<PolyIdentifier>) -> Result<Self, SynthesisError> {
        Self::new_binop_table::<XorBinop>(bit_width, over, "XOR table")
    }

    pub fn new_and_table(bit_width: usize, over: Vec<PolyIdentifier>) -> Result<Self, SynthesisError> {
        Self::new_binop_table::<AndBinop>(bit_width, over, "AND table")
    }

    pub fn new_or_table(bit_width: usize, over: Vec<PolyIdentifier>) -> Result<Self, SynthesisError> {
        Self::new_binop_table::<OrBinop>(bit_width, over, "OR table")
    }

    pub fn new_binop_table<B: Binop>(bit_width: usize, over: Vec<PolyIdentifier>, name: &'static str) -> Result<Self, SynthesisError> {
        let table = TwoKeysOneValueBinopTable::<E, B>::new(bit_width, name);

        Ok(Self {
            name: name,
            apply_over: over,
            table_to_apply: table.box_clone(),
            name_generator: None,
            can_be_combined: true,
        })
    }

    pub fn functional_name(&self) -> String {
        if let Some(gen) = self.name_generator.as_ref() {
            gen()
        } else {
            self.name.to_string()
            // format!("{} over {:?}", self.table_to_apply.name(), self.apply_over)
        }
    }

    pub fn applies_over(&self) -> &[PolyIdentifier] {
        &self.apply_over
    }

    pub fn can_be_combined(&self) -> bool {
        self.can_be_combined && self.table_to_apply.allows_combining()
    }

    #[track_caller]
    pub fn is_valid_entry(&self, values: &[E::Fr]) -> bool {
        let num_keys = self.table_to_apply.num_keys();
        let num_values = self.table_to_apply.num_values();

        assert_eq!(num_keys + num_values, values.len());

        let (keys, values) = values.split_at(num_keys);

        self.table_to_apply.is_valid_entry(keys, values)
    }

    pub fn table_id(&self) -> E::Fr {
        self.table_to_apply.table_id()
    }

    pub fn size(&self) -> usize {
        self.table_to_apply.table_size()
    }

    pub fn width(&self) -> usize {
        self.table_to_apply.num_keys() + self.table_to_apply.num_values()
    }

    pub fn get_table_values_for_polys(&self) -> Vec<Vec<E::Fr>> {
        self.table_to_apply.get_table_values_for_polys()
    }

    pub fn query(&self, keys: &[E::Fr]) -> Result<Vec<E::Fr>, SynthesisError> {
        self.table_to_apply.query(keys)
    }

    pub fn as_internal(&self) -> &dyn LookupTableInternal<E> {
        self.table_to_apply.as_ref()
    }
}

/// Apply multiple tables at the same time to corresponding columns
#[derive(Debug)]
pub struct MultiTableApplication<E: Engine> {
    name: &'static str,
    apply_over: Vec<PolyIdentifier>,
    tables_to_apply: Vec<Box<dyn LookupTableInternal<E>>>,
    table_size: usize,
    id: E::Fr,
}

impl<E: Engine> PartialEq for MultiTableApplication<E> {
    fn eq(&self, other: &Self) -> bool {
        self.name == other.name && self.apply_over == other.apply_over && &self.tables_to_apply == &other.tables_to_apply && self.table_size == other.table_size
    }
}

impl<E: Engine> Eq for MultiTableApplication<E> {}

impl<E: Engine> MultiTableApplication<E> {
    pub fn name(&self) -> String {
        format!("Table {} of size {}", self.name, self.table_size)
    }

    pub fn functional_name(&self) -> String {
        self.name.to_string()
    }

    pub fn new_range_table_of_width_3(width: usize, over: Vec<PolyIdentifier>) -> Result<Self, SynthesisError> {
        let table = RangeCheckTableOverSingleColumn::new(width);

        let name = "Range check table";

        Ok(Self {
            name: name,
            apply_over: over,
            tables_to_apply: vec![table.box_clone(), table.box_clone(), table.box_clone()],
            table_size: 1 << width,
            id: table_id_from_string::<E::Fr>(name),
        })
    }

    pub fn applies_over(&self) -> &[PolyIdentifier] {
        &self.apply_over
    }

    pub fn is_valid_entry(&self, values: &[E::Fr]) -> bool {
        assert_eq!(values.len(), 3);
        let mut all_values = values;
        let mut valid = true;
        for t in self.tables_to_apply.iter() {
            let num_keys = t.num_keys();
            let num_values = t.num_values();
            let (keys, rest) = all_values.split_at(num_keys);
            let (values, rest) = rest.split_at(num_values);
            valid &= t.is_valid_entry(keys, values);
            all_values = rest;
        }

        valid
    }

    pub fn size(&self) -> usize {
        self.table_size
    }

    pub fn table_id(&self) -> E::Fr {
        self.id
    }

    pub fn width(&self) -> usize {
        let mut width = 0;
        for t in self.tables_to_apply.iter() {
            width += t.num_keys();
            width += t.num_values();
        }

        width
    }
}
#[derive(Clone)]
pub struct RangeCheckTableOverSingleColumn<E: Engine> {
    table_entries: Vec<E::Fr>,
    entries_map: std::collections::HashMap<E::Fr, usize>,
    bits: usize,
}

impl<E: Engine> RangeCheckTableOverSingleColumn<E> {
    pub fn new(bits: usize) -> Self {
        let mut entries = Vec::with_capacity(1 << bits);
        let mut map = std::collections::HashMap::with_capacity(1 << bits);
        for i in 0..(1 << bits) {
            let value = E::Fr::from_str(&i.to_string()).unwrap();
            entries.push(value);
            map.insert(value, i);
        }

        Self {
            table_entries: entries,
            entries_map: map,
            bits,
        }
    }
}

impl<E: Engine> std::fmt::Debug for RangeCheckTableOverSingleColumn<E> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("RangeCheckTableOverSingleColumn").field("bits", &self.bits).finish()
    }
}

impl<E: Engine> LookupTableInternal<E> for RangeCheckTableOverSingleColumn<E> {
    fn name(&self) -> &'static str {
        RANGE_CHECK_SINGLE_APPLICATION_TABLE_NAME
    }
    fn table_size(&self) -> usize {
        debug_assert_eq!(1usize << self.bits, self.table_entries.len());
        1usize << self.bits
    }
    fn num_keys(&self) -> usize {
        1
    }
    fn num_values(&self) -> usize {
        0
    }
    fn allows_combining(&self) -> bool {
        false
    }
    fn is_valid_entry(&self, keys: &[E::Fr], values: &[E::Fr]) -> bool {
        assert!(keys.len() == 1);
        assert!(values.len() == 0);

        self.table_entries.contains(&keys[0])
    }
    fn query(&self, keys: &[E::Fr]) -> Result<Vec<E::Fr>, SynthesisError> {
        assert!(keys.len() == 1);

        if self.entries_map.get(&keys[0]).is_some() {
            return Ok(vec![]);
        } else {
            return Err(SynthesisError::Unsatisfiable);
        }
    }
    fn get_table_values_for_polys(&self) -> Vec<Vec<E::Fr>> {
        vec![self.table_entries.clone()]
    }
    fn table_id(&self) -> E::Fr {
        table_id_from_string(self.name())
    }
    fn sort(&self, _values: &[E::Fr], _column: usize) -> Result<Vec<E::Fr>, SynthesisError> {
        unimplemented!()
    }
    fn box_clone(&self) -> Box<dyn LookupTableInternal<E>> {
        Box::from(self.clone())
    }
    fn column_is_trivial(&self, column_num: usize) -> bool {
        assert!(column_num < 1);

        false
    }
}

#[derive(Clone)]
pub struct RangeCheckTableOverOneColumnOfWidth3<E: Engine> {
    table_entries: Vec<E::Fr>,
    dummy_entries: Vec<E::Fr>,
    bits: usize,
}

impl<E: Engine> RangeCheckTableOverOneColumnOfWidth3<E> {
    pub fn new(bits: usize) -> Self {
        let mut entries = Vec::with_capacity(1 << bits);
        for i in 0..(1 << bits) {
            let value = E::Fr::from_str(&i.to_string()).unwrap();
            entries.push(value);
        }

        let dummy_entries = vec![E::Fr::zero(); 1 << bits];
        Self {
            table_entries: entries,
            dummy_entries,
            bits,
        }
    }
}

impl<E: Engine> std::fmt::Debug for RangeCheckTableOverOneColumnOfWidth3<E> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("RangeCheckTableOverOneColumnOfWidth3").field("bits", &self.bits).finish()
    }
}

impl<E: Engine> LookupTableInternal<E> for RangeCheckTableOverOneColumnOfWidth3<E> {
    fn name(&self) -> &'static str {
        "Range check table for a single column only with width 3"
    }
    fn table_size(&self) -> usize {
        debug_assert_eq!(1usize << self.bits, self.table_entries.len());
        1usize << self.bits
    }
    fn num_keys(&self) -> usize {
        3
    }
    fn num_values(&self) -> usize {
        0
    }
    fn allows_combining(&self) -> bool {
        true
    }
    fn is_valid_entry(&self, keys: &[E::Fr], values: &[E::Fr]) -> bool {
        assert!(keys.len() == 3);
        assert!(values.len() == 0);

        let repr = keys[0].into_repr().as_ref()[0];
        let mut valid = repr < (1 << self.bits);
        valid = valid & keys[1].is_zero();
        valid = valid & keys[2].is_zero();

        valid
    }
    fn query(&self, keys: &[E::Fr]) -> Result<Vec<E::Fr>, SynthesisError> {
        assert!(keys.len() == 3);

        let is_valid = self.is_valid_entry(keys, &[]);

        if is_valid {
            return Ok(vec![]);
        } else {
            return Err(SynthesisError::Unsatisfiable);
        }
    }
    fn get_table_values_for_polys(&self) -> Vec<Vec<E::Fr>> {
        vec![self.table_entries.clone(), self.dummy_entries.clone(), self.dummy_entries.clone()]
    }
    fn table_id(&self) -> E::Fr {
        table_id_from_string(self.name())
    }
    fn sort(&self, values: &[E::Fr], _column: usize) -> Result<Vec<E::Fr>, SynthesisError> {
        unimplemented!()
    }
    fn box_clone(&self) -> Box<dyn LookupTableInternal<E>> {
        Box::from(self.clone())
    }

    fn column_is_trivial(&self, column_num: usize) -> bool {
        assert!(column_num < 3);

        if column_num == 0 {
            false
        } else {
            true
        }
    }
}

pub trait Binop: 'static + Clone + Copy + Send + Sync + std::fmt::Debug + PartialEq + Eq {
    const NAME: &'static str;
    fn apply(x: usize, y: usize) -> usize;
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct XorBinop;

impl Binop for XorBinop {
    const NAME: &'static str = "XOR binop";

    fn apply(x: usize, y: usize) -> usize {
        x ^ y
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct AndBinop;

impl Binop for AndBinop {
    const NAME: &'static str = "AND binop";

    fn apply(x: usize, y: usize) -> usize {
        x & y
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct OrBinop;

impl Binop for OrBinop {
    const NAME: &'static str = "OR binop";

    fn apply(x: usize, y: usize) -> usize {
        x | y
    }
}

#[derive(Clone)]
pub struct TwoKeysOneValueBinopTable<E: Engine, B: Binop> {
    table_entries: [Vec<E::Fr>; 3],
    table_lookup_map: std::collections::HashMap<(E::Fr, E::Fr), E::Fr>,
    bits: usize,
    name: &'static str,
    _binop_marker: std::marker::PhantomData<B>,
}

impl<E: Engine, B: Binop> TwoKeysOneValueBinopTable<E, B> {
    pub fn new(bits: usize, name: &'static str) -> Self {
        let mut key_0 = Vec::with_capacity(1 << bits);
        let mut key_1 = Vec::with_capacity(1 << bits);
        let mut value_0 = Vec::with_capacity(1 << bits);

        let mut map = std::collections::HashMap::with_capacity(1 << (bits * 2));
        for x in 0..(1 << bits) {
            for y in 0..(1 << bits) {
                let z = B::apply(x, y);

                let x = E::Fr::from_str(&x.to_string()).unwrap();
                let y = E::Fr::from_str(&y.to_string()).unwrap();
                let z = E::Fr::from_str(&z.to_string()).unwrap();
                key_0.push(x);
                key_1.push(y);
                value_0.push(z);

                map.insert((x, y), z);
            }
        }

        Self {
            table_entries: [key_0, key_1, value_0],
            table_lookup_map: map,
            bits,
            name,
            _binop_marker: std::marker::PhantomData,
        }
    }
}

impl<E: Engine, B: Binop> std::fmt::Debug for TwoKeysOneValueBinopTable<E, B> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("TwoKeysOneValueBinopTable").field("bits", &self.bits).field("binop", &B::NAME).finish()
    }
}

impl<E: Engine, B: Binop> LookupTableInternal<E> for TwoKeysOneValueBinopTable<E, B> {
    fn name(&self) -> &'static str {
        self.name
    }
    fn table_size(&self) -> usize {
        debug_assert_eq!(1usize << (self.bits * 2), self.table_entries[0].len());
        1usize << (self.bits * 2)
    }
    fn num_keys(&self) -> usize {
        2
    }
    fn num_values(&self) -> usize {
        1
    }
    fn allows_combining(&self) -> bool {
        true
    }
    fn is_valid_entry(&self, keys: &[E::Fr], values: &[E::Fr]) -> bool {
        assert!(keys.len() == 2);
        assert!(values.len() == 1);

        if let Some(entry) = self.table_lookup_map.get(&(keys[0], keys[1])) {
            return entry == &values[0];
        }

        false
    }
    fn query(&self, keys: &[E::Fr]) -> Result<Vec<E::Fr>, SynthesisError> {
        assert!(keys.len() == 2);

        if let Some(entry) = self.table_lookup_map.get(&(keys[0], keys[1])) {
            return Ok(vec![*entry]);
        }

        Err(SynthesisError::Unsatisfiable)
    }

    fn get_table_values_for_polys(&self) -> Vec<Vec<E::Fr>> {
        vec![self.table_entries[0].clone(), self.table_entries[1].clone(), self.table_entries[2].clone()]
    }
    fn table_id(&self) -> E::Fr {
        table_id_from_string(self.name())
    }
    fn sort(&self, values: &[E::Fr], _column: usize) -> Result<Vec<E::Fr>, SynthesisError> {
        unimplemented!()
    }
    fn box_clone(&self) -> Box<dyn LookupTableInternal<E>> {
        Box::from(self.clone())
    }

    fn column_is_trivial(&self, column_num: usize) -> bool {
        assert!(column_num < 3);

        false
    }
}

pub fn table_id_from_string<F: PrimeField>(s: &str) -> F {
    let mut h = tiny_keccak::keccak256(s.as_bytes());
    for i in 0..4 {
        h[i] = 0u8;
    }

    use crate::pairing::ff::PrimeFieldRepr;

    let mut repr = F::Repr::default();
    repr.read_be(&h[..]).unwrap();

    F::from_repr(repr).unwrap()
}

#[derive(Clone)]
pub struct KeyValueSet<E: Engine> {
    pub inner: [E::Fr; 3],
}

impl<E: Engine> Copy for KeyValueSet<E> {}

impl<E: Engine> KeyValueSet<E> {
    pub fn new(set: [E::Fr; 3]) -> Self {
        Self { inner: set }
    }

    pub fn from_slice(input: &[E::Fr]) -> Self {
        debug_assert_eq!(input.len(), 3);
        let mut inner = [E::Fr::zero(); 3];
        inner.copy_from_slice(input);

        Self { inner }
    }
}

impl<E: Engine> std::hash::Hash for KeyValueSet<E> {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.inner.hash(state);
    }
}

impl<E: Engine> std::cmp::PartialEq for KeyValueSet<E> {
    fn eq(&self, other: &Self) -> bool {
        self.inner == other.inner
    }
}

impl<E: Engine> std::cmp::Eq for KeyValueSet<E> {}

impl<E: Engine> std::cmp::Ord for KeyValueSet<E> {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        match self.inner[0].into_repr().cmp(&other.inner[0].into_repr()) {
            std::cmp::Ordering::Equal => {
                // return self.inner[1].into_repr().cmp(&other.inner[1].into_repr());
                match self.inner[1].into_repr().cmp(&other.inner[1].into_repr()) {
                    std::cmp::Ordering::Equal => {
                        self.inner[2].into_repr().cmp(&other.inner[2].into_repr())
                        // match self.inner[2].into_repr().cmp(&other.inner[2].into_repr()) {
                        //     std::cmp::Ordering::Equal => {
                        //         panic!("keys and values have duality for {:?} and {:?}", &self, &other);
                        //     }
                        //     ord @ _ => {
                        //         return ord;
                        //     }
                        // }
                    }
                    ord @ _ => {
                        return ord;
                    }
                }
            }
            ord @ _ => {
                return ord;
            }
        }
    }
}

impl<E: Engine> std::cmp::PartialOrd for KeyValueSet<E> {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl<E: Engine> std::fmt::Debug for KeyValueSet<E> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("KeyValueSet").field("inner", &self.inner).finish()
    }
}

pub(crate) struct LookupDataHolder<E: Engine> {
    pub(crate) eta: E::Fr,
    pub(crate) f_poly_unpadded_values: Option<Polynomial<E::Fr, Values>>,
    pub(crate) t_poly_unpadded_values: Option<Polynomial<E::Fr, Values>>,
    pub(crate) t_shifted_unpadded_values: Option<Polynomial<E::Fr, Values>>,
    pub(crate) s_poly_unpadded_values: Option<Polynomial<E::Fr, Values>>,
    pub(crate) s_shifted_unpadded_values: Option<Polynomial<E::Fr, Values>>,
    // pub(crate) f_poly_monomial: Option<Polynomial<E::Fr, Coefficients>>,
    pub(crate) t_poly_monomial: Option<Polynomial<E::Fr, Coefficients>>,
    pub(crate) s_poly_monomial: Option<Polynomial<E::Fr, Coefficients>>,
    pub(crate) selector_poly_monomial: Option<Polynomial<E::Fr, Coefficients>>,
    pub(crate) table_type_poly_monomial: Option<Polynomial<E::Fr, Coefficients>>,
}

#[derive(Clone, Copy, Debug)]
pub(crate) struct LookupQuery<E: Engine> {
    pub(crate) s_at_z_omega: E::Fr,
    pub(crate) grand_product_at_z_omega: E::Fr,
    pub(crate) t_at_z: E::Fr,
    pub(crate) t_at_z_omega: E::Fr,
    pub(crate) selector_at_z: E::Fr,
    pub(crate) table_type_at_z: E::Fr,
}
