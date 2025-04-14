use crate::pairing::ff::{Field, PrimeField};
use crate::pairing::Engine;

use crate::SynthesisError;
use std::marker::PhantomData;

use super::cs::*;

#[derive(Debug, Clone)]
pub struct TestAssembly<E: Engine, P: PlonkConstraintSystemParams<E>> {
    m: usize,
    n: usize,

    num_inputs: usize,
    num_aux: usize,

    input_assingments: Vec<E::Fr>,
    aux_assingments: Vec<E::Fr>,

    inputs_map: Vec<usize>,

    is_finalized: bool,

    next_step_leftover_from_previous_gate: Option<(E::Fr, P::NextTraceStepCoefficients)>,

    _marker: std::marker::PhantomData<P>,
}

impl<E: Engine, P: PlonkConstraintSystemParams<E>> ConstraintSystem<E, P> for TestAssembly<E, P> {
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

        self.n += 1;

        Ok(input_var)
    }

    // allocate an abstract gate
    fn new_gate(&mut self, variables: P::StateVariables, this_step_coeffs: P::ThisTraceStepCoefficients, next_step_coeffs: P::NextTraceStepCoefficients) -> Result<(), SynthesisError> {
        // check that leftover of this gate is satisfied

        if let Some((value_leftover, coeffs)) = self.next_step_leftover_from_previous_gate.take() {
            let mut leftover = value_leftover;
            for (&var, coeff) in variables.as_ref().iter().rev().zip(coeffs.as_ref().iter()) {
                let mut value = self.get_value(var)?;
                value.mul_assign(&coeff);

                leftover.add_assign(&value);
            }

            if leftover.is_zero() == false {
                return Err(SynthesisError::Unsatisfiable);
            }
        }

        // now check for THIS gate

        let mut gate_value = E::Fr::zero();

        let mut this_step_coeffs_iter = this_step_coeffs.as_ref().iter();

        // first take an LC
        for (&var, coeff) in variables.as_ref().iter().zip(&mut this_step_coeffs_iter) {
            let mut value = self.get_value(var)?;
            value.mul_assign(&coeff);

            gate_value.add_assign(&value);
        }

        // multiplication
        let mut q_m = *(this_step_coeffs_iter.next().unwrap());
        q_m.mul_assign(&self.get_value(variables.as_ref()[0])?);
        q_m.mul_assign(&self.get_value(variables.as_ref()[1])?);
        gate_value.add_assign(&q_m);

        // constant
        gate_value.add_assign(this_step_coeffs_iter.next().unwrap());

        assert!(next_step_coeffs.as_ref().len() <= 1);
        if next_step_coeffs.as_ref().len() != 0 {
            assert!(P::CAN_ACCESS_NEXT_TRACE_STEP == true);
            if next_step_coeffs.as_ref()[0].is_zero() == false {
                self.next_step_leftover_from_previous_gate = Some((gate_value, next_step_coeffs));
            }
            // assert!(self.next_step_vars.is_some());
            // let next_step_vars = self.next_step_vars.take().expect("must have some next step variables")
            // for (&var, coeff) in variables.as_ref().iter().rev()
            //                 .zip(next_step_coeffs.as_ref().iter())
            // {
            //     let mut value = self.get_value(var)?;
            //     value.mul_assign(&coeff);

            //     gate_value.add_assign(&value);
            // }
        } else {
            if gate_value.is_zero() == false {
                return Err(SynthesisError::Unsatisfiable);
            }
        }

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

impl<E: Engine, P: PlonkConstraintSystemParams<E>> TestAssembly<E, P> {
    pub fn new() -> Self {
        let tmp = Self {
            n: 0,
            m: 0,

            num_inputs: 0,
            num_aux: 0,

            input_assingments: vec![],
            aux_assingments: vec![],

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

    // pub fn is_satisfied(&self, in_a_middle: bool) -> bool {
    //     // expect a small number of inputs
    //     for (i, gate) in self.input_gates.iter().enumerate()
    //     {
    //         let Gate::<E::Fr> {
    //             variables: [a_var, b_var, c_var],
    //             coefficients: [q_l, q_r, q_o, q_m, q_c, q_c_next]
    //         } = *gate;

    //         assert!(q_c.is_zero(), "should not hardcode a constant into the input gate");
    //         assert!(q_c_next.is_zero(), "input gates should not link to the next gate");

    //         let a_value = self.get_value(a_var).expect("must get a variable value");
    //         let b_value = self.get_value(b_var).expect("must get a variable value");
    //         let c_value = self.get_value(c_var).expect("must get a variable value");

    //         let input_value = self.input_assingments[i];
    //         let mut res = input_value;
    //         res.negate();

    //         let mut tmp = q_l;
    //         tmp.mul_assign(&a_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_r;
    //         tmp.mul_assign(&b_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_o;
    //         tmp.mul_assign(&c_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_m;
    //         tmp.mul_assign(&a_value);
    //         tmp.mul_assign(&b_value);
    //         res.add_assign(&tmp);

    //         if !res.is_zero() {
    //             println!("Unsatisfied at input gate {}: {:?}", i+1, gate);
    //             println!("A value = {}, B value = {}, C value = {}", a_value, b_value, c_value);
    //             return false;
    //         }
    //     }

    //     for (i, gate_pair) in self.aux_gates.windows(2).enumerate()
    //     {
    //         let this_gate = gate_pair[0];
    //         let next_gate = &gate_pair[1];

    //         let Gate::<E::Fr> {
    //             variables: [a_var, b_var, c_var],
    //             coefficients: [q_l, q_r, q_o, q_m, q_c, q_c_next]
    //         } = this_gate;

    //         let a_value = self.get_value(a_var).expect("must get a variable value");
    //         let b_value = self.get_value(b_var).expect("must get a variable value");
    //         let c_value = self.get_value(c_var).expect("must get a variable value");

    //         let next_gate_c_var = next_gate.variables[2];

    //         let c_next_value = self.get_value(next_gate_c_var).expect("must get a variable value");

    //         let mut res = q_c;

    //         let mut tmp = q_l;
    //         tmp.mul_assign(&a_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_r;
    //         tmp.mul_assign(&b_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_o;
    //         tmp.mul_assign(&c_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_m;
    //         tmp.mul_assign(&a_value);
    //         tmp.mul_assign(&b_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_c_next;
    //         tmp.mul_assign(&c_next_value);
    //         res.add_assign(&tmp);

    //         if !res.is_zero() {
    //             println!("Unsatisfied at aux gate {}", i+1);
    //             println!("Gate {:?}", this_gate);
    //             println!("A = {}, B = {}, C = {}", a_value, b_value, c_value);
    //             return false;
    //         }
    //     }

    //     if !in_a_middle {
    //         let i = self.aux_gates.len();
    //         let last_gate = *self.aux_gates.last().unwrap();

    //         let Gate::<E::Fr> {
    //             variables: [a_var, b_var, c_var],
    //             coefficients: [q_l, q_r, q_o, q_m, q_c, q_c_next]
    //         } = last_gate;

    //         let a_value = self.get_value(a_var).expect("must get a variable value");
    //         let b_value = self.get_value(b_var).expect("must get a variable value");
    //         let c_value = self.get_value(c_var).expect("must get a variable value");

    //         assert!(q_c_next.is_zero(), "last gate should not be linked to the next one");

    //         let mut res = q_c;

    //         let mut tmp = q_l;
    //         tmp.mul_assign(&a_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_r;
    //         tmp.mul_assign(&b_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_o;
    //         tmp.mul_assign(&c_value);
    //         res.add_assign(&tmp);

    //         let mut tmp = q_m;
    //         tmp.mul_assign(&a_value);
    //         tmp.mul_assign(&b_value);
    //         res.add_assign(&tmp);

    //         if !res.is_zero() {
    //             println!("Unsatisfied at aux gate {}", i+1);
    //             println!("Gate {:?}", last_gate);
    //             println!("A = {}, B = {}, C = {}", a_value, b_value, c_value);
    //             return false;
    //         }
    //     }

    //     true
    // }

    pub fn num_gates(&self) -> usize {
        self.n
    }
}
