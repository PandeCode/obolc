#pragma once

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

enum class WindowManager : uint8_t { Unknown, I3, Hyprland, Sway, Other };

class SystemMonitor : public QObject {
  Q_OBJECT

public:
  struct SystemInfo {
    double cpuUsage = 0.0;
    double memoryUsage = 0.0;
    double swapUsage = 0.0;
    QString currentWindow;
    QString currentWorkspace;
  };

  SystemMonitor(QObject *parent = nullptr);
  SystemInfo getSystemInfo() const;
  WindowManager getWindowManager() const;

signals:
  void systemInfoUpdated();

private slots:
  void updateSystemInfo();

private:
  SystemInfo m_systemInfo;
  WindowManager m_windowManager = WindowManager::Unknown;
  QTimer *m_updateTimer;
  QProcess *m_workspaceProcess = nullptr;

  // CPU monitoring
  struct CpuTimes {
    long long idle = 0;
    long long total = 0;
  };
  CpuTimes m_lastCpuTimes;

  void detectWindowManager();
  void setupSystemMonitoring();
  void setupWorkspaceMonitoring();
  void updateCpuUsage();
  void updateMemoryUsage();
  void updateWorkspaceInfo();
  void updateI3Info();
  QString findFocusedWindow(const QJsonObject &node);
  void updateHyprlandInfo();
};
