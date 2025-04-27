use crate::pairing::Engine;
use crate::plonk::polynomials::*;

pub mod generator;
pub mod prover;

pub struct IndexedSetup<E: Engine> {
    pub a_num_non_zero: usize,
    pub b_num_non_zero: usize,
    pub c_num_non_zero: usize,
    pub domain_h_size: usize,
    pub domain_k_size: usize,
    pub a_matrix_poly: Polynomial<E::Fr, Coefficients>,
    pub b_matrix_poly: Polynomial<E::Fr, Coefficients>,
    pub c_matrix_poly: Polynomial<E::Fr, Coefficients>,
    pub a_row_poly: Polynomial<E::Fr, Coefficients>,
    pub b_row_poly: Polynomial<E::Fr, Coefficients>,
    pub c_row_poly: Polynomial<E::Fr, Coefficients>,
    pub a_col_poly: Polynomial<E::Fr, Coefficients>,
    pub b_col_poly: Polynomial<E::Fr, Coefficients>,
    pub c_col_poly: Polynomial<E::Fr, Coefficients>,
    pub a_row_indexes: Vec<usize>,
    pub b_row_indexes: Vec<usize>,
    pub c_row_indexes: Vec<usize>,
    pub a_col_indexes: Vec<usize>,
    pub b_col_indexes: Vec<usize>,
    pub c_col_indexes: Vec<usize>,
}

pub struct Proof<E: Engine> {
    pub opening_on_domain_k_for_beta_3: E::G1Affine,
    pub a_val_on_beta_3: E::Fr,
    pub b_val_on_beta_3: E::Fr,
    pub c_val_on_beta_3: E::Fr,
    pub a_row_on_beta_3: E::Fr,
    pub b_row_on_beta_3: E::Fr,
    pub c_row_on_beta_3: E::Fr,
    pub a_col_on_beta_3: E::Fr,
    pub b_col_on_beta_3: E::Fr,
    pub c_col_on_beta_3: E::Fr,
    pub f_3_at_beta_3: E::Fr,
}
