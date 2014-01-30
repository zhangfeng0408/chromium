// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_OZONE_EVENT_FACTORY_DELEGATE_EVDEV_H_
#define UI_EVENTS_OZONE_EVENT_FACTORY_DELEGATE_EVDEV_H_

#include "base/compiler_specific.h"
#include "base/files/file_path.h"
#include "ui/events/events_export.h"
#include "ui/events/ozone/evdev/event_converter.h"
#include "ui/events/ozone/evdev/event_modifiers.h"
#include "ui/events/ozone/event_factory_ozone.h"

#if defined(USE_UDEV)
#include "ui/events/ozone/evdev/scoped_udev.h"
#endif

namespace ui {

// Ozone events implementation for the Linux input subsystem ("evdev").
class EVENTS_EXPORT EventFactoryEvdev : public EventFactoryOzone {
 public:
  EventFactoryEvdev();
  virtual ~EventFactoryEvdev();

  virtual void StartProcessingEvents() OVERRIDE;

 private:
  // Open device at path & starting processing events.
  void AttachInputDevice(const base::FilePath& file_path);

  // Scan & open devices in /dev/input (without udev).
  void StartProcessingEventsManual();

#if defined(USE_UDEV)
  // Scan & open devices using udev.
  void StartProcessingEventsUdev();
#endif

  // Owned per-device event converters (by path).
  std::map<base::FilePath, EventConverterEvdev*> converters_;

#if defined(USE_UDEV)
  // Udev daemon connection.
  scoped_udev udev_;
#endif

  EventModifiersEvdev modifiers_;

  DISALLOW_COPY_AND_ASSIGN(EventFactoryEvdev);
};

}  // namespace ui

#endif  // UI_EVENTS_OZONE_EVENT_FACTORY_DELEGATE_EVDEV_H_
