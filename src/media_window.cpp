#include "media_window.hpp"

#include "mpris.hpp"

#include "utils.hpp"

#include <LayerShellQt/Shell>
#include <LayerShellQt/window.h>

#include <QDir>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QScrollArea>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

#include <cstdint>

#include <iostream>
#include <memory>
#include <optional>
#include <print>

// PlayerWidget Implementation
PlayerWidget::PlayerWidget(const QString &playerName, QWidget *parent)
    : QFrame(parent), m_playerName(playerName) {
  setupUI();
  setupControls();

  // Style the widget
  setFrameStyle(QFrame::Box);
  setLineWidth(1);
  setStyleSheet("QFrame { margin: 5px; padding: 5px; border: 1px solid gray; "
                "border-radius: 5px; }");
}

void PlayerWidget::setupUI() {
  // Main horizontal layout (image on the left, everything else on the right)
  auto *mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->setSpacing(10);

  // Album Art
  m_albumArt = new QLabel(this);
  m_albumArt->setFixedSize(96, 96);
  m_albumArt->setAlignment(Qt::AlignCenter);
  m_albumArt->setStyleSheet("border: 1px solid gray;");
  mainLayout->addWidget(m_albumArt);

  auto *rightLayout = new QVBoxLayout();
  rightLayout->setSpacing(6);

  m_playerNameLabel = new QLabel(m_playerName, this);
  m_titleLabel = new QLabel("No track", this);
  m_artistLabel = new QLabel("No artist", this);

  m_playerNameLabel->setAlignment(Qt::AlignLeft);
  m_titleLabel->setAlignment(Qt::AlignLeft);
  m_artistLabel->setAlignment(Qt::AlignLeft);

  // Style labels
  m_playerNameLabel->setStyleSheet("font-weight: bold; color: #0066cc;");
  m_titleLabel->setStyleSheet("font-size: 14px;");
  m_artistLabel->setStyleSheet("font-size: 12px; color: #666;");

  rightLayout->addWidget(m_playerNameLabel);
  rightLayout->addWidget(m_titleLabel);
  rightLayout->addWidget(m_artistLabel);

  // Progress bar
  m_progressBar = new QProgressBar(this);
  m_progressBar->setMinimum(0);
  m_progressBar->setMaximum(100);
  rightLayout->addWidget(m_progressBar);

  // Playback Controls (2 rows)
  auto *controlsRow1 = new QHBoxLayout();
  controlsRow1->setSpacing(4);

  m_seekBackBtn = new QPushButton("⏪", this); // -5s
  m_prevBtn = new QPushButton("⏮", this);
  m_playPauseBtn = new QPushButton("⏯", this);
  m_nextBtn = new QPushButton("⏭", this);
  m_seekFwdBtn = new QPushButton("⏩", this); // +5s

  // Style buttons
  QString buttonStyle =
      "QPushButton { min-width: 30px; min-height: 30px; font-size: 16px; }";
  m_seekBackBtn->setStyleSheet(buttonStyle);
  m_prevBtn->setStyleSheet(buttonStyle);
  m_playPauseBtn->setStyleSheet(buttonStyle);
  m_nextBtn->setStyleSheet(buttonStyle);
  m_seekFwdBtn->setStyleSheet(buttonStyle);

  controlsRow1->addStretch();
  controlsRow1->addWidget(m_seekBackBtn);
  controlsRow1->addWidget(m_prevBtn);
  controlsRow1->addWidget(m_playPauseBtn);
  controlsRow1->addWidget(m_nextBtn);
  controlsRow1->addWidget(m_seekFwdBtn);
  controlsRow1->addStretch();

  rightLayout->addLayout(controlsRow1);

  auto *controlsRow2 = new QHBoxLayout();
  controlsRow2->setSpacing(4);

  m_loopBtn = new QPushButton("🔁", this);
  m_shuffleBtn = new QPushButton("🔀", this);

  m_loopBtn->setStyleSheet(buttonStyle);
  m_shuffleBtn->setStyleSheet(buttonStyle);

  controlsRow2->addStretch();
  controlsRow2->addWidget(m_loopBtn);
  controlsRow2->addWidget(m_shuffleBtn);
  controlsRow2->addStretch();

  rightLayout->addLayout(controlsRow2);

  // Combine left and right sections
  mainLayout->addLayout(rightLayout);
  setLayout(mainLayout);
}

