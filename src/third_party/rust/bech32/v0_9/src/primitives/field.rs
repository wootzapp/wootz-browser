// SPDX-License-Identifier: MIT

//! Generic Field Traits

use core::{fmt, hash, ops};

/// A generic field.
pub trait Field:
    Sized
    + PartialEq
    + Eq
    + Clone
    + hash::Hash
    + fmt::Debug
    + fmt::Display
    + ops::Add<Self, Output = Self>
    + ops::Sub<Self, Output = Self>
    + ops::AddAssign
    + ops::SubAssign
    + ops::Mul<Self, Output = Self>
    + ops::MulAssign
    + ops::Div<Self, Output = Self>
    + ops::DivAssign
    + for<'a> ops::Add<&'a Self, Output = Self>
    + for<'a> ops::AddAssign<&'a Self>
    + for<'a> ops::Sub<&'a Self, Output = Self>
    + for<'a> ops::SubAssign<&'a Self>
    + for<'a> ops::Mul<&'a Self, Output = Self>
    + for<'a> ops::MulAssign<&'a Self>
    + for<'a> ops::Div<&'a Self, Output = Self>
    + for<'a> ops::DivAssign<&'a Self>
    + ops::Neg
{
    /// The zero constant of the field.
    const ZERO: Self;

    /// The one constant of the field.
    const ONE: Self;

    /// A primitive element, i.e. a generator of the multiplicative group of the field.
    const GENERATOR: Self;

    /// The order of the multiplicative group of the field.
    const MULTIPLICATIVE_ORDER: usize;

    /// All factors of the multiplicative order, in increasing order.
    ///
    /// Include both 1 and the number itself. So for example if you have `n` distinct
    /// prime factors which each appearing once, this array would have size `2^n`.
    const MULTIPLICATIVE_ORDER_FACTORS: &'static [usize];

    /// Adds a value to `self`. This is a helper function for implementing the
    /// [`ops::Add`] and [`ops::AddAssign`] traits, which should probably be called
    /// instead of calling this.
    fn _add(&self, other: &Self) -> Self;

    /// Subtracts a value from `self`. This is a helper function for implementing the
    /// [`ops::Sub`] and [`ops::SubAssign`] traits, which should probably be called
    /// instead of calling this.
    fn _sub(&self, other: &Self) -> Self;

    /// Multiplies a value by `self`. This is a helper function for implementing the
    /// [`ops::Mul`] and [`ops::MulAssign`] traits, which should probably be called
    /// instead of calling this.
    fn _mul(&self, other: &Self) -> Self;

    /// Divides a value from `self`. This is a helper function for implementing the
    /// [`ops::Div`] and [`ops::DivAssign`] traits, which should probably be called
    /// instead of calling this.
    fn _div(&self, other: &Self) -> Self;

    /// Computes the additive inverse of an element.
    fn _neg(self) -> Self;

    /// Computes the multiplicative inverse of an element.
    fn multiplicative_inverse(self) -> Self;

    /// Takes the element to the power of some integer.
    fn powi(&self, mut n: i64) -> Self {
        let base = if n >= 0 {
            self.clone()
        } else {
            n *= -1;
            self.clone().multiplicative_inverse()
        };
        n %= Self::MULTIPLICATIVE_ORDER as i64;

        let mut mask = Self::MULTIPLICATIVE_ORDER.next_power_of_two() as i64;
        let mut ret = Self::ONE;
        while mask > 0 {
            ret *= ret.clone();
            if n & mask != 0 {
                ret *= base.clone();
            }
            mask >>= 1;
        }
        ret
    }

    /// The multiplicative order of an element.
    fn multiplicative_order(&self) -> usize {
        for &ord in Self::MULTIPLICATIVE_ORDER_FACTORS {
            if self.powi(ord as i64) == Self::ONE {
                return ord;
            }
        }
        panic!(
            "bug: `ExtensionField::MULTIPLICATIVE_ORDER_FACTORS` did not include full group order"
        );
    }
}

/// Trait describing a simple extension field (field obtained from another by
/// adjoining one element).
pub trait ExtensionField: Field + From<Self::BaseField> {
    /// The type of the base field.
    type BaseField: Field;

    /// The degree of the extension.
    ///
    /// Must be strictly greater than 1.
    const DEGREE: usize;

    /// An extension field is defined as `GF32[x]/p(x)`, for some irreducible
    /// monic polynomial p whose degree then becomes the degree of the extension.
    ///
    /// If p(x) = x^d + ... p_1x + p_0 we can represent p by an element of
    /// the extension field, specifically the image of p(x) - x^d. Equivalently,
    /// if zeta is the image of x in the quotient map, then this value is
    /// equal to zeta^d.
    ///
    /// This value is used to define multiplication in the extension field.
    const POLYNOMIAL: Self;

