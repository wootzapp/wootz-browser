// SPDX-License-Identifier: MIT

//! Extension Fields over GF32
//!
//! Correcting errors in BCH codes requires working over an extension field
//! of GF32 (or whatever the base field is, which in this library is always
//! GF32 represented using the bech32 alphabet).
//!
//! We support specifically the fields GF1024 and GF32768 (the extension
//! fields of degree 2 and 3, respectively), though we have tried to write
//! the code in such a way that more can be added if codes require them.
//!

use core::{fmt, ops};

use super::field::{ExtensionField, Field};
use crate::Fe32;

/// An element of the extension field.
#[derive(Copy, Clone, PartialEq, Eq, Hash)]
pub struct Fe32Ext<const DEG: usize> {
    /// The polynomial representation of the element in "little-endian" order;
    /// that is, the element is the sum `inner[i] * EXT_ELEM^i`.
    inner: [Fe32; DEG],
}

impl<const DEG: usize> From<Fe32> for Fe32Ext<DEG> {
    fn from(fe: Fe32) -> Self {
        let mut ret = Self { inner: [Fe32::Q; DEG] };
        ret.inner[0] = fe;
        ret
    }
}

impl<const DEG: usize> fmt::Debug for Fe32Ext<DEG> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result { fmt::Display::fmt(self, f) }
}

impl<const DEG: usize> fmt::Display for Fe32Ext<DEG> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        for elem in &self.inner {
            elem.fmt(f)?;
        }
        Ok(())
    }
}

impl<const DEG: usize> ops::Mul<&Fe32> for Fe32Ext<DEG> {
    type Output = Fe32Ext<DEG>;
    fn mul(mut self, other: &Fe32) -> Self::Output {
        for elem in &mut self.inner {
            *elem *= other;
        }
        self
    }
}

impl<const DEG: usize> ops::Mul<Fe32> for Fe32Ext<DEG> {
    type Output = Fe32Ext<DEG>;
    fn mul(self, other: Fe32) -> Self::Output { self.mul(&other) }
}

impl<const DEG: usize> ops::Mul<Fe32> for &Fe32Ext<DEG> {
    type Output = Fe32Ext<DEG>;
    fn mul(self, other: Fe32) -> Self::Output { (*self).mul(other) }
}

impl<const DEG: usize> ops::Mul<&Fe32> for &Fe32Ext<DEG> {
    type Output = Fe32Ext<DEG>;
    fn mul(self, other: &Fe32) -> Self::Output { (*self).mul(other) }
}

impl<const DEG: usize> Fe32Ext<DEG>
where
    Self: ExtensionField,
{
    /// Constructs a new extension-field element given a polynomial representation
    /// of the element in terms of the base field.
    pub const fn new(inner: [Fe32; DEG]) -> Self { Self { inner } }

    /// Multiplies a given element by the extension-defining element.
    fn mul_by_ext_elem(&mut self) {
        let xn_coeff = self.inner[DEG - 1];
        for i in (0..DEG - 1).rev() {
            self.inner[i + 1] = self.inner[i];
        }
        self.inner[0] = Fe32::Q;
        for i in 0..DEG {
            self.inner[i] += xn_coeff * Self::POLYNOMIAL.inner[i]
        }
    }

    // We just use naive n^2 muliplication because this is easy to write in
    // generic code, and because our GF32 implementation makes multiplication
    // (almost) as cheap as addition.
    //
    // Specifically for DEG = 2, 3 which we care about, Karatsuba multiplication
    // may be more efficient -- but since it trades off adds for mults, it's not
    // obviously so. Maybe worth benchmarking in the future.
    fn mul_by_elem(&self, other: &Self) -> Self {
        let mut acc = Self::ZERO;
        for xi in other.inner.iter().rev() {
            acc.mul_by_ext_elem();
            acc += self * xi;
        }
        acc
    }
}

/// The field of order 1024.
pub type Fe1024 = Fe32Ext<2>;

impl Field for Fe1024 {
    /// The zero element of the field.
    const ZERO: Self = Self::new([Fe32::Q, Fe32::Q]);

    /// The one element of the field.
    const ONE: Self = Self::new([Fe32::P, Fe32::Q]);

    // Chosen somewhat arbitrarily.
    /// A generator of the field.
    const GENERATOR: Self = Self::new([Fe32::P, Fe32::H]);

    /// The order of the multiplicative group of the field.
    ///
    /// This constant also serves as a compile-time check that we can count
    /// the entire field using a `usize` as a counter.
    const MULTIPLICATIVE_ORDER: usize = 1023;

    const MULTIPLICATIVE_ORDER_FACTORS: &'static [usize] = &[1, 3, 11, 31, 33, 93, 341, 1023];

    #[inline]
    fn _add(&self, other: &Self) -> Self {
        Self::new([self.inner[0] + other.inner[0], self.inner[1] + other.inner[1]])
    }

