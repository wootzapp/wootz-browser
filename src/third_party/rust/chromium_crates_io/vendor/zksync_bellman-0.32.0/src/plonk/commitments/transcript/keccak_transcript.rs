use crate::byteorder::{BigEndian, ByteOrder};
use crate::pairing::ff::{PrimeField, PrimeFieldRepr};
use tiny_keccak::Keccak;

use super::*;

#[derive(Clone)]
pub struct RollingKeccakTranscript<F: PrimeField> {
    state_part_0: [u8; 32],
    state_part_1: [u8; 32],
    challenge_counter: u32,
    _marker: std::marker::PhantomData<F>,
}

impl<F: PrimeField> RollingKeccakTranscript<F> {
    const SHAVE_BITS: u32 = 256 - F::CAPACITY;
    // const REPR_SIZE: usize = std::mem::size_of::<F::Repr>();
    const REPR_SIZE: usize = (((F::NUM_BITS as usize) / 64) + 1) * 8;
    const DST_0_TAG: u32 = 0;
    const DST_1_TAG: u32 = 1;
    const CHALLENGE_DST_TAG: u32 = 2;

    fn update(&mut self, bytes: &[u8]) {
        let old_state_0 = self.state_part_0;

        let mut input = vec![0u8; bytes.len() + 32 + 32 + 4];
        BigEndian::write_u32(&mut input[0..4], Self::DST_0_TAG);
        input[4..36].copy_from_slice(&old_state_0[..]);
        input[36..68].copy_from_slice(&self.state_part_1[..]);
        input[68..].copy_from_slice(bytes);

        let mut hasher = Keccak::new_keccak256();
        hasher.update(&input);
        hasher.finalize(&mut self.state_part_0);

        let mut input = vec![0u8; bytes.len() + 32 + 32 + 4];
        BigEndian::write_u32(&mut input[0..4], Self::DST_1_TAG);
        input[4..36].copy_from_slice(&old_state_0[..]);
        input[36..68].copy_from_slice(&self.state_part_1[..]);
        input[68..].copy_from_slice(bytes);

        let mut hasher = Keccak::new_keccak256();
        hasher.update(&input);
        hasher.finalize(&mut self.state_part_1);
    }

    fn query(&mut self) -> [u8; 32] {
        let mut input = vec![0u8; 4 + 32 + 32 + 4];
        BigEndian::write_u32(&mut input[0..4], Self::CHALLENGE_DST_TAG);
        input[4..36].copy_from_slice(&self.state_part_0[..]);
        input[36..68].copy_from_slice(&self.state_part_1[..]);
        BigEndian::write_u32(&mut input[68..72], self.challenge_counter);

        self.challenge_counter += 1;

        let mut value = [0u8; 32];
        let mut hasher = Keccak::new_keccak256();
        hasher.update(&input);
        hasher.finalize(&mut value);

        value
    }
}

impl<F: PrimeField> Prng<F> for RollingKeccakTranscript<F> {
    type Input = [u8; 32];
    type InitializationParameters = ();

    fn new() -> Self {
        assert!(F::NUM_BITS < 256);
        Self {
            state_part_0: [0u8; 32],
            state_part_1: [0u8; 32],
            challenge_counter: 0,
            _marker: std::marker::PhantomData,
        }
    }

    fn commit_input(&mut self, input: &Self::Input) {
        self.commit_bytes(input)
    }

    fn get_challenge(&mut self) -> F {
        let value = self.query();

        // let mut value: [u8; 32] = [0; 32];
        // self.state.finalize(&mut value);

        // self.state = Keccak::new_keccak256();
        // self.state.update(&value);

        let mut repr = F::Repr::default();
        let shaving_mask: u64 = 0xffffffffffffffff >> (Self::SHAVE_BITS % 64);
        repr.read_be(&value[..]).expect("will read");
        let last_limb_idx = repr.as_ref().len() - 1;
        repr.as_mut()[last_limb_idx] &= shaving_mask;
        let value = F::from_repr(repr).expect("in a field");

        // println!("Outputting {}", value);

        value
    }
}

impl<F: PrimeField> Transcript<F> for RollingKeccakTranscript<F> {
    fn commit_bytes(&mut self, bytes: &[u8]) {
        // println!("Committing bytes {:?}", bytes);
        // self.state.update(&bytes);
        self.update(&bytes);
    }

    fn commit_field_element(&mut self, element: &F) {
        // dbg!(element);
        // println!("Committing field element {:?}", element);
        let repr = element.into_repr();
        let mut bytes: Vec<u8> = vec![0u8; Self::REPR_SIZE];
        repr.write_be(&mut bytes[..]).expect("should write");

        // self.state.update(&bytes[..]);
        self.update(&bytes);
    }

    fn get_challenge_bytes(&mut self) -> Vec<u8> {
        let value = self.query();
        // let value = *(self.state.finalize().as_array());
        // self.state.update(&value[..]);

        // println!("Outputting {:?}", value);

        Vec::from(&value[..])
    }

    fn commit_fe<FF: PrimeField>(&mut self, element: &FF) {
        let repr = element.into_repr();
        let mut bytes: Vec<u8> = vec![0u8; Self::REPR_SIZE];
        repr.write_be(&mut bytes[..]).expect("should write");
        self.update(&bytes);
    }
}
