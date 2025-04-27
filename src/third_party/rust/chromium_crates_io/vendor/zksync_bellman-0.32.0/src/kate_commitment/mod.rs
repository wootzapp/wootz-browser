use crate::ff::{Field, PrimeField};
use crate::multiexp;
use crate::pairing::{CurveAffine, CurveProjective, Engine};
use crate::plonk::polynomials::*;
use crate::worker::Worker;
use crate::SynthesisError;
use std::sync::Arc;

pub trait CrsType {}

pub struct CrsForMonomialForm;
pub struct CrsForLagrangeForm;
pub struct CrsForLagrangeFormOnCoset;

impl CrsType for CrsForMonomialForm {}
impl CrsType for CrsForLagrangeForm {}
impl CrsType for CrsForLagrangeFormOnCoset {}

pub struct Crs<E: Engine, T: CrsType, #[cfg(feature = "allocator")] A: std::alloc::Allocator + Default = std::alloc::Global> {
    #[cfg(feature = "allocator")]
    pub g1_bases: Arc<Vec<E::G1Affine, A>>,
    #[cfg(not(feature = "allocator"))]
    pub g1_bases: Arc<Vec<E::G1Affine>>,
    #[cfg(feature = "allocator")]
    pub g2_monomial_bases: Arc<Vec<E::G2Affine, A>>,
    #[cfg(not(feature = "allocator"))]
    pub g2_monomial_bases: Arc<Vec<E::G2Affine>>,

    _marker: std::marker::PhantomData<T>,
}

#[cfg(feature = "allocator")]
impl<E: Engine, T: CrsType, A: std::alloc::Allocator + Default> Crs<E, T, A> {
    pub fn new_in(g1_bases: Vec<E::G1Affine, A>, g2_monomial_bases: Vec<E::G2Affine, A>) -> Self {
        Self {
            g1_bases: Arc::new(g1_bases),
            g2_monomial_bases: Arc::new(g2_monomial_bases),
            _marker: std::marker::PhantomData,
        }
    }
}

use crate::byteorder::BigEndian;
use crate::byteorder::ReadBytesExt;
use crate::byteorder::WriteBytesExt;
use std::io::{Read, Write};

impl<E: Engine, T: CrsType> PartialEq for Crs<E, T> {
    fn eq(&self, other: &Self) -> bool {
        self.g1_bases == other.g1_bases && self.g2_monomial_bases == other.g2_monomial_bases
    }
}

impl<E: Engine, T: CrsType> Eq for Crs<E, T> {}

impl<E: Engine, T: CrsType> Crs<E, T> {
    pub fn new(g1_bases: Vec<E::G1Affine>, g2_monomial_bases: Vec<E::G2Affine>) -> Self {
        Self {
            g1_bases: Arc::new(g1_bases),
            g2_monomial_bases: Arc::new(g2_monomial_bases),
            _marker: std::marker::PhantomData,
        }
    }

    pub fn write<W: Write>(&self, mut writer: W) -> std::io::Result<()> {
        writer.write_u64::<BigEndian>(self.g1_bases.len() as u64)?;
        for g in &self.g1_bases[..] {
            writer.write_all(g.into_uncompressed().as_ref())?;
        }

        writer.write_u64::<BigEndian>(self.g2_monomial_bases.len() as u64)?;
        for g in &self.g2_monomial_bases[..] {
            writer.write_all(g.into_uncompressed().as_ref())?;
        }

        Ok(())
    }

    pub fn read<R: Read>(mut reader: R) -> std::io::Result<Self> {
        use crate::pairing::EncodedPoint;

        let mut g1_repr = <E::G1Affine as CurveAffine>::Uncompressed::empty();
        let mut g2_repr = <E::G2Affine as CurveAffine>::Uncompressed::empty();

        let num_g1 = reader.read_u64::<BigEndian>()?;

        let mut g1_bases = Vec::with_capacity(num_g1 as usize);

        for _ in 0..num_g1 {
            reader.read_exact(g1_repr.as_mut())?;
            let p = g1_repr.into_affine().map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))?;
            g1_bases.push(p);
        }

        let num_g2 = reader.read_u64::<BigEndian>()?;
        assert!(num_g2 == 2u64);

        let mut g2_bases = Vec::with_capacity(num_g2 as usize);

        for _ in 0..num_g2 {
            reader.read_exact(g2_repr.as_mut())?;
            let p = g2_repr.into_affine().map_err(|e| std::io::Error::new(std::io::ErrorKind::InvalidData, e))?;
            g2_bases.push(p);
        }

        let new = Self {
            g1_bases: Arc::new(g1_bases),
            g2_monomial_bases: Arc::new(g2_bases),

            _marker: std::marker::PhantomData,
        };

        Ok(new)
    }
}

impl<E: Engine> Crs<E, CrsForMonomialForm> {
    pub fn dummy_crs(size: usize) -> Self {
        assert!(size.is_power_of_two());

        let g1 = vec![E::G1Affine::one(); size];
        let g2 = vec![E::G2Affine::one(); 2];

        Self {
            g1_bases: Arc::new(g1),
            g2_monomial_bases: Arc::new(g2),
            _marker: std::marker::PhantomData,
        }
    }

    pub fn crs_42(size: usize, worker: &Worker) -> Self {
        // kind of how ceremony would work
        assert!(size.is_power_of_two());
        Self::non_power_of_two_crs_42(size, worker)
    }

    pub fn non_power_of_two_crs_42(size: usize, worker: &Worker) -> Self {
        let mut g2 = vec![E::G2Affine::one(); 2];

        use crate::domain::EvaluationDomain;
        use crate::group::Scalar;
        use crate::pairing::Wnaf;

        let mut coeffs = vec![Scalar::<E>(E::Fr::one()); size];

        {
            let gen = E::Fr::from_str("42").unwrap();

            g2[1] = g2[1].mul(gen.into_repr()).into_affine();

            worker.scope(coeffs.len(), |scope, chunk| {
                for (i, p) in coeffs.chunks_mut(chunk).enumerate() {
                    scope.spawn(move |_| {
                        let mut current_p = gen.pow(&[(i * chunk) as u64]);

                        for p in p.iter_mut() {
                            p.0 = current_p;
                            current_p.mul_assign(&gen);
                        }
                    });
                }
            });
        }

        let mut g1_wnaf = Wnaf::new();
        let g1_wnaf = g1_wnaf.base(E::G1Affine::one().into_projective(), size);

        let mut g1 = vec![E::G1Affine::zero().into_projective(); size];

        worker.scope(g1.len(), |scope, chunk| {
            for (g1, p) in g1.chunks_mut(chunk).zip(coeffs.chunks(chunk)) {
                let mut g1_wnaf = g1_wnaf.shared();
                scope.spawn(move |_| {
                    for (g1, p) in g1.iter_mut().zip(p.iter()) {
                        // Compute final exponent
                        let exp = p.0;

                        // Exponentiate
                        *g1 = g1_wnaf.scalar(exp.into_repr());
                    }

                    // Batch normalize
                    E::G1::batch_normalization(g1);
                });
            }
        });

        let g1: Vec<_> = g1.into_iter().map(|el| el.into_affine()).collect();

        Self {
            g1_bases: Arc::new(g1),
            g2_monomial_bases: Arc::new(g2),
            _marker: std::marker::PhantomData,
        }
    }
}

impl<E: Engine> Crs<E, CrsForLagrangeForm> {
    // Completely invalid, only for testing purposes
    pub fn dummy_crs(size: usize) -> Self {
        assert!(size.is_power_of_two());

        let g1 = vec![E::G1Affine::one(); size];
        let g2 = vec![E::G2Affine::one(); 2];

        Self {
            g1_bases: Arc::new(g1),
            g2_monomial_bases: Arc::new(g2),
            _marker: std::marker::PhantomData,
        }
    }

    pub fn crs_42(size: usize, worker: &Worker) -> Self {
        let tmp = Crs::<E, CrsForMonomialForm>::crs_42(size, &worker);

        Self::from_powers(&tmp, size, &worker)
    }

    pub fn from_powers(powers: &Crs<E, CrsForMonomialForm>, size: usize, worker: &Worker) -> Self {
        assert!(size.is_power_of_two());
        assert!(size <= powers.g1_bases.len());

        let g2 = powers.g2_monomial_bases.as_ref().to_vec();
        let g1 = powers.g1_bases.as_ref()[..size].to_vec();

        let g1 = g1.into_iter().map(|el| Point(el.into_projective())).collect();

        use crate::domain::EvaluationDomain;
        use crate::group::Point;

        let mut g1 = EvaluationDomain::from_coeffs(g1).expect("must fit into the domain");
        g1.transform_powers_of_tau_into_lagrange_basis(&worker);
        let mut g1: Vec<_> = g1.into_coeffs().into_iter().map(|el| el.0).collect();

        worker.scope(g1.len(), |scope, chunk| {
            for g1 in g1.chunks_mut(chunk) {
                scope.spawn(move |_| {
                    // Batch normalize
                    E::G1::batch_normalization(g1);
                });
            }
        });

        let g1: Vec<_> = g1.into_iter().map(|el| el.into_affine()).collect();

        Self {
            g1_bases: Arc::new(g1),
            g2_monomial_bases: Arc::new(g2),
            _marker: std::marker::PhantomData,
        }
    }
}

impl<E: Engine> Crs<E, CrsForLagrangeFormOnCoset> {
    // Completely invalid, only for testing purposes
    pub fn dummy_crs(size: usize) -> Self {
        assert!(size.is_power_of_two());

        let g1 = vec![E::G1Affine::one(); size];
        let g2 = vec![E::G2Affine::one(); 2];

        Self {
            g1_bases: Arc::new(g1),
            g2_monomial_bases: Arc::new(g2),
            _marker: std::marker::PhantomData,
        }
    }

