use crate::ff::*;

use crate::pairing::Engine;

#[macro_use]
mod impl_macro;

#[derive(PrimeField)]
#[PrimeFieldModulus = "3618502788666131213697322783095070105623107215331596699973092056135872020481"]
#[PrimeFieldGenerator = "3"]
pub struct Fr(FrRepr);

pub trait TransparentEngine: Engine {}

pub trait PartialReductionField: PrimeField {
    /// Adds another element by this element without reduction.
    fn add_assign_unreduced(&mut self, other: &Self);

    /// Subtracts another element by this element without reduction.
    fn sub_assign_unreduced(&mut self, other: &Self);

    /// Multiplies another element by this element without reduction.
    fn mul_assign_unreduced(&mut self, other: &Self);

    /// Reduces this element.
    fn reduce_once(&mut self);

    /// Reduces this element by max of three moduluses.
    fn reduce_completely(&mut self);

    fn overflow_factor(&self) -> usize;
}

pub trait PartialTwoBitReductionField: PartialReductionField {
    /// Subtracts another element by this element without reduction.
    fn sub_assign_twice_unreduced(&mut self, other: &Self);

    /// Reduces this element by two moduluses.
    fn reduce_twice(&mut self);

    /// Reduces this element by max of three moduluses.
    fn reduce_completely(&mut self);
}

pub mod engine {
    use super::Fr;

    use super::impl_macro::*;

    transparent_engine_impl! {Transparent252, Fr}
}

pub use self::engine::Transparent252;

pub(crate) mod proth;
pub(crate) mod proth_engine;

#[cfg(test)]
mod test {
    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_bench_proth_lde() {
        use super::proth::Fr as FrOptimized;
        use super::Fr as FrMontNaive;
        use crate::plonk::commitments::transparent::utils::*;
        use crate::plonk::polynomials::*;
        use crate::worker::*;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};
        use std::time::Instant;

        let poly_sizes = vec![1_000_000, 2_000_000, 4_000_000];

        let worker = Worker::new();

