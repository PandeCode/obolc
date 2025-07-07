#include "utils.hpp"

#include <cinttypes>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>
#include <stdexcept>
#include <string>

#include <unistd.h>

bool isWayland() {
  static const bool _isWayland = [] {
    bool ret = qgetenv("XDG_SESSION_TYPE") == "wayland" ||
               !qgetenv("WAYLAND_DISPLAY").isEmpty();
    if (ret)
      std::println("Running on Wayland");
    return ret;
  }();
  return _isWayland;
}

static std::filesystem::path getExecutableDir() {
  static std::filesystem::path _getExecutableDir = [] {
    char buffer[4096];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len == -1)
      throw std::runtime_error("Failed to get executable path");
    buffer[len] = '\0';
    return std::filesystem::path(buffer).parent_path();
  }();
  return _getExecutableDir;
}

std::filesystem::path getAssetFile(const std::string &filename) {
  // 1. ~/.config/obolc/assets
  auto home = std::getenv("HOME");
  if (home) {
    auto userPath =
        std::filesystem::path(home) / ".config" / "obolc" / "assets" / filename;
    if (std::filesystem::exists(userPath))
      return userPath;
  }

  // 2. $out/share/obolc/assets (if installed via Nix)
  auto execDir = getExecutableDir();
  auto systemPath =
      execDir.parent_path() / "share" / "obolc" / "assets" / filename;
  if (std::filesystem::exists(systemPath))
    return systemPath;

  // 3. Local dev: next to binary
  auto localPath = execDir / "assets" / filename;
  if (std::filesystem::exists(localPath))
    return localPath;

  throw std::runtime_error("Asset not found: " + filename);
}

std::string readFile(const std::string &filePath) {
  std::ifstream file(filePath);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file file: " + filePath);
  }
  std::ostringstream contents;
  contents << file.rdbuf();
  return contents.str();
}

std::string readAssetFile(const std::string &filename) {
  auto assetPath = getAssetFile(filename);
  return readFile(assetPath);
}

QString exec(const QString &pname) {
  QProcess process;
  process.start(pname);
  process.waitForFinished();
  return process.readAllStandardOutput();
}

QString exec(const QString &pname, const QStringList &params) {
  QProcess process;
  process.start(pname, params);
  process.waitForFinished();
  return process.readAllStandardOutput();
}

QString cleanTitle(QString title) {
  title.replace(QRegularExpression(R"( — Zen Twilight)"),
                "");                                   // " — Zen Twilight"
  title.replace(QRegularExpression(R"( - Nvim)"), ""); // " - Nvim"
  title.replace(QRegularExpression(R"(Zellij\s*\(.*\)\s*-\s*)"),
                ""); // "Zellij (...) - "
  title = title.trimmed();
  return title;
}
