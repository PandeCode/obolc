#include "panel.hpp"

#include "media_window.hpp"
#include "mpris.hpp"
#include "tray.hpp"

#include "utils.hpp"

#include <LayerShellQt/Shell>
#include <LayerShellQt/window.h>

#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QTextStream>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <cstdint>
#include <print>

// Panel implementation
Panel::Panel(QWidget *parent) : QWidget(parent) {

  m_systemMonitor = new SystemMonitor(this);
  connect(m_systemMonitor, &SystemMonitor::systemInfoUpdated, this,
          &Panel::updateSystemDisplay);

  setupWindow();
  setupUI();
  setupTimer();
  setupMedia();
}

void Panel::setupMediaWindow(MediaWindow *mediaWindow) {
  m_mediaWindow = mediaWindow;
}
void Panel::setupMpris(Mpris *mpris) { m_mpris = mpris; }

void Panel::setupWindow() {
  setWindowTitle("Panel");

  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint |
                 Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
  setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
  setAttribute(Qt::WA_AlwaysShowToolTips, true);
  setAttribute(Qt::WA_TranslucentBackground);

  QScreen *screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();

  setGeometry(0, 0, screenGeometry.width(), static_cast<int>(m_panelHeight));
}

static inline QLabel *mkLabelClass(const char *className, QWidget *parent) {
  auto _ = new QLabel("", parent);
  _->setProperty("class", className);
  return _;
}

void Panel::setupUI() {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 0, 8, 0);
  layout->setSpacing(6);

  m_menuButton = new QPushButton(
      QIcon(QString::fromStdString(getAssetFile("nix.svg").string())), "",
      this);
  m_menuButton->setToolTip("Application Menu");
  m_menuButton->setProperty("class", "start");

  m_workspaceLabel = mkLabelClass("workspace", this);
  m_windowLabel = mkLabelClass("window", this);

  layout->addWidget(m_menuButton);
  layout->addWidget(m_workspaceLabel);
  layout->addWidget(m_windowLabel);

  // Center spacer
  layout->addStretch();

  m_mediaBtn = new QPushButton("", this);
  m_mediaBtn->setFlat(true);
  m_mediaBtn->setCursor(Qt::PointingHandCursor);
  m_mediaBtn->setProperty("class", "mediaBtn");

  connect(m_mediaBtn, &QPushButton::clicked, this, &Panel::onMediaClicked);

  layout->addWidget(m_mediaBtn);

  layout->addStretch();

  // End
  m_cpuLabel = mkLabelClass("cpu", this);
  m_memoryLabel = mkLabelClass("memory", this);
  m_swapLabel = mkLabelClass("swap", this);
  m_dateLabel = mkLabelClass("date", this);
  m_timeLabel = mkLabelClass("time", this);

  layout->addWidget(m_cpuLabel);
  layout->addWidget(m_memoryLabel);
  layout->addWidget(m_swapLabel);
  layout->addWidget(m_dateLabel);
  layout->addWidget(m_timeLabel);
  layout->addWidget(m_timeLabel);

  m_tray = new Tray(this);
  layout->addWidget(m_tray);

  connect(m_menuButton, &QPushButton::clicked, this, &Panel::onMenuClicked);

  setLayout(layout);
}

void Panel::setupMedia() {
  m_clockTimer = new QTimer(this);
  connect(m_clockTimer, &QTimer::timeout, this, &Panel::updateMedia);
  m_clockTimer->start(1000);
};

std::optional<std::tuple<QString, QString>>
getPlayerInfo(Mpris *mpris, std::optional<QString> playerName = std::nullopt) {
  auto formatMetadata = [](const QVariantMap &metadata)
      -> std::optional<std::tuple<QString, QString>> {
    QString title = metadata.value("xesam:title").toString();
    QStringList artists = metadata.value("xesam:artist").toStringList();
    QString artistStr = artists.join(", ");
    if (!title.isEmpty() || !artistStr.isEmpty()) {
      return std::make_optional(std::make_tuple(title, artistStr));
    }
    return std::nullopt;
  };

  if (playerName.has_value()) {
    auto &player = playerName.value();
    QVariantMap metadata = mpris->getPlayerMetadata(player);
    return formatMetadata(metadata);
  } else {
    QStringList players = mpris->getPlayers();
    for (const QString &player : players) {
      QVariantMap metadata = mpris->getPlayerMetadata(player);
      auto info = formatMetadata(metadata);
      if (info.has_value()) {
        return info;
      }
    }
  }

  return std::nullopt;
}

