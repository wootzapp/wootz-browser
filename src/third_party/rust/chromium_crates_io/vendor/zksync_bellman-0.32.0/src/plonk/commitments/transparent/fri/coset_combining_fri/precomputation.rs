use crate::ff::PrimeField;

use super::*;
use crate::plonk::domains::Domain;
use crate::plonk::fft::cooley_tukey_ntt::OmegasInvBitreversed;
use crate::plonk::fft::cooley_tukey_ntt::{bitreverse, log2_floor};
use crate::plonk::fft::distribute_powers;
use crate::worker::Worker;

impl<F: PrimeField> FriPrecomputations<F> for OmegasInvBitreversed<F> {
    fn new_for_domain_size(size: usize) -> Self {
        let domain = Domain::<F>::new_for_size(size as u64).expect("domain must exist");
        let worker = Worker::new();
        Self::new_for_domain(&domain, &worker)
    }

    fn omegas_inv_bitreversed(&self) -> &[F] {
        &self.omegas[..]
    }

    fn domain_size(&self) -> usize {
        self.domain_size
    }
}

pub struct CosetOmegasInvBitreversed<F: PrimeField> {
    pub omegas: Vec<F>,
    domain_size: usize,
}

impl<F: PrimeField> CosetOmegasInvBitreversed<F> {
    pub fn new_for_domain(domain: &Domain<F>, worker: &Worker) -> Self {
        let domain_size = domain.size as usize;

        let omega = domain.generator.inverse().expect("must exist");
        let precomputation_size = domain_size / 2;

        let log_n = log2_floor(precomputation_size);

        let mut omegas = vec![F::zero(); precomputation_size];
        let geninv = F::multiplicative_generator().inverse().expect("must exist");

        worker.scope(omegas.len(), |scope, chunk| {
            for (i, v) in omegas.chunks_mut(chunk).enumerate() {
                scope.spawn(move |_| {
                    let mut u = omega.pow(&[(i * chunk) as u64]);
                    u.mul_assign(&geninv);
                    for v in v.iter_mut() {
                        *v = u;
                        u.mul_assign(&omega);
                    }
                });
            }
        });

        if omegas.len() > 2 {
            for k in 0..omegas.len() {
                let rk = bitreverse(k, log_n as usize);
                if k < rk {
                    omegas.swap(rk, k);
                }
            }
        }

        CosetOmegasInvBitreversed { omegas, domain_size }
    }
}

impl<F: PrimeField> FriPrecomputations<F> for CosetOmegasInvBitreversed<F> {
    fn new_for_domain_size(size: usize) -> Self {
        let domain = Domain::<F>::new_for_size(size as u64).expect("domain must exist");
        let worker = Worker::new();
        Self::new_for_domain(&domain, &worker)
    }

    fn omegas_inv_bitreversed(&self) -> &[F] {
        &self.omegas[..]
    }

    fn domain_size(&self) -> usize {
        self.domain_size
    }
}
