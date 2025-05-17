use super::tree_hash::*;
use super::*;
use crate::pairing::ff::{PrimeField, PrimeFieldRepr};
use crate::pairing::Engine;
use crate::plonk::commitments::transparent::utils::log2_floor;
use crate::worker::Worker;

#[derive(Debug)]
pub struct BinaryTree<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    pub(crate) size: usize,
    pub(crate) num_leafs: usize,
    pub(crate) num_combined: usize,
    pub(crate) nodes: Vec<H::Output>,
    pub(crate) params: BinaryTreeParams,
    pub(crate) tree_hasher: H,
    _marker: std::marker::PhantomData<E>,
}

#[derive(Clone, PartialEq, Eq, Debug)]
pub struct BinaryTreeParams {
    pub values_per_leaf: usize,
}

use std::time::Instant;

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> BinaryTree<E, H> {
    fn hash_into_leaf(tree_hasher: &H, values: &[E::Fr]) -> H::Output {
        tree_hasher.leaf_hash(values)
    }

    fn make_full_path(&self, leaf_index: usize, leaf_pair_hash: H::Output) -> Vec<H::Output> {
        let mut nodes = &self.nodes[..];

        let mut path = vec![];
        path.push(leaf_pair_hash);

        let mut idx = leaf_index;
        idx >>= 1;

        for _ in 0..log2_floor(nodes.len() / 2) {
            let half_len = nodes.len() / 2;
            let (next_level, this_level) = nodes.split_at(half_len);
            let pair_idx = idx ^ 1usize;
            let value = this_level[pair_idx];
            path.push(value);
            idx >>= 1;
            nodes = next_level;
        }

        path
    }

    pub(crate) fn size(&self) -> usize {
        self.size
    }

    pub(crate) fn num_leafs(&self) -> usize {
        self.num_leafs
    }

    pub fn create_from_combined_leafs(leafs: &[Vec<&[E::Fr]>], num_combined: usize, tree_hasher: H, params: &BinaryTreeParams) -> Self {
        let values_per_leaf = params.values_per_leaf;
        let num_leafs = leafs.len();
        let values_per_leaf_supplied = leafs[0].len() * leafs[0][0].len();
        assert_eq!(num_combined, leafs[0].len(), "invalid number of total combined leafs");
        assert_eq!(values_per_leaf, values_per_leaf_supplied, "values per leaf from params and from data is not consistent");
        assert!(num_leafs.is_power_of_two(), "tree must be binary");

        let num_nodes = num_leafs;

        let size = num_leafs * values_per_leaf;

        let mut nodes = vec![H::placeholder_output(); num_nodes];

        let worker = Worker::new();

        let mut leaf_hashes = vec![H::placeholder_output(); num_leafs];

        let hasher_ref = &tree_hasher;

        {
            worker.scope(leaf_hashes.len(), |scope, chunk| {
                for (i, lh) in leaf_hashes.chunks_mut(chunk).enumerate() {
                    scope.spawn(move |_| {
                        let mut scratch_space = Vec::with_capacity(values_per_leaf);
                        let base_idx = i * chunk;
                        for (j, lh) in lh.iter_mut().enumerate() {
                            // idx is index of the leaf
                            let idx = base_idx + j;
                            let leaf_values_ref = &leafs[idx];
                            for &lv in leaf_values_ref.iter() {
                                scratch_space.extend_from_slice(lv);
                            }
                            debug_assert_eq!(scratch_space.len(), values_per_leaf);
                            *lh = hasher_ref.leaf_hash(&scratch_space[..]);
                            scratch_space.truncate(0);
                        }
                    });
                }
            });
        }

        println!("Leaf hashes completed");

        // leafs are now encoded and hashed, so let's make a tree

        let num_levels = log2_floor(num_leafs) as usize;
        let mut nodes_for_hashing = &mut nodes[..];

        // separately hash last level, which hashes leaf hashes into first nodes
        {
            let _level = num_levels - 1;
            let inputs = &mut leaf_hashes[..];
            let (_, outputs) = nodes_for_hashing.split_at_mut(nodes_for_hashing.len() / 2);
            assert!(outputs.len() * 2 == inputs.len());
            assert!(outputs.len().is_power_of_two());

            worker.scope(outputs.len(), |scope, chunk| {
                for (o, i) in outputs.chunks_mut(chunk).zip(inputs.chunks(chunk * 2)) {
                    scope.spawn(move |_| {
                        let mut hash_input = [H::placeholder_output(); 2];
                        for (o, i) in o.iter_mut().zip(i.chunks(2)) {
                            hash_input[0] = i[0];
                            hash_input[1] = i[1];
                            *o = hasher_ref.node_hash(&hash_input, _level);
                        }
                    });
                }
            });
        }

        for _level in (0..(num_levels - 1)).rev() {
            // do the trick - split
            let (next_levels, inputs) = nodes_for_hashing.split_at_mut(nodes_for_hashing.len() / 2);
            let (_, outputs) = next_levels.split_at_mut(next_levels.len() / 2);
            assert!(outputs.len() * 2 == inputs.len());
            assert!(outputs.len().is_power_of_two());

            worker.scope(outputs.len(), |scope, chunk| {
                for (o, i) in outputs.chunks_mut(chunk).zip(inputs.chunks(chunk * 2)) {
                    scope.spawn(move |_| {
                        let mut hash_input = [H::placeholder_output(); 2];
                        for (o, i) in o.iter_mut().zip(i.chunks(2)) {
                            hash_input[0] = i[0];
                            hash_input[1] = i[1];
                            *o = hasher_ref.node_hash(&hash_input, _level);
                        }
                    });
                }
            });

            nodes_for_hashing = next_levels;
        }

        Self {
            size: size,
            num_leafs: num_leafs,
            nodes: nodes,
            num_combined,
            tree_hasher: tree_hasher,
            params: params.clone(),
            _marker: std::marker::PhantomData,
        }
    }

    pub(crate) fn create(values: &[E::Fr], tree_hasher: H, params: &BinaryTreeParams) -> Self {
        assert!(params.values_per_leaf.is_power_of_two());

        let values_per_leaf = params.values_per_leaf;
        let num_leafs = values.len() / values_per_leaf;
        assert!(num_leafs.is_power_of_two());

        let num_nodes = num_leafs;

        // size is a total number of elements
        let size = values.len();

        let mut nodes = vec![H::placeholder_output(); num_nodes];

        let worker = Worker::new();

        let mut leaf_hashes = vec![H::placeholder_output(); num_leafs];

        let hasher_ref = &tree_hasher;

        {
            worker.scope(leaf_hashes.len(), |scope, chunk| {
                for (i, lh) in leaf_hashes.chunks_mut(chunk).enumerate() {
                    scope.spawn(move |_| {
                        let base_idx = i * chunk;
                        for (j, lh) in lh.iter_mut().enumerate() {
                            let idx = base_idx + j;
                            let values_start = idx * values_per_leaf;
                            let values_end = values_start + values_per_leaf;
                            *lh = hasher_ref.leaf_hash(&values[values_start..values_end]);
                        }
                    });
                }
            });
        }

        // leafs are now encoded and hashed, so let's make a tree

        let num_levels = log2_floor(num_leafs) as usize;
        let mut nodes_for_hashing = &mut nodes[..];

        // separately hash last level, which hashes leaf hashes into first nodes
        {
            let _level = num_levels - 1;
            let inputs = &mut leaf_hashes[..];
            let (_, outputs) = nodes_for_hashing.split_at_mut(nodes_for_hashing.len() / 2);
            assert!(outputs.len() * 2 == inputs.len());
            assert!(outputs.len().is_power_of_two());

            worker.scope(outputs.len(), |scope, chunk| {
                for (o, i) in outputs.chunks_mut(chunk).zip(inputs.chunks(chunk * 2)) {
                    scope.spawn(move |_| {
                        let mut hash_input = [H::placeholder_output(); 2];
                        for (o, i) in o.iter_mut().zip(i.chunks(2)) {
                            hash_input[0] = i[0];
                            hash_input[1] = i[1];
                            *o = hasher_ref.node_hash(&hash_input, _level);
                        }
                    });
                }
            });
        }

        for _level in (0..(num_levels - 1)).rev() {
            // do the trick - split
            let (next_levels, inputs) = nodes_for_hashing.split_at_mut(nodes_for_hashing.len() / 2);
            let (_, outputs) = next_levels.split_at_mut(next_levels.len() / 2);
            assert!(outputs.len() * 2 == inputs.len());
            assert!(outputs.len().is_power_of_two());

            worker.scope(outputs.len(), |scope, chunk| {
                for (o, i) in outputs.chunks_mut(chunk).zip(inputs.chunks(chunk * 2)) {
                    scope.spawn(move |_| {
                        let mut hash_input = [H::placeholder_output(); 2];
                        for (o, i) in o.iter_mut().zip(i.chunks(2)) {
                            hash_input[0] = i[0];
                            hash_input[1] = i[1];
                            *o = hasher_ref.node_hash(&hash_input, _level);
                        }
                    });
                }
            });

            nodes_for_hashing = next_levels;
        }

        Self {
            size: size,
            nodes: nodes,
            num_leafs: num_leafs,
            num_combined: 1,
            tree_hasher: tree_hasher,
            params: params.clone(),
            _marker: std::marker::PhantomData,
        }
    }

    pub fn get_commitment(&self) -> H::Output {
        self.nodes[1].clone()
    }

    pub fn produce_query(&self, indexes: Vec<usize>, values: &[E::Fr]) -> Query<E, H> {
        // we never expect that query is mis-alligned, so check it
        debug_assert!(indexes[0] % self.params.values_per_leaf == 0);
        debug_assert!(indexes.len() == self.params.values_per_leaf);
        debug_assert!(indexes == (indexes[0]..(indexes[0] + self.params.values_per_leaf)).collect::<Vec<_>>());
        debug_assert!(*indexes.last().expect("is some") < self.size());
        debug_assert!(*indexes.last().expect("is some") < values.len());

        let query_values = Vec::from(&values[indexes[0]..(indexes[0] + self.params.values_per_leaf)]);

        let leaf_index = indexes[0] / self.params.values_per_leaf;

        let pair_index = leaf_index ^ 1;

        let leaf_pair_hash = self
            .tree_hasher
            .leaf_hash(&values[(pair_index * self.params.values_per_leaf)..((pair_index + 1) * self.params.values_per_leaf)]);

        let path = self.make_full_path(leaf_index, leaf_pair_hash);

        Query::<E, H> {
            indexes: indexes,
            values: query_values,
            path: path,
        }
    }

    pub fn produce_multiquery(&self, indexes: Vec<usize>, num_combined: usize, leafs: &[Vec<&[E::Fr]>]) -> MultiQuery<E, H> {
        // debug_assert!(indexes[0] % self.params.values_per_leaf == 0);
        // debug_assert!(indexes.len() == self.params.values_per_leaf);
        debug_assert!(indexes == (indexes[0]..(indexes[0] + (self.params.values_per_leaf / self.num_combined))).collect::<Vec<_>>());
        debug_assert!(*indexes.last().expect("is some") < self.size());
        debug_assert!(leafs[0].len() == num_combined);

        let leaf_index = indexes[0] / (self.params.values_per_leaf / num_combined);

        let mut query_values = Vec::with_capacity(indexes.len());
        let this_leaf = &leafs[leaf_index];

        for part in this_leaf.iter() {
            query_values.push(part.to_vec());
        }

        let pair_index = leaf_index ^ 1;

        let mut scratch_space = Vec::with_capacity(self.params.values_per_leaf);

        let pair_leaf_combination = &leafs[pair_index];

        for r in pair_leaf_combination.iter() {
            // walk over the polynomials
            scratch_space.extend_from_slice(r);
        }

        let leaf_pair_hash = self.tree_hasher.leaf_hash(&scratch_space);

        let path = self.make_full_path(leaf_index, leaf_pair_hash);

        MultiQuery::<E, H> {
            indexes: indexes,
            values: query_values,
            num_combined,
            path: path,
        }
    }

    pub fn verify_query(commitment: &H::Output, query: &Query<E, H>, params: &BinaryTreeParams, tree_hasher: &H) -> bool {
        if query.values().len() != params.values_per_leaf {
            return false;
        }

        let mut hash = tree_hasher.leaf_hash(query.values());
        let mut idx = query.indexes()[0] / params.values_per_leaf;
        let mut hash_input = [H::placeholder_output(); 2];

        for el in query.path.iter() {
            {
                if idx & 1usize == 0 {
                    hash_input[0] = hash;
                    hash_input[1] = *el;
                } else {
                    hash_input[0] = *el;
                    hash_input[1] = hash;
                }
            }
            hash = tree_hasher.node_hash(&hash_input, 0);
            idx >>= 1;
        }

        &hash == commitment
    }

    pub fn verify_multiquery(commitment: &H::Output, query: &MultiQuery<E, H>, params: &BinaryTreeParams, tree_hasher: &H) -> bool {
        let values = query.values_flattened();
        if values.len() != params.values_per_leaf {
            return false;
        }

        let num_combined = query.num_combined();

        let mut hash = tree_hasher.leaf_hash(&values);
        let mut idx = query.indexes()[0] / (params.values_per_leaf / num_combined);
        let mut hash_input = [H::placeholder_output(); 2];

        for el in query.path.iter() {
            {
                if idx & 1usize == 0 {
                    hash_input[0] = hash;
                    hash_input[1] = *el;
                } else {
                    hash_input[0] = *el;
                    hash_input[1] = hash;
                }
            }
            hash = tree_hasher.node_hash(&hash_input, 0);
            idx >>= 1;
        }

        &hash == commitment
    }
}

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> PartialEq for BinaryTree<E, H> {
    fn eq(&self, other: &Self) -> bool {
        self.get_commitment() == other.get_commitment()
    }
}

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> Eq for BinaryTree<E, H> {}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Query<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    indexes: Vec<usize>,
    values: Vec<E::Fr>,
    path: Vec<H::Output>,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct MultiQuery<E: Engine, H: BinaryTreeHasher<E::Fr>> {
    indexes: Vec<usize>,
    values: Vec<Vec<E::Fr>>,
    num_combined: usize,
    path: Vec<H::Output>,
}

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> Query<E, H> {
    pub fn indexes(&self) -> Vec<usize> {
        self.indexes.clone()
    }

    pub fn values(&self) -> &[E::Fr] {
        &self.values
    }

    pub fn path(&self) -> &[H::Output] {
        &self.path
    }
}

impl<E: Engine, H: BinaryTreeHasher<E::Fr>> MultiQuery<E, H> {
    fn indexes(&self) -> Vec<usize> {
        self.indexes.clone()
    }

    fn values_flattened(&self) -> Vec<E::Fr> {
        let mut concat = Vec::with_capacity(self.values.len() + self.values[0].len());
        for v in self.values.iter() {
            concat.extend_from_slice(&v[..]);
        }

        concat
    }

    fn num_combined(&self) -> usize {
        debug_assert_eq!(self.num_combined, self.values.len());

        self.num_combined
    }
}