    /// The element which is adjoined to the base field to get this field.
    ///
    /// In other words, the image of x in the isomorphism from
    /// [`Self::BaseField`]`[x]`/[`Self::POLYNOMIAL`] to [`Self`].
    const EXT_ELEM: Self;
}

macro_rules! impl_ops_for_fe {
    (impl for $op:ident) => {
        // add
        impl core::ops::Add<$op> for $op {
            type Output = Self;
            #[inline]
            fn add(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._add(&other)
            }
        }

        impl core::ops::Add<&$op> for $op {
            type Output = Self;
            #[inline]
            fn add(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._add(other)
            }
        }

        impl core::ops::Add<$op> for &$op {
            type Output = $op;
            #[inline]
            fn add(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._add(&other)
            }
        }

        impl core::ops::Add<&$op> for &$op {
            type Output = $op;
            #[inline]
            fn add(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._add(other)
            }
        }

        impl core::ops::AddAssign for $op {
            #[inline]
            fn add_assign(&mut self, other: $op) {
                use $crate::primitives::Field as _;
                *self = self._add(&other)
            }
        }

        impl core::ops::AddAssign<&$op> for $op {
            #[inline]
            fn add_assign(&mut self, other: &$op) {
                use $crate::primitives::Field as _;
                *self = self._add(other)
            }
        }

        // sub
        impl core::ops::Sub<$op> for $op {
            type Output = Self;
            #[inline]
            fn sub(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._sub(&other)
            }
        }

        impl core::ops::Sub<&$op> for $op {
            type Output = Self;
            #[inline]
            fn sub(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._sub(other)
            }
        }

        impl core::ops::Sub<$op> for &$op {
            type Output = $op;
            #[inline]
            fn sub(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._sub(&other)
            }
        }

        impl core::ops::Sub<&$op> for &$op {
            type Output = $op;
            #[inline]
            fn sub(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._sub(other)
            }
        }

        impl core::ops::SubAssign for $op {
            #[inline]
            fn sub_assign(&mut self, other: $op) {
                use $crate::primitives::Field as _;
                *self = self._sub(&other)
            }
        }

        impl core::ops::SubAssign<&$op> for $op {
            #[inline]
            fn sub_assign(&mut self, other: &$op) {
                use $crate::primitives::Field as _;
                *self = self._sub(other)
            }
        }

        // mul
        impl core::ops::Mul<$op> for $op {
            type Output = Self;
            #[inline]
            fn mul(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._mul(&other)
            }
        }

        impl core::ops::Mul<&$op> for $op {
            type Output = Self;
            #[inline]
            fn mul(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._mul(other)
            }
        }

        impl core::ops::Mul<$op> for &$op {
            type Output = $op;
            #[inline]
            fn mul(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._mul(&other)
            }
        }

        impl core::ops::Mul<&$op> for &$op {
            type Output = $op;
            #[inline]
            fn mul(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._mul(other)
            }
        }

        impl core::ops::MulAssign for $op {
            #[inline]
            fn mul_assign(&mut self, other: $op) {
                use $crate::primitives::Field as _;
                *self = self._mul(&other)
            }
        }

        impl core::ops::MulAssign<&$op> for $op {
            #[inline]
            fn mul_assign(&mut self, other: &$op) {
                use $crate::primitives::Field as _;
                *self = self._mul(other)
            }
        }

        // div
        impl core::ops::Div<$op> for $op {
            type Output = Self;
            #[inline]
            fn div(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._div(&other)
            }
        }

        impl core::ops::Div<&$op> for $op {
            type Output = Self;
            #[inline]
            fn div(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._div(other)
            }
        }

        impl core::ops::Div<$op> for &$op {
            type Output = $op;
            #[inline]
            fn div(self, other: $op) -> $op {
                use $crate::primitives::Field as _;
                self._div(&other)
            }
        }

        impl core::ops::Div<&$op> for &$op {
            type Output = $op;
            #[inline]
            fn div(self, other: &$op) -> $op {
                use $crate::primitives::Field as _;
                self._div(other)
            }
        }

        impl core::ops::DivAssign for $op {
            #[inline]
            fn div_assign(&mut self, other: $op) {
                use $crate::primitives::Field as _;
                *self = self._div(&other)
            }
        }

        impl core::ops::DivAssign<&$op> for $op {
            #[inline]
            fn div_assign(&mut self, other: &$op) {
                use $crate::primitives::Field as _;
                *self = self._div(other)
            }
        }

        // neg
        impl core::ops::Neg for $op {
            type Output = Self;
            #[inline]
            fn neg(self) -> Self {
                use $crate::primitives::Field as _;
                self._neg()
            }
        }
    };
}
pub(super) use impl_ops_for_fe;
