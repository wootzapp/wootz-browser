use crate::pairing::ff::{Field, PrimeField};
use crate::pairing::Engine;

use crate::SynthesisError;

use super::cs::{PlonkConstraintSystemParams, StateVariablesSet, TraceStepCoefficients};
use crate::plonk::cs::gates::Index as PlonkIndex;
use crate::plonk::cs::gates::Variable as PlonkVariable;

use super::cs::Circuit as PlonkCircuit;
use super::cs::ConstraintSystem as PlonkConstraintSystem;

use std::marker::PhantomData;

use std::collections::{HashMap, HashSet};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MergeLcVariant {
    AIsTheOnlyMeaningful,
    BIsTheOnlyMeaningful,
    MergeABWithConstantC,
    MergeACThroughConstantB,
    MergeBCThroughConstantA,
    CIsTheOnlyMeaningful,
}

impl MergeLcVariant {
    pub fn into_u8(&self) -> u8 {
        match self {
            MergeLcVariant::AIsTheOnlyMeaningful => 1u8,
            MergeLcVariant::BIsTheOnlyMeaningful => 2u8,
            MergeLcVariant::MergeABWithConstantC => 3u8,
            MergeLcVariant::MergeACThroughConstantB => 4u8,
            MergeLcVariant::MergeBCThroughConstantA => 5u8,
            MergeLcVariant::CIsTheOnlyMeaningful => 6u8,
        }
    }

    pub fn from_u8(value: u8) -> std::io::Result<Self> {
        let s = match value {
            1u8 => MergeLcVariant::AIsTheOnlyMeaningful,
            2u8 => MergeLcVariant::BIsTheOnlyMeaningful,
            3u8 => MergeLcVariant::MergeABWithConstantC,
            4u8 => MergeLcVariant::MergeACThroughConstantB,
            5u8 => MergeLcVariant::MergeBCThroughConstantA,
            6u8 => MergeLcVariant::CIsTheOnlyMeaningful,
            _ => {
                use std::io::{Error, ErrorKind};
                let custom_error = Error::new(ErrorKind::Other, "unknown LC merging variant");

                return Err(custom_error);
            }
        };

        Ok(s)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LcTransformationVariant {
    IsSingleVariable,
    IntoSingleGate,
    IntoMultipleGates,
    IsConstant,
}

impl LcTransformationVariant {
    pub fn into_u8(&self) -> u8 {
        match self {
            LcTransformationVariant::IsSingleVariable => 1u8,
            LcTransformationVariant::IntoSingleGate => 2u8,
            LcTransformationVariant::IntoMultipleGates => 3u8,
            LcTransformationVariant::IsConstant => 4u8,
        }
    }

    pub fn from_u8(value: u8) -> std::io::Result<Self> {
        let s = match value {
            1u8 => LcTransformationVariant::IsSingleVariable,
            2u8 => LcTransformationVariant::IntoSingleGate,
            3u8 => LcTransformationVariant::IntoMultipleGates,
            4u8 => LcTransformationVariant::IsConstant,
            _ => {
                use std::io::{Error, ErrorKind};
                let custom_error = Error::new(ErrorKind::Other, "unknown LC transformation variant");

                return Err(custom_error);
            }
        };

        Ok(s)
    }
}

struct TranspilationScratchSpace<E: Engine> {
    scratch_space_for_vars: Vec<PlonkVariable>,
    scratch_space_for_coeffs: Vec<E::Fr>,
    scratch_space_for_booleans: Vec<bool>,
}

impl<E: Engine> TranspilationScratchSpace<E> {
    fn new(width: usize) -> Self {
        Self {
            scratch_space_for_vars: Vec::with_capacity(width),
            scratch_space_for_coeffs: Vec::with_capacity(width),
            scratch_space_for_booleans: Vec::with_capacity(width),
        }
    }

    fn clear(&mut self) {
        self.scratch_space_for_vars.truncate(0);
        self.scratch_space_for_coeffs.truncate(0);
        self.scratch_space_for_booleans.truncate(0);
    }
}

// These are transpilation options over A * B - C = 0 constraint
#[derive(Clone, Copy, PartialEq, Eq)]
pub enum TranspilationVariant {
    IntoQuadraticGate,
    IntoAdditionGate(LcTransformationVariant),
    MergeLinearCombinations(MergeLcVariant, LcTransformationVariant),
    IntoMultiplicationGate((LcTransformationVariant, LcTransformationVariant, LcTransformationVariant)),
}

use crate::byteorder::BigEndian;
use crate::byteorder::ReadBytesExt;
use crate::byteorder::WriteBytesExt;
use std::io::{Read, Write};

impl TranspilationVariant {
    pub fn into_u8(&self) -> u8 {
        match self {
            TranspilationVariant::IntoQuadraticGate => 1u8,
            TranspilationVariant::IntoAdditionGate(_) => 2u8,
            TranspilationVariant::MergeLinearCombinations(_, _) => 3u8,
            TranspilationVariant::IntoMultiplicationGate(_) => 4u8,
        }
    }

    pub fn write<W: Write>(&self, mut writer: W) -> std::io::Result<()> {
        let prefix = self.into_u8();
        writer.write_u8(prefix)?;
        match self {
            TranspilationVariant::IntoAdditionGate(subhint) => {
                let subhint = subhint.into_u8();
                writer.write_u8(subhint)?;
            }
            TranspilationVariant::MergeLinearCombinations(variant, subhint) => {
                let variant = variant.into_u8();
                writer.write_u8(variant)?;

                let subhint = subhint.into_u8();
                writer.write_u8(subhint)?;
            }
            TranspilationVariant::IntoMultiplicationGate(hints) => {
                let (h_a, h_b, h_c) = hints;
                writer.write_u8(h_a.into_u8())?;
                writer.write_u8(h_b.into_u8())?;
                writer.write_u8(h_c.into_u8())?;
            }
            _ => {}
        }

        Ok(())
    }

    pub fn read<R: Read>(mut reader: R) -> std::io::Result<Self> {
        let prefix = reader.read_u8()?;

        match prefix {
            1u8 => {
                return Ok(TranspilationVariant::IntoQuadraticGate);
            }
            2u8 => {
                let subhint = LcTransformationVariant::from_u8(reader.read_u8()?)?;

                return Ok(TranspilationVariant::IntoAdditionGate(subhint));
            }
            3u8 => {
                let variant = MergeLcVariant::from_u8(reader.read_u8()?)?;
                let subhint = LcTransformationVariant::from_u8(reader.read_u8()?)?;

                return Ok(TranspilationVariant::MergeLinearCombinations(variant, subhint));
            }
            4u8 => {
                let subhint_a = LcTransformationVariant::from_u8(reader.read_u8()?)?;
                let subhint_b = LcTransformationVariant::from_u8(reader.read_u8()?)?;
                let subhint_c = LcTransformationVariant::from_u8(reader.read_u8()?)?;

                return Ok(TranspilationVariant::IntoMultiplicationGate((subhint_a, subhint_b, subhint_c)));
            }
            _ => {}
        }

        use std::io::{Error, ErrorKind};
        let custom_error = Error::new(ErrorKind::Other, "unknown transpilation variant");

        Err(custom_error)
    }
}

pub fn read_transpilation_hints<R: Read>(mut reader: R) -> std::io::Result<Vec<(usize, TranspilationVariant)>> {
    let num_hints = reader.read_u64::<BigEndian>()?;
    let mut hints = Vec::with_capacity(num_hints as usize);

    for _ in 0..num_hints {
        let idx = reader.read_u64::<BigEndian>()?;
        let hint = TranspilationVariant::read(&mut reader)?;
        hints.push((idx as usize, hint));
    }

    Ok(hints)
}

pub fn write_transpilation_hints<W: Write>(hints: &Vec<(usize, TranspilationVariant)>, mut writer: W) -> std::io::Result<()> {
    writer.write_u64::<BigEndian>(hints.len() as u64)?;
    for (idx, h) in hints.iter() {
        writer.write_u64::<BigEndian>(*idx as u64)?;
        h.write(&mut writer)?;
    }

    Ok(())
}

impl std::fmt::Debug for TranspilationVariant {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            TranspilationVariant::IntoQuadraticGate => {
                writeln!(f, "Variant: into quadratic gate")?;
            }
            TranspilationVariant::IntoAdditionGate(_) => {
                writeln!(f, "Variant: make an addition gate")?;
            }
            TranspilationVariant::MergeLinearCombinations(merge_type, _) => {
                writeln!(f, "Variant: merge linear combinations as {:?}", merge_type)?;
            }
            TranspilationVariant::IntoMultiplicationGate(b) => {
                writeln!(f, "Variant: into combinatoric multiplication gate")?;
                writeln!(f, "A: {:?}", b.0)?;
                writeln!(f, "B: {:?}", b.1)?;
                writeln!(f, "C: {:?}", b.2)?;
            }
        }

        Ok(())
    }
}

pub struct Transpiler<E: Engine, P: PlonkConstraintSystemParams<E>> {
    current_constraint_index: usize,
    current_plonk_input_idx: usize,
    current_plonk_aux_idx: usize,
    scratch: HashSet<crate::cs::Variable>,
    // deduplication_scratch: HashMap<crate::cs::Variable, E::Fr>,
    deduplication_scratch: HashMap<crate::cs::Variable, usize>,
    transpilation_scratch_space: Option<TranspilationScratchSpace<E>>,
    hints: Vec<(usize, TranspilationVariant)>,
    n: usize,
    _marker_e: std::marker::PhantomData<E>,
    _marker_p: std::marker::PhantomData<P>,
}

