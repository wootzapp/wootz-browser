#[cxx::bridge]
mod ffi {

    extern "Rust"{
       fn from_cpp(account_meta: &solana_wallet::SolanaAccountMeta) -> SolanaAccountMeta;
       fn to_cpp(account_meta: &SolanaAccountMeta) -> solana_wallet::SolanaAccountMeta;
       
       fn from_value(value: &Value) -> Option<SolanaAccountMeta>;
       fn to_value(account_meta: &SolanaAccountMeta) -> Value;
    }

    unsafe extern "C++"{
        include!("components/solana_wallet/browser/src/solana_account_meta.h");

        type SolanaAccountMeta;

        fn SolanaAccountMeta::ToMojomSolanaAccountMeta()->;
    }
}