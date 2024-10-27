// SPDX-License-Identifier: MIT

//! Polynomials over Finite Fields

#[cfg(all(feature = "alloc", not(feature = "std"), not(test)))]
use alloc::vec::Vec;
use core::{iter, ops};

use super::Field;

/// A polynomial over some field.
#[derive(PartialEq, Eq, Clone, Debug, Hash)]
pub struct Polynomial<F> {
    inner: Vec<F>,
}

impl<F> Polynomial<F> {
    /// Constructor for a polynomial from a vector of coefficients.
    ///
    /// These coefficients are in "little endian" order. That is, the ith
    /// coefficient is the one multiplied by x^i.
    pub fn new(f: Vec<F>) -> Self { Self { inner: f } }
}

impl<F: Field> Polynomial<F> {
    /// The degree of the polynomial.
    ///
    /// For constants it will return zero, including for the constant zero.
    pub fn degree(&self) -> usize {
        debug_assert_ne!(self.inner.len(), 0, "polynomials never have no terms");
        let degree_without_leading_zeros = self.inner.len() - 1;
        let leading_zeros = self.inner.iter().rev().take_while(|el| **el == F::ZERO).count();
        degree_without_leading_zeros - leading_zeros
    }

    /// The leading term of the polynomial.
    ///
    /// For the constant 0 polynomial, will return 0.
    pub fn leading_term(&self) -> F {
        for term in self.inner.iter().rev() {
            if *term != F::ZERO {
                return term.clone();
            }
        }
        F::ZERO
    }
}

impl<F: Field> Polynomial<F> {
    /// Finds all roots of the polynomial in the given field, in
    /// no particular order.
    ///
    /// Does not consider multiplicity; it assumes there are no
    /// repeated roots. (FIXME we really ought to do so, and
    /// definitely should before exposing this function in the
    /// public API.)
    ///
    /// If the polynomial does not split, then the returned vector
    /// will have length strictly less than [`Self::degree`]. If
    /// the polynomial _does_ split then the length will be equal.
    ///
    /// For constants, will return vec![0] for the constant 0 and the
    /// empty vector for any other constant. Probably the caller wants
    /// to check if they have a constant and special-case this.
    pub fn find_distinct_roots(&self) -> Vec<F> {
        // Implements Chien search

        let mut ret = Vec::with_capacity(self.degree());
        // Check if zero is a root
        if self.inner.is_empty() || self.leading_term() == F::ZERO {
            ret.push(F::ZERO);
        }
        // Special-case constants, which have 0 as a root iff they are the constant 0.
        if self.degree() == 1 {
            return ret;
            // from here on out we know self.inner[0] won't panic
        }

        // Vector of [1, gen, gen^2, ...] up to the degree d.
        debug_assert_eq!(F::GENERATOR.multiplicative_order(), F::MULTIPLICATIVE_ORDER);
        let gen_power = iter::successors(Some(F::ONE), |gen| Some(F::GENERATOR * gen))
            .take(self.degree() + 1)
            .collect::<Vec<F>>();

        // We special-cased 0 above. So now we can check every nonzero element
        // to see if it is a root. We brute-force this using Chein's algorithm,
        // which exploits the fact that we can go from f(alpha^i) to f(alpha^{i+1})
        // pretty efficiently. So iterate through all the powers of the generator
        // in this way.
        let mut cand = F::ONE;
        let mut eval = self.clone();
        for _ in 0..F::MULTIPLICATIVE_ORDER {
            let sum = eval.inner.iter().cloned().fold(F::ZERO, F::add);
            if sum == F::ZERO {
                ret.push(cand.clone());
            }

            for (i, gen_power) in gen_power.iter().enumerate() {
                eval.inner[i] *= gen_power;
            }
            cand *= F::GENERATOR;
        }

        ret
    }

