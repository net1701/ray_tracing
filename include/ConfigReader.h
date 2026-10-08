/*
 * ConfigReader.h
 *
 *  Created on: Jan 14, 2014
 *      Author: sigi
 */

#ifndef CONFIGREADER_H_
#define CONFIGREADER_H_
#include "Log.h"

#include <string>
#include <sstream>
#include <map>
#include <vector>

class ConfigReader {
  typedef std::map<std::string, std::string> ConfigMap;
  ConfigMap configMap;
public:
  ConfigReader(const std::string &filename);
//	std::string GetAsString(const std::string &key, const char* defaultValue="");
  template<typename TYPE> TYPE GetAs(const std::string &key, TYPE defaultValue);
  template<typename TYPE> std::vector<TYPE> GetAsVectorOf(
      const std::string &key);
  template<typename TYPE> void GetAsArrayOf(const std::string& key, TYPE *array,
      int n, TYPE defaultValue);
};

template<typename TYPE>
inline TYPE ConfigReader::GetAs(const std::string& key, TYPE defaultValue) {
  ConfigMap::iterator itMap = configMap.find(key);
  TYPE res;
  if (itMap != configMap.end()) {
    std::istringstream istr(itMap->second);
    istr >> res;
  } else {
    res = defaultValue;
  }
  return res;
}

template<typename TYPE>
inline std::vector<TYPE> ConfigReader::GetAsVectorOf(const std::string& key) {
  ConfigMap::iterator itMap = configMap.find(key);
  std::vector<TYPE> res;
  if (itMap != configMap.end()) {
    std::istringstream istr(itMap->second);
    TYPE x;
    while (istr >> x) {
//			LOG("*** " << key << " -> " << x);
      res.push_back(x);
    }
  }
  return res;
}

template<typename TYPE>
inline void ConfigReader::GetAsArrayOf(const std::string& key, TYPE *array,
    int n, TYPE defaultValue) {
  std::vector<TYPE> tmpVec = GetAsVectorOf<TYPE>(key);
  for (int i = 0; i < n; ++i) {
    array[i] = i < tmpVec.size() ? tmpVec[i] : defaultValue;
  }
}

#endif /* CONFIGREADER_H_ */