    #[inline]
    fn _sub(&self, other: &Self) -> Self { self._add(other) }

    #[inline]
    fn _mul(&self, other: &Self) -> Self { self.mul_by_elem(other) }

    #[inline]
    fn _div(&self, other: &Self) -> Self { other.multiplicative_inverse() * self }

    #[inline]
    fn _neg(self) -> Self { self }

    fn multiplicative_inverse(self) -> Self {
        // Aliases to make the below equations easier to read
        let a0 = self.inner[0];
        let a1 = self.inner[1];
        let p0 = Self::POLYNOMIAL.inner[0];
        let p1 = Self::POLYNOMIAL.inner[1];

        // Inverse of the 2x2 multiplication matrix defined by a0, a1.
        let det = (a0 * a0) + (p1 * a0 * a1) + (p0 * a1 * a1);
        Self::new([(a0 + p1 * a1) / det, (Fe32::Q - a1) / det])
    }
}
super::impl_ops_for_fe!(impl for Fe1024);

impl ExtensionField for Fe1024 {
    type BaseField = Fe32;

    const DEGREE: usize = 2;

    // Ultimately it doesn't really matter what choice of polynomial we make
    // here. We choose the value from BIP 93, which we note differs from the
    // value used in bech32 error correcting code, such as
    // https://github.com/sipa/bech32/commit/e97932d4c86e343ace49ae6170ae0c4871820152
    //
    // (Specifically, the third element of that exp table, 311, expanded into binary,
    // 01001 10111, and mapped back to Fe32, gives us FH rather than PP. But really,
    // it doesn't matter, except that it is part of the `Fe1024` API and we cannot
    // change it once we have published it, since changing it amounts to moving to
    // a different, though isomorphic, field.)
    const POLYNOMIAL: Self = Self::new([Fe32::P, Fe32::P]);

    /// The element zeta such that the extension field is defined as `GF32[zeta]`.
    ///
    /// Alternately, the image of x in the mapping `GF32[x]/p(x) -> <the field>`
    const EXT_ELEM: Self = Self::new([Fe32::Q, Fe32::P]);
}

/// The field of order 32768.
pub type Fe32768 = Fe32Ext<3>;

impl Field for Fe32768 {
    /// The zero element of the field.
    const ZERO: Self = Self::new([Fe32::Q, Fe32::Q, Fe32::Q]);

    /// The one element of the field.
    const ONE: Self = Self::new([Fe32::P, Fe32::Q, Fe32::Q]);

    // Chosen somewhat arbitrarily, by just guessing values until one came
    // out with the correct order.
    /// A generator of the field.
    const GENERATOR: Self = Self::new([Fe32::A, Fe32::C, Fe32::Q]);

    /// The order of the multiplicative group of the field.
    ///
    /// This constant also serves as a compile-time check that we can count
    /// the entire field using a `usize` as a counter.
    const MULTIPLICATIVE_ORDER: usize = 32767;

    const MULTIPLICATIVE_ORDER_FACTORS: &'static [usize] = &[1, 7, 31, 151, 217, 1057, 4681, 32767];

    #[inline]
    fn _add(&self, other: &Self) -> Self {
        Self::new([
            self.inner[0] + other.inner[0],
            self.inner[1] + other.inner[1],
            self.inner[2] + other.inner[2],
        ])
    }

    #[inline]
    fn _sub(&self, other: &Self) -> Self { self._add(other) }

    #[inline]
    fn _mul(&self, other: &Self) -> Self { self.mul_by_elem(other) }

    #[inline]
    fn _div(&self, other: &Self) -> Self { other.multiplicative_inverse() * self }

    #[inline]
    fn _neg(self) -> Self { self }

    fn multiplicative_inverse(self) -> Self {
        // Unlike in the GF1024 case we don't bother being generic over
        // arbitrary values of POLYNOMIAL, since doing so means a ton
        // of extra work for everybody (me, the reviewer, and the CPU
        // that has to do a bunch of mulitplications by values that
        // turn out to always be 0).
        debug_assert_eq!(Self::POLYNOMIAL, Self::new([Fe32::P, Fe32::P, Fe32::Q]));
        // Aliases to make the below equations easier to read
        let a0 = self.inner[0];
        let a1 = self.inner[1];
        let a2 = self.inner[2];

        let a0_2 = a0 * a0;
        let a1_2 = a1 * a1;
        let a2_2 = a2 * a2;

        let a0a1 = a0 * a1;
        let a0a2 = a0 * a2;
        let a1a2 = a1 * a2;

        // Inverse of the 3x3 multiplication matrix defined by a0, a1, a2.
        let det = (a0_2 * a0) + a1_2 * (a0 + a1) + a2_2 * (a0 + a1 + a2) + (a0 * a1a2);
        Self::new([
            (a0_2 + a1_2 + a2_2 + a1a2) / det,
            (a2_2 + a0a1) / det,
            (a1_2 + a2_2 + a0a2) / det,
        ])
    }
}
super::impl_ops_for_fe!(impl for Fe32768);

