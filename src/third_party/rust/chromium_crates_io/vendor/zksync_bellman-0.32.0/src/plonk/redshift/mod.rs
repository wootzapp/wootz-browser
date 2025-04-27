mod generator;
mod prover;

// use crate::Engine;
// use crate::plonk::plonk::prover::PlonkSetup;
// use crate::plonk::commitments::CommitmentScheme;
// use crate::plonk::Transcript;
// use crate::SynthesisError;

// use crate::pairing::ff::{Field, PrimeField};

// pub struct PlonkChunkedNonhomomorphicProof<E: Engine, S: CommitmentScheme<E::Fr> >{
//     pub a_opening_value: E::Fr,
//     pub b_opening_value: E::Fr,
//     pub c_opening_value: E::Fr,
//     pub q_l_opening_value: E::Fr,
//     pub q_r_opening_value: E::Fr,
//     pub q_o_opening_value: E::Fr,
//     pub q_m_opening_value: E::Fr,
//     pub q_c_opening_value: E::Fr,
//     pub s_id_opening_value: E::Fr,
//     pub sigma_1_opening_value: E::Fr,
//     pub sigma_2_opening_value: E::Fr,
//     pub sigma_3_opening_value: E::Fr,
//     pub z_1_unshifted_opening_value: E::Fr,
//     pub z_2_unshifted_opening_value: E::Fr,
//     pub z_1_shifted_opening_value: E::Fr,
//     pub z_2_shifted_opening_value: E::Fr,
//     pub t_low_opening_value: E::Fr,
//     pub t_mid_opening_value: E::Fr,
//     pub t_high_opening_value: E::Fr,
//     pub a_commitment: S::Commitment,
//     pub b_commitment: S::Commitment,
//     pub c_commitment: S::Commitment,
//     pub z_1_commitment: S::Commitment,
//     pub z_2_commitment: S::Commitment,
//     pub t_low_commitment: S::Commitment,
//     pub t_mid_commitment: S::Commitment,
//     pub t_high_commitment: S::Commitment,
//     pub openings_proof: S::OpeningProof,
// }

// use crate::plonk::domains::Domain;

// fn evaluate_inverse_vanishing_poly<E: Engine>(vahisning_size: usize, point: E::Fr) -> E::Fr {
//     assert!(vahisning_size.is_power_of_two());

//     // update from the paper - it should not hold for the last generator, omega^(n) in original notations

//     // Z(X) = (X^(n+1) - 1) / (X - omega^(n)) => Z^{-1}(X) = (X - omega^(n)) / (X^(n+1) - 1)

//     let domain = Domain::<E::Fr>::new_for_size(vahisning_size as u64).expect("should fit");
//     let n_domain_omega = domain.generator;
//     let root = n_domain_omega.pow([(vahisning_size - 1) as u64]);

//     let mut numerator = point;
//     numerator.sub_assign(&root);

//     let mut denominator = point.pow([vahisning_size as u64]);
//     denominator.sub_assign(&E::Fr::one());

//     let denominator = denominator.inverse().expect("must exist");

//     numerator.mul_assign(&denominator);

//     numerator
// }

// fn evaluate_lagrange_poly<E: Engine>(vahisning_size:usize, poly_number: usize, at: E::Fr) -> E::Fr {
//     assert!(vahisning_size.is_power_of_two());

//     let mut repr = E::Fr::zero().into_repr();
//     repr.as_mut()[0] = vahisning_size as u64;

//     let size_fe = E::Fr::from_repr(repr).expect("is a valid representation");
//     // let size_inv = n_fe.inverse().expect("must exist");

//     // L_0(X) = (Z_H(X) / (X - 1)).(1/n) and L_0(1) = 1
//     // L_1(omega) = 1 = L_0(omega * omega^-1)

//     let domain = Domain::<E::Fr>::new_for_size(vahisning_size as u64).expect("domain of this size should exist");
//     let omega = domain.generator;

//     let omega_inv = omega.inverse().expect("must exist");

//     let argument_multiplier = omega_inv.pow([poly_number as u64]);
//     let mut argument = at;
//     argument.mul_assign(&argument_multiplier);

//     let mut numerator = argument.pow([vahisning_size as u64]);
//     numerator.sub_assign(&E::Fr::one());

//     let mut denom = argument;
//     denom.sub_assign(&E::Fr::one());
//     denom.mul_assign(&size_fe);

//     let denom_inv = denom.inverse().expect("must exist");

//     numerator.mul_assign(&denom_inv);

//     numerator
// }
