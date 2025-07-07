#include "mpris.hpp"
#include "utils.hpp"

#include <QApplication>
#include <QDBusReply>
#include <QDebug>

QDBusInterface *Mpris::dbusIface = nullptr;
QTimer *Mpris::timer = nullptr;
QStringList Mpris::activePlayers = {};

std::optional<QString> Mpris::getCurrentLyrics() {
  auto ret = exec("lyrics-line.sh");
  return (ret.isEmpty() ? std::nullopt : std::make_optional(ret));
}

PlayerData Mpris::getPlayerData(const QString &identity) {
  auto ret = exec(
                 "playerctl",
                 QStringList()
                     << "-p" << identity << "metadata"
                     << "--format"
                     << "{{title}}\n{{artist}}\n{{position}}\n{{mpris:length}}")
                 .split('\n');

  QString title = ret.value(0);
  QString artist = ret.value(1);
  int32_t position = ret.value(2).toInt();
  int32_t length = ret.value(3).toInt();

  return {title, artist, position, length};
}

Mpris::Mpris(QObject *parent) : QObject(parent) {
  if (dbusIface or timer) {
    return;
  }

  initializeDBus();
  setObjectName("mpris");
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Mpris::updatePlayers);

  // Initial update
  updatePlayers();
}

Mpris::~Mpris() {}

void Mpris::cleanup() {
  if (timer) {
    timer->stop();
  }
  delete dbusIface;
}

void Mpris::initializeDBus() {
  dbusIface =
      new QDBusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus",
                         "org.freedesktop.DBus", QDBusConnection::sessionBus());
}

QStringList Mpris::fetchActivePlayersFromDBus() {
  QStringList mprisPlayers;

  QDBusReply<QStringList> reply = dbusIface->call("ListNames");
  if (!reply.isValid()) {
    qWarning() << "Failed to get D-Bus names:" << reply.error().message();
    return mprisPlayers;
  }

  QStringList allServices = reply.value();
  for (const QString &service : allServices) {
    if (service.startsWith("org.mpris.MediaPlayer2.")) {
      mprisPlayers << service;
    }
  }

  return mprisPlayers;
}

void Mpris::updatePlayers() {
  QStringList newPlayers = fetchActivePlayersFromDBus();

  if (newPlayers != activePlayers) {
    activePlayers = newPlayers;
    emit playersChanged(activePlayers);

    qDebug() << "Active MPRIS players:" << activePlayers;

    // Emit metadata for all active players
    for (const QString &player : activePlayers) {
      qWarning() << "PLayer: " << player;
      QVariantMap metadata = getPlayerMetadata(player);
      if (!metadata.isEmpty()) {
        emit playerMetadataChanged(player, metadata);
      }
    }
  }
}

QVariantMap Mpris::getPlayerMetadata(const QString &playerName) {
  QVariantMap metadata;

  QDBusInterface metadataIface(playerName, "/org/mpris/MediaPlayer2",
                               "org.freedesktop.DBus.Properties",
                               QDBusConnection::sessionBus());

  if (!metadataIface.isValid()) {
    qWarning() << "Failed to create metadata interface for" << playerName;
    return metadata;
  }

  QDBusReply<QVariant> metaReply =
      metadataIface.call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");

  if (!metaReply.isValid()) {
    qWarning() << "Failed to get metadata from" << playerName << ":"
               << metaReply.error().message();
    return metadata;
  }

  metadata = qdbus_cast<QVariantMap>(metaReply.value());

  // qDebug() << "Metadata for player:" << playerName;
  // for (auto it = metadata.constBegin(); it != metadata.constEnd(); ++it) {
  // qDebug() << "  " << it.key() << ":" << it.value();
  // }

  return metadata;
}

void Mpris::startMonitoring(int intervalMs) {
  if (timer) {
    timer->start(intervalMs);
    qDebug() << "Started MPRIS monitoring with interval:" << intervalMs << "ms";
  }
}

void Mpris::stopMonitoring() {
  if (timer) {
    timer->stop();
    qDebug() << "Stopped MPRIS monitoring";
  }
}
