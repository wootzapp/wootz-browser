use crate::pairing::{
    Engine,
};

use crate::pairing::ff::{
    Field,
    PrimeField,
    PrimeFieldRepr,
    BitIterator
};

use crate::{
    SynthesisError,
};

use crate::plonk::better_better_cs::cs::{
    Variable, 
    ConstraintSystem,
    ArithmeticTerm,
    MainGateTerm,
    Width4MainGateWithDNextEquation,
    MainGateEquation,
    GateEquationInternal,
    GateEquation,
    LinearCombinationOfTerms,
    PolynomialMultiplicativeTerm,
    PolynomialInConstraint,
    TimeDilation
};

use super::assignment::{
    Assignment
};

use super::num::{AllocatedNum};

use crate::plonk::better_better_cs::cs::PlonkConstraintSystemParams;

pub struct Rescue125<E: Engine> {
    _marker: std::marker::PhantomData<E>
}

enum RescueStateSimplifier<E: Engine> {
    Number(AllocatedNum<E>),
    Constant(E::Fr)
}


#[derive(Clone, Debug, Hash)]
pub struct Rescue5CustomGate(pub [LinearCombinationOfTerms; 3]);

impl<E: Engine> GateEquationInternal<E> for Rescue5CustomGate {
    fn degree(&self) -> usize {
        2
    }

    fn num_constraints(&self) -> usize {
        3
    }

    fn get_constraint(&self) -> &LinearCombinationOfTerms<E> {
        unreachable!("must not try to access single constraint of Rescue alpha  5 gate");
    }

    fn get_constraints(&self) -> &[LinearCombinationOfTerms<E>] {
        &self.0[..]
    }
}

impl GateEquation for Rescue5CustomGate {
    // Width4MainGateWithDNextEquation is NOT generic, so this is fine
    // and safe since it's sync!
    fn static_description() -> &'static Self {
        static mut VALUE: Option<Rescue5CustomGate> = None;
        static INIT: std::sync::Once = std::sync::Once::new();

        unsafe {
            INIT.call_once(||{
                VALUE = Some(Rescue5CustomGate::default());
            });

            VALUE.as_ref().unwrap()
        }
    }
}


impl std::default::Default for Rescue5CustomGate {
    fn default() -> Self {
        Self::get_equation()
    }
}

impl Rescue5CustomGate {
    pub fn get_equation() -> Self {
        let mut term_square: Vec<PolynomialMultiplicativeTerm> = Vec::with_capacity(2);
        // constant
        term_square.push(
            PolynomialMultiplicativeTerm(
                vec![
                    PolynomialInConstraint::VariablesPolynomial(0, TimeDilation(0)),
                    PolynomialInConstraint::VariablesPolynomial(0, TimeDilation(0))
                ]
            )
        );

        term_square.push(
            PolynomialMultiplicativeTerm(
                vec![
                    PolynomialInConstraint::VariablesPolynomial(1, TimeDilation(0))
                ]
            )
        );

        let mut term_quad: Vec<PolynomialMultiplicativeTerm> = Vec::with_capacity(2);
        // constant
        term_quad.push(
            PolynomialMultiplicativeTerm(
                vec![
                    PolynomialInConstraint::VariablesPolynomial(1, TimeDilation(0)),
                    PolynomialInConstraint::VariablesPolynomial(1, TimeDilation(0))
                ]
            )
        );

        term_quad.push(
            PolynomialMultiplicativeTerm(
                vec![
                    PolynomialInConstraint::VariablesPolynomial(2, TimeDilation(0))
                ]
            )
        );

        let mut term_fifth: Vec<PolynomialMultiplicativeTerm> = Vec::with_capacity(2);
        // constant
        term_fifth.push(
            PolynomialMultiplicativeTerm(
                vec![
                    PolynomialInConstraint::VariablesPolynomial(0, TimeDilation(0)),
                    PolynomialInConstraint::VariablesPolynomial(2, TimeDilation(0))
                ]
            )
        );

        term_fifth.push(
            PolynomialMultiplicativeTerm(
                vec![
                    PolynomialInConstraint::VariablesPolynomial(3, TimeDilation(0))
                ]
            )
        );

        Self([
            LinearCombinationOfTerms(term_square),
            LinearCombinationOfTerms(term_quad),
            LinearCombinationOfTerms(term_fifth)])
    }
}

