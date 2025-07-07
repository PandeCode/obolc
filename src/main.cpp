#include "media_window.hpp"
#include "mpris.hpp"
#include "panel.hpp"
#include "utils.hpp"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QTimer>
#include <QVariantMap>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setStyleSheet(QString::fromStdString(readAssetFile("style.css")));

  Mpris mpris(&app);
  mpris.startMonitoring(5000);

  MediaWindow wm;

  Panel panel;
  panel.setupMediaWindow(&wm);
  panel.setupMpris(&mpris);
  panel.show();

  return app.exec();
}
