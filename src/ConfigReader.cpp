/*
 * ConfigReader.cpp
 *
 *  Created on: Jan 14, 2014
 *      Author: sigi
 */

#include "ConfigReader.h"
#include "Log.h"
#include <fstream>

using namespace std;

string trim(const string &str) {
  int i1 = 0;
  for (; i1 < str.size(); ++i1) {
    if (str[i1] != ' ' && str[i1] != '\t') {
      break;
    }
  }

  int i2 = str.size() - 1;
  for (; i2 > i1; --i2) {
    if (str[i2] != ' ' && str[i2] != '\t') {
      break;
    }
  }
  ++i2;

  string res;
  if (i1 == i2) {
    res = "";
  } else {
    res = str.substr(i1, i2 - i1);
  }

  return res;
}

ConfigReader::ConfigReader(const std::string &filename) {
  LOG("reading configuration file: \"" << filename << "\"");
  ifstream ifs(filename.c_str());
  string line;
  int lineno = 0;
  while (getline(ifs, line)) {
    ++lineno;

    size_t hashPos = line.find('#');
    if (hashPos != line.npos) {
      line = line.substr(0, hashPos);
    }

    size_t eqPos = line.find('=');
    if (eqPos != line.npos) {
      string key = trim(line.substr(0, eqPos));
      string val = trim(eqPos + 1 < line.size() ? line.substr(eqPos + 1) : "");
      configMap[key] = val;
//			LOG("option OK: key = \"" << key << "\" val = \"" << val << "\"");
    } else {
      for (int i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c != ' ' && c != '\t') {
          LOG("ERROR: unexpected text in configuration, line: " << lineno);
          goto nextline;
        }
      }
    }
    nextline: ;
  }
}

//std::string ConfigReader::GetAsString(const std::string& key, const char *defaultValue) {
//	auto itMap = configMap.find(key);
//	string res;
//	if (itMap != configMap.end()) {
//		res = itMap->second;
//	} else {
//		res = defaultValue;
//	}
//	return res;
//}
//
