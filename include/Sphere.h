/*
 * Sphere.h
 *
 *  Created on: Jan 18, 2014
 *      Author: sigi
 */

#ifndef SPHERE_H_
#define SPHERE_H_

#include "Vec3d.h"

// start with an easy data placement, and parallelization over the pixel of the final image
// each thread evaluate one ray throw the screen.
// TODO - not sure that I have enough time.
// evaluate optimizations:
// - using float4
// - using alignment directives
// - with different data placement (that is, x[n], y[n], z[n]... this can be useful
//   if evaluating different objects intersections on different threads).
// - using YUV or other color spaces
struct Sphere {
  Vec3d center;
  float radius;
  Vec3d color;        // RGB
  Vec3d reflectivity; // RGB, specular reflectivity factors
};

#endif /* SPHERE_H_ */
