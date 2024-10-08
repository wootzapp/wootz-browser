// SPDX-License-Identifier: MIT

//! Degree-2 [BCH] code checksum.
//!
//! [BCH]: <https://en.wikipedia.org/wiki/BCH_code>

#[cfg(all(feature = "alloc", not(feature = "std"), not(test)))]
use alloc::vec::Vec;
#[cfg(feature = "alloc")]
use core::fmt;
#[cfg(feature = "alloc")]
use core::marker::PhantomData;
use core::{mem, ops};

#[cfg(feature = "alloc")]
use super::Polynomial;
use crate::primitives::hrp::Hrp;
#[cfg(feature = "alloc")]
use crate::Fe1024;
use crate::Fe32;

/// Trait defining a particular checksum.
///
/// For users, this can be treated as a marker trait; none of the associated data
/// are end-user relevant.
pub trait Checksum {
    /// An unsigned integer type capable of holding a packed version of the generator
    /// polynomial (without its leading 1) and target residue (which will have the
    /// same width).
    ///
    /// Generally, this is the number of characters in the checksum times 5. So e.g.
    /// for bech32, which has a 6-character checksum, we need 30 bits, so we can use
    /// u32 here.
    ///
    /// The smallest type possible should be used, for efficiency reasons, but the
    /// only operations we do on these types are bitwise xor and shifts, so it should
    /// be pretty efficient no matter what.
    type MidstateRepr: PackedFe32;

    /// The length of the code.
    ///
    /// The length of the code is how long a coded message can be (including the
    /// checksum!) for the code to retain its error-correcting properties.
    const CODE_LENGTH: usize;

    /// The number of characters in the checksum.
    ///
    /// Alternately, the degree of the generator polynomial. This is **not** the same
    /// as `Self::CODE_LENGTH`.
    const CHECKSUM_LENGTH: usize;

    /// The coefficients of the generator polynomial, except the leading monic term,
    /// in "big-endian" (highest-degree coefficients get leftmost bits) order, along
    /// with the 4 shifts of the generator.
    ///
    /// The shifts are literally the generator polynomial left-shifted (i.e. multiplied
    /// by the appropriate power of 2) in the field. That is, the 5 entries in this
    /// array are the generator times { P, Z, Y, G, S } in that order.
    ///
    /// These cannot be usefully pre-computed because of Rust's limited constfn support
    /// as of 1.67, so they must be specified manually for each checksum. To check the
    /// values for consistency, run [`Self::sanity_check`].
    const GENERATOR_SH: [Self::MidstateRepr; 5];

    /// The residue, modulo the generator polynomial, that a valid codeword will have.
    const TARGET_RESIDUE: Self::MidstateRepr;

    /// Sanity checks that the various constants of the trait are set in a way that they
    /// are consistent with each other.
    ///
    /// This function never needs to be called by users, but anyone defining a checksum
    /// should add a unit test to their codebase which calls this.
    fn sanity_check() {
        // Check that the declared midstate type can actually hold the whole checksum.
        assert!(Self::CHECKSUM_LENGTH <= Self::MidstateRepr::WIDTH);

        // Check that the provided generator polynomials are, indeed, the same polynomial just shifted.
        for i in 1..5 {
            for j in 0..Self::MidstateRepr::WIDTH {
                let last = Self::GENERATOR_SH[i - 1].unpack(j);
                let curr = Self::GENERATOR_SH[i].unpack(j);
                // GF32 is defined by extending GF2 with a root of x^5 + x^3 + 1 = 0
                // which when written as bit coefficients is 41 = 0. Hence xoring
                // (adding, in GF32) by 41 is the way to reduce x^5.
                assert_eq!(
                    curr,
                    (last << 1) ^ if last & 0x10 == 0x10 { 41 } else { 0 },
                    "Element {} of generator << 2^{} was incorrectly computed. (Should have been {} << 1)",
                    j, i, last,
                );
            }
        }
    }
}

