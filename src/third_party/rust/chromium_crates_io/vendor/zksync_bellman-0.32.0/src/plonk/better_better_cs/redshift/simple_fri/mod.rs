// pub mod fri;
// pub mod query_producer;
// pub mod verifier;
// pub mod precomputation;

use crate::pairing::ff::{Field, PrimeField};
use crate::pairing::Engine;
use crate::plonk::commitments::transcript::*;
use crate::worker::Worker;
use crate::SynthesisError;

use crate::plonk::better_better_cs::redshift::binary_tree::*;
use crate::plonk::better_better_cs::redshift::tree_hash::*;
use crate::plonk::commitments::transcript::Prng;
use crate::plonk::fft::cooley_tukey_ntt::*;
use crate::plonk::polynomials::*;

pub struct FriCombiner<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    precomputations: OmegasInvBitreversed<E::Fr>,
    fri_domain_size: usize,
    lde_factor: usize,
    output_coeffs_at_degree_plus_one: usize,
    folding_schedule: Vec<usize>,
    tree_hasher: H,
    optimal_values_per_leaf: usize,
    coset_factor: E::Fr,
}

pub struct FriOraclesSet<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    pub intermediate_oracles: Vec<BinaryTree<E, H>>,
    pub intermediate_roots: Vec<H::Output>,
    pub intermediate_leaf_values: Vec<Vec<E::Fr>>,
    pub intermediate_challenges: Vec<Vec<E::Fr>>,
    pub final_coefficients: Vec<E::Fr>,
}

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> FriCombiner<E, H> {
    pub fn initialize_for_domain_size(size: usize, lde_factor: usize, output_coeffs_at_degree_plus_one: usize, coset_factor: E::Fr, optimal_values_per_leaf: usize, hasher: H) -> Self {
        assert!(output_coeffs_at_degree_plus_one.is_power_of_two());
        assert!(lde_factor.is_power_of_two());

        let precomputations = OmegasInvBitreversed::new_for_domain_size(size);
        let mut schedule = vec![];

        let folding_factor = size / lde_factor / output_coeffs_at_degree_plus_one;

        assert!(folding_factor.is_power_of_two());

        let mut size_left = log2_floor(folding_factor) as usize;

        let base = log2_floor(optimal_values_per_leaf) as usize;

        while size_left >= base {
            size_left -= base;
            schedule.push(base);
        }

        if size_left != 0 {
            schedule.push(size_left)
        }

        Self {
            precomputations,
            fri_domain_size: size,
            lde_factor,
            output_coeffs_at_degree_plus_one,
            folding_schedule: schedule,
            tree_hasher: hasher,
            optimal_values_per_leaf,
            coset_factor,
        }
    }

    pub fn perform_fri_assuming_bitreversed<P: Prng<E::Fr, Input = H::Output>>(&self, lde_values: &[E::Fr], prng: &mut P, worker: &Worker) -> Result<FriOraclesSet<E, H>, SynthesisError> {
        let mut coset_schedule_index = 0;
        let coset_factor = self.folding_schedule[coset_schedule_index];

        let mut total_wrap_factor = 1;
        for s in self.folding_schedule.iter() {
            let coeff = 1 << *s;
            total_wrap_factor *= coeff;
        }

        let initial_domain_size = lde_values.len();

        assert_eq!(self.precomputations.domain_size(), initial_domain_size);

        let mut two = E::Fr::one();
        two.double();
        let two_inv = two.inverse().expect("should exist");

        let initial_degree_plus_one = initial_domain_size / self.lde_factor;
        assert_eq!(
            initial_degree_plus_one / total_wrap_factor,
            self.output_coeffs_at_degree_plus_one,
            "number of FRI round does not match the ouput degree: \
            initial degree+1 =  {}, wrapping factor {}, output at degree+1 = {}",
            initial_degree_plus_one,
            total_wrap_factor,
            self.output_coeffs_at_degree_plus_one
        );

        let mut intermediate_oracles = vec![];
        let mut intermediate_values = vec![];
        let mut intermediate_roots = vec![];

        let mut challenges = vec![];
        let num_challenges = coset_factor;
        let mut next_domain_challenges = {
            let mut challenges = vec![];
            for _ in 0..num_challenges {
                challenges.push(prng.get_challenge());
            }

            challenges
        };

        challenges.push(next_domain_challenges.clone());

        let mut values_slice = lde_values.as_ref();

        let omegas_inv_bitreversed: &[E::Fr] = self.precomputations.bit_reversed_omegas();

        // if we would precompute all N we would have
        // [0, N/2, N/4, 3N/4, N/8, N/2 + N/8, N/8 + N/4, N/8 + N/4 + N/2, ...]
        // but we only precompute half of them and have
        // [0, N/4, N/8, N/8 + N/4, ...]

        let mut this_domain_size = lde_values.len();

        // step 0: fold totally by 2
        // step 1: fold totally by 4
        // etc...

        let num_steps = self.folding_schedule.len();

        // we do NOT need to make the first (largest) tree cause it's values are simulated
        // so we will cover the first step later on separately
        for (fri_step, coset_factor) in self.folding_schedule.iter().enumerate() {
            let coset_factor = *coset_factor;
            let wrapping_factor = 1 << coset_factor;
            let next_domain_size = this_domain_size / wrapping_factor;
            let mut next_values = vec![E::Fr::zero(); next_domain_size];

            // we combine like this with FRI trees being aware of the FRI computations
            //            next_value(omega**)
            //          /                     \
            //    intermediate(omega*)       intermediate(-omega*)
            //    /           \                   /            \
            // this(omega)   this(-omega)     this(omega')    this(-omega')
            //
            // so omega* = omega^2i. omega' = sqrt(-omega^2i) = sqrt(omega^(N/2 + 2i)) = omega^N/4 + i
            //
            // we expect values to come bitreversed, so this(omega) and this(-omega) are always adjustent to each other
            // because in normal emumeration it would be elements b0XYZ and b1XYZ, and now it's bZYX0 and bZYX1
            //
            // this(omega^(N/4 + i)) for b00YZ has a form b01YZ, so bitreversed it's bZY00 and bZY10
            // this(-omega^(N/4 + i)) obviously has bZY11, so they are all near in initial values

            worker.scope(next_values.len(), |scope, chunk| {
                for (i, v) in next_values.chunks_mut(chunk).enumerate() {
                    let next_domain_challenges = next_domain_challenges.clone();
                    scope.spawn(move |_| {
                        let initial_k = i * chunk;
                        let mut this_level_values = Vec::with_capacity(wrapping_factor);
                        let mut next_level_values = vec![E::Fr::zero(); wrapping_factor];
                        for (j, v) in v.iter_mut().enumerate() {
                            let batch_id = initial_k + j;
                            let values_offset = batch_id * wrapping_factor;
                            for (wrapping_step, challenge) in next_domain_challenges.iter().enumerate() {
                                let base_omega_idx = (batch_id * wrapping_factor) >> (1 + wrapping_step);
                                let expected_this_level_values = wrapping_factor >> wrapping_step;
                                let expected_next_level_values = wrapping_factor >> (wrapping_step + 1);
                                let inputs = if wrapping_step == 0 {
                                    &values_slice[values_offset..(values_offset + wrapping_factor)]
                                } else {
                                    &this_level_values[..expected_this_level_values]
                                };

                                // imagine first FRI step, first wrapping step
                                // in values we have f(i), f(i + N/2), f(i + N/4), f(i + N/4 + N/2), f(i + N/8), ...
                                // so we need to use omega(i) for the first pair, omega(i + N/4) for the second, omega(i + N/8)
                                // on the next step we would have f'(2i), f'(2i + N/2), f'(2i + N/4), f'(2i + N/4 + N/2)
                                // so we would have to pick omega(2i) and omega(2i + N/4)
                                // this means LSB is always the same an only depend on the pair_idx below
                                // MSB is more tricky
                                // for a batch number 0 we have i = 0
                                // for a batch number 1 due to bitreverse we have index equal to b000001xxx where LSB are not important in the batch
                                // such value actually gives i = bxxx100000 that is a bitreverse of the batch number with proper number of bits
                                // due to precomputed omegas also being bitreversed we just need a memory location b000001xxx >> 1

                                debug_assert_eq!(inputs.len() / 2, expected_next_level_values);

                                for (pair_idx, (pair, o)) in inputs.chunks(2).zip(next_level_values[..expected_next_level_values].iter_mut()).enumerate() {
                                    debug_assert!(base_omega_idx & pair_idx == 0);
                                    let omega_idx = base_omega_idx + pair_idx;
                                    let omega_inv = omegas_inv_bitreversed[omega_idx];
                                    let f_at_omega = pair[0];
                                    let f_at_minus_omega = pair[1];
                                    let mut v_even_coeffs = f_at_omega;
                                    v_even_coeffs.add_assign(&f_at_minus_omega);

                                    let mut v_odd_coeffs = f_at_omega;
                                    v_odd_coeffs.sub_assign(&f_at_minus_omega);
                                    v_odd_coeffs.mul_assign(&omega_inv);

                                    let mut tmp = v_odd_coeffs;
                                    tmp.mul_assign(&challenge);
                                    tmp.add_assign(&v_even_coeffs);
                                    tmp.mul_assign(&two_inv);

                                    *o = tmp;
                                }

                                this_level_values.clear();
                                this_level_values.clone_from(&next_level_values);
                            }

                            *v = next_level_values[0];
                        }
                    });
                }
            });

            // until we hit the last step we take newly produced values
            // and make an oracle from them
            if fri_step < num_steps - 1 {
                coset_schedule_index += 1;
                this_domain_size = next_domain_size;
                let coset_factor = self.folding_schedule[coset_schedule_index];

                let tree_params = BinaryTreeParams { values_per_leaf: (1 << coset_factor) };

                let intermediate_oracle = BinaryTree::create(&next_values, self.tree_hasher.clone(), &tree_params);

                let root = intermediate_oracle.get_commitment();
                let num_challenges = coset_factor;
                next_domain_challenges = {
                    prng.commit_input(&root);
                    let mut challenges = vec![];
                    for _ in 0..num_challenges {
                        challenges.push(prng.get_challenge());
                    }

                    challenges
                };

                challenges.push(next_domain_challenges.clone());
                intermediate_roots.push(root);
                intermediate_oracles.push(intermediate_oracle);
            }

            intermediate_values.push(next_values);

            values_slice = intermediate_values.last().expect("is something").as_ref();
        }

        assert_eq!(challenges.len(), num_steps);
        assert_eq!(intermediate_roots.len(), num_steps - 1);
        assert_eq!(intermediate_oracles.len(), num_steps - 1);
        assert_eq!(intermediate_values.len(), num_steps);

        let mut final_poly_values = Polynomial::from_values(values_slice.to_vec())?;
        final_poly_values.bitreverse_enumeration(&worker);
        let final_poly_coeffs = if self.coset_factor == E::Fr::one() {
            final_poly_values.icoset_fft(&worker)
        } else {
            final_poly_values.icoset_fft_for_generator(&worker, &self.coset_factor)
        };

        let mut final_poly_coeffs = final_poly_coeffs.into_coeffs();

        let mut degree = final_poly_coeffs.len() - 1;
        for c in final_poly_coeffs.iter().rev() {
            if c.is_zero() {
                degree -= 1;
            } else {
                break;
            }
        }

        assert!(degree < self.output_coeffs_at_degree_plus_one, "polynomial degree is too large, coeffs = {:?}", final_poly_coeffs);

        final_poly_coeffs.truncate(self.output_coeffs_at_degree_plus_one);

        let set = FriOraclesSet {
            intermediate_oracles,
            intermediate_roots,
            intermediate_leaf_values: intermediate_values,
            intermediate_challenges: challenges,
            final_coefficients: final_poly_coeffs,
        };

        Ok(set)
    }
}