    /// Given a BCH generator polynomial, find an element alpha that maximizes the
    /// consecutive range i..j such that `alpha^i `through `alpha^j` are all roots
    /// of the polynomial.
    ///
    /// (Despite the name, the returned element might not actually be a primitive
    /// element. For a "primitive BCH code" it will be, but in general not. But
    /// there is no standard name for the element this function returns, and
    /// "primitive element" is suggestive.)
    ///
    /// # Panics
    ///
    /// Panics if there are fewer roots than the degree of the polynomial, or if
    /// the longest geometric series in the roots appears to be of the form
    /// alpha*beta^i where alpha is not 1. Either situation indicates that your
    /// BCH generator polynomial is weird in some way, and you should file a bug
    /// or (more likely) fix your polynomial.
    ///
    /// # Returns
    ///
    /// Returns a primitive element, its order (which is the length of the code),
    /// and the longest range of exponents of the element which are roots of the
    /// polynomial. For the avoidance of doubt it returns a [`ops::RangeInclusive`]
    /// (syntax `a..=b`) rather than the more-common [`ops::Range`] (syntax `a..b`).
    /// Both endpoints are included in the set of values.
    ///
    /// Internally this function analyzes the roots in an arbitrary (randomized)
    /// order, and therefore may return different values on consecutive runs. If
    /// there is a particular "elegant" value you are looking for, it may be
    /// worthwhile to run the function multiple times.
    pub fn bch_generator_primitive_element(&self) -> (F, usize, ops::RangeInclusive<usize>) {
        let roots = self.find_distinct_roots();
        debug_assert!(roots.len() <= self.degree(),);
        assert_eq!(
            self.degree(),
            roots.len(),
            "Found {} roots ({:?}) for a polynomial of degree {}; polynomial appears not to split.",
            roots.len(),
            roots,
            self.degree(),
        );

        // Brute-force (worst case n^3 in the length of the polynomial) the longest
        // geometric series within the set of roots. The common ratio between these
        // roots will be our primitive element.
        //
        // We also learn the length of the series and the first root in the series.
        let mut max_length = 0;
        let mut max_start = F::ZERO;
        let mut max_ratio = F::ZERO;
        for r1 in &roots {
            for r2 in &roots {
                if r1 == r2 {
                    continue;
                }
                let ratio = r2.clone() / r1;

                let mut len = 1;
                let mut elem = r1.clone();
                while roots.contains(&(elem.clone() * &ratio)) {
                    len += 1;
                    elem *= &ratio;
                }
                if len > max_length {
                    max_length = len;
                    max_start = r2.clone();
                    max_ratio = ratio;
                }
            }
        }

        // We have the primitive element (max_ratio) and the first element in the
        // series with that ratio (max_start). To get the actual exponents of the
        // series, we need i such that max_start = max_ratio^i.
        //
        // It may occur that no such i exists, if the entire series is in a coset
        // of the group generated by max_ratio. In *theory* this means that we
        // should go back and find the second-longest geometric series and try
        // that, because for a real-life BCH code this situation indicates that
        // something is wrong and we should just panic.
        let code_len = max_ratio.multiplicative_order();

        let mut min_index = None;
        let mut base = F::ONE;
        for i in 0..code_len {
            base *= &max_ratio;
            if base == max_start {
                min_index = Some(i);
            }
        }

        let min_index = match min_index {
            Some(idx) => idx,
            None => panic!("Found geometric series within roots starting from {} (ratio {} length {}), but this series does not consist of powers of any generator.", max_start, max_ratio, max_length),
        };

        // We write `a..=b - 1` instead of `a..b` because RangeInclusive is actually
        // a different type than Range, so the two syntaxes are not equivalent here.
        (max_ratio, code_len, min_index..=min_index + max_length - 1)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{Fe1024, Fe32};

    #[test]
    fn roots() {
        let bip93_poly = Polynomial::<Fe1024>::new(
            [
                Fe32::S,
                Fe32::S,
                Fe32::C,
                Fe32::M,
                Fe32::L,
                Fe32::E,
                Fe32::E,
                Fe32::E,
                Fe32::Q,
                Fe32::G,
                Fe32::_3,
                Fe32::M,
                Fe32::E,
                Fe32::P,
            ]
            .iter()
            .copied()
            .map(Fe1024::from)
            .collect(),
        );

        assert_eq!(bip93_poly.degree(), 13);

        let (elem, order, root_indices) = bip93_poly.bch_generator_primitive_element();
        // Basically, only the order and the length of the `root_indices` range are
        // guaranteed to be consistent across runs. There will be two possible ranges,
        // a "low" one (in this case 9..16) and a "high one" (77..84) whose generator
        // is the multiplicative inverse of the small one. These correspond to the
        // fact that for any order-93 element, e^x = (e^-1)^(93 - x).
        //
        // Then in addition to the element/inverse symmetry, for this polynomial there
        // is also an entire second generator which can produce the same set of roots.
        // So we get that one and _its_ inverse.
        //
        // Also, BTW, the range 77..84 appears in the Appendix to BIP93 so you can
        // verify its correctness there.
        assert_eq!(order, 93);
        // These next three assertions just illustrate the above comment...
        assert_eq!(
            Fe1024::new([Fe32::Q, Fe32::_9]).multiplicative_inverse(),
            Fe1024::new([Fe32::G, Fe32::G]),
        );
        assert_eq!(
            Fe1024::new([Fe32::Q, Fe32::_9]).powi(9),
            Fe1024::new([Fe32::G, Fe32::G]).powi(84),
        );
        assert_eq!(
            Fe1024::new([Fe32::Q, Fe32::_9]).powi(16),
            Fe1024::new([Fe32::G, Fe32::G]).powi(77),
        );
        // ...and these ones are actual unit tests.
        if elem == Fe1024::new([Fe32::_9, Fe32::_9]) {
            assert_eq!(root_indices, 9..=16);
        } else if elem == Fe1024::new([Fe32::Q, Fe32::G]) {
            assert_eq!(root_indices, 77..=84);
        } else if elem == Fe1024::new([Fe32::Q, Fe32::_9]) {
            assert_eq!(root_indices, 9..=16);
        } else if elem == Fe1024::new([Fe32::G, Fe32::G]) {
            assert_eq!(root_indices, 77..=84);
        } else {
            panic!("Unexpected generator {}", elem);
        }
    }
}
