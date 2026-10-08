#include "Log.h"
#include <iostream>
#include "Config.h"
#include "Scene.h"

using namespace std;

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "USAGE: ray_tracing <file.dat>" << endl;
    return 1;
  }

  Config cfg;
  cfg.Read(argv[1]);

  Scene scene(cfg.GetCuConfig());
  scene.InitializeRandomScene();
  scene.DoRender();
  scene.SaveOutput(cfg.OutputFile());
  LOG("ALL DONE.");
}
