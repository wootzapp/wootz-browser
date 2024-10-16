# Changelog
All notable changes to this library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this library adheres to Rust's notion of
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- `zcash_address::ZcashAddress::{can_receive_memo, can_receive_as, matches_receiver}`
- `zcash_address::unified::Address::{can_receive_memo, has_receiver_of_type, contains_receiver}`
- Module `zcash_address::testing` under the `test-dependencies` feature.
- Module `zcash_address::unified::address::testing` under the 
  `test-dependencies` feature.

## [0.3.2] - 2024-03-06
### Added
- `zcash_address::convert`:
  - `TryFromRawAddress::try_from_raw_tex`
  - `TryFromAddress::try_from_tex`
  - `ToAddress::from_tex`

## [0.3.1] - 2024-01-12
### Fixed
- Stubs for `zcash_address::convert` traits that are created by `rust-analyzer`
  and similar LSPs no longer reference crate-private type aliases.

## [0.3.0] - 2023-06-06
### Changed
- Bumped bs58 dependency to `0.5`.

## [0.2.1] - 2023-04-15
### Changed
- Bumped internal dependency to `bech32 0.9`.

## [0.2.0] - 2022-10-19
### Added
- `zcash_address::ConversionError`
- `zcash_address::TryFromAddress`
- `zcash_address::TryFromRawAddress`
- `zcash_address::ZcashAddress::convert_if_network`
- A `TryFrom<Typecode>` implementation for `usize`.

### Changed
- MSRV is now 1.52

### Removed
- `zcash_address::FromAddress` (use `TryFromAddress` instead).

## [0.1.0] - 2022-05-11
Initial release.