    pub fn crs_42(size: usize, worker: &Worker) -> Self {
        let tmp = Crs::<E, CrsForMonomialForm>::crs_42(size, &worker);

        Self::from_powers(&tmp, size, &worker)
    }

    pub fn from_powers(powers: &Crs<E, CrsForMonomialForm>, size: usize, worker: &Worker) -> Self {
        assert!(size.is_power_of_two());
        assert!(size <= powers.g1_bases.len());

        let g2 = powers.g2_monomial_bases.as_ref().to_vec();
        let g1 = powers.g1_bases.as_ref()[..size].to_vec();

        let g1: Vec<_> = g1.into_iter().map(|el| Point(el.into_projective())).collect();

        use crate::domain::EvaluationDomain;
        use crate::group::Point;

        let mut g1 = EvaluationDomain::from_coeffs(g1).expect("must fit into the domain");

        g1.transform_powers_of_tau_into_lagrange_basis_on_coset(&worker);
        let mut g1: Vec<_> = g1.into_coeffs().into_iter().map(|el| el.0).collect();

        worker.scope(g1.len(), |scope, chunk| {
            for g1 in g1.chunks_mut(chunk) {
                scope.spawn(move |_| {
                    // Batch normalize
                    E::G1::batch_normalization(g1);
                });
            }
        });

        let g1: Vec<_> = g1.into_iter().map(|el| el.into_affine()).collect();

        Self {
            g1_bases: Arc::new(g1),
            g2_monomial_bases: Arc::new(g2),
            _marker: std::marker::PhantomData,
        }
    }
}

pub(crate) fn elements_into_representations<E: Engine>(worker: &Worker, scalars: &[E::Fr]) -> Result<Vec<<E::Fr as PrimeField>::Repr>, SynthesisError> {
    let mut representations = vec![<E::Fr as PrimeField>::Repr::default(); scalars.len()];
    worker.scope(scalars.len(), |scope, chunk| {
        for (scalar, repr) in scalars.chunks(chunk).zip(representations.chunks_mut(chunk)) {
            scope.spawn(move |_| {
                for (scalar, repr) in scalar.iter().zip(repr.iter_mut()) {
                    *repr = scalar.into_repr();
                }
            });
        }
    });

    Ok(representations)
}

pub fn commit_using_monomials<E: Engine>(poly: &Polynomial<E::Fr, Coefficients>, crs: &Crs<E, CrsForMonomialForm>, worker: &Worker) -> Result<E::G1Affine, SynthesisError> {
    let scalars_repr = elements_into_representations::<E>(&worker, &poly.as_ref())?;

    let res = multiexp::dense_multiexp::<E::G1Affine>(&worker, &crs.g1_bases[..scalars_repr.len()], &scalars_repr)?;

    Ok(res.into_affine())
}

pub fn commit_using_values<E: Engine>(poly: &Polynomial<E::Fr, Values>, crs: &Crs<E, CrsForLagrangeForm>, worker: &Worker) -> Result<E::G1Affine, SynthesisError> {
    assert_eq!(poly.size(), crs.g1_bases.len());

    let scalars_repr = elements_into_representations::<E>(&worker, &poly.as_ref())?;

    let res = multiexp::dense_multiexp::<E::G1Affine>(&worker, &crs.g1_bases, &scalars_repr)?;

    Ok(res.into_affine())
}

pub fn commit_using_raw_values<E: Engine>(values: &[E::Fr], crs: &Crs<E, CrsForLagrangeForm>, worker: &Worker) -> Result<E::G1Affine, SynthesisError> {
    assert_eq!(values.len().next_power_of_two(), crs.g1_bases.len());
    let scalars_repr = elements_into_representations::<E>(&worker, &values)?;

    let res = multiexp::dense_multiexp::<E::G1Affine>(&worker, &crs.g1_bases[0..values.len()], &scalars_repr)?;

    Ok(res.into_affine())
}

use crate::source::QueryDensity;

pub fn commit_using_values_with_density<E: Engine, D, Q>(values: &[E::Fr], density: D, crs: &Crs<E, CrsForLagrangeForm>, worker: &Worker) -> Result<E::G1Affine, SynthesisError>
where
    for<'a> &'a Q: QueryDensity,
    D: Send + Sync + 'static + Clone + AsRef<Q>,
{
    use futures::Future;

    // assert_eq!(values.len(), crs.g1_bases.len());
    let scalars_repr = elements_into_representations::<E>(&worker, &values)?;

    // scalars_repr.resize(crs.g1_bases.len(), <E::Fr as PrimeField>::Repr::default());

    let res = multiexp::multiexp(&worker, (crs.g1_bases.clone(), 0), density, Arc::new(scalars_repr)).wait()?;

    Ok(res.into_affine())
}

pub fn commit_using_values_on_coset<E: Engine>(poly: &Polynomial<E::Fr, Values>, crs: &Crs<E, CrsForLagrangeFormOnCoset>, worker: &Worker) -> Result<E::G1Affine, SynthesisError> {
    assert_eq!(poly.size(), crs.g1_bases.len());
    let scalars_repr = elements_into_representations::<E>(&worker, &poly.as_ref())?;

    let res = multiexp::dense_multiexp::<E::G1Affine>(&worker, &crs.g1_bases, &scalars_repr)?;

    Ok(res.into_affine())
}

pub fn calculate_batch_opening_quotient_from_monomials<E: Engine>(
    polys: &[Polynomial<E::Fr, Coefficients>],
    challenges: &[E::Fr],
    at: E::Fr,
    worker: &Worker,
) -> Result<Polynomial<E::Fr, Coefficients>, SynthesisError> {
    assert_eq!(polys.len(), challenges.len());
    assert!(polys.len() > 0);
    let mut tmp = polys[0].clone();
    tmp.scale(worker, challenges[0]);
    for (p, c) in polys[1..].iter().zip(challenges[1..].iter()) {
        tmp.add_assign_scaled(worker, p, c);
    }

    let quotient = divide_single::<E>(tmp.as_ref(), at);

    Polynomial::from_coeffs(quotient)
}

pub fn open_from_monomials<E: Engine>(
    poly: &Polynomial<E::Fr, Coefficients>,
    at: E::Fr,
    _expected_value: E::Fr,
    crs: &Crs<E, CrsForMonomialForm>,
    worker: &Worker,
) -> Result<E::G1Affine, SynthesisError> {
    assert!(poly.size().is_power_of_two());
    let division_result = divide_single::<E>(poly.as_ref(), at);
    assert!(division_result.len().is_power_of_two());
    let division_result = Polynomial::from_coeffs(division_result)?;

    let opening_proof = commit_using_monomials(&division_result, &crs, &worker)?;

    Ok(opening_proof)
}

pub fn open_from_values<E: Engine>(poly: &Polynomial<E::Fr, Values>, at: E::Fr, expected_value: E::Fr, crs: &Crs<E, CrsForLagrangeForm>, worker: &Worker) -> Result<E::G1Affine, SynthesisError> {
    assert!(poly.size().is_power_of_two());
    let division_result = vec![E::Fr::one(); poly.size()];
    let mut division_result = Polynomial::from_values(division_result)?;
    division_result.distribute_powers(&worker, division_result.omega);
    division_result.sub_constant(&worker, &at);
    division_result.batch_inversion(&worker)?;

    worker.scope(division_result.size(), |scope, chunk_size| {
        for (result, values) in division_result.as_mut().chunks_mut(chunk_size).zip(poly.as_ref().chunks(chunk_size)) {
            scope.spawn(move |_| {
                for (r, &val) in result.iter_mut().zip(values.iter()) {
                    let mut tmp = val;
                    tmp.sub_assign(&expected_value);
                    r.mul_assign(&tmp);
                }
            });
        }
    });

    let opening_proof = commit_using_values(&division_result, &crs, &worker)?;

    Ok(opening_proof)
}

pub fn open_from_values_on_coset<E: Engine>(
    poly: &Polynomial<E::Fr, Values>,
    coset_factor: E::Fr,
    at: E::Fr,
    expected_value: E::Fr,
    crs: &Crs<E, CrsForLagrangeFormOnCoset>,
    worker: &Worker,
) -> Result<E::G1Affine, SynthesisError> {
    assert!(poly.size().is_power_of_two());
    let division_result = vec![coset_factor; poly.size()];
    let mut division_result = Polynomial::from_values(division_result)?; // [g, g, g, g, ...]
    division_result.distribute_powers(&worker, division_result.omega); // [g, g*omega, g*omega^2, ...]
    division_result.sub_constant(&worker, &at); // g - z, g*omega - z, g*omega^2 - z, ...]
    division_result.batch_inversion(&worker)?;

    worker.scope(division_result.size(), |scope, chunk_size| {
        for (result, values) in division_result.as_mut().chunks_mut(chunk_size).zip(poly.as_ref().chunks(chunk_size)) {
            scope.spawn(move |_| {
                for (r, &val) in result.iter_mut().zip(values.iter()) {
                    let mut tmp = val;
                    tmp.sub_assign(&expected_value);
                    r.mul_assign(&tmp);
                }
            });
        }
    });

    let opening_proof = commit_using_values_on_coset(&division_result, &crs, &worker)?;

    Ok(opening_proof)
}

