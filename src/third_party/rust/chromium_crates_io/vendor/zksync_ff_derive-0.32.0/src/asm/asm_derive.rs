use num_bigint::BigUint;
use num_integer::Integer;
use num_traits::{One, ToPrimitive};
use quote::TokenStreamExt;
use std::str::FromStr;

use super::super::{fetch_attr, fetch_wrapped_ident, get_temp, get_temp_with_literal};
use crate::asm::impls_4::*;
use crate::utils::*;

const MODULUS_PREFIX: &str = "MODULUS_";
const MODULUS_NEGATED_PREFIX: &str = "MODULUS_NEG_";

// #[proc_macro_derive(PrimeFieldAsm, attributes(PrimeFieldModulus, PrimeFieldGenerator, UseADX))]
pub fn prime_field_asm_impl(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    // Parse the type definition
    let ast: syn::DeriveInput = syn::parse(input).unwrap();

    // The struct we're deriving for is a wrapper around a "Repr" type we must construct.
    let repr_ident = fetch_wrapped_ident(&ast.data).expect("PrimeField derive only operates over tuple structs of a single item");

    // We're given the modulus p of the prime field
    let modulus: BigUint = fetch_attr("PrimeFieldModulus", &ast.attrs)
        .expect("Please supply a PrimeFieldModulus attribute")
        .parse()
        .expect("PrimeFieldModulus should be a number");

    // We may be provided with a generator of p - 1 order. It is required that this generator be quadratic
    // nonresidue.
    let generator: BigUint = fetch_attr("PrimeFieldGenerator", &ast.attrs)
        .expect("Please supply a PrimeFieldGenerator attribute")
        .parse()
        .expect("PrimeFieldGenerator should be a number");

    // User may opt-in for feature to generate CIOS based multiplication operation
    let use_adx: Option<bool> = fetch_attr("UseADX", &ast.attrs).map(|el| el.parse().expect("UseADX should be `true` or `false`"));

    assert!(use_adx.unwrap(), "For now only ADX backend is used");

    // The arithmetic in this library only works if the modulus*2 is smaller than the backing
    // representation. Compute the number of limbs we need.
    let mut limbs = 1;
    {
        let mod2 = (&modulus) << 1; // modulus * 2
        let mut cur = BigUint::one() << 64; // always 64-bit limbs for now
        while cur < mod2 {
            limbs += 1;
            cur = cur << 64;
        }
    }

    assert_eq!(limbs, 4, "can only derive for 4 limb fitting modulus");

    let modulus_limbs = biguint_to_real_u64_vec(modulus.clone(), limbs);
    let top_limb = modulus_limbs.last().unwrap().clone().to_u64().unwrap();
    let can_use_optimistic_cios_mul = {
        let mut can_use = true;
        if top_limb == 0 {
            can_use = false;
        }

        if top_limb > (std::u64::MAX / 2) - 1 {
            can_use = false;
        }
        can_use
    };

    let can_use_optimistic_cios_sqr = {
        let mut can_use = true;
        if top_limb == 0 {
            can_use = false;
        }

        if top_limb > (std::u64::MAX / 4) - 1 {
            assert!(!can_use, "can not use optimistic CIOS for this modulus");
            can_use = false;
        }
        can_use
    };

    assert!(can_use_optimistic_cios_mul, "Can only derive for moduluses that fit in 255 bits - epsilon");
    assert!(can_use_optimistic_cios_sqr, "Can only derive for moduluses that fit in 254 bits - epsilon");

    let random_id = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().subsec_nanos();

    let mut gen = proc_macro2::TokenStream::new();

    let (constants_impl, mont_inv, sqrt_impl) = prime_field_constants_with_inv_and_sqrt(&ast.ident, &repr_ident, modulus, limbs, generator, random_id);

    gen.extend(constants_impl);
    gen.extend(prime_field_repr_impl(&repr_ident, limbs));
    gen.extend(prime_field_impl(&ast.ident, &repr_ident, mont_inv, limbs, random_id));
    gen.extend(sqrt_impl);

    // Return the generated impl
    gen.into()
}

