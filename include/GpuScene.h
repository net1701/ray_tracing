/*
 * GpuScene.h
 *
 *  Created on: Jan 27, 2014
 *      Author: sigi
 */

#ifndef GPUSCENE_H_
#define GPUSCENE_H_
#include "curand_kernel.h"
#include "Log.h"
#include <stdio.h>

// closest intercept data
struct Intersection {
  int i_sph;
  float dSQ;
  Vec3d x;
  Vec3d norm;
  Vec3d refl;
};

struct GpuScene {
  CuConfig *cuConfig;
  Sphere *spheres;
  Light *lights;
  // final output bitmap
  uchar4 *ibitmap;
  // environment texture array
  cudaArray* envArray;
  // evaluated on the GPU:

  // versors
  Vec3d lx_versor;
  Vec3d ly_versor;
  // the origin of the screen, that is the 3D coord of the upper-left corner:
  Vec3d screen_origin;
  float x_pixel_pitch;
  float y_pixel_pitch;

  __device__ void InitializeRandom(void);
  __device__ void EvalIntersection(Vec3d ray, Vec3d origin, int i_ignore,
                                   Sphere *spheres_array, Intersection &intersection);
  __device__ void DoRender(void);
  __device__ void Test(void);
  __device__ void EvalLightening(Intersection &intersection,  Sphere *spheres_array,
                                 Light *lights_array, Vec3d &rgb);
};

__device__ unsigned int spheres_count = 0;