void Panel::updateMedia() {
  auto lyricsOpt = Mpris::getCurrentLyrics();
  if (lyricsOpt.has_value()) {
    m_mediaBtn->setText(lyricsOpt.value());
    return;
  }

  auto playerInfoOpt = getPlayerInfo(m_mpris);
  QString artist = "..";
  QString title = "..";
  if (playerInfoOpt.has_value()) {
    const auto &value = playerInfoOpt.value();
    artist = std::get<0>(value);
    title = std::get<1>(value);
  }
  m_mediaBtn->setText(QString("%1 - %2").arg(artist, title));
}

void Panel::setupTimer() {
  m_clockTimer = new QTimer(this);
  connect(m_clockTimer, &QTimer::timeout, this, &Panel::updateTime);
  m_clockTimer->start(1000); // Update every second
  updateTime();              // Initial update
}

void Panel::updateTime() {
  QDateTime now = QDateTime::currentDateTime();
  m_timeLabel->setText(now.toString("hh:mm:ss"));
  m_dateLabel->setText(now.toString("ddd MMM dd"));
}

void Panel::updateSystemDisplay() {
  SystemMonitor::SystemInfo info = m_systemMonitor->getSystemInfo();

  // Update system metrics
  m_cpuLabel->setText(
      QString("󰍛 %1%").arg(QString::number(info.cpuUsage, 'f', 1)));

  m_memoryLabel->setText(
      QString("󰍜 %1%").arg(QString::number(info.memoryUsage, 'f', 1)));

  if (info.swapUsage > 0.1) {
    m_swapLabel->setText(
        QString("󰟀 %1%").arg(QString::number(info.swapUsage, 'f', 1)));
    m_swapLabel->show();
  } else {
    m_swapLabel->hide();
  }

  // Update workspace and window info
  WindowManager wm = m_systemMonitor->getWindowManager();
  QString wmName;
  switch (wm) {
  case WindowManager::I3:
    wmName = "i3";
    break;
  case WindowManager::Hyprland:
    wmName = "Hyprland";
    break;
  case WindowManager::Sway:
    wmName = "Sway";
    break;
  default:
    wmName = "Unknown";
    break;
  }

  m_workspaceLabel->setText(
      QString("%1: %2").arg(wmName, info.currentWorkspace));

  // Truncate long window titles
  QString windowText = cleanTitle(info.currentWindow);

  if (windowText.length() > 40) {
    windowText = windowText.left(37) + "...";
  }
  m_windowLabel->setText(windowText);
}

void Panel::onMediaClicked() {
  if (m_mediaWindow) {
    if (m_mediaWindow->isVisible())
      m_mediaWindow->hide();
    else
      m_mediaWindow->show();
  }
}

void Panel::onMenuClicked() {
  SystemMonitor::SystemInfo info = m_systemMonitor->getSystemInfo();
  QMessageBox msg(this);
  msg.setWindowTitle("Panel Info");
  msg.setText(
      QString("Window Manager: %1\nWorkspace: %2\nWindow: %3\nCPU: %4%\nRAM: "
              "%5%\nSwap: %6%")
          .arg(m_systemMonitor->getWindowManager() == WindowManager::I3 ? "i3"
               : m_systemMonitor->getWindowManager() == WindowManager::Hyprland
                   ? "Hyprland"
                   : "Other")
          .arg(info.currentWorkspace)
          .arg(info.currentWindow)
          .arg(QString::number(info.cpuUsage, 'f', 1))
          .arg(QString::number(info.memoryUsage, 'f', 1))
          .arg(QString::number(info.swapUsage, 'f', 1)));
  msg.setIcon(QMessageBox::Information);
  msg.exec();
}

void Panel::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);

  if (auto *hndl = windowHandle()) {
    hndl->setProperty("_NET_WM_DESKTOP", 0xFFFFFFFF);

    if (isWayland()) {
      if (LayerShellQt::Window *lsWin = LayerShellQt::Window::get(hndl)) {
        lsWin->setAnchors(LayerShellQt::Window::Anchors::fromInt(
            LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorLeft |
            LayerShellQt::Window::AnchorRight));
        lsWin->setLayer(LayerShellQt::Window::LayerTop);
        lsWin->setExclusiveZone(height());
        lsWin->setKeyboardInteractivity(
            LayerShellQt::Window::KeyboardInteractivityOnDemand);
      }
    }
  }
}
