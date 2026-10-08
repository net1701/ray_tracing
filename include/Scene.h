/*
 * Scene.h
 *
 *  Created on: Jan 18, 2014
 *      Author: sigi
 */

#ifndef SCENE_H_
#define SCENE_H_

#include "CuConfig.h"
#include "Sphere.h"
#include "Light.h"
#include "GpuScene.h"

class Scene {
  const CuConfig *h_cuConfig;

  GpuScene *h_scene;
  GpuScene *d_scene;
  uchar4 *h_outputBitmap;
  Scene(const Scene &) {
  }
  void operator =(const Scene &) {
  }
public:
  Scene(const CuConfig *cuConfig);
  ~Scene();
  void InitializeRandomScene(void);
  void DoRender(void);
  void SaveOutput(const std::string &filename);
};

#endif /* SCENE_H_ */

