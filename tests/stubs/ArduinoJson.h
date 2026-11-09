#pragma once
#include <string>
#include <vector>
struct JsonVariantConst {
  bool isNull() const { return true; }
  template<class T> bool is() const {return false;}
  template<class T> T as() const {return T{};}
  JsonVariantConst operator[](const char *) const {return {};}
};
struct JsonObjectConst: JsonVariantConst {};
struct JsonArray {};
struct JsonArrayConst {
  std::vector<JsonVariantConst> elements;
  std::vector<JsonVariantConst>::const_iterator begin() const {return elements.begin();}
  std::vector<JsonVariantConst>::const_iterator end() const {return elements.end();}
};
struct JsonDocument {
  template<class T> bool is() const {return false;}
  template<class T> T as() const {return T{};}
};
inline bool deserializeJson(JsonDocument &, const std::string &) {return true;}