/// Given a polynomial representation for your generator polynomial and your
/// target residue, outputs a `impl Checksum` block.
///
/// You must specify an extension field. You should try [`crate::Fe1024`], and if
/// you get an error about a polynomial not splitting, try [`crate::Fe32768`].
///
/// Used like
///
/// ```
/// # #[cfg(feature = "alloc")] {
/// use core::convert::TryFrom;
///
/// use bech32::{Fe32, Fe1024, PrintImpl};
/// use bech32::primitives::checksum::PackedFe32;
///
/// // In codes specified in BIPs, the code generator polynomial and residue
/// // are often only given indirectly, in the reference code which encodes
/// // it in a packed form (shifted multiple times).
/// //
/// // For example in the BIP173 Python reference code you will see an array
/// // called `generator` whose first entry is 0x3b6a57b2. This first entry
/// // is the generator polynomial in packed form.
/// //
/// // To get the expanded polynomial form you can use `u128::unpack` like so:
/// let unpacked_poly = (0..6)
///     .rev() // Note .rev() to convert from BE integer literal to LE polynomial!
///     .map(|i| 0x3b6a57b2u128.unpack(i))
///     .map(|u| Fe32::try_from(u).unwrap())
///     .collect::<Vec<_>>();
/// let unpacked_residue = (0..6)
///     .rev()
///     .map(|i| 0x1u128.unpack(i))
///     .map(|u| Fe32::try_from(u).unwrap())
///     .collect::<Vec<_>>();
/// println!(
///     "{}",
///     PrintImpl::<Fe1024>::new(
///         "Bech32",
///         &unpacked_poly,
///         &unpacked_residue,
///     ),
/// );
/// # }
/// ```
///
/// The awkward API is to allow this type to be used in the widest set of
/// circumstances, including in nostd settings. (However, the underlying
/// polynomial math requires the `alloc` feature.)
///
/// Both polynomial representations should be in little-endian order, so that
/// the coefficient of x^i appears in the ith slot. The generator polynomial
/// should be a monic polynomial but you should not include the monic term,
/// so that both `generator` and `target` are arrays of the same length.
///
/// **This function should never need to be called by users, but will be helpful
/// for developers.**
///
/// In general, when defining a checksum, it is best to call this method (and
/// to add a unit test that calls [`Checksum::sanity_check`] rather than trying
/// to compute the values yourself. The reason is that the specific values
/// used depend on the representation of extension fields, which may differ
/// between implementations (and between specifications) of your BCH code.
#[cfg(feature = "alloc")]
pub struct PrintImpl<'a, ExtField = Fe1024> {
    name: &'a str,
    generator: &'a [Fe32],
    target: &'a [Fe32],
    bit_len: usize,
    hex_width: usize,
    midstate_repr: &'static str,
    phantom: PhantomData<ExtField>,
}

#[cfg(feature = "alloc")]
impl<'a, ExtField> PrintImpl<'a, ExtField> {
    /// Constructor for an object to print an impl-block for the [`Checksum`] trait.
    ///
    /// # Panics
    ///
    /// Panics if any of the input values fail various sanity checks.
    pub fn new(name: &'a str, generator: &'a [Fe32], target: &'a [Fe32]) -> Self {
        // Sanity checks.
        assert_ne!(name.len(), 0, "type name cannot be the empty string",);
        assert_ne!(
            generator.len(),
            0,
            "generator polynomial cannot be the empty string (constant 1)"
        );
        assert_ne!(target.len(), 0, "target residue cannot be the empty string");
        if generator.len() != target.len() {
            let hint = if generator.len() == target.len() + 1 {
                " (you should not include the monic term of the generator polynomial"
            } else if generator.len() > target.len() {
                " (you may need to zero-pad your target residue)"
            } else {
                ""
            };
            panic!(
                "Generator length {} does not match target residue length {}{}",
                generator.len(),
                target.len(),
                hint
            );
        }

        let bit_len = 5 * target.len();
        let (hex_width, midstate_repr) = if bit_len <= 32 {
            (8, "u32")
        } else if bit_len <= 64 {
            (16, "u64")
        } else if bit_len <= 128 {
            (32, "u128")
        } else {
            panic!("Generator length {} cannot exceed 25, as we cannot represent it by packing bits into a Rust numeric type", generator.len());
        };
        // End sanity checks.
        PrintImpl {
            name,
            generator,
            target,
            bit_len,
            hex_width,
            midstate_repr,
            phantom: PhantomData,
        }
    }
}