pub fn perform_batched_divisor_for_opening<E: Engine>(
    mut polynomials: Vec<Polynomial<E::Fr, Values>>,
    open_at: E::Fr,
    opening_values: &[E::Fr],
    challenge: E::Fr,
    challenge_start: E::Fr,
    worker: &Worker,
) -> Result<(Polynomial<E::Fr, Values>, E::Fr), SynthesisError> {
    assert!(polynomials.len() == opening_values.len(), "different number of polynomials and opening values");
    // assert!(polynomials.len() > 1, "should aggregate only two or more polynomials");

    let size = polynomials[0].size();
    assert!(size.is_power_of_two());

    let common_divisor = vec![E::Fr::one(); size];

    let mut common_divisor = Polynomial::from_values(common_divisor)?;
    common_divisor.distribute_powers(&worker, common_divisor.omega);
    common_divisor.sub_constant(&worker, &open_at);
    common_divisor.batch_inversion(&worker)?;

    for (p, v) in polynomials.iter_mut().zip(opening_values.iter()) {
        assert!(p.size() == size);
        p.sub_constant(&worker, v);
    }

    let rest: Vec<_> = polynomials.drain(1..).collect();

    let mut aggregation = polynomials.pop().expect("one polynomial left");
    if challenge_start != E::Fr::one() {
        aggregation.scale(&worker, challenge);
    }

    let mut this_challenge = challenge_start;
    this_challenge.mul_assign(&challenge);

    for other in rest.into_iter() {
        aggregation.add_assign_scaled(&worker, &other, &this_challenge);
        this_challenge.mul_assign(&challenge);
    }

    aggregation.mul_assign(&worker, &common_divisor);
    drop(common_divisor);

    // return next challenge and aggregation
    Ok((aggregation, this_challenge))
}

pub fn perform_batch_opening_from_values<E: Engine>(
    polynomials: Vec<Polynomial<E::Fr, Values>>,
    crs: &Crs<E, CrsForLagrangeForm>,
    open_at: E::Fr,
    opening_values: &[E::Fr],
    challenge: E::Fr,
    worker: &Worker,
) -> Result<E::G1Affine, SynthesisError> {
    let (aggregation, _) = perform_batched_divisor_for_opening::<E>(polynomials, open_at, opening_values, challenge, E::Fr::one(), &worker)?;

    let opening_proof = commit_using_values(&aggregation, &crs, &worker)?;

    Ok(opening_proof)
}

pub fn is_valid_opening<E: Engine>(commitment: E::G1Affine, z: E::Fr, opening_value: E::Fr, opening_proof: E::G1Affine, g2_by_x: E::G2Affine) -> bool {
    // (f(x) - f(z))/(x - z) = op(x)

    // f(x) = f(z) + op(x) * (x - z)
    // e(f(x) - f(z) + z*op(x), 1) = e(op(x), x)
    // e(f(x) - f(z) + z*op(x), 1) * e(-op(x), x) == 1 // e(0, 0)

    let mut pair_with_1_part = commitment.into_projective();
    let gen_by_opening_value = E::G1Affine::one().mul(opening_value.into_repr());
    let proof_by_z = opening_proof.mul(z.into_repr());

    pair_with_1_part.sub_assign(&gen_by_opening_value);
    pair_with_1_part.add_assign(&proof_by_z);

    let mut pair_with_x_part = opening_proof;
    pair_with_x_part.negate();

    let result = E::final_exponentiation(&E::miller_loop(&[
        (&pair_with_1_part.into_affine().prepare(), &E::G2Affine::one().prepare()),
        (&pair_with_x_part.prepare(), &g2_by_x.prepare()),
    ]));

    if let Some(res) = result {
        return res == E::Fqk::one();
    }

    false
}

pub fn is_valid_multiopening<E: Engine>(commitments: &[E::G1Affine], z: E::Fr, opening_values: &[E::Fr], opening_proof: E::G1Affine, challenge: E::Fr, g2_by_x: E::G2Affine) -> bool {
    assert!(commitments.len() == opening_values.len());
    // \sum_{i} alpha^i (f(x) - f(z))/(x - z) = op(x)

    // \sum_{i} alpha^i (f(x) - f(z)) - op(x) * (x - z) = 0
    // e(\sum_{i} alpha^i (f(x) - f(z)) + z*op(x), 1) = e(op(x), x)
    // e(\sum_{i} alpha^i (f(x) - f(z)) + z*op(x), 1) * e(-op(x), x) == 1 // e(0, 0)

    let mut aggregation = E::G1::zero();

    let mut this_challenge = E::Fr::one();
    // later change for efficiency
    for (c, v) in commitments.iter().zip(opening_values.iter()) {
        let mut pair_with_1_part = c.into_projective();
        let gen_by_opening_value = E::G1Affine::one().mul(v.into_repr());
        pair_with_1_part.sub_assign(&gen_by_opening_value);

        pair_with_1_part.mul_assign(this_challenge.into_repr());
        aggregation.add_assign(&pair_with_1_part);

        this_challenge.mul_assign(&challenge);
    }

    let proof_by_z = opening_proof.mul(z.into_repr());

    aggregation.add_assign(&proof_by_z);

    let mut pair_with_x_part = opening_proof;
    pair_with_x_part.negate();

    let result = E::final_exponentiation(&E::miller_loop(&[
        (&aggregation.into_affine().prepare(), &E::G2Affine::one().prepare()),
        (&pair_with_x_part.prepare(), &g2_by_x.prepare()),
    ]));

    if let Some(res) = result {
        return res == E::Fqk::one();
    }

    false
}

pub(crate) fn divide_single<E: Engine>(poly: &[E::Fr], opening_point: E::Fr) -> Vec<E::Fr> {
    // we are only interested in quotient without a reminder, so we actually don't need opening value
    let mut b = opening_point;
    b.negate();

    let mut q = vec![E::Fr::zero(); poly.len()];

    let mut tmp = E::Fr::zero();
    let mut found_one = false;
    for (q, r) in q.iter_mut().rev().skip(1).zip(poly.iter().rev()) {
        if !found_one {
            if r.is_zero() {
                continue;
            } else {
                found_one = true;
            }
        }

        let mut lead_coeff = *r;
        lead_coeff.sub_assign(&tmp);
        *q = lead_coeff;
        tmp = lead_coeff;
        tmp.mul_assign(&b);
    }

    q
}

pub fn make_crs_from_ignition_transcript<S: AsRef<std::ffi::OsStr> + ?Sized>(path: &S) -> Result<Crs<crate::pairing::bn256::Bn256, CrsForMonomialForm>, SynthesisError> {
    use crate::ff::{PrimeField, PrimeFieldRepr};
    use crate::pairing::bn256::{Bn256, Fq, Fq12, Fq2};
    use crate::pairing::EncodedPoint;
    use std::io::BufRead;

    const CHUNKS: usize = 20;

    let base_path = std::path::Path::new(&path);

    let mut g1_bases = Vec::with_capacity(100800000 + 1);
    g1_bases.push(<Bn256 as Engine>::G1Affine::one());
    let mut g2_bases = vec![<Bn256 as Engine>::G2Affine::one()];

    for i in 0..CHUNKS {
        let full_path = base_path.join(&format!("transcript{:02}.dat", i));
        println!("Opening {}", full_path.to_string_lossy());
        let file = std::fs::File::open(full_path).map_err(|e| SynthesisError::IoError(e))?;
        let mut reader = std::io::BufReader::with_capacity(1 << 24, file);

        // skip 28 bytes
        let mut tmp = [0u8; 28];
        reader.read_exact(&mut tmp).expect("must skip 28 bytes");

        let mut fq_repr = <Fq as PrimeField>::Repr::default();
        let b_coeff = Fq::from_str("3").unwrap();

        fq_repr.as_mut()[0] = 0x3bf938e377b802a8;
        fq_repr.as_mut()[1] = 0x020b1b273633535d;
        fq_repr.as_mut()[2] = 0x26b7edf049755260;
        fq_repr.as_mut()[3] = 0x2514c6324384a86d;

        let c0 = Fq::from_raw_repr(fq_repr).expect("c0 for B coeff for G2");

        fq_repr.as_mut()[0] = 0x38e7ecccd1dcff67;
        fq_repr.as_mut()[1] = 0x65f0b37d93ce0d3e;
        fq_repr.as_mut()[2] = 0xd749d0dd22ac00aa;
        fq_repr.as_mut()[3] = 0x0141b9ce4a688d4d;

        let c1 = Fq::from_raw_repr(fq_repr).expect("c0 for B coeff for G2");

        let b_coeff_fq2 = Fq2 { c0: c0, c1: c1 };

        for _ in 0..5_040_000 {
            // we have to manually read X and Y coordinates
            for k in 0..4 {
                fq_repr.as_mut()[k] = reader.read_u64::<BigEndian>().expect("must read u64");
            }

            let x = Fq::from_repr(fq_repr).expect("must be valid field element encoding");

            for k in 0..4 {
                fq_repr.as_mut()[k] = reader.read_u64::<BigEndian>().expect("must read u64");
            }

            let y = Fq::from_repr(fq_repr).expect("must be valid field element encoding");

            // manual on-curve check
            {
                let mut lhs = y;
                lhs.square();

                let mut rhs = x;
                rhs.square();
                rhs.mul_assign(&x);
                rhs.add_assign(&b_coeff);

                assert!(lhs == rhs);
            }

            let p = <Bn256 as Engine>::G1Affine::from_xy_unchecked(x, y);

            g1_bases.push(p);
        }

        if i == 0 {
            // read G2
            {
                for k in 0..4 {
                    fq_repr.as_mut()[k] = reader.read_u64::<BigEndian>().expect("must read u64");
                }

                let x_c0 = Fq::from_repr(fq_repr).expect("must be valid field element encoding");

                for k in 0..4 {
                    fq_repr.as_mut()[k] = reader.read_u64::<BigEndian>().expect("must read u64");
                }

                let x_c1 = Fq::from_repr(fq_repr).expect("must be valid field element encoding");

                for k in 0..4 {
                    fq_repr.as_mut()[k] = reader.read_u64::<BigEndian>().expect("must read u64");
                }

                let y_c0 = Fq::from_repr(fq_repr).expect("must be valid field element encoding");

                for k in 0..4 {
                    fq_repr.as_mut()[k] = reader.read_u64::<BigEndian>().expect("must read u64");
                }

                let y_c1 = Fq::from_repr(fq_repr).expect("must be valid field element encoding");

                let x = Fq2 { c0: x_c0, c1: x_c1 };

                let y = Fq2 { c0: y_c0, c1: y_c1 };

                {
                    let mut lhs = y;
                    lhs.square();

                    let mut rhs = x;
                    rhs.square();
                    rhs.mul_assign(&x);
                    rhs.add_assign(&b_coeff_fq2);

                    assert!(lhs == rhs);
                }

                let g2 = <Bn256 as Engine>::G2Affine::from_xy_unchecked(x, y);

                g2_bases.push(g2);

                // sanity check by using pairing
                {
                    // check e(g1, g2^x) == e(g1^{x}, g2)
                    let valid = Bn256::final_exponentiation(&Bn256::miller_loop(&[(&g1_bases[0].prepare(), &g2.prepare())])).unwrap()
                        == Bn256::final_exponentiation(&Bn256::miller_loop(&[(&g1_bases[1].prepare(), &g2_bases[0].prepare())])).unwrap();

                    assert!(valid);
                }
            }
            // read G2
            let mut tmp = [0u8; 128];
            reader.read_exact(&mut tmp).expect("must skip 128 bytes of irrelevant G2 point");
        }

        // read to end
        reader.consume(64);

        assert_eq!(reader.fill_buf().unwrap().len(), 0);
    }

    assert_eq!(g1_bases.len(), 100800000 + 1);
    assert_eq!(g2_bases.len(), 2);

    let new = Crs::<crate::pairing::bn256::Bn256, CrsForMonomialForm> {
        g1_bases: Arc::new(g1_bases),
        g2_monomial_bases: Arc::new(g2_bases),

        _marker: std::marker::PhantomData,
    };

    Ok(new)
}

