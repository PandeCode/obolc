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

#include "media_window.hpp"
#include "mpris.hpp"
#include "system_info.hpp"
#include "tray.hpp"

class Panel : public QWidget {
  Q_OBJECT

  uint8_t m_panelHeight = 42;

public:
  Panel(QWidget *parent = nullptr);
  void setupMediaWindow(MediaWindow *mediaWindow);
  void setupMpris(Mpris *mpris);
  MediaWindow *m_mediaWindow;
  Mpris *m_mpris;
  void onMediaClicked();

private:
  SystemMonitor *m_systemMonitor;
  QLabel *m_timeLabel;
  QLabel *m_dateLabel;
  QLabel *m_workspaceLabel;
  QLabel *m_windowLabel;
  QLabel *m_cpuLabel;
  QLabel *m_memoryLabel;
  QLabel *m_swapLabel;
  QPushButton *m_mediaBtn;
  QPushButton *m_menuButton;
  QTimer *m_clockTimer;

  Tray *m_tray;

  void setupWindow();
  void setupUI();
  void setupTimer();
  void setupMedia();

private slots:
  void updateTime();
  void updateMedia();
  void updateSystemDisplay();
  void onMenuClicked();

protected:
  void showEvent(QShowEvent *event) override;
};

// #include "panel.moc"
