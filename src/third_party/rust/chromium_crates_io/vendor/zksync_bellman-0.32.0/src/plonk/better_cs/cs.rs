use crate::pairing::ff::Field;
use crate::pairing::Engine;

use crate::SynthesisError;
use std::marker::PhantomData;

pub use crate::plonk::cs::variable::*;

pub trait Circuit<E: Engine, P: PlonkConstraintSystemParams<E>> {
    fn synthesize<CS: ConstraintSystem<E, P>>(&self, cs: &mut CS) -> Result<(), SynthesisError>;
}

pub trait CustomGateMarker<E: Engine>: Sized {}

// pub trait TraceStepCoefficients<'a, E: Engine>: Sized
//     + AsRef<[E::Fr]>
//     + Copy
//     + Clone
//     + PartialEq
//     + Eq
//     + ExactSizeIterator<Item = &'a E::Fr> { }

// pub trait PlonkConstraintSystemParams<E: Engine, T1, T2> where
//     for <'a> T1: TraceStepCoefficients<'a, E>,
//     for <'a> T2: TraceStepCoefficients<'a, E> {
//     const STATE_WIDTH: usize;
//     const HAS_CUSTOM_GATES: bool;
//     const CAN_ACCESS_NEXT_TRACE_STEP: bool;

//     type CustomGateType: CustomGateMarker<E>;
// }

pub trait TraceStepCoefficients<E: Engine>: Sized + AsRef<[E::Fr]> + Copy + Clone + PartialEq + Eq {
    fn empty() -> Self;
    fn identity() -> Self;
    fn negate(&mut self);
    fn from_coeffs(coeffs: &[E::Fr]) -> Self;
}

pub trait StateVariablesSet: Sized + AsRef<[Variable]> + Copy + Clone + PartialEq + Eq {
    fn from_variable_and_padding(variable: Variable, padding: Variable) -> Self;
    fn from_variables(variables: &[Variable]) -> Self;
}

pub trait PlonkConstraintSystemParams<E: Engine>: Clone {
    const STATE_WIDTH: usize;
    const HAS_CUSTOM_GATES: bool;
    const CAN_ACCESS_NEXT_TRACE_STEP: bool;

    type StateVariables: StateVariablesSet;
    type ThisTraceStepCoefficients: TraceStepCoefficients<E>;
    type NextTraceStepCoefficients: TraceStepCoefficients<E>;

    type CustomGateType: CustomGateMarker<E>;
}

pub trait ConstraintSystem<E: Engine, P: PlonkConstraintSystemParams<E>> {
    // allocate a variable
    fn alloc<F>(&mut self, value: F) -> Result<Variable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>;

    // allocate an input variable
    fn alloc_input<F>(&mut self, value: F) -> Result<Variable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>;

    fn new_gate(&mut self, variables: P::StateVariables, this_step_coeffs: P::ThisTraceStepCoefficients, next_step_coeffs: P::NextTraceStepCoefficients) -> Result<(), SynthesisError>;

    fn get_value(&self, _variable: Variable) -> Result<E::Fr, SynthesisError> {
        Err(SynthesisError::AssignmentMissing)
    }

    fn get_dummy_variable(&self) -> Variable;
}

pub struct NoCustomGate;
impl<E: Engine> CustomGateMarker<E> for NoCustomGate {}

impl StateVariablesSet for [Variable; 3] {
    fn from_variable_and_padding(variable: Variable, padding: Variable) -> Self {
        [variable, padding, padding]
    }
    fn from_variables(variables: &[Variable]) -> Self {
        debug_assert_eq!(variables.len(), 3);

        [variables[0], variables[1], variables[2]]
    }
}

impl StateVariablesSet for [Variable; 4] {
    fn from_variable_and_padding(variable: Variable, padding: Variable) -> Self {
        [variable, padding, padding, padding]
    }
    fn from_variables(variables: &[Variable]) -> Self {
        debug_assert_eq!(variables.len(), 4);

        [variables[0], variables[1], variables[2], variables[3]]
    }
}

// impl<E: Engine> TraceStepCoefficients<E> for () {
//     fn empty() -> Self {
//         ()
//     }
//     fn identity() -> Self {
//         ()
//     }
//     fn from_coeffs(coeffs: &[E::Fr]) -> Self {
//         debug_assert_eq!(coeffs.len(), 0);

//         ()
//     }
// }

// impl<E: Engine> AsRef<[E::Fr]> for () {
//     fn as_ref(&self) -> &[E::Fr] {
//         &[]
//     }
// }