#[cfg(test)]
pub(crate) mod test {
    use super::*;
    use crate::ff::{Field, PrimeField};
    use crate::pairing::bn256::{Bn256, Fr};
    use crate::plonk::polynomials::*;
    use crate::worker::Worker;

    #[test]
    fn test_transformations_of_crs_1() {
        let worker = Worker::new();

        let monomial = Crs::<Bn256, CrsForMonomialForm>::crs_42(1, &worker);
        let lagrange = Crs::<Bn256, CrsForLagrangeForm>::crs_42(1, &worker);
        let lagrange_coset = Crs::<Bn256, CrsForLagrangeFormOnCoset>::crs_42(1, &worker);

        println!("Monomial = {:?}", monomial.g1_bases);
        println!("Lagrange = {:?}", lagrange.g1_bases);
        println!("Lagrange coset = {:?}", lagrange_coset.g1_bases);
    }

    #[test]
    fn test_transformations_of_crs_2() {
        let worker = Worker::new();

        let monomial = Crs::<Bn256, CrsForMonomialForm>::crs_42(2, &worker);
        let lagrange = Crs::<Bn256, CrsForLagrangeForm>::crs_42(2, &worker);
        let lagrange_coset = Crs::<Bn256, CrsForLagrangeFormOnCoset>::crs_42(2, &worker);

        println!("Monomial = {:?}", monomial.g1_bases);
        println!("Lagrange = {:?}", lagrange.g1_bases);
        println!("Lagrange coset = {:?}", lagrange_coset.g1_bases);

        // for a poly in a form a + bx
        // commitment is a + b*tau
        // values on domain are a+b, a-b
        // commitment bases are (1+tau)/2, (1-tau)/2
        // commitment is (a+b)(1+tau)/2 + (a-b)(1-tau)/2 = a/2 + a*tau/2 + b/2 + b*tau/2 + a/2 - a*tau/2 - b/2 + b*tau/2 = a + tau*b
        // valus on coset are a + gen*b, a - gen*b
        // commitment is a*(b_0 + b_1) + gen*b*(b_0 - b_1) = a * tau*b
        // so bases must be b_0 + b_1 = 1 and b_0 - b_1 = tau / gen
        // so b_0 = 1 + tau/gen/2, b_1 = 1 - tau/gen/2

        let one = Fr::one();

        let mut two = Fr::one();
        two.double();

        let poly = Polynomial::<Fr, Coefficients>::from_coeffs(vec![one, two]).unwrap();
        let values = poly.clone().fft(&worker);
        let values_on_coset = poly.clone().coset_fft(&worker);

        let mut tmp = Fr::multiplicative_generator();
        tmp.mul_assign(&two);
        tmp.add_assign(&one);

        assert!(tmp == values_on_coset.as_ref()[0]);

        let commitment = commit_using_monomials(&poly, &monomial, &worker).unwrap();
        let commitment_values = commit_using_values(&values, &lagrange, &worker).unwrap();
        let commitment_values_on_coset = commit_using_values_on_coset(&values_on_coset, &lagrange_coset, &worker).unwrap();

        assert!(commitment == commitment_values);
        assert!(commitment == commitment_values_on_coset);
    }

    #[test]
    fn test_transformations_of_crs_4() {
        let worker = Worker::new();

        let monomial = Crs::<Bn256, CrsForMonomialForm>::crs_42(4, &worker);
        let lagrange = Crs::<Bn256, CrsForLagrangeForm>::crs_42(4, &worker);
        let lagrange_coset = Crs::<Bn256, CrsForLagrangeFormOnCoset>::crs_42(4, &worker);

        let one = Fr::one();

        let mut two = Fr::one();
        two.double();

        let poly = Polynomial::<Fr, Coefficients>::from_coeffs(vec![one, two, one, two]).unwrap();
        let values = poly.clone().fft(&worker);
        let values_on_coset = poly.clone().coset_fft(&worker);

        let commitment = commit_using_monomials(&poly, &monomial, &worker).unwrap();
        let commitment_values = commit_using_values(&values, &lagrange, &worker).unwrap();
        let commitment_values_on_coset = commit_using_values_on_coset(&values_on_coset, &lagrange_coset, &worker).unwrap();

        assert!(commitment == commitment_values);
        assert!(commitment == commitment_values_on_coset);
    }

    #[test]
    fn test_transformations_of_crs_large() {
        let worker = Worker::new();

        let size = 1024;

        let monomial = Crs::<Bn256, CrsForMonomialForm>::crs_42(size, &worker);
        let lagrange = Crs::<Bn256, CrsForLagrangeForm>::crs_42(size, &worker);
        let lagrange_coset = Crs::<Bn256, CrsForLagrangeFormOnCoset>::crs_42(size, &worker);

        let mut two = Fr::one();
        two.double();

        let poly = Polynomial::<Fr, Coefficients>::from_coeffs(vec![two; size]).unwrap();
        let values = poly.clone().fft(&worker);
        let values_on_coset = poly.clone().coset_fft(&worker);

        let commitment = commit_using_monomials(&poly, &monomial, &worker).unwrap();
        let commitment_values = commit_using_values(&values, &lagrange, &worker).unwrap();
        let commitment_values_on_coset = commit_using_values_on_coset(&values_on_coset, &lagrange_coset, &worker).unwrap();

        assert!(commitment == commitment_values);
        assert!(commitment == commitment_values_on_coset);
    }

    #[test]
    fn test_opening_large() {
        let worker = Worker::new();

        let size = 1024;

        let monomial = Crs::<Bn256, CrsForMonomialForm>::crs_42(size, &worker);
        let lagrange = Crs::<Bn256, CrsForLagrangeForm>::crs_42(size, &worker);
        let lagrange_coset = Crs::<Bn256, CrsForLagrangeFormOnCoset>::crs_42(size, &worker);

        let mut two = Fr::one();
        two.double();

        let poly = Polynomial::<Fr, Coefficients>::from_coeffs(vec![two; size]).unwrap();
        let values = poly.clone().fft(&worker);
        let values_on_coset = poly.clone().coset_fft(&worker);

        let z = Fr::from_str("1337").unwrap();

        let poly_at_z = poly.evaluate_at(&worker, z);
        let values_at_z = values.barycentric_evaluate_at(&worker, z).unwrap();
        let valus_on_coset_at_z = values_on_coset.barycentric_over_coset_evaluate_at(&worker, z, &Fr::multiplicative_generator()).unwrap();

        assert!(poly_at_z == values_at_z);
        assert!(poly_at_z == valus_on_coset_at_z);

        let commitment = commit_using_monomials(&poly, &monomial, &worker).unwrap();
        let commitment_values = commit_using_values(&values, &lagrange, &worker).unwrap();
        let commitment_values_on_coset = commit_using_values_on_coset(&values_on_coset, &lagrange_coset, &worker).unwrap();

        assert!(commitment == commitment_values);
        assert!(commitment == commitment_values_on_coset);

        let opening_poly = open_from_monomials(&poly, z, poly_at_z, &monomial, &worker).unwrap();
        let opening_values = open_from_values(&values, z, poly_at_z, &lagrange, &worker).unwrap();
        let opening_values_on_coset = open_from_values_on_coset(&values_on_coset, Fr::multiplicative_generator(), z, poly_at_z, &lagrange_coset, &worker).unwrap();

        assert!(opening_poly == opening_values);
        assert!(opening_poly == opening_values_on_coset);

        let valid = is_valid_opening::<Bn256>(commitment, z, poly_at_z, opening_poly, monomial.g2_monomial_bases[1]);

        assert!(valid);
    }

