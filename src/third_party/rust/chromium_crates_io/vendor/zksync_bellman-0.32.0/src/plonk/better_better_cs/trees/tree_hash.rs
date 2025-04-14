use crate::pairing::ff::{Field, PrimeField};

pub trait BinaryTreeHasher<F: PrimeField>: Sized + Send + Sync + Clone {
    type Output: Sized + Clone + Copy + Send + Sync + PartialEq + Eq;

    fn placeholder_output() -> Self::Output;
    fn leaf_hash(&self, input: &[F]) -> Self::Output;
    fn node_hash(&self, input: &[Self::Output; 2], level: usize) -> Self::Output;
}
