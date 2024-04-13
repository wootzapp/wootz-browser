

#include <string>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/unguessable_token.h"

#include "content/public/browser/storage_partition.h"

#include "services/network/public/mojom/network_context.mojom.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"

class Profile;

class ThrottlingPageHandler
    : public Throttling::mojom::PageHandler,
      public network::NetworkContext {
 public:
  ThrottlingPageHandler(
      mojo::PendingReceiver<throttling::mojom::PageHandler>
          receiver,
      Profile* profile);
  ~ThrottlingPageHandler() override;

  ThrottlingPageHandler(
      const ThrottlingPageHandler&) = delete;
  ThrottlingPageHandler& operator=(
      const ThrottlingPageHandler&) = delete;

  // throttling::mojom::PageHandler
  void SetCustomNetworkConditions(network::mojom::NetworkConditionsPtr conditions) override;
  
 private:
  const base::UnguessableToken devtools_token_;
  raw_ptr<StoragePartition> storage_partition_;
  mojo::Receiver<Throttling::mojom::PageHandler> receiver_;
  mojo::Remote<throttling::mojom::Page> page_;
  base::WeakPtrFactory<ThrottlingPageHandler> weak_ptr_factory_{
      this};
};