    #[test]
    #[ignore] // TODO(ignored-test): Failure.
    fn test_open_ignition_setup() {
        let large_setup = make_crs_from_ignition_transcript("/Users/alexvlasov/Downloads/setup").unwrap();
        let base_path = std::path::Path::new("/Users/alexvlasov/Downloads/setup/processed");

        for n in 20..=26 {
            let full_path = base_path.join(&format!("setup_2^{}.key", n));
            println!("Opening {}", full_path.to_string_lossy());
            let file = std::fs::File::create(full_path).unwrap();

            let size = 1 << n;

            let truncated_key = Crs::<Bn256, CrsForMonomialForm> {
                g1_bases: Arc::new(large_setup.g1_bases[..size].to_vec()),
                g2_monomial_bases: large_setup.g2_monomial_bases.clone(),

                _marker: std::marker::PhantomData,
            };

            let mut writer = std::io::BufWriter::with_capacity(1 << 24, file);
            truncated_key.write(&mut writer).unwrap();
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Failure.
    fn transform_ignition_setup() {
        let base_path = std::path::Path::new("/Users/alexvlasov/Downloads/setup/processed");

        let worker = crate::worker::Worker::new();

        for n in 20..=26 {
            let full_path = base_path.join(&format!("setup_2^{}.key", n));
            println!("Opening {}", full_path.to_string_lossy());
            let file = std::fs::File::open(full_path).unwrap();
            let mut reader = std::io::BufReader::with_capacity(1 << 24, file);
            let monomial_form = Crs::<Bn256, CrsForMonomialForm>::read(&mut reader).unwrap();

            let size = 1 << n;

            let lagrange = Crs::<Bn256, CrsForLagrangeForm>::from_powers(&monomial_form, size, &worker);

            let full_path = base_path.join(&format!("setup_2^{}_lagrange.key", n));
            println!("Opening {}", full_path.to_string_lossy());
            let file = std::fs::File::create(full_path).unwrap();
            let mut writer = std::io::BufWriter::with_capacity(1 << 24, file);

            lagrange.write(&mut writer).unwrap();
        }
    }

    #[test]
    fn test_crs_serialization() {
        let worker = Worker::new();
        let mut buffer = Vec::with_capacity(1 << 28);
        let crs = Crs::<Bn256, CrsForMonomialForm>::crs_42(1024, &worker);
        crs.write(&mut buffer).expect("must serialize CRS");

        let new = Crs::<Bn256, CrsForMonomialForm>::read(&buffer[..]).expect("must deserialize CRS");

        assert!(new == crs);
    }

    use rand::Rng;

    pub(crate) fn make_random_field_elements<F: PrimeField>(worker: &Worker, num_elements: usize) -> Vec<F> {
        use rand::{ChaChaRng, Rand, Rng, SeedableRng, XorShiftRng};

        let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);

        make_random_field_elements_for_rng(worker, num_elements, rng)
    }

    pub(crate) fn make_random_field_elements_for_rng<F: PrimeField, R: Rng>(worker: &Worker, num_elements: usize, mut rng: R) -> Vec<F> {
        let mut result = vec![F::zero(); num_elements];

        use rand::{ChaChaRng, Rand, Rng, SeedableRng, XorShiftRng};

        worker.scope(result.len(), |scope, chunk| {
            for r in result.chunks_mut(chunk) {
                let seed: [u32; 4] = rng.gen();
                let subrng = ChaChaRng::from_seed(&seed);
                scope.spawn(move |_| {
                    let mut subrng = subrng;
                    for r in r.iter_mut() {
                        *r = subrng.gen();
                    }
                });
            }
        });

        result
    }

    fn make_random_g1_points<G: CurveAffine>(worker: &Worker, num_elements: usize) -> Vec<G> {
        use rand::{ChaChaRng, Rand, Rng, SeedableRng, XorShiftRng};

        let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);

        make_random_g1_points_for_rng(worker, num_elements, rng)
    }

    fn make_random_g1_points_for_rng<G: CurveAffine, R: Rng>(worker: &Worker, num_elements: usize, mut rng: R) -> Vec<G> {
        let mut result = vec![G::zero(); num_elements];

        use rand::{ChaChaRng, Rand, Rng, SeedableRng, XorShiftRng};

        worker.scope(result.len(), |scope, chunk| {
            for r in result.chunks_mut(chunk) {
                let seed: [u32; 4] = rng.gen();
                let subrng = ChaChaRng::from_seed(&seed);
                scope.spawn(move |_| {
                    let mut subrng = subrng;
                    for r in r.iter_mut() {
                        let p: G::Projective = subrng.gen();
                        *r = p.into_affine();
                    }
                });
            }
        });

        result
    }

    #[test]
    #[ignore]
    fn test_multiexp_performance_on_large_data() {
        use crate::pairing::bn256::{Bn256, Fr};
        use std::time::Instant;

        let max_size = 1 << 26;
        let worker = Worker::new();

        assert!(worker.cpus >= 16, "should be tested only on large machines");
        println!("Generating scalars");
        let scalars = make_random_field_elements::<Fr>(&worker, max_size);
        println!("Generating points");
        let points = make_random_g1_points::<<Bn256 as Engine>::G1Affine>(&worker, max_size);
        println!("Done");

        for size in vec![1 << 23, 1 << 24, 1 << 25, 1 << 26] {
            for cpus in vec![16, 32, 48, 64] {
                // for cpus in vec![16, 24, 32] {
                let s = &scalars[..size];
                let g = &points[..size];

                let subworker = Worker::new_with_cpus(cpus);

                let now = Instant::now();

                // copy-paste, but ok

                let subtime = Instant::now();

                let scalars_repr = super::elements_into_representations::<Bn256>(&subworker, s).unwrap();

                println!("Scalars conversion taken {:?}", subtime.elapsed());

                let subtime = Instant::now();

                let _ = multiexp::dense_multiexp::<<Bn256 as Engine>::G1Affine>(&subworker, g, &scalars_repr).unwrap();

                println!("Multiexp taken {:?}", subtime.elapsed());

                println!("Total time taken for {} points on {} cpus = {:?}", size, cpus, now.elapsed());
            }
        }
    }

    #[test]
    #[ignore]
    fn test_future_based_multiexp_performance_on_large_data() {
        use crate::pairing::bn256::{Bn256, Fr};
        use std::sync::Arc;
        use std::time::Instant;

        let max_size = 1 << 26;
        let worker = Worker::new();

        assert!(worker.cpus >= 16, "should be tested only on large machines");
        println!("Generating scalars");
        let scalars = make_random_field_elements::<Fr>(&worker, max_size);
        println!("Generating points");
        let points = make_random_g1_points::<<Bn256 as Engine>::G1Affine>(&worker, max_size);
        println!("Done");

        for size in vec![1 << 23, 1 << 24, 1 << 25, 1 << 26] {
            for cpus in vec![16, 32, 48, 64] {
                // for cpus in vec![16, 24, 32] {
                let s = &scalars[..size];
                let g = points[..size].to_vec();
                let g = Arc::from(g);

                let subworker = Worker::new_with_cpus(cpus);

                let now = Instant::now();

                // copy-paste, but ok

                let subtime = Instant::now();

                let scalars_repr = super::elements_into_representations::<Bn256>(&subworker, s).unwrap();

                let scalars_repr = Arc::from(scalars_repr);

                println!("Scalars conversion taken {:?}", subtime.elapsed());

                let subtime = Instant::now();

                let _ = multiexp::future_based_multiexp::<<Bn256 as Engine>::G1Affine>(&subworker, Arc::clone(&g), Arc::clone(&scalars_repr)).wait();

                println!("Future based multiexp taken {:?}", subtime.elapsed());

                println!("Total time taken for {} points on {} cpus = {:?}", size, cpus, now.elapsed());
            }
        }
    }

    #[test]
    #[ignore]
    fn test_long_naive_division() {
        use crate::pairing::bn256::{Bn256, Fr};
        use std::time::Instant;

        let max_size = 1 << 26;
        let worker = Worker::new();

        assert!(worker.cpus >= 16, "should be tested only on large machines");
        println!("Generating scalars");
        let scalars = make_random_field_elements::<Fr>(&worker, max_size);
        let divide_at = Fr::from_str("1234567890").unwrap();
        println!("Done");

        for size in vec![1 << 23, 1 << 24, 1 << 25, 1 << 26] {
            let s = &scalars[..size];
            let now = Instant::now();

            let _ = divide_single::<Bn256>(s, divide_at);

            println!("Total time taken for {} points division = {:?}", size, now.elapsed());
        }
    }

    fn serialize_affine_points_for_fpga<E: Engine, W: std::io::Write>(points: &[E::G1Affine], mut dst: W) -> Result<(), std::io::Error> {
        use crate::pairing::ff::PrimeFieldRepr;

        println!("First point = {}", points[0]);
        for p in points.iter() {
            let (x, y) = p.into_xy_unchecked();
            let repr = x.into_raw_repr();
            repr.write_le(&mut dst)?;

            let repr = y.into_raw_repr();
            repr.write_le(&mut dst)?;
        }

        Ok(())
    }

