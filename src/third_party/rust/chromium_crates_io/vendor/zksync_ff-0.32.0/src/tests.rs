extern crate rand;

extern crate test as rust_test;
extern crate num_bigint;
extern crate num_traits;
extern crate num_integer;

#[cfg(feature = "derive")]
mod benches {
    use crate::Field;
    use crate::*;
    use super::rust_test::Bencher;
    use rand::{Rng, XorShiftRng, SeedableRng};
    use super::num_bigint::BigUint;
    use super::num_traits::identities::{Zero, One};
    use super::num_traits::{ToPrimitive, Num};
    use super::num_integer::Integer;

    #[derive(PrimeField)]
    #[PrimeFieldModulus = "21888242871839275222246405745257275088696311157297823662689037894645226208583"]
    #[PrimeFieldGenerator = "2"]
    struct Fr(pub FrRepr);

    #[bench]
    fn bench_arith(bencher: &mut Bencher) {
        let rng = &mut XorShiftRng::from_seed([0x5dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
        let mut a: Fr = rng.gen();
        let b: Fr = rng.gen();

        bencher.iter(|| {
            for _ in 0..100 {
                a.mul_assign(&b);
            }
        });

    }

    #[bench]
    fn bench_cios(bencher: &mut Bencher) {
        let rng = &mut XorShiftRng::from_seed([0x5dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
        let mut a: Fr = rng.gen();
        let b: Fr = rng.gen();

        fn modular_inverse(el: &BigUint, modulus: &BigUint) -> BigUint {
            let mut a = el.clone();
            let mut new = BigUint::one();
            let mut old = BigUint::zero();
            let mut q = modulus.clone();
            let mut r = BigUint::zero();
            let mut h = BigUint::zero();
            let mut positive = false;
            while !a.is_zero() {
                let (q_new, r_new) = q.div_mod_floor(&a);
                r = r_new;
                q = q_new;
                h = (q * new.clone() + old.clone()) % modulus;
                old = new;
                new = h;
                q = a;
                a = r;
                positive = !positive;
            }
            if positive {
                return old;
            } else {
                return modulus - old;
            }
        }

        let mut modulus = BigUint::from_str_radix("21888242871839275222246405745257275088696311157297823662689037894645226208583", 10).unwrap();
        let mut mont_r = (BigUint::one() << 256) % modulus.clone();
        let r_inv = modular_inverse(&mont_r, &modulus);

        let (_, rem) = (mont_r.clone() * r_inv.clone()).div_mod_floor(&modulus);
        assert!(rem == BigUint::one());

        let modulus_512 = (modulus.clone() << 256);

        let r_r_inv = (r_inv.clone() << 256);
        let subtracted = r_r_inv - BigUint::one();

        let (mont_k, rem) = subtracted.div_mod_floor(&modulus_512);
        let mut mont_k = mont_k % (BigUint::one() << 256);

        let mut mont_k_fixed = [0u64; 4];
        for i in 0..4 {
            let limb = mont_k.clone() % (BigUint::one() << 64);
            mont_k_fixed[i] = limb.to_u64().unwrap();
            mont_k = mont_k.clone() >> 64;
        }

        let mut modulus_fixed = [0u64; 4];
        for i in 0..4 {
            let limb = modulus.clone() % (BigUint::one() << 64);
            modulus_fixed[i] = limb.to_u64().unwrap();
            modulus = modulus.clone() >> 64;
        }

        let mont_k = mont_k_fixed;
        let modulus = modulus_fixed;


        fn cios_mul(a: &[u64; 4], b: &[u64; 4], mont_k: &[u64; 4], modulus: &[u64; 4]) -> [u64; 4] {
            let mut t = [0u64; 8];
            let limbs = b.len();
            for i in 0..limbs {
                let mut carry = 0u64;
                let limb = b[i];
                for j in 0..limbs {
                    t[j] = crate::arith_impl::mac_with_carry(t[j], a[j], limb, &mut carry);
                }
                t[limbs] = crate::arith_impl::adc(t[limbs], 0, &mut carry);
                t[limbs+1] = carry;
                let mut m = crate::arith_impl::mac_with_carry(0u64, t[0], mont_k[0], &mut 0u64);
                let mut m = m as u64;
                let mut carry = 0;
                crate::arith_impl::mac_with_carry(t[0], m, modulus[0], &mut carry);
                for j in 1..limbs {
                    t[j-1] = crate::arith_impl::mac_with_carry(t[j], m, modulus[j], &mut carry);
                }
                t[limbs-1] = crate::arith_impl::adc(t[limbs], 0, &mut carry);
                t[limbs] = t[limbs+1] + carry;
            }

            let mut u = t;
            let mut borrow = 0;
            for i in 0..limbs {
                t[i] = crate::arith_impl::sbb(u[i], modulus[i], &mut borrow);
            }
            t[limbs] = crate::arith_impl::sbb(u[limbs], 0, &mut borrow);
            if borrow == 0 {
                return [t[0], t[1], t[2], t[3]];
            } else {
                return [u[0], u[1], u[2], u[3]];
            }
        }

        let a_repr = (a.0).0;
        let b_repr = (b.0).0;

        bencher.iter(|| {
            for _ in 0..100 {
                cios_mul(&a_repr, &b_repr, &mont_k, &modulus);
            }
        });

    }
}





