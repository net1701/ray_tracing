/*
 * Config.h
 *
 *  Created on: Jan 14, 2014
 *      Author: sigi
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "Vec3d.h"
#include "CuConfig.h"
#include "EasyBMP.h"

#include <string>
#include <inttypes.h>
#include <vector>

class Config {
   CuConfig cu_conf;
   std::string output_file;
   std::string environment_file;
   BMP environment_bmp;
public:
   Config();
   ~Config();
   void Initialize();
   void Finalize();
   void Read(const std::string &filename);
   const CuConfig *GetCuConfig() const {
      return &cu_conf;
   }
   std::string OutputFile(void) const {
      return output_file;
   }
};

#endif /* CONFIG_H_ */