// by convention last coefficient is a coefficient for a jump to the next step
fn allocate_into_cs<E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P>>(
    cs: &mut CS,
    needs_next_step: bool,
    variables: &[PlonkVariable],
    coefficients: &[E::Fr],
) -> Result<(), SynthesisError> {
    if needs_next_step {
        debug_assert!(coefficients.len() == P::STATE_WIDTH + 1 + 1 + 1);
        debug_assert!(P::CAN_ACCESS_NEXT_TRACE_STEP);

        cs.new_gate(
            P::StateVariables::from_variables(variables),
            P::ThisTraceStepCoefficients::from_coeffs(&coefficients[0..(P::STATE_WIDTH + 2)]),
            P::NextTraceStepCoefficients::from_coeffs(&coefficients[(P::STATE_WIDTH + 2)..]),
        )?;
    } else {
        debug_assert!(coefficients.len() >= P::STATE_WIDTH + 1 + 1);
        debug_assert!(coefficients.last().unwrap().is_zero());

        cs.new_gate(
            P::StateVariables::from_variables(variables),
            P::ThisTraceStepCoefficients::from_coeffs(&coefficients[0..(P::STATE_WIDTH + 2)]),
            P::NextTraceStepCoefficients::from_coeffs(&coefficients[(P::STATE_WIDTH + 2)..]),
        )?;
    }

    Ok(())
}

fn evaluate_lc<E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P>>(
    cs: &CS,
    lc: &LinearCombination<E>,
    // multiplier: E::Fr,
    free_term_constant: E::Fr,
) -> Result<E::Fr, SynthesisError> {
    let mut final_value = E::Fr::zero();
    for (var, coeff) in lc.as_ref().iter() {
        let mut may_be_value = cs.get_value(convert_variable(*var))?;
        may_be_value.mul_assign(&coeff);
        final_value.add_assign(&may_be_value);
    }

    final_value.add_assign(&free_term_constant);

    Ok(final_value)
}

fn evaluate_over_variables<E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P>>(
    cs: &CS,
    variables: &[(Variable, E::Fr)],
    free_term_constant: E::Fr,
) -> Result<E::Fr, SynthesisError> {
    let mut final_value = E::Fr::zero();
    for (var, coeff) in variables.iter() {
        let mut may_be_value = cs.get_value(convert_variable(*var))?;
        may_be_value.mul_assign(&coeff);
        final_value.add_assign(&may_be_value);
    }

    final_value.add_assign(&free_term_constant);

    Ok(final_value)
}

fn evaluate_over_plonk_variables<E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P>>(
    cs: &CS,
    variables: &[(PlonkVariable, E::Fr)],
    free_term_constant: E::Fr,
) -> Result<E::Fr, SynthesisError> {
    let mut final_value = E::Fr::zero();
    for (var, coeff) in variables.iter() {
        let mut may_be_value = cs.get_value(*var)?;
        may_be_value.mul_assign(&coeff);
        final_value.add_assign(&may_be_value);
    }

    final_value.add_assign(&free_term_constant);

    Ok(final_value)
}

fn evaluate_over_plonk_variables_and_coeffs<E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P>>(
    cs: &CS,
    variables: &[PlonkVariable],
    coeffs: &[E::Fr],
    free_term_constant: E::Fr,
) -> Result<E::Fr, SynthesisError> {
    debug_assert_eq!(variables.len(), coeffs.len());
    let mut final_value = E::Fr::zero();
    for (var, coeff) in variables.iter().zip(coeffs.iter()) {
        let mut may_be_value = cs.get_value(*var)?;
        may_be_value.mul_assign(&coeff);
        final_value.add_assign(&may_be_value);
    }

    final_value.add_assign(&free_term_constant);

    Ok(final_value)
}

fn enforce_lc_as_gates<E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P>>(
    cs: &mut CS,
    mut lc: LinearCombination<E>,
    multiplier: E::Fr,
    free_term_constant: E::Fr,
    collapse_into_single_variable: bool,
    scratch_space: &mut TranspilationScratchSpace<E>,
) -> Result<(Option<PlonkVariable>, E::Fr, LcTransformationVariant), SynthesisError> {
    assert!(P::CAN_ACCESS_NEXT_TRACE_STEP, "Transliper only works for proof systems with access to next step");

    assert!(scratch_space.scratch_space_for_vars.len() == 0);
    assert!(scratch_space.scratch_space_for_coeffs.len() == 0);
    assert!(scratch_space.scratch_space_for_booleans.len() == 0);

    let zero_fr = E::Fr::zero();
    let one_fr = E::Fr::one();
    let mut minus_one_fr = E::Fr::one();
    minus_one_fr.negate();

    // trivial case - single variable

    assert!(lc.0.len() > 0);

    if lc.0.len() == 1 {
        if free_term_constant.is_zero() {
            // this linear combination contains only one variable and no constant
            // term, so we just leave it as is,
            // but ONLY if we just need to collapse LC into a variable
            if collapse_into_single_variable {
                let (var, coeff) = lc.0[0];

                return Ok((Some(convert_variable(var)), coeff, LcTransformationVariant::IsSingleVariable));
            }
        }
    }

    // everything else should be handled here by making a new variable

    // scale if necessary
    if multiplier.is_zero() {
        assert!(free_term_constant.is_zero());
        unreachable!();
        // it's a constraint 0 * LC = 0
    } else {
        if multiplier != one_fr {
            for (_, c) in lc.0.iter_mut() {
                c.mul_assign(&multiplier);
            }
        }
    }

    // if we need to collaplse an LC into a single variable for
    // future use we allocate it and then subtract it from linear combination
    // to have an enforcement LC == 0 in all the cases

    let final_variable = if collapse_into_single_variable {
        let may_be_new_value = evaluate_lc::<E, P, CS>(&*cs, &lc, free_term_constant);
        let new_var = cs.alloc(|| may_be_new_value)?;

        Some(new_var)
    } else {
        None
    };

    if let Some(var) = final_variable {
        subtract_variable_unchecked(&mut lc, convert_variable_back(var));
    }

    let num_terms = lc.0.len();

    // we have two options:
    // - fit everything into a single gate (in case of number terms in the linear combination
    // smaller than a width of the state)
    // - make a required number of extra variables and chain it

    if num_terms <= P::STATE_WIDTH {
        // we can just make a single gate

        // fill the gate with nothing first and replace after
        scratch_space.scratch_space_for_vars.resize(P::STATE_WIDTH, cs.get_dummy_variable());
        scratch_space.scratch_space_for_booleans.resize(P::STATE_WIDTH, false);
        scratch_space.scratch_space_for_coeffs.resize(P::STATE_WIDTH, zero_fr);

        let it = lc.0.into_iter();

        // we can consume and never have leftovers
        let mut idx = 0;
        for (var, coeff) in it {
            if scratch_space.scratch_space_for_booleans[idx] == false {
                scratch_space.scratch_space_for_booleans[idx] = true;
                scratch_space.scratch_space_for_coeffs[idx] = coeff;
                scratch_space.scratch_space_for_vars[idx] = convert_variable(var);
                idx += 1;
            }
        }

        // add multiplication coefficient, constant and next step one

        scratch_space.scratch_space_for_coeffs.push(zero_fr);
        scratch_space.scratch_space_for_coeffs.push(free_term_constant);
        scratch_space.scratch_space_for_coeffs.push(zero_fr);

        allocate_into_cs(cs, false, &*scratch_space.scratch_space_for_vars, &*scratch_space.scratch_space_for_coeffs)?;

        scratch_space.clear();

        let hint = LcTransformationVariant::IntoSingleGate;

        return Ok((final_variable, one_fr, hint));
    } else {
        // we can take:
        // - STATE_WIDTH variables to form the first gate and place their sum into the last wire of the next gate
        // - every time take STATE_WIDTH-1 variables and place their sum + last wire into the next gate last wire

        // we have also made a final variable already, so there is NO difference
        let cycles = ((lc.0.len() - P::STATE_WIDTH) + (P::STATE_WIDTH - 2)) / (P::STATE_WIDTH - 1); // ceil
        let mut it = lc.0.into_iter();

        // this is a placeholder variable that must go into the
        // corresponding trace polynomial at the NEXT time step
        let mut next_step_var_in_chain = {
            scratch_space.scratch_space_for_vars.resize(P::STATE_WIDTH, cs.get_dummy_variable());
            scratch_space.scratch_space_for_booleans.resize(P::STATE_WIDTH, false);
            scratch_space.scratch_space_for_coeffs.resize(P::STATE_WIDTH, zero_fr);

            // we can consume and never have leftovers

            let mut idx = 0;
            for (var, coeff) in &mut it {
                if scratch_space.scratch_space_for_booleans[idx] == false {
                    scratch_space.scratch_space_for_booleans[idx] = true;
                    scratch_space.scratch_space_for_coeffs[idx] = coeff;
                    scratch_space.scratch_space_for_vars[idx] = convert_variable(var);
                    idx += 1;
                    if idx == P::STATE_WIDTH {
                        break;
                    }
                }
            }

            // for a P::STATE_WIDTH variables we make a corresponding LC
            // ~ a + b + c + d + constant. That will be equal to d_next
            let may_be_new_intermediate_value =
                evaluate_over_plonk_variables_and_coeffs::<E, P, CS>(&*cs, &*scratch_space.scratch_space_for_vars, &*scratch_space.scratch_space_for_coeffs, free_term_constant);

            // we manually allocate the new variable
            let new_intermediate_var = cs.alloc(|| may_be_new_intermediate_value)?;

            // no multiplication coefficient,
            // but -1 to link to the next trace step (we enforce == 0)
            scratch_space.scratch_space_for_coeffs.push(zero_fr); // no multiplication
            scratch_space.scratch_space_for_coeffs.push(free_term_constant); // add constant
            scratch_space.scratch_space_for_coeffs.push(minus_one_fr); // -1 for a d_next

            allocate_into_cs(cs, true, &*scratch_space.scratch_space_for_vars, &*scratch_space.scratch_space_for_coeffs)?;

            scratch_space.clear();

            new_intermediate_var
        };

        // run over the rest

        // we can only take one less cause
        // we've already used one of the variable
        let consume_from_lc = P::STATE_WIDTH - 1;
        for _ in 0..(cycles - 1) {
            scratch_space.scratch_space_for_vars.resize(consume_from_lc, cs.get_dummy_variable());
            scratch_space.scratch_space_for_booleans.resize(consume_from_lc, false);
            scratch_space.scratch_space_for_coeffs.resize(consume_from_lc, zero_fr);

            // we can consume and never have leftovers

            let mut idx = 0;
            for (var, coeff) in &mut it {
                if scratch_space.scratch_space_for_booleans[idx] == false {
                    scratch_space.scratch_space_for_booleans[idx] = true;
                    scratch_space.scratch_space_for_coeffs[idx] = coeff;
                    scratch_space.scratch_space_for_vars[idx] = convert_variable(var);
                    idx += 1;
                    if idx == consume_from_lc {
                        break;
                    }
                }
            }

            // push +1 and the allocated variable from the previous step

            scratch_space.scratch_space_for_coeffs.push(one_fr);
            scratch_space.scratch_space_for_vars.push(next_step_var_in_chain);

            let may_be_new_intermediate_value = evaluate_over_plonk_variables_and_coeffs::<E, P, CS>(&*cs, &*scratch_space.scratch_space_for_vars, &*scratch_space.scratch_space_for_coeffs, zero_fr);

            let new_intermediate_var = cs.alloc(|| may_be_new_intermediate_value)?;

            // no multiplication coefficient and no constant now,
            // but -1 to link to the next trace step
            scratch_space.scratch_space_for_coeffs.push(zero_fr);
            scratch_space.scratch_space_for_coeffs.push(zero_fr);
            scratch_space.scratch_space_for_coeffs.push(minus_one_fr);

            allocate_into_cs(cs, true, &*scratch_space.scratch_space_for_vars, &*scratch_space.scratch_space_for_coeffs)?;

            scratch_space.clear();

            next_step_var_in_chain = new_intermediate_var;
        }

        // final step - we just make a single gate, last one
        {
            scratch_space.scratch_space_for_vars.resize(P::STATE_WIDTH - 1, cs.get_dummy_variable());
            scratch_space.scratch_space_for_booleans.resize(P::STATE_WIDTH - 1, false);
            scratch_space.scratch_space_for_coeffs.resize(P::STATE_WIDTH - 1, zero_fr);

            // we can consume and never have leftovers

            let mut idx = 0;
            for (var, coeff) in &mut it {
                if scratch_space.scratch_space_for_booleans[idx] == false {
                    scratch_space.scratch_space_for_booleans[idx] = true;
                    scratch_space.scratch_space_for_coeffs[idx] = coeff;
                    scratch_space.scratch_space_for_vars[idx] = convert_variable(var);
                    idx += 1;
                }
            }

            assert!(idx < P::STATE_WIDTH);
            // append d_next
            scratch_space.scratch_space_for_vars.push(next_step_var_in_chain);
            scratch_space.scratch_space_for_coeffs.push(one_fr);

            // no multiplication coefficient, no constant, no next step
            scratch_space.scratch_space_for_coeffs.push(zero_fr);
            scratch_space.scratch_space_for_coeffs.push(zero_fr);
            scratch_space.scratch_space_for_coeffs.push(zero_fr);

            allocate_into_cs(cs, false, &*scratch_space.scratch_space_for_vars, &*scratch_space.scratch_space_for_coeffs)?;

            scratch_space.clear();
        }

        assert!(it.next().is_none());

        let hint = LcTransformationVariant::IntoMultipleGates;

        return Ok((final_variable, one_fr, hint));
    }
}

