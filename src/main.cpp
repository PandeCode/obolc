#include "panel.hpp"
#include <LayerShellQt/Shell>
#include <LayerShellQt/window.h>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  // LayerShellQt::Shell::useLayerShell();

  Panel panel;
  panel.show();

  return app.exec();
}
