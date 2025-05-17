use super::super::cs_old::*;
use super::binary_tree::{BinaryTree, BinaryTreeParams};
use super::multioracle::Multioracle;
use super::setup::*;
use super::tree_hash::BinaryTreeHasher;
use super::*;
use crate::pairing::ff::{Field, PrimeField, PrimeFieldRepr};
use crate::pairing::Engine;
use crate::plonk::better_cs::utils::*;
use crate::plonk::commitments::transcript::Prng;
use crate::plonk::commitments::transparent::utils::log2_floor;
use crate::plonk::domains::*;
use crate::plonk::fft::cooley_tukey_ntt::*;
use crate::plonk::polynomials::*;
use crate::worker::Worker;
use crate::SynthesisError;
use simple_fri::*;

pub(crate) fn get_precomputed_x_lde<E: Engine>(domain_size: usize, worker: &Worker) -> Result<Polynomial<E::Fr, Values>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();
    let mut x_poly = Polynomial::from_values(vec![coset_factor; domain_size])?;
    x_poly.distribute_powers(&worker, x_poly.omega);
    x_poly.bitreverse_enumeration(&worker);

    Ok(x_poly)
}

pub(crate) fn get_precomputed_inverse_divisor<E: Engine>(vanishing_size: usize, domain_size: usize, worker: &Worker) -> Result<Polynomial<E::Fr, Values>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();
    let mut vanishing_poly_inverse_bitreversed = evaluate_vanishing_polynomial_of_degree_on_domain_size::<E::Fr>(vanishing_size as u64, &coset_factor, domain_size as u64, &worker)?;
    vanishing_poly_inverse_bitreversed.batch_inversion(&worker)?;
    vanishing_poly_inverse_bitreversed.bitreverse_enumeration(&worker);

    Ok(vanishing_poly_inverse_bitreversed)
}

pub(crate) struct FirstPartialProverState<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_ldes: Vec<Polynomial<E::Fr, Values>>,
    witness_polys_unpadded_values: Vec<Polynomial<E::Fr, Values>>,
    witness_multioracle_tree: BinaryTree<E, H>,
}

pub(crate) struct FirstProverMessage<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    pub(crate) n: usize,
    pub(crate) num_inputs: usize,
    pub(crate) input_values: Vec<E::Fr>,
    pub(crate) witness_multioracle_commitment: H::Output,
}

pub(crate) struct FirstVerifierMessage<E: Engine> {
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,
}

pub(crate) struct SecondPartialProverState<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_ldes: Vec<Polynomial<E::Fr, Values>>,
    witness_polys_in_monomial_form: Vec<Polynomial<E::Fr, Coefficients>>,
    witness_multioracle_tree: BinaryTree<E, H>,
    grand_product_polynomial_lde: Vec<Polynomial<E::Fr, Values>>,
    grand_product_polynomial_in_monomial_form: Vec<Polynomial<E::Fr, Coefficients>>,
    grand_product_polynomial_multioracle_tree: BinaryTree<E, H>,
}

pub(crate) struct SecondProverMessage<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    pub(crate) grand_product_oracle_commitment: H::Output,

    _marker: std::marker::PhantomData<E>,
}

pub(crate) struct SecondVerifierMessage<E: Engine> {
    pub(crate) alpha: E::Fr,
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,
}

pub(crate) struct ThirdPartialProverState<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_ldes: Vec<Polynomial<E::Fr, Values>>,
    witness_polys_in_monomial_form: Vec<Polynomial<E::Fr, Coefficients>>,
    witness_multioracle_tree: BinaryTree<E, H>,
    grand_product_polynomial_lde: Vec<Polynomial<E::Fr, Values>>,
    grand_product_polynomial_in_monomial_form: Vec<Polynomial<E::Fr, Coefficients>>,
    grand_product_polynomial_multioracle_tree: BinaryTree<E, H>,
    t_poly_parts_ldes: Vec<Polynomial<E::Fr, Values>>,
    t_poly_parts: Vec<Polynomial<E::Fr, Coefficients>>,
    t_poly_parts_multioracle_tree: BinaryTree<E, H>,
}

pub(crate) struct ThirdProverMessage<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    pub(crate) quotient_poly_oracle_commitment: H::Output,

    _marker: std::marker::PhantomData<E>,
}

pub(crate) struct ThirdVerifierMessage<E: Engine> {
    pub(crate) alpha: E::Fr,
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,
    pub(crate) z: E::Fr,
}

pub(crate) struct FourthPartialProverState<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_ldes: Vec<Polynomial<E::Fr, Values>>,
    witness_polys_in_monomial_form: Vec<Polynomial<E::Fr, Coefficients>>,
    witness_multioracle_tree: BinaryTree<E, H>,
    grand_product_polynomial_lde: Vec<Polynomial<E::Fr, Values>>,
    grand_product_polynomial_in_monomial_form: Vec<Polynomial<E::Fr, Coefficients>>,
    grand_product_polynomial_multioracle_tree: BinaryTree<E, H>,
    t_poly_parts_ldes: Vec<Polynomial<E::Fr, Values>>,
    t_poly_parts: Vec<Polynomial<E::Fr, Coefficients>>,
    t_poly_parts_multioracle_tree: BinaryTree<E, H>,
    wire_values_at_z: Vec<(usize, E::Fr)>,
    wire_values_at_z_omega: Vec<(usize, E::Fr)>,
    setup_values_at_z: Vec<E::Fr>,
    permutation_polynomials_at_z: Vec<E::Fr>,
    gate_selector_polynomials_at_z: Vec<E::Fr>,
    grand_product_at_z: E::Fr,
    grand_product_at_z_omega: E::Fr,
    quotient_polynomial_parts_at_z: Vec<E::Fr>,
}

pub(crate) struct FourthProverMessage<E: Engine> {
    pub(crate) wire_values_at_z: Vec<(usize, E::Fr)>,
    pub(crate) wire_values_at_z_omega: Vec<(usize, E::Fr)>,
    pub(crate) setup_values_at_z: Vec<E::Fr>,
    pub(crate) permutation_polynomials_at_z: Vec<E::Fr>,
    pub(crate) gate_selector_polynomials_at_z: Vec<E::Fr>,
    pub(crate) grand_product_at_z: E::Fr,
    pub(crate) grand_product_at_z_omega: E::Fr,
    pub(crate) quotient_polynomial_parts_at_z: Vec<E::Fr>,
}

