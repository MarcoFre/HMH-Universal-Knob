#pragma once
// HackMyHome Universal Knob v0.1.3 — data model and UI state machine.
// Based on the hardware map of Power Control 0.27.3 / ONX2424G013.
// No secrets, fixed HA entity IDs or automatic actuation in this file.
#include "esphome.h"
#include <ArduinoJson.h>
#include <lvgl.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

class HMHUniversalKnob {
 public:
  struct Device {
    std::string entity, name, domain, hvac_modes;
    int features = 0;
    bool dim = false, color = false, kelvin = false;
    float min_temp = 7, max_temp = 35, temp_step = 0.5f;
    float min_kelvin = 2000, max_kelvin = 6500;
  };
  struct State {
    std::string entity, status, hvac_mode;
    std::string power_unit, voltage_unit, current_unit, energy_unit;
    float brightness = NAN, hue = NAN, saturation = NAN, kelvin = NAN;
    float target = NAN, temperature = NAN, humidity = NAN;
    float position = NAN, speed = NAN, volume = NAN;
    float power = NAN, voltage = NAN, current = NAN, energy = NAN, power_factor = NAN;
    bool muted = false, valid = false;
  };

  std::vector<Device> devices;
  State live;
  size_t index = 0;
  int ui_mode = 0; // 0=browse, 1=select action/parameter, 2=adjust draft
  int field_index = 0;
  float draft = 0;
  bool pending = false;
  std::string pending_entity, pending_op, pending_value;
  std::string tx_entity, tx_op, tx_value; // freeze parameters for an API call
  std::string error;
  bool ha_online = false;
  uint32_t last_state_ms = 0;
  uint32_t last_catalog_ms = 0;
  // The knob has two independent time behaviours:
  // 1) suppress rapid encoder edges, leaving a deliberate one-step response;
  // 2) after inactivity in an action/edit screen, return to device browsing.
  // HA polling and catalog refresh must NEVER renew the interaction timer.
  uint32_t last_encoder_step_ms = 0;
  bool encoder_step_seen = false;
  uint32_t last_interaction_ms = 0;

  void note_interaction(uint32_t now_ms = millis()) {
    last_interaction_ms = now_ms;
  }

  bool return_to_browse_if_idle(uint32_t now_ms = millis(), uint32_t timeout_ms = 6500u) {
    if (ui_mode == 0 || last_interaction_ms == 0) return false;
    if ((uint32_t)(now_ms - last_interaction_ms) < timeout_ms) return false;
    ui_mode = 0;
    field_index = 0;
    draft = 0;
    // Pending commands that were explicitly confirmed are NOT modified;
    // an unsent draft is always abandoned, without issuing a command.
    encoder_step_seen = false; // next detent switches devices immediately
    return true;
  }

  static float numeric(JsonVariantConst v) {
    if (v.isNull()) return NAN;
    if (v.is<const char *>()) {
      const char *s = v.as<const char *>();
      if (!s || !*s) return NAN;
      char *end = nullptr;
      const float n = strtof(s, &end);
      return end != s && *end == '\0' && std::isfinite(n) ? n : NAN;
    }
    if (v.is<float>() || v.is<double>() || v.is<int>() || v.is<long>()) return v.as<float>();
    return NAN;
  }
  static std::string str(JsonVariantConst v) {
    if (v.isNull()) return "";
    const char *s = v.as<const char *>();
    return s ? s : "";
  }
  static std::string fmt(float value, int decimals = 0) {
    if (!std::isfinite(value)) return "--";
    char b[32]; snprintf(b, sizeof(b), decimals == 2 ? "%.2f" : (decimals ? "%.1f" : "%.0f"), value);
    return b;
  }
  static float bound(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
  }
  static std::vector<std::string> split(const std::string &s) {
    std::vector<std::string> out; size_t i = 0;
    while (i < s.size()) {
      size_t p = s.find(',', i);
      std::string x = s.substr(i, p == std::string::npos ? p : p - i);
      if (!x.empty()) out.push_back(x);
      if (p == std::string::npos) break;
      i = p + 1;
    }
    return out;
  }
  const Device *selected() const {
    return index < devices.size() ? &devices[index] : nullptr;
  }

