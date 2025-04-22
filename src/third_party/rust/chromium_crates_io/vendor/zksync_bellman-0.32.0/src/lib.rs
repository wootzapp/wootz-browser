#![allow(dead_code, unused_imports, unused_mut, unused_variables, unused_macros, unused_assignments, unreachable_patterns)]
#![cfg_attr(feature = "allocator", feature(allocator_api))]
#![allow(clippy::needless_borrow, clippy::needless_borrows_for_generic_args)]

#[macro_use]
extern crate cfg_if;
extern crate bit_vec;
extern crate byteorder;
pub extern crate pairing;
extern crate rand;

pub use pairing::*;
pub use smallvec;

use crate::pairing::ff;
pub use ff::*;

#[macro_use]
mod log;

pub mod domain;
pub mod groth16;

#[cfg(feature = "gm17")]
pub mod gm17;

#[cfg(feature = "sonic")]
pub mod sonic;

#[cfg(feature = "plonk")]
pub mod plonk;

#[macro_use]
#[cfg(feature = "plonk")]
extern crate lazy_static;

#[cfg(feature = "marlin")]
pub mod marlin;

#[cfg(any(feature = "marlin", feature = "plonk"))]
pub mod kate_commitment;

pub mod constants;
mod group;
mod multiexp;
mod prefetch;
mod source;

#[cfg(test)]
mod tests;

cfg_if! {
    if #[cfg(feature = "multicore")] {
        #[cfg(feature = "wasm")]
        compile_error!("Multicore feature is not yet compatible with wasm target arch");

        mod multicore;
        pub mod worker {
            pub use super::multicore::*;
        }
    } else {
        mod singlecore;
        pub mod worker {
            pub use super::singlecore::*;
        }
    }
}

mod cs;
pub use self::cs::*;

use std::env;
use std::str::FromStr;

cfg_if! {
    if #[cfg(any(not(feature = "nolog"), feature = "sonic"))] {
        fn verbose_flag() -> bool {
            option_env!("BELLMAN_VERBOSE").unwrap_or("0") == "1"
        }
    }
}
