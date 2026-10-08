/*
 * Config.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: sigi
 */

#include "Log.h"
#include "Config.h"
#include "ConfigReader.h"

#include <iostream>
#include <exception>

Config::Config() {
}

Config::~Config() {
}

void Config::Initialize() {
}

void Config::Finalize() {
}

void Config::Read(const std::string& filename) {
  ConfigReader cr(filename);
  cu_conf.seed = cr.GetAs<uint64_t>("seed", 0LL);
  cu_conf.view_point = cr.GetAsVectorOf<float>("view_point");
  cu_conf.target_point = cr.GetAsVectorOf<float>("target_point");
  cu_conf.ambient_rgb = cr.GetAsVectorOf<float>("ambient_rgb");
  cu_conf.background_rgb = cr.GetAsVectorOf<float>("background_rgb");
  cu_conf.screen_dist = cr.GetAs<float>("screen_dist", 0.0);
  cr.GetAsArrayOf<float>("screen_size", cu_conf.screen_size, 2, 0.0);
  cr.GetAsArrayOf<int>("screen_pixel_size", cu_conf.screen_pixel_size, 2, 0);
  cu_conf.n_spheres = cr.GetAs<int>("n_spheres", 0);
  if (cu_conf.n_spheres > MAX_SHARED_SPHERES) {
    LOG("ERROR: n_spheres > MAX_SHARED_SPHERES (" << MAX_SHARED_SPHERES << ")");
    throw std::exception();
  }
  cu_conf.n_recursion = cr.GetAs<int>("n_recursion", 5);
  cu_conf.rgb_min = cr.GetAs<float>("rgb_min", 0.0);
  cu_conf.reflectivity_rgb = cr.GetAsVectorOf<float>("reflectivity_rgb");
  cu_conf.bbox_min = cr.GetAsVectorOf<float>("bbox_min");
  cu_conf.bbox_max = cr.GetAsVectorOf<float>("bbox_max");
  cu_conf.bbox_lights_min = cr.GetAsVectorOf<float>("bbox_lights_min");
  cu_conf.bbox_lights_max = cr.GetAsVectorOf<float>("bbox_lights_max");
  cu_conf.light_intensity = cr.GetAs<float>("light_intensity", 1.0);
  cu_conf.radius_min = cr.GetAs<float>("radius_min", 1.0);
  cu_conf.radius_max = cr.GetAs<float>("radius_max", 1.0);
  cu_conf.n_lights = cr.GetAs<int>("n_lights", 0);
  if (cu_conf.n_lights > MAX_SHARED_LIGHTS) {
    LOG("ERROR: n_lights > MAX_SHARED_LIGHTS (" << MAX_SHARED_LIGHTS << ")");
    throw std::exception();
  }
  output_file = cr.GetAs<std::string>("output_file", "output.bmp");
  environment_file = cr.GetAs<std::string>("environment_file",
      "environment.bmp");
  LOG("--- CONFIG LOG BEGIN ---");
  LOG("seed                = " << cu_conf.seed);
  LOG("view_point          = " << cu_conf.view_point);
  LOG("target_point        = " << cu_conf.target_point);
  LOG("ambient_rgb         = " << cu_conf.ambient_rgb);
  LOG("screen_dist         = " << cu_conf.screen_dist);
  LOG(
      "screen_size         = ( " << cu_conf.screen_size[0] << " , " << cu_conf.screen_size[1] << " )");
  LOG(
      "screen_pixel_size   = ( " << cu_conf.screen_pixel_size[0] << " , " << cu_conf.screen_pixel_size[1] << " )");
  LOG("n_spheres           = " << cu_conf.n_spheres);
  LOG("n_recursion         = " << cu_conf.n_recursion);
  LOG("rgb_min             = " << cu_conf.rgb_min);
  LOG("reflectivity_rgb    = " << cu_conf.reflectivity_rgb);
  LOG("bbox_min            = " << cu_conf.bbox_min);
  LOG("bbox_max            = " << cu_conf.bbox_max);
  LOG("bbox_lights_min     = " << cu_conf.bbox_lights_min);
  LOG("bbox_lights_max     = " << cu_conf.bbox_lights_max);
  LOG("light_intensity     = " << cu_conf.light_intensity);
  LOG("radius_min          = " << cu_conf.radius_min);
  LOG("radius_max          = " << cu_conf.radius_max);
  LOG("n_lights            = " << cu_conf.n_lights);
  LOG("output_file         = " << output_file);
  LOG("environment_file    = " << environment_file);
  LOG("---- CONFIG LOG END ----");

  if (environment_file.size() > 0) {
    environment_bmp.ReadFromFile(environment_file.c_str());
  }
}

