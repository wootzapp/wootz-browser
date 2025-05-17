use crate::pairing::ff::Field;
use crate::pairing::{CurveProjective, Engine};
use std::marker::PhantomData;

mod adapted_helper;
mod adapted_prover;
mod adapted_verifier;
pub mod batch;
pub mod generator;
pub mod helper;
pub mod parameters;
pub mod poly;
pub mod prover;
pub mod verifier;

pub use self::batch::Batch;
pub use self::verifier::MultiVerifier;

pub use self::adapted_prover::{create_advice, create_advice_on_information_and_srs, create_advice_on_srs, create_proof, create_proof_on_srs};
pub use self::generator::{
    generate_parameters, generate_parameters_on_srs, generate_parameters_on_srs_and_information, generate_random_parameters, generate_srs, get_circuit_parameters,
    get_circuit_parameters_for_succinct_sonic, CircuitParameters,
};
pub use self::parameters::{Parameters, PreparedVerifyingKey, Proof, SxyAdvice, VerifyingKey};

pub use self::adapted_verifier::{verify_aggregate, verify_proofs};

pub use self::adapted_helper::create_aggregate;
