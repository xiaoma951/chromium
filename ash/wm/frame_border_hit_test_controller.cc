// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/frame_border_hit_test_controller.h"

#include "ash/ash_constants.h"
#include "ash/wm/header_painter.h"
#include "ash/wm/resize_handle_window_targeter.h"
#include "ash/wm/window_state_observer.h"
#include "ui/aura/env.h"
#include "ui/aura/window.h"
#include "ui/aura/window_observer.h"
#include "ui/aura/window_targeter.h"
#include "ui/base/hit_test.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/window/non_client_view.h"

namespace ash {

FrameBorderHitTestController::FrameBorderHitTestController(views::Widget* frame)
    : frame_window_(frame->GetNativeWindow()) {
  frame_window_->SetEventTargeter(scoped_ptr<ui::EventTargeter>(
      new ResizeHandleWindowTargeter(frame_window_, NULL)));
}

FrameBorderHitTestController::~FrameBorderHitTestController() {
}

// static
int FrameBorderHitTestController::NonClientHitTest(
    views::NonClientFrameView* view,
    HeaderPainter* header_painter,
    const gfx::Point& point) {
  gfx::Rect expanded_bounds = view->bounds();
  int outside_bounds = kResizeOutsideBoundsSize;

  if (aura::Env::GetInstance()->is_touch_down())
    outside_bounds *= kResizeOutsideBoundsScaleForTouch;
  expanded_bounds.Inset(-outside_bounds, -outside_bounds);

  if (!expanded_bounds.Contains(point))
    return HTNOWHERE;

  // Check the frame first, as we allow a small area overlapping the contents
  // to be used for resize handles.
  views::Widget* frame = view->GetWidget();
  bool can_ever_resize = frame->widget_delegate()->CanResize();
  // Don't allow overlapping resize handles when the window is maximized or
  // fullscreen, as it can't be resized in those states.
  int resize_border =
      frame->IsMaximized() || frame->IsFullscreen() ? 0 :
      kResizeInsideBoundsSize;
  int frame_component = view->GetHTComponentForFrame(point,
                                                     resize_border,
                                                     resize_border,
                                                     kResizeAreaCornerSize,
                                                     kResizeAreaCornerSize,
                                                     can_ever_resize);
  if (frame_component != HTNOWHERE)
    return frame_component;

  int client_component = frame->client_view()->NonClientHitTest(point);
  if (client_component != HTNOWHERE)
    return client_component;

  return header_painter->NonClientHitTest(point);
}

}  // namespace ash