impl<E: Engine, P: PlonkConstraintSystemParams<E>> Transpiler<E, P> {
    pub fn new() -> Self {
        Self {
            current_constraint_index: 0,
            current_plonk_input_idx: 0,
            current_plonk_aux_idx: 0,
            scratch: HashSet::with_capacity((E::Fr::NUM_BITS * 2) as usize),
            deduplication_scratch: HashMap::with_capacity((E::Fr::NUM_BITS * 2) as usize),
            transpilation_scratch_space: Some(TranspilationScratchSpace::<E>::new(P::STATE_WIDTH * 2)),
            hints: vec![],
            n: 0usize,
            _marker_e: std::marker::PhantomData,
            _marker_p: std::marker::PhantomData,
        }
    }

    pub fn num_gates(&self) -> usize {
        self.n
    }

    pub fn into_hints(self) -> Vec<(usize, TranspilationVariant)> {
        self.hints
    }

    pub fn into_hints_and_num_gates(self) -> (usize, Vec<(usize, TranspilationVariant)>) {
        (self.n, self.hints)
    }

    fn increment_lc_number(&mut self) -> usize {
        let current_lc_number = self.current_constraint_index;
        self.current_constraint_index += 1;

        current_lc_number
    }
}

impl<E: Engine, P: PlonkConstraintSystemParams<E>> PlonkConstraintSystem<E, P> for Transpiler<E, P> {
    fn alloc<F>(&mut self, value: F) -> Result<PlonkVariable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>,
    {
        let var = crate::ConstraintSystem::<E>::alloc(self, || "alloc aux var", value)?;

        Ok(convert_variable(var))
    }

    // allocate an input variable
    fn alloc_input<F>(&mut self, value: F) -> Result<PlonkVariable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>,
    {
        let var = crate::ConstraintSystem::<E>::alloc_input(self, || "alloc input var", value)?;

        self.n += 1;

        Ok(convert_variable(var))
    }

    fn new_gate(&mut self, _variables: P::StateVariables, _this_step_coeffs: P::ThisTraceStepCoefficients, _next_step_coeffs: P::NextTraceStepCoefficients) -> Result<(), SynthesisError> {
        // Transpiler does NOT allocate any gates himself
        self.n += 1;

        Ok(())
    }

    fn get_dummy_variable(&self) -> PlonkVariable {
        PlonkVariable::new_unchecked(PlonkIndex::Aux(0))
    }
}

impl<'a, E: Engine, P: PlonkConstraintSystemParams<E>> PlonkConstraintSystem<E, P> for &'a mut Transpiler<E, P> {
    fn alloc<F>(&mut self, value: F) -> Result<PlonkVariable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>,
    {
        let var = crate::ConstraintSystem::<E>::alloc(self, || "alloc aux var", value)?;

        Ok(convert_variable(var))
    }

    // allocate an input variable
    fn alloc_input<F>(&mut self, value: F) -> Result<PlonkVariable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>,
    {
        let var = crate::ConstraintSystem::<E>::alloc_input(self, || "alloc input var", value)?;

        self.n += 1;

        Ok(convert_variable(var))
    }

    fn new_gate(&mut self, _variables: P::StateVariables, _this_step_coeffs: P::ThisTraceStepCoefficients, _next_step_coeffs: P::NextTraceStepCoefficients) -> Result<(), SynthesisError> {
        // Transpiler does NOT allocate any gates himself
        self.n += 1;

        Ok(())
    }

    fn get_dummy_variable(&self) -> PlonkVariable {
        PlonkVariable::new_unchecked(PlonkIndex::Aux(0))
    }
}

impl<E: Engine, P: PlonkConstraintSystemParams<E>> crate::ConstraintSystem<E> for Transpiler<E, P> {
    type Root = Self;

    fn one() -> crate::Variable {
        crate::Variable::new_unchecked(crate::Index::Input(0))
    }

    fn alloc<F, A, AR>(&mut self, _: A, _f: F) -> Result<crate::Variable, crate::SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, crate::SynthesisError>,
        A: FnOnce() -> AR,
        AR: Into<String>,
    {
        self.current_plonk_aux_idx += 1;

        Ok(crate::Variable::new_unchecked(crate::Index::Aux(self.current_plonk_aux_idx)))
    }

    fn alloc_input<F, A, AR>(&mut self, _: A, _f: F) -> Result<crate::Variable, crate::SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, crate::SynthesisError>,
        A: FnOnce() -> AR,
        AR: Into<String>,
    {
        self.current_plonk_input_idx += 1;

        Ok(crate::Variable::new_unchecked(crate::Index::Input(self.current_plonk_input_idx)))
    }