    fn serialize_scalars_for_fpga<E: Engine, W: std::io::Write>(scalars: &[E::Fr], mut dst: W) -> Result<(), std::io::Error> {
        use crate::pairing::ff::PrimeFieldRepr;

        println!("First scalar = {}", scalars[0]);
        for s in scalars.iter() {
            let repr = s.into_repr();
            repr.write_le(&mut dst)?;
        }

        Ok(())
    }

    fn serialize_projective_points_for_fpga<E: Engine, W: std::io::Write>(points: &[E::G1], mut dst: W) -> Result<(), std::io::Error> {
        use crate::pairing::ff::PrimeFieldRepr;

        let (x, y, z) = points[1].into_xyz_unchecked();

        println!("Second bucket (for scalar = 1): X = {}, Y = {}, Z = {}", x, y, z);
        for p in points.iter() {
            let (x, y, z) = p.into_xyz_unchecked();
            let repr = x.into_raw_repr();
            repr.write_le(&mut dst)?;

            let repr = y.into_raw_repr();
            repr.write_le(&mut dst)?;

            let repr = z.into_raw_repr();
            repr.write_le(&mut dst)?;
        }

        Ok(())
    }

    fn simulate_first_buckets<E: Engine>(points: &[E::G1Affine], scalars: &[E::Fr], c: usize, random_point: E::G1Affine) -> Vec<E::G1> {
        use crate::pairing::ff::PrimeFieldRepr;
        use crate::pairing::ff::ScalarEngine;

        let skip = 0;
        let mask = (1u64 << c) - 1u64;
        let p = random_point.into_projective();
        let mut buckets = vec![p; 1 << c];

        for (exp, point) in scalars.iter().zip(points.iter()) {
            let this_exp = exp.into_repr();

            let mut this_exp = this_exp;
            this_exp.shr(skip);
            let this_exp = this_exp.as_ref()[0] & mask;
            buckets[this_exp as usize].add_assign_mixed(point);
        }

        buckets
    }

    fn test_multiexp_bn254(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>) {
        use crate::pairing::bn256::Bn256;
        test_multiexps_inner::<Bn256>(max_size, sizes, num_cpus);
    }

    fn test_multiexp_bn254_compact(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>) {
        use crate::pairing::compact_bn256::Bn256;
        test_multiexps_inner::<Bn256>(max_size, sizes, num_cpus);
    }

