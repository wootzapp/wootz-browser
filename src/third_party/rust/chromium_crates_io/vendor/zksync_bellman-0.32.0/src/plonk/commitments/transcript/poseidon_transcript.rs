use poseidon_hash::{PoseidonEngine, PoseidonHashParams, StatefulSponge};
use crate::pairing::ff::{PrimeField, PrimeFieldRepr};
use crate::byteorder::{ByteOrder, BigEndian};

use super::*;

#[derive(Clone)]
pub struct PoseidonTranscript<'a, E: PoseidonEngine> {
    state: StatefulSponge<'a, E>,
}

impl<'a, E: PoseidonEngine> PoseidonTranscript<'a, E> {
    pub fn from_params(params: &'a E::Params) -> Self {
        let stateful = StatefulSponge::new(params);

        Self {
            state: stateful
        }
    }
}


impl<'a, E: PoseidonEngine> Prng<E::Fr> for PoseidonTranscript<'a, E> {
    type Input = E::Fr;
    type InitializationParameters = &'a E::Params;

    fn new() -> Self {
        unimplemented!()
    }

    fn new_from_params(params: Self::InitializationParameters) -> Self {
        let stateful = StatefulSponge::new(params);

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