    fn enforce<A, AR, LA, LB, LC>(&mut self, _ann: A, a: LA, b: LB, c: LC)
    where
        A: FnOnce() -> AR,
        AR: Into<String>,
        LA: FnOnce(crate::LinearCombination<E>) -> crate::LinearCombination<E>,
        LB: FnOnce(crate::LinearCombination<E>) -> crate::LinearCombination<E>,
        LC: FnOnce(crate::LinearCombination<E>) -> crate::LinearCombination<E>,
    {
        let zero_fr = E::Fr::zero();
        let one_fr = E::Fr::one();

        let mut negative_one_fr = E::Fr::one();
        negative_one_fr.negate();

        // we need to determine the type of transformation constraint

        // let's handle trivial cases first

        // A or B or C are just constant terms

        let (a_has_constant, a_constant_term, a_lc_is_empty, a_lc) = deduplicate_and_split_linear_term::<E, Self>(a(crate::LinearCombination::zero()), &mut self.deduplication_scratch);
        let (b_has_constant, b_constant_term, b_lc_is_empty, b_lc) = deduplicate_and_split_linear_term::<E, Self>(b(crate::LinearCombination::zero()), &mut self.deduplication_scratch);
        let (c_has_constant, c_constant_term, c_lc_is_empty, c_lc) = deduplicate_and_split_linear_term::<E, Self>(c(crate::LinearCombination::zero()), &mut self.deduplication_scratch);

        let a_is_constant = a_has_constant & a_lc_is_empty;
        let b_is_constant = b_has_constant & b_lc_is_empty;
        let c_is_constant = c_has_constant & c_lc_is_empty;

        // debug_assert!(a_has_constant || !a_lc_is_empty);
        // debug_assert!(b_has_constant || !b_lc_is_empty);
        // debug_assert!(c_has_constant || !c_lc_is_empty);

        match (a_is_constant, b_is_constant, c_is_constant) {
            (true, true, true) => {
                unreachable!("R1CS has a gate 1 * 1 = 1");
            }
            (true, false, true) | (false, true, true) => {
                // println!("C * LC = C");
                // we have something like c0 * LC = c1
                // do we form an "addition gate", that may take more than
                // one gate itself
                let lc = if !a_is_constant {
                    a_lc
                } else if !b_is_constant {
                    b_lc
                } else {
                    unreachable!("Either A or B LCs are constant");
                };

                let multiplier = if a_is_constant {
                    a_constant_term
                } else if b_is_constant {
                    b_constant_term
                } else {
                    unreachable!("Must take multiplier from A or B");
                };

                let mut free_constant_term = if a_is_constant {
                    b_constant_term
                } else if b_is_constant {
                    a_constant_term
                } else {
                    unreachable!("Either A or B LCs are constant");
                };

                free_constant_term.mul_assign(&multiplier);
                free_constant_term.sub_assign(&c_constant_term);

                let mut space = self.transpilation_scratch_space.take().unwrap();

                let (_, _, hint) = enforce_lc_as_gates(self, lc, multiplier, free_constant_term, false, &mut space).expect("must allocate LCs as gates for constraint like c0 * LC = c1");

                self.transpilation_scratch_space = Some(space);

                let current_lc_number = self.increment_lc_number();

                // println!("Hint = {:?}", hint);

                self.hints.push((current_lc_number, TranspilationVariant::IntoAdditionGate(hint)));

                return;
            }
            (false, false, true) => {
                // println!("LC * LC = C");
                // potential quadatic gate, but ig general
                // it's a full multiplication gate
                let (is_quadratic_gate, _coeffs) = check_for_quadratic_gate::<E>(&a_lc, &b_lc, &c_lc, a_constant_term, b_constant_term, c_constant_term);
                if is_quadratic_gate {
                    let current_lc_number = self.increment_lc_number();
                    // we don't pass a call to any function that allocates a gate,
                    // so we count ourselves
                    self.n += 1;

                    let hint = TranspilationVariant::IntoQuadraticGate;

                    // println!("Hint = {:?}", hint);

                    self.hints.push((current_lc_number, hint));

                    return;
                }

                let mut space = self.transpilation_scratch_space.take().unwrap();

                let (_new_a_var, _, hint_a) = enforce_lc_as_gates(self, a_lc, one_fr, a_constant_term, true, &mut space).expect("must allocate A LC as gates for constraint like LC * LC = c1");

                let (_new_b_var, _, hint_b) = enforce_lc_as_gates(self, b_lc, one_fr, b_constant_term, true, &mut space).expect("must allocate B LC as gates for constraint like LC * LC = c1");

                self.transpilation_scratch_space = Some(space);

                let current_lc_number = self.increment_lc_number();

                let hint_c = LcTransformationVariant::IsConstant;

                let hint = TranspilationVariant::IntoMultiplicationGate((hint_a, hint_b, hint_c));
                // we don't pass a call to any function that allocates a gate,
                // so we count ourselves
                self.n += 1;

                // println!("Hint = {:?}", hint);

                self.hints.push((current_lc_number, hint));
            }
            (true, false, false) | (false, true, false) => {
                // sometihng like LC * const = LC
                // so we can merge them into one long linear combination
                let multiplier = if a_is_constant {
                    a_constant_term
                } else if b_is_constant {
                    b_constant_term
                } else {
                    unreachable!()
                };

                let lc_variant = if a_is_constant {
                    MergeLcVariant::MergeBCThroughConstantA
                } else {
                    MergeLcVariant::MergeACThroughConstantB
                };

                if multiplier == zero_fr {
                    // LC_AB * 0 = LC_C => LC_C == 0
                    // not sure that any sane circuit will have this,
                    // but we cover this variant

                    let mut space = self.transpilation_scratch_space.take().unwrap();

                    let (_, _, hint_c) = enforce_lc_as_gates(self, c_lc, one_fr, c_constant_term, false, &mut space).expect("must allocate LCs as gates for constraint like 0 = LC_C");

                    self.transpilation_scratch_space = Some(space);

                    let current_lc_number = self.increment_lc_number();

                    let hint = TranspilationVariant::MergeLinearCombinations(MergeLcVariant::CIsTheOnlyMeaningful, hint_c);

                    // println!("Hint = {:?}", hint);

                    self.hints.push((current_lc_number, hint));

                    return;
                }

                let mut final_lc = if !a_is_constant {
                    a_lc
                } else if !b_is_constant {
                    b_lc
                } else {
                    unreachable!()
                };

                if multiplier != one_fr {
                    for (_, c) in final_lc.0.iter_mut() {
                        c.mul_assign(&multiplier);
                    }
                }

                let mut free_constant_term = if a_is_constant {
                    a_constant_term
                } else if b_is_constant {
                    b_constant_term
                } else {
                    unreachable!()
                };
                free_constant_term.mul_assign(&multiplier);
                free_constant_term.sub_assign(&c_constant_term);

                // let final_lc = final_lc - &c;
                let final_lc = subtract_lcs_with_dedup_stable::<E, Self>(final_lc, c_lc, &mut self.deduplication_scratch);

                let mut space = self.transpilation_scratch_space.take().unwrap();

                let (_, _, hint_lc) = enforce_lc_as_gates(self, final_lc, one_fr, free_constant_term, false, &mut space).expect("must allocate LCs as gates for constraint like c0 * LC = LC");

                self.transpilation_scratch_space = Some(space);

                let current_lc_number = self.increment_lc_number();

                let hint = TranspilationVariant::MergeLinearCombinations(lc_variant, hint_lc);

                // println!("Hint = {:?}", hint);

                self.hints.push((current_lc_number, hint));

                return;
            }
            (true, true, false) => {
                // const * const = LC
                // A and B are some constants
                // also strange one, but we cover all the options exhaustively
                let mut tmp = a_constant_term;
                tmp.mul_assign(&b_constant_term);

                let mut free_constant_term = c_constant_term;
                free_constant_term.sub_assign(&tmp);

                let mut space = self.transpilation_scratch_space.take().unwrap();

                let (_, _, hint_lc) = enforce_lc_as_gates(self, c_lc, one_fr, free_constant_term, false, &mut space).expect("must allocate LCs as gates for constraint like c0 * c1 = LC");

                self.transpilation_scratch_space = Some(space);

                let current_lc_number = self.increment_lc_number();

                let hint = TranspilationVariant::MergeLinearCombinations(MergeLcVariant::CIsTheOnlyMeaningful, hint_lc);

                // println!("Hint = {:?}", hint);

                self.hints.push((current_lc_number, hint));
            }
            (false, false, false) => {
                // LC * LC = LC
                // potentially it can still be quadratic
                let (is_quadratic_gate, _coeffs) = is_quadratic_gate::<E, Self>(&a_lc, &b_lc, &c_lc, &mut self.scratch);
                if is_quadratic_gate {
                    let current_lc_number = self.increment_lc_number();

                    let hint = TranspilationVariant::IntoQuadraticGate;
                    // we don't pass a call to any function that allocates a gate,
                    // so we count ourselves
                    self.n += 1;

                    // println!("Hint = {:?}", hint);

                    self.hints.push((current_lc_number, hint));

                    return;
                }

                // rewrite into addition gates and multiplication gates

                let mut space = self.transpilation_scratch_space.take().unwrap();

                let (_new_a_var, _, hint_a) = enforce_lc_as_gates(self, a_lc, one_fr, a_constant_term, true, &mut space).expect("must allocate A LC as gates for constraint like LC * LC = LC");
                let (_new_b_var, _, hint_b) = enforce_lc_as_gates(self, b_lc, one_fr, b_constant_term, true, &mut space).expect("must allocate B LC as gates for constraint like LC * LC = LC");
                let (_new_c_var, _, hint_c) = enforce_lc_as_gates(self, c_lc, one_fr, c_constant_term, true, &mut space).expect("must allocate C LC as gates for constraint like LC * LC = LC");

                self.transpilation_scratch_space = Some(space);

                let current_lc_number = self.increment_lc_number();

                let hint = TranspilationVariant::IntoMultiplicationGate((hint_a, hint_b, hint_c));
                // we don't pass a call to any function that allocates a gate,
                // so we count ourselves
                self.n += 1;

                // println!("Hint = {:?}", hint);

                self.hints.push((current_lc_number, hint));
            }
        }
    }

    fn push_namespace<NR, N>(&mut self, _: N)
    where
        NR: Into<String>,
        N: FnOnce() -> NR,
    {
        // Do nothing; we don't care about namespaces in this context.
    }

    fn pop_namespace(&mut self) {
        // Do nothing; we don't care about namespaces in this context.
    }

    fn get_root(&mut self) -> &mut Self::Root {
        self
    }
}

// List of heuristics

use crate::{ConstraintSystem, LinearCombination, Variable};

