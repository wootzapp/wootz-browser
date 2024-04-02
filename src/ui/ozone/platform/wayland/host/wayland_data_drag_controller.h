// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_DATA_DRAG_CONTROLLER_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_DATA_DRAG_CONTROLLER_H_

#include <list>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#include "base/files/scoped_file.h"
#include "base/gtest_prod_util.h"
#include "base/memory/raw_ptr.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_refptr.h"
#include "base/memory/weak_ptr.h"
#include "base/synchronization/atomic_flag.h"
#include "base/task/task_runner.h"
#include "ui/base/dragdrop/mojom/drag_drop_types.mojom-forward.h"
#include "ui/base/dragdrop/os_exchange_data_provider.h"
#include "ui/events/platform/platform_event_dispatcher.h"
#include "ui/gfx/geometry/point_f.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/ozone/platform/wayland/common/wayland_object.h"
#include "ui/ozone/platform/wayland/host/wayland_data_device.h"
#include "ui/ozone/platform/wayland/host/wayland_data_source.h"
#include "ui/ozone/platform/wayland/host/wayland_exchange_data_provider.h"
#include "ui/ozone/platform/wayland/host/wayland_pointer.h"
#include "ui/ozone/platform/wayland/host/wayland_serial_tracker.h"
#include "ui/ozone/platform/wayland/host/wayland_touch.h"
#include "ui/ozone/platform/wayland/host/wayland_window_observer.h"

class SkBitmap;

namespace base {
class TimeTicks;
}