void PlayerWidget::setupControls() {
  // Button functionality
  connect(m_seekBackBtn, &QPushButton::clicked, this, [this] {
    exec(QString("playerctl -p %1 position 5-").arg(m_playerName));
  });
  connect(m_seekFwdBtn, &QPushButton::clicked, this, [this] {
    exec(QString("playerctl -p %1 position 5+").arg(m_playerName));
  });
  connect(m_prevBtn, &QPushButton::clicked, this, [this] {
    exec(QString("playerctl -p %1 previous").arg(m_playerName));
  });
  connect(m_playPauseBtn, &QPushButton::clicked, this, [this] {
    exec(QString("playerctl -p %1 play-pause").arg(m_playerName));
  });
  connect(m_nextBtn, &QPushButton::clicked, this,
          [this] { exec(QString("playerctl -p %1 next").arg(m_playerName)); });
  connect(m_loopBtn, &QPushButton::clicked, this, [this] {
    exec(QString("playerctl -p %1 loop Toggle").arg(m_playerName));
  });
  connect(m_shuffleBtn, &QPushButton::clicked, this, [this] {
    exec(QString("playerctl -p %1 shuffle Toggle").arg(m_playerName));
  });
}

void PlayerWidget::updatePlayerData(const QString &title, const QString &artist,
                                    int position, int length,
                                    const QString &albumArtPath) {
  m_titleLabel->setText(title.isEmpty() ? "No track" : title);
  m_artistLabel->setText(artist.isEmpty() ? "No artist" : artist);

  if (length > 0) {
    m_progressBar->setValue(position);
    m_progressBar->setMaximum(length);

    // Format time for display
    int posMin = position / 60;
    int posSec = position % 60;
    int lenMin = length / 60;
    int lenSec = length % 60;

    m_playerNameLabel->setText(QString("%1 [%2:%3 / %4:%5]")
                                   .arg(m_playerName)
                                   .arg(posMin, 2, 10, QChar('0'))
                                   .arg(posSec, 2, 10, QChar('0'))
                                   .arg(lenMin, 2, 10, QChar('0'))
                                   .arg(lenSec, 2, 10, QChar('0')));
  } else {
    m_progressBar->setValue(0);
    m_progressBar->setMaximum(100);
    m_playerNameLabel->setText(m_playerName);
  }

  // Update album art if path is provided and different
  if (!albumArtPath.isEmpty() && QFile::exists(albumArtPath)) {
    QPixmap newPixmap(albumArtPath);
    if (!newPixmap.isNull()) {
      m_albumArtPix = newPixmap.scaled(96, 96, Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
      m_albumArt->setPixmap(m_albumArtPix);
    }
  }
}

// MediaWindow Implementation
MediaWindow::MediaWindow(QWidget *parent) : QWidget(parent) {
  setupWindow();
  setupUI();

  auto *updateTimer = new QTimer(this);
  connect(updateTimer, &QTimer::timeout, this, &MediaWindow::updateData);
  updateTimer->start(1000); // Update every second
}

void MediaWindow::setupWindow() {
  setWindowTitle("MediaWindow - All Players");

  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint |
                 Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
  setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
  setAttribute(Qt::WA_AlwaysShowToolTips, true);

  // Make it larger to accommodate multiple players
  setGeometry(0, 0, 500, 600);
}

void MediaWindow::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(5, 5, 5, 5);
  mainLayout->setSpacing(0);

  // Create scroll area
  m_scrollArea = new QScrollArea(this);
  m_scrollArea->setWidgetResizable(true);
  m_scrollArea->setVerticalScrollBarPolicy(
      Qt::ScrollBarPolicy::ScrollBarAsNeeded);
  m_scrollArea->setHorizontalScrollBarPolicy(
      Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

  // Create content widget for scroll area
  m_scrollContent = new QWidget();
  m_scrollLayout = new QVBoxLayout(m_scrollContent);
  m_scrollLayout->setContentsMargins(0, 0, 0, 0);
  m_scrollLayout->setSpacing(5);
  m_scrollLayout->addStretch(); // Add stretch at the end

  m_scrollContent->setLayout(m_scrollLayout);
  m_scrollArea->setWidget(m_scrollContent);

  mainLayout->addWidget(m_scrollArea);
  setLayout(mainLayout);
}

PlayerWidget *MediaWindow::findOrCreatePlayerWidget(const QString &playerName) {
  // Find existing widget
  for (auto *widget : m_playerWidgets) {
    if (widget->getPlayerName() == playerName) {
      return widget;
    }
  }

  // Create new widget
  auto *newWidget = new PlayerWidget(playerName, this);
  m_playerWidgets.append(newWidget);

  // Insert before the stretch (which is the last item)
  m_scrollLayout->insertWidget(m_scrollLayout->count() - 1, newWidget);

  return newWidget;
}

void MediaWindow::removeUnusedPlayerWidgets(
    const QStringList &activePlayerNames) {
  auto it = m_playerWidgets.begin();
  while (it != m_playerWidgets.end()) {
    if (!activePlayerNames.contains((*it)->getPlayerName())) {
      auto *widget = *it;
      m_scrollLayout->removeWidget(widget);
      widget->deleteLater();
      it = m_playerWidgets.erase(it);
    } else {
      ++it;
    }
  }
}

QString getActivePlayer() {
  static QStringList priority = {"spotifyd", "spotify_player", "spotify",
                                 "firefox", "chrome"};

  QString raw = exec("playerctl", {"-l"});
  QStringList activePlayers = raw.split('\n', Qt::SkipEmptyParts);

  for (const QString &prefix : priority) {
    for (const QString &player : activePlayers) {
      if (player.startsWith(prefix)) {
        QString status = exec("playerctl", {"-p", player, "status"}).trimmed();
        if (status == "Playing") {
          return player;
        }
      }
    }
  }

  // Fallback: first matching player by prefix
  for (const QString &prefix : priority) {
    for (const QString &player : activePlayers) {
      if (player.startsWith(prefix)) {
        return player;
      }
    }
  }

  return {}; // empty string if no match
}

QString getAlbumArt(QString playerOverride = {}) {
  QString player = playerOverride;
  if (player.isEmpty()) {
    player = getActivePlayer();
  }

  QString artUrl =
      exec("playerctl", {"-p", player, "metadata", "mpris:artUrl"}).trimmed();

  // Fallback: look for paused players
  if (artUrl.isEmpty()) {
    QStringList players =
        exec("playerctl", {"-l"}).split('\n', Qt::SkipEmptyParts);
    for (const QString &p : players) {
      QString status = exec("playerctl", {"-p", p, "status"}).trimmed();
      if (status == "Paused") {
        QString fallbackArt =
            exec("playerctl", {"-p", p, "metadata", "mpris:artUrl"}).trimmed();
        if (!fallbackArt.isEmpty()) {
          artUrl = fallbackArt;
          break;
        }
      }
    }
  }

  if (artUrl.isEmpty()) {
    return {}; // still nothing
  }

  if (artUrl.startsWith("file://")) {
    return artUrl.mid(QString("file://").length());
  }

  if (artUrl.startsWith("http://") || artUrl.startsWith("https://")) {
    QString cacheDir = QDir::homePath() + "/.cache/spotifyPictureCache";
    QDir().mkpath(cacheDir);
    QString fileName = QFileInfo(QUrl(artUrl).path()).fileName();
    QString localPath = cacheDir + "/" + fileName;

    if (!QFile::exists(localPath)) {
      // Download using wget -c quietly
      QProcess::execute("wget", {"-c", "-q", artUrl, "-O", localPath});
    }

    return localPath;
  }

  return {}; // unknown format
}

void MediaWindow::updateData() {
  if (this->isHidden())
    return;

  auto players =
      exec("playerctl", QStringList() << "-l").split('\n', Qt::SkipEmptyParts);
  players.removeAll("");

  removeUnusedPlayerWidgets(players);

  for (const auto &playerName : players) {
    if (playerName.trimmed().isEmpty())
      continue;

    auto playerData = Mpris::getPlayerData(playerName);
    auto *playerWidget = findOrCreatePlayerWidget(playerName);

    auto albumArt = getAlbumArt(playerName);
    qWarning() << "Album Art: " << albumArt;

    playerWidget->updatePlayerData(playerData.title, playerData.artist,
                                   playerData.position, playerData.length,
                                   albumArt);
  }
}

void MediaWindow::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);

  if (auto *hndl = windowHandle()) {
    hndl->setProperty("_NET_WM_DESKTOP", 0xFFFFFFFF);

    if (isWayland()) {
      if (LayerShellQt::Window *lsWin = LayerShellQt::Window::get(hndl)) {
        lsWin->setAnchors(LayerShellQt::Window::Anchors::fromInt(
            LayerShellQt::Window::AnchorTop));
        lsWin->setLayer(LayerShellQt::Window::LayerTop);
        lsWin->setKeyboardInteractivity(
            LayerShellQt::Window::KeyboardInteractivityOnDemand);
      }
    }
  }
}

// #include "mediawindow.moc"
