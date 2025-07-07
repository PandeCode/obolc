#pragma once

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <string>

bool isWayland();

std::string readFile(const std::string &filePath);
std::filesystem::path getAssetFile(const std::string &filename);
std::string readAssetFile(const std::string &filename);

QString exec(const QString &pname, const QStringList &params);
QString exec(const QString &pname);
QString cleanTitle(QString title);

enum class Color : uint8_t {
  Base00,
  Base01,
  Base02,
  Base03,
  Base04,
  Base05,
  Base06,
  Base07,
  Base08,
  Base09,
  Base0A,
  Base0B,
  Base0C,
  Base0D,
  Base0E,
  Base0F,
};

inline QString getColor(Color color, bool reload = false) {
  static QMap<QString, QString> palette;
  static bool loaded = false;

  if (!loaded || reload) {
    QFile file(QDir::homePath() + "/.config/stylix/palette.json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
      if (err.error == QJsonParseError::NoError && doc.isObject()) {
        for (auto it = doc.object().begin(); it != doc.object().end(); ++it) {
          if (it.value().isString())
            palette[it.key()] = it.value().toString();
        }
      }
    }

    if (palette.isEmpty()) {
      palette = {
          {"base00", "1a1b26"}, {"base01", "16161e"}, {"base02", "2f3549"},
          {"base03", "444b6a"}, {"base04", "787c99"}, {"base05", "a9b1d6"},
          {"base06", "cbccd1"}, {"base07", "d5d6db"}, {"base08", "c0caf5"},
          {"base09", "a9b1d6"}, {"base0A", "0db9d7"}, {"base0B", "9ece6a"},
          {"base0C", "b4f9f8"}, {"base0D", "2ac3de"}, {"base0E", "bb9af7"},
          {"base0F", "f7768e"},
      };
    }

    loaded = true;
  }

  static const QMap<Color, QString> colorMap = {
      {Color::Base00, "base00"}, {Color::Base01, "base01"},
      {Color::Base02, "base02"}, {Color::Base03, "base03"},
      {Color::Base04, "base04"}, {Color::Base05, "base05"},
      {Color::Base06, "base06"}, {Color::Base07, "base07"},
      {Color::Base08, "base08"}, {Color::Base09, "base09"},
      {Color::Base0A, "base0A"}, {Color::Base0B, "base0B"},
      {Color::Base0C, "base0C"}, {Color::Base0D, "base0D"},
      {Color::Base0E, "base0E"}, {Color::Base0F, "base0F"},
  };

  return palette.value(colorMap.value(color),
                       "#000000"); // fallback to black if weirdly missing
}