#[cfg(feature = "alloc")]
impl<'a, ExtField> fmt::Display for PrintImpl<'a, ExtField>
where
    ExtField: super::ExtensionField + From<Fe32>,
{
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        // Generator polynomial as a polynomial over GF1024
        let gen_poly = {
            let mut v = Vec::with_capacity(self.generator.len() + 1);
            v.push(ExtField::ONE);
            v.extend(self.generator.iter().cloned().map(ExtField::from));
            Polynomial::new(v)
        };
        let (_gen, length, _exponents) = gen_poly.bch_generator_primitive_element();

        write!(f, "// Code block generated by Checksum::print_impl polynomial ")?;
        for fe in self.generator {
            write!(f, "{}", fe)?;
        }
        write!(f, " target ")?;
        for fe in self.target {
            write!(f, "{}", fe)?;
        }
        f.write_str("\n")?;
        writeln!(f, "impl Checksum for {} {{", self.name)?;
        writeln!(
            f,
            "    type MidstateRepr = {}; // checksum packs into {} bits",
            self.midstate_repr, self.bit_len
        )?;
        writeln!(f, "    const CODE_LENGTH: usize = {};", length)?;
        writeln!(f, "    const CHECKSUM_LENGTH: usize = {};", gen_poly.degree())?;
        writeln!(f, "    const GENERATOR_SH: [{}; 5] = [", self.midstate_repr)?;
        let mut gen5 = self.generator.to_vec();
        for _ in 0..5 {
            let gen_packed = u128::pack(gen5.iter().copied().map(From::from));
            writeln!(f, "        0x{:0width$x},", gen_packed, width = self.hex_width)?;
            gen5.iter_mut().for_each(|x| *x *= Fe32::Z);
        }
        writeln!(f, "    ];")?;
        writeln!(
            f,
            "    const TARGET_RESIDUE: {} = {:?};",
            self.midstate_repr,
            u128::pack(self.target.iter().copied().map(From::from))
        )?;
        f.write_str("}")
    }
}

/// A checksum engine, which can be used to compute or verify a checksum.
///
/// Use this to verify a checksum, feed it the data to be checksummed using
/// the `Self::input_*` methods.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct Engine<Ck: Checksum> {
    residue: Ck::MidstateRepr,
}

impl<Ck: Checksum> Default for Engine<Ck> {
    fn default() -> Self { Self::new() }
}

impl<Ck: Checksum> Engine<Ck> {
    /// Constructs a new checksum engine with no data input.
    #[inline]
    pub fn new() -> Self { Engine { residue: Ck::MidstateRepr::ONE } }

    /// Feeds `hrp` into the checksum engine.
    #[inline]
    pub fn input_hrp(&mut self, hrp: Hrp) {
        for fe in HrpFe32Iter::new(&hrp) {
            self.input_fe(fe)
        }
    }

    /// Adds a single gf32 element to the checksum engine.
    ///
    /// This is where the actual checksum computation magic happens.
    #[inline]
    pub fn input_fe(&mut self, e: Fe32) {
        let xn = self.residue.mul_by_x_then_add(Ck::CHECKSUM_LENGTH, e.into());
        for i in 0..5 {
            if xn & (1 << i) != 0 {
                self.residue = self.residue ^ Ck::GENERATOR_SH[i];
            }
        }
    }