  bool load_catalog(const std::string &json) {
    JsonDocument doc;
    const auto err = deserializeJson(doc, json);
    if (err || !doc.is<JsonArray>()) { error = "CATALOGO JSON NON VALIDO"; return false; }
    const std::string keep = selected() ? selected()->entity : "";
    std::vector<Device> next;
    for (JsonVariantConst v : doc.as<JsonArrayConst>()) {
      if (!v.is<JsonObjectConst>() || next.size() >= 24) break;
      Device d;
      d.entity = str(v["entity"]); d.name = str(v["name"]);
      d.domain = str(v["domain"]); d.hvac_modes = str(v["modes"]);
      if (d.entity.find('.') == std::string::npos || d.entity.size() > 110) continue;
      if (d.name.empty()) d.name = d.entity;
      if (d.name.size() > 95) d.name.resize(95);
      d.features = v["features"].isNull() ? 0 : v["features"].as<int>();
      d.dim = v["dim"].as<bool>(); d.color = v["color"].as<bool>();
      d.kelvin = v["kelvin"].as<bool>();
      d.min_temp = std::isfinite(numeric(v["min_temp"])) ? numeric(v["min_temp"]) : 7;
      d.max_temp = std::isfinite(numeric(v["max_temp"])) ? numeric(v["max_temp"]) : 35;
      d.temp_step = std::isfinite(numeric(v["temp_step"])) ? bound(numeric(v["temp_step"]), 0.1f, 5.f) : 0.5f;
      d.min_kelvin = std::isfinite(numeric(v["min_kelvin"])) ? numeric(v["min_kelvin"]) : 2000;
      d.max_kelvin = std::isfinite(numeric(v["max_kelvin"])) ? numeric(v["max_kelvin"]) : 6500;
      if (d.max_temp < d.min_temp) std::swap(d.max_temp, d.min_temp);
      if (d.max_kelvin < d.min_kelvin) std::swap(d.max_kelvin, d.min_kelvin);
      next.push_back(d);
    }
    devices.swap(next);
    index = 0;
    for (size_t i = 0; i < devices.size(); i++) if (devices[i].entity == keep) { index = i; break; }
    if (!selected() || selected()->entity != keep) {
      ui_mode = 0; field_index = 0; live = State{};
    }
    if (field_index >= (int)fields().size()) field_index = 0;
    last_catalog_ms = millis(); error.clear();
    return true;
  }

  bool load_state(const std::string &json) {
    JsonDocument doc;
    const auto err = deserializeJson(doc, json);
    if (err || !doc.is<JsonObjectConst>()) { error = "STATO JSON NON VALIDO"; return false; }
    JsonObjectConst o = doc.as<JsonObjectConst>();
    const auto *d = selected();
    if (!d || str(o["entity"]) != d->entity) return false; // late response to another page
    State s;
    s.entity = d->entity; s.status = str(o["state"]); s.hvac_mode = str(o["hvac_mode"]);
    s.brightness = numeric(o["brightness"]); s.hue = numeric(o["hue"]);
    s.saturation = numeric(o["saturation"]);
    s.kelvin = numeric(o["kelvin"]); s.target = numeric(o["target"]);
    s.temperature = numeric(o["temperature"]); s.humidity = numeric(o["humidity"]);
    s.position = numeric(o["position"]); s.speed = numeric(o["speed"]);
    s.volume = numeric(o["volume"]); s.power = numeric(o["power"]);
    s.voltage = numeric(o["voltage"]); s.current = numeric(o["current"]);
    s.energy = numeric(o["energy"]); s.power_factor = numeric(o["power_factor"]); s.muted = o["muted"].as<bool>();
    s.power_unit = str(o["power_unit"]); s.voltage_unit = str(o["voltage_unit"]);
    s.current_unit = str(o["current_unit"]); s.energy_unit = str(o["energy_unit"]);
    s.valid = !(s.status.empty() || s.status == "unavailable" || s.status == "unknown");
    live = s; last_state_ms = millis(); error.clear(); return true;
  }