pub(crate) struct FourthVerifierMessage<E: Engine> {
    pub(crate) alpha: E::Fr,
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,
    pub(crate) z: E::Fr,
    pub(crate) v: E::Fr,
}

pub(crate) struct FifthProverMessage<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    pub(crate) fri_intermediate_roots: Vec<H::Output>,
    pub(crate) final_coefficients: Vec<E::Fr>,
}

struct WitnessOpeningRequest<'a, F: PrimeField> {
    polynomials: Vec<&'a Polynomial<F, Values>>,
    opening_point: F,
    opening_values: Vec<F>,
}

struct SetupOpeningRequest<'a, F: PrimeField> {
    polynomials: Vec<&'a Polynomial<F, Values>>,
    setup_point: F,
    setup_values: Vec<F>,
    opening_point: F,
    opening_values: Vec<F>,
}

pub struct RedshiftProver<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    sorted_gates: Vec<Box<dyn GateEquationInternal>>,
    gate_constants: Vec<Vec<E::Fr>>,
    precomputed_omegas: BitReversedOmegas<E::Fr>,
    precomputed_omegas_inv: OmegasInvBitreversed<E::Fr>,
    tree_hasher: H,
    state_width: usize,
}

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> RedshiftProver<E, H> {
    pub(crate) fn first_step<P: PlonkConstraintSystemParams<E>, MG: MainGateEquation>(
        assembly: TrivialAssembly<E, P, MG>,
        tree_hasher: H,
        worker: &Worker,
    ) -> Result<(Self, FirstPartialProverState<E, H>, FirstProverMessage<E, H>), SynthesisError> {
        assert!(assembly.is_finalized);

        let input_values = assembly.input_assingments.clone();

        let n = assembly.n();
        let size = n.next_power_of_two();
        let num_inputs = assembly.num_inputs;

        let required_domain_size = n + 1;
        assert!(required_domain_size.is_power_of_two());

        let non_residues = make_non_residues::<E::Fr>(P::STATE_WIDTH - 1);

        let (full_assignments, _) = assembly.make_state_and_witness_polynomials(&worker)?;

        let gate_constants = assembly.sorted_gate_constants;
        let sorted_gates = assembly.sorted_gates;

        // Commit wire polynomials

        let omegas_bitreversed = BitReversedOmegas::<E::Fr>::new_for_domain_size(size.next_power_of_two());
        let omegas_inv_bitreversed = <OmegasInvBitreversed<E::Fr> as CTPrecomputations<E::Fr>>::new_for_domain_size(size.next_power_of_two());

        let mut first_message = FirstProverMessage::<E, H> {
            n: n,
            num_inputs: num_inputs,
            input_values: input_values.clone(),
            witness_multioracle_commitment: H::placeholder_output(),
        };

        let s = Self {
            sorted_gates,
            gate_constants,
            precomputed_omegas: omegas_bitreversed,
            precomputed_omegas_inv: omegas_inv_bitreversed,
            tree_hasher: tree_hasher.clone(),
            state_width: P::STATE_WIDTH,
        };

        let coset_factor = E::Fr::multiplicative_generator();

        let mut witness_polys_ldes = vec![];

        for wire_poly in full_assignments.iter() {
            let lde = Polynomial::from_values(wire_poly.clone())?
                .ifft_using_bitreversed_ntt(&worker, &s.precomputed_omegas_inv, &E::Fr::one())?
                .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, &s.precomputed_omegas, &coset_factor)?;

            witness_polys_ldes.push(lde);
        }

        // now transform assignments in the polynomials

        let mut witness_polys_unpadded_values = vec![];

        for p in full_assignments.into_iter() {
            let p = Polynomial::from_values_unpadded(p)?;
            witness_polys_unpadded_values.push(p);
        }

        let multioracle = Multioracle::new_from_polynomials(&witness_polys_ldes, tree_hasher, FRI_VALUES_PER_LEAF, &worker);

        let tree = multioracle.tree;
        first_message.witness_multioracle_commitment = tree.get_commitment();

        let first_state = FirstPartialProverState::<E, H> {
            required_domain_size: n + 1,
            non_residues: non_residues,
            input_values: input_values.clone(),
            witness_polys_ldes: witness_polys_ldes,
            witness_polys_unpadded_values: witness_polys_unpadded_values,
            witness_multioracle_tree: tree,
        };

        Ok((s, first_state, first_message))
    }

    pub(crate) fn second_step_from_first_step(
        &self,
        first_state: FirstPartialProverState<E, H>,
        first_verifier_message: FirstVerifierMessage<E>,
        permutation_polynomials: &Vec<Polynomial<E::Fr, Values>>,
        worker: &Worker,
    ) -> Result<(SecondPartialProverState<E, H>, SecondProverMessage<E, H>), SynthesisError> {
        let FirstVerifierMessage { beta, gamma, .. } = first_verifier_message;

        assert_eq!(first_state.witness_polys_unpadded_values.len(), self.state_width, "first state must containt assignment poly values");

        let mut grand_products_protos_with_gamma = first_state.witness_polys_unpadded_values.clone();

        // add gamma here to save computations later
        for p in grand_products_protos_with_gamma.iter_mut() {
            p.add_constant(&worker, &gamma);
        }

        let required_domain_size = first_state.required_domain_size;

        let domain = Domain::new_for_size(required_domain_size as u64)?;

        let mut domain_elements = materialize_domain_elements_with_natural_enumeration(&domain, &worker);

        domain_elements.pop().expect("must pop last element for omega^i");

        let mut domain_elements_poly_by_beta = Polynomial::from_values_unpadded(domain_elements)?;
        domain_elements_poly_by_beta.scale(&worker, beta);

        // we take A, B, C, ... values and form (A + beta * X * non_residue + gamma), etc and calculate their grand product

        let mut z_num = {
            let mut grand_products_proto_it = grand_products_protos_with_gamma.iter().cloned();

            let mut z_1 = grand_products_proto_it.next().unwrap();
            z_1.add_assign(&worker, &domain_elements_poly_by_beta);

            for (mut p, non_res) in grand_products_proto_it.zip(first_state.non_residues.iter()) {
                p.add_assign_scaled(&worker, &domain_elements_poly_by_beta, non_res);
                z_1.mul_assign(&worker, &p);
            }

            z_1
        };

        // we take A, B, C, ... values and form (A + beta * perm_a + gamma), etc and calculate their grand product

        let z_den = {
            assert_eq!(permutation_polynomials.len(), grand_products_protos_with_gamma.len());
            let mut grand_products_proto_it = grand_products_protos_with_gamma.into_iter();
            let mut permutation_polys_it = permutation_polynomials.iter();

            let mut z_2 = grand_products_proto_it.next().unwrap();
            z_2.add_assign_scaled(&worker, permutation_polys_it.next().unwrap(), &beta);

            for (mut p, perm) in grand_products_proto_it.zip(permutation_polys_it) {
                // permutation polynomials
                p.add_assign_scaled(&worker, perm, &beta);
                z_2.mul_assign(&worker, &p);
            }

            z_2.batch_inversion(&worker)?;

            z_2
        };

        z_num.mul_assign(&worker, &z_den);
        drop(z_den);

        let z = z_num.calculate_shifted_grand_product(&worker)?;

        assert!(z.size().is_power_of_two());

        assert!(z.as_ref()[0] == E::Fr::one());
        assert!(z.as_ref().last().expect("must exist") == &E::Fr::one());

        // interpolate on the main domain
        let grand_product_in_monomial_form = z.ifft_using_bitreversed_ntt(&worker, &self.precomputed_omegas_inv, &E::Fr::one())?;

        let coset_factor = E::Fr::multiplicative_generator();

        let grand_product_lde = grand_product_in_monomial_form
            .clone()
            .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, &self.precomputed_omegas, &coset_factor)?;

        let grand_product_ldes = vec![grand_product_lde];

        let multioracle = Multioracle::new_from_polynomials(&grand_product_ldes, self.tree_hasher.clone(), FRI_VALUES_PER_LEAF, &worker);

        let mut witness_polys_in_monomial_form = vec![];
        for mut p in first_state.witness_polys_unpadded_values.into_iter() {
            p.pad_to_domain()?;
            let p = p.ifft_using_bitreversed_ntt(&worker, &self.precomputed_omegas_inv, &E::Fr::one())?;
            witness_polys_in_monomial_form.push(p);
        }

        let tree = multioracle.tree;
        let commitment = tree.get_commitment();

        let state = SecondPartialProverState::<E, H> {
            required_domain_size,
            non_residues: first_state.non_residues,
            input_values: first_state.input_values,
            witness_polys_ldes: first_state.witness_polys_ldes,
            witness_polys_in_monomial_form: witness_polys_in_monomial_form,
            witness_multioracle_tree: first_state.witness_multioracle_tree,
            grand_product_polynomial_lde: grand_product_ldes,
            grand_product_polynomial_in_monomial_form: vec![grand_product_in_monomial_form],
            grand_product_polynomial_multioracle_tree: tree,
        };

        let message = SecondProverMessage::<E, H> {
            grand_product_oracle_commitment: commitment,
            _marker: std::marker::PhantomData,
        };

        Ok((state, message))
    }

    pub(crate) fn third_step_from_second_step(
        &self,
        second_state: SecondPartialProverState<E, H>,
        second_verifier_message: SecondVerifierMessage<E>,
        setup: &SetupMultioracle<E, H>,
        worker: &Worker,
    ) -> Result<(ThirdPartialProverState<E, H>, ThirdProverMessage<E, H>), SynthesisError> {
        use std::sync::Arc;

        let mut quotient_linearization_challenge = E::Fr::one();
        let SecondVerifierMessage { alpha, beta, gamma, .. } = second_verifier_message;

        let coset_factor = E::Fr::multiplicative_generator();

        let required_domain_size = second_state.required_domain_size;

        println!("Domain size = {}", required_domain_size);

        let mut max_degree = self.state_width + 1;
        let assume_gate_selectors = self.sorted_gates.len() > 1;

        let mut extra_precomputations_storage_by_time_dilation = std::collections::HashMap::new();

        for gate in self.sorted_gates.iter() {
            let mut degree = gate.degree();
            if assume_gate_selectors {
                degree += 1;
            }

            if degree > max_degree {
                max_degree = degree;
            }
        }

        assert!(max_degree <= 5, "we do not support very high order constraints yet");

        let quotient_degree_factor = (max_degree - 1).next_power_of_two();
        // let quotient_degree_factor = 8;

        let quotient_degree = quotient_degree_factor * required_domain_size;

        assert!(quotient_degree.is_power_of_two());

        let one = E::Fr::one();
        let mut minus_one = one;
        minus_one.negate();

        // we also assume bitreverse
        let mut t = Polynomial::<E::Fr, Values>::new_for_size(quotient_degree)?;

        let mut global_scratch_space = t.clone();

        let mut public_inputs_processed = false;

        let partition_factor = LDE_FACTOR / quotient_degree_factor;

        let use_gate_selectors = self.sorted_gates.len() > 1;
        let selectors_indexes = setup.gate_selectors_indexes.clone();
        println!("Have {} gate selector indexes", selectors_indexes.len());
        let mut selectors_range_it = selectors_indexes.into_iter();

        let mut local_scratch_space = if use_gate_selectors { Some(global_scratch_space.clone()) } else { None };

        for (i, (gate, constants)) in self.sorted_gates.iter().zip(self.gate_constants.iter()).enumerate() {
            let mut constants_iter = constants.iter();
            if i == 0 {
                assert!(gate.can_include_public_inputs());
            }
            let constant_term_index = gate.put_public_inputs_into_selector_id();
            for constraint in gate.get_constraints().iter() {
                for term in constraint.0.iter() {
                    let mut must_refill_scratch = true;
                    let mut base = match term.0 {
                        Coefficient::PlusOne => one,
                        Coefficient::MinusOne => minus_one,
                        Coefficient::Other => *constants_iter.next().unwrap(),
                    };

                    for poly in term.1.iter() {
                        match poly {
                            PolynomialInConstraint::VariablesPolynomial(poly_num, TimeDilation(0)) => {
                                let poly_lde_ref = &second_state.witness_polys_ldes[*poly_num];
                                let poly_lde_part = poly_lde_ref.clone_subset_assuming_bitreversed(partition_factor)?;
                                let scratch_space = if use_gate_selectors { local_scratch_space.as_mut().unwrap() } else { &mut global_scratch_space };

                                if must_refill_scratch {
                                    must_refill_scratch = false;
                                    scratch_space.reuse_allocation(&poly_lde_part);
                                } else {
                                    scratch_space.mul_assign(&worker, &poly_lde_part);
                                }
                            }
                            PolynomialInConstraint::VariablesPolynomial(poly_num, TimeDilation(dilation)) => {
                                let dilated_poly = if let Some(dilated_poly) = extra_precomputations_storage_by_time_dilation.get(&(*poly_num, TimeDilation(*dilation))) {
                                    dilated_poly
                                } else {
                                    let poly_lde_ref = &second_state.witness_polys_ldes[*poly_num];
                                    let poly_lde_part = poly_lde_ref.clone_subset_assuming_bitreversed(partition_factor)?;

                                    let shift = quotient_degree_factor * (*dilation);
                                    let dilated_poly = poly_lde_part.clone_shifted_assuming_bitreversed(shift, &worker)?;

                                    extra_precomputations_storage_by_time_dilation.insert((*poly_num, TimeDilation(*dilation)), dilated_poly);

                                    extra_precomputations_storage_by_time_dilation.get(&(*poly_num, TimeDilation(*dilation))).unwrap()
                                };

                                let scratch_space = if use_gate_selectors { local_scratch_space.as_mut().unwrap() } else { &mut global_scratch_space };

                                if must_refill_scratch {
                                    must_refill_scratch = false;
                                    scratch_space.reuse_allocation(dilated_poly);
                                } else {
                                    scratch_space.mul_assign(&worker, dilated_poly);
                                }
                            }
                            PolynomialInConstraint::SetupPolynomial(str_id, poly_num, TimeDilation(0)) => {
                                let id = PolyIdentifier::SetupPolynomial(str_id, *poly_num);
                                let idx = setup.setup_ids.iter().position(|el| el == &id).unwrap();
                                let poly_lde_ref = &setup.polynomial_ldes[idx];
                                let poly_lde_part = poly_lde_ref.clone_subset_assuming_bitreversed(partition_factor)?;

                                let process_public_inputs = if constant_term_index.is_some() {
                                    public_inputs_processed == false && constant_term_index.unwrap() == *poly_num
                                } else {
                                    false
                                };

                                let scratch_space = if use_gate_selectors { local_scratch_space.as_mut().unwrap() } else { &mut global_scratch_space };

                                if process_public_inputs {
                                    public_inputs_processed = true;
                                    // processing public inputs
                                    assert!(base == one); // base coefficient for here should be one
                                    let mut inputs_poly = Polynomial::<E::Fr, Values>::new_for_size(required_domain_size)?;

                                    for (idx, &input) in second_state.input_values.iter().enumerate() {
                                        inputs_poly.as_mut()[idx] = input;
                                    }
                                    // go into monomial form

                                    let inputs_poly = inputs_poly.ifft_using_bitreversed_ntt(&worker, &self.precomputed_omegas_inv, &E::Fr::one())?;

                                    let mut inputs_poly = inputs_poly.bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR / partition_factor, &self.precomputed_omegas, &coset_factor)?;

                                    // add constants selectors vector
                                    inputs_poly.add_assign(&worker, &poly_lde_part);

                                    if must_refill_scratch {
                                        must_refill_scratch = false;
                                        scratch_space.reuse_allocation(&inputs_poly);
                                    } else {
                                        scratch_space.mul_assign(&worker, &inputs_poly);
                                    }
                                } else {
                                    if must_refill_scratch {
                                        must_refill_scratch = false;
                                        scratch_space.reuse_allocation(&poly_lde_part);
                                    } else {
                                        scratch_space.mul_assign(&worker, &poly_lde_part);
                                    }
                                }
                            }
                            _ => {
                                unimplemented!()
                            }
                        }
                    }

                    base.mul_assign(&quotient_linearization_challenge);

                    if use_gate_selectors {
                        let scratch_space = local_scratch_space.as_ref().unwrap();
                        global_scratch_space.add_assign_scaled(&worker, &scratch_space, &base);
                    } else {
                        // add into T poly directly
                        t.add_assign_scaled(&worker, &global_scratch_space, &base);
                    }
                }

                // if use_gate_selectors {
                //     let selector_idx = (&mut selectors_range_it).next().expect(&format!("must get gate selector for gate {}", i));
                //     let poly_lde_ref = &setup.polynomial_ldes[selector_idx];
                //     let poly_lde_part = poly_lde_ref.clone_subset_assuming_bitreversed(
                //         partition_factor
                //     )?;
                //     global_scratch_space.mul_assign(&worker, &poly_lde_part);
                //     t.add_assign(&worker, &global_scratch_space);
                // }

                quotient_linearization_challenge.mul_assign(&alpha);
            }

            if use_gate_selectors {
                let selector_idx = (&mut selectors_range_it).next().expect(&format!("must get gate selector for gate {}", i));
                let poly_lde_ref = &setup.polynomial_ldes[selector_idx];
                let poly_lde_part = poly_lde_ref.clone_subset_assuming_bitreversed(partition_factor)?;
                global_scratch_space.mul_assign(&worker, &poly_lde_part);
                t.add_assign(&worker, &global_scratch_space);
            }
        }

        let mut scratch_space = global_scratch_space;

        let grand_product_lde_bitreversed = second_state.grand_product_polynomial_lde[0].clone_subset_assuming_bitreversed(partition_factor)?;

        let shift = quotient_degree_factor;

        let grand_product_shifted_lde_bitreversed = grand_product_lde_bitreversed.clone_shifted_assuming_bitreversed(shift, &worker)?;

        let non_residues = second_state.non_residues.clone();

        // For both Z_1 and Z_2 we first check for grand products
        // z*(X)(A + beta*X + gamma)(B + beta*k_1*X + gamma)(C + beta*K_2*X + gamma) -
        // - (A + beta*perm_a(X) + gamma)(B + beta*perm_b(X) + gamma)(C + beta*perm_c(X) + gamma)*Z(X*Omega)== 0

        // we use evaluations of the polynomial X and K_i * X on a large domain's coset

        {
            let mut contrib_z = grand_product_lde_bitreversed.clone();

            // A + beta*X + gamma

            let poly_lde_ref = &second_state.witness_polys_ldes[0];
            let poly_lde_part = poly_lde_ref.clone_subset_assuming_bitreversed(partition_factor)?;

            scratch_space.reuse_allocation(&poly_lde_part);
            drop(poly_lde_ref);

            scratch_space.add_constant(&worker, &gamma);
            let x_precomp = get_precomputed_x_lde::<E>(quotient_degree, &worker)?;

            scratch_space.add_assign_scaled(&worker, &x_precomp, &beta);
            contrib_z.mul_assign(&worker, &scratch_space);

            assert_eq!(non_residues.len() + 1, self.state_width);

            for (w, non_res) in second_state.witness_polys_ldes[1..].iter().zip(non_residues.iter()) {
                let mut factor = beta;
                factor.mul_assign(&non_res);

                let poly_lde_part = w.clone_subset_assuming_bitreversed(partition_factor)?;

                scratch_space.reuse_allocation(&poly_lde_part);
                scratch_space.add_constant(&worker, &gamma);
                scratch_space.add_assign_scaled(&worker, &x_precomp, &factor);
                contrib_z.mul_assign(&worker, &scratch_space);
            }

            t.add_assign_scaled(&worker, &contrib_z, &quotient_linearization_challenge);

            drop(contrib_z);

            let mut contrib_z = grand_product_shifted_lde_bitreversed;

            // A + beta*perm_a + gamma

            for (w, perm_idx) in second_state.witness_polys_ldes.iter().zip(setup.permutations_ranges[0].clone()) {
                let poly_lde_part = w.clone_subset_assuming_bitreversed(partition_factor)?;

                let perm_part = setup.polynomial_ldes[perm_idx].clone_subset_assuming_bitreversed(partition_factor)?;

                scratch_space.reuse_allocation(&poly_lde_part);
                scratch_space.add_constant(&worker, &gamma);
                scratch_space.add_assign_scaled(&worker, &perm_part, &beta);
                contrib_z.mul_assign(&worker, &scratch_space);
            }

            t.sub_assign_scaled(&worker, &contrib_z, &quotient_linearization_challenge);

            drop(contrib_z);
        }

        quotient_linearization_challenge.mul_assign(&alpha);

        // z(omega^0) - 1 == 0

        let l_0 = calculate_lagrange_poly::<E::Fr>(&worker, required_domain_size.next_power_of_two(), 0)?;

        {
            let mut z_minus_one_by_l_0 = grand_product_lde_bitreversed;
            z_minus_one_by_l_0.sub_constant(&worker, &E::Fr::one());

            let l_coset_lde_bitreversed = l_0.bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR / partition_factor, &self.precomputed_omegas, &coset_factor)?;

            z_minus_one_by_l_0.mul_assign(&worker, &l_coset_lde_bitreversed);

            t.add_assign_scaled(&worker, &z_minus_one_by_l_0, &quotient_linearization_challenge);

            drop(z_minus_one_by_l_0);
        }

        drop(scratch_space);

        let divisor_inversed = get_precomputed_inverse_divisor::<E>(required_domain_size, quotient_degree, &worker)?;
        t.mul_assign(&worker, &divisor_inversed);

        t.bitreverse_enumeration(&worker);

        let t_poly_in_monomial_form = t.icoset_fft_for_generator(&worker, &E::Fr::multiplicative_generator());

        fn get_degree<F: PrimeField>(poly: &Polynomial<F, Coefficients>) -> usize {
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

        println!("T degree = {}", get_degree::<E::Fr>(&t_poly_in_monomial_form));

        let t_poly_parts = t_poly_in_monomial_form.break_into_multiples(required_domain_size)?;

        let mut t_poly_parts_ldes = vec![];
        for p in t_poly_parts.iter() {
            let lde = p.clone().bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, &self.precomputed_omegas, &coset_factor)?;

            t_poly_parts_ldes.push(lde);
        }

        let multioracle = Multioracle::new_from_polynomials(&t_poly_parts_ldes, self.tree_hasher.clone(), FRI_VALUES_PER_LEAF, &worker);

        let tree = multioracle.tree;
        let commitment = tree.get_commitment();

        let state = ThirdPartialProverState::<E, H> {
            required_domain_size,
            non_residues: second_state.non_residues,
            input_values: second_state.input_values,
            witness_polys_ldes: second_state.witness_polys_ldes,
            witness_polys_in_monomial_form: second_state.witness_polys_in_monomial_form,
            witness_multioracle_tree: second_state.witness_multioracle_tree,
            grand_product_polynomial_lde: second_state.grand_product_polynomial_lde,
            grand_product_polynomial_in_monomial_form: second_state.grand_product_polynomial_in_monomial_form,
            grand_product_polynomial_multioracle_tree: second_state.grand_product_polynomial_multioracle_tree,
            t_poly_parts_ldes: t_poly_parts_ldes,
            t_poly_parts: t_poly_parts,
            t_poly_parts_multioracle_tree: tree,
        };

        let message = ThirdProverMessage::<E, H> {
            quotient_poly_oracle_commitment: commitment,
            _marker: std::marker::PhantomData,
        };

        Ok((state, message))
    }

    pub(crate) fn fourth_step_from_third_step(
        &self,
        third_state: ThirdPartialProverState<E, H>,
        third_verifier_message: ThirdVerifierMessage<E>,
        setup: &SetupMultioracle<E, H>,
        worker: &Worker,
    ) -> Result<(FourthPartialProverState<E, H>, FourthProverMessage<E>), SynthesisError> {
        let ThirdVerifierMessage { z, .. } = third_verifier_message;
        let required_domain_size = third_state.required_domain_size;

        let domain = Domain::new_for_size(required_domain_size as u64)?;

        let mut state = FourthPartialProverState::<E, H> {
            required_domain_size,
            non_residues: third_state.non_residues,
            input_values: third_state.input_values,
            witness_polys_ldes: third_state.witness_polys_ldes,
            witness_polys_in_monomial_form: third_state.witness_polys_in_monomial_form,
            witness_multioracle_tree: third_state.witness_multioracle_tree,
            grand_product_polynomial_lde: third_state.grand_product_polynomial_lde,
            grand_product_polynomial_in_monomial_form: third_state.grand_product_polynomial_in_monomial_form,
            grand_product_polynomial_multioracle_tree: third_state.grand_product_polynomial_multioracle_tree,
            t_poly_parts_ldes: third_state.t_poly_parts_ldes,
            t_poly_parts: third_state.t_poly_parts,
            t_poly_parts_multioracle_tree: third_state.t_poly_parts_multioracle_tree,
            wire_values_at_z: vec![],
            wire_values_at_z_omega: vec![],
            setup_values_at_z: vec![E::Fr::zero(); setup.setup_ids.len()],
            permutation_polynomials_at_z: vec![],
            gate_selector_polynomials_at_z: vec![],
            grand_product_at_z: E::Fr::zero(),
            grand_product_at_z_omega: E::Fr::zero(),
            quotient_polynomial_parts_at_z: vec![],
        };

        let mut z_by_omega = z;
        z_by_omega.mul_assign(&domain.generator);

        // now need to go over constraints and decide what polynomials to open where

        let mut dilation_maps = std::collections::HashMap::new();

        for gate in self.sorted_gates.iter() {
            for constraint in gate.get_constraints().iter() {
                for term in constraint.0.iter() {
                    for poly in term.1.iter() {
                        match poly {
                            PolynomialInConstraint::VariablesPolynomial(poly_num, TimeDilation(dilation)) => {
                                let poly_id = PolyIdentifier::VariablesPolynomial(*poly_num);
                                let key = TimeDilation(*dilation);
                                let entry = dilation_maps.entry(key).or_insert(vec![]);
                                if !entry.contains(&poly_id) {
                                    entry.push(poly_id);
                                }
                            }
                            PolynomialInConstraint::SetupPolynomial(str_id, poly_num, TimeDilation(0)) => {
                                let poly_id = PolyIdentifier::SetupPolynomial(str_id, *poly_num);
                                let key = TimeDilation(0);
                                let entry = dilation_maps.entry(key).or_insert(vec![]);
                                if !entry.contains(&poly_id) {
                                    entry.push(poly_id);
                                }
                            }
                            _ => {
                                unimplemented!()
                            }
                        }
                    }
                }
            }
        }

        let mut keys: Vec<_> = dilation_maps.keys().map(|el| el.clone()).collect();
        keys.sort_by(|a, b| a.0.cmp(&b.0));

        assert!(keys.len() <= 2, "large time dilations are not supported");

        let points_set = vec![z, z_by_omega];

        for (i, key) in keys.into_iter().enumerate() {
            let poly_ids = (&mut dilation_maps).remove(&key).unwrap();
            for id in poly_ids.into_iter() {
                if let Some(setup_poly_idx) = setup.setup_ids.iter().position(|el| el == &id) {
                    assert!(i == 0, "don't support setup polys with dilation yet");
                    let poly_ref = &setup.polynomials_in_monomial_form[setup_poly_idx];
                    let evaluate_at = points_set[i];
                    let value = poly_ref.evaluate_at(&worker, evaluate_at);

                    state.setup_values_at_z[setup_poly_idx] = value;
                } else {
                    if let PolyIdentifier::VariablesPolynomial(state_idx) = id {
                        let poly_ref = &state.witness_polys_in_monomial_form[state_idx];
                        let evaluate_at = points_set[i];
                        let value = poly_ref.evaluate_at(&worker, evaluate_at);
                        if i == 0 {
                            state.wire_values_at_z.push((state_idx, value));
                        } else if i == 1 {
                            state.wire_values_at_z_omega.push((state_idx, value));
                        }
                    } else {
                        unimplemented!()
                    }
                }
            }

            if i == 0 {
                // also open permutation polys and quotient
                for setup_poly_idx in setup.permutations_ranges[0].clone() {
                    let poly_ref = &setup.polynomials_in_monomial_form[setup_poly_idx];
                    let value = poly_ref.evaluate_at(&worker, z);

                    state.permutation_polynomials_at_z.push(value);
                }

                for selector_poly_idx in setup.gate_selectors_indexes.iter() {
                    let poly_ref = &setup.polynomials_in_monomial_form[*selector_poly_idx];
                    let value = poly_ref.evaluate_at(&worker, z);

                    state.gate_selector_polynomials_at_z.push(value);
                }

                // let mut quotient_at_z = E::Fr::zero();
                // let mut current = E::Fr::one();
                // let mut z_in_domain_size = z.pow(&[required_domain_size as u64]);
                for poly_ref in state.t_poly_parts.iter() {
                    let value_of_part = poly_ref.evaluate_at(&worker, z);

                    state.quotient_polynomial_parts_at_z.push(value_of_part);
                }
            }
        }

        state.grand_product_at_z = state.grand_product_polynomial_in_monomial_form[0].evaluate_at(&worker, z);
        state.grand_product_at_z_omega = state.grand_product_polynomial_in_monomial_form[0].evaluate_at(&worker, z_by_omega);

        let message = FourthProverMessage::<E> {
            wire_values_at_z: state.wire_values_at_z.clone(),
            wire_values_at_z_omega: state.wire_values_at_z_omega.clone(),
            setup_values_at_z: state.setup_values_at_z.clone(),
            permutation_polynomials_at_z: state.permutation_polynomials_at_z.clone(),
            gate_selector_polynomials_at_z: state.gate_selector_polynomials_at_z.clone(),
            grand_product_at_z: state.grand_product_at_z,
            grand_product_at_z_omega: state.grand_product_at_z_omega,
            quotient_polynomial_parts_at_z: state.quotient_polynomial_parts_at_z.clone(),
        };

        Ok((state, message))
    }

    fn perform_fri<P: Prng<E::Fr, Input = H::Output>>(
        &self,
        aggregation_challenge: E::Fr,
        witness_opening_requests: Vec<WitnessOpeningRequest<E::Fr>>,
        setup_opening_requests: Vec<SetupOpeningRequest<E::Fr>>,
        worker: &Worker,
        prng: &mut P,
    ) -> Result<FriOraclesSet<E, H>, SynthesisError> {
        let mut len = 0;
        for r in witness_opening_requests.iter() {
            for p in r.polynomials.iter() {
                if len == 0 {
                    len = p.size();
                } else {
                    assert_eq!(p.size(), len, "poly lengths are different!");
                }
            }
        }

        for r in setup_opening_requests.iter() {
            for p in r.polynomials.iter() {
                if len == 0 {
                    len = p.size();
                } else {
                    assert_eq!(p.size(), len, "poly lengths are different!");
                }
            }
        }

        assert!(len != 0);
        let required_divisor_size = len;

        let mut final_aggregate = Polynomial::from_values(vec![E::Fr::zero(); required_divisor_size])?;

        let mut precomputed_bitreversed_coset_divisor = Polynomial::from_values(vec![E::Fr::one(); required_divisor_size])?;
        precomputed_bitreversed_coset_divisor.distribute_powers(&worker, precomputed_bitreversed_coset_divisor.omega);
        precomputed_bitreversed_coset_divisor.scale(&worker, E::Fr::multiplicative_generator());
        precomputed_bitreversed_coset_divisor.bitreverse_enumeration(&worker);

        let mut scratch_space_numerator = final_aggregate.clone();
        let mut scratch_space_denominator = final_aggregate.clone();

        let mut alpha = E::Fr::one();

        for witness_request in witness_opening_requests.iter() {
            let z = witness_request.opening_point;
            let mut minus_z = z;
            minus_z.negate();
            scratch_space_denominator.reuse_allocation(&precomputed_bitreversed_coset_divisor);
            scratch_space_denominator.add_constant(&worker, &minus_z);
            scratch_space_denominator.batch_inversion(&worker)?;
            for (poly, value) in witness_request.polynomials.iter().zip(witness_request.opening_values.iter()) {
                scratch_space_numerator.reuse_allocation(&poly);
                let mut v = *value;
                v.negate();
                scratch_space_numerator.add_constant(&worker, &v);
                scratch_space_numerator.mul_assign(&worker, &scratch_space_denominator);
                if aggregation_challenge != E::Fr::one() {
                    scratch_space_numerator.scale(&worker, alpha);
                }

                final_aggregate.add_assign(&worker, &scratch_space_numerator);

                alpha.mul_assign(&aggregation_challenge);
            }
        }

        for setup_request in setup_opening_requests.iter() {
            // for now assume a single setup point per poly and setup point is the same for all polys
            // (omega - y)(omega - z) = omega^2 - (z+y)*omega + zy

            let setup_point = setup_request.setup_point;
            let opening_point = setup_request.opening_point;

            let mut t0 = setup_point;
            t0.add_assign(&opening_point);
            t0.negate();

            let mut t1 = setup_point;
            t1.mul_assign(&opening_point);

            scratch_space_denominator.reuse_allocation(&precomputed_bitreversed_coset_divisor);
            worker.scope(scratch_space_denominator.as_ref().len(), |scope, chunk| {
                for den in scratch_space_denominator.as_mut().chunks_mut(chunk) {
                    scope.spawn(move |_| {
                        for d in den.iter_mut() {
                            let mut result = *d;
                            result.square();
                            result.add_assign(&t1);

                            let mut tmp = t0;
                            tmp.mul_assign(&d);

                            result.add_assign(&tmp);

                            *d = result;
                        }
                    });
                }
            });

            scratch_space_denominator.batch_inversion(&worker)?;

            // each numerator must have a value removed of the polynomial that interpolates the following points:
            // (setup_x, setup_y)
            // (opening_x, opening_y)
            // such polynomial is linear and has a form e.g setup_y + (X - setup_x) * (witness_y - setup_y) / (witness_x - setup_x)

            for ((poly, value), setup_value) in setup_request.polynomials.iter().zip(setup_request.opening_values.iter()).zip(setup_request.setup_values.iter()) {
                scratch_space_numerator.reuse_allocation(&poly);

                let intercept = setup_value;
                let mut t0 = opening_point;
                t0.sub_assign(&setup_point);

                let mut slope = t0.inverse().expect("must exist");

                let mut t1 = *value;
                t1.sub_assign(&setup_value);

                slope.mul_assign(&t1);

                worker.scope(scratch_space_numerator.as_ref().len(), |scope, chunk| {
                    for (num, omega) in scratch_space_numerator.as_mut().chunks_mut(chunk).zip(precomputed_bitreversed_coset_divisor.as_ref().chunks(chunk)) {
                        scope.spawn(move |_| {
                            for (n, omega) in num.iter_mut().zip(omega.iter()) {
                                let mut result = *omega;
                                result.sub_assign(&setup_point);
                                result.mul_assign(&slope);
                                result.add_assign(&intercept);

                                n.sub_assign(&result);
                            }
                        });
                    }
                });

                scratch_space_numerator.mul_assign(&worker, &scratch_space_denominator);
                if aggregation_challenge != E::Fr::one() {
                    scratch_space_numerator.scale(&worker, alpha);
                }

                final_aggregate.add_assign(&worker, &scratch_space_numerator);

                alpha.mul_assign(&aggregation_challenge);
            }
        }

        let fri_combiner = FriCombiner::initialize_for_domain_size(required_divisor_size, LDE_FACTOR, 1, E::Fr::multiplicative_generator(), FRI_VALUES_PER_LEAF, self.tree_hasher.clone());

        println!("Start making FRI oracles");

        let oracles = fri_combiner.perform_fri_assuming_bitreversed(&final_aggregate.as_ref(), prng, &worker)?;

        Ok(oracles)
    }

    pub(crate) fn fifth_step_from_fourth_step<P: Prng<E::Fr, Input = H::Output>>(
        &self,
        fourth_state: FourthPartialProverState<E, H>,
        fourth_verifier_message: FourthVerifierMessage<E>,
        setup: &SetupMultioracle<E, H>,
        prng: &mut P,
        worker: &Worker,
    ) -> Result<FifthProverMessage<E, H>, SynthesisError> {
        let FourthVerifierMessage { z, v, .. } = fourth_verifier_message;
        let required_domain_size = fourth_state.required_domain_size;

        let domain = Domain::new_for_size(required_domain_size as u64)?;

        let mut z_by_omega = z;
        z_by_omega.mul_assign(&domain.generator);

        // now we need to sort polynomials and gates by
        // - first filter setup polynomials
        // - each setup is opened separately at reference point and required point
        // - then filter witness polys
        // - open them at every required point

        println!("Start making setup opening requests");

        let mut setup_opening_requests = vec![];

        // TODO: do better

        {
            let mut setup_values = vec![];
            let mut setup_poly_refs = vec![];

            for (i, _) in setup.setup_ids.iter().enumerate() {
                setup_values.push(setup.setup_poly_values[i]);
                setup_poly_refs.push(&setup.polynomial_ldes[i]);
            }

            let range_of_permutation_polys = setup.permutations_ranges[0].clone();

            for (value, perm_ref) in setup.setup_poly_values[range_of_permutation_polys.clone()]
                .iter()
                .zip(setup.polynomial_ldes[range_of_permutation_polys].iter())
            {
                setup_values.push(*value);
                setup_poly_refs.push(perm_ref);
            }

            for selector_poly_idx in setup.gate_selectors_indexes.iter() {
                let poly_ref = &setup.polynomial_ldes[*selector_poly_idx];
                let value = setup.setup_poly_values[*selector_poly_idx];

                setup_values.push(value);
                setup_poly_refs.push(poly_ref);
            }

            let mut opening_values = vec![];
            opening_values.extend_from_slice(&fourth_state.setup_values_at_z[..]);
            opening_values.extend_from_slice(&fourth_state.permutation_polynomials_at_z[..]);
            opening_values.extend_from_slice(&fourth_state.gate_selector_polynomials_at_z[..]);

            assert_eq!(setup_values.len(), opening_values.len(), "number of setup values is not equal to number of opening values");

            let request = SetupOpeningRequest {
                polynomials: setup_poly_refs,
                setup_point: setup.setup_point,
                setup_values: setup_values,
                opening_point: z,
                opening_values: opening_values,
            };

            setup_opening_requests.push(request);
        }

        println!("Start making witness opening assignments");

        let mut witness_opening_requests = vec![];

        let opening_points = vec![z, z_by_omega];
        let storages = vec![&fourth_state.wire_values_at_z, &fourth_state.wire_values_at_z_omega];

        for dilation in 0usize..=1usize {
            let mut per_dilation_set = vec![];
            for gate in self.sorted_gates.iter() {
                for constraint in gate.get_constraints().iter() {
                    for term in constraint.0.iter() {
                        for poly in term.1.iter() {
                            match poly {
                                PolynomialInConstraint::VariablesPolynomial(poly_num, TimeDilation(dil)) => {
                                    if dil == &dilation {
                                        if !per_dilation_set.contains(poly_num) {
                                            per_dilation_set.push(*poly_num)
                                        }
                                    }
                                }
                                _ => {}
                            }
                        }
                    }
                }
            }

            let mut opening_values = vec![];
            let mut opening_refs = vec![];
            let open_at = opening_points[dilation];
            let storage = storages[dilation];
            for id in per_dilation_set.into_iter() {
                let poly_ref = &fourth_state.witness_polys_ldes[id];

                let mut tmp: Vec<_> = storage.iter().filter(|el| el.0 == id).collect();

                assert_eq!(tmp.len(), 1);
                let value = tmp.pop().unwrap().1;

                opening_values.push(value);
                opening_refs.push(poly_ref);
            }

            if dilation == 0 {
                opening_values.push(fourth_state.grand_product_at_z);
                opening_refs.push(&fourth_state.grand_product_polynomial_lde[0]);
            } else if dilation == 1 {
                opening_values.push(fourth_state.grand_product_at_z_omega);
                opening_refs.push(&fourth_state.grand_product_polynomial_lde[0]);
            }

            let request = WitnessOpeningRequest {
                polynomials: opening_refs,
                opening_point: open_at,
                opening_values: opening_values,
            };

            witness_opening_requests.push(request);
        }

        let fri_oracles_set = self.perform_fri(v, witness_opening_requests, setup_opening_requests, &worker, prng)?;

        let commitments = fri_oracles_set.intermediate_roots.clone();
        let coeffs = fri_oracles_set.final_coefficients.clone();

        let message = FifthProverMessage {
            fri_intermediate_roots: commitments,
            final_coefficients: coeffs,
        };

        let num_queries = 32;

        use super::multioracle::Multioracle;

        {
            let idx_start = 0;
            let indexes: Vec<usize> = (idx_start..(idx_start + FRI_VALUES_PER_LEAF)).collect();

            let setup_tree_params = setup.tree.params.clone();
            let witness_tree_params = fourth_state.witness_multioracle_tree.params.clone();
            let grand_product_tree_params = fourth_state.grand_product_polynomial_multioracle_tree.params.clone();
            let t_poly_tree_params = fourth_state.t_poly_parts_multioracle_tree.params.clone();
            let mut fri_subtrees_params = vec![];
            for s in fri_oracles_set.intermediate_oracles.iter() {
                fri_subtrees_params.push(s.params.clone());
            }

            let setup_query = setup.tree.produce_multiquery(
                indexes.clone(),
                setup.polynomial_ldes.len(),
                &Multioracle::<E, H>::combine_leafs(&setup.polynomial_ldes, FRI_VALUES_PER_LEAF, &worker),
            );

            let witness_query = fourth_state.witness_multioracle_tree.produce_multiquery(
                indexes.clone(),
                fourth_state.witness_polys_ldes.len(),
                &Multioracle::<E, H>::combine_leafs(&fourth_state.witness_polys_ldes, FRI_VALUES_PER_LEAF, &worker),
            );

            let grand_product_query = fourth_state.grand_product_polynomial_multioracle_tree.produce_multiquery(
                indexes.clone(),
                fourth_state.grand_product_polynomial_lde.len(),
                &Multioracle::<E, H>::combine_leafs(&fourth_state.grand_product_polynomial_lde, FRI_VALUES_PER_LEAF, &worker),
            );

            let quotient_query = fourth_state.t_poly_parts_multioracle_tree.produce_multiquery(
                indexes.clone(),
                fourth_state.t_poly_parts_ldes.len(),
                &Multioracle::<E, H>::combine_leafs(&fourth_state.t_poly_parts_ldes, FRI_VALUES_PER_LEAF, &worker),
            );

            let mut fri_queries = vec![];
            for ((vals, tree), params) in fri_oracles_set
                .intermediate_leaf_values
                .iter()
                .zip(fri_oracles_set.intermediate_oracles.iter())
                .zip(fri_subtrees_params.iter())
            {
                let idx_start = 0;
                let indexes: Vec<usize> = (idx_start..(idx_start + params.values_per_leaf)).collect();

                let query = tree.produce_query(indexes, &vals);

                fri_queries.push(query);
            }

            let hasher = setup.tree.tree_hasher.clone();

            let _ = BinaryTree::verify_multiquery(&setup.tree.get_commitment(), &setup_query, &setup_tree_params, &hasher);

            let _ = BinaryTree::verify_multiquery(&setup.tree.get_commitment(), &witness_query, &witness_tree_params, &hasher);

            let _ = BinaryTree::verify_multiquery(&setup.tree.get_commitment(), &grand_product_query, &grand_product_tree_params, &hasher);

            let _ = BinaryTree::verify_multiquery(&setup.tree.get_commitment(), &quotient_query, &t_poly_tree_params, &hasher);

            for (query, params) in fri_queries.into_iter().zip(fri_subtrees_params.iter()) {
                let _ = BinaryTree::verify_query(&setup.tree.get_commitment(), &query, &params, &hasher);
            }
        }

        Ok(message)
    }
}
