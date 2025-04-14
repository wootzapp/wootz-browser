pub use super::proth::Fr;

use super::impl_macro::*;

use super::TransparentEngine;

transparent_engine_impl! {Transparent252, Fr}

impl TransparentEngine for Transparent252 {}
