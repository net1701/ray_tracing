#include "Vec3d.h"
#include "Log.h"

Vec3d::Vec3d(const std::vector<float>& std_vector) {
  v.x = std_vector.size() > 0 ? std_vector[0] : 0.0;
  v.y = std_vector.size() > 1 ? std_vector[1] : 0.0;
  v.z = std_vector.size() > 2 ? std_vector[2] : 0.0;
}

std::ostream &operator <<(std::ostream &ostr, Vec3d &vec3d) {
  ostr << "( " << vec3d.v.x << " , " << vec3d.v.y << " , " << vec3d.v.z << " )";
  return ostr;
}