namespace ui {

class OSExchangeData;
class ScopedEventDispatcher;
class WaylandConnection;
class WaylandDataDeviceManager;
class WaylandDataOffer;
class WaylandWindow;
class WaylandWindowManager;
class WaylandShmBuffer;
class WaylandSurface;

// WaylandDataDragController implements regular mouse/touch-driven data exchange
// on top of the Wayland Drag-and-Drop protocol. Data can be dragged within
// Chromium windows, or between Chromium and other applications in both
// directions.
//
// Outgoing drag sessions start via the StartSession() method. For more context,
// see WaylandTopLevelWindow::StartDrag().
//
// Incoming drag sessions start with calls to OnDragOffer/OnDragEnter() from the
// Wayland side (the data device), and end up in calls to WaylandWindow's
// OnDragEnter() and OnDragDataAvailable(), but two ways of getting there are
// possible:
//
// 1. The drag has been initiated from a Chromium window. In this case, the data
// that is being dragged is available right away, and therefore the controller
// can forward the data to the window immediately.
//
// 2. The data is being dragged from another application. In this case, the
// window is notified right away about the enter event and a data fetching task
// is posted to the thread pool. Once fully fetched, the data is delivered to
// the entered window. If the drag cursor leaves the window or the entered
// windows gets destroyed while the data is still being fetched, the fetching
// task is cancelled and the whole drag session is aborted.
class WaylandDataDragController : public WaylandDataDevice::DragDelegate,
                                  public WaylandDataSource::Delegate,
                                  public WaylandWindowObserver,
                                  public PlatformEventDispatcher {
 public:
  enum class State {
    kIdle,      // Doing nothing special
    kStarted,   // The outgoing drag is in progress.
    kFetching,  // The incoming data is fetched from the source.
  };

  WaylandDataDragController(WaylandConnection* connection,
                            WaylandDataDeviceManager* data_device_manager,
                            WaylandPointer::Delegate* pointer_delegate,
                            WaylandTouch::Delegate* touch_delegate);
  WaylandDataDragController(const WaylandDataDragController&) = delete;
  WaylandDataDragController& operator=(const WaylandDataDragController&) =
      delete;
  ~WaylandDataDragController() override;

  // Starts a data drag and drop session for |data|. Returns true if it is
  // successfully started, false otherwise. Only one DND session can run at a
  // given time.
  bool StartSession(const ui::OSExchangeData& data,
                    int operations,
                    mojom::DragEventSource source);

  // Updates the drag image. An empty |image| may be used to hide a previously
  // set non-empty drag image, and a non-empty |image| shows the drag image
  // again if it was previously hidden.
  //
  // This must be called during an active drag session.
  void UpdateDragImage(const gfx::ImageSkia& image,
                       const gfx::Vector2d& offset);

  State state() const { return state_; }

  // TODO(crbug.com/896640): Remove once focus is fixed during DND sessions.
  WaylandWindow* entered_window() const { return window_; }

  // Returns false iff the data is for a window dragging session.
  bool ShouldReleaseCaptureForDrag(ui::OSExchangeData* data) const;

  void DumpState(std::ostream& out) const;

 private:
  // Cancellation flags are ref-counted to ensure they remain valid even if the
  // controller has already cancelled and released its instance so that it is
  // able to track only the current fetching task, on which it's interested in.
  using CancelFlag = base::RefCountedData<base::AtomicFlag>;

  FRIEND_TEST_ALL_PREFIXES(WaylandDataDragControllerTest, ReceiveDrag);
  FRIEND_TEST_ALL_PREFIXES(WaylandDataDragControllerTest, StartDrag);
  FRIEND_TEST_ALL_PREFIXES(WaylandDataDragControllerTest, StartDragWithText);
  FRIEND_TEST_ALL_PREFIXES(WaylandDataDragControllerTest, AsyncNoopStartDrag);
  FRIEND_TEST_ALL_PREFIXES(WaylandDataDragControllerTest,
                           StartDragWithWrongMimeType);
  FRIEND_TEST_ALL_PREFIXES(WaylandDataDragControllerTest,
                           ForeignDragHandleAskAction);

  // WaylandDataDevice::DragDelegate:
  bool IsDragSource() const override;
  void DrawIcon() override;
  void OnDragOffer(std::unique_ptr<WaylandDataOffer> offer) override;
  void OnDragEnter(WaylandWindow* window,
                   const gfx::PointF& location,
                   base::TimeTicks timestamp,
                   uint32_t serial) override;
  void OnDragMotion(const gfx::PointF& location,
                    base::TimeTicks timestamp) override;
  void OnDragLeave(base::TimeTicks timestamp) override;
  void OnDragDrop(base::TimeTicks timestamp) override;
  const WaylandWindow* GetDragTarget() const override;

  // WaylandDataSource::Delegate:
  void OnDataSourceFinish(WaylandDataSource* source,
                          base::TimeTicks timestamp,
                          bool completed) override;
  void OnDataSourceSend(WaylandDataSource* source,
                        const std::string& mime_type,
                        std::string* contents) override;

  // WaylandWindowObserver:
  void OnWindowRemoved(WaylandWindow* window) override;

  // Starts the process of fetching data offered by an external client (ie:
  // incoming drag session). The actual I/O is performed in a separate thread
  // using ThreadPool infra. Once data for all supported mime types is fetched,
  // the OnDataFetchingFinished callback is fired.
  void PostDataFetchingTask(const gfx::PointF& location,
                            base::TimeTicks start_time,
                            const scoped_refptr<CancelFlag>& cancel_flag);

  void OnDataFetchingFinished(
      base::TimeTicks start_time,
      std::unique_ptr<ui::OSExchangeData> received_data);
  void CancelDataFetchingIfNeeded();

  std::optional<wl::Serial> GetAndValidateSerialForDrag(
      mojom::DragEventSource source);

  void SetOfferedExchangeDataProvider(const OSExchangeData& data);
  const WaylandExchangeDataProvider* GetOfferedExchangeDataProvider() const;

  // Checks whether |data| holds information about a window dragging session.
  bool IsWindowDraggingSession(const ui::OSExchangeData& data) const;

  // Sets up everything for starting a window dragging session using regular
  // drag and drop, if |data| holds information about a window dragging session
  // (as reported by IsWindowDraggingSession()). |origin_window_| must be set
  // before calling this.
  void SetUpWindowDraggingSessionIfNeeded(const ui::OSExchangeData& data);

  // Sends an ET_MOUSE_RELEASED event to the window that currently has capture.
  // Must only be called if |pointer_grabber_for_window_drag_| is valid. This
  // resets |pointer_grabber_for_window_drag_|.
  void DispatchPointerRelease(base::TimeTicks timestamp);

  // PlatformEventDispatcher:
  bool CanDispatchEvent(const PlatformEvent& event) override;
  uint32_t DispatchEvent(const PlatformEvent& event) override;

  SkBitmap GetIconBitmap();
  void DrawIconInternal();
  static void OnDragSurfaceFrame(void* data,
                                 struct wl_callback* callback,
                                 uint32_t time);

  // Returns the task runner instance used to run data fetching tasks in
  // incoming drag sessions.
  base::TaskRunner& GetDataFetchTaskRunner();

  const raw_ptr<WaylandConnection> connection_;
  const raw_ptr<WaylandDataDeviceManager> data_device_manager_;
  const raw_ptr<WaylandDataDevice> data_device_;
  const raw_ptr<WaylandWindowManager> window_manager_;
  const raw_ptr<WaylandPointer::Delegate> pointer_delegate_;
  const raw_ptr<WaylandTouch::Delegate> touch_delegate_;

  State state_ = State::kIdle;
  std::optional<mojom::DragEventSource> drag_source_;

  // Data offered by us to the other side.
  std::unique_ptr<WaylandDataSource> data_source_;

  // When dragging is started from Chromium, |offered_exchange_data_provider_|
  // holds the provider for the data to be sent through Wayland protocol.
  std::unique_ptr<OSExchangeDataProvider> offered_exchange_data_provider_;

  // Offer to receive data from another process via drag-and-drop, or null if
  // no drag-and-drop from another process is in progress.
  //
  // The data offer from another Wayland client through wl_data_device, that
  // triggered the current drag and drop session. If null, either there is no
  // dnd session running or Chromium is the data source.
  std::unique_ptr<WaylandDataOffer> data_offer_;

  // The window that initiated the drag session. Can be null when the session
  // has been started by an external Wayland client.
  raw_ptr<WaylandWindow> origin_window_ = nullptr;

  // Current window under pointer.
  raw_ptr<WaylandWindow, DanglingUntriaged> window_ = nullptr;

  // The most recent location received while dragging the data.
  gfx::PointF last_drag_location_;

  // Drag icon related variables.
  std::unique_ptr<WaylandSurface> icon_surface_;
  float icon_surface_buffer_scale_ = 1.0f;
  std::unique_ptr<WaylandShmBuffer> icon_buffer_;
  gfx::ImageSkia icon_image_;
  // pending_icon_offset_ is the offset from the image to the cursor to be
  // applied on the next DrawIconInternal().
  // current_icon_offset_ holds the actual current offset from the drag image
  // to the cursor. It's used for calculating the right values to be used with
  // wl_surface.offset to ensure the offset specified in pending_icon_offset_
  // becomes the actual offset from the image to the cursor.
  gfx::Point pending_icon_offset_;
  gfx::Point current_icon_offset_;
  wl::Object<wl_callback> icon_frame_callback_;

  // Keeps track of the window that holds the pointer grab, i.e. the window that
  // will receive the mouse release event from DispatchPointerRelease().
  raw_ptr<WaylandWindow> pointer_grabber_for_window_drag_ = nullptr;

  std::unique_ptr<ScopedEventDispatcher> nested_dispatcher_;

  // Flag used to notify the data fetcher task, which runs on thread pool, that
  // it should abort the operation. i.e: used only in incoming dnd sessions.
  scoped_refptr<CancelFlag> data_fetch_cancel_flag_;

  // Sequenced task runner used to post fetch tasks to.
  scoped_refptr<base::TaskRunner> data_fetch_task_runner_;

  base::WeakPtrFactory<WaylandDataDragController> weak_factory_{this};
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_WAYLAND_DATA_DRAG_CONTROLLER_H_
