
use base::Value;


#[derive(Debug, PartialEq)]
pub struct SolanaAccountMeta {
    pub pubkey: String,
    pub address_table_lookup_index: Option<u8>,
    pub is_signer: bool,
    pub is_writable: bool,
}


impl PartialEq for SolanaAccountMeta {
    fn eq(&self, other: &Self) -> bool {
        self.pubkey == other.pubkey &&
        self.address_table_lookup_index == other.address_table_lookup_index &&
        self.is_signer == other.is_signer &&
        self.is_writable == other.is_writable
    }
}

impl SolanaAccountMeta {
    pub fn from_cpp(account_meta: &solana_wallet::SolanaAccountMeta) -> Self {
        Self {
            pubkey: account_meta.pubkey.clone(),
            address_table_lookup_index: account_meta.address_table_lookup_index,
            is_signer: account_meta.is_signer,
            is_writable: account_meta.is_writable,
        }
    }

    pub fn to_cpp(&self) -> solana_wallet::SolanaAccountMeta {
        solana_wallet::SolanaAccountMeta::ne(
            self.pubkey.clone(),
            self.address_table_lookup_index,
            self.is_signer,
            self.is_writable,
        )
    }

    pub fn from_value(value: &Value) -> Option<Self> {
        if let Value::Dict(dict) = value {
            let pubkey = dict.get("pubkey")?.as_str()?.to_string();
            let is_signer = dict.get("is_signer")?.as_bool()?;
            let is_writable = dict.get("is_writable")?.as_bool()?;
            let address_table_lookup_index = match dict.get("address_table_lookup_index") {
                Some(val) => Some(val.as_u64()? as u8),
                None => None,
            };

            Some(Self {
                pubkey,
                address_table_lookup_index,
                is_signer,
                is_writable,
            })
        } else {
            None
        }
    }

    pub fn to_value(&self) -> Value {
        let mut dict = Value::dict();
        dict.insert("pubkey".to_string(), Value::string(self.pubkey.clone()));
        dict.insert("is_signer".to_string(), Value::bool(self.is_signer));
        dict.insert("is_writable".to_string(), Value::bool(self.is_writable));
        if let Some(index) = self.address_table_lookup_index {
            dict.insert("address_table_lookup_index".to_string(), Value::uint(index as u64));
        }
        Value::Dict(dict)
    }
}
