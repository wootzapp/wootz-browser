use crate::pairing::ff::{Field, PrimeField};
use crate::pairing::Engine;

use crate::SynthesisError;
use std::marker::PhantomData;

use super::cs::*;

#[derive(Debug, Clone)]
pub struct OneShotTestAssembly<E: Engine, P: PlonkConstraintSystemParams<E>> {
    m: usize,
    n: usize,

    num_inputs: usize,
    num_aux: usize,

    input_assingments: Vec<E::Fr>,
    aux_assingments: Vec<E::Fr>,

    input_gates: Vec<(P::StateVariables, P::ThisTraceStepCoefficients, P::NextTraceStepCoefficients)>,
    aux_gates: Vec<(P::StateVariables, P::ThisTraceStepCoefficients, P::NextTraceStepCoefficients)>,

    inputs_map: Vec<usize>,

    is_finalized: bool,

    next_step_leftover_from_previous_gate: Option<(E::Fr, P::NextTraceStepCoefficients)>,

    _marker: std::marker::PhantomData<P>,
}

impl<E: Engine, P: PlonkConstraintSystemParams<E>> ConstraintSystem<E, P> for OneShotTestAssembly<E, P> {
    // allocate a variable
    fn alloc<F>(&mut self, value: F) -> Result<Variable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>,
    {
        let value = value()?;

        self.num_aux += 1;
        let index = self.num_aux;
        self.aux_assingments.push(value);

        // println!("Allocated variable Aux({}) with value {}", index, value);

        Ok(Variable(Index::Aux(index)))
    }

    // allocate an input variable
    fn alloc_input<F>(&mut self, value: F) -> Result<Variable, SynthesisError>
    where
        F: FnOnce() -> Result<E::Fr, SynthesisError>,
    {
        let value = value()?;

        self.num_inputs += 1;
        let index = self.num_inputs;
        self.input_assingments.push(value);

        let input_var = Variable(Index::Input(index));

        let dummy = self.get_dummy_variable();

        let vars = P::StateVariables::from_variable_and_padding(input_var, dummy);
        let mut this_step_coeffs = P::ThisTraceStepCoefficients::identity();
        this_step_coeffs.negate(); // we use -1 here to later add to the constants polynomial using + sign
        let next_step_coeffs = P::NextTraceStepCoefficients::empty();

        self.input_gates.push((vars, this_step_coeffs, next_step_coeffs));
        self.n += 1;

        Ok(input_var)
    }

    // allocate an abstract gate
    fn new_gate(&mut self, variables: P::StateVariables, this_step_coeffs: P::ThisTraceStepCoefficients, next_step_coeffs: P::NextTraceStepCoefficients) -> Result<(), SynthesisError> {
        self.aux_gates.push((variables, this_step_coeffs, next_step_coeffs));

        self.n += 1;

        Ok(())
    }

    fn get_value(&self, var: Variable) -> Result<E::Fr, SynthesisError> {
        let value = match var {
            Variable(Index::Aux(0)) => {
                E::Fr::zero()
                // return Err(SynthesisError::AssignmentMissing);
            }
            Variable(Index::Input(0)) => {
                return Err(SynthesisError::AssignmentMissing);
            }
            Variable(Index::Input(input)) => self.input_assingments[input - 1],
            Variable(Index::Aux(aux)) => self.aux_assingments[aux - 1],
        };

        Ok(value)
    }

    fn get_dummy_variable(&self) -> Variable {
        self.dummy_variable()
    }
}

impl<E: Engine, P: PlonkConstraintSystemParams<E>> OneShotTestAssembly<E, P> {
    pub fn new() -> Self {
        let tmp = Self {
            n: 0,
            m: 0,

            num_inputs: 0,
            num_aux: 0,

            input_assingments: vec![],
            aux_assingments: vec![],

            input_gates: vec![],
            aux_gates: vec![],

            inputs_map: vec![],

            is_finalized: false,

            next_step_leftover_from_previous_gate: None,

            _marker: std::marker::PhantomData,
        };

        tmp
    }

    pub fn new_with_size_hints(num_inputs: usize, num_aux: usize) -> Self {
        let tmp = Self {
            n: 0,
            m: 0,

            num_inputs: 0,
            num_aux: 0,

            input_assingments: Vec::with_capacity(num_inputs),
            aux_assingments: Vec::with_capacity(num_aux),

            input_gates: Vec::with_capacity(num_inputs),
            aux_gates: Vec::with_capacity(num_aux),

            inputs_map: Vec::with_capacity(num_inputs),

            is_finalized: false,

            next_step_leftover_from_previous_gate: None,

            _marker: std::marker::PhantomData,
        };

        tmp
    }