impl<E: Engine> Rescue125<E> {
    const RATE: usize = 2;
    const CAPACITY: usize = 1;
    const STATE_WIDTH: usize = Self::RATE + Self::CAPACITY;
    const ALPHA: u64 = 5;
    const NUM_ROUNDS_DOUBLED: usize = 44;

    pub fn hash<CS: ConstraintSystem<E, MainGate = Width4MainGateWithDNextEquation>>(
        cs: &mut CS,
        input: &[AllocatedNum<E>]
    ) -> Result<AllocatedNum<E>, SynthesisError> {
        assert_eq!(input.len(), Self::RATE);
        assert_eq!(CS::Params::STATE_WIDTH, 4);
        assert!(CS::Params::CAN_ACCESS_NEXT_TRACE_STEP);

        let one = E::Fr::one();
        let zero = E::Fr::zero();
        let mut minus_one = one;
        minus_one.negate();

        let mut round_constant_placeholder = E::Fr::one();
        round_constant_placeholder.double();

        let dummy_var = CS::get_dummy_variable();

        // with first sbox
        // (input[0] + round_constant[0]) ^ [alpha_inv] -> state[0]
        // (input[1] + round_constant[1]) ^ [alpha_inv] -> state[1]
        // round_constant[2] ^ [alpha_inv] -> state[2]

        // but we proof otherwise 
        // state[0] ^ 5 = (input[0] + round_constant[0])
        // state[1] ^ 5 = (input[1] + round_constant[1])
        // state[2] = constant 

        // with state width of 4 we can have

        // a = state[0]
        // b = state[0] ^ 2
        // c = state[0] ^ 4 = b^2
        // d = c * a = state[0]^5

        // and BEFORE it we have a gate
        // input[0] + round_constant[0] - d_next = 0

        let state_0 = AllocatedNum::<E>::alloc(
            cs,
            || {
                Ok(E::Fr::one())
            }
        )?;

        let state_0_squared = state_0.clone();
        let state_0_quad = state_0.clone();
        let state_0_fifth = state_0.clone();

        let state_1 = AllocatedNum::<E>::alloc(
            cs,
            || {
                Ok(E::Fr::one())
            }
        )?;

        let state_1_squared = state_1.clone();
        let state_1_quad = state_1.clone();
        let state_1_fifth = state_1.clone();

        // input[0] + round_constant[0] - state[0]^5 = 0

        cs.new_single_gate_for_trace_step(
            Width4MainGateWithDNextEquation::static_description(), 
            &[one, minus_one, zero, zero, zero, round_constant_placeholder, zero], 
            &[input[0].get_variable(), state_0_fifth.get_variable(), dummy_var, dummy_var], 
            &[]
        )?;

        // now it's time for a custom gate

        cs.new_single_gate_for_trace_step(
            Rescue5CustomGate::static_description(), 
            &[], 
            &[state_0.get_variable(), state_0_squared.get_variable(), state_0_quad.get_variable(), state_0_fifth.get_variable()], 
            &[]
        )?;

        // same for input[1]

        cs.new_single_gate_for_trace_step(
            Width4MainGateWithDNextEquation::static_description(), 
            &[one, minus_one, zero, zero, zero, round_constant_placeholder, zero], 
            &[input[1].get_variable(), state_0_fifth.get_variable(), dummy_var, dummy_var], 
            &[]
        )?;

        // now it's time for a custom gate

        cs.new_single_gate_for_trace_step(
            Rescue5CustomGate::static_description(), 
            &[], 
            &[state_1.get_variable(), state_1_squared.get_variable(), state_1_quad.get_variable(), state_1_fifth.get_variable()], 
            &[]
        )?;

        // now apply MDS, add constants and make next sbox manually
        // state_0 * coeff + state_1 * coeff + constant + round_constant 
        // 

        let mut state_0_after_mds_and_round_const = AllocatedNum::<E>::alloc(
            cs,
            || {
                Ok(E::Fr::one())
            }
        )?;

        cs.new_single_gate_for_trace_step(
            Width4MainGateWithDNextEquation::static_description(), 
            &[round_constant_placeholder, round_constant_placeholder, minus_one, zero, round_constant_placeholder, zero, zero], 
            &[state_0.get_variable(), state_1.get_variable(), state_0_after_mds_and_round_const.get_variable(), dummy_var], 
            &[]
        )?;

        let mut state_1_after_mds_and_round_const = AllocatedNum::<E>::alloc(
            cs,
            || {
                Ok(E::Fr::one())
            }
        )?;

        cs.new_single_gate_for_trace_step(
            Width4MainGateWithDNextEquation::static_description(), 
            &[round_constant_placeholder, round_constant_placeholder, minus_one, zero, round_constant_placeholder, zero, zero], 
            &[state_0.get_variable(), state_1.get_variable(), state_1_after_mds_and_round_const.get_variable(), dummy_var], 
            &[]
        )?;

        let mut state_2_after_mds_and_round_const = AllocatedNum::<E>::alloc(
            cs,
            || {
                Ok(E::Fr::one())
            }
        )?;

        cs.new_single_gate_for_trace_step(
            Width4MainGateWithDNextEquation::static_description(), 
            &[round_constant_placeholder, round_constant_placeholder, minus_one, zero, round_constant_placeholder, zero, zero], 
            &[state_0.get_variable(), state_1.get_variable(), state_2_after_mds_and_round_const.get_variable(), dummy_var], 
            &[]
        )?;

        for round in 1..Self::NUM_ROUNDS_DOUBLED {
            let (state_0, state_1, state_2) = if round & 1 == 0 {
                let state_0 = AllocatedNum::<E>::alloc(
                    cs,
                    || {
                        Ok(E::Fr::one())
                    }
                )?;
        
                let state_0_squared = state_0.clone();
                let state_0_quad = state_0.clone();
        
                let state_1 = AllocatedNum::<E>::alloc(
                    cs,
                    || {
                        Ok(E::Fr::one())
                    }
                )?;
        
                let state_1_squared = state_1.clone();
                let state_1_quad = state_1.clone();

                let state_2 = AllocatedNum::<E>::alloc(
                    cs,
                    || {
                        Ok(E::Fr::one())
                    }
                )?;
        
                let state_2_squared = state_2.clone();
                let state_2_quad = state_2.clone();

                // now it's time for a custom gates cause we already had round constant added
        
                cs.new_single_gate_for_trace_step(
                    Rescue5CustomGate::static_description(), 
                    &[], 
                    &[state_0.get_variable(), state_0_squared.get_variable(), state_0_quad.get_variable(), state_0_after_mds_and_round_const.get_variable()], 
                    &[]
                )?;
        
                cs.new_single_gate_for_trace_step(
                    Rescue5CustomGate::static_description(), 
                    &[], 
                    &[state_1.get_variable(), state_1_squared.get_variable(), state_1_quad.get_variable(), state_1_after_mds_and_round_const.get_variable()], 
                    &[]
                )?;
        
                cs.new_single_gate_for_trace_step(
                    Rescue5CustomGate::static_description(), 
                    &[], 
                    &[state_2.get_variable(), state_2_squared.get_variable(), state_2_quad.get_variable(), state_2_after_mds_and_round_const.get_variable()], 
                    &[]
                )?;

                (state_0, state_1, state_2)
            } else {
                // this s-box is just making into 5th power
        
                let state_0_squared = state_0_after_mds_and_round_const.clone();
                let state_0_quad = state_0_after_mds_and_round_const.clone();
                let state_0_fifth = state_0_after_mds_and_round_const.clone();
        
                let state_1_squared = state_1_after_mds_and_round_const.clone();
                let state_1_quad = state_1_after_mds_and_round_const.clone();
                let state_1_fifth = state_1_after_mds_and_round_const.clone();
        
                let state_2_squared = state_2_after_mds_and_round_const.clone();
                let state_2_quad = state_2_after_mds_and_round_const.clone();
                let state_2_fifth = state_2_after_mds_and_round_const.clone();

                cs.new_single_gate_for_trace_step(
                    Rescue5CustomGate::static_description(), 
                    &[], 
                    &[state_0_after_mds_and_round_const.get_variable(), state_0_squared.get_variable(), state_0_quad.get_variable(), state_0_fifth.get_variable()], 
                    &[]
                )?;
        
                cs.new_single_gate_for_trace_step(
                    Rescue5CustomGate::static_description(), 
                    &[], 
                    &[state_1_after_mds_and_round_const.get_variable(), state_1_squared.get_variable(), state_1_quad.get_variable(), state_1_fifth.get_variable()], 
                    &[]
                )?;
        
                cs.new_single_gate_for_trace_step(
                    Rescue5CustomGate::static_description(), 
                    &[], 
                    &[state_2_after_mds_and_round_const.get_variable(), state_2_squared.get_variable(), state_2_quad.get_variable(), state_2_fifth.get_variable()], 
                    &[]
                )?;

                (state_0_fifth, state_1_fifth, state_2_fifth)
            };

            // mds and constant

            state_0_after_mds_and_round_const = AllocatedNum::<E>::alloc(
                cs,
                || {
                    Ok(E::Fr::one())
                }
            )?;
    
            cs.new_single_gate_for_trace_step(
                Width4MainGateWithDNextEquation::static_description(), 
                &[round_constant_placeholder, round_constant_placeholder, round_constant_placeholder, minus_one, round_constant_placeholder, zero, zero], 
                &[state_0.get_variable(), state_1.get_variable(), state_2.get_variable(), state_0_after_mds_and_round_const.get_variable()], 
                &[]
            )?;
    
            state_1_after_mds_and_round_const = AllocatedNum::<E>::alloc(
                cs,
                || {
                    Ok(E::Fr::one())
                }
            )?;
    
            cs.new_single_gate_for_trace_step(
                Width4MainGateWithDNextEquation::static_description(), 
                &[round_constant_placeholder, round_constant_placeholder, round_constant_placeholder, minus_one, round_constant_placeholder, zero, zero], 
                &[state_0.get_variable(), state_1.get_variable(), state_2.get_variable(), state_1_after_mds_and_round_const.get_variable()], 
                &[]
            )?;
    
            state_2_after_mds_and_round_const = AllocatedNum::<E>::alloc(
                cs,
                || {
                    Ok(E::Fr::one())
                }
            )?;
    
            cs.new_single_gate_for_trace_step(
                Width4MainGateWithDNextEquation::static_description(), 
                &[round_constant_placeholder, round_constant_placeholder, round_constant_placeholder, minus_one, round_constant_placeholder, zero, zero], 
                &[state_0.get_variable(), state_1.get_variable(), state_2.get_variable(), state_2_after_mds_and_round_const.get_variable()], 
                &[]
            )?;
        }

        Ok(state_0_after_mds_and_round_const)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::plonk::better_better_cs::cs::*;

    #[test]
    fn test_trivial_circuit_with_gate_agnostic_cs() {
        use crate::pairing::bn256::{Bn256, Fr};

        let mut assembly = TrivialAssembly::<Bn256, PlonkCsWidth4WithNextStepParams, Width4MainGateWithDNextEquation>::new();
        let before = assembly.n;

        let input_0 = AllocatedNum::alloc(
            &mut assembly, 
            || {
                Ok(Fr::one())
            }
        ).unwrap();

        let input_1 = AllocatedNum::alloc(
            &mut assembly, 
            || {
                Ok(Fr::one())
            }
        ).unwrap();

        let _ = Rescue125::hash(&mut assembly, &vec![input_0, input_1]).unwrap();

        assert!(assembly.constraints.len() == 2);
        let num_gates = assembly.n - before;
        println!("Single rescue r = 2, c = 1, alpha = 5 invocation takes {} gates", num_gates);

        for (gate, density) in assembly.gate_density.0.into_iter() {
            println!("Custom gate {:?} selector = {:?}", gate, density);
        }

        // println!("Assembly state polys = {:?}", assembly.storage.state_map);

        // println!("Assembly setup polys = {:?}", assembly.storage.setup_map);
    }
}