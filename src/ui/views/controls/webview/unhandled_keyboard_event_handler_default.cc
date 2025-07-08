// Copyright 2013 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/input/native_web_keyboard_event.h"
#include "ui/events/event.h"
#include "build/buildflag.h"
#include "ui/views/controls/webview/unhandled_keyboard_event_handler.h"
#include "ui/views/focus/focus_manager.h"

namespace views {

// static
bool UnhandledKeyboardEventHandler::HandleNativeKeyboardEvent(
    const input::NativeWebKeyboardEvent& event,
    FocusManager* focus_manager) {
  if (event.skip_if_unhandled) {
    return false;
  }

#if BUILDFLAG(IS_ANDROID)
  // Android doesn’t use the desktop KeyEvent path here.
  return false;
#else
   // Desktop: forward the underlying ui::KeyEvent.
   return !focus_manager->OnKeyEvent(
       *(event.os_event->AsKeyEvent()));
#endif
}

}  // namespace views
