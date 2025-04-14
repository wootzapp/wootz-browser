use crate::ff::PrimeField;
use crate::pairing::{CurveAffine, Engine};
use crate::plonk::commitments::transcript::Transcript;
use crate::plonk::domains::Domain;
use crate::plonk::polynomials::*;
use crate::worker::Worker;
use crate::SynthesisError;

pub(crate) fn calculate_inverse_vanishing_polynomial_in_a_coset<F: PrimeField>(worker: &Worker, poly_size: usize, vahisning_size: usize) -> Result<Polynomial<F, Values>, SynthesisError> {
    assert!(poly_size.is_power_of_two());
    assert!(vahisning_size.is_power_of_two());

    // update from the paper - it should not hold for the last generator, omega^(n) in original notations

    // Z(X) = (X^(n+1) - 1) / (X - omega^(n)) => Z^{-1}(X) = (X - omega^(n)) / (X^(n+1) - 1)

    let domain = Domain::<F>::new_for_size(vahisning_size as u64)?;
    let n_domain_omega = domain.generator;
    let mut root = n_domain_omega.pow([(vahisning_size - 1) as u64]);
    root.negate();

    let multiplicative_generator = F::multiplicative_generator();

    let mut negative_one = F::one();
    negative_one.negate();

    let mut numerator = Polynomial::<F, Values>::from_values(vec![multiplicative_generator; poly_size])?;
    // evaluate X in linear time

    numerator.distribute_powers(&worker, numerator.omega);
    numerator.add_constant(&worker, &root);

    // numerator.add_constant(&worker, &negative_one);
    // now it's a series of values in a coset

    // now we should evaluate X^(n+1) - 1 in a linear time

    let shift = multiplicative_generator.pow([vahisning_size as u64]);

    let mut denominator = Polynomial::<F, Values>::from_values(vec![shift; poly_size])?;

    // elements are h^size - 1, (hg)^size - 1, (hg^2)^size - 1, ...

    denominator.distribute_powers(&worker, denominator.omega.pow([vahisning_size as u64]));
    denominator.add_constant(&worker, &negative_one);

    denominator.batch_inversion(&worker)?;

    numerator.mul_assign(&worker, &denominator);

    Ok(numerator)
}

pub(crate) fn evaluate_inverse_vanishing_poly_with_last_point_cut<F: PrimeField>(vahisning_size: usize, point: F) -> F {
    assert!(vahisning_size.is_power_of_two());

    // update from the paper - it should not hold for the last generator, omega^(n) in original notations

    // Z(X) = (X^(n+1) - 1) / (X - omega^(n)) => Z^{-1}(X) = (X - omega^(n)) / (X^(n+1) - 1)

    let domain = Domain::<F>::new_for_size(vahisning_size as u64).expect("should fit");
    let n_domain_omega = domain.generator;
    let root = n_domain_omega.pow([(vahisning_size - 1) as u64]);

    let mut numerator = point;
    numerator.sub_assign(&root);

    let mut denominator = point.pow([vahisning_size as u64]);
    denominator.sub_assign(&F::one());

    let denominator = denominator.inverse().expect("must exist");

    numerator.mul_assign(&denominator);

    numerator
}

pub(crate) fn calculate_lagrange_poly<F: PrimeField>(worker: &Worker, poly_size: usize, poly_number: usize) -> Result<Polynomial<F, Coefficients>, SynthesisError> {
    assert!(poly_size.is_power_of_two());
    assert!(poly_number < poly_size);

    let mut poly = Polynomial::<F, Values>::from_values(vec![F::zero(); poly_size])?;

    poly.as_mut()[poly_number] = F::one();

    Ok(poly.ifft(&worker))
}

pub(crate) fn evaluate_vanishing_polynomial_of_degree_on_domain_size<F: PrimeField>(
    vanishing_degree: u64,
    coset_factor: &F,
    domain_size: u64,
    worker: &Worker,
) -> Result<Polynomial<F, Values>, SynthesisError> {
    let domain = Domain::<F>::new_for_size(domain_size)?;
    let domain_generator = domain.generator;

    let coset_factor = coset_factor.pow(&[vanishing_degree]);

    let domain_generator_in_vanishing_power = domain_generator.pow(&[vanishing_degree]);

    let mut minus_one = F::one();
    minus_one.negate();

    let mut result = vec![minus_one; domain.size as usize];

    worker.scope(result.len(), |scope, chunk_size| {
        for (chunk_id, chunk) in result.chunks_mut(chunk_size).enumerate() {
            scope.spawn(move |_| {
                let start = chunk_id * chunk_size;
                let mut pow = domain_generator_in_vanishing_power.pow(&[start as u64]);
                pow.mul_assign(&coset_factor);
                for el in chunk.iter_mut() {
                    el.add_assign(&pow);
                    pow.mul_assign(&domain_generator_in_vanishing_power);
                }
            });
        }
    });

    Polynomial::from_values(result)
}

