use super::*;

/// Convert BigUint into a vector of 64-bit limbs.
pub(crate) fn biguint_to_real_u64_vec(mut v: BigUint, limbs: usize) -> Vec<u64> {
    let m = BigUint::one() << 64;
    let mut ret = vec![];

    while v > BigUint::zero() {
        let rem: BigUint = &v % &m;
        ret.push(rem.to_u64().unwrap());
        v = v >> 64;
    }

    while ret.len() < limbs {
        ret.push(0);
    }

    assert!(ret.len() == limbs);

    ret
}

/// Convert BigUint into a tokenized vector of 64-bit limbs.
pub(crate) fn biguint_to_u64_vec(v: BigUint, limbs: usize) -> proc_macro2::TokenStream {
    let ret = biguint_to_real_u64_vec(v, limbs);
    quote!([#(#ret,)*])
}

pub(crate) fn biguint_num_bits(mut v: BigUint) -> u32 {
    let mut bits = 0;

    while v != BigUint::zero() {
        v = v >> 1;
        bits += 1;
    }

    bits
}
