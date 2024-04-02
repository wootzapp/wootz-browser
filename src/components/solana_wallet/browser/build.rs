fn main() {
    autocxx_build::Builder::new("src/solana_wallet.h", &[&"path/to/your/include"])
        .flag_if_supported("-std=c++14")
        .compile("autocxx-solana_wallet");
    println!("cargo:rerun-if-changed=src/solana_wallet.h");
}