    /// Inputs the target residue of the checksum.
    ///
    /// Checksums are generated by appending the target residue to the input
    /// string, then computing the actual residue, and then replacing the
    /// target with the actual. This method lets us compute the actual residue
    /// without doing any string concatenations.
    #[inline]
    pub fn input_target_residue(&mut self) {
        for i in 0..Ck::CHECKSUM_LENGTH {
            self.input_fe(Fe32(Ck::TARGET_RESIDUE.unpack(Ck::CHECKSUM_LENGTH - i - 1)));
        }
    }

    /// Returns for the current checksum residue.
    #[inline]
    pub fn residue(&self) -> &Ck::MidstateRepr { &self.residue }
}

/// Trait describing an integer type which can be used as a "packed" sequence of Fe32s.
///
/// This is implemented for u32, u64 and u128, as a way to treat these primitive types as
/// packed coefficients of polynomials over GF32 (up to some maximal degree, of course).
///
/// This is useful because then multiplication by x reduces to simply left-shifting by 5,
/// and addition of entire polynomials can be done by xor.
pub trait PackedFe32: Copy + PartialEq + Eq + ops::BitXor<Self, Output = Self> {
    /// The one constant, for which stdlib provides no existing trait.
    const ONE: Self;

    /// The number of fe32s that can fit into the type; computed as floor(bitwidth / 5).
    const WIDTH: usize = mem::size_of::<Self>() * 8 / 5;

    /// Takes an iterator of `u8`s (or [`Fe32`]s converted to `u8`s) and packs
    /// them into a [`Self`].
    ///
    /// For sequences representing polynomials, the iterator should yield the
    /// coefficients in little-endian order, i.e. the 0th coefficien first.
    ///
    /// # Panics
    ///
    /// May panic if the iterator yields more items than can fit into the bit-packed
    /// type.
    fn pack<I: Iterator<Item = u8>>(iter: I) -> Self;

    /// Extracts the coefficient of the x^n from the packed polynomial.
    fn unpack(&self, n: usize) -> u8;

    /// Multiply the polynomial by x, drop its highest coefficient (and return it), and
    /// add a new field element to the now-0 constant coefficient.
    ///
    /// Takes the degree of the polynomial as an input; for checksum applications
    /// this should basically always be `Checksum::CHECKSUM_WIDTH`.
    fn mul_by_x_then_add(&mut self, degree: usize, add: u8) -> u8;
}

/// A placeholder type used as part of the [`NoChecksum`] "checksum".
///
/// [`NoChecksum`]: crate::primitives::NoChecksum
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub struct PackedNull;

impl ops::BitXor<PackedNull> for PackedNull {
    type Output = PackedNull;
    #[inline]
    fn bitxor(self, _: PackedNull) -> PackedNull { PackedNull }
}

impl PackedFe32 for PackedNull {
    const ONE: Self = PackedNull;
    #[inline]
    fn unpack(&self, _: usize) -> u8 { 0 }
    #[inline]
    fn mul_by_x_then_add(&mut self, _: usize, _: u8) -> u8 { 0 }

    #[inline]
    fn pack<I: Iterator<Item = u8>>(mut iter: I) -> Self {
        if iter.next().is_some() {
            panic!("Cannot pack anything into a PackedNull");
        }
        Self
    }
}

