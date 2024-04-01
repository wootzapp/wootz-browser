#include "components/solana_wallet/browser/include/solana_account_meta.h"

namespace solana_wallet {

mojom::SolanaAccountMetaPtr SolanaAccountMeta::ToMojomSolanaAccountMeta()
    const {
  mojom::OptionalUint8Ptr index = nullptr;
  if (address_table_lookup_index) {
    index = mojom::OptionalUint8::New(*address_table_lookup_index);
  }
  return mojom::SolanaAccountMeta::New(pubkey, std::move(index), is_signer,
                                       is_writable);
}

// static
void SolanaAccountMeta::FromMojomSolanaAccountMetas(
    const std::vector<mojom::SolanaAccountMetaPtr>& mojom_account_metas,
    std::vector<SolanaAccountMeta>* account_metas) {
  if (!account_metas) {
    return;
  }
  account_metas->clear();
  for (const auto& mojom_account_meta : mojom_account_metas) {
    std::optional<uint8_t> addr_table_lookup_index = std::nullopt;
    if (mojom_account_meta->addr_table_lookup_index) {
      addr_table_lookup_index =
          mojom_account_meta->addr_table_lookup_index->val;
    }
    account_metas->push_back(SolanaAccountMeta(
        mojom_account_meta->pubkey, addr_table_lookup_index,
        mojom_account_meta->is_signer, mojom_account_meta->is_writable));
  }
}

}  // namespace solana_wallet
