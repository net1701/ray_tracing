/*
 * Scene.cpp
 *
 *  Created on: Jan 18, 2014
 *      Author: sigi
 */

#include "Scene.h"
#include "Log.h"
#include "EasyBMP.h"

#include <cuda.h>
#include <cassert>

// texture<float, cudaTextureType2D, cudaReadModeElementType> texEnvRef;

Scene::Scene(const CuConfig *cuConfig) {
  h_cuConfig = cuConfig;
  CUDACHECK(cudaHostAlloc(&h_scene, sizeof(GpuScene), cudaHostAllocDefault));
  CUDACHECK(cudaMalloc(&d_scene, sizeof(GpuScene)));
  h_scene->cuConfig = 0;
  h_scene->spheres = 0;
  h_scene->lights = 0;
  h_scene->ibitmap = 0;
  h_scene->envArray = 0;

  CUDACHECK(cudaMalloc(&h_scene->cuConfig, sizeof(CuConfig)));
  CUDACHECK(cudaMalloc(&h_scene->spheres, cuConfig->n_spheres * sizeof(Sphere)));
  CUDACHECK(cudaMalloc(&h_scene->lights, cuConfig->n_lights * sizeof(Light)));

  int bitmap_size = cuConfig->screen_pixel_size[0]
      * cuConfig->screen_pixel_size[1];
  CUDACHECK(cudaMalloc(&h_scene->ibitmap, bitmap_size * sizeof(uchar4)));
  CUDACHECK(
      cudaHostAlloc(&h_outputBitmap, bitmap_size * sizeof(uchar4),
          cudaHostAllocDefault));

  CUDACHECK(
      cudaMemcpy(h_scene->cuConfig, cuConfig, sizeof(CuConfig),
          cudaMemcpyHostToDevice));
  CUDACHECK(
      cudaMemcpy(d_scene, h_scene, sizeof(GpuScene), cudaMemcpyHostToDevice));

//  cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(32, 0, 0, 0,
//     cudaChannelFormatKindFloat);
//  CUDACHECK( cudaMallocArray(&envArray, &texEnvRef.channelDesc, width, height) );
//  CUDACHECK( cudaMemcpyToArray(cuArray, 0, 0, hData, size, cudaMemcpyHostToDevice));
}

Scene::~Scene() {
  CUDACHECK(cudaFreeHost(h_outputBitmap));
  h_outputBitmap = 0;

  CUDAFREE_AND_CHECK(h_scene->ibitmap);
  CUDAFREE_AND_CHECK(h_scene->lights);
  CUDAFREE_AND_CHECK(h_scene->spheres);
  CUDAFREE_AND_CHECK(h_scene->cuConfig);

  CUDAFREE_AND_CHECK(d_scene);
  d_scene = 0;

  CUDACHECK(cudaFreeHost(h_scene));
  h_scene = 0;
}

__global__ void InitializeRandomSceneK(GpuScene *s) {
  s->InitializeRandom();
}

void Scene::InitializeRandomScene(void) {
  int threads_per_block = 32;
  int n_blocks = (h_cuConfig->n_spheres + threads_per_block - 1)
      / threads_per_block;

  cudaEvent_t start, stop;
  float elapsedTime;

  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start, 0);

  CUDACHECK(cudaGetLastError());
  InitializeRandomSceneK<<<threads_per_block, n_blocks>>>(d_scene);
  CUDACHECK(cudaGetLastError());

  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  LOG("INITIALIZATION TIME: " << elapsedTime << " ms");
}

__global__ void DoRenderK(GpuScene *s) {
  s->DoRender();
}

void Scene::DoRender(void) {
  dim3 threads_per_block;
  threads_per_block.x = 16; // at least 10...
  threads_per_block.y = 8;
  threads_per_block.z = 1;

  assert(threads_per_block.x >= MAX_SHARED_SPHERES);

  dim3 n_blocks;
  n_blocks.x = (h_cuConfig->screen_pixel_size[0] + threads_per_block.x - 1)
      / threads_per_block.x;
  n_blocks.y = (h_cuConfig->screen_pixel_size[1] + threads_per_block.y - 1)
      / threads_per_block.y;
  n_blocks.z = 1;
  LOG(
      "threads_per_block: " << threads_per_block.x << ", " << threads_per_block.y << ", " << threads_per_block.z);
  LOG(
      "           blocks: " << n_blocks.x << ", " << n_blocks.y << ", " << n_blocks.z);

  cudaEvent_t start, stop;
  float elapsedTime;

  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start, 0);

  CUDACHECK(cudaGetLastError());
  DoRenderK<<<n_blocks, threads_per_block>>>(d_scene);
  CUDACHECK(cudaGetLastError());

  cudaEventRecord(stop, 0);
  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&elapsedTime, start, stop);
  LOG("RAY-TRACING TIME: " << elapsedTime << " ms");
}

void Scene::SaveOutput(const std::string &filename) {
  int bitmap_size = h_cuConfig->screen_pixel_size[0]
      * h_cuConfig->screen_pixel_size[1];
  CUDACHECK(
      cudaMemcpy(h_outputBitmap, h_scene->ibitmap, bitmap_size * sizeof(uchar4),
          cudaMemcpyDeviceToHost));

  BMP save_bmp;
  save_bmp.SetBitDepth(24);
  save_bmp.SetSize(h_cuConfig->screen_pixel_size[0],
      h_cuConfig->screen_pixel_size[1]);

  for (int iy = 0; iy < h_cuConfig->screen_pixel_size[1]; ++iy) {
    for (int ix = 0; ix < h_cuConfig->screen_pixel_size[0]; ++ix) {
      RGBApixel pixel;
      pixel.Red = h_outputBitmap[ix + iy * h_cuConfig->screen_pixel_size[0]].x;
      pixel.Green =
          h_outputBitmap[ix + iy * h_cuConfig->screen_pixel_size[0]].y;
      pixel.Blue = h_outputBitmap[ix + iy * h_cuConfig->screen_pixel_size[0]].z;
      save_bmp.SetPixel(ix, iy, pixel);
    }
  }

  save_bmp.WriteToFile(filename.c_str());
}