__device__ inline void GpuScene::InitializeRandom(void) {
  int id = threadIdx.x + blockIdx.x * blockDim.x;

  curandState state;
  curand_init(cuConfig->seed, id, 0, &state);
  if (id < cuConfig->n_spheres) {
    spheres[id].radius = curand_uniform(&state)
        * (cuConfig->radius_max - cuConfig->radius_min) + cuConfig->radius_min;
    spheres[id].center.v.x = curand_uniform(&state)
        * (cuConfig->bbox_max.v.x - cuConfig->bbox_min.v.x)
        + cuConfig->bbox_min.v.x;
    spheres[id].center.v.y = curand_uniform(&state)
        * (cuConfig->bbox_max.v.y - cuConfig->bbox_min.v.y)
        + cuConfig->bbox_min.v.y;
    spheres[id].center.v.z = curand_uniform(&state)
        * (cuConfig->bbox_max.v.z - cuConfig->bbox_min.v.z)
        + cuConfig->bbox_min.v.z;
#ifdef SPHERES_SIMPLE_COLORS
    spheres[id].color.v.x = (id%3 == 0) ? cuConfig->rgb_min : 0.0;
    spheres[id].color.v.y = (id%3 == 1) ? cuConfig->rgb_min : 0.0;
    spheres[id].color.v.z = (id%3 == 2) ? cuConfig->rgb_min : 0.0;
#elif SPHERES_SATURATE_COLORS
    spheres[id].color.v.x = curand_uniform(&state) > 0.5 ? 1.0 : cuConfig->rgb_min;
    spheres[id].color.v.y = curand_uniform(&state) > 0.5 ? 1.0 : cuConfig->rgb_min;
    spheres[id].color.v.z = curand_uniform(&state) > 0.5 ? 1.0 : cuConfig->rgb_min;
#else
    spheres[id].color.v.x = cuConfig->rgb_min
        + (1.0 - cuConfig->rgb_min) * curand_uniform(&state);
    spheres[id].color.v.y = cuConfig->rgb_min
        + (1.0 - cuConfig->rgb_min) * curand_uniform(&state);
    spheres[id].color.v.z = cuConfig->rgb_min
        + (1.0 - cuConfig->rgb_min) * curand_uniform(&state);
#endif
    spheres[id].reflectivity.v.x = cuConfig->reflectivity_rgb.v.x;
    spheres[id].reflectivity.v.y = cuConfig->reflectivity_rgb.v.y;
    spheres[id].reflectivity.v.z = cuConfig->reflectivity_rgb.v.z;

    __threadfence();

    unsigned int old_count = atomicInc(&spheres_count, cuConfig->n_spheres);
    if (old_count == cuConfig->n_spheres - 1) {
      // avoid overlapping:
      // the last thread that completes its job move overlapping spheres sequentially
      for (int i = 1; i < cuConfig->n_spheres; ++i) {
        // try to move at most 100 times - then give up: it's possible there is
        // not room at all for all the spheres!
        for (int k = 0; k < 100; ++k) {
          bool overlapping = false;
          float dmin;
          int j;
          for (j = 0; j < i; ++j) {
            dmin = spheres[i].radius + spheres[j].radius;
            if ((spheres[i].center - spheres[j].center).NormSQ()
                < dmin * dmin) {
              overlapping = true;
              break;
            }
          }
          if (overlapping) {
            spheres[i].center.v.x = curand_uniform(&state)
                * (cuConfig->bbox_max.v.x - cuConfig->bbox_min.v.x)
                + cuConfig->bbox_min.v.x;
            spheres[i].center.v.y = curand_uniform(&state)
                * (cuConfig->bbox_max.v.y - cuConfig->bbox_min.v.y)
                + cuConfig->bbox_min.v.y;
            spheres[i].center.v.z = curand_uniform(&state)
                * (cuConfig->bbox_max.v.z - cuConfig->bbox_min.v.z)
                + cuConfig->bbox_min.v.z;
          } else {
            break;
          }
        }
      }
      __threadfence();
    }
    cudaPrintf("sphere %d:  %f %f %f   %f %f %f  %f\n", id,
        spheres[id].center.v.x, spheres[id].center.v.y, spheres[id].center.v.z,
        spheres[id].reflectivity.v.x, spheres[id].reflectivity.v.y, spheres[id].reflectivity.v.z,
        spheres[id].radius);
  }

  if (id < cuConfig->n_lights) {
    lights[id].position.v.x = curand_uniform(&state)
        * (cuConfig->bbox_lights_max.v.x - cuConfig->bbox_lights_min.v.x)
        + cuConfig->bbox_lights_min.v.x;
    lights[id].position.v.y = curand_uniform(&state)
        * (cuConfig->bbox_lights_max.v.y - cuConfig->bbox_lights_min.v.y)
        + cuConfig->bbox_lights_min.v.y;
    lights[id].position.v.z = curand_uniform(&state)
        * (cuConfig->bbox_lights_max.v.z - cuConfig->bbox_lights_min.v.z)
        + cuConfig->bbox_lights_min.v.z;
    // white lights
    lights[id].color.v.x = cuConfig->light_intensity;
    lights[id].color.v.y = cuConfig->light_intensity;
    lights[id].color.v.z = cuConfig->light_intensity;
    cudaPrintf("light:  %f %f %f\n", lights[id].position.v.x, lights[id].position.v.y, lights[id].position.v.z);
  }
  if (id == 0) {
//    // test swap
//    __threadfence();
//    Sphere tmp = spheres[0];
//    spheres[0] = spheres[1];
//    spheres[1] = tmp;

    for (int i=0; i<cuConfig->n_spheres; ++i) {
    cudaPrintf("xx sphere %d:  %f %f %f   %f %f %f  %f %f %f  %f\n", i,
        spheres[i].center.v.x, spheres[i].center.v.y, spheres[i].center.v.z,
        spheres[i].color.v.x, spheres[i].color.v.y, spheres[i].color.v.z,
        spheres[i].reflectivity.v.x, spheres[i].reflectivity.v.y, spheres[i].reflectivity.v.z,
        spheres[i].radius);
    }

    Vec3d screen_versor = (cuConfig->target_point - cuConfig->view_point);
    screen_versor = screen_versor / screen_versor.Norm();
    // gram schmidt to find vertical axis.
    // note: screen versor aligned with z axis is not supported here (division by 0)
    float projection = screen_versor * Vec3d(0.0, 0.0, 1.0);
    ly_versor = projection * screen_versor - Vec3d(0.0, 0.0, 1.0);
    ly_versor = ly_versor / ly_versor.Norm();

    // find the x screen versor with a cross product:
    lx_versor = CrossProd(ly_versor, screen_versor);

    // the position of the center of the screen, evaluated from the distance from the
    // view point:
    Vec3d screen_center = cuConfig->view_point
        + cuConfig->screen_dist * screen_versor;
    screen_origin = screen_center - 0.5 * cuConfig->screen_size[0] * lx_versor
        - 0.5 * cuConfig->screen_size[1] * ly_versor;

    x_pixel_pitch = cuConfig->screen_size[0] / cuConfig->screen_pixel_size[0];
    y_pixel_pitch = cuConfig->screen_size[1] / cuConfig->screen_pixel_size[1];
    // finally, ready to draw the rays!
    cudaPrintf("screen center: %7.3f %7.3f %7.3f\n", screen_center.v.x, screen_center.v.y, screen_center.v.z);cudaPrintf("screen origin: %7.3f %7.3f %7.3f\n", screen_origin.v.x, screen_origin.v.y, screen_origin.v.z);cudaPrintf("screen_versor: %7.3f %7.3f %7.3f\n", screen_versor.v.x, screen_versor.v.y, screen_versor.v.z);cudaPrintf("lx_versor:     %7.3f %7.3f %7.3f\n", lx_versor.v.x, lx_versor.v.y, lx_versor.v.z);cudaPrintf("ly_versor:     %7.3f %7.3f %7.3f\n", ly_versor.v.x, ly_versor.v.y, ly_versor.v.z);

  }
}

