#pragma once
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QFrame>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollArea>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

class PlayerWidget : public QFrame {
  Q_OBJECT
public:
  PlayerWidget(const QString &playerName, QWidget *parent = nullptr);
  void updatePlayerData(const QString &title, const QString &artist,
                        int position, int length, const QString &albumArtPath);
  QString getPlayerName() const { return m_playerName; }

private:
  QString m_playerName;
  QLabel *m_titleLabel;
  QLabel *m_artistLabel;
  QLabel *m_playerNameLabel;
  QProgressBar *m_progressBar;
  QPixmap m_albumArtPix;
  QLabel *m_albumArt;

  QPushButton *m_seekBackBtn;
  QPushButton *m_prevBtn;
  QPushButton *m_playPauseBtn;
  QPushButton *m_nextBtn;
  QPushButton *m_seekFwdBtn;
  QPushButton *m_loopBtn;
  QPushButton *m_shuffleBtn;

  void setupUI();
  void setupControls();
};

class MediaWindow : public QWidget {
  Q_OBJECT
public:
  MediaWindow(QWidget *parent = nullptr);

private:
  QScrollArea *m_scrollArea;
  QWidget *m_scrollContent;
  QVBoxLayout *m_scrollLayout;
  QList<PlayerWidget *> m_playerWidgets;

  void setupWindow();
  void setupUI();
  PlayerWidget *findOrCreatePlayerWidget(const QString &playerName);
  void removeUnusedPlayerWidgets(const QStringList &activePlayerNames);

private slots:
  void updateData();

protected:
  void showEvent(QShowEvent *event) override;
};