// Implement PrimeFieldRepr for the wrapped ident `repr` with `limbs` limbs.
fn prime_field_repr_impl(repr: &syn::Ident, limbs: usize) -> proc_macro2::TokenStream {
    quote! {

        #[derive(Copy, Clone, PartialEq, Eq, Default, ::serde::Serialize, ::serde::Deserialize)]
        pub struct #repr(
            pub [u64; #limbs]
        );

        impl ::std::fmt::Debug for #repr
        {
            fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(f, "0x")?;
                for i in self.0.iter().rev() {
                    write!(f, "{:016x}", *i)?;
                }

                Ok(())
            }
        }

        impl ::rand::Rand for #repr {
            #[inline(always)]
            fn rand<R: ::rand::Rng>(rng: &mut R) -> Self {
                #repr(rng.gen())
            }
        }

        impl ::std::fmt::Display for #repr {
            fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(f, "0x")?;
                for i in self.0.iter().rev() {
                    write!(f, "{:016x}", *i)?;
                }

                Ok(())
            }
        }

        impl std::hash::Hash for #repr {
            fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
                for limb in self.0.iter() {
                    limb.hash(state);
                }
            }
        }

        impl AsRef<[u64]> for #repr {
            #[inline(always)]
            fn as_ref(&self) -> &[u64] {
                &self.0
            }
        }

        impl AsMut<[u64]> for #repr {
            #[inline(always)]
            fn as_mut(&mut self) -> &mut [u64] {
                &mut self.0
            }
        }

        impl From<u64> for #repr {
            #[inline(always)]
            fn from(val: u64) -> #repr {
                use std::default::Default;

                let mut repr = Self::default();
                repr.0[0] = val;
                repr
            }
        }

        impl Ord for #repr {
            #[inline(always)]
            fn cmp(&self, other: &#repr) -> ::std::cmp::Ordering {
                for (a, b) in self.0.iter().rev().zip(other.0.iter().rev()) {
                    if a < b {
                        return ::std::cmp::Ordering::Less
                    } else if a > b {
                        return ::std::cmp::Ordering::Greater
                    }
                }

                ::std::cmp::Ordering::Equal
            }
        }

        impl PartialOrd for #repr {
            #[inline(always)]
            fn partial_cmp(&self, other: &#repr) -> Option<::std::cmp::Ordering> {
                Some(self.cmp(other))
            }
        }

        impl crate::ff::PrimeFieldRepr for #repr {
            #[inline(always)]
            fn is_odd(&self) -> bool {
                self.0[0] & 1 == 1
            }

            #[inline(always)]
            fn is_even(&self) -> bool {
                !self.is_odd()
            }

            #[inline(always)]
            fn is_zero(&self) -> bool {
                self.0.iter().all(|&e| e == 0)
            }

            #[inline(always)]
            fn shr(&mut self, mut n: u32) {
                if n as usize >= 64 * #limbs {
                    *self = Self::from(0);
                    return;
                }

                while n >= 64 {
                    let mut t = 0;
                    for i in self.0.iter_mut().rev() {
                        ::std::mem::swap(&mut t, i);
                    }
                    n -= 64;
                }

                if n > 0 {
                    let mut t = 0;
                    for i in self.0.iter_mut().rev() {
                        let t2 = *i << (64 - n);
                        *i >>= n;
                        *i |= t;
                        t = t2;
                    }
                }
            }

            #[inline(always)]
            fn div2(&mut self) {
                let mut t = 0;
                for i in self.0.iter_mut().rev() {
                    let t2 = *i << 63;
                    *i >>= 1;
                    *i |= t;
                    t = t2;
                }
            }

            #[inline(always)]
            fn mul2(&mut self) {
                let mut last = 0;
                for i in &mut self.0 {
                    let tmp = *i >> 63;
                    *i <<= 1;
                    *i |= last;
                    last = tmp;
                }
            }

            #[inline(always)]
            fn shl(&mut self, mut n: u32) {
                if n as usize >= 64 * #limbs {
                    *self = Self::from(0);
                    return;
                }

                while n >= 64 {
                    let mut t = 0;
                    for i in &mut self.0 {
                        ::std::mem::swap(&mut t, i);
                    }
                    n -= 64;
                }

                if n > 0 {
                    let mut t = 0;
                    for i in &mut self.0 {
                        let t2 = *i >> (64 - n);
                        *i <<= n;
                        *i |= t;
                        t = t2;
                    }
                }
            }

            #[inline(always)]
            fn num_bits(&self) -> u32 {
                let mut ret = (#limbs as u32) * 64;
                for i in self.0.iter().rev() {
                    let leading = i.leading_zeros();
                    ret -= leading;
                    if leading != 64 {
                        break;
                    }
                }

                ret
            }

            #[inline(always)]
            fn add_nocarry(&mut self, other: &#repr) {
                let mut carry = 0;

                for (a, b) in self.0.iter_mut().zip(other.0.iter()) {
                    *a = crate::ff::adc(*a, *b, &mut carry);
                }
            }

            #[inline(always)]
            fn sub_noborrow(&mut self, other: &#repr) {
                let mut borrow = 0;

                for (a, b) in self.0.iter_mut().zip(other.0.iter()) {
                    *a = crate::ff::sbb(*a, *b, &mut borrow);
                }
            }
        }
    }
}