impl ExtensionField for Fe32768 {
    type BaseField = Fe32;

    const DEGREE: usize = 3;

    // Arbitrary irreducible polynomial x^3 = x + 1
    const POLYNOMIAL: Self = Self::new([Fe32::P, Fe32::P, Fe32::Q]);

    /// The element zeta such that the extension field is defined as `GF32[zeta]`.
    ///
    /// Alternately, the image of x in the mapping `GF32[x]/p(x) -> <the field>`
    const EXT_ELEM: Self = Self::new([Fe32::Q, Fe32::P, Fe32::Q]);
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn gf1024_div() {
        for a0 in 0..32 {
            for a1 in 0..32 {
                let gf1 = Fe1024::new([Fe32(a0), Fe32(a1)]);
                if gf1 == Fe1024::ZERO {
                    continue;
                }
                assert_eq!(gf1 / gf1, Fe1024::ONE);
            }
        }

        const ITERS: u8 = 10; // max 32.
        for a0 in 0..ITERS {
            for a1 in 0..ITERS {
                for b0 in 0..ITERS {
                    for b1 in 0..ITERS {
                        let gf1 = Fe1024::new([Fe32(a0), Fe32(a1)]);
                        let gf2 = Fe1024::new([Fe32(b0), Fe32(b1)]);
                        if gf1 == Fe1024::ZERO {
                            continue;
                        }
                        let rat = gf2 / gf1;
                        assert_eq!(rat * gf1, gf2);
                        assert_eq!(gf1 * rat, gf2);
                    }
                }
            }
        }
    }

    #[test]
    fn gf1024_mult() {
        // Check that all base field elements to the power of 32 are themselves
        for i in 0..32 {
            let mut sq = Fe32(i);
            for _ in 0..5 {
                sq = sq * sq;
            }
            assert_eq!(sq, Fe32(i));
        }
        // Check that all ext field elements to the power of 1024 are themselves
        for j in 0..32 {
            for i in 0..32 {
                let mut sq = Fe1024::new([Fe32(i), Fe32(j)]);
                for _ in 0..10 {
                    sq = sq * sq;
                }
                assert_eq!(sq, Fe1024::new([Fe32(i), Fe32(j)]));
            }
        }

        assert_eq!(Fe1024::EXT_ELEM * Fe1024::EXT_ELEM, Fe1024::POLYNOMIAL,);
    }

    #[test]
    fn gf1024_mult_inverse() {
        assert_eq!(Fe1024::ONE.multiplicative_inverse(), Fe1024::ONE);

        for i in 0..32 {
            for j in 0..32 {
                if i != 0 || j != 0 {
                    let fe1024 = Fe1024::new([Fe32(i), Fe32(j)]);
                    assert_eq!(fe1024.multiplicative_inverse().multiplicative_inverse(), fe1024,);
                }
            }
        }
    }

    #[test]
    fn gf1024_powi() {
        // A "random" element
        let elem = Fe1024::new([Fe32::K, Fe32::L]);
        assert_eq!(elem.powi(2), elem * elem);
        assert_eq!(elem.powi(3), elem * elem * elem);
        assert_eq!(elem.powi(0), Fe1024::ONE);
        assert_eq!(elem.powi(-1), elem.multiplicative_inverse());

        assert_eq!(elem.multiplicative_order(), 1023);
        assert_eq!(elem.powi(3).multiplicative_order(), 341);
        assert_eq!(elem.powi(341).multiplicative_order(), 3);
    }

    #[test]
    fn gf32768_mult_inverse() {
        assert_eq!(Fe32768::ONE.multiplicative_inverse(), Fe32768::ONE);

        for i in 0..32 {
            for j in 0..32 {
                for k in 0..32 {
                    if i != 0 || j != 0 || k != 0 {
                        let fe32768 = Fe32768::new([Fe32(i), Fe32(j), Fe32(k)]);
                        assert_eq!(
                            fe32768.multiplicative_inverse().multiplicative_inverse(),
                            fe32768,
                        );
                    }
                }
            }
        }
    }

    #[test]
    fn gf32768_powi() {
        // A "random" element
        let elem = Fe32768::new([Fe32::A, Fe32::C, Fe32::Q]);
        assert_eq!(elem.powi(2), elem * elem);
        assert_eq!(elem.powi(3), elem * elem * elem);
        assert_eq!(elem.powi(0), Fe32768::ONE);
        assert_eq!(elem.powi(-1), elem.multiplicative_inverse());

        assert_eq!(elem.multiplicative_order(), 32767);
        assert_eq!(elem.powi(7).multiplicative_order(), 4681);
        assert_eq!(elem.powi(341).multiplicative_order(), 1057);
    }
}