__device__ inline void GpuScene::Test(void) {
  // coordinate origin: the up-left corner
  int ix = threadIdx.x + blockIdx.x * blockDim.x;
  int iy = threadIdx.y + blockIdx.y * blockDim.y;
  ibitmap[ix + iy * cuConfig->screen_pixel_size[0]].x = ix % 256;
  ibitmap[ix + iy * cuConfig->screen_pixel_size[0]].y = iy % 256;
  ibitmap[ix + iy * cuConfig->screen_pixel_size[0]].z = 0;
}

__device__ inline
void GpuScene::EvalIntersection(Vec3d ray, Vec3d origin,
    int i_ignore, Sphere *spheres_array, Intersection &intersection) {
  intersection.i_sph = -1;

  for (int i_sph = 0; i_sph < cuConfig->n_spheres; ++i_sph) {
// this is bad - better lo leave it out.
//    if (i_sph == i_ignore) {
//      continue;
//    }
    Vec3d c_o = spheres_array[i_sph].center - origin;
    // <d> = <c> - <c>*<ray> / (<ray>*<ray>) * <ray>
    // distance^2, ray <-> sphere:
    float dSQ_sph = (c_o - (c_o * ray) / ray.NormSQ() * ray).NormSQ();

    if (dSQ_sph < spheres_array[i_sph].radius * spheres_array[i_sph].radius) {
      // TODO: there is intercept, so store the evaluation of this pixel
      // for the "intercept scan step", to avoid divergence

      // evaluate ray-sphere intersections.
      // ray parametric in "t": <x> = <view_point> + <ray> * t
      // |<ray> * t  - <c>|^2 = radius^2
      // |<ray>|^2 * t^2 - 2 * t * <c>*<ray> + |<c>|^2 - radius^2 = 0
      // solving for t:
      float a = ray.NormSQ();
      float b = -2. * c_o * ray;
      float c = c_o.NormSQ() - spheres_array[i_sph].radius * spheres_array[i_sph].radius;
      // this could be evaluated and used outside in place of dSQ: heavier but done just once
      float det = b * b - 4. * a * c;
      // interested in the lowest value of t only:
      float t = (-b - sqrt(det)) / (2. * a);
      Vec3d x = origin + t * ray;
      float dSQ_intercection = (x-origin).NormSQ();
      if (t > 0.0 && (intersection.i_sph < 0 || dSQ_intercection < intersection.dSQ)) {
        intersection.i_sph = i_sph;
        intersection.dSQ = dSQ_intercection;
        // coordinates of the intersection:
        intersection.x = x;
        // to evaluate both the specular reflection and the Lambert's law, finally.
        // normal (not normalized) in this point:
        intersection.norm = intersection.x - spheres_array[i_sph].center;
        // reflected ray:
        // <refl> = <ray> - 2 * <ray> * <norm> / |<norm>|^2 * <norm>
        intersection.refl = ray
            - 2. * (ray * intersection.norm) / intersection.norm.NormSQ()
                * intersection.norm;
      }
    }
  }
}

__device__ inline
void GpuScene::EvalLightening(Intersection &intersection, Sphere *spheres_array, Light *lights_array, Vec3d &rgb) {
  if (intersection.i_sph >= 0) {
    rgb.v.x = cuConfig->ambient_rgb.v.x * spheres_array[intersection.i_sph].color.v.x;
    rgb.v.y = cuConfig->ambient_rgb.v.y * spheres_array[intersection.i_sph].color.v.y;
    rgb.v.z = cuConfig->ambient_rgb.v.z * spheres_array[intersection.i_sph].color.v.z;

    for (int i_lgh = 0; i_lgh < cuConfig->n_lights; ++i_lgh) {
      // light-intersection ray.
      Vec3d ray = intersection.x - lights_array[i_lgh].position;

      // check for shadows
      bool shadowed = false;
      for (int i_sph = 0; i_sph < cuConfig->n_spheres; ++i_sph) {
        Vec3d c_l = spheres_array[i_sph].center - lights_array[i_lgh].position;
        float dSQ = (c_l - (c_l * ray) / ray.NormSQ() * ray).NormSQ();
        if (dSQ < spheres_array[i_sph].radius * spheres_array[i_sph].radius) {
          // there is a shadow.
          // check that it's between the light and the intersection.
          float p = c_l * ray;
          float pnormSQ = p * p / ray.NormSQ();
          if (p > 0 && pnormSQ < ray.NormSQ()) {
            shadowed = true;
            break;
          }
        }
      }
      if (!shadowed) {
        // apply Lambert law for this light.
        float lambert = -(intersection.norm * ray)
            / (intersection.norm.Norm() * ray.Norm());
        if (lambert < 0.0) {
          lambert = 0.0;
        }

        // distance ignored - global lightening balance step needed afterwards otherwise
        // TODO: add intensity-distance relationship
        rgb.v.x += lambert * spheres_array[intersection.i_sph].color.v.x
            * lights_array[i_lgh].color.v.x
            * (1.0 - spheres_array[intersection.i_sph].reflectivity.v.x);
        rgb.v.y += lambert * spheres_array[intersection.i_sph].color.v.y
            * lights_array[i_lgh].color.v.y
            * (1.0 - spheres_array[intersection.i_sph].reflectivity.v.y);
        rgb.v.z += lambert * spheres_array[intersection.i_sph].color.v.z
            * lights_array[i_lgh].color.v.z
            * (1.0 - spheres_array[intersection.i_sph].reflectivity.v.z);
      }
    }
  } else {
    // get the environment background
    // TODO: use the environment texture if present
    rgb = cuConfig->background_rgb;
  }
}