pub(crate) fn evaluate_vanishing_for_size<F: PrimeField>(point: &F, vanishing_domain_size: u64) -> F {
    let mut result = point.pow(&[vanishing_domain_size]);
    result.sub_assign(&F::one());

    result
}

pub(crate) fn evaluate_l0_at_point<F: PrimeField>(domain_size: u64, point: F) -> Result<F, SynthesisError> {
    let size_as_fe = F::from_str(&format!("{}", domain_size)).unwrap();

    let mut den = point;
    den.sub_assign(&F::one());
    den.mul_assign(&size_as_fe);

    let den = den.inverse().ok_or(SynthesisError::DivisionByZero)?;

    let mut num = point.pow(&[domain_size]);
    num.sub_assign(&F::one());
    num.mul_assign(&den);

    Ok(num)
}

pub(crate) fn evaluate_lagrange_poly_at_point<F: PrimeField>(poly_number: usize, domain: &Domain<F>, point: F) -> Result<F, SynthesisError> {
    // lagrange polynomials have a form
    // (omega^i / N) / (X - omega^i) * (X^N - 1)

    let mut num = evaluate_vanishing_for_size(&point, domain.size);
    let omega_power = domain.generator.pow(&[poly_number as u64]);
    num.mul_assign(&omega_power);

    let size_as_fe = F::from_str(&format!("{}", domain.size)).unwrap();

    let mut den = point;
    den.sub_assign(&omega_power);
    den.mul_assign(&size_as_fe);

    let den = den.inverse().ok_or(SynthesisError::DivisionByZero)?;

    num.mul_assign(&den);

    Ok(num)
}

use crate::ff::SqrtField;

pub fn make_non_residues<F: PrimeField + SqrtField>(num: usize) -> Vec<F> {
    // create largest domain possible
    assert!(F::S < 63);
    let domain_size = 1u64 << (F::S as u64);

    let domain = Domain::<F>::new_for_size(domain_size).expect("largest domain must exist");

    make_non_residues_for_domain(num, &domain)
}

pub fn make_non_residues_for_domain<F: PrimeField + SqrtField>(num: usize, domain: &Domain<F>) -> Vec<F> {
    use crate::ff::LegendreSymbol;

    // we need to check that
    // - some k is not a residue
    // - it's NOT a part of coset formed as other_k * {1, omega^1, ...}

    let mut non_residues = vec![];
    let mut current = F::one();
    let one = F::one();
    for _ in 0..num {
        loop {
            if current.legendre() != LegendreSymbol::QuadraticNonResidue {
                current.add_assign(&one);
            } else {
                let mut is_unique = true;
                {
                    // first pow into the domain size
                    let tmp = current.pow(&[domain.size]);
                    // then check: if it's in some other coset, then
                    // X^N == other_k ^ N
                    for t in Some(one).iter().chain(non_residues.iter()) {
                        if !is_unique {
                            break;
                        }
                        let t_in_domain_size = t.pow(&[domain.size]);
                        if tmp == t_in_domain_size {
                            is_unique = false;
                        }
                    }
                }
                if is_unique {
                    non_residues.push(current);
                    current.add_assign(&one);
                    break;
                }
            }
        }
    }

    non_residues
}

pub fn commit_point_as_xy<E: Engine, T: Transcript<E::Fr>>(transcript: &mut T, point: &E::G1Affine) {
    use crate::ff::Field;

    if point.is_zero() {
        transcript.commit_fe(&E::Fq::zero());
        transcript.commit_fe(&E::Fq::zero());
    } else {
        let (x, y) = point.into_xy_unchecked();
        transcript.commit_fe(&x);
        transcript.commit_fe(&y);
    }
}

#[cfg(test)]
mod test {
    #[test]
    #[ignore = "Test is too slow"]
    fn test_lagrange_poly_explicit_multicore_validity() {
        use super::*;
        use crate::ff::{Field, PrimeField};
        use crate::pairing::bn256::Fr;

        if cfg!(debug_assertions) {
            println!("Will be too slow to run in test mode, abort");
            return;
        }

        use crate::worker::Worker;
        use rand::{Rand, Rng, SeedableRng, XorShiftRng};

        let size: usize = 1 << 21;
        let worker = Worker::new();

        let mut reference: Option<Polynomial<_, _>> = None;

        for _ in 0..100 {
            for num_cpus in 1..=32 {
                let subworker = Worker::new_with_cpus(num_cpus);
                let candidate = calculate_lagrange_poly::<Fr>(&subworker, size.next_power_of_two(), 0).unwrap();

                if let Some(to_compare) = reference.take() {
                    assert_eq!(candidate.as_ref(), to_compare.as_ref(), "mismatch for {} cpus", num_cpus);
                } else {
                    reference = Some(candidate);
                }

                println!("Completed for {} cpus", num_cpus);
            }
        }
    }
}
