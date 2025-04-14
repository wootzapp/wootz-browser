use super::*;

pub(crate) enum PrecomputationsForPolynomial<'a, F: PrimeField> {
    Borrowed(&'a Polynomial<F, Values>),
    Owned(Polynomial<F, Values>),
    None,
}

impl<'a, F: PrimeField> AsRef<Polynomial<F, Values>> for PrecomputationsForPolynomial<'a, F> {
    fn as_ref(&self) -> &Polynomial<F, Values> {
        match self {
            PrecomputationsForPolynomial::Borrowed(b) => b,
            PrecomputationsForPolynomial::Owned(o) => &o,
            PrecomputationsForPolynomial::None => {
                unreachable!("precomputations must have been made");
            }
        }
    }
}

impl<'a, F: PrimeField> PrecomputationsForPolynomial<'a, F> {
    pub(crate) fn into_poly(self) -> Polynomial<F, Values> {
        match self {
            PrecomputationsForPolynomial::Borrowed(b) => b.clone(),
            PrecomputationsForPolynomial::Owned(o) => o,
            PrecomputationsForPolynomial::None => {
                unreachable!("precomputations must have been made");
            }
        }
    }
}

pub(crate) fn get_precomputed_permutation_poly_lde_for_index<'a, E: Engine, CP: CTPrecomputations<E::Fr>>(
    index: usize,
    domain_size: usize,
    setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
    setup_precomputations: &Option<&'a SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
    precomputed_omegas: &mut PrecomputedOmegas<E::Fr, CP>,
    worker: &Worker,
) -> Result<PrecomputationsForPolynomial<'a, E::Fr>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();

    if let Some(prec) = setup_precomputations {
        let p = &prec.permutation_polynomials_on_coset_of_size_4n_bitreversed[index];

        return Ok(PrecomputationsForPolynomial::Borrowed(p));
    } else {
        match &*precomputed_omegas {
            PrecomputedOmegas::None => {
                *precomputed_omegas = PrecomputedOmegas::Owned(CP::new_for_domain_size(domain_size), E::Fr::one());
            }
            _ => {}
        }

        let p = setup.permutation_polynomials[index]
            .clone()
            .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

        return Ok(PrecomputationsForPolynomial::Owned(p));
    }
}

pub(crate) fn get_precomputed_selector_lde_for_index<'a, E: Engine, CP: CTPrecomputations<E::Fr>>(
    index: usize,
    domain_size: usize,
    setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
    setup_precomputations: &Option<&'a SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
    precomputed_omegas: &mut PrecomputedOmegas<E::Fr, CP>,
    worker: &Worker,
) -> Result<PrecomputationsForPolynomial<'a, E::Fr>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();

    if let Some(prec) = setup_precomputations {
        let p = &prec.selector_polynomials_on_coset_of_size_4n_bitreversed[index];

        return Ok(PrecomputationsForPolynomial::Borrowed(p));
    } else {
        match &*precomputed_omegas {
            PrecomputedOmegas::None => {
                *precomputed_omegas = PrecomputedOmegas::Owned(CP::new_for_domain_size(domain_size), E::Fr::one());
            }
            _ => {}
        }

        let p = setup.selector_polynomials[index]
            .clone()
            .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

        return Ok(PrecomputationsForPolynomial::Owned(p));
    }
}

pub(crate) fn get_precomputed_next_step_selector_lde_for_index<'a, E: Engine, CP: CTPrecomputations<E::Fr>>(
    index: usize,
    domain_size: usize,
    setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
    setup_precomputations: &Option<&'a SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
    precomputed_omegas: &mut PrecomputedOmegas<E::Fr, CP>,
    worker: &Worker,
) -> Result<PrecomputationsForPolynomial<'a, E::Fr>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();

    if let Some(prec) = setup_precomputations {
        let p = &prec.next_step_selector_polynomials_on_coset_of_size_4n_bitreversed[index];

        return Ok(PrecomputationsForPolynomial::Borrowed(p));
    } else {
        match &*precomputed_omegas {
            PrecomputedOmegas::None => {
                *precomputed_omegas = PrecomputedOmegas::Owned(CP::new_for_domain_size(domain_size), E::Fr::one());
            }
            _ => {}
        }

        let p = setup.next_step_selector_polynomials[index]
            .clone()
            .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

        return Ok(PrecomputationsForPolynomial::Owned(p));
    }
}

pub(crate) fn get_precomputed_x_lde<'a, E: Engine>(
    domain_size: usize,
    setup_precomputations: &Option<&'a SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
    worker: &Worker,
) -> Result<PrecomputationsForPolynomial<'a, E::Fr>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();

    if let Some(prec) = setup_precomputations {
        let p = &prec.x_on_coset_of_size_4n_bitreversed;

        return Ok(PrecomputationsForPolynomial::Borrowed(p));
    } else {
        let mut x_poly = Polynomial::from_values(vec![coset_factor; domain_size * LDE_FACTOR])?;
        x_poly.distribute_powers(&worker, x_poly.omega);
        x_poly.bitreverse_enumeration(&worker);

        return Ok(PrecomputationsForPolynomial::Owned(x_poly));
    }
}