fn is_quadratic_gate<E: Engine, CS: ConstraintSystem<E>>(
    a: &LinearCombination<E>,
    b: &LinearCombination<E>,
    c: &LinearCombination<E>,
    scratch: &mut HashSet<crate::cs::Variable>,
) -> (bool, (E::Fr, E::Fr, E::Fr)) {
    let zero = E::Fr::zero();

    let (_a_containts_constant, a_constant_coeff) = get_constant_term::<E, CS>(&a);
    let (_b_containts_constant, b_constant_coeff) = get_constant_term::<E, CS>(&b);
    let (_c_containts_constant, c_constant_coeff) = get_constant_term::<E, CS>(&c);

    let (a_is_linear, a_linear_var, a_linear_var_coeff) = is_linear_term::<E, CS>(&a, scratch);
    let (b_is_linear, b_linear_var, b_linear_var_coeff) = is_linear_term::<E, CS>(&b, scratch);
    let (c_is_linear, c_linear_var, c_linear_var_coeff) = is_linear_term::<E, CS>(&c, scratch);

    let (c_is_constant, _) = is_constant::<E, CS>(&c);

    let is_quadratic;
    if c_is_constant {
        is_quadratic = a_is_linear && b_is_linear && a_linear_var == b_linear_var;
    } else {
        if a_is_linear && b_is_linear && c_is_linear && a_linear_var == b_linear_var && b_linear_var == c_linear_var {
            is_quadratic = true;
        } else {
            return (false, (zero, zero, zero));
        }
    }

    if is_quadratic {
        // something like (v - 1) * (v - 1) = (v - 1)
        // and we can make a quadratic gate

        let mut quadratic_term = a_linear_var_coeff;
        quadratic_term.mul_assign(&b_linear_var_coeff);

        let mut linear_term_0 = a_constant_coeff;
        linear_term_0.mul_assign(&b_linear_var_coeff);

        let mut linear_term_1 = b_constant_coeff;
        linear_term_1.mul_assign(&a_linear_var_coeff);

        let mut linear_term = linear_term_0;
        linear_term.add_assign(&linear_term_1);
        if c_is_linear {
            linear_term.sub_assign(&c_linear_var_coeff);
        }

        let mut constant_term = a_constant_coeff;
        constant_term.mul_assign(&b_constant_coeff);

        if c_constant_coeff != zero {
            constant_term.sub_assign(&c_constant_coeff);
        }

        return (true, (constant_term, linear_term, quadratic_term));
    }

    (false, (zero, zero, zero))
}

fn check_for_quadratic_gate<E: Engine>(
    a: &LinearCombination<E>,
    b: &LinearCombination<E>,
    c: &LinearCombination<E>,
    a_constant_term: E::Fr,
    b_constant_term: E::Fr,
    c_constant_term: E::Fr,
) -> (bool, (E::Fr, E::Fr, E::Fr)) {
    let zero = E::Fr::zero();

    let a_is_linear = a.0.len() == 1;
    let b_is_linear = b.0.len() == 1;
    let c_is_linear = c.0.len() == 1;

    let c_is_empty = c.0.len() == 0;

    let mut a_linear_var_coeff = E::Fr::zero();
    let mut b_linear_var_coeff = E::Fr::zero();
    let mut c_linear_var_coeff = E::Fr::zero();

    let mut is_quadratic = false;
    if c_is_empty {
        if a_is_linear && b_is_linear {
            let (a_linear_var, a_coeff) = a.0[0];
            let (b_linear_var, b_coeff) = b.0[0];
            a_linear_var_coeff = a_coeff;
            b_linear_var_coeff = b_coeff;

            is_quadratic = a_linear_var == b_linear_var;
        }
    } else {
        if a_is_linear && b_is_linear && c_is_linear {
            let (a_linear_var, a_coeff) = a.0[0];
            let (b_linear_var, b_coeff) = b.0[0];
            let (c_linear_var, c_coeff) = c.0[0];

            a_linear_var_coeff = a_coeff;
            b_linear_var_coeff = b_coeff;
            c_linear_var_coeff = c_coeff;

            is_quadratic = a_linear_var == b_linear_var && b_linear_var == c_linear_var;
        }
    }

    if is_quadratic {
        // something like (v - 1) * (v - 1) = (v - 1)
        // and we can make a quadratic gate

        let mut quadratic_term = a_linear_var_coeff;
        quadratic_term.mul_assign(&b_linear_var_coeff);

        let mut linear_term_0 = a_constant_term;
        linear_term_0.mul_assign(&b_linear_var_coeff);

        let mut linear_term_1 = b_constant_term;
        linear_term_1.mul_assign(&a_linear_var_coeff);

        let mut linear_term = linear_term_0;
        linear_term.add_assign(&linear_term_1);
        if c_is_linear {
            linear_term.sub_assign(&c_linear_var_coeff);
        }

        let mut constant_term = a_constant_term;
        constant_term.mul_assign(&b_constant_term);

        if c_constant_term != zero {
            constant_term.sub_assign(&c_constant_term);
        }

        return (true, (constant_term, linear_term, quadratic_term));
    }

    (false, (zero, zero, zero))
}

fn is_constant<E: Engine, CS: ConstraintSystem<E>>(lc: &LinearCombination<E>) -> (bool, E::Fr) {
    // formally it's an empty LC, so it's a constant 0
    if lc.as_ref().len() == 0 {
        return (true, E::Fr::zero());
    }

    let result = get_constant_term::<E, CS>(&lc);

    if result.0 && lc.as_ref().len() == 1 {
        return result;
    }

    (false, E::Fr::zero())
}

fn get_constant_term<E: Engine, CS: ConstraintSystem<E>>(lc: &LinearCombination<E>) -> (bool, E::Fr) {
    let cs_one = CS::one();

    for (var, coeff) in lc.as_ref().iter() {
        if var == &cs_one {
            return (true, *coeff);
        }
    }

    (false, E::Fr::zero())
}

fn get_first_variable<E: Engine, CS: ConstraintSystem<E>>(lc: &LinearCombination<E>) -> (bool, Variable) {
    let cs_one = CS::one();

    for (var, _) in lc.as_ref().iter() {
        if var != &cs_one {
            return (true, *var);
        }
    }

    (false, cs_one)
}

fn get_first_variable_with_coeff<E: Engine, CS: ConstraintSystem<E>>(lc: &LinearCombination<E>) -> (bool, Variable, E::Fr) {
    let cs_one = CS::one();

    for (var, coeff) in lc.as_ref().iter() {
        if var != &cs_one {
            return (true, *var, *coeff);
        }
    }

    (false, cs_one, E::Fr::zero())
}

fn num_unique_values<E: Engine, CS: ConstraintSystem<E>>(lc: &LinearCombination<E>, scratch: &mut HashSet<crate::cs::Variable>) -> (bool, usize) {
    let cs_one = CS::one();

    debug_assert!(scratch.is_empty());

    let mut contains_constant = false;

    for (var, _) in lc.as_ref().iter() {
        if var != &cs_one {
            scratch.insert(*var);
        } else {
            contains_constant = true;
        }
    }

    let num_unique_without_constant = scratch.len();

    scratch.clear();

    (contains_constant, num_unique_without_constant)
}

fn is_linear_term<E: Engine, CS: ConstraintSystem<E>>(lc: &LinearCombination<E>, scratch: &mut HashSet<crate::cs::Variable>) -> (bool, Variable, E::Fr) {
    let cs_one = CS::one();

    debug_assert!(scratch.is_empty());

    let mut linear_coeff = E::Fr::zero();

    for (var, coeff) in lc.as_ref().iter() {
        if var != &cs_one {
            scratch.insert(*var);
            linear_coeff = *coeff;
        }
    }

    let num_unique_without_constant = scratch.len();

    if num_unique_without_constant == 1 {
        let terms: Vec<_> = scratch.drain().collect();
        let term = terms[0];

        return (true, term, linear_coeff);
    } else {
        scratch.clear();

        return (false, cs_one, E::Fr::zero());
    }
}

fn deduplicate_stable<E: Engine, CS: ConstraintSystem<E>>(lc: LinearCombination<E>, scratch: &mut HashMap<crate::cs::Variable, usize>) -> LinearCombination<E> {
    assert!(scratch.is_empty());

    if lc.as_ref().len() == 0 {
        return lc;
    }

    let mut deduped_vec: Vec<(crate::cs::Variable, E::Fr)> = Vec::with_capacity(lc.as_ref().len());

    for (var, coeff) in lc.0.into_iter() {
        if let Some(existing_index) = scratch.get(&var) {
            let (_, c) = &mut deduped_vec[*existing_index];
            c.add_assign(&coeff);
        } else {
            let new_idx = deduped_vec.len();
            deduped_vec.push((var, coeff));
            scratch.insert(var, new_idx);
        }
    }

    // let _initial_len = deduped_vec.len();

    deduped_vec = deduped_vec.into_iter().filter(|(_var, coeff)| !coeff.is_zero()).collect();

    // let _final_len = deduped_vec.len();

    // if _initial_len != _final_len {
    //     println!("Encountered constraint with zero coeff for variable!");
    // }

    // assert!(deduped_vec.len() != 0);

    scratch.clear();

    LinearCombination(deduped_vec)
}

fn deduplicate_and_split_linear_term<E: Engine, CS: ConstraintSystem<E>>(lc: LinearCombination<E>, scratch: &mut HashMap<crate::cs::Variable, usize>) -> (bool, E::Fr, bool, LinearCombination<E>) {
    assert!(scratch.is_empty());

    if lc.as_ref().len() == 0 {
        return (true, E::Fr::zero(), true, lc);
    }

    let cs_one = CS::one();
    let mut constant_term = E::Fr::zero();

    let mut deduped_vec: Vec<(crate::cs::Variable, E::Fr)> = Vec::with_capacity(lc.as_ref().len());

    for (var, coeff) in lc.0.into_iter() {
        if var != cs_one {
            if let Some(existing_index) = scratch.get(&var) {
                let (_, c) = &mut deduped_vec[*existing_index];
                c.add_assign(&coeff);
            } else {
                let new_idx = deduped_vec.len();
                deduped_vec.push((var, coeff));
                scratch.insert(var, new_idx);
            }
        } else {
            constant_term.add_assign(&coeff);
        }
    }

    deduped_vec = deduped_vec.into_iter().filter(|(_var, coeff)| !coeff.is_zero()).collect();

    scratch.clear();

    let has_constant = !constant_term.is_zero();
    let lc_is_empty = deduped_vec.len() == 0;

    (has_constant, constant_term, lc_is_empty, LinearCombination(deduped_vec))
}