macro_rules! impl_packed_fe32 {
    ($ty:ident) => {
        impl PackedFe32 for $ty {
            const ONE: Self = 1;

            #[inline]
            fn unpack(&self, n: usize) -> u8 {
                debug_assert!(n < Self::WIDTH);
                (*self >> (n * 5)) as u8 & 0x1f
            }

            #[inline]
            fn mul_by_x_then_add(&mut self, degree: usize, add: u8) -> u8 {
                debug_assert!(degree > 0);
                debug_assert!(degree <= Self::WIDTH);
                debug_assert!(add < 32);
                let ret = self.unpack(degree - 1);
                *self &= !(0x1f << ((degree - 1) * 5));
                *self <<= 5;
                *self |= Self::from(add);
                ret
            }

            #[inline]
            fn pack<I: Iterator<Item = u8>>(iter: I) -> Self {
                let mut ret: Self = 0;
                for (n, elem) in iter.enumerate() {
                    debug_assert!(elem < 32);
                    debug_assert!(n < Self::WIDTH);
                    ret <<= 5;
                    ret |= Self::from(elem);
                }
                ret
            }
        }
    };
}
impl_packed_fe32!(u32);
impl_packed_fe32!(u64);
impl_packed_fe32!(u128);

/// Iterator that yields the field elements that are input into a checksum algorithm for an [`Hrp`].
pub struct HrpFe32Iter<'hrp> {
    /// `None` once the hrp high fes have been yielded.
    high_iter: Option<crate::primitives::hrp::LowercaseByteIter<'hrp>>,
    /// `None` once the hrp low fes have been yielded.
    low_iter: Option<crate::primitives::hrp::LowercaseByteIter<'hrp>>,
}

impl<'hrp> HrpFe32Iter<'hrp> {
    /// Creates an iterator that yields the field elements of `hrp` as they are input into the
    /// checksum algorithm.
    #[inline]
    pub fn new(hrp: &'hrp Hrp) -> Self {
        let high_iter = hrp.lowercase_byte_iter();
        let low_iter = hrp.lowercase_byte_iter();

        Self { high_iter: Some(high_iter), low_iter: Some(low_iter) }
    }
}

impl<'hrp> Iterator for HrpFe32Iter<'hrp> {
    type Item = Fe32;
    #[inline]
    fn next(&mut self) -> Option<Fe32> {
        if let Some(ref mut high_iter) = &mut self.high_iter {
            match high_iter.next() {
                Some(high) => return Some(Fe32(high >> 5)),
                None => {
                    self.high_iter = None;
                    return Some(Fe32::Q);
                }
            }
        }
        if let Some(ref mut low_iter) = &mut self.low_iter {
            match low_iter.next() {
                Some(low) => return Some(Fe32(low & 0x1f)),
                None => self.low_iter = None,
            }
        }
        None
    }

    #[inline]
    fn size_hint(&self) -> (usize, Option<usize>) {
        let high = match &self.high_iter {
            Some(high_iter) => {
                let (min, max) = high_iter.size_hint();
                (min + 1, max.map(|max| max + 1)) // +1 for the extra Q
            }
            None => (0, Some(0)),
        };
        let low = match &self.low_iter {
            Some(low_iter) => low_iter.size_hint(),
            None => (0, Some(0)),
        };

        let min = high.0 + 1 + low.0;
        let max = high.1.zip(low.1).map(|(high, low)| high + 1 + low);

        (min, max)
    }
}

#[cfg(test)]
mod tests {
    #[cfg(feature = "alloc")]
    use core::convert::TryFrom;

    use super::*;

    #[test]
    fn pack_unpack() {
        let packed = u128::pack([0, 0, 0, 1].iter().copied());
        assert_eq!(packed, 1);
        assert_eq!(packed.unpack(0), 1);
        assert_eq!(packed.unpack(3), 0);

        let packed = u128::pack([1, 2, 3, 4].iter().copied());
        assert_eq!(packed, 0b00001_00010_00011_00100);
        assert_eq!(packed.unpack(0), 4);
        assert_eq!(packed.unpack(1), 3);
        assert_eq!(packed.unpack(2), 2);
        assert_eq!(packed.unpack(3), 1);
    }

