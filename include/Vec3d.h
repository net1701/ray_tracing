/*
 * Vec3d.h
 *
 *  Created on: Jan 15, 2014
 *      Author: sigi
 */

#ifndef VEC3D_H_
#define VEC3D_H_

#include <vector>
#include <iostream>
#include <cmath>

struct Vec3d {
  // OPTIMIZATION: changing float3 to float4 here the kernel time is 30% less.
  // this datatype is used for both all the vectors, positions and the RGB colors.
  float4 v;
  __host__ Vec3d(const std::vector<float> &std_vector);
  __device__ __host__ Vec3d() {
  }

  __device__ __host__ Vec3d(float x, float y, float z) {
    v.x = x;
    v.y = y;
    v.z = z;
  }

  __device__ __host__ void Reset(void) {
    v.x = 0.0;
    v.y = 0.0;
    v.z = 0.0;
  }

  __device__ __host__ float NormSQ(void) const {
    return v.x * v.x + v.y * v.y + v.z * v.z;
  }
  __device__ __host__ float Norm(void) const {
    return std::sqrt(NormSQ());
  }
  __device__ __host__ float operator *(Vec3d o) {
    return v.x * o.v.x + v.y * o.v.y + v.z * o.v.z;
  }
  __device__   __host__ Vec3d operator -(Vec3d o) {
    Vec3d r;
    r.v.x = v.x - o.v.x;
    r.v.y = v.y - o.v.y;
    r.v.z = v.z - o.v.z;
    return r;
  }
  __device__   __host__ Vec3d operator +(Vec3d o) {
    Vec3d r;
    r.v.x = v.x + o.v.x;
    r.v.y = v.y + o.v.y;
    r.v.z = v.z + o.v.z;
    return r;
  }
  __device__   __host__ Vec3d operator /(float f) {
    Vec3d r;
    r.v.x = v.x / f;
    r.v.y = v.y / f;
    r.v.z = v.z / f;
    return r;
  }
};

__device__   __host__   inline Vec3d operator *(float f, Vec3d o) {
  Vec3d r;
  r.v.x = f * o.v.x;
  r.v.y = f * o.v.y;
  r.v.z = f * o.v.z;
  return r;
}

__device__   __host__   inline Vec3d CrossProd(Vec3d a, Vec3d b) {
  Vec3d r;
  r.v.x = a.v.y * b.v.z - a.v.z * b.v.y;
  r.v.y = a.v.z * b.v.x - a.v.x * b.v.z;
  r.v.z = a.v.x * b.v.y - a.v.y * b.v.x;
  return r;
}

std::ostream &operator <<(std::ostream &ostr, Vec3d &vec3d);

#endif /* VEC3D_H_ */