fn subtract_lcs_with_dedup_stable<E: Engine, CS: ConstraintSystem<E>>(
    lc_0: LinearCombination<E>,
    lc_1: LinearCombination<E>,
    scratch: &mut HashMap<crate::cs::Variable, usize>,
) -> LinearCombination<E> {
    assert!(scratch.is_empty());

    if lc_0.as_ref().len() == 0 && lc_1.as_ref().len() == 0 {
        return lc_0;
    }

    let mut deduped_vec: Vec<(crate::cs::Variable, E::Fr)> = Vec::with_capacity(lc_0.as_ref().len() + lc_1.as_ref().len());

    for (var, coeff) in lc_0.0.into_iter() {
        if let Some(existing_index) = scratch.get(&var) {
            let (_, c) = &mut deduped_vec[*existing_index];
            c.add_assign(&coeff);
        } else {
            let new_idx = deduped_vec.len();
            deduped_vec.push((var, coeff));
            scratch.insert(var, new_idx);
        }
    }

    for (var, coeff) in lc_1.0.into_iter() {
        if let Some(existing_index) = scratch.get(&var) {
            let (_, c) = &mut deduped_vec[*existing_index];
            c.sub_assign(&coeff);
        } else {
            let new_idx = deduped_vec.len();
            let mut coeff_negated = coeff;
            coeff_negated.negate();
            deduped_vec.push((var, coeff_negated));
            scratch.insert(var, new_idx);
        }
    }

    deduped_vec = deduped_vec.into_iter().filter(|(_var, coeff)| !coeff.is_zero()).collect();

    scratch.clear();

    LinearCombination(deduped_vec)
}

fn subtract_variable_unchecked<E: Engine>(lc: &mut LinearCombination<E>, variable: Variable) {
    let mut minus_one = E::Fr::one();
    minus_one.negate();

    lc.0.push((variable, minus_one));
}

fn split_constant_term<E: Engine, CS: ConstraintSystem<E>>(mut lc: LinearCombination<E>) -> (LinearCombination<E>, E::Fr) {
    if lc.as_ref().len() == 0 {
        return (lc, E::Fr::zero());
    }

    let mut idx = None;
    let cs_one = CS::one();
    let mut constant_coeff = E::Fr::zero();

    for (i, (var, coeff)) in lc.0.iter().enumerate() {
        if var == &cs_one {
            idx = Some(i);
            constant_coeff = *coeff;
            break;
        }
    }

    if let Some(idx) = idx {
        let _ = lc.0.swap_remove(idx);
        return (lc, constant_coeff);
    } else {
        return (lc, constant_coeff);
    }
}

pub struct Adaptor<'a, E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P> + 'a> {
    cs: &'a mut CS,
    hints: &'a Vec<(usize, TranspilationVariant)>,
    current_constraint_index: usize,
    current_hint_index: usize,
    scratch: HashSet<crate::cs::Variable>,
    // deduplication_scratch: HashMap<crate::cs::Variable, E::Fr>,
    deduplication_scratch: HashMap<crate::cs::Variable, usize>,
    transpilation_scratch_space: Option<TranspilationScratchSpace<E>>,
    _marker_e: std::marker::PhantomData<E>,
    _marker_p: std::marker::PhantomData<P>,
}

impl<'a, E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P> + 'a> Adaptor<'a, E, P, CS> {
    fn get_next_hint(&mut self) -> (usize, TranspilationVariant) {
        let current_hint_index = self.current_hint_index;
        let expected_constraint_index = self.current_constraint_index;

        let next_hint = self.hints[current_hint_index].clone();

        assert!(next_hint.0 == expected_constraint_index);

        self.current_hint_index += 1;
        self.current_constraint_index += 1;

        next_hint
    }
}

