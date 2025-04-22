use rescue_hash::{RescueEngine, RescueHashParams, StatefulRescue};
use crate::pairing::ff::{PrimeField, PrimeFieldRepr};
use crate::byteorder::{ByteOrder, BigEndian};

use super::*;

#[derive(Clone)]
pub struct RescueTranscript<'a, E: RescueEngine> {
    state: StatefulRescue<'a, E>,
}

impl<'a, E: RescueEngine> RescueTranscript<'a, E> {
    pub fn from_params(params: &'a E::Params) -> Self {
        let stateful = StatefulRescue::new(params);

        Self {
            state: stateful
        }
    }
}


impl<'a, E: RescueEngine> Prng<E::Fr> for RescueTranscript<'a, E> {
    type Input = E::Fr;
    type InitializationParameters = &'a E::Params;

    fn new() -> Self {
        unimplemented!()
    }

    fn new_from_params(params: Self::InitializationParameters) -> Self {
        let stateful = StatefulRescue::new(params);

        Self {
            state: stateful
        }
    }

    fn commit_input(&mut self, input: &Self::Input) {
        self.state.absorb_single_value(*input);
    }

    fn get_challenge(&mut self) -> E::Fr {
        let value = self.state.squeeze_out_single();

        value
    }
}