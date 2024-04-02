#include "chrome/browser/ui/webui/donuts/donuts.mojom.h"
#include "mojo/public/cpp/bindings/receiver.h"
#include "mojo/public/cpp/bindings/remote.h"

class DonutsPageHandler : public donuts::mojom::PageHandler {
 public:
  DonutsPageHandler(mojo::PendingReceiver<donuts::mojom::PageHandler> receiver,
                    mojo::PendingRemote<donuts::mojom::Page> page);

  DonutsPageHandler(const DonutsPageHandler&) = delete;
  DonutsPageHandler& operator=(const DonutsPageHandler&) = delete;

  ~DonutsPageHandler() override;

 private:
  mojo::Receiver<donuts::mojom::PageHandler> receiver_;
  mojo::Remote<donuts::mojom::Page> page_;

  // Triggered by some outside event
  void OnBakingDonutsFinished(uint32_t num_donuts);

  // donuts::mojom::PageHandler:
  void StartPilotLight() override;
  void BakeDonuts(uint32_t num_donuts) override;
  void GetNumberOfDonuts(GetNumberOfDonutsCallback callback) override;
};