pub(crate) fn get_precomputed_inverse_divisor<'a, E: Engine>(
    domain_size: usize,
    setup_precomputations: &Option<&'a SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
    worker: &Worker,
) -> Result<PrecomputationsForPolynomial<'a, E::Fr>, SynthesisError> {
    let coset_factor = E::Fr::multiplicative_generator();

    if let Some(prec) = setup_precomputations {
        let p = &prec.inverse_divisor_on_coset_of_size_4n_bitreversed;

        return Ok(PrecomputationsForPolynomial::Borrowed(p));
    } else {
        let mut vanishing_poly_inverse_bitreversed = evaluate_vanishing_polynomial_of_degree_on_domain_size::<E::Fr>(domain_size as u64, &coset_factor, (domain_size * LDE_FACTOR) as u64, &worker)?;
        vanishing_poly_inverse_bitreversed.batch_inversion(&worker)?;
        vanishing_poly_inverse_bitreversed.bitreverse_enumeration(&worker);

        return Ok(PrecomputationsForPolynomial::Owned(vanishing_poly_inverse_bitreversed));
    }
}

pub(crate) enum PrecomputedOmegas<'a, F: PrimeField, CP: CTPrecomputations<F>> {
    Borrowed(&'a CP, F),
    Owned(CP, F),
    None,
}

impl<'a, F: PrimeField, CP: CTPrecomputations<F>> AsRef<CP> for PrecomputedOmegas<'a, F, CP> {
    fn as_ref(&self) -> &CP {
        match self {
            PrecomputedOmegas::Borrowed(b, _) => b,
            PrecomputedOmegas::Owned(o, _) => &o,
            PrecomputedOmegas::None => {
                unreachable!("precomputations must have been made");
            }
        }
    }
}

pub(crate) struct FirstPartialProverState<E: Engine, P: PlonkConstraintSystemParams<E>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_as_coeffs: Vec<Polynomial<E::Fr, Coefficients>>,
    witness_polys_unpadded_values: Vec<Polynomial<E::Fr, Values>>,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct FirstProverMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) n: usize,
    pub(crate) num_inputs: usize,
    pub(crate) input_values: Vec<E::Fr>,
    pub(crate) wire_commitments: Vec<E::G1Affine>,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct FirstVerifierMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,

    pub(crate) _marker: std::marker::PhantomData<P>,
}

pub(crate) struct SecondPartialProverState<E: Engine, P: PlonkConstraintSystemParams<E>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_as_coeffs: Vec<Polynomial<E::Fr, Coefficients>>,
    z_in_monomial_form: Polynomial<E::Fr, Coefficients>,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct SecondProverMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) z_commitment: E::G1Affine,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct SecondVerifierMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) alpha: E::Fr,
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,

    pub(crate) _marker: std::marker::PhantomData<P>,
}

pub(crate) struct ThirdPartialProverState<E: Engine, P: PlonkConstraintSystemParams<E>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_as_coeffs: Vec<Polynomial<E::Fr, Coefficients>>,
    z_in_monomial_form: Polynomial<E::Fr, Coefficients>,
    t_poly_parts: Vec<Polynomial<E::Fr, Coefficients>>,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct ThirdProverMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) quotient_poly_commitments: Vec<E::G1Affine>,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct ThirdVerifierMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) alpha: E::Fr,
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,
    pub(crate) z: E::Fr,

    pub(crate) _marker: std::marker::PhantomData<P>,
}

pub(crate) struct FourthPartialProverState<E: Engine, P: PlonkConstraintSystemParams<E>> {
    required_domain_size: usize,
    non_residues: Vec<E::Fr>,
    input_values: Vec<E::Fr>,
    witness_polys_as_coeffs: Vec<Polynomial<E::Fr, Coefficients>>,
    z_in_monomial_form: Polynomial<E::Fr, Coefficients>,
    t_poly_parts: Vec<Polynomial<E::Fr, Coefficients>>,
    linearization_polynomial: Polynomial<E::Fr, Coefficients>,
    wire_values_at_z: Vec<E::Fr>,
    wire_values_at_z_omega: Vec<E::Fr>,
    permutation_polynomials_at_z: Vec<E::Fr>,
    grand_product_at_z_omega: E::Fr,
    quotient_polynomial_at_z: E::Fr,
    linearization_polynomial_at_z: E::Fr,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct FourthProverMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) wire_values_at_z: Vec<E::Fr>,
    pub(crate) wire_values_at_z_omega: Vec<E::Fr>,
    pub(crate) permutation_polynomials_at_z: Vec<E::Fr>,
    pub(crate) grand_product_at_z_omega: E::Fr,
    pub(crate) quotient_polynomial_at_z: E::Fr,
    pub(crate) linearization_polynomial_at_z: E::Fr,

    _marker: std::marker::PhantomData<P>,
}

pub(crate) struct FourthVerifierMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) alpha: E::Fr,
    pub(crate) beta: E::Fr,
    pub(crate) gamma: E::Fr,
    pub(crate) z: E::Fr,
    pub(crate) v: E::Fr,

    pub(crate) _marker: std::marker::PhantomData<P>,
}

pub(crate) struct FifthProverMessage<E: Engine, P: PlonkConstraintSystemParams<E>> {
    pub(crate) opening_proof_at_z: E::G1Affine,
    pub(crate) opening_proof_at_z_omega: E::G1Affine,

    _marker: std::marker::PhantomData<P>,
}