    // return variable that is not in a constraint formally, but has some value
    fn dummy_variable(&self) -> Variable {
        Variable(Index::Aux(0))
    }

    pub fn is_well_formed(&self) -> bool {
        // check that last gate does not chain further!
        self.next_step_leftover_from_previous_gate.is_none()
    }

    pub fn num_gates(&self) -> usize {
        self.n
    }
}

impl<E: Engine> OneShotTestAssembly<E, PlonkCsWidth4WithNextStepParams> {
    pub fn is_satisfied(&self, in_a_middle: bool) -> bool {
        // expect a small number of inputs
        for (_i, (_vars, this_step_coeffs, next_step_coeffs)) in self.input_gates.iter().enumerate() {
            for c in this_step_coeffs.as_ref().iter().skip(1) {
                assert!(c.is_zero(), "input gate must contatain only one coefficient");
            }
            for c in next_step_coeffs.as_ref().iter() {
                assert!(c.is_zero(), "input gate must contatain no next step coefficients");
            }
        }

        for (i, gate_pair) in self.aux_gates.windows(2).enumerate() {
            let this_gate = &gate_pair[0];
            let next_gate = &gate_pair[1];

            let mut this_gate_value = E::Fr::zero();

            let mut coeffs_iter = this_gate.1.as_ref().iter();

            // addition
            for (&this_var, this_coeff) in this_gate.0.as_ref().iter().zip(&mut coeffs_iter) {
                let mut tmp = self.get_value(this_var).expect("must get a variable value");
                tmp.mul_assign(&this_coeff);

                this_gate_value.add_assign(&tmp);
            }

            // multiplication

            let mut tmp = self.get_value(this_gate.0.as_ref()[0]).expect("must get a variable value");
            tmp.mul_assign(&self.get_value(this_gate.0.as_ref()[1]).expect("must get a variable value"));
            tmp.mul_assign(&(&mut coeffs_iter.next().unwrap()));

            this_gate_value.add_assign(&tmp);

            // constant

            this_gate_value.add_assign(&(&mut coeffs_iter.next().unwrap()));

            // next step part

            for (&next_var, next_step_coeffs_coeff) in next_gate.0.as_ref().iter().rev().zip(this_gate.2.as_ref().iter()) {
                let mut tmp = self.get_value(next_var).expect("must get a variable value");
                tmp.mul_assign(&next_step_coeffs_coeff);

                this_gate_value.add_assign(&tmp);
            }

            if !this_gate_value.is_zero() {
                println!("Unsatisfied at aux gate {}", i + 1);
                println!("Gate {:?}", this_gate);
                // println!("A = {}, B = {}, C = {}", a_value, b_value, c_value);
                return false;
            }
        }

        if !in_a_middle {
            let i = self.aux_gates.len();
            let last_gate = *self.aux_gates.last().unwrap();

            let this_gate = last_gate;

            let mut this_gate_value = E::Fr::zero();

            let mut coeffs_iter = this_gate.1.as_ref().iter();

            // addition
            for (&this_var, this_coeff) in this_gate.0.as_ref().iter().zip(&mut coeffs_iter) {
                let mut tmp = self.get_value(this_var).expect("must get a variable value");
                tmp.mul_assign(&this_coeff);

                this_gate_value.add_assign(&tmp);
            }

            // multiplication

            let mut tmp = self.get_value(this_gate.0.as_ref()[0]).expect("must get a variable value");
            tmp.mul_assign(&self.get_value(this_gate.0.as_ref()[1]).expect("must get a variable value"));
            tmp.mul_assign(&(&mut coeffs_iter.next().unwrap()));

            this_gate_value.add_assign(&tmp);

            // constant

            this_gate_value.add_assign(&(&mut coeffs_iter.next().unwrap()));

            // next step part must be empty

            for c in this_gate.2.as_ref().iter() {
                assert!(c.is_zero(), "last gate must not wrap around");
            }

            if !this_gate_value.is_zero() {
                println!("Unsatisfied at last aux gate {}", i + 1);
                println!("Gate {:?}", this_gate);
                // println!("A = {}, B = {}, C = {}", a_value, b_value, c_value);
                return false;
            }
        }

        true
    }
}