    fn test_multiexps_inner<E: Engine>(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>) {
        use std::sync::Arc;
        use std::time::Instant;

        let worker = Worker::new();

        println!("Generating scalars");
        let scalars = make_random_field_elements::<E::Fr>(&worker, max_size);
        println!("Generating points");
        let points = make_random_g1_points::<E::G1Affine>(&worker, max_size);
        println!("Done");

        for size in sizes {
            for &cpus in &num_cpus {
                let s = &scalars[..size];
                let g = points[..size].to_vec();

                let subworker = Worker::new_with_cpus(cpus);

                let scalars_repr = super::elements_into_representations::<E>(&subworker, s).unwrap();

                let subtime = Instant::now();

                let _ = multiexp::dense_multiexp::<E::G1Affine>(&subworker, &g, &scalars_repr).unwrap();

                println!("Dense simple multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::dense_unrolled_multiexp_with_prefetch::<<Bn256 as Engine>::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("Dense unrolled multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::dense_multiexp_uniform::<E::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("Dense uniform multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::stack_allocated_dense_multiexp::<<Bn256 as Engine>::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("Dense stack multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::map_reduce_multiexp::<E::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("Map reduce multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::producer_consumer_dense_multiexp::<<Bn256 as Engine>::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("ProdCons stack multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::stack_allocated_uncompensated_dense_multiexp::<<Bn256 as Engine>::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("Uncompensated stack multiexp of size {} taken {:?} on {} cpus", size, subtime.elapsed(), cpus);

                // let subtime = Instant::now();

                // let _ = multiexp::dense_multiexp_with_manual_unrolling::<<Bn256 as Engine>::G1Affine>(
                //     &subworker,
                //     &g,
                //     &scalars_repr
                // ).unwrap();

                // println!("Dense manually unrolled multiexp taken {:?} on {} cpus", subtime.elapsed(), cpus);

                // let g = Arc::from(g);
                // let scalars_repr = Arc::from(scalars_repr);

                // let subtime = Instant::now();

                // let _ = multiexp::future_based_multiexp::<<Bn256 as Engine>::G1Affine>(
                //     &subworker,
                //     Arc::clone(&g),
                //     Arc::clone(&scalars_repr)
                // ).wait();

                // println!("Future based multiexp taken {:?} on {} cpus", subtime.elapsed(), cpus);
            }
        }
    }

    // fn test_multiexps_over_window_sizes_bn254(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>) {
    //     use crate::pairing::bn256::Bn256;
    //     test_multiexps_over_window_sizes::<Bn256>(max_size, sizes, num_cpus, windows);
    // }

    // fn test_multiexps_over_window_sizes_bn254_compact(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>) {
    //     use crate::pairing::compact_bn256::Bn256;
    //     test_multiexps_over_window_sizes::<Bn256>(max_size, sizes, num_cpus, windows);
    // }

    // fn test_multiexps_over_window_sizes<E: Engine>(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>) {
    //     use std::time::Instant;
    //     use std::sync::Arc;

    //     let worker = Worker::new();

    //     println!("Generating scalars");
    //     let scalars = make_random_field_elements::<E::Fr>(&worker, max_size);
    //     println!("Generating points");
    //     let points = make_random_g1_points::<E::G1Affine>(&worker, max_size);
    //     println!("Done");

    //     for size in sizes {
    //         for &cpus in &num_cpus {
    //             let mut subresults = vec![];
    //             for &window in &windows {
    //                 let s = &scalars[..size];
    //                 let g = points[..size].to_vec();

    //                 let subworker = Worker::new_with_cpus(cpus);

    //                 let scalars_repr = super::elements_into_representations::<E>(
    //                     &subworker,
    //                     s
    //                 ).unwrap();

    //                 let subtime = Instant::now();

    //                 let window = window as u32;

    //                 let _ = multiexp::map_reduce_multiexp_over_fixed_window::<E::G1Affine>(
    //                     &subworker,
    //                     &g,
    //                     &scalars_repr,
    //                     window
    //                 ).unwrap();

    //                 subresults.push((window, subtime.elapsed().as_millis()));

    //                 // println!("Map reduce multiexp of size {} taken {:?} on {} cpus with window size = {}", size, subtime.elapsed(), cpus, window);
    //             }

    //             subresults.sort_by(|a, b| {
    //                 a.1.cmp(&b.1)
    //             });

    //             println!("Map reduce multiexp of size {} on {} CPUs:", size, cpus);
    //             for (window, time_ms) in &subresults[0..3] {
    //                 println!("Window = {}, time = {} ms", window, time_ms);
    //             }
    //         }
    //     }
    // }

    // fn test_buffered_multiexps_bn254_compact(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>, buffer_sizes: Vec<usize>) {
    //     use crate::pairing::compact_bn256::Bn256;
    //     test_buffered_multiexp::<Bn256>(max_size, sizes, num_cpus, windows, buffer_sizes);
    // }

    // fn test_buffered_multiexp<E: Engine>(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>, buffer_sizes: Vec<usize>) {
    //     use std::time::Instant;
    //     use std::sync::Arc;

    //     let worker = Worker::new();

    //     println!("Generating scalars");
    //     let scalars = make_random_field_elements::<E::Fr>(&worker, max_size);
    //     println!("Generating points");
    //     let points = make_random_g1_points::<E::G1Affine>(&worker, max_size);
    //     println!("Done");

    //     for size in sizes {
    //         for &cpus in &num_cpus {
    //             for &buffer_size in &buffer_sizes {
    //                 let mut subresults = vec![];
    //                 for &window in &windows {
    //                     let s = &scalars[..size];
    //                     let g = points[..size].to_vec();

    //                     let subworker = Worker::new_with_cpus(cpus);

    //                     let scalars_repr = super::elements_into_representations::<E>(
    //                         &subworker,
    //                         s
    //                     ).unwrap();

    //                     let subtime = Instant::now();

    //                     let window = window as u32;

    //                     let _ = multiexp::buffered_multiexp_over_fixed_window_and_buffer_size::<E::G1Affine>(
    //                         &subworker,
    //                         &g,
    //                         &scalars_repr,
    //                         window,
    //                         buffer_size
    //                     ).unwrap();

    //                     subresults.push((window, subtime.elapsed().as_millis()));

    //                     // println!("Map reduce multiexp of size {} taken {:?} on {} cpus with window size = {}", size, subtime.elapsed(), cpus, window);
    //                 }

    //                 subresults.sort_by(|a, b| {
    //                     a.1.cmp(&b.1)
    //                 });

    //                 println!("Map reduce multiexp of size {} on {} CPUs with buffer size {}:", size, cpus, buffer_size);
    //                 for (window, time_ms) in &subresults[0..3] {
    //                     println!("Window = {}, time = {} ms", window, time_ms);
    //                 }
    //             }
    //         }
    //     }
    // }

    // fn test_future_based_multiexps_over_window_sizes_bn254_compact(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>) {
    //     use crate::pairing::compact_bn256::Bn256;
    //     test_future_based_multiexps_over_window_sizes::<Bn256>(max_size, sizes, num_cpus, windows);
    // }

    // fn test_future_based_multiexps_over_window_sizes_bn254(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>) {
    //     use crate::pairing::bn256::Bn256;
    //     test_future_based_multiexps_over_window_sizes::<Bn256>(max_size, sizes, num_cpus, windows);
    // }

    // #[ignore]
    // #[test]
    // fn test_optimal_bn254_multiexp() {
    //     // use crate::pairing::bn256::Bn256;
    //     use crate::pairing::compact_bn256::Bn256;
    //     test_optimal_multiexp::<Bn256>(2, 1 << 24, 24, 12, true);
    //     test_optimal_multiexp::<Bn256>(2, 1 << 24, 24, 12, false);
    //     test_optimal_multiexp::<Bn256>(2, 1 << 25, 24, 11, true);
    //     test_optimal_multiexp::<Bn256>(2, 1 << 25, 24, 11, false);
    // }

    // fn test_optimal_multiexp<E: Engine>(max_parallel_jobs: usize, max_size: usize, cpus_per_job: usize, window: usize, same_base: bool) {
    //     use futures::executor::block_on;
    //     use futures::future::join_all;

    //     use std::time::Instant;
    //     use std::sync::Arc;
    //     use crate::source::FullDensity;

    //     let mut bases = vec![];
    //     let mut scalars = vec![];
    //     let worker = Worker::new();

    //     assert!(max_parallel_jobs >= 1);

    //     use rand::{XorShiftRng, SeedableRng, Rand, Rng, ChaChaRng};

    //     let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);

    //     for _ in 0..max_parallel_jobs {
    //         let seed: [u32; 4] = rng.gen();
    //         let mut subrng = ChaChaRng::from_seed(&seed);

    //         let sc = make_random_field_elements_for_rng::<E::Fr, _>(&worker, max_size, &mut subrng);
    //         let p = make_random_g1_points_for_rng::<E::G1Affine, _>(&worker, max_size, &mut subrng);
    //         let s = super::elements_into_representations::<E>(
    //             &worker,
    //             &sc
    //         ).unwrap();

    //         bases.push(Arc::from(p));
    //         scalars.push(Arc::from(s));
    //     }

    //     for num_jobs in 1..=max_parallel_jobs {
    //         let mut jobs = vec![];
    //         let subworker = Worker::new_with_cpus(cpus_per_job * num_jobs);
    //         let subtime = Instant::now();
    //         let window = window as u32;

    //         for idx in 0..num_jobs {
    //             let id = if same_base {
    //                 0
    //             } else {
    //                 idx
    //             };
    //             let p = Arc::clone(&bases[id]);
    //             let s = Arc::clone(&scalars[idx]);

    //             let job = multiexp::future_based_dense_multiexp_over_fixed_width_windows(
    //                 &subworker,
    //                 p,
    //                 s,
    //                 window
    //             );

    //             // let job = multiexp::multiexp_with_fixed_width::<_, _, _, _>(
    //             //     &subworker,
    //             //     (p, 0),
    //             //     FullDensity,
    //             //     s,
    //             //     window
    //             // );

    //             jobs.push(job);
    //         }

    //         let joiner = join_all(jobs);

    //         let _ = block_on(joiner);

    //         let elapsed = subtime.elapsed();
    //         if same_base {
    //             print!("For same bases: ");
    //         } else {
    //             print!("For different bases: ");
    //         }
    //         println!("{} jobs of size {} with {} CPUs per job and {} bits window taken {:?}", num_jobs, max_size, cpus_per_job, window, elapsed);
    //     }
    // }

    // #[ignore]
    // #[test]
    // fn test_l3_shared_multiexp_bn254() {
    //     // use crate::pairing::bn256::Bn256;
    //     use crate::pairing::compact_bn256::Bn256;
    //     test_l3_shared_multiexp::<Bn256>(4, 1 << 24, 24, 12);
    //     test_l3_shared_multiexp::<Bn256>(4, 1 << 25, 24, 12);
    //     test_optimal_multiexp::<Bn256>(1, 1 << 24, 24, 12, true);
    //     test_optimal_multiexp::<Bn256>(1, 1 << 25, 24, 12, true);
    //     test_optimal_multiexp::<Bn256>(1, 1 << 24, 16, 16, true);
    //     test_optimal_multiexp::<Bn256>(1, 1 << 25, 16, 16, true);
    //     test_optimal_multiexp::<Bn256>(2, 1 << 24, 24, 12, true);
    //     test_optimal_multiexp::<Bn256>(2, 1 << 25, 24, 12, true);
    //     test_optimal_multiexp::<Bn256>(3, 1 << 24, 16, 16, true);
    //     test_optimal_multiexp::<Bn256>(3, 1 << 25, 16, 16, true);
    //     test_optimal_multiexp::<Bn256>(4, 1 << 24, 16, 16, true);
    //     test_optimal_multiexp::<Bn256>(4, 1 << 25, 16, 16, true);
    // }

    // fn test_l3_shared_multiexp<E: Engine>(max_parallel_jobs: usize, max_size: usize, cpus_per_job: usize, window: usize) {
    //     use std::time::Instant;

    //     let mut bases = vec![];
    //     let mut scalars = vec![];
    //     let worker = Worker::new();

    //     use rand::{XorShiftRng, SeedableRng, Rand, Rng, ChaChaRng};

    //     let rng = &mut XorShiftRng::from_seed([0x3dbe6259, 0x8d313d76, 0x3237db17, 0xe5bc0654]);

    //     for _ in 0..max_parallel_jobs {
    //         let seed: [u32; 4] = rng.gen();
    //         let mut subrng = ChaChaRng::from_seed(&seed);

    //         let sc = make_random_field_elements_for_rng::<E::Fr, _>(&worker, max_size, &mut subrng);
    //         let p = make_random_g1_points_for_rng::<E::G1Affine, _>(&worker, max_size, &mut subrng);
    //         let s = super::elements_into_representations::<E>(
    //             &worker,
    //             &sc
    //         ).unwrap();

    //         bases.push(p);
    //         scalars.push(s);
    //     }

    //     for j in 1..=max_parallel_jobs {
    //         let subworker = Worker::new_with_cpus(cpus_per_job * j);

    //         let subtime = Instant::now();

    //         let mut exps = vec![];
    //         for i in 0..j {
    //             exps.push(&scalars[i][..]);
    //         }

    //         println!("Running for {} parallel job", j);
    //         let _ = multiexp::l3_shared_multexp(
    //             &subworker,
    //             &bases[0][..],
    //             &exps[..],
    //         ).unwrap();

    //         let elapsed = subtime.elapsed();

    //         println!("L3 shared multiexp for {} jobs of size {} with {} CPUs per job and {} bits window taken {:?}", j, max_size, cpus_per_job, window, elapsed);
    //     }
    // }

    fn test_future_based_multiexps_over_window_sizes<E: Engine>(max_size: usize, sizes: Vec<usize>, num_cpus: Vec<usize>, windows: Vec<usize>) {
        use crate::source::FullDensity;
        use std::sync::Arc;
        use std::time::Instant;

        let worker = Worker::new();

        println!("Generating scalars");
        let scalars = make_random_field_elements::<E::Fr>(&worker, max_size);
        println!("Generating points");
        let points = make_random_g1_points::<E::G1Affine>(&worker, max_size);
        println!("Done");

        for size in sizes {
            for &cpus in &num_cpus {
                let mut subresults = vec![];
                let mut alt_subresults = vec![];

                let s = &scalars[..size];
                let g = points[..size].to_vec();

                let scalars_repr = super::elements_into_representations::<E>(&worker, s).unwrap();

                let g = Arc::from(g);
                let s = Arc::from(scalars_repr);

                for &window in &windows {
                    let subworker = Worker::new_with_cpus(cpus);

                    let subtime = Instant::now();

                    let window = window as u32;

                    let _ = multiexp::future_based_dense_multiexp_over_fixed_width_windows(&subworker, Arc::clone(&g), Arc::clone(&s), window).wait();

                    alt_subresults.push((window, subtime.elapsed().as_millis()));

                    let subtime = Instant::now();

                    let _ = multiexp::multiexp_with_fixed_width::<_, _, _, _>(&subworker, (Arc::clone(&g), 0), FullDensity, Arc::clone(&s), window).wait();

                    subresults.push((window, subtime.elapsed().as_millis()));
                }

                subresults.sort_by(|a, b| a.1.cmp(&b.1));

                alt_subresults.sort_by(|a, b| a.1.cmp(&b.1));

                println!("Standard future based multiexp of size {} on {} CPUs:", size, cpus);
                for (window, time_ms) in &subresults[0..3] {
                    println!("Window = {}, time = {} ms", window, time_ms);
                }

                println!("Tuned future based multiexp of size {} on {} CPUs:", size, cpus);
                for (window, time_ms) in &alt_subresults[0..3] {
                    println!("Window = {}, time = {} ms", window, time_ms);
                }
            }
        }
    }

    #[test]
    #[ignore]
    fn test_different_multiexps() {
        test_multiexp_bn254(1 << 20, vec![1 << 20], vec![3, 4, 6]);
    }

    #[test]
    #[ignore]
    fn test_large_data_different_multiexps() {
        let max_size = 1 << 26;
        let worker = Worker::new();

        assert!(worker.cpus >= 16, "should be tested only on large machines");

        let sizes = vec![1 << 23, 1 << 24, 1 << 25, 1 << 26];
        let cpus = vec![8, 12, 16, 24, 32, 48];
        // test_multiexp_bn254(max_size, sizes, cpus);
        test_multiexp_bn254_compact(max_size, sizes, cpus);
    }

    // #[test]
    // #[ignore]
    // fn test_small_data_different_windows() {
    //     let max_size = 1 << 20;

    //     let sizes = vec![1 << 16, 1 << 17, 1 << 18, 1 << 19, 1 << 20];
    //     let cpus = vec![3, 4, 6];
    //     let windows = vec![7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
    //     test_multiexps_over_window_sizes_bn254(max_size, sizes, cpus, windows);
    // }

    // #[test]
    // #[ignore]
    // fn test_large_data_different_windows_multiexp() {
    //     let max_size = 1 << 26;
    //     let worker = Worker::new();

    //     assert!(worker.cpus >= 16, "should be tested only on large machines");

    //     let sizes = vec![1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26];
    //     let cpus = vec![8, 12, 16, 24, 32, 48];
    //     let windows = vec![7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
    //     // test_multiexps_over_window_sizes_bn254(max_size, sizes, cpus, windows);
    //     test_multiexps_over_window_sizes_bn254_compact(max_size, sizes, cpus, windows);
    // }

    // #[test]
    // #[ignore]
    // fn test_large_data_buffered_multiexp() {
    //     let max_size = 1 << 26;
    //     let worker = Worker::new();

    //     assert!(worker.cpus >= 16, "should be tested only on large machines");

    //     let sizes = vec![1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26];
    //     let cpus = vec![8, 12, 16, 24, 32, 48];
    //     let windows = vec![10, 11, 12, 13, 14, 15, 16];
    //     let buffer_sizes = vec![4, 8, 16, 32, 64, 128];
    //     // test_multiexps_over_window_sizes_bn254(max_size, sizes, cpus, windows);
    //     test_buffered_multiexps_bn254_compact(max_size, sizes, cpus, windows, buffer_sizes);
    // }

    // #[test]
    // #[ignore]
    // fn future_based_test_large_data_different_windows() {
    //     let max_size = 1 << 26;
    //     let worker = Worker::new();

    //     assert!(worker.cpus >= 16, "should be tested only on large machines");

    //     let sizes = vec![1 << 20, 1 << 21, 1 << 22, 1 << 23, 1 << 24, 1 << 25, 1 << 26];
    //     let cpus = vec![8, 12, 16, 24, 32, 48];
    //     let windows = vec![7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
    //     // test_future_based_multiexps_over_window_sizes_bn254(max_size, sizes, cpus, windows);
    //     test_future_based_multiexps_over_window_sizes_bn254_compact(max_size, sizes, cpus, windows);
    // }

    fn make_random_points_with_unknown_discrete_log<E: Engine>(dst: &[u8], seed: &[u8], num_points: usize) -> Vec<E::G1Affine> {
        let mut result = vec![];

        use rand::chacha::ChaChaRng;
        use rand::{Rng, SeedableRng};
        // Create an RNG based on the outcome of the random beacon
        let mut rng = {
            // if we use Blake hasher
            let input: Vec<u8> = dst.iter().chain(seed.iter()).cloned().collect();
            let h = blake2s_simd::blake2s(&input);
            assert!(h.as_bytes().len() == 32);
            let mut seed = [0u32; 8];
            for (i, chunk) in h.as_bytes().chunks_exact(8).enumerate() {
                seed[i] = (&chunk[..]).read_u32::<BigEndian>().expect("digest is large enough for this to work");
            }

            ChaChaRng::from_seed(&seed)
        };

        for _ in 0..num_points {
            let point: E::G1 = rng.gen();

            result.push(point.into_affine());
        }

        result
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn produce_fpga_test_vectors() {
        use crate::pairing::bls12_381::Bls12;
        use crate::pairing::ff::ScalarEngine;

        let worker = crate::worker::Worker::new();

        let random_point = make_random_points_with_unknown_discrete_log::<Bls12>(&b"fpga_dst"[..], &hex::decode(crate::constants::ETH_BLOCK_10_000_000_HASH).unwrap(), 1)[0];

        let (x, y) = random_point.into_xy_unchecked();
        println!("Random point in Montgomery form: X = {}, Y = {}", x.into_raw_repr(), y.into_raw_repr());

        let base_path = std::path::Path::new("./");

        for n in vec![6, 7, 20] {
            let points_path = base_path.join(&format!("input_points_2^{}.key", n));
            let scalars_path = base_path.join(&format!("input_scalars_2^{}.key", n));
            let buckets_path = base_path.join(&format!("output_buckets_2^{}.key", n));

            println!("Opening {}", points_path.to_string_lossy());

            let file = std::fs::File::create(points_path).unwrap();
            let mut points_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(scalars_path).unwrap();
            let mut scalars_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(buckets_path).unwrap();
            let mut buckets_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let size = 1 << n;

            let scalars = make_random_field_elements::<<Bls12 as ScalarEngine>::Fr>(&worker, size);
            let points = make_random_g1_points::<<Bls12 as Engine>::G1Affine>(&worker, size);
            let buckets = simulate_first_buckets::<Bls12>(&points, &scalars, 13, random_point);

            serialize_affine_points_for_fpga::<Bls12, _>(&points, &mut points_file).unwrap();
            serialize_scalars_for_fpga::<Bls12, _>(&scalars, &mut scalars_file).unwrap();
            serialize_projective_points_for_fpga::<Bls12, _>(&buckets, &mut buckets_file).unwrap();
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn produce_bn254_fpga_test_vectors() {
        use crate::pairing::bn256::{Bn256, Fr};
        use crate::pairing::ff::ScalarEngine;

        let bucket_width = 16;
        let worker = crate::worker::Worker::new();

        let random_point = make_random_points_with_unknown_discrete_log::<Bn256>(&b"fpga_dst"[..], &hex::decode(crate::constants::ETH_BLOCK_10_000_000_HASH).unwrap(), 1)[0];

        let (x, y) = random_point.into_xy_unchecked();
        println!("Random point in Montgomery form: X = {}, Y = {}", x.into_raw_repr(), y.into_raw_repr());

        let base_path = std::path::Path::new("./");

        let mut num_buckets = (Fr::NUM_BITS as usize) / bucket_width;
        if (Fr::NUM_BITS as usize) % bucket_width != 0 {
            num_buckets += 1;
        }

        for n in vec![6, 7, 20] {
            let points_path = base_path.join(&format!("bn_254_input_points_2^{}_width_{}.key", n, bucket_width));
            let scalars_path = base_path.join(&format!("bn_254_input_scalars_2^{}_width_{}.key", n, bucket_width));
            let initial_buckets_path = base_path.join(&format!("bn_254_input_buckets_2^{}_width_{}.key", n, bucket_width));
            let buckets_path = base_path.join(&format!("bn_254_output_buckets_2^{}_width_{}.key", n, bucket_width));

            println!("Opening {}", points_path.to_string_lossy());

            let file = std::fs::File::create(points_path).unwrap();
            let mut points_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(scalars_path).unwrap();
            let mut scalars_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(initial_buckets_path).unwrap();
            let mut initial_buckets_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(buckets_path).unwrap();
            let mut buckets_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let size = 1 << n;

            let scalars = make_random_field_elements::<<Bn256 as ScalarEngine>::Fr>(&worker, size);
            let points = make_random_g1_points::<<Bn256 as Engine>::G1Affine>(&worker, size);
            let initial_buckets = vec![random_point.into_projective(); num_buckets * (1 << bucket_width)];
            let buckets = simulate_first_buckets::<Bn256>(&points, &scalars, bucket_width, random_point);

            serialize_affine_points_for_fpga::<Bn256, _>(&points, &mut points_file).unwrap();
            serialize_scalars_for_fpga::<Bn256, _>(&scalars, &mut scalars_file).unwrap();
            serialize_projective_points_for_fpga::<Bn256, _>(&initial_buckets, &mut initial_buckets_file).unwrap();
            serialize_projective_points_for_fpga::<Bn256, _>(&buckets, &mut buckets_file).unwrap();
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn produce_fpga_window_12_test_vectors() {
        let width = 12;
        use crate::pairing::bls12_381::Bls12;
        use crate::pairing::ff::ScalarEngine;

        let worker = crate::worker::Worker::new();

        let random_point = make_random_points_with_unknown_discrete_log::<Bls12>(&b"fpga_dst"[..], &hex::decode(crate::constants::ETH_BLOCK_10_000_000_HASH).unwrap(), 1)[0];

        let (x, y) = random_point.into_xy_unchecked();
        println!("Random point in Montgomery form: X = {}, Y = {}", x.into_raw_repr(), y.into_raw_repr());

        let base_path = std::path::Path::new("./");

        for n in vec![6, 7, 20] {
            let points_path = base_path.join(&format!("input_points_2^{}.key", n));
            let scalars_path = base_path.join(&format!("input_scalars_2^{}.key", n));
            let buckets_path = base_path.join(&format!("width_{}_output_buckets_2^{}.key", width, n));

            println!("Opening {}", points_path.to_string_lossy());

            let file = std::fs::File::create(points_path).unwrap();
            let mut points_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(scalars_path).unwrap();
            let mut scalars_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let file = std::fs::File::create(buckets_path).unwrap();
            let mut buckets_file = std::io::BufWriter::with_capacity(1 << 24, file);

            let size = 1 << n;

            let scalars = make_random_field_elements::<<Bls12 as ScalarEngine>::Fr>(&worker, size);
            let points = make_random_g1_points::<<Bls12 as Engine>::G1Affine>(&worker, size);
            let buckets = simulate_first_buckets::<Bls12>(&points, &scalars, width, random_point);

            serialize_affine_points_for_fpga::<Bls12, _>(&points, &mut points_file).unwrap();
            serialize_scalars_for_fpga::<Bls12, _>(&scalars, &mut scalars_file).unwrap();
            serialize_projective_points_for_fpga::<Bls12, _>(&buckets, &mut buckets_file).unwrap();
        }
    }
}
