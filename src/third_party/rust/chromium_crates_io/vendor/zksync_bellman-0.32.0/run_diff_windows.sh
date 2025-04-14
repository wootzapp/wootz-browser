#!/bin/sh

cargo test --release -- --ignored --nocapture test_large_data_different_windows_multiexp
