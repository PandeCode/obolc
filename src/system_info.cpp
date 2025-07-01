#include "system_info.hpp"

SystemMonitor::SystemMonitor(QObject *parent) : QObject(parent) {
  detectWindowManager();
  setupSystemMonitoring();
  setupWorkspaceMonitoring();
}

SystemMonitor::SystemInfo SystemMonitor::getSystemInfo() const {
  return m_systemInfo;
}

WindowManager SystemMonitor::getWindowManager() const {
  return m_windowManager;
}

void SystemMonitor::updateSystemInfo() {
  updateCpuUsage();
  updateMemoryUsage();
  updateWorkspaceInfo();
  emit systemInfoUpdated();
}

void SystemMonitor::detectWindowManager() {
  QString sessionType = qgetenv("XDG_SESSION_TYPE");
  QString currentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
  QString waylandDisplay = qgetenv("WAYLAND_DISPLAY");

  // Check for specific window managers
  if (QProcess::execute("pgrep", QStringList() << "i3") == 0) {
    m_windowManager = WindowManager::I3;
  } else if (QProcess::execute("pgrep", QStringList() << "Hyprland") == 0) {
    m_windowManager = WindowManager::Hyprland;
  } else if (QProcess::execute("pgrep", QStringList() << "sway") == 0) {
    m_windowManager = WindowManager::Sway;
  } else {
    m_windowManager = WindowManager::Other;
  }
}

void SystemMonitor::setupSystemMonitoring() {
  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, &QTimer::timeout, this,
          &SystemMonitor::updateSystemInfo);
  m_updateTimer->start(2000); // Update every 2 seconds

  // Initial update
  updateSystemInfo();
}

void SystemMonitor::setupWorkspaceMonitoring() {
  // Set up workspace monitoring based on WM
  if (m_windowManager == WindowManager::I3) {
    // i3 IPC monitoring would go here
    // For now, we'll poll with the timer
  } else if (m_windowManager == WindowManager::Hyprland) {
    // Hyprland socket monitoring would go here
    // For now, we'll poll with the timer
  }
}

void SystemMonitor::updateCpuUsage() {
  QFile file("/proc/stat");
  if (!file.open(QIODevice::ReadOnly))
    return;

  QString line = file.readLine();
  QStringList parts = line.split(QRegularExpression("\\s+"));

  if (parts.size() >= 8) {
    long long user = parts[1].toLongLong();
    long long nice = parts[2].toLongLong();
    long long system = parts[3].toLongLong();
    long long idle = parts[4].toLongLong();
    long long iowait = parts[5].toLongLong();
    long long irq = parts[6].toLongLong();
    long long softirq = parts[7].toLongLong();

    long long totalIdle = idle + iowait;
    long long totalNonIdle = user + nice + system + irq + softirq;
    long long total = totalIdle + totalNonIdle;

    if (m_lastCpuTimes.total != 0) {
      long long totalDiff = total - m_lastCpuTimes.total;
      long long idleDiff = totalIdle - m_lastCpuTimes.idle;

      if (totalDiff > 0) {
        m_systemInfo.cpuUsage =
            ((double)(totalDiff - idleDiff) / totalDiff) * 100.0;
      }
    }

    m_lastCpuTimes.idle = totalIdle;
    m_lastCpuTimes.total = total;
  }
}

void SystemMonitor::updateMemoryUsage() {
  QFile file("/proc/meminfo");
  if (!file.open(QIODevice::ReadOnly))
    return;

  QTextStream stream(&file);
  long long memTotal = 0, memAvailable = 0, swapTotal = 0, swapFree = 0;

  QString line;
  while (stream.readLineInto(&line)) {
    if (line.startsWith("MemTotal:")) {
      memTotal = line.split(QRegularExpression("\\s+"))[1].toLongLong();
    } else if (line.startsWith("MemAvailable:")) {
      memAvailable = line.split(QRegularExpression("\\s+"))[1].toLongLong();
    } else if (line.startsWith("SwapTotal:")) {
      swapTotal = line.split(QRegularExpression("\\s+"))[1].toLongLong();
    } else if (line.startsWith("SwapFree:")) {
      swapFree = line.split(QRegularExpression("\\s+"))[1].toLongLong();
    }
  }

  if (memTotal > 0) {
    m_systemInfo.memoryUsage =
        ((double)(memTotal - memAvailable) / memTotal) * 100.0;
  }

  if (swapTotal > 0) {
    m_systemInfo.swapUsage =
        ((double)(swapTotal - swapFree) / swapTotal) * 100.0;
  }
}

void SystemMonitor::updateWorkspaceInfo() {
  switch (m_windowManager) {
  case WindowManager::I3:
    updateI3Info();
    break;
  case WindowManager::Hyprland:
    updateHyprlandInfo();
    break;
  default:
    m_systemInfo.currentWorkspace = "Unknown WM";
    m_systemInfo.currentWindow = "";
    break;
  }
}

void SystemMonitor::updateI3Info() {
  // Get current workspace
  QProcess process;
  process.start("i3-msg", QStringList() << "-t" << "get_workspaces");
  process.waitForFinished();

  QJsonDocument doc = QJsonDocument::fromJson(process.readAllStandardOutput());
  if (doc.isArray()) {
    QJsonArray workspaces = doc.array();
    for (const QJsonValue &workspace : workspaces) {
      QJsonObject ws = workspace.toObject();
      if (ws["focused"].toBool()) {
        m_systemInfo.currentWorkspace = ws["name"].toString();
        break;
      }
    }
  }

  // Get focused window
  process.start("i3-msg", QStringList() << "-t" << "get_tree");
  process.waitForFinished();

  doc = QJsonDocument::fromJson(process.readAllStandardOutput());
  if (doc.isObject()) {
    QString windowName = findFocusedWindow(doc.object());
    m_systemInfo.currentWindow = windowName.isEmpty() ? "Desktop" : windowName;
  }
}

QString SystemMonitor::findFocusedWindow(const QJsonObject &node) {
  if (node["focused"].toBool() && !node["name"].toString().isEmpty()) {
    return node["name"].toString();
  }

  QJsonArray nodes = node["nodes"].toArray();
  for (const QJsonValue &child : nodes) {
    QString result = findFocusedWindow(child.toObject());
    if (!result.isEmpty())
      return result;
  }

  QJsonArray floating = node["floating_nodes"].toArray();
  for (const QJsonValue &child : floating) {
    QString result = findFocusedWindow(child.toObject());
    if (!result.isEmpty())
      return result;
  }

  return QString();
}

void SystemMonitor::updateHyprlandInfo() {
  // Get active workspace
  QProcess process;
  process.start("hyprctl", QStringList() << "activeworkspace" << "-j");
  process.waitForFinished();

  QJsonDocument doc = QJsonDocument::fromJson(process.readAllStandardOutput());
  if (doc.isObject()) {
    QJsonObject workspace = doc.object();
    m_systemInfo.currentWorkspace = QString::number(workspace["id"].toInt());
  }

  // Get active window
  process.start("hyprctl", QStringList() << "activewindow" << "-j");
  process.waitForFinished();

  doc = QJsonDocument::fromJson(process.readAllStandardOutput());
  if (doc.isObject()) {
    QJsonObject window = doc.object();
    QString title = window["title"].toString();
    m_systemInfo.currentWindow = title.isEmpty() ? "Desktop" : title;
  }
}
