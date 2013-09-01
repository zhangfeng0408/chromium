// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/screen.h"

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "base/logging.h"
#include "ui/gfx/display.h"

namespace {

bool GetScreenWorkArea(gfx::Rect* out_rect) {
  gboolean ok;
  guchar* raw_data = NULL;
  gint data_len = 0;
  ok = gdk_property_get(gdk_get_default_root_window(),  // a gdk window
                        gdk_atom_intern("_NET_WORKAREA", FALSE),  // property
                        gdk_atom_intern("CARDINAL", FALSE),  // property type
                        0,  // byte offset into property
                        0xff,  // property length to retrieve
                        false,  // delete property after retrieval?
                        NULL,  // returned property type
                        NULL,  // returned data format
                        &data_len,  // returned data len
                        &raw_data);  // returned data
  if (!ok)
    return false;

  // We expect to get four longs back: x, y, width, height.
  if (data_len < static_cast<gint>(4 * sizeof(glong))) {
    NOTREACHED();
    g_free(raw_data);
    return false;
  }

  glong* data = reinterpret_cast<glong*>(raw_data);
  gint x = data[0];
  gint y = data[1];
  gint width = data[2];
  gint height = data[3];
  g_free(raw_data);

  out_rect->SetRect(x, y, width, height);
  return true;
}

gfx::Display GetDisplayForMonitorNum(GdkScreen* screen, gint monitor_num) {
  GdkRectangle bounds;
  gdk_screen_get_monitor_geometry(screen, monitor_num, &bounds);
  // Use |monitor_num| as display id.
  gfx::Display display(monitor_num, gfx::Rect(bounds));
  if (gdk_screen_get_primary_monitor(screen) == monitor_num) {
    gfx::Rect rect;
    if (GetScreenWorkArea(&rect))
      display.set_work_area(gfx::IntersectRects(rect, display.bounds()));
  }
  return display;
}

gfx::Display GetMonitorAreaNearestWindow(gfx::NativeView view) {
  GdkScreen* screen = gdk_screen_get_default();
  gint monitor_num = 0;
  if (view && GTK_IS_WINDOW(view)) {
    GtkWidget* top_level = gtk_widget_get_toplevel(view);
    DCHECK(GTK_IS_WINDOW(top_level));
    GtkWindow* window = GTK_WINDOW(top_level);
    screen = gtk_window_get_screen(window);
    monitor_num = gdk_screen_get_monitor_at_window(
        screen,
        gtk_widget_get_window(top_level));
  }
  return GetDisplayForMonitorNum(screen, monitor_num);
}

class ScreenGtk : public gfx::Screen {
 public:
  ScreenGtk() {
  }

  virtual ~ScreenGtk() {
  }

  virtual bool IsDIPEnabled() OVERRIDE {
    return false;
  }

  virtual gfx::Point GetCursorScreenPoint() OVERRIDE {
    gint x, y;
    gdk_display_get_pointer(gdk_display_get_default(), NULL, &x, &y, NULL);
    return gfx::Point(x, y);
  }

  // Returns the window under the cursor.
  virtual gfx::NativeWindow GetWindowUnderCursor() OVERRIDE {
    GdkWindow* window = gdk_window_at_pointer(NULL, NULL);
    if (!window)
      return NULL;

    gpointer data = NULL;
    gdk_window_get_user_data(window, &data);
    GtkWidget* widget = reinterpret_cast<GtkWidget*>(data);
    if (!widget)
      return NULL;
    widget = gtk_widget_get_toplevel(widget);
    return GTK_IS_WINDOW(widget) ? GTK_WINDOW(widget) : NULL;
  }

  virtual gfx::NativeWindow GetWindowAtScreenPoint(const gfx::Point& point)
      OVERRIDE {
    NOTIMPLEMENTED();
    return NULL;
  }

  // Returns the number of displays.
  // Mirrored displays are excluded; this method is intended to return the
  // number of distinct, usable displays.
  virtual int GetNumDisplays() const OVERRIDE {
    // This query is kinda bogus for Linux -- do we want number of X screens?
    // The number of monitors Xinerama has?  We'll just use whatever GDK uses.
    GdkScreen* screen = gdk_screen_get_default();
    return gdk_screen_get_n_monitors(screen);
  }

  virtual std::vector<gfx::Display> GetAllDisplays() const OVERRIDE {
    GdkScreen* screen = gdk_screen_get_default();
    gint num_of_displays = gdk_screen_get_n_monitors(screen);
    std::vector<gfx::Display> all_displays;
    for (gint i = 0; i < num_of_displays; ++i)
      all_displays.push_back(GetDisplayForMonitorNum(screen, i));
    return all_displays;
  }

  // Returns the display nearest the specified window.
  virtual gfx::Display GetDisplayNearestWindow(
      gfx::NativeView view) const OVERRIDE {
    // Do not use the _NET_WORKAREA here, this is supposed to be an area on a
    // specific monitor, and _NET_WORKAREA is a hint from the WM that
    // generally spans across all monitors.  This would make the work area
    // larger than the monitor.
    // TODO(danakj) This is a work-around as there is no standard way to get
    // this area, but it is a rect that we should be computing.  The standard
    // means to compute this rect would be to watch all windows with
    // _NET_WM_STRUT(_PARTIAL) hints, and subtract their space from the
    // physical area of the display to construct a work area.
    // TODO(oshima): Implement Observer.
    return GetMonitorAreaNearestWindow(view);
  }

  // Returns the the display nearest the specified point.
  virtual gfx::Display GetDisplayNearestPoint(
      const gfx::Point& point) const OVERRIDE {
    GdkScreen* screen = gdk_screen_get_default();
    gint monitor = gdk_screen_get_monitor_at_point(
        screen, point.x(), point.y());
    // TODO(oshima): Implement Observer.
    return GetDisplayForMonitorNum(screen, monitor);
  }

  // Returns the display that most closely intersects the provided bounds.
  virtual gfx::Display GetDisplayMatching(
      const gfx::Rect& match_rect) const OVERRIDE {
    // TODO(thestig) Implement multi-monitor support.
    return GetPrimaryDisplay();
  }

  // Returns the primary display.
  virtual gfx::Display GetPrimaryDisplay() const OVERRIDE {
    GdkScreen* screen = gdk_screen_get_default();
    gint primary_monitor_index = gdk_screen_get_primary_monitor(screen);
    // TODO(oshima): Implement Observer.
    return GetDisplayForMonitorNum(screen, primary_monitor_index);
  }

  virtual void AddObserver(gfx::DisplayObserver* observer) OVERRIDE {
    // TODO(oshima): crbug.com/122863.
  }

  virtual void RemoveObserver(gfx::DisplayObserver* observer) OVERRIDE {
    // TODO(oshima): crbug.com/122863.
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ScreenGtk);
};

}  // namespace

namespace gfx {

Screen* CreateNativeScreen() {
  return new ScreenGtk;
}

}  // namespace gfx