impl<'a, E: Engine, P: PlonkConstraintSystemParams<E>, CS: PlonkConstraintSystem<E, P> + 'a> crate::ConstraintSystem<E> for Adaptor<'a, E, P, CS> {
    type Root = Self;

    fn one() -> crate::Variable {
        crate::Variable::new_unchecked(crate::Index::Input(0))
    }

    fn alloc<F, A, AR>(&mut self, _: A, f: F) -> Result<crate::Variable, crate::SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, crate::SynthesisError>,
        A: FnOnce() -> AR,
        AR: Into<String>,
    {
        let var = self.cs.alloc(|| f().map_err(|_| crate::SynthesisError::AssignmentMissing))?;

        Ok(match var {
            PlonkVariable(PlonkIndex::Aux(index)) => crate::Variable::new_unchecked(crate::Index::Aux(index)),
            _ => unreachable!("Map aux into aux"),
        })
    }

    fn alloc_input<F, A, AR>(&mut self, _: A, f: F) -> Result<crate::Variable, crate::SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, crate::SynthesisError>,
        A: FnOnce() -> AR,
        AR: Into<String>,
    {
        let var = self.cs.alloc_input(|| f().map_err(|_| crate::SynthesisError::AssignmentMissing))?;

        Ok(match var {
            PlonkVariable(PlonkIndex::Input(index)) => crate::Variable::new_unchecked(crate::Index::Input(index)),
            _ => unreachable!("Map input into input"),
        })
    }

    fn enforce<A, AR, LA, LB, LC>(&mut self, _ann: A, a: LA, b: LB, c: LC)
    where
        A: FnOnce() -> AR,
        AR: Into<String>,
        LA: FnOnce(crate::LinearCombination<E>) -> crate::LinearCombination<E>,
        LB: FnOnce(crate::LinearCombination<E>) -> crate::LinearCombination<E>,
        LC: FnOnce(crate::LinearCombination<E>) -> crate::LinearCombination<E>,
    {
        let zero_fr = E::Fr::zero();
        let one_fr = E::Fr::one();
        let mut minus_one_fr = E::Fr::one();
        minus_one_fr.negate();

        let (_, hint) = { self.get_next_hint() };

        let _hint = hint.clone();

        // we need to determine the type of transformation constraint

        // let's handle trivial cases first

        // A or B or C are just constant terms

        let (a_has_constant, a_constant_term, a_lc_is_empty, a_lc) = deduplicate_and_split_linear_term::<E, Self>(a(crate::LinearCombination::zero()), &mut self.deduplication_scratch);
        let (b_has_constant, b_constant_term, b_lc_is_empty, b_lc) = deduplicate_and_split_linear_term::<E, Self>(b(crate::LinearCombination::zero()), &mut self.deduplication_scratch);
        let (c_has_constant, c_constant_term, c_lc_is_empty, c_lc) = deduplicate_and_split_linear_term::<E, Self>(c(crate::LinearCombination::zero()), &mut self.deduplication_scratch);

        let a_is_constant = a_has_constant & a_lc_is_empty;
        let b_is_constant = b_has_constant & b_lc_is_empty;
        let c_is_constant = c_has_constant & c_lc_is_empty;

        let dummy_var = self.cs.get_dummy_variable();

        // variables are left, right, output
        // coefficients are left, right, output, multiplication, constant

        let mut space = self.transpilation_scratch_space.take().unwrap();

        match hint {
            TranspilationVariant::IntoQuadraticGate => {
                let var = if !a_lc_is_empty {
                    convert_variable(a_lc.0[0].0)
                } else if !b_lc_is_empty {
                    convert_variable(b_lc.0[0].0)
                } else if !c_lc_is_empty {
                    convert_variable(c_lc.0[0].0)
                } else {
                    unreachable!();
                };

                let (is_quadratic, coeffs) = check_for_quadratic_gate(&a_lc, &b_lc, &c_lc, a_constant_term, b_constant_term, c_constant_term);

                debug_assert!(is_quadratic);

                let (c0, c1, c2) = coeffs;

                space.scratch_space_for_coeffs.resize(P::STATE_WIDTH, zero_fr);
                space.scratch_space_for_vars.resize(P::STATE_WIDTH, dummy_var);

                space.scratch_space_for_coeffs[0] = c1;
                space.scratch_space_for_coeffs.push(c2);
                space.scratch_space_for_coeffs.push(c0);
                space.scratch_space_for_coeffs.push(zero_fr);

                // ~ A*A + A + const == A*(B=A) + A + const
                space.scratch_space_for_vars[0] = var;
                space.scratch_space_for_vars[1] = var;

                allocate_into_cs(self.cs, false, &*space.scratch_space_for_vars, &*space.scratch_space_for_coeffs).expect("must make a quadratic gate");
            }
            TranspilationVariant::IntoMultiplicationGate(hints) => {
                let (t_a, t_b, t_c) = hints;
                let mut q_m = one_fr;
                let mut q_c = one_fr;
                let a_var = match t_a {
                    hint @ LcTransformationVariant::IntoSingleGate | hint @ LcTransformationVariant::IntoMultipleGates => {
                        let (new_a_var, a_coeff, _variant) =
                            enforce_lc_as_gates(self.cs, a_lc, one_fr, a_constant_term, true, &mut space).expect("must allocate A variable to transpile A LC for multiplication gate");

                        assert!(a_coeff == one_fr);

                        assert!(_variant == hint);

                        new_a_var.expect("transpiler must create a new variable for LC A")
                    }
                    LcTransformationVariant::IsSingleVariable => {
                        assert!(!a_lc_is_empty);
                        let (var, coeff) = a_lc.0[0];
                        q_m.mul_assign(&coeff); // collapse coeff before A*B

                        convert_variable(var)
                    }
                    _ => {
                        unreachable!("{:?}", t_a)
                    }
                };

                let b_var = match t_b {
                    hint @ LcTransformationVariant::IntoSingleGate | hint @ LcTransformationVariant::IntoMultipleGates => {
                        let (new_b_var, b_coeff, _variant) =
                            enforce_lc_as_gates(self.cs, b_lc, one_fr, b_constant_term, true, &mut space).expect("must allocate B variable to transpile B LC for multiplication gate");

                        assert!(b_coeff == one_fr);

                        assert!(_variant == hint);

                        new_b_var.expect("transpiler must create a new variable for LC B")
                    }
                    LcTransformationVariant::IsSingleVariable => {
                        assert!(!b_lc_is_empty);
                        let (var, coeff) = b_lc.0[0];
                        q_m.mul_assign(&coeff); // collapse coeffs before A*B

                        convert_variable(var)
                    }
                    _ => {
                        unreachable!("{:?}", t_b)
                    }
                };

                let (c_is_just_a_constant, c_var) = match t_c {
                    hint @ LcTransformationVariant::IntoSingleGate | hint @ LcTransformationVariant::IntoMultipleGates => {
                        let (new_c_var, c_coeff, _variant) =
                            enforce_lc_as_gates(self.cs, c_lc, one_fr, c_constant_term, true, &mut space).expect("must allocate C variable to transpile C LC for multiplication gate");

                        assert!(c_coeff == one_fr);

                        assert!(_variant == hint);

                        (false, Some(new_c_var.expect("transpiler must create a new variable for LC C")))
                    }
                    LcTransformationVariant::IsSingleVariable => {
                        assert!(!c_lc_is_empty);
                        let (var, coeff) = c_lc.0[0];
                        q_c = coeff;

                        (false, Some(convert_variable(var)))
                    }
                    LcTransformationVariant::IsConstant => {
                        assert!(c_lc_is_empty);
                        assert!(c_has_constant);

                        (true, None)
                    }
                };

                if c_is_just_a_constant {
                    let mut constant_term = c_constant_term;
                    constant_term.negate(); // - C

                    space.scratch_space_for_coeffs.resize(P::STATE_WIDTH, zero_fr);
                    space.scratch_space_for_vars.resize(P::STATE_WIDTH, dummy_var);

                    space.scratch_space_for_coeffs.push(q_m);
                    space.scratch_space_for_coeffs.push(constant_term);
                    space.scratch_space_for_coeffs.push(zero_fr);

                    // ~ A*B + const == 0
                    space.scratch_space_for_vars[0] = a_var;
                    space.scratch_space_for_vars[1] = b_var;

                    allocate_into_cs(self.cs, false, &*space.scratch_space_for_vars, &*space.scratch_space_for_coeffs).expect("must make a multiplication gate with C being constant");
                } else {
                    // Plain multiplication gate

                    q_c.negate(); // - C

                    let c_var = c_var.expect("C must be a variable");

                    space.scratch_space_for_coeffs.resize(P::STATE_WIDTH, zero_fr);
                    space.scratch_space_for_vars.resize(P::STATE_WIDTH, dummy_var);

                    space.scratch_space_for_coeffs[2] = q_c;
                    space.scratch_space_for_coeffs.push(q_m);
                    space.scratch_space_for_coeffs.push(zero_fr);
                    space.scratch_space_for_coeffs.push(zero_fr);

                    // ~ A*B - C == 0
                    space.scratch_space_for_vars[0] = a_var;
                    space.scratch_space_for_vars[1] = b_var;
                    space.scratch_space_for_vars[2] = c_var;

                    allocate_into_cs(self.cs, false, &*space.scratch_space_for_vars, &*space.scratch_space_for_coeffs).expect("must make a plain multiplication gate");
                }
            }
            // make an addition gate
            TranspilationVariant::IntoAdditionGate(hint) => {
                // these are simple enforcements that are not a part of multiplication gate
                // or merge of LCs

                if c_is_constant {
                    let lc = if !a_is_constant {
                        a_lc
                    } else if !b_is_constant {
                        b_lc
                    } else {
                        unreachable!("Either A or B LCs are constant");
                    };

                    let multiplier = if a_is_constant {
                        a_constant_term
                    } else if b_is_constant {
                        b_constant_term
                    } else {
                        unreachable!("Must take multiplier from A or B");
                    };

                    let mut free_constant_term = if a_is_constant {
                        b_constant_term
                    } else if b_is_constant {
                        a_constant_term
                    } else {
                        unreachable!("Either A or B LCs are constant");
                    };

                    free_constant_term.mul_assign(&multiplier);
                    free_constant_term.sub_assign(&c_constant_term);

                    let (_, _, _variant) = enforce_lc_as_gates(self.cs, lc, multiplier, free_constant_term, false, &mut space).expect("must allocate variable to transpile LC == 0 gate");

                    assert!(hint == _variant);
                } else {
                    // c is not a constant and it's handled by MergeLCs
                    unreachable!();
                }
            }
            TranspilationVariant::MergeLinearCombinations(merge_variant, merge_hint) => {
                let multiplier = if a_is_constant {
                    a_constant_term
                } else if b_is_constant {
                    b_constant_term
                } else {
                    unreachable!()
                };

                let mut free_constant_term;

                let lc_into_rewriting = match merge_variant {
                    MergeLcVariant::MergeACThroughConstantB => {
                        assert!(b_is_constant);
                        let mut final_lc = a_lc;
                        free_constant_term = a_constant_term;
                        if multiplier != one_fr {
                            for (_, c) in final_lc.0.iter_mut() {
                                c.mul_assign(&multiplier);
                            }
                            free_constant_term.mul_assign(&multiplier);
                        }

                        free_constant_term.sub_assign(&c_constant_term);

                        subtract_lcs_with_dedup_stable::<E, Self>(final_lc, c_lc, &mut self.deduplication_scratch)
                        // final_lc - &c
                    }
                    MergeLcVariant::MergeBCThroughConstantA => {
                        assert!(a_is_constant);
                        let mut final_lc = b_lc;
                        free_constant_term = b_constant_term;
                        if multiplier != one_fr {
                            for (_, c) in final_lc.0.iter_mut() {
                                c.mul_assign(&multiplier);
                            }
                            free_constant_term.mul_assign(&multiplier);
                        }

                        free_constant_term.sub_assign(&c_constant_term);

                        subtract_lcs_with_dedup_stable::<E, Self>(final_lc, c_lc, &mut self.deduplication_scratch)
                        // final_lc - &c
                    }
                    MergeLcVariant::CIsTheOnlyMeaningful => {
                        free_constant_term = a_constant_term;
                        free_constant_term.mul_assign(&b_constant_term);
                        free_constant_term.negate();
                        free_constant_term.add_assign(&c_constant_term);

                        c_lc
                    }
                    _ => {
                        unreachable!()
                    }
                };

                let h = merge_hint;

                match h {
                    hint @ LcTransformationVariant::IntoSingleGate | hint @ LcTransformationVariant::IntoMultipleGates => {
                        let (new_c_var, _coeff, _variant) =
                            enforce_lc_as_gates(self.cs, lc_into_rewriting, one_fr, free_constant_term, false, &mut space).expect("must allocate gates to transpile merging of LCs");

                        assert!(_coeff == one_fr);

                        assert!(new_c_var.is_none());
                        assert!(_variant == hint);
                    }
                    _ => {
                        unreachable!("{:?}", h);
                    }
                };
            }
        }

        space.clear();
        self.transpilation_scratch_space = Some(space);
    }

    fn push_namespace<NR, N>(&mut self, _: N)
    where
        NR: Into<String>,
        N: FnOnce() -> NR,
    {
        // Do nothing; we don't care about namespaces in this context.
    }

    fn pop_namespace(&mut self) {
        // Do nothing; we don't care about namespaces in this context.
    }

    fn get_root(&mut self) -> &mut Self::Root {
        self
    }
}

fn convert_variable(r1cs_variable: crate::Variable) -> PlonkVariable {
    let var = match r1cs_variable.get_unchecked() {
        crate::Index::Input(0) => {
            unreachable!("can not convert input variable number 0 (CS::one)")
        }
        crate::Index::Aux(0) => {
            unreachable!("can not convert aux variable labeled as 0 (taken by Plonk CS)")
        }
        crate::Index::Input(i) => PlonkVariable(PlonkIndex::Input(i)),
        crate::Index::Aux(i) => PlonkVariable(PlonkIndex::Aux(i)),
    };

    var
}

fn convert_variable_back(plonk_variable: PlonkVariable) -> crate::Variable {
    let var = match plonk_variable.get_unchecked() {
        crate::plonk::cs::variable::Index::Input(0) => {
            unreachable!("can not convert input variable number 0 (does not exist in plonk)")
        }
        crate::plonk::cs::variable::Index::Aux(0) => {
            unreachable!("can not convert aux variable labeled as 0 (does not exist in plonk, dummy gate)")
        }
        crate::plonk::cs::variable::Index::Input(i) => crate::Variable(crate::Index::Input(i)),
        crate::plonk::cs::variable::Index::Aux(i) => crate::Variable(crate::Index::Aux(i)),
    };

    var
}

use std::cell::Cell;

pub struct AdaptorCircuit<'a, E: Engine, P: PlonkConstraintSystemParams<E>, C: crate::Circuit<E>> {
    circuit: Cell<Option<C>>,
    hints: &'a Vec<(usize, TranspilationVariant)>,
    _marker_e: std::marker::PhantomData<E>,
    _marker_p: std::marker::PhantomData<P>,
}

impl<'a, E: Engine, P: PlonkConstraintSystemParams<E>, C: crate::Circuit<E>> AdaptorCircuit<'a, E, P, C> {
    pub fn new<'b>(circuit: C, hints: &'b Vec<(usize, TranspilationVariant)>) -> Self
    where
        'b: 'a,
    {
        Self {
            circuit: Cell::new(Some(circuit)),
            hints: hints,
            _marker_e: std::marker::PhantomData,
            _marker_p: std::marker::PhantomData,
        }
    }
}