impl<E: Engine> ProverAssembly4WithNextStep<E> {
    pub(crate) fn first_step_with_lagrange_form_key(
        self,
        worker: &Worker,
        crs_vals: &Crs<E, CrsForLagrangeForm>,
    ) -> Result<(FirstPartialProverState<E, PlonkCsWidth4WithNextStepParams>, FirstProverMessage<E, PlonkCsWidth4WithNextStepParams>), SynthesisError> {
        use crate::pairing::CurveAffine;
        use std::sync::Arc;

        assert!(self.is_finalized);

        let input_values = self.input_assingments.clone();

        let n = self.n;
        let num_inputs = self.num_inputs;

        let required_domain_size = n + 1;
        assert!(required_domain_size.is_power_of_two());

        let non_residues = make_non_residues::<E::Fr>(<PlonkCsWidth4WithNextStepParams as PlonkConstraintSystemParams<E>>::STATE_WIDTH - 1);

        let full_assignments = self.make_witness_polynomials()?;

        // Commit wire polynomials

        let mut first_message = FirstProverMessage::<E, PlonkCsWidth4WithNextStepParams> {
            n: n,
            num_inputs: num_inputs,
            input_values: input_values.clone(),
            wire_commitments: Vec::with_capacity(4),

            _marker: std::marker::PhantomData,
        };

        for wire_poly in full_assignments.iter() {
            let commitment = commit_using_raw_values(&wire_poly, &crs_vals, &worker)?;

            first_message.wire_commitments.push(commitment);
        }

        // now transform assignments in the polynomials

        let mut assignment_polynomials = vec![];
        for p in full_assignments.into_iter() {
            let p = Polynomial::from_values_unpadded(p)?;
            assignment_polynomials.push(p);
        }

        let state = FirstPartialProverState::<E, PlonkCsWidth4WithNextStepParams> {
            required_domain_size,
            non_residues,
            input_values: input_values.clone(),
            witness_polys_as_coeffs: vec![],
            witness_polys_unpadded_values: assignment_polynomials,

            _marker: std::marker::PhantomData,
        };

        Ok((state, first_message))
    }

    pub(crate) fn first_step_with_monomial_form_key<CPI: CTPrecomputations<E::Fr>>(
        self,
        worker: &Worker,
        crs_mons: &Crs<E, CrsForMonomialForm>,
        precomputed_omegas_inv: &mut PrecomputedOmegas<E::Fr, CPI>,
    ) -> Result<(FirstPartialProverState<E, PlonkCsWidth4WithNextStepParams>, FirstProverMessage<E, PlonkCsWidth4WithNextStepParams>), SynthesisError> {
        use crate::pairing::CurveAffine;
        use std::sync::Arc;

        assert!(self.is_finalized);

        let input_values = self.input_assingments.clone();

        let n = self.n;
        let num_inputs = self.num_inputs;

        let required_domain_size = n + 1;
        assert!(required_domain_size.is_power_of_two());

        let non_residues = make_non_residues::<E::Fr>(<PlonkCsWidth4WithNextStepParams as PlonkConstraintSystemParams<E>>::STATE_WIDTH - 1);

        let full_assignments = self.make_witness_polynomials()?;

        // Commit wire polynomials

        let mut first_message = FirstProverMessage::<E, PlonkCsWidth4WithNextStepParams> {
            n: n,
            num_inputs: num_inputs,
            input_values: input_values.clone(),
            wire_commitments: Vec::with_capacity(4),

            _marker: std::marker::PhantomData,
        };

        let mut wire_polys_as_coefficients = Vec::with_capacity(full_assignments.len());

        match &*precomputed_omegas_inv {
            PrecomputedOmegas::None => {
                *precomputed_omegas_inv = PrecomputedOmegas::Owned(CPI::new_for_domain_size(required_domain_size), E::Fr::one());
            }
            _ => {}
        }

        for wire_poly in full_assignments.iter() {
            let as_poly = Polynomial::from_values(wire_poly.clone())?;
            let as_coeffs = as_poly.ifft_using_bitreversed_ntt(&worker, precomputed_omegas_inv.as_ref(), &E::Fr::one())?;

            let commitment = commit_using_monomials(&as_coeffs, &crs_mons, &worker)?;

            wire_polys_as_coefficients.push(as_coeffs);

            first_message.wire_commitments.push(commitment);
        }

        // now transform assignments in the polynomials

        let mut assignment_polynomials = vec![];
        for p in full_assignments.into_iter() {
            let p = Polynomial::from_values_unpadded(p)?;
            assignment_polynomials.push(p);
        }

        let state = FirstPartialProverState::<E, PlonkCsWidth4WithNextStepParams> {
            required_domain_size,
            non_residues,
            input_values: input_values.clone(),
            witness_polys_as_coeffs: wire_polys_as_coefficients,
            witness_polys_unpadded_values: assignment_polynomials,

            _marker: std::marker::PhantomData,
        };

        Ok((state, first_message))
    }

