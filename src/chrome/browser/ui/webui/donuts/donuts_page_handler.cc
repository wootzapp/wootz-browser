#include "chrome/browser/ui/webui/donuts/donuts_page_handler.h"
#include "chrome/browser/ui/webui/donuts/donuts.mojom.h"

namespace {
bool pilot_light_on = false;      // Tracks the state of the pilot light
uint32_t total_donuts_baked = 0;  // Tracks the total number of donuts baked
}  // namespace

DonutsPageHandler::DonutsPageHandler(
    mojo::PendingReceiver<donuts::mojom::PageHandler> receiver,
    mojo::PendingRemote<donuts::mojom::Page> page)
    : receiver_(this, std::move(receiver)), page_(std::move(page)) {}

DonutsPageHandler::~DonutsPageHandler() {
  pilot_light_on = false;
}

// Triggered by outside asynchronous event; sends information to the renderer.
void DonutsPageHandler::OnBakingDonutsFinished(uint32_t num_donuts) {
  page_->DonutsBaked(num_donuts);
}

void DonutsPageHandler::StartPilotLight() {
  // Start the pilot light (turn it on).
  pilot_light_on = true;
  // In a real application, additional logic might be necessary here.
}

void DonutsPageHandler::BakeDonuts(uint32_t num_donuts) {
  if (!pilot_light_on) {
    // Can't bake donuts without the pilot light!
    return;  // Or handle the error in a way that makes sense for your
             // application.
  }
  // Simulate baking donuts. In a real application, you might start a timer or
  // trigger a background task here.
  // For the sake of demonstration, we'll assume the donuts are instantly baked.
  OnBakingDonutsFinished(num_donuts);
}

// Triggered by getNumberOfDonuts() call in TS; sends a response back to the
// renderer.
void DonutsPageHandler::GetNumberOfDonuts(GetNumberOfDonutsCallback callback) {
  std::move(callback).Run(total_donuts_baked);
}
