#pragma once

#include <QObject>
#include <QWidget>

class TrayIcon : public QWidget {
  Q_OBJECT

public:
  TrayIcon(QWidget *parent = nullptr);
};

class Tray : public QWidget {
  Q_OBJECT

public:
  Tray(QWidget *parent = nullptr);

  QLabel *m_text;
};
