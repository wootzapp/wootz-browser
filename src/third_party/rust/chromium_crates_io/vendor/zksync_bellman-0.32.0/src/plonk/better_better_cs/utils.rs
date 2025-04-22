use crate::pairing::ff::PrimeField;
use crate::plonk::domains::*;
use crate::worker::Worker;
use crate::SynthesisError;

use crate::plonk::polynomials::*;

pub trait FieldBinop<F: PrimeField>: 'static + Copy + Clone + Send + Sync + std::fmt::Debug {
    fn apply(&self, dest: &mut F, source: &F);
}

pub(crate) fn binop_over_slices<F: PrimeField, B: FieldBinop<F>>(worker: &Worker, binop: &B, dest: &mut [F], source: &[F]) {
    assert_eq!(dest.len(), source.len());
    worker.scope(dest.len(), |scope, chunk| {
        for (dest, source) in dest.chunks_mut(chunk).zip(source.chunks(chunk)) {
            scope.spawn(move |_| {
                for (dest, source) in dest.iter_mut().zip(source.iter()) {
                    binop.apply(dest, source);
                }
            });
        }
    });
}

#[derive(Clone, Copy, Debug)]
pub struct BinopAddAssign;

impl<F: PrimeField> FieldBinop<F> for BinopAddAssign {
    #[inline(always)]
    fn apply(&self, dest: &mut F, source: &F) {
        dest.add_assign(source);
    }
}

#[derive(Clone, Copy, Debug)]
pub struct BinopAddAssignScaled<F: PrimeField> {
    pub scale: F,
}

impl<F: PrimeField> BinopAddAssignScaled<F> {
    pub fn new(scale: F) -> Self {
        Self { scale }
    }
}

impl<F: PrimeField> FieldBinop<F> for BinopAddAssignScaled<F> {
    #[inline(always)]
    fn apply(&self, dest: &mut F, source: &F) {
        let mut tmp = self.scale;
        tmp.mul_assign(&source);

        dest.add_assign(&tmp);
    }
}

pub(crate) fn get_degree<F: PrimeField>(poly: &Polynomial<F, Coefficients>) -> usize {
    let mut degree = poly.as_ref().len() - 1;
    for c in poly.as_ref().iter().rev() {
        if c.is_zero() {
            degree -= 1;
        } else {
            break;
        }
    }

    degree
}

pub(crate) fn calculate_inverse_vanishing_polynomial_with_last_point_cut<F: PrimeField>(
    worker: &Worker,
    poly_size: usize,
    vahisning_size: usize,
    coset_factor: F,
) -> Result<Polynomial<F, Values>, SynthesisError> {
    assert!(poly_size.is_power_of_two());
    assert!(vahisning_size.is_power_of_two());

    // update from the paper - it should not hold for the last generator, omega^(n) in original notations
    // Z(X) = (X^(n+1) - 1) / (X - omega^(n)) => Z^{-1}(X) = (X - omega^(n)) / (X^(n+1) - 1)

    let domain = Domain::<F>::new_for_size(vahisning_size as u64)?;
    let n_domain_omega = domain.generator;
    let mut root = n_domain_omega.pow([(vahisning_size - 1) as u64]);
    root.negate();

    let mut negative_one = F::one();
    negative_one.negate();

    let mut numerator = Polynomial::<F, Values>::from_values(vec![coset_factor; poly_size])?;
    // evaluate X in linear time

    numerator.distribute_powers(&worker, numerator.omega);
    numerator.add_constant(&worker, &root);

    // numerator.add_constant(&worker, &negative_one);
    // now it's a series of values in a coset

    // now we should evaluate X^(n+1) - 1 in a linear time

    let shift = coset_factor.pow([vahisning_size as u64]);

    let mut denominator = Polynomial::<F, Values>::from_values(vec![shift; poly_size])?;

    // elements are h^size - 1, (hg)^size - 1, (hg^2)^size - 1, ...

    denominator.distribute_powers(&worker, denominator.omega.pow([vahisning_size as u64]));
    denominator.add_constant(&worker, &negative_one);

    denominator.batch_inversion(&worker)?;

    numerator.mul_assign(&worker, &denominator);

    Ok(numerator)
}