  std::vector<std::string> fields() const {
    std::vector<std::string> f;
    const auto *d = selected(); if (!d) return f;
    const auto &t = d->domain;
    if (t == "light") {
      f.push_back("POWER");
      if (d->dim) f.push_back("BRIGHTNESS");
      if (d->color) { f.push_back("COLOR"); f.push_back("SATURATION"); }
      if (d->kelvin) f.push_back("WHITE K");
    } else if (t == "switch" || t == "input_boolean") {
      f.push_back("POWER");
      if (std::isfinite(live.power)) f.push_back("WATT");
      if (std::isfinite(live.voltage)) f.push_back("VOLT");
      if (std::isfinite(live.current)) f.push_back("AMPERE");
      if (std::isfinite(live.energy)) f.push_back("ENERGY");
      if (std::isfinite(live.power_factor)) f.push_back("COS PHI");
    } else if (t == "climate") {
      if (d->features & 1) f.push_back("TARGET");
      if (!d->hvac_modes.empty()) f.push_back("HVAC MODE");
      if (std::isfinite(live.temperature)) f.push_back("ROOM TEMP");
      if (std::isfinite(live.humidity)) f.push_back("HUMIDITY");
    } else if (t == "cover") {
      if (d->features & 4) f.push_back("POSITION");
      if (d->features & 1) f.push_back("OPEN");
      if (d->features & 8) f.push_back("STOP");
      if (d->features & 2) f.push_back("CLOSE");
    } else if (t == "fan") {
      f.push_back("POWER");
      if (d->features & 1) f.push_back("SPEED");
    } else if (t == "media_player") {
      f.push_back("PLAY/PAUSE");
      if (d->features & 4) f.push_back("VOLUME");
      if (d->features & 8) f.push_back("MUTE");
    } else if (t == "scene" || t == "script") {
      f.push_back("RUN");
    }
    return f;
  }

  std::string field() const {
    const auto f = fields();
    if (f.empty()) return "MONITOR";
    return f[(size_t)std::min((int)f.size()-1, std::max(0, field_index))];
  }
  bool adjustable() const {
    auto f = field();
    return f=="BRIGHTNESS" || f=="COLOR" || f=="SATURATION" || f=="WHITE K" || f=="TARGET" ||
           f=="HVAC MODE" || f=="POSITION" || f=="SPEED" || f=="VOLUME";
  }
  float current_value() const {
    const auto f = field();
    if (f == "BRIGHTNESS") return std::isfinite(live.brightness) ? live.brightness : 0;
    if (f == "COLOR") return std::isfinite(live.hue) ? live.hue : 0;
    if (f == "SATURATION") return std::isfinite(live.saturation) ? live.saturation : 100;
    if (f == "WHITE K") return std::isfinite(live.kelvin) ? live.kelvin : 3000;
    if (f == "TARGET") return std::isfinite(live.target) ? live.target : 20;
    if (f == "POSITION") return std::isfinite(live.position) ? live.position : 0;
    if (f == "SPEED") return std::isfinite(live.speed) ? live.speed : 0;
    if (f == "VOLUME") return std::isfinite(live.volume) ? live.volume : 0;
    if (f == "HVAC MODE") {
      auto modes = selected() ? split(selected()->hvac_modes) : std::vector<std::string>{};
      for (size_t i = 0; i < modes.size(); i++) if (modes[i] == live.status) return (float)i;
    }
    return 0;
  }
  // Returns true only when a physical encoder event produces a UI change.
  // Keeping this check here rather than in YAML also protects all UI modes.
  bool rotate(int direction, uint32_t now_ms = millis(), uint32_t min_step_ms = 180u,
              uint32_t idle_timeout_ms = 6500u) {
    if (devices.empty() || direction == 0) return false;
    // If a timeout elapsed but its 500-ms periodic task has not run yet,
    // this detent must navigate to the next device, not change a stale draft.
    return_to_browse_if_idle(now_ms, idle_timeout_ms);
    if (encoder_step_seen && (uint32_t)(now_ms - last_encoder_step_ms) < min_step_ms) return false;
    encoder_step_seen = true;
    last_encoder_step_ms = now_ms;
    note_interaction(now_ms);
    const int step = direction > 0 ? 1 : -1;
    if (ui_mode == 0) {
      index = (index + devices.size() + step) % devices.size();
      field_index = 0; live = State{}; last_state_ms = 0;
    } else if (ui_mode == 1) {
      const auto f = fields();
      if (!f.empty()) field_index = (field_index + (int)f.size() + step) % (int)f.size();
    } else if (ui_mode == 2) {
      const auto *d = selected(); if (!d) return false;
      const auto f = field();
      float increment = 1, lo = 0, hi = 100;
      if (f == "COLOR") { hi = 359; increment = 5; }
      if (f == "WHITE K") { lo = d->min_kelvin; hi = d->max_kelvin; increment = 100; }
      if (f == "TARGET") { lo = d->min_temp; hi = d->max_temp; increment = d->temp_step; }
      if (f == "HVAC MODE") { lo = 0; hi = std::max(0, (int)split(d->hvac_modes).size()-1); }
      if (f == "VOLUME") { increment = 5; }
      if (f == "BRIGHTNESS" || f == "SATURATION" || f == "POSITION" || f == "SPEED") { increment = 5; }
      draft = bound(draft + step * increment, lo, hi);
    }
    return true;
  }
  void back(uint32_t now_ms = millis()) {
    note_interaction(now_ms);
    encoder_step_seen = false;
    if (ui_mode == 2) ui_mode = 1; // cancel draft without actuation
    else ui_mode = 0;
  }
  void queue(const std::string &op, const std::string &value = "") {
    const auto *d = selected(); if (!d || !live.valid || !ha_online) return;
    pending_entity = d->entity; pending_op = op; pending_value = value; pending = true;
  }
  void click(uint32_t now_ms = millis()) {
    const auto *d = selected(); if (!d) return;
    note_interaction(now_ms);
    encoder_step_seen = false; // the first adjustment detent is not lost
    if (ui_mode == 0) { ui_mode = 1; field_index = 0; return; }
    const auto f = field();
    if (fields().empty()) return;
    if (ui_mode == 1 && adjustable()) { draft = current_value(); ui_mode = 2; return; }
    if (ui_mode == 2) {
      if (f == "HVAC MODE") {
        const auto modes = split(d->hvac_modes);
        int p = (int)roundf(draft);
        if (p >= 0 && p < (int)modes.size()) queue("climate_mode", modes[(size_t)p]);
      } else {
        std::string op;
        if (f == "BRIGHTNESS") op = "brightness";
        else if (f == "COLOR") op = "hue";
        else if (f == "SATURATION") op = "saturation";
        else if (f == "WHITE K") op = "kelvin";
        else if (f == "TARGET") op = "target";
        else if (f == "POSITION") op = "position";
        else if (f == "SPEED") op = "speed";
        else if (f == "VOLUME") op = "volume";
        if (!op.empty()) queue(op, fmt(draft, f == "TARGET"));
      }
      ui_mode = 1; return;
    }
    if (f == "POWER") queue("toggle");
    else if (f == "PLAY/PAUSE") queue("play_pause");
    else if (f == "MUTE") queue("mute");
    else if (f == "OPEN") queue("open");
    else if (f == "CLOSE") queue("close");
    else if (f == "STOP") queue("stop");
    else if (f == "RUN") queue("run");
  }
  void stage_send() {
    tx_entity = pending_entity; tx_op = pending_op; tx_value = pending_value;
    pending = false;
  }