        for poly_size in poly_sizes.into_iter() {
            let res1 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| FrMontNaive::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<FrMontNaive, _>::from_coeffs(coeffs).unwrap();
                let start = Instant::now();
                let eval_result = poly.lde(&worker, 16).unwrap();
                println!("LDE with factor 16 for size {} with naive mont reduction taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            let res2 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| FrOptimized::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<FrOptimized, _>::from_coeffs(coeffs).unwrap();
                let start = Instant::now();
                let eval_result = poly.lde(&worker, 16).unwrap();
                println!("LDE with factor 16 for size {} with optimized mont reduction taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            assert_eq!(format!("{}", res1[0]), format!("{}", res2[0]));
        }
    }

    #[test]
    fn test_proth_field() {
        use super::proth::Fr as FrOptimized;
        use super::Fr as FrMontNaive;
        use crate::ff::{to_hex, Field, PrimeField};

        let one_naive = FrMontNaive::from_str("1").unwrap();
        let one_optimized = FrOptimized::from_str("1").unwrap();

        println!("{}", FrMontNaive::one());
        println!("{}", FrOptimized::one());

        println!("{}", one_naive.into_raw_repr());
        println!("{}", one_optimized.into_raw_repr());

        let mut tmp0 = one_naive;
        tmp0.mul_assign(&one_naive);

        let mut tmp1 = one_optimized;
        tmp1.mul_assign(&one_optimized);

        assert_eq!(to_hex(&tmp0), to_hex(&tmp1));

        assert_eq!(to_hex(&FrMontNaive::multiplicative_generator()), to_hex(&FrOptimized::multiplicative_generator()));
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_bench_precomputations_for_proth_fft() {
        use super::proth::Fr as FrOptimized;
        use super::Fr as FrMontNaive;
        use crate::plonk::commitments::transparent::precomputations::*;
        use crate::plonk::commitments::transparent::utils::*;
        use crate::plonk::domains::Domain;
        use crate::plonk::fft::fft::best_fft;
        use crate::plonk::fft::with_precomputation::fft::best_fft as best_fft_with_precomputations;
        use crate::plonk::fft::with_precomputation::FftPrecomputations;
        use crate::plonk::polynomials::*;
        use crate::worker::*;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};
        use std::time::Instant;
        let poly_sizes = vec![32_000_000, 64_000_000];

        let worker = Worker::new();

        for poly_size in poly_sizes.into_iter() {
            let domain = Domain::<FrOptimized>::new_for_size(poly_size).unwrap();
            let precomp = PrecomputedOmegas::<FrOptimized>::new_for_domain_size(domain.size as usize);
            let omega = domain.generator;
            let log_n = domain.power_of_two as u32;

            let poly_size = domain.size as usize;

            let res1 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let mut coeffs = (0..poly_size).map(|_| FrOptimized::rand(rng)).collect::<Vec<_>>();
                let start = Instant::now();
                best_fft(&mut coeffs, &worker, &omega, log_n, None);
                println!("FFT for size {} without precomputation taken {:?}", poly_size, start.elapsed());

                coeffs
            };

            let res2 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let mut coeffs = (0..poly_size).map(|_| FrOptimized::rand(rng)).collect::<Vec<_>>();
                let start = Instant::now();
                best_fft_with_precomputations(&mut coeffs, &worker, &omega, log_n, None, &precomp);
                println!("FFT for size {} with precomputation taken {:?}", poly_size, start.elapsed());

                coeffs
            };

            assert!(res1 == res2);
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_bench_precomputations_for_proth_lde() {
        use super::proth::Fr as FrOptimized;
        use super::Fr as FrMontNaive;
        use crate::plonk::commitments::transparent::precomputations::*;
        use crate::plonk::commitments::transparent::utils::*;
        use crate::plonk::fft::with_precomputation::FftPrecomputations;
        use crate::plonk::polynomials::*;
        use crate::worker::*;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};
        use std::time::Instant;

        let poly_sizes = vec![1_000_000, 2_000_000, 4_000_000];

        let worker = Worker::new();

        for poly_size in poly_sizes.into_iter() {
            let res1 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| FrMontNaive::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<FrMontNaive, _>::from_coeffs(coeffs).unwrap();
                let start = Instant::now();
                let eval_result = poly.lde(&worker, 16).unwrap();
                println!("LDE with factor 16 for size {} with naive mont reduction taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            let res2 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| FrOptimized::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<FrOptimized, _>::from_coeffs(coeffs).unwrap();
                let precomp = PrecomputedOmegas::<FrOptimized>::new_for_domain_size(poly.size());
                let start = Instant::now();
                let eval_result = poly.lde_using_multiple_cosets_with_precomputation(&worker, 16, &precomp).unwrap();
                println!("LDE with factor 16 for size {} with optimized mont reduction and precomputation taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            assert_eq!(format!("{}", res1[0]), format!("{}", res2[0]));
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_bench_ct_ploth_lde() {
        use super::proth::Fr;
        use crate::plonk::commitments::transparent::utils::*;
        use crate::plonk::fft::cooley_tukey_ntt::{BitReversedOmegas, CTPrecomputations};
        use crate::plonk::polynomials::*;
        use crate::worker::*;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};
        use std::time::Instant;

        let poly_sizes = vec![1_000_000, 2_000_000, 4_000_000];

        // let poly_sizes = vec![2];

        let worker = Worker::new();

        for poly_size in poly_sizes.into_iter() {
            let poly_size = poly_size as usize;

            let res1 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<Fr, _>::from_coeffs(coeffs).unwrap();
                let start = Instant::now();
                let eval_result = poly.lde_using_multiple_cosets(&worker, 16).unwrap();
                println!("LDE with factor 16 for size {} taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            let res2 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<Fr, _>::from_coeffs(coeffs).unwrap();
                let precomp = BitReversedOmegas::<Fr>::new_for_domain_size(poly.size());
                let start = Instant::now();
                let eval_result = poly.lde_using_bitreversed_ntt(&worker, 16, &precomp).unwrap();
                println!("LDE with factor 16 for size {} with optimized ntt taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            assert_eq!(res1, res2);

            assert!(res1 == res2);
        }
    }

    // #[test]
    // fn test_bench_noalloc_bit_reversed_ploth_lde() {
    //     use rand::{XorShiftRng, SeedableRng, Rand, Rng};
    //     use super::proth::Fr as Fr;
    //     use crate::plonk::polynomials::*;
    //     use std::time::Instant;
    //     use crate::worker::*;
    //     use crate::plonk::commitments::transparent::utils::*;
    //     use crate::plonk::fft::cooley_tukey_ntt::{CTPrecomputations, BitReversedOmegas};

    //     let poly_sizes = vec![32, 64,1_000_000, 2_000_000, 4_000_000];

    //     let worker = Worker::new();

    //     for poly_size in poly_sizes.into_iter() {
    //         let poly_size = poly_size as usize;

    //         let res1 = {
    //             let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
    //             let coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();

    //             let poly = Polynomial::<Fr, _>::from_coeffs(coeffs).unwrap();
    //             let start = Instant::now();
    //             let eval_result = poly.lde(&worker, 16).unwrap();
    //             println!("LDE with factor 16 for size {} taken {:?}", poly_size, start.elapsed());

    //             eval_result.into_coeffs()
    //         };

    //         let res2 = {
    //             let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
    //             let coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();

    //             let poly = Polynomial::<Fr, _>::from_coeffs(coeffs).unwrap();
    //             let precomp = BitReversedOmegas::<Fr>::new_for_domain_size(poly.size());
    //             let start = Instant::now();
    //             let eval_result = poly.lde_using_bitreversed_ntt_no_allocations_lowest_bits_reversed(&worker, 16, &precomp).unwrap();
    //             println!("LDE with factor 16 for size {} with optimized ntt taken {:?}", poly_size, start.elapsed());

    //             eval_result.into_coeffs()
    //         };

    //         use crate::ff::PrimeField;

    //         fn check_permutation<F: PrimeField>(one: &[F], two: &[F]) -> (bool, Vec<usize>) {
    //             let mut permutation: Vec<usize> = (0..one.len()).collect();
    //             let mut valid = true;

    //             for (i, el) in one.iter().enumerate() {
    //                 let mut idx = 0;
    //                 let mut found = false;
    //                 for (j, el2) in two.iter().enumerate() {
    //                     if *el == *el2 {
    //                         idx = j;
    //                         found = true;
    //                         break;
    //                     }
    //                 }
    //                 if !found {
    //                     println!("Not found for {}", i);
    //                     valid = false;
    //                     break;
    //                 }
    //                 permutation[i] = idx;
    //             }

    //             (valid, permutation)
    //         }

    //         if poly_size < 1000 {
    //             let (valid, permutation) = check_permutation(&res1, &res2);

    //             assert!(valid);
    //         }
    //     }
    // }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_bench_partial_reduction_bitreversed_lde() {
        use super::proth::Fr;
        use super::PartialTwoBitReductionField;
        use crate::ff::Field;
        use crate::ff::PrimeField;
        use crate::plonk::commitments::transparent::utils::*;
        use crate::plonk::fft::cooley_tukey_ntt::{BitReversedOmegas, CTPrecomputations};
        use crate::plonk::polynomials::*;
        use crate::worker::*;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};
        use std::time::Instant;

        let poly_sizes = vec![32, 64, 1_000_000, 2_000_000, 4_000_000];

        let worker = Worker::new();

        for poly_size in poly_sizes.into_iter() {
            let poly_size = poly_size as usize;

            let res1 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<Fr, _>::from_coeffs(coeffs).unwrap();
                let start = Instant::now();
                let eval_result = poly.lde(&worker, 16).unwrap();
                println!("LDE with factor 16 for size {} taken {:?}", poly_size, start.elapsed());

                eval_result.into_coeffs()
            };

            let mut res2 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();

                let poly = Polynomial::<Fr, _>::from_coeffs(coeffs).unwrap();
                let precomp = BitReversedOmegas::<Fr>::new_for_domain_size(poly.size());
                let start = Instant::now();
                let eval_result = poly.bitreversed_lde_using_bitreversed_ntt(&worker, 16, &precomp, &<Fr as Field>::one()).unwrap();
                println!("LDE with factor 16 for size {} bitreversed {:?}", poly_size, start.elapsed());

                eval_result
            };

            if poly_size < 1000 {
                res2.bitreverse_enumeration(&worker);
                assert!(res1 == res2.into_coeffs());
            }
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Failure (on MacOS).
    fn test_bench_ct_proth_fft() {
        use super::proth::Fr;
        use crate::plonk::commitments::transparent::utils::*;
        use crate::plonk::domains::Domain;
        use crate::plonk::fft::cooley_tukey_ntt::{best_ct_ntt, BitReversedOmegas, CTPrecomputations};
        use crate::plonk::fft::fft::best_fft;
        use crate::plonk::polynomials::*;
        use crate::worker::*;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};
        use std::time::Instant;

        // let poly_sizes = vec![1_000_000, 2_000_000];
        let poly_sizes = vec![1_000_000];

        // let poly_sizes = vec![2];

        let worker = Worker::new();

        for poly_size in poly_sizes.into_iter() {
            let poly_size = poly_size as usize;
            let poly_size = poly_size.next_power_of_two();

            let _res1 = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let mut coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();
                let log_n = log2_floor(coeffs.len());
                let domain = Domain::new_for_size(coeffs.len() as u64).unwrap();

                let start = Instant::now();
                best_fft(&mut coeffs, &worker, &domain.generator, log_n, Some(8));
                println!("FFT for size {} taken {:?}", poly_size, start.elapsed());

                coeffs
            };

            let (input, output) = {
                let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);
                let mut coeffs = (0..poly_size).map(|_| Fr::rand(rng)).collect::<Vec<_>>();
                let input = coeffs.clone();
                let log_n = log2_floor(coeffs.len());
                let precomp = BitReversedOmegas::<Fr>::new_for_domain_size(coeffs.len());

                let start = Instant::now();
                best_ct_ntt(&mut coeffs, &worker, log_n, Some(8), &precomp);
                println!("CT FFT for size {} taken {:?}", poly_size, start.elapsed());

                (input, coeffs)
            };

            let mut writer = std::io::BufWriter::with_capacity(1 << 24, std::fs::File::create("./fft_test_input.data").unwrap());

            use crate::pairing::ff::PrimeFieldRepr;
            use crate::pairing::ff::{Field, PrimeField};
            use std::io::Write;

            let mut scratch = vec![];
            for el in input.into_iter() {
                let repr = el.into_raw_repr();
                repr.write_le(&mut scratch).unwrap();
                writer.write_all(&scratch).unwrap();
                scratch.truncate(0);
            }

            println!("Results");
            for el in output[0..16].iter() {
                println!("{}", el.into_raw_repr());
            }

            let domain = Domain::<Fr>::new_for_size(poly_size as u64).unwrap();

            println!("Omegas");

            assert!(domain.generator.pow(&[1u64 << 20]) == Fr::one());

            for i in 0..=20 {
                let pow = 1u64 << i;
                println!("Omega^{} = {}", pow, domain.generator.pow(&[pow]).into_raw_repr());
            }

            println!("Idenity = {}", Fr::one().into_raw_repr());
        }
    }
}
