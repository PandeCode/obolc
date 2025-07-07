#include "tray.hpp"

Tray::Tray(QWidget *parent) : QWidget(parent) {
  m_text = new QLabel("tray", this);

  QHBoxLayout *layout = new QHBoxLayout(this);

  layout->setContentsMargins(8, 0, 8, 0);
  layout->setSpacing(6);

  layout->QLayout::addWidget(m_text);

  setLayout(layout);
}

TrayIcon::TrayIcon(QWidget *parent) : QWidget(parent) {}
