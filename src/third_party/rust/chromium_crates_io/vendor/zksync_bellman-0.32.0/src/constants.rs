pub const ETH_BLOCK_10_000_000_HASH: &'static str = "aa20f7bde5be60603f11a45fc4923aab7552be775403fc00c2e6b805e6297dbe";

use crate::byteorder::{BigEndian, ReadBytesExt};
use crate::pairing::{CurveProjective, Engine};

pub fn make_random_points_with_unknown_discrete_log_from_seed<E: Engine>(dst: &[u8], seed: &[u8], num_points: usize) -> Vec<E::G1Affine> {
    let mut result = vec![];

    use rand::chacha::ChaChaRng;
    use rand::{Rng, SeedableRng};
    // Create an RNG based on the outcome of the random beacon
    let mut rng = {
        // if we use Blake hasher
        let input: Vec<u8> = dst.iter().chain(seed.iter()).cloned().collect();
        let h = blake2s_simd::blake2s(&input);
        assert!(h.as_bytes().len() == 32);
        let mut seed = [0u32; 8];
        for (i, chunk) in h.as_bytes().chunks_exact(8).enumerate() {
            seed[i] = (&chunk[..]).read_u32::<BigEndian>().expect("digest is large enough for this to work");
        }

        ChaChaRng::from_seed(&seed)
    };

    for _ in 0..num_points {
        let point: E::G1 = rng.gen();

        result.push(point.into_affine());
    }

    result
}

pub fn make_random_points_with_unknown_discrete_log<E: Engine>(dst: &[u8], num_points: usize) -> Vec<E::G1Affine> {
    make_random_points_with_unknown_discrete_log_from_seed::<E>(dst, &hex::decode(crate::constants::ETH_BLOCK_10_000_000_HASH).unwrap(), num_points)
}
