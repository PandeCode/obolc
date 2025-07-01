#include "panel.hpp"


#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QHBoxLayout>
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
#include "utils.hpp"
#include <LayerShellQt/Shell>
#include <LayerShellQt/window.h>
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

void Panel::setupWindow() {
  setWindowTitle("Panel");

  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint |
                 Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
  setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
  setAttribute(Qt::WA_AlwaysShowToolTips, true);
  setAttribute(Qt::WA_TranslucentBackground);

  // Position at top of screen
  QScreen *screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();

  int panelHeight = 32;
  setGeometry(0, 0, screenGeometry.width(), panelHeight);

  // styling
  setStyleSheet(/*css*/ R"(
QWidget {
background-color: #1e1e2e;
color: #cdd6f4;
font-family: 'JetBrains Mono', 'Fira Code', monospace;
font-size: 12px;
font-weight: 500;
}
QPushButton {
background-color: #313244;
border: none;
border-radius: 6px;
padding: 4px 12px;
margin: 4px 2px;
color: #cdd6f4;
}
QPushButton:hover {
background-color: #585b70;
}
QLabel {
padding: 4px 8px;
margin: 2px;
border-radius: 4px;
}
.workspace {
background-color: #313244;
color: #fab387;
font-weight: bold;
}
.window {
background-color: #11111b;
color: #94e2d5;
max-width: 300px;
}
.cpu {
background-color: #11111b;
color: #f38ba8;
}
.memory {
background-color: #11111b;
color: #a6e3a1;
}
.swap {
background-color: #11111b;
color: #f9e2af;
}
.time {
background-color: #313244;
color: #89b4fa;
font-weight: bold;
}
.date {
background-color: #11111b;
color: #cba6f7;
})");
}

void Panel::setupUI() {
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 0, 8, 0);
  layout->setSpacing(6);

  // Left side - Menu and workspace info
  m_menuButton = new QPushButton("", this);
  m_menuButton->setToolTip("Application Menu");

  m_workspaceLabel = new QLabel("", this);
  m_workspaceLabel->setProperty("class", "workspace");
  m_workspaceLabel->setStyleSheet("QLabel { background-color: #313244; "
                                  "color: #fab387; font-weight: bold; }");

  m_windowLabel = new QLabel("", this);
  m_windowLabel->setProperty("class", "window");
  m_windowLabel->setStyleSheet("QLabel { background-color: #11111b; color: "
                               "#94e2d5; max-width: 300px; }");

  layout->addWidget(m_menuButton);
  layout->addWidget(m_workspaceLabel);
  layout->addWidget(m_windowLabel);

  // Center spacer
  layout->addStretch();

  m_mediaLabel = new QLabel("", this);
  m_mediaLabel->setProperty("class", "media");
  m_mediaLabel->setStyleSheet(
      "QLabel {background-color: #11111b; color: white; }");

  layout->addWidget(m_mediaLabel);

  layout->addStretch();

  // Right side - System info and time
  m_cpuLabel = new QLabel("", this);
  m_cpuLabel->setStyleSheet(
      "QLabel { background-color: #11111b; color: #f38ba8; }");

  m_memoryLabel = new QLabel("", this);
  m_memoryLabel->setStyleSheet(
      "QLabel { background-color: #11111b; color: #a6e3a1; }");

  m_swapLabel = new QLabel("", this);
  m_swapLabel->setStyleSheet(
      "QLabel { background-color: #11111b; color: #f9e2af; }");

  m_dateLabel = new QLabel("", this);
  m_dateLabel->setStyleSheet(
      "QLabel { background-color: #11111b; color: #cba6f7; }");

  m_timeLabel = new QLabel("", this);
  m_timeLabel->setStyleSheet("QLabel { background-color: #313244; color: "
                             "#89b4fa; font-weight: bold; }");

  layout->addWidget(m_cpuLabel);
  layout->addWidget(m_memoryLabel);
  layout->addWidget(m_swapLabel);
  layout->addWidget(m_dateLabel);
  layout->addWidget(m_timeLabel);

  // Connect signals
  connect(m_menuButton, &QPushButton::clicked, this, &Panel::onMenuClicked);

  setLayout(layout);
}

void Panel::setupMedia() {
  m_clockTimer = new QTimer(this);
  connect(m_clockTimer, &QTimer::timeout, this, &Panel::updateMedia);
  m_clockTimer->start(1000);
}

void Panel::updateMedia() {
  QProcess process;
  // process.start("playerctl", QStringList() << "metadata" << "--format" << "{{title}} {{}}");
  process.start("lyrics-line.sh", QStringList());
  process.waitForFinished();
  auto stdout = process.readAllStandardOutput();

   m_mediaLabel->setText(stdout);
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
  QString windowText = info.currentWindow;
  if (windowText.length() > 40) {
    windowText = windowText.left(37) + "...";
  }
  m_windowLabel->setText(windowText);
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
      std::println("Running on Wayland");
      // LayerShellQt::Shell::useLayerShell();
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
