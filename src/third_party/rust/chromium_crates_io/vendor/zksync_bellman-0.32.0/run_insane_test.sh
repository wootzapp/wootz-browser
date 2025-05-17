#!/bin/sh

# cargo test --release -- --ignored --nocapture test_multiexp_performance_on_large_data
RUSTFLAGS="-C target-cpu=native -C target_feature=+bmi2,+adx,+sse4.1" cargo +nightly test --release --features "asm" -- --ignored --nocapture test_large_data_different_multiexps