  std::string display_value() const {
    const auto *d = selected(); if (!d) return "--";
    const auto f = field();
    const bool edit = ui_mode == 2;
    const float v = edit ? draft : current_value();
    if (ui_mode > 0) {
      if (f == "BRIGHTNESS" || f == "SATURATION" || f == "POSITION" || f == "SPEED" || f == "VOLUME") return fmt(v) + "%";
      if (f == "COLOR") return fmt(v) + " deg";
      if (f == "WHITE K") return fmt(v) + " K";
      if (f == "TARGET") return fmt(v, 1) + " C";
      if (f == "WATT") return fmt(live.power) + " " + live.power_unit;
      if (f == "VOLT") return fmt(live.voltage) + " " + live.voltage_unit;
      if (f == "AMPERE") return fmt(live.current,1) + " " + live.current_unit;
      if (f == "ENERGY") return fmt(live.energy,1) + " " + live.energy_unit;
      if (f == "COS PHI") return fmt(live.power_factor,2);
      if (f == "ROOM TEMP") return fmt(live.temperature,1) + " C";
      if (f == "HUMIDITY") return fmt(live.humidity) + "%";
      if (f == "HVAC MODE") {
        const auto modes = split(d->hvac_modes); const int i = (int)roundf(v);
        return (i >= 0 && i < (int)modes.size()) ? modes[(size_t)i] : "--";
      }
    }
    if (!live.valid || live.entity != d->entity) return "--";
    if (d->domain == "climate") return fmt(live.target, 1) + " C";
    if (d->domain == "cover") return fmt(live.position) + "%";
    if (d->domain == "light" && std::isfinite(live.brightness) && live.status == "on") return fmt(live.brightness) + "%";
    if (d->domain == "fan" && std::isfinite(live.speed) && live.status == "on") return fmt(live.speed) + "%";
    if (d->domain == "media_player" && std::isfinite(live.volume)) return fmt(live.volume) + "%";
    if (d->domain == "sensor") return live.status;
    if (live.status == "on") return "ON";
    if (live.status == "off") return "OFF";
    if (live.status == "open") return "OPEN";
    if (live.status == "closed") return "CLOSED";
    if (live.status == "playing") return "PLAYING";
    return live.status;
  }
  std::string telemetry() const {
    if (!live.valid) return "DATI NON DISPONIBILI";
    const auto *d = selected(); if (!d) return "";
    std::string a;
    auto append = [&](const char *key, float v, const char *unit, int decimals=0) {
      if (!std::isfinite(v)) return;
      if (!a.empty()) a += "   ";
      a += key; a += " "; a += fmt(v, decimals); a += unit;
    };
    if (d->domain == "climate") {
      append("T", live.temperature, "C", 1); append("H", live.humidity, "%");
    } else if (d->domain == "switch" || d->domain == "input_boolean") {
      append("P", live.power, live.power_unit.c_str());
      append("V", live.voltage, live.voltage_unit.c_str());
      append("I", live.current, live.current_unit.c_str(), 1);
    } else if (d->domain == "cover") {
      append("POS", live.position, "%");
    } else if (d->domain == "sensor") {
      append("T", live.temperature, "C", 1); append("H", live.humidity, "%");
    } else if (d->domain == "light") {
      append("LUM", live.brightness, "%");
    }
    if (a.empty()) a = live.status;
    if (a.size() > 37) a.resize(37);
    return a;
  }

