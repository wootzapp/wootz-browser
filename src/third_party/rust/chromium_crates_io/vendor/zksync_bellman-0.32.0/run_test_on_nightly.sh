#!/bin/sh

# RUSTFLAGS="-C target-cpu=native -C target_feature=+bmi2,+adx,+sse4.1" cargo +nightly test --release -- --nocapture test_valid_bn254_multiexp
# RUSTFLAGS="-C target-cpu=native -C target_feature=+bmi2,+adx,+sse4.1" cargo +nightly test --release -- --nocapture test_dense_multiexp_vs_new_multiexp
RUSTFLAGS="-C target-cpu=native -C target_feature=+bmi2,+adx,+sse4.1" cargo +nightly test --release --features "asm" -- --nocapture test_valid_bn254_multiexp
RUSTFLAGS="-C target-cpu=native -C target_feature=+bmi2,+adx,+sse4.1" cargo +nightly test --release --features "asm" -- --nocapture test_dense_multiexp_vs_new_multiexp
