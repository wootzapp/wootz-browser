#include <optional>
#include <string>
#include <vector>

#include "base/values.h"
#include "components/solana_wallet/common/solana_wallet.mojom.h"

namespace solana_wallet {

struct SolanaAccountMeta {
  SolanaAccountMeta(const std::string& pubkey,
                    std::optional<uint8_t> address_table_lookup_index,
                    bool is_signer,
                    bool is_writable);
  ~SolanaAccountMeta();

  SolanaAccountMeta(const SolanaAccountMeta&);
  bool operator==(const SolanaAccountMeta&) const;

  mojom::SolanaAccountMetaPtr ToMojomSolanaAccountMeta() const;
  base::Value::Dict ToValue() const;

  static void FromMojomSolanaAccountMetas(
      const std::vector<mojom::SolanaAccountMetaPtr>& mojom_account_metas,
      std::vector<SolanaAccountMeta>* account_metas);
  static std::optional<SolanaAccountMeta> FromValue(
      const base::Value::Dict& value);

  std::string pubkey;
  std::optional<uint8_t> address_table_lookup_index;
  bool is_signer;
  bool is_writable;
};

}  // namespace solana_wallet

#endif  //SOLANA_WALLET_BROWSER_SOLANA_ACCOUNT_META_H_