    pub(crate) fn second_step_from_first_step<CPI: CTPrecomputations<E::Fr>>(
        first_state: FirstPartialProverState<E, PlonkCsWidth4WithNextStepParams>,
        first_verifier_message: FirstVerifierMessage<E, PlonkCsWidth4WithNextStepParams>,
        setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
        crs_mons: &Crs<E, CrsForMonomialForm>,
        setup_precomputations: &Option<&SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
        precomputed_omegas_inv: &mut PrecomputedOmegas<E::Fr, CPI>,
        worker: &Worker,
    ) -> Result<(SecondPartialProverState<E, PlonkCsWidth4WithNextStepParams>, SecondProverMessage<E, PlonkCsWidth4WithNextStepParams>), SynthesisError> {
        let FirstVerifierMessage { beta, gamma, .. } = first_verifier_message;

        assert_eq!(first_state.witness_polys_unpadded_values.len(), 4, "first state must containt assignment poly values");

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

        let mut permutation_polynomials_values_of_size_n_minus_one = vec![];

        if let Some(prec) = setup_precomputations {
            for p in prec.permutation_polynomials_values_of_size_n_minus_one.iter() {
                let pp = PrecomputationsForPolynomial::Borrowed(p);
                permutation_polynomials_values_of_size_n_minus_one.push(pp);
            }
        } else {
            match &*precomputed_omegas_inv {
                PrecomputedOmegas::None => {
                    *precomputed_omegas_inv = PrecomputedOmegas::Owned(CPI::new_for_domain_size(required_domain_size), E::Fr::one());
                }
                _ => {}
            }

            // we need to only do up to the last one
            for p in setup.permutation_polynomials.iter() {
                let as_values = p.clone().fft(&worker);
                let mut as_values = as_values.into_coeffs();
                as_values.pop().expect("must shorted permutation polynomial values by one");

                let p = Polynomial::from_values_unpadded(as_values)?;
                let p = PrecomputationsForPolynomial::Owned(p);

                permutation_polynomials_values_of_size_n_minus_one.push(p);
            }
        }

        let z_den = {
            assert_eq!(permutation_polynomials_values_of_size_n_minus_one.len(), grand_products_protos_with_gamma.len());
            let mut grand_products_proto_it = grand_products_protos_with_gamma.into_iter();
            let mut permutation_polys_it = permutation_polynomials_values_of_size_n_minus_one.iter();

            let mut z_2 = grand_products_proto_it.next().unwrap();
            z_2.add_assign_scaled(&worker, permutation_polys_it.next().unwrap().as_ref(), &beta);

            for (mut p, perm) in grand_products_proto_it.zip(permutation_polys_it) {
                // permutation polynomials
                p.add_assign_scaled(&worker, perm.as_ref(), &beta);
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
        // println!("Z last = {}", z.as_ref().last().unwrap());
        // assert!(z.as_ref().last().expect("must exist") == &E::Fr::one());

        // interpolate on the main domain
        let z_in_monomial_form = z.ifft_using_bitreversed_ntt(&worker, precomputed_omegas_inv.as_ref(), &E::Fr::one())?;

        let z_commitment = commit_using_monomials(&z_in_monomial_form, &crs_mons, &worker)?;

        let state = SecondPartialProverState::<E, PlonkCsWidth4WithNextStepParams> {
            required_domain_size,
            non_residues: first_state.non_residues,
            input_values: first_state.input_values,
            witness_polys_as_coeffs: first_state.witness_polys_as_coeffs,
            z_in_monomial_form: z_in_monomial_form,

            _marker: std::marker::PhantomData,
        };

        let message = SecondProverMessage::<E, PlonkCsWidth4WithNextStepParams> {
            z_commitment: z_commitment,
            _marker: std::marker::PhantomData,
        };

        Ok((state, message))
    }

    pub(crate) fn third_step_from_second_step<CP: CTPrecomputations<E::Fr>, CPI: CTPrecomputations<E::Fr>>(
        second_state: SecondPartialProverState<E, PlonkCsWidth4WithNextStepParams>,
        second_verifier_message: SecondVerifierMessage<E, PlonkCsWidth4WithNextStepParams>,
        setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
        crs_mons: &Crs<E, CrsForMonomialForm>,
        setup_precomputations: &Option<&SetupPolynomialsPrecomputations<E, PlonkCsWidth4WithNextStepParams>>,
        precomputed_omegas: &mut PrecomputedOmegas<E::Fr, CP>,
        precomputed_omegas_inv: &mut PrecomputedOmegas<E::Fr, CPI>,
        worker: &Worker,
    ) -> Result<(ThirdPartialProverState<E, PlonkCsWidth4WithNextStepParams>, ThirdProverMessage<E, PlonkCsWidth4WithNextStepParams>), SynthesisError> {
        let z_in_monomial_form = second_state.z_in_monomial_form;

        // those are z(x*Omega) formally
        let mut z_shifted_in_monomial_form = z_in_monomial_form.clone();
        z_shifted_in_monomial_form.distribute_powers(&worker, z_in_monomial_form.omega);

        // now we have to LDE everything and compute quotient polynomial
        // also to save on openings that we will have to do from the monomial form anyway

        let witness_polys_in_monomial_form = second_state.witness_polys_as_coeffs;

        let required_domain_size = second_state.required_domain_size;
        let coset_factor = E::Fr::multiplicative_generator();

        match &*precomputed_omegas {
            PrecomputedOmegas::None => {
                *precomputed_omegas = PrecomputedOmegas::Owned(CP::new_for_domain_size(required_domain_size), E::Fr::one());
            }
            _ => {}
        }

        match &*precomputed_omegas_inv {
            PrecomputedOmegas::None => {
                *precomputed_omegas_inv = PrecomputedOmegas::Owned(CPI::new_for_domain_size(required_domain_size), E::Fr::one());
            }
            _ => {}
        }

        let mut witness_ldes_on_coset = vec![];
        let mut witness_next_ldes_on_coset = vec![];

        for (idx, monomial) in witness_polys_in_monomial_form.iter().enumerate() {
            // this is D polynomial and we need to make next
            if idx == <PlonkCsWidth4WithNextStepParams as PlonkConstraintSystemParams<E>>::STATE_WIDTH - 1 {
                let mut d_next = monomial.clone();
                d_next.distribute_powers(&worker, d_next.omega);

                let lde = d_next.bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

                witness_next_ldes_on_coset.push(lde);
            }

            let lde = monomial
                .clone()
                .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;
            witness_ldes_on_coset.push(lde);
        }

        let SecondVerifierMessage { alpha, beta, gamma, .. } = second_verifier_message;

        // calculate first part of the quotient polynomial - the gate itself
        // A + B + C + D + AB + CONST + D_NEXT == 0 everywhere but on the last point of the domain

        let mut quotient_linearization_challenge = E::Fr::one();
        let input_values = second_state.input_values;

        let (mut t_1, mut tmp) = {
            // Include the public inputs
            let mut inputs_poly = Polynomial::<E::Fr, Values>::new_for_size(required_domain_size)?;
            for (idx, &input) in input_values.iter().enumerate() {
                inputs_poly.as_mut()[idx] = input;
            }
            // go into monomial form

            let mut inputs_poly = inputs_poly.ifft_using_bitreversed_ntt(&worker, precomputed_omegas_inv.as_ref(), &E::Fr::one())?;

            // add constants selectors vector
            inputs_poly.add_assign(&worker, setup.selector_polynomials.last().unwrap());

            // LDE
            let mut t_1 = inputs_poly.bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

            // t_1 is now q_constant

            // Q_A * A
            let mut tmp = witness_ldes_on_coset[0].clone();
            let a_selector = get_precomputed_selector_lde_for_index(0, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
            tmp.mul_assign(&worker, &a_selector.as_ref());
            t_1.add_assign(&worker, &tmp);
            drop(a_selector);

            // Q_B * B
            tmp.reuse_allocation(&witness_ldes_on_coset[1]);
            let b_selector = get_precomputed_selector_lde_for_index(1, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
            tmp.mul_assign(&worker, &b_selector.as_ref());
            t_1.add_assign(&worker, &tmp);
            drop(b_selector);

            // Q_C * C
            tmp.reuse_allocation(&witness_ldes_on_coset[2]);
            let c_selector = get_precomputed_selector_lde_for_index(2, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
            tmp.mul_assign(&worker, c_selector.as_ref());
            t_1.add_assign(&worker, &tmp);
            drop(c_selector);

            // Q_D * D
            tmp.reuse_allocation(&witness_ldes_on_coset[3]);
            let d_selector = get_precomputed_selector_lde_for_index(3, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
            tmp.mul_assign(&worker, d_selector.as_ref());
            t_1.add_assign(&worker, &tmp);
            drop(d_selector);

            // Q_M * A * B
            tmp.reuse_allocation(&witness_ldes_on_coset[0]);
            tmp.mul_assign(&worker, &witness_ldes_on_coset[1]);
            let m_selector = get_precomputed_selector_lde_for_index(4, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
            tmp.mul_assign(&worker, &m_selector.as_ref());
            t_1.add_assign(&worker, &tmp);
            drop(m_selector);

            tmp.reuse_allocation(&witness_next_ldes_on_coset[0]);
            let d_next_selector = get_precomputed_next_step_selector_lde_for_index(0, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
            tmp.mul_assign(&worker, d_next_selector.as_ref());
            t_1.add_assign(&worker, &tmp);
            drop(d_next_selector);

            (t_1, tmp)
        };

        // drop(witness_ldes_on_coset);
        drop(witness_next_ldes_on_coset);

        // now compute the permutation argument

        let z_coset_lde_bitreversed = z_in_monomial_form
            .clone()
            .bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

        assert!(z_coset_lde_bitreversed.size() == required_domain_size * LDE_FACTOR);

        let z_shifted_coset_lde_bitreversed = z_shifted_in_monomial_form.bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

        assert!(z_shifted_coset_lde_bitreversed.size() == required_domain_size * LDE_FACTOR);

        let non_residues = make_non_residues::<E::Fr>(<PlonkCsWidth4WithNextStepParams as PlonkConstraintSystemParams<E>>::STATE_WIDTH - 1);

        // For both Z_1 and Z_2 we first check for grand products
        // z*(X)(A + beta*X + gamma)(B + beta*k_1*X + gamma)(C + beta*K_2*X + gamma) -
        // - (A + beta*perm_a(X) + gamma)(B + beta*perm_b(X) + gamma)(C + beta*perm_c(X) + gamma)*Z(X*Omega)== 0

        // we use evaluations of the polynomial X and K_i * X on a large domain's coset

        quotient_linearization_challenge.mul_assign(&alpha);

        {
            let mut contrib_z = z_coset_lde_bitreversed.clone();

            // A + beta*X + gamma

            tmp.reuse_allocation(&witness_ldes_on_coset[0]);
            tmp.add_constant(&worker, &gamma);
            let x_precomp = get_precomputed_x_lde(required_domain_size, setup_precomputations, &worker)?;
            tmp.add_assign_scaled(&worker, x_precomp.as_ref(), &beta);
            contrib_z.mul_assign(&worker, &tmp);

            assert_eq!(non_residues.len() + 1, witness_ldes_on_coset.len());

            for (w, non_res) in witness_ldes_on_coset[1..].iter().zip(non_residues.iter()) {
                let mut factor = beta;
                factor.mul_assign(&non_res);

                tmp.reuse_allocation(&w);
                tmp.add_constant(&worker, &gamma);
                tmp.add_assign_scaled(&worker, x_precomp.as_ref(), &factor);
                contrib_z.mul_assign(&worker, &tmp);
            }

            t_1.add_assign_scaled(&worker, &contrib_z, &quotient_linearization_challenge);

            drop(contrib_z);

            let mut contrib_z = z_shifted_coset_lde_bitreversed;

            // A + beta*perm_a + gamma

            for (idx, w) in witness_ldes_on_coset.iter().enumerate() {
                let perm = get_precomputed_permutation_poly_lde_for_index(idx, required_domain_size, &setup, &setup_precomputations, precomputed_omegas, &worker)?;
                tmp.reuse_allocation(&w);
                tmp.add_constant(&worker, &gamma);
                tmp.add_assign_scaled(&worker, perm.as_ref(), &beta);
                contrib_z.mul_assign(&worker, &tmp);
            }

            t_1.sub_assign_scaled(&worker, &contrib_z, &quotient_linearization_challenge);

            drop(contrib_z);
        }

        quotient_linearization_challenge.mul_assign(&alpha);

        // z(omega^0) - 1 == 0

        let l_0 = calculate_lagrange_poly::<E::Fr>(&worker, required_domain_size.next_power_of_two(), 0)?;

        {
            let mut z_minus_one_by_l_0 = z_coset_lde_bitreversed;
            z_minus_one_by_l_0.sub_constant(&worker, &E::Fr::one());

            let l_coset_lde_bitreversed = l_0.bitreversed_lde_using_bitreversed_ntt(&worker, LDE_FACTOR, precomputed_omegas.as_ref(), &coset_factor)?;

            z_minus_one_by_l_0.mul_assign(&worker, &l_coset_lde_bitreversed);

            t_1.add_assign_scaled(&worker, &z_minus_one_by_l_0, &quotient_linearization_challenge);

            drop(z_minus_one_by_l_0);
        }

        drop(tmp);

        let divisor_inversed = get_precomputed_inverse_divisor(required_domain_size, setup_precomputations, &worker)?;
        t_1.mul_assign(&worker, divisor_inversed.as_ref());

        t_1.bitreverse_enumeration(&worker);

        let t_poly_in_monomial_form = t_1.icoset_fft_for_generator(&worker, &E::Fr::multiplicative_generator());

        let t_poly_parts = t_poly_in_monomial_form.break_into_multiples(required_domain_size)?;

        let state = ThirdPartialProverState::<E, PlonkCsWidth4WithNextStepParams> {
            required_domain_size,
            non_residues: second_state.non_residues,
            input_values: input_values,
            witness_polys_as_coeffs: witness_polys_in_monomial_form,
            z_in_monomial_form: z_in_monomial_form,
            t_poly_parts: t_poly_parts,

            _marker: std::marker::PhantomData,
        };

        let mut message = ThirdProverMessage::<E, PlonkCsWidth4WithNextStepParams> {
            quotient_poly_commitments: Vec::with_capacity(4),
            _marker: std::marker::PhantomData,
        };

        for t_part in state.t_poly_parts.iter() {
            let t_part_commitment = commit_using_monomials(&t_part, &crs_mons, &worker)?;

            message.quotient_poly_commitments.push(t_part_commitment);
        }

        Ok((state, message))
    }

    pub(crate) fn fourth_step_from_third_step(
        third_state: ThirdPartialProverState<E, PlonkCsWidth4WithNextStepParams>,
        third_verifier_message: ThirdVerifierMessage<E, PlonkCsWidth4WithNextStepParams>,
        setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
        worker: &Worker,
    ) -> Result<(FourthPartialProverState<E, PlonkCsWidth4WithNextStepParams>, FourthProverMessage<E, PlonkCsWidth4WithNextStepParams>), SynthesisError> {
        let ThirdVerifierMessage { alpha, beta, gamma, z, .. } = third_verifier_message;
        let required_domain_size = third_state.required_domain_size;

        let domain = Domain::new_for_size(required_domain_size as u64)?;

        let mut state = FourthPartialProverState::<E, PlonkCsWidth4WithNextStepParams> {
            required_domain_size,
            non_residues: third_state.non_residues,
            input_values: third_state.input_values,
            witness_polys_as_coeffs: third_state.witness_polys_as_coeffs,
            z_in_monomial_form: third_state.z_in_monomial_form,
            t_poly_parts: third_state.t_poly_parts,
            linearization_polynomial: Polynomial::<E::Fr, Coefficients>::new_for_size(0)?,
            wire_values_at_z: vec![],
            wire_values_at_z_omega: vec![],
            permutation_polynomials_at_z: vec![],
            grand_product_at_z_omega: E::Fr::zero(),
            quotient_polynomial_at_z: E::Fr::zero(),
            linearization_polynomial_at_z: E::Fr::zero(),

            _marker: std::marker::PhantomData,
        };

        let mut z_by_omega = z;
        z_by_omega.mul_assign(&domain.generator);

        for (idx, p) in state.witness_polys_as_coeffs.iter().enumerate() {
            let value_at_z = p.evaluate_at(&worker, z);
            state.wire_values_at_z.push(value_at_z);
            if idx == 3 {
                let value_at_z_omega = p.evaluate_at(&worker, z_by_omega);
                state.wire_values_at_z_omega.push(value_at_z_omega);
            }
        }

        for p in setup.permutation_polynomials[..(setup.permutation_polynomials.len() - 1)].iter() {
            let value_at_z = p.evaluate_at(&worker, z);
            state.permutation_polynomials_at_z.push(value_at_z);
        }

        let z_at_z_omega = state.z_in_monomial_form.evaluate_at(&worker, z_by_omega);
        state.grand_product_at_z_omega = z_at_z_omega;

        let t_at_z = {
            let mut result = E::Fr::zero();
            let mut current = E::Fr::one();
            let z_in_domain_size = z.pow(&[required_domain_size as u64]);
            for p in state.t_poly_parts.iter() {
                let mut subvalue_at_z = p.evaluate_at(&worker, z);
                subvalue_at_z.mul_assign(&current);
                result.add_assign(&subvalue_at_z);
                current.mul_assign(&z_in_domain_size);
            }

            result
        };

        state.quotient_polynomial_at_z = t_at_z;

        let mut quotient_linearization_challenge = E::Fr::one();

        let r = {
            // Q_const
            let mut r = setup.selector_polynomials[5].clone();

            // Q_A * A(z)
            r.add_assign_scaled(&worker, &setup.selector_polynomials[0], &state.wire_values_at_z[0]);

            // Q_B * B(z)
            r.add_assign_scaled(&worker, &setup.selector_polynomials[1], &state.wire_values_at_z[1]);

            // Q_C * C(z)
            r.add_assign_scaled(&worker, &setup.selector_polynomials[2], &state.wire_values_at_z[2]);

            // Q_D * D(z)
            r.add_assign_scaled(&worker, &setup.selector_polynomials[3], &state.wire_values_at_z[3]);

            // Q_M * A(z) * B(z)
            let mut scaling_factor = state.wire_values_at_z[0];
            scaling_factor.mul_assign(&state.wire_values_at_z[1]);
            r.add_assign_scaled(&worker, &setup.selector_polynomials[4], &scaling_factor);

            // Q_D_Next * D(z*omega)

            r.add_assign_scaled(&worker, &setup.next_step_selector_polynomials[0], &state.wire_values_at_z_omega[0]);

            quotient_linearization_challenge.mul_assign(&alpha);

            // + (a(z) + beta*z + gamma)*()*()*()*Z(x)

            let mut factor = quotient_linearization_challenge;
            for (wire_at_z, non_residue) in state.wire_values_at_z.iter().zip(Some(E::Fr::one()).iter().chain(&state.non_residues)) {
                let mut t = z;
                t.mul_assign(&non_residue);
                t.mul_assign(&beta);
                t.add_assign(&wire_at_z);
                t.add_assign(&gamma);

                factor.mul_assign(&t);
            }

            r.add_assign_scaled(&worker, &state.z_in_monomial_form, &factor);

            // - (a(z) + beta*perm_a + gamma)*()*()*z(z*omega) * beta * perm_d(X)

            let mut factor = quotient_linearization_challenge;
            factor.mul_assign(&beta);
            factor.mul_assign(&z_at_z_omega);

            for (wire_at_z, perm_at_z) in state.wire_values_at_z.iter().zip(state.permutation_polynomials_at_z.iter()) {
                let mut t = *perm_at_z;
                t.mul_assign(&beta);
                t.add_assign(&wire_at_z);
                t.add_assign(&gamma);

                factor.mul_assign(&t);
            }

            r.sub_assign_scaled(&worker, &setup.permutation_polynomials.last().expect("last permutation poly"), &factor);

            // + L_0(z) * Z(x)

            quotient_linearization_challenge.mul_assign(&alpha);

            let mut factor = evaluate_l0_at_point(required_domain_size as u64, z)?;
            factor.mul_assign(&quotient_linearization_challenge);

            r.add_assign_scaled(&worker, &state.z_in_monomial_form, &factor);

            r
        };

        // commit the linearization polynomial

        let r_at_z = r.evaluate_at(&worker, z);
        state.linearization_polynomial_at_z = r_at_z;

        state.linearization_polynomial = r;

        // sanity check - verification
        {
            let mut lhs = t_at_z;
            let vanishing_at_z = evaluate_vanishing_for_size(&z, required_domain_size as u64);
            lhs.mul_assign(&vanishing_at_z);

            let mut quotient_linearization_challenge = E::Fr::one();

            let mut rhs = r_at_z;

            // add public inputs
            {
                for (idx, input) in state.input_values.iter().enumerate() {
                    let mut tmp = evaluate_lagrange_poly_at_point(idx, &domain, z)?;
                    tmp.mul_assign(&input);

                    rhs.add_assign(&tmp);
                }
            }

            quotient_linearization_challenge.mul_assign(&alpha);

            // - \alpha (a + perm(z) * beta + gamma)*()*(d + gamma) & z(z*omega)

            let mut z_part = z_at_z_omega;

            assert_eq!(state.permutation_polynomials_at_z.len() + 1, state.wire_values_at_z.len());

            for (w, p) in state.wire_values_at_z.iter().zip(state.permutation_polynomials_at_z.iter()) {
                let mut tmp = *p;
                tmp.mul_assign(&beta);
                tmp.add_assign(&gamma);
                tmp.add_assign(&w);

                z_part.mul_assign(&tmp);
            }

            // last poly value and gamma
            let mut tmp = gamma;
            tmp.add_assign(&state.wire_values_at_z.iter().rev().next().unwrap());

            z_part.mul_assign(&tmp);
            z_part.mul_assign(&quotient_linearization_challenge);

            rhs.sub_assign(&z_part);

            quotient_linearization_challenge.mul_assign(&alpha);

            // - L_0(z) * \alpha^2

            let mut l_0_at_z = evaluate_l0_at_point(required_domain_size as u64, z)?;
            l_0_at_z.mul_assign(&quotient_linearization_challenge);

            rhs.sub_assign(&l_0_at_z);

            if lhs != rhs {
                return Err(SynthesisError::Unsatisfiable);
            }
        }

        let message = FourthProverMessage::<E, PlonkCsWidth4WithNextStepParams> {
            wire_values_at_z: state.wire_values_at_z.clone(),
            wire_values_at_z_omega: state.wire_values_at_z_omega.clone(),
            permutation_polynomials_at_z: state.permutation_polynomials_at_z.clone(),
            grand_product_at_z_omega: state.grand_product_at_z_omega,
            quotient_polynomial_at_z: state.quotient_polynomial_at_z,
            linearization_polynomial_at_z: state.linearization_polynomial_at_z,

            _marker: std::marker::PhantomData,
        };

        Ok((state, message))
    }

    pub(crate) fn fifth_step_from_fourth_step(
        mut fourth_state: FourthPartialProverState<E, PlonkCsWidth4WithNextStepParams>,
        fourth_verifier_message: FourthVerifierMessage<E, PlonkCsWidth4WithNextStepParams>,
        setup: &SetupPolynomials<E, PlonkCsWidth4WithNextStepParams>,
        crs_mons: &Crs<E, CrsForMonomialForm>,
        worker: &Worker,
    ) -> Result<FifthProverMessage<E, PlonkCsWidth4WithNextStepParams>, SynthesisError> {
        let FourthVerifierMessage { z, v, .. } = fourth_verifier_message;
        let required_domain_size = fourth_state.required_domain_size;

        let domain = Domain::new_for_size(required_domain_size as u64)?;

        let mut z_by_omega = z;
        z_by_omega.mul_assign(&domain.generator);

        // open at z:
        // t_i(x) * z^{domain_size*i}
        // r(x)
        // witnesses
        // permutations except of the last one

        // open at z*omega:
        // z(x)
        // next step witnesses (if any)

        let mut multiopening_challenge = E::Fr::one();

        let mut poly_to_divide_at_z = fourth_state.t_poly_parts.drain(0..1).collect::<Vec<_>>().pop().unwrap();
        let z_in_domain_size = z.pow(&[required_domain_size as u64]);
        let mut power_of_z = z_in_domain_size;
        for t_part in fourth_state.t_poly_parts.into_iter() {
            poly_to_divide_at_z.add_assign_scaled(&worker, &t_part, &power_of_z);
            power_of_z.mul_assign(&z_in_domain_size);
        }

        // linearization polynomial
        multiopening_challenge.mul_assign(&v);
        poly_to_divide_at_z.add_assign_scaled(&worker, &fourth_state.linearization_polynomial, &multiopening_challenge);

        debug_assert_eq!(multiopening_challenge, v.pow(&[1 as u64]));

        // all witness polys
        for w in fourth_state.witness_polys_as_coeffs.iter() {
            multiopening_challenge.mul_assign(&v);
            poly_to_divide_at_z.add_assign_scaled(&worker, &w, &multiopening_challenge);
        }

        debug_assert_eq!(multiopening_challenge, v.pow(&[(1 + 4) as u64]));

        // all except of the last permutation polys
        for p in setup.permutation_polynomials[..(setup.permutation_polynomials.len() - 1)].iter() {
            multiopening_challenge.mul_assign(&v);
            poly_to_divide_at_z.add_assign_scaled(&worker, &p, &multiopening_challenge);
        }

        debug_assert_eq!(multiopening_challenge, v.pow(&[(1 + 4 + 3) as u64]));

        multiopening_challenge.mul_assign(&v);

        let mut poly_to_divide_at_z_omega = fourth_state.z_in_monomial_form;
        poly_to_divide_at_z_omega.scale(&worker, multiopening_challenge);

        multiopening_challenge.mul_assign(&v);

        // d should be opened at z*omega due to d_next
        poly_to_divide_at_z_omega.add_assign_scaled(&worker, &fourth_state.witness_polys_as_coeffs[3], &multiopening_challenge);
        fourth_state.witness_polys_as_coeffs.truncate(0); // drop

        debug_assert_eq!(multiopening_challenge, v.pow(&[(1 + 4 + 3 + 1 + 1) as u64]));

        // division in monomial form is sequential, so we parallelize the divisions

        let mut polys = vec![(poly_to_divide_at_z, z), (poly_to_divide_at_z_omega, z_by_omega)];

        worker.scope(polys.len(), |scope, chunk| {
            for p in polys.chunks_mut(chunk) {
                scope.spawn(move |_| {
                    let (poly, at) = &p[0];
                    let at = *at;
                    let result = divide_single::<E>(poly.as_ref(), at);
                    p[0] = (Polynomial::from_coeffs(result).unwrap(), at);
                });
            }
        });

        let open_at_z_omega = polys.pop().unwrap().0;
        let open_at_z = polys.pop().unwrap().0;

        let opening_at_z = commit_using_monomials(&open_at_z, &crs_mons, &worker)?;

        let opening_at_z_omega = commit_using_monomials(&open_at_z_omega, &crs_mons, &worker)?;

        let message = FifthProverMessage::<E, PlonkCsWidth4WithNextStepParams> {
            opening_proof_at_z: opening_at_z,
            opening_proof_at_z_omega: opening_at_z_omega,

            _marker: std::marker::PhantomData,
        };

        Ok(message)
    }
}