__device__ inline void GpuScene::DoRender(void) {
  // OPTIMIZATION: caching the geometry in the shared memory improved the execution
  // time of the 60%.
  __shared__ Sphere sh_spheres[MAX_SHARED_SPHERES];
  __shared__ Light  sh_lights[MAX_SHARED_LIGHTS];

  // coordinate origin: the up-left corner
  int ix = threadIdx.x + blockIdx.x * blockDim.x;
  int iy = threadIdx.y + blockIdx.y * blockDim.y;

  if (threadIdx.x < MAX_SHARED_SPHERES) {
      sh_spheres[threadIdx.x] = spheres[threadIdx.x];
  }
  if (threadIdx.x < MAX_SHARED_LIGHTS) {
    sh_lights[threadIdx.x] = lights[threadIdx.x];
  }

  __syncthreads();

  Vec3d screen_point = screen_origin + ix * x_pixel_pitch * lx_versor
      + iy * y_pixel_pitch * ly_versor;

  Vec3d ray = screen_point - cuConfig->view_point;

  Intersection intersection;
  EvalIntersection(ray, cuConfig->view_point, -1, sh_spheres, intersection);
  Vec3d rgb(0.0, 0.0, 0.0); // result
  EvalLightening(intersection, sh_spheres, sh_lights, rgb);

  Vec3d pixel_rgb = rgb;  // global result
  Vec3d refl_cur(1.0, 1.0, 1.0);

  for (int i_refl = 0;
      i_refl < cuConfig->n_recursion && intersection.i_sph >= 0;
      ++i_refl)
  {
    Intersection intersection2;
    EvalIntersection(intersection.refl-intersection.x, intersection.x,
        intersection.i_sph, sh_spheres, intersection2);
    Vec3d rgb2;
    EvalLightening(intersection2, sh_spheres, sh_lights, rgb2);
    refl_cur.v.x *= sh_spheres[intersection.i_sph].reflectivity.v.x;
    refl_cur.v.y *= sh_spheres[intersection.i_sph].reflectivity.v.y;
    refl_cur.v.z *= sh_spheres[intersection.i_sph].reflectivity.v.z;
    pixel_rgb.v.x += refl_cur.v.x*rgb2.v.x;
    pixel_rgb.v.y += refl_cur.v.y*rgb2.v.y;
    pixel_rgb.v.z += refl_cur.v.z*rgb2.v.z;
    intersection = intersection2;
  }
//   pixel_rgb = pixel_rgb / (cuConfig->n_lights);
  if (pixel_rgb.v.x < 0.0)
    pixel_rgb.v.x = 0.0;
  if (pixel_rgb.v.y < 0.0)
    pixel_rgb.v.y = 0.0;
  if (pixel_rgb.v.z < 0.0)
    pixel_rgb.v.z = 0.0;
  if (pixel_rgb.v.x > 1.0)
    pixel_rgb.v.x = 1.0;
  if (pixel_rgb.v.y > 1.0)
    pixel_rgb.v.y = 1.0;
  if (pixel_rgb.v.z > 1.0)
    pixel_rgb.v.z = 1.0;

  ibitmap[ix + iy * cuConfig->screen_pixel_size[0]].x = (unsigned char) (255
      * pixel_rgb.v.x);
  ibitmap[ix + iy * cuConfig->screen_pixel_size[0]].y = (unsigned char) (255
      * pixel_rgb.v.y);
  ibitmap[ix + iy * cuConfig->screen_pixel_size[0]].z = (unsigned char) (255
      * pixel_rgb.v.z);
}

#endif /* GPUSCENE_H_ */
