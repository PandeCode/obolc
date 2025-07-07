#pragma once

#include <QDBusConnection>
#include <QDBusInterface>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>

#include <optional>

struct PlayerData {
  QString title, artist;
  int32_t position, length;
};

class Mpris : public QObject {
  Q_OBJECT

public:
  explicit Mpris(QObject *parent = nullptr);
  ~Mpris();

  QStringList getPlayers() const { return activePlayers; }
  QVariantMap getPlayerMetadata(const QString &playerName);
  void startMonitoring(int intervalMs = 5000);
  void stopMonitoring();
  void cleanup();

  std::optional<QString> getLyrics();

public slots:
  void updatePlayers();

  static std::optional<QString> getCurrentLyrics();
  static PlayerData getPlayerData(const QString &identity);

signals:
  void playersChanged(const QStringList &players);
  void playerMetadataChanged(const QString &playerName,
                             const QVariantMap &metadata);

private:
  static QDBusInterface *dbusIface;
  static QTimer *timer;
  static QStringList activePlayers;

  void initializeDBus();
  QStringList fetchActivePlayersFromDBus();
};