    #[test]
    #[cfg(feature = "alloc")]
    fn bech32() {
        // In codes that Pieter specifies typically the generator polynomial is
        // only given indirectly, in the reference code which encodes it in a
        // packed form (shifted multiple times).
        //
        // For example in the BIP173 Python reference code you will see an array
        // called `generator` whose first entry is 0x3b6a57b2. This first entry
        // is the generator polynomial in packed form.
        //
        // To get the expanded polynomial form you can use `u128::unpack` like so:
        let unpacked_poly = (0..6)
            .rev() // Note .rev() to convert from BE integer literal to LE polynomial!
            .map(|i| 0x3b6a57b2u128.unpack(i))
            .map(|u| Fe32::try_from(u).unwrap())
            .collect::<Vec<_>>();
        assert_eq!(unpacked_poly, [Fe32::A, Fe32::K, Fe32::_5, Fe32::_4, Fe32::A, Fe32::J],);
        // To get a version of the above with bech32 chars instead of Fe32s, which
        // can be a bit hard to print, just stick a `.map(Fe32::to_char)` into the
        // above iterator chain.

        // Ok, exposition over. The actual unit test follows.

        // Run with -- --nocapture to see the output of this. This unit test
        // does not check the exact output because it is not deterministic,
        // and cannot check the code semantics because Rust does not have
        // any sort of `eval`, but you can manually check the output works.
        let _s = PrintImpl::<Fe1024>::new(
            "Bech32",
            &[Fe32::A, Fe32::K, Fe32::_5, Fe32::_4, Fe32::A, Fe32::J],
            &[Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::P],
        )
        .to_string();
        #[cfg(feature = "std")]
        println!("{}", _s);
    }

    #[test]
    #[cfg(feature = "alloc")]
    fn descriptor() {
        // This magic constant came from Bitcoin Core, src/script/descriptor.cpp.
        //
        // Note that this generator polynomial has degree 8, not 6, reflected
        // in the initial range being (0..8).
        let unpacked_poly = (0..8)
            .rev() // Note .rev() to convert from BE integer literal to LE polynomial!
            .map(|i| 0xf5dee51989u64.unpack(i))
            .map(|u| Fe32::try_from(u).unwrap())
            .collect::<Vec<_>>();
        assert_eq!(
            unpacked_poly,
            [Fe32::_7, Fe32::H, Fe32::_0, Fe32::W, Fe32::_2, Fe32::X, Fe32::V, Fe32::F],
        );

        // Run with -- --nocapture to see the output of this. This unit test
        // does not check the exact output because it is not deterministic,
        // and cannot check the code semantics because Rust does not have
        // any sort of `eval`, but you can manually check the output works.
        let _s = PrintImpl::<crate::Fe32768>::new(
            "DescriptorChecksum",
            &[Fe32::_7, Fe32::H, Fe32::_0, Fe32::W, Fe32::_2, Fe32::X, Fe32::V, Fe32::F],
            &[Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::P],
        )
        .to_string();
        #[cfg(feature = "std")]
        println!("{}", _s);
    }
}

#[cfg(bench)]
mod benches {
    use std::io::{sink, Write};

    use test::{black_box, Bencher};

    use crate::{Fe1024, Fe32, Fe32768, PrintImpl};

    #[bench]
    fn compute_bech32_params(bh: &mut Bencher) {
        bh.iter(|| {
            let im = PrintImpl::<Fe1024>::new(
                "Bech32",
                &[Fe32::A, Fe32::K, Fe32::_5, Fe32::_4, Fe32::A, Fe32::J],
                &[Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::P],
            );
            let res = write!(sink(), "{}", im);
            black_box(&im);
            black_box(&res);
        })
    }

    #[bench]
    fn compute_descriptor_params(bh: &mut Bencher) {
        bh.iter(|| {
            let im = PrintImpl::<Fe32768>::new(
                "DescriptorChecksum",
                &[Fe32::_7, Fe32::H, Fe32::_0, Fe32::W, Fe32::_2, Fe32::X, Fe32::V, Fe32::F],
                &[Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::Q, Fe32::P],
            );
            let res = write!(sink(), "{}", im);
            black_box(&im);
            black_box(&res);
        })
    }
}
