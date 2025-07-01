#include "utils.hpp"
#include <QtCore/qcontainerfwd.h>
#include <QtCore/qtenvironmentvariables.h>

bool isWayland() {
  static const bool _isWayland = [] {
    const QString sessionType = qgetenv("XDG_SESSION_TYPE");
    return sessionType == "wayland" || !qgetenv("WAYLAND_DISPLAY").isEmpty();
  }();
  return _isWayland;
}