impl<E: Engine> TraceStepCoefficients<E> for [E::Fr; 0] {
    fn empty() -> Self {
        []
    }
    fn identity() -> Self {
        []
    }
    fn negate(&mut self) {}
    fn from_coeffs(coeffs: &[E::Fr]) -> Self {
        debug_assert_eq!(coeffs.len(), 0);

        []
    }
}

impl<E: Engine> TraceStepCoefficients<E> for [E::Fr; 1] {
    fn empty() -> Self {
        [E::Fr::zero()]
    }
    fn identity() -> Self {
        [E::Fr::one()]
    }
    fn negate(&mut self) {
        for c in self.iter_mut() {
            c.negate();
        }
    }
    fn from_coeffs(coeffs: &[E::Fr]) -> Self {
        debug_assert_eq!(coeffs.len(), 1);

        [coeffs[0]]
    }
}

impl<E: Engine> TraceStepCoefficients<E> for [E::Fr; 3] {
    fn empty() -> Self {
        [E::Fr::zero(); 3]
    }
    fn identity() -> Self {
        [E::Fr::one(), E::Fr::zero(), E::Fr::zero()]
    }
    fn negate(&mut self) {
        for c in self.iter_mut() {
            c.negate();
        }
    }
    fn from_coeffs(coeffs: &[E::Fr]) -> Self {
        debug_assert_eq!(coeffs.len(), 3);

        [coeffs[0], coeffs[1], coeffs[2]]
    }
}

impl<E: Engine> TraceStepCoefficients<E> for [E::Fr; 4] {
    fn empty() -> Self {
        [E::Fr::zero(); 4]
    }
    fn identity() -> Self {
        [E::Fr::one(), E::Fr::zero(), E::Fr::zero(), E::Fr::zero()]
    }
    fn negate(&mut self) {
        for c in self.iter_mut() {
            c.negate();
        }
    }
    fn from_coeffs(coeffs: &[E::Fr]) -> Self {
        debug_assert_eq!(coeffs.len(), 4);

        [coeffs[0], coeffs[1], coeffs[2], coeffs[3]]
    }
}

impl<E: Engine> TraceStepCoefficients<E> for [E::Fr; 5] {
    fn empty() -> Self {
        [E::Fr::zero(); 5]
    }
    fn identity() -> Self {
        [E::Fr::one(), E::Fr::zero(), E::Fr::zero(), E::Fr::zero(), E::Fr::zero()]
    }
    fn negate(&mut self) {
        for c in self.iter_mut() {
            c.negate();
        }
    }
    fn from_coeffs(coeffs: &[E::Fr]) -> Self {
        debug_assert_eq!(coeffs.len(), 5);

        [coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4]]
    }
}

impl<E: Engine> TraceStepCoefficients<E> for [E::Fr; 6] {
    fn empty() -> Self {
        [E::Fr::zero(); 6]
    }
    fn identity() -> Self {
        [E::Fr::one(), E::Fr::zero(), E::Fr::zero(), E::Fr::zero(), E::Fr::zero(), E::Fr::zero()]
    }
    fn negate(&mut self) {
        for c in self.iter_mut() {
            c.negate();
        }
    }
    fn from_coeffs(coeffs: &[E::Fr]) -> Self {
        debug_assert_eq!(coeffs.len(), 6);

        [coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4], coeffs[5]]
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct PlonkCsWidth3WithNextStepParams;
impl<E: Engine> PlonkConstraintSystemParams<E> for PlonkCsWidth3WithNextStepParams {
    const STATE_WIDTH: usize = 3;
    const HAS_CUSTOM_GATES: bool = false;
    const CAN_ACCESS_NEXT_TRACE_STEP: bool = true;

    type StateVariables = [Variable; 3];
    type ThisTraceStepCoefficients = [E::Fr; 5];
    type NextTraceStepCoefficients = [E::Fr; 1];

    type CustomGateType = NoCustomGate;
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub struct PlonkCsWidth4WithNextStepParams;
impl<E: Engine> PlonkConstraintSystemParams<E> for PlonkCsWidth4WithNextStepParams {
    const STATE_WIDTH: usize = 4;
    const HAS_CUSTOM_GATES: bool = false;
    const CAN_ACCESS_NEXT_TRACE_STEP: bool = true;

    type StateVariables = [Variable; 4];
    type ThisTraceStepCoefficients = [E::Fr; 6];
    type NextTraceStepCoefficients = [E::Fr; 1];

    type CustomGateType = NoCustomGate;
}