  // HMH visual identity: icon selection is based on the HA domain, never on
  // device names (which may be user-defined and localized).
  static size_t icon_index(const std::string &domain) {
    if (domain == "light") return 0;
    if (domain == "switch" || domain == "input_boolean") return 1;
    if (domain == "climate") return 2;
    if (domain == "cover") return 3;
    if (domain == "fan") return 4;
    if (domain == "media_player") return 5;
    if (domain == "scene" || domain == "script") return 6;
    if (domain == "sensor" || domain == "binary_sensor") return 7;
    return 8;
  }
  static uint32_t color_from_hs(float hue, float saturation) {
    // HA's hs_color reports H=0..360, S=0..100; white V=1.
    if (!std::isfinite(hue) || !std::isfinite(saturation)) return 0xFBBF24;
    float h = std::fmod(hue, 360.0f); if (h < 0) h += 360.0f;
    float s = bound(saturation / 100.0f, 0.0f, 1.0f);
    float c = s;
    float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = 1.0f - c;
    float r=0, g=0, b=0;
    if (h < 60) { r=c; g=x; }
    else if (h < 120) { r=x; g=c; }
    else if (h < 180) { g=c; b=x; }
    else if (h < 240) { g=x; b=c; }
    else if (h < 300) { r=x; b=c; }
    else { r=c; b=x; }
    return ((uint32_t)std::round((r+m)*255) << 16) |
           ((uint32_t)std::round((g+m)*255) << 8) |
           ((uint32_t)std::round((b+m)*255));
  }
  uint32_t current_icon_color(const Device *d) const {
    if (!d || !ha_online || !live.valid || live.entity != d->entity) return 0x64748B;
    const std::string &domain = d->domain;
    const std::string &state = live.status;
    if (domain == "light") {
      if (state != "on") return 0x64748B;
      return d->color ? color_from_hs(live.hue, live.saturation) : 0xFBBF24;
    }
    if (domain == "switch" || domain == "input_boolean") return state == "on" ? 0x38BDF8 : 0x64748B;
    if (domain == "climate") {
      if (state == "off") return 0x64748B;
      if (state == "heat" || state == "heat_cool") return 0xFB923C;
      if (state == "cool") return 0x60A5FA;
      return 0x22C55E;
    }
    if (domain == "cover") return
      (state == "open" || state == "opening" || state == "closing" ||
       (std::isfinite(live.position) && live.position > 0.5f)) ? 0x22C55E : 0x64748B;
    if (domain == "fan") return state == "on" ? 0x34D399 : 0x64748B;
    if (domain == "media_player") return state == "playing" ? 0xC084FC : 0x64748B;
    if (domain == "scene" || domain == "script") return 0xC084FC;
    if (domain == "sensor" || domain == "binary_sensor") return 0x38BDF8;
    return 0x94A3B8;
  }
  void update_icon(const Device *d, lv_obj_t *disk,
                   const std::array<lv_obj_t *, 9> &icons) const {
    const size_t active = d ? icon_index(d->domain) : 8;
    const uint32_t color = current_icon_color(d);
    for (size_t i = 0; i < icons.size(); ++i) {
      if (i == active) lv_obj_remove_flag(icons[i], LV_OBJ_FLAG_HIDDEN);
      else lv_obj_add_flag(icons[i], LV_OBJ_FLAG_HIDDEN);
      if (i == active) lv_obj_set_style_image_recolor(icons[i], lv_color_hex(color), LV_PART_MAIN);
    }
    lv_obj_set_style_bg_color(disk, lv_color_hex(color == 0x64748B ? 0x0F172A : 0x112235), LV_PART_MAIN);
    lv_obj_set_style_border_color(disk, lv_color_hex(color), LV_PART_MAIN);
  }