impl<'a, E: Engine, P: PlonkConstraintSystemParams<E>, C: crate::Circuit<E>> PlonkCircuit<E, P> for AdaptorCircuit<'a, E, P, C> {
    fn synthesize<CS: PlonkConstraintSystem<E, P>>(&self, cs: &mut CS) -> Result<(), SynthesisError> {
        let mut adaptor = Adaptor::<E, P, CS> {
            cs: cs,
            hints: self.hints,
            current_constraint_index: 0,
            current_hint_index: 0,
            scratch: HashSet::with_capacity((E::Fr::NUM_BITS * 2) as usize),
            deduplication_scratch: HashMap::with_capacity((E::Fr::NUM_BITS * 2) as usize),
            transpilation_scratch_space: Some(TranspilationScratchSpace::new(P::STATE_WIDTH * 2)),
            _marker_e: std::marker::PhantomData,
            _marker_p: std::marker::PhantomData,
        };

        let c = self.circuit.replace(None).expect("Must replace a circuit out from cell");

        c.synthesize(&mut adaptor)
    }
}

#[test]
#[ignore] // TODO(ignored-test): Failure.
fn transpile_xor_using_new_adaptor() {
    use super::cs::PlonkCsWidth4WithNextStepParams;
    use super::generator::*;
    use super::keys::*;
    use super::prover::*;
    use super::test_assembly::*;
    use super::utils::make_non_residues;
    use super::verifier::*;
    use crate::cs::Circuit;
    use crate::kate_commitment::*;
    use crate::pairing::bn256::{Bn256, Fr};
    use crate::plonk::commitments::transcript::keccak_transcript::*;
    use crate::plonk::commitments::transcript::*;
    use crate::plonk::domains::Domain;
    use crate::plonk::fft::cooley_tukey_ntt::*;
    use crate::tests::XORDemo;
    use crate::worker::Worker;

    let c = XORDemo::<Bn256> {
        a: None,
        b: None,
        _marker: PhantomData,
    };

    let mut transpiler = Transpiler::<Bn256, PlonkCsWidth4WithNextStepParams>::new();

    c.synthesize(&mut transpiler).expect("sythesize into traspilation must succeed");

    let hints = transpiler.hints;

    for (constraint_id, hint) in hints.iter() {
        println!("Constraint {} into {:?}", constraint_id, hint);
    }

    // let c = XORDemo::<Bn256> {
    //     a: None,
    //     b: None,
    //     _marker: PhantomData
    // };

    let c = XORDemo::<Bn256> {
        a: Some(true),
        b: Some(false),
        _marker: PhantomData,
    };

    let adapted_curcuit = AdaptorCircuit::<Bn256, PlonkCsWidth4WithNextStepParams, _>::new(c.clone(), &hints);

    let mut assembly = TestAssembly::<Bn256, PlonkCsWidth4WithNextStepParams>::new();
    adapted_curcuit.synthesize(&mut assembly).expect("sythesize of transpiled into CS must succeed");
    let num_gates = assembly.num_gates();
    println!("Transpiled into {} gates", num_gates);

    let adapted_curcuit = AdaptorCircuit::<Bn256, PlonkCsWidth4WithNextStepParams, _>::new(c.clone(), &hints);
    let mut assembly = GeneratorAssembly4WithNextStep::<Bn256>::new();
    adapted_curcuit.synthesize(&mut assembly).expect("sythesize of transpiled into CS must succeed");
    assembly.finalize();

    let worker = Worker::new();

    let setup = assembly.setup(&worker).unwrap();

    let crs_mons = Crs::<Bn256, CrsForMonomialForm>::crs_42(setup.permutation_polynomials[0].size(), &worker);
    let crs_vals = Crs::<Bn256, CrsForLagrangeForm>::crs_42(setup.permutation_polynomials[0].size(), &worker);

    let verification_key = VerificationKey::from_setup(&setup, &worker, &crs_mons).unwrap();

    let precomputations = SetupPolynomialsPrecomputations::from_setup(&setup, &worker).unwrap();

    let mut assembly = ProverAssembly4WithNextStep::<Bn256>::new();

    let adapted_curcuit = AdaptorCircuit::<Bn256, PlonkCsWidth4WithNextStepParams, _>::new(c.clone(), &hints);

    adapted_curcuit.synthesize(&mut assembly).expect("must work");

    assembly.finalize();

    let size = setup.permutation_polynomials[0].size();

    let domain = Domain::<Fr>::new_for_size(size as u64).unwrap();
    let non_residues = make_non_residues::<Fr>(3);
    println!("Non residues = {:?}", non_residues);

    type Transcr = RollingKeccakTranscript<Fr>;

    let omegas_bitreversed = BitReversedOmegas::<Fr>::new_for_domain_size(size.next_power_of_two());
    let omegas_inv_bitreversed = <OmegasInvBitreversed<Fr> as CTPrecomputations<Fr>>::new_for_domain_size(size.next_power_of_two());

    let proof = assembly
        .prove::<Transcr, _, _>(&worker, &setup, &precomputations, &crs_vals, &crs_mons, &omegas_bitreversed, &omegas_inv_bitreversed, None)
        .unwrap();

    let is_valid = verify::<Bn256, PlonkCsWidth4WithNextStepParams, Transcr>(&proof, &verification_key, None).unwrap();

    assert!(is_valid);

    // println!("Verification key = {:?}", verification_key);
    // println!("Proof = {:?}", proof);

    let mut key_writer = std::io::BufWriter::with_capacity(1 << 24, std::fs::File::create("./xor_vk.key").unwrap());
    verification_key.write(&mut key_writer).unwrap();

    let mut proof_writer = std::io::BufWriter::with_capacity(1 << 24, std::fs::File::create("./xor_proof.proof").unwrap());
    proof.write(&mut proof_writer).unwrap();
}

#[test]
#[ignore] // TODO(ignored-test): Failure.
fn transpile_xor_and_prove_with_no_precomputations() {
    use super::cs::PlonkCsWidth4WithNextStepParams;
    use super::generator::*;
    use super::keys::*;
    use super::prover::*;
    use super::test_assembly::*;
    use super::utils::make_non_residues;
    use super::verifier::*;
    use crate::cs::Circuit;
    use crate::kate_commitment::*;
    use crate::pairing::bn256::{Bn256, Fr};
    use crate::plonk::commitments::transcript::keccak_transcript::*;
    use crate::plonk::commitments::transcript::*;
    use crate::plonk::domains::Domain;
    use crate::plonk::fft::cooley_tukey_ntt::*;
    use crate::tests::XORDemo;
    use crate::worker::Worker;

    let c = XORDemo::<Bn256> {
        a: None,
        b: None,
        _marker: PhantomData,
    };

    let mut transpiler = Transpiler::<Bn256, PlonkCsWidth4WithNextStepParams>::new();

    c.synthesize(&mut transpiler).expect("sythesize into traspilation must succeed");

    let hints = transpiler.hints;

    for (constraint_id, hint) in hints.iter() {
        println!("Constraint {} into {:?}", constraint_id, hint);
    }

    // let c = XORDemo::<Bn256> {
    //     a: None,
    //     b: None,
    //     _marker: PhantomData
    // };

    let c = XORDemo::<Bn256> {
        a: Some(true),
        b: Some(false),
        _marker: PhantomData,
    };

    let adapted_curcuit = AdaptorCircuit::<Bn256, PlonkCsWidth4WithNextStepParams, _>::new(c.clone(), &hints);

    let mut assembly = TestAssembly::<Bn256, PlonkCsWidth4WithNextStepParams>::new();
    adapted_curcuit.synthesize(&mut assembly).expect("sythesize of transpiled into CS must succeed");
    let num_gates = assembly.num_gates();
    println!("Transpiled into {} gates", num_gates);

    let adapted_curcuit = AdaptorCircuit::<Bn256, PlonkCsWidth4WithNextStepParams, _>::new(c.clone(), &hints);
    let mut assembly = GeneratorAssembly4WithNextStep::<Bn256>::new();
    adapted_curcuit.synthesize(&mut assembly).expect("sythesize of transpiled into CS must succeed");
    assembly.finalize();

    let worker = Worker::new();

    let setup = assembly.setup(&worker).unwrap();

    let crs_mons = Crs::<Bn256, CrsForMonomialForm>::crs_42(setup.permutation_polynomials[0].size(), &worker);

    let verification_key = VerificationKey::from_setup(&setup, &worker, &crs_mons).unwrap();

    let size = setup.permutation_polynomials[0].size();

    let domain = Domain::<Fr>::new_for_size(size as u64).unwrap();
    let non_residues = make_non_residues::<Fr>(3);
    println!("Non residues = {:?}", non_residues);

    type Transcr = RollingKeccakTranscript<Fr>;

    let proof = super::super::prove_by_steps::<_, _, Transcr>(c, &hints, &setup, None, &crs_mons, None).unwrap();

    let is_valid = verify::<Bn256, PlonkCsWidth4WithNextStepParams, Transcr>(&proof, &verification_key, None).unwrap();

    assert!(is_valid);

    // println!("Verification key = {:?}", verification_key);
    // println!("Proof = {:?}", proof);

    let mut key_writer = std::io::BufWriter::with_capacity(1 << 24, std::fs::File::create("./xor_vk.key").unwrap());
    verification_key.write(&mut key_writer).unwrap();

    let mut proof_writer = std::io::BufWriter::with_capacity(1 << 24, std::fs::File::create("./xor_proof.proof").unwrap());
    proof.write(&mut proof_writer).unwrap();
}
