// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "content/common/input/render_input_router.h"

#include <utility>
#include <vector>

#include "base/check.h"
#include "base/command_line.h"
#include "base/debug/stack_trace.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/memory/ptr_util.h"
#include "content/common/input/input_router_config_helper.h"
#include "mojo/public/cpp/bindings/callback_helpers.h"
#include "mojo/public/cpp/bindings/pending_receiver.h"
#include "mojo/public/cpp/bindings/pending_remote.h"

namespace {

class UnboundWidgetInputHandler : public blink::mojom::WidgetInputHandler {
 public:
  void SetFocus(blink::mojom::FocusState focus_state) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void MouseCaptureLost() override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void SetEditCommandsForNextKeyEvent(
      std::vector<blink::mojom::EditCommandPtr> commands) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void CursorVisibilityChanged(bool visible) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void ImeSetComposition(const std::u16string& text,
                         const std::vector<ui::ImeTextSpan>& ime_text_spans,
                         const gfx::Range& range,
                         int32_t start,
                         int32_t end,
                         ImeSetCompositionCallback callback) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void ImeCommitText(const std::u16string& text,
                     const std::vector<ui::ImeTextSpan>& ime_text_spans,
                     const gfx::Range& range,
                     int32_t relative_cursor_position,
                     ImeCommitTextCallback callback) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void ImeFinishComposingText(bool keep_selection) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void RequestTextInputStateUpdate() override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void RequestCompositionUpdates(bool immediate_request,
                                 bool monitor_request) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void DispatchEvent(std::unique_ptr<blink::WebCoalescedInputEvent> event,
                     DispatchEventCallback callback) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void DispatchNonBlockingEvent(
      std::unique_ptr<blink::WebCoalescedInputEvent> event) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
  void WaitForInputProcessed(WaitForInputProcessedCallback callback) override {
    DLOG(WARNING) << "Input request on unbound interface";
  }
#if BUILDFLAG(IS_ANDROID)
  void AttachSynchronousCompositor(
      mojo::PendingRemote<blink::mojom::SynchronousCompositorControlHost>
          control_host,
      mojo::PendingAssociatedRemote<blink::mojom::SynchronousCompositorHost>
          host,
      mojo::PendingAssociatedReceiver<blink::mojom::SynchronousCompositor>
          compositor_request) override {
    NOTREACHED() << "Input request on unbound interface";
  }
#endif
  void GetFrameWidgetInputHandler(
      mojo::PendingAssociatedReceiver<blink::mojom::FrameWidgetInputHandler>
          request) override {
    NOTREACHED() << "Input request on unbound interface";
  }
  void UpdateBrowserControlsState(cc::BrowserControlsState constraints,
                                  cc::BrowserControlsState current,
                                  bool animate) override {
    NOTREACHED() << "Input request on unbound interface";
  }
};

base::LazyInstance<UnboundWidgetInputHandler>::Leaky g_unbound_input_handler =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace content {

RenderInputRouter::~RenderInputRouter() {}

RenderInputRouter::RenderInputRouter(
    InputRouterImplClient* host,
    InputDispositionHandler* handler,
    std::unique_ptr<FlingSchedulerBase> fling_scheduler,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner)
    : fling_scheduler_(std::move(fling_scheduler)),
      input_router_impl_client_(host),
      input_disposition_handler_(handler),
      task_runner_(std::move(task_runner)) {}

void RenderInputRouter::SetupInputRouter(float device_scale_factor) {
  TRACE_EVENT("toplevel.flow", "RenderInputRouter::SetupInputRouter");

  input_router_ = std::make_unique<InputRouterImpl>(
      this, input_disposition_handler_, fling_scheduler_.get(),
      GetInputRouterConfigForPlatform(task_runner_));

  // input_router_ recreated, need to update the force_enable_zoom_ state.
  input_router_->SetForceEnableZoom(force_enable_zoom_);
  input_router_->SetDeviceScaleFactor(device_scale_factor);
}

void RenderInputRouter::BindRenderInputRouterInterfaces(
    mojo::PendingRemote<blink::mojom::RenderInputRouterClient> remote) {
  client_remote_.reset();

  client_remote_.Bind(std::move(remote), task_runner_);
}

void RenderInputRouter::RendererWidgetCreated(bool for_frame_widget) {
  TRACE_EVENT("toplevel.flow", "RenderInputRouter::RendererWidgetCreated");

  client_remote_->GetWidgetInputHandler(
      widget_input_handler_.BindNewPipeAndPassReceiver(task_runner_),
      input_router_->BindNewHost(task_runner_));

  if (for_frame_widget) {
    widget_input_handler_->GetFrameWidgetInputHandler(
        frame_widget_input_handler_.BindNewEndpointAndPassReceiver(
            task_runner_));
  }
}

void RenderInputRouter::SetForceEnableZoom(bool enabled) {
  force_enable_zoom_ = enabled;
  input_router_->SetForceEnableZoom(enabled);
}

void RenderInputRouter::SetDeviceScaleFactor(float device_scale_factor) {
  input_router_->SetDeviceScaleFactor(device_scale_factor);
}

void RenderInputRouter::ProgressFlingIfNeeded(base::TimeTicks current_time) {
  TRACE_EVENT("toplevel.flow", "RenderInputRouter::ProgressFlingIfNeeded");
  fling_scheduler_->ProgressFlingOnBeginFrameIfneeded(current_time);
}

blink::mojom::WidgetInputHandler* RenderInputRouter::GetWidgetInputHandler() {
  TRACE_EVENT("toplevel.flow", "RenderInputRouter::GetWidgetInputHandler");

  if (widget_input_handler_) {
    return widget_input_handler_.get();
  }
  // TODO(dtapuska): Remove the need for the unbound interface. It is
  // possible that a RVHI may make calls to a WidgetInputHandler when
  // the main frame is remote. This is because of ordering issues during
  // widget shutdown, so we present an UnboundWidgetInputHandler had
  // DLOGS the message calls.
  return g_unbound_input_handler.Pointer();
}

void RenderInputRouter::OnImeCompositionRangeChanged(
    const gfx::Range& range,
    const std::optional<std::vector<gfx::Rect>>& character_bounds,
    const std::optional<std::vector<gfx::Rect>>& line_bounds) {
  input_router_impl_client_->OnImeCompositionRangeChanged(
      range, character_bounds, line_bounds);
}
void RenderInputRouter::OnImeCancelComposition() {
  input_router_impl_client_->OnImeCancelComposition();
}

StylusInterface* RenderInputRouter::GetStylusInterface() {
  return input_router_impl_client_->GetStylusInterface();
}

void RenderInputRouter::OnStartStylusWriting() {
  input_router_impl_client_->OnStartStylusWriting();
}

bool RenderInputRouter::IsWheelScrollInProgress() {
  return input_router_impl_client_->IsWheelScrollInProgress();
}

bool RenderInputRouter::IsAutoscrollInProgress() {
  return input_router_impl_client_->IsAutoscrollInProgress();
}

void RenderInputRouter::SetMouseCapture(bool capture) {
  input_router_impl_client_->SetMouseCapture(capture);
}

void RenderInputRouter::SetAutoscrollSelectionActiveInMainFrame(
    bool autoscroll_selection) {
  input_router_impl_client_->SetAutoscrollSelectionActiveInMainFrame(
      autoscroll_selection);
}

void RenderInputRouter::RequestMouseLock(
    bool from_user_gesture,
    bool unadjusted_movement,
    InputRouterImpl::RequestMouseLockCallback response) {
  input_router_impl_client_->RequestMouseLock(
      from_user_gesture, unadjusted_movement, std::move(response));
}

gfx::Size RenderInputRouter::GetRootWidgetViewportSize() {
  return input_router_impl_client_->GetRootWidgetViewportSize();
}

blink::mojom::InputEventResultState RenderInputRouter::FilterInputEvent(
    const blink::WebInputEvent& event,
    const ui::LatencyInfo& latency_info) {
  return input_router_impl_client_->FilterInputEvent(event, latency_info);
}

void RenderInputRouter::IncrementInFlightEventCount() {
  input_router_impl_client_->IncrementInFlightEventCount();
}

void RenderInputRouter::NotifyUISchedulerOfGestureEventUpdate(
    blink::WebInputEvent::Type gesture_event) {
  input_router_impl_client_->NotifyUISchedulerOfGestureEventUpdate(
      gesture_event);
}

void RenderInputRouter::DecrementInFlightEventCount(
    blink::mojom::InputEventResultSource ack_source) {
  input_router_impl_client_->DecrementInFlightEventCount(ack_source);
}

void RenderInputRouter::DidOverscroll(const ui::DidOverscrollParams& params) {
  input_router_impl_client_->DidOverscroll(params);
}

void RenderInputRouter::DidStartScrollingViewport() {
  input_router_impl_client_->DidStartScrollingViewport();
}

void RenderInputRouter::OnInvalidInputEventSource() {
  input_router_impl_client_->OnInvalidInputEventSource();
}

void RenderInputRouter::ForwardGestureEventWithLatencyInfo(
    const blink::WebGestureEvent& gesture_event,
    const ui::LatencyInfo& latency_info) {
  input_router_impl_client_->ForwardGestureEventWithLatencyInfo(gesture_event,
                                                                latency_info);
}
void RenderInputRouter::ForwardWheelEventWithLatencyInfo(
    const blink::WebMouseWheelEvent& wheel_event,
    const ui::LatencyInfo& latency_info) {
  input_router_impl_client_->ForwardWheelEventWithLatencyInfo(wheel_event,
                                                              latency_info);
}

blink::mojom::FrameWidgetInputHandler*
RenderInputRouter::GetFrameWidgetInputHandler() {
  if (!frame_widget_input_handler_) {
    return nullptr;
  }
  return frame_widget_input_handler_.get();
}

}  // namespace content