  void render(lv_obj_t *title, lv_obj_t *counter, lv_obj_t *type,
              lv_obj_t *value, lv_obj_t *field_label, lv_obj_t *info,
              lv_obj_t *hint, lv_obj_t *bar, lv_obj_t *icon_disk,
              const std::array<lv_obj_t *, 9> &icons) const {
    const auto *d = selected();
    update_icon(d, icon_disk, icons);
    if (!d) {
      lv_label_set_text(title, "HMH UNIVERSAL KNOB");
      lv_label_set_text(counter, ha_online ? "ETICHETTA: HMH_Device" : "HOME ASSISTANT OFFLINE");
      lv_label_set_text(type, "NESSUN DISPOSITIVO");
      lv_label_set_text(value, "--");
      lv_label_set_text(field_label, "ATTESA CATALOGO");
      lv_label_set_text(info, error.empty() ? "Assegna HMH_Device" : error.c_str());
      lv_label_set_text(hint, "La lista si aggiorna automaticamente");
      lv_bar_set_value(bar, 0, LV_ANIM_OFF); return;
    }
    char idx[35]; snprintf(idx, sizeof(idx), "%d / %d", (int)index+1, (int)devices.size());
    lv_label_set_text(counter, idx);
    lv_label_set_text(title, d->name.c_str());
    std::string kind = d->domain;
    if (!ha_online) kind += "  / OFFLINE";
    else if (!live.valid || live.entity != d->entity) kind += "  / SINCRONIZZO";
    lv_label_set_text(type, kind.c_str());
    lv_label_set_text(value, display_value().c_str());
    lv_label_set_text(field_label, ui_mode == 0 ? "DISPOSITIVO" : field().c_str());
    lv_label_set_text(info, telemetry().c_str());
    lv_label_set_text(hint, !error.empty() ? error.c_str() :
                           ui_mode == 0 ? "RUOTA / PREMI" :
                           ui_mode == 1 ? "RUOTA / USA" :
                                          "RUOTA / INVIA");
    float percent = 0;
    if (ui_mode > 0 && adjustable()) {
      const auto f = field(); const float v = ui_mode == 2 ? draft : current_value();
      if (f == "TARGET") percent = 100 * (v-d->min_temp) / std::max(0.1f, d->max_temp-d->min_temp);
      else if (f == "WHITE K") percent = 100 * (v-d->min_kelvin) / std::max(1.f, d->max_kelvin-d->min_kelvin);
      else if (f == "COLOR") percent = v / 3.59f;
      else if (f == "HVAC MODE") percent = 100 * v / std::max(1.f, (float)split(d->hvac_modes).size()-1);
      else percent = v;
    } else if (d->domain == "light") percent = live.brightness;
    else if (d->domain == "cover") percent = live.position;
    else if (d->domain == "fan") percent = live.speed;
    else if (d->domain == "media_player") percent = live.volume;
    lv_bar_set_value(bar, std::isfinite(percent) ? (int)bound(percent, 0, 100) : 0, LV_ANIM_OFF);
    lv_obj_set_style_text_color(value,
      lv_color_hex(ui_mode == 2 ? 0xFBBF24 : live.valid ? 0xF8FAFC : 0x94A3B8), LV_PART_MAIN);
  }
};

static HMHUniversalKnob hmh_knob;
