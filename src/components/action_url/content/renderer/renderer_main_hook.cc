// REMOVED: Duplicate instantiation was causing crashes
// SensitiveElementMaskAgent is now only created in chrome_content_renderer_client.cc
// to avoid double instantiation and memory corruption

// #include "components/action_url/content/renderer/sensitive_element_mask_agent.h"

// In the function/hook where RenderFrameObservers are created for each frame:

// Example (pseudo):
// void SomeRendererFrameInitFunction(content::RenderFrame* render_frame) {
//   ...
//    new sensitive_masking::SensitiveElementMaskAgent(render_frame);  // REMOVED - duplicate!
//   ...
// } 