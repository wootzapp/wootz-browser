#!/bin/sh

RUSTFLAGS="-C target-cpu=native -C target_feature=+bmi2,+adx,+sse4.1" cargo +nightly test --release --features "asm" -- --ignored --nocapture test_insane_size_lde