fn prime_field_constants_with_inv_and_sqrt(
    name: &syn::Ident,
    repr: &syn::Ident,
    modulus: BigUint,
    limbs: usize,
    generator: BigUint,
    random_id: u32,
) -> (proc_macro2::TokenStream, u64, proc_macro2::TokenStream) {
    let modulus_num_bits = biguint_num_bits(modulus.clone());

    // The number of bits we should "shave" from a randomly sampled reputation, i.e.,
    // if our modulus is 381 bits and our representation is 384 bits, we should shave
    // 3 bits from the beginning of a randomly sampled 384 bit representation to
    // reduce the cost of rejection sampling.
    let repr_shave_bits = (64 * limbs as u32) - biguint_num_bits(modulus.clone());

    // Compute R = 2**(64 * limbs) mod m
    let r = (BigUint::one() << (limbs * 64)) % &modulus;

    // modulus - 1 = 2^s * t
    let mut s: u32 = 0;
    let mut t = &modulus - BigUint::from_str("1").unwrap();
    while t.is_even() {
        t = t >> 1;
        s += 1;
    }

    // Compute 2^s root of unity given the generator
    let root_of_unity = biguint_to_u64_vec((generator.clone().modpow(&t, &modulus) * &r) % &modulus, limbs);
    let generator = biguint_to_u64_vec((generator.clone() * &r) % &modulus, limbs);

    let mod_minus_1_over_2 = biguint_to_u64_vec((&modulus - BigUint::from_str("1").unwrap()) >> 1, limbs);
    let legendre_impl = quote! {
        fn legendre(&self) -> crate::ff::LegendreSymbol {
            // s = self^((modulus - 1) // 2)
            let s = self.pow(#mod_minus_1_over_2);
            if s == Self::zero() {
                crate::ff::LegendreSymbol::Zero
            } else if s == Self::one() {
                crate::ff::LegendreSymbol::QuadraticResidue
            } else {
                crate::ff::LegendreSymbol::QuadraticNonResidue
            }
        }
    };

    let sqrt_impl = if (&modulus % BigUint::from_str("4").unwrap()) == BigUint::from_str("3").unwrap() {
        let mod_minus_3_over_4 = biguint_to_u64_vec((&modulus - BigUint::from_str("3").unwrap()) >> 2, limbs);

        // Compute -R as (m - r)
        let rneg = biguint_to_u64_vec(&modulus - &r, limbs);

        quote! {
            impl crate::ff::SqrtField for #name {
                #legendre_impl

                fn sqrt(&self) -> Option<Self> {
                    // Shank's algorithm for q mod 4 = 3
                    // https://eprint.iacr.org/2012/685.pdf (page 9, algorithm 2)

                    let mut a1 = self.pow(#mod_minus_3_over_4);

                    let mut a0 = a1;
                    a0.square();
                    a0.mul_assign(self);

                    if a0.0 == #repr(#rneg) {
                        None
                    } else {
                        a1.mul_assign(self);
                        Some(a1)
                    }
                }
            }
        }
    } else if (&modulus % BigUint::from_str("16").unwrap()) == BigUint::from_str("1").unwrap() {
        let t_plus_1_over_2 = biguint_to_u64_vec((&t + BigUint::one()) >> 1, limbs);
        let t = biguint_to_u64_vec(t.clone(), limbs);

        quote! {
            impl crate::ff::SqrtField for #name {
                #legendre_impl

                fn sqrt(&self) -> Option<Self> {
                    // Tonelli-Shank's algorithm for q mod 16 = 1
                    // https://eprint.iacr.org/2012/685.pdf (page 12, algorithm 5)

                    match self.legendre() {
                        crate::ff::LegendreSymbol::Zero => Some(*self),
                        crate::ff::LegendreSymbol::QuadraticNonResidue => None,
                        crate::ff::LegendreSymbol::QuadraticResidue => {
                            let mut c = #name(ROOT_OF_UNITY);
                            let mut r = self.pow(#t_plus_1_over_2);
                            let mut t = self.pow(#t);
                            let mut m = S;

                            while t != Self::one() {
                                let mut i = 1;
                                {
                                    let mut t2i = t;
                                    t2i.square();
                                    loop {
                                        if t2i == Self::one() {
                                            break;
                                        }
                                        t2i.square();
                                        i += 1;
                                    }
                                }

                                for _ in 0..(m - i - 1) {
                                    c.square();
                                }
                                r.mul_assign(&c);
                                c.square();
                                t.mul_assign(&c);
                                m = i;
                            }

                            Some(r)
                        }
                    }
                }
            }
        }
    } else {
        quote! {}
    };

    // Compute R^2 mod m
    let r2 = biguint_to_u64_vec((&r * &r) % &modulus, limbs);

    let r = biguint_to_u64_vec(r, limbs);

    // 2^k - modulus
    let modulus_negated = (BigUint::one() << (64 * limbs)) - &modulus;

    let modulus = biguint_to_real_u64_vec(modulus, limbs);
    let modulus_negated = biguint_to_real_u64_vec(modulus_negated, limbs);

    // Compute -m^-1 mod 2**64 by exponentiating by totient(2**64) - 1
    let mut inv = 1u64;
    for _ in 0..63 {
        inv = inv.wrapping_mul(inv);
        inv = inv.wrapping_mul(modulus[0]);
    }
    inv = inv.wrapping_neg();

    let mut constants_gen = quote! {
        /// This is the modulus m of the prime field
        const MODULUS: #repr = #repr([#(#modulus,)*]);

        /// The number of bits needed to represent the modulus.
        const MODULUS_BITS: u32 = #modulus_num_bits;

        /// The number of bits that must be shaved from the beginning of
        /// the representation when randomly sampling.
        const REPR_SHAVE_BITS: u32 = #repr_shave_bits;

        /// 2^{limbs*64} mod m
        const R: #repr = #repr(#r);

        /// 2^{limbs*64*2} mod m
        const R2: #repr = #repr(#r2);

        /// -(m^{-1} mod m) mod m
        const INV: u64 = #inv;

        /// Multiplicative generator of `MODULUS` - 1 order, also quadratic
        /// nonresidue.
        const GENERATOR: #repr = #repr(#generator);

        /// 2^s * t = MODULUS - 1 with t odd
        const S: u32 = #s;

        /// 2^s root of unity computed by GENERATOR^t
        const ROOT_OF_UNITY: #repr = #repr(#root_of_unity);
    };

    for i in 0..4 {
        let m = get_temp_with_literal(&format!("{}{}_", MODULUS_PREFIX, random_id), i);
        let n = get_temp_with_literal(&format!("{}{}_", MODULUS_NEGATED_PREFIX, random_id), i);
        let value = modulus[i];
        let limb_neg = modulus_negated[i];

        constants_gen.extend(quote! {
            #[no_mangle]
            static #m: u64 = #value;
            #[no_mangle]
            static #n: u64 = #limb_neg;
        });
    }

    (constants_gen, inv, sqrt_impl)
}

/// Implement PrimeField for the derived type.
fn prime_field_impl(name: &syn::Ident, repr: &syn::Ident, mont_inv: u64, limbs: usize, random_id: u32) -> proc_macro2::TokenStream {
    // The parameter list for the mont_reduce() internal method.
    // r0: u64, mut r1: u64, mut r2: u64, ...
    let mut mont_paramlist = proc_macro2::TokenStream::new();
    mont_paramlist.append_separated(
        (0..(limbs * 2)).map(|i| (i, get_temp(i))).map(|(i, x)| {
            if i != 0 {
                quote! {mut #x: u64}
            } else {
                quote! {#x: u64}
            }
        }),
        proc_macro2::Punct::new(',', proc_macro2::Spacing::Alone),
    );

    // Implement montgomery reduction for some number of limbs
    fn mont_impl(limbs: usize) -> proc_macro2::TokenStream {
        let mut gen = proc_macro2::TokenStream::new();

        for i in 0..limbs {
            {
                let temp = get_temp(i);
                gen.extend(quote! {
                    let k = #temp.wrapping_mul(INV);
                    let mut carry = 0;
                    crate::ff::mac_with_carry(#temp, k, MODULUS.0[0], &mut carry);
                });
            }

            for j in 1..limbs {
                let temp = get_temp(i + j);
                gen.extend(quote! {
                    #temp = crate::ff::mac_with_carry(#temp, k, MODULUS.0[#j], &mut carry);
                });
            }

            let temp = get_temp(i + limbs);

            if i == 0 {
                gen.extend(quote! {
                    #temp = crate::ff::adc(#temp, 0, &mut carry);
                });
            } else {
                gen.extend(quote! {
                    #temp = crate::ff::adc(#temp, carry2, &mut carry);
                });
            }

            if i != (limbs - 1) {
                gen.extend(quote! {
                    let carry2 = carry;
                });
            }
        }

        for i in 0..limbs {
            let temp = get_temp(limbs + i);

            gen.extend(quote! {
                (self.0).0[#i] = #temp;
            });
        }

        gen
    }

    let top_limb_index = limbs - 1;

    let montgomery_impl = mont_impl(limbs);

    // (self.0).0[0], (self.0).0[1], ..., 0, 0, 0, 0, ...
    let mut into_repr_params = proc_macro2::TokenStream::new();
    into_repr_params.append_separated(
        (0..limbs).map(|i| quote! { (self.0).0[#i] }).chain((0..limbs).map(|_| quote! {0})),
        proc_macro2::Punct::new(',', proc_macro2::Spacing::Alone),
    );

    let modulus_random_prefix = format!("{}{}_", MODULUS_PREFIX, random_id);
    let modulus_neg_random_prefix = format!("{}{}_", MODULUS_NEGATED_PREFIX, random_id);

    let mul_asm_impl = mul_impl(mont_inv, &modulus_random_prefix);
    let sqr_asm_impl = sqr_impl(mont_inv, &modulus_random_prefix);
    // let add_asm_impl = add_impl(MODULUS_PREFIX);
    let add_asm_impl = add_impl(&modulus_neg_random_prefix);
    let sub_asm_impl = sub_impl(&modulus_random_prefix);
    // let sub_asm_impl = sub_impl(MODULUS_NEGATED_PREFIX);
    // let double_asm_impl = double_impl(MODULUS_PREFIX);
    let double_asm_impl = double_impl(&modulus_neg_random_prefix);

    quote! {
        impl ::std::marker::Copy for #name { }

        impl ::std::clone::Clone for #name {
            fn clone(&self) -> #name {
                *self
            }
        }

        impl ::std::cmp::PartialEq for #name {
            fn eq(&self, other: &#name) -> bool {
                self.0 == other.0
            }
        }

        impl ::std::cmp::Eq for #name { }

        impl ::std::fmt::Debug for #name
        {
            fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(f, "{}({:?})", stringify!(#name), self.into_repr())
            }
        }

        /// Elements are ordered lexicographically.
        impl Ord for #name {
            #[inline(always)]
            fn cmp(&self, other: &#name) -> ::std::cmp::Ordering {
                self.into_repr().cmp(&other.into_repr())
            }
        }

        impl PartialOrd for #name {
            #[inline(always)]
            fn partial_cmp(&self, other: &#name) -> Option<::std::cmp::Ordering> {
                Some(self.cmp(other))
            }
        }

        impl ::std::fmt::Display for #name {
            fn fmt(&self, f: &mut ::std::fmt::Formatter) -> ::std::fmt::Result {
                write!(f, "{}({})", stringify!(#name), self.into_repr())
            }
        }

        impl ::rand::Rand for #name {
            /// Computes a uniformly random element using rejection sampling.
            fn rand<R: ::rand::Rng>(rng: &mut R) -> Self {
                loop {
                    let mut tmp = #name(#repr::rand(rng));

                    // Mask away the unused bits at the beginning.
                    tmp.0.as_mut()[#top_limb_index] &= 0xffffffffffffffff >> REPR_SHAVE_BITS;

                    if tmp.is_valid() {
                        return tmp
                    }
                }
            }
        }

        impl From<#name> for #repr {
            fn from(e: #name) -> #repr {
                e.into_repr()
            }
        }

        impl crate::ff::PrimeField for #name {
            type Repr = #repr;

            fn from_repr(r: #repr) -> Result<#name, crate::ff::PrimeFieldDecodingError> {
                let mut r = #name(r);
                if r.is_valid() {
                    r.mul_assign(&#name(R2));

                    Ok(r)
                } else {
                    Err(crate::ff::PrimeFieldDecodingError::NotInField(format!("{}", r.0)))
                }
            }

            fn from_raw_repr(r: #repr) -> Result<Self, crate::ff::PrimeFieldDecodingError> {
                let mut r = #name(r);
                if r.is_valid() {
                    Ok(r)
                } else {
                    Err(crate::ff::PrimeFieldDecodingError::NotInField(format!("{}", r.0)))
                }
            }

            fn into_repr(&self) -> #repr {
                let mut r = *self;
                r.mont_reduce(
                    #into_repr_params
                );

                r.0
            }

            fn into_raw_repr(&self) -> #repr {
                let r = *self;

                r.0
            }

            fn char() -> #repr {
                MODULUS
            }

            const NUM_BITS: u32 = MODULUS_BITS;

            const CAPACITY: u32 = Self::NUM_BITS - 1;

            fn multiplicative_generator() -> Self {
                #name(GENERATOR)
            }

            const S: u32 = S;

            fn root_of_unity() -> Self {
                #name(ROOT_OF_UNITY)
            }

        }

        impl crate::ff::Field for #name {
            #[inline]
            fn zero() -> Self {
                #name(#repr::from(0))
            }

            #[inline]
            fn one() -> Self {
                #name(R)
            }

            #[inline]
            fn is_zero(&self) -> bool {
                self.0.is_zero()
            }

            #[inline]
            fn add_assign(&mut self, other: &#name) {
                (self.0).0 = #name::add_asm_adx_with_reduction(&(self.0).0, &(other.0).0);
            }

            #[inline]
            fn double(&mut self) {
                (self.0).0 = Self::double_asm_adx_with_reduction(&(self.0).0);
            }

            #[inline]
            fn sub_assign(&mut self, other: &#name) {
                (self.0).0 = Self::sub_asm_adx_with_reduction(&(self.0).0, &(other.0).0);
            }

            #[inline]
            fn negate(&mut self) {
                if !self.is_zero() {
                    let mut tmp = MODULUS;
                    tmp.sub_noborrow(&self.0);
                    self.0 = tmp;
                }
            }

            fn inverse(&self) -> Option<Self> {
                if self.is_zero() {
                    None
                } else {
                    // Guajardo Kumar Paar Pelzl
                    // Efficient Software-Implementation of Finite Fields with Applications to Cryptography
                    // Algorithm 16 (BEA for Inversion in Fp)

                    let one = #repr::from(1);

                    let mut u = self.0;
                    let mut v = MODULUS;
                    let mut b = #name(R2); // Avoids unnecessary reduction step.
                    let mut c = Self::zero();

                    while u != one && v != one {
                        while u.is_even() {
                            u.div2();

                            if b.0.is_even() {
                                b.0.div2();
                            } else {
                                b.0.add_nocarry(&MODULUS);
                                b.0.div2();
                            }
                        }

                        while v.is_even() {
                            v.div2();

                            if c.0.is_even() {
                                c.0.div2();
                            } else {
                                c.0.add_nocarry(&MODULUS);
                                c.0.div2();
                            }
                        }

                        if v < u {
                            u.sub_noborrow(&v);
                            b.sub_assign(&c);
                        } else {
                            v.sub_noborrow(&u);
                            c.sub_assign(&b);
                        }
                    }

                    if u == one {
                        Some(b)
                    } else {
                        Some(c)
                    }
                }
            }

            #[inline(always)]
            fn frobenius_map(&mut self, _: usize) {
                // This has no effect in a prime field.
            }

            #[inline]
            fn mul_assign(&mut self, other: &#name)
            {
                (self.0).0 = Self::mont_mul_asm_adx_with_reduction(&(self.0).0, &(other.0).0);
            }

            #[inline]
            fn square(&mut self)
            {
                (self.0).0 = Self::mont_sqr_asm_adx_with_reduction(&(self.0).0);
            }
        }

        impl std::default::Default for #name {
            fn default() -> Self {
                Self::zero()
            }
        }

        impl std::hash::Hash for #name {
            fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
                for limb in self.0.as_ref().iter() {
                    limb.hash(state);
                }
            }
        }

        impl #name {
            /// Determines if the element is really in the field. This is only used
            /// internally.
            #[inline(always)]
            fn is_valid(&self) -> bool {
                self.0 < MODULUS
            }

            /// Subtracts the modulus from this element if this element is not in the
            /// field. Only used interally.
            #[inline(always)]
            fn reduce(&mut self) {
                if !self.is_valid() {
                    self.0.sub_noborrow(&MODULUS);
                }
            }

            #[inline(always)]
            fn mont_reduce(
                &mut self,
                #mont_paramlist
            )
            {
                // The Montgomery reduction here is based on Algorithm 14.32 in
                // Handbook of Applied Cryptography
                // <http://cacr.uwaterloo.ca/hac/about/chap14.pdf>.

                #montgomery_impl

                self.reduce();
            }

            #mul_asm_impl

            #sqr_asm_impl

            #add_asm_impl

            #sub_asm_impl

            #double_asm_impl
        }

        impl ::serde::Serialize for #name {
            fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
                where S: ::serde::Serializer
            {
                let repr = self.into_repr();
                repr.serialize(serializer)
            }
        }

        impl<'de> ::serde::Deserialize<'de> for #name {
            fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
            where D: ::serde::Deserializer<'de>
            {
                let repr = #repr::deserialize(deserializer)?;
                let new = Self::from_repr(repr).map_err(::serde::de::Error::custom)?;

                Ok(new)
            }
        }
    }
}
