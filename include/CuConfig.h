/*
 * CuConfig.h
 *
 *  Created on: Jan 18, 2014
 *      Author: sigi
 */

#ifndef CUCONFIG_H_
#define CUCONFIG_H_

#ifndef MAX_SHARED_SPHERES
#define MAX_SHARED_SPHERES 10
#endif

#ifndef MAX_SHARED_LIGHTS
#define MAX_SHARED_LIGHTS 2
#endif

#include "Vec3d.h"
#include <inttypes.h>

// Cuda ray tracer configuration - parameters to be sent to the GPU
// OPTIMIATION: 10% is gained properly ordering this structure.
struct CuConfig {
   Vec3d view_point;
   Vec3d target_point;
   Vec3d ambient_rgb;
   Vec3d background_rgb; // if no environment BMP is present
   Vec3d reflectivity_rgb;
   Vec3d bbox_min;
   Vec3d bbox_max;
   Vec3d bbox_lights_min;
   Vec3d bbox_lights_max;
   uint64_t seed;
   // distance view_point <-> screen
   float screen_dist;
   // screen "real world" 2D size
   float screen_size[2];
   float rgb_min;
   float light_intensity;
   float radius_min;
   float radius_max;
   // screen pixel 2D size
   int screen_pixel_size[2];
   int n_spheres;
   int n_recursion;
   int n_lights;
};

#endif /* CUCONFIG_H_ */
