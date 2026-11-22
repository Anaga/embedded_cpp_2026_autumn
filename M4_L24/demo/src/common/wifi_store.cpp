#include "wifi_store.h"

#include <Preferences.h>
#include <stdio.h>
#include <string.h>

namespace wifi_store {

namespace {

constexpr const char *NAMESPACE = "wifi";
constexpr const char *KEY_COUNT = "n";

void key(char *buf, size_t size, char prefix, uint8_t index) {
  snprintf(buf, size, "%c%u", prefix, static_cast<unsigned>(index));
}

// Rewrites the whole list. Five short strings: simpler than patching keys.
bool save(const Network *nets, uint8_t count) {
  Preferences prefs;
  if (!prefs.begin(NAMESPACE, false)) {
    return false;
  }
  prefs.clear();
  bool ok = (prefs.putUChar(KEY_COUNT, count) == 1U);
  for (uint8_t i = 0U; i < count; i++) {
    char k[4];
    key(k, sizeof(k), 's', i);
    ok = ok && (prefs.putString(k, nets[i].ssid) > 0U);
    key(k, sizeof(k), 'p', i);
    // An open network has an empty password. putString() returns 0 for
    // an empty string, so that is not an error here.
    prefs.putString(k, nets[i].pass);
  }
  prefs.end();
  return ok;
}

}  // namespace

uint8_t load(Network *out, uint8_t max) {
  Preferences prefs;
  // Read-write on purpose: read-only fails with "nvs_open failed: NOT_FOUND"
  // on a new board, because the namespace does not exist yet. Read-write
  // creates it (empty) and nothing else is written here.
  if (!prefs.begin(NAMESPACE, false)) {
    return 0U;
  }
  uint8_t count = prefs.getUChar(KEY_COUNT, 0U);
  if (count > max) {
    count = max;
  }
  for (uint8_t i = 0U; i < count; i++) {
    char k[4];
    out[i].ssid[0] = '\0';
    out[i].pass[0] = '\0';
    key(k, sizeof(k), 's', i);
    if (prefs.isKey(k)) {
      prefs.getString(k, out[i].ssid, sizeof(out[i].ssid));
    }
    key(k, sizeof(k), 'p', i);
    if (prefs.isKey(k)) {
      prefs.getString(k, out[i].pass, sizeof(out[i].pass));
    }
  }
  prefs.end();
  return count;
}

Result add(const char *ssid, const char *pass) {
  size_t ssidLen = strlen(ssid);
  size_t passLen = strlen(pass);
  if ((ssidLen == 0U) || (ssidLen > SSID_MAX)) {
    return Result::BAD_SSID;
  }
  if ((passLen != 0U) && ((passLen < PASS_MIN) || (passLen > PASS_MAX))) {
    return Result::BAD_PASS;
  }

  Network nets[MAX_NETWORKS];
  uint8_t count = load(nets, MAX_NETWORKS);

  Result result = Result::ADDED;
  uint8_t slot = count;
  for (uint8_t i = 0U; i < count; i++) {
    if (strcmp(nets[i].ssid, ssid) == 0) {
      slot   = i;
      result = Result::REPLACED;
    }
  }
  if (slot == MAX_NETWORKS) {
    return Result::FULL;
  }

  strncpy(nets[slot].ssid, ssid, SSID_MAX);
  nets[slot].ssid[SSID_MAX] = '\0';
  strncpy(nets[slot].pass, pass, PASS_MAX);
  nets[slot].pass[PASS_MAX] = '\0';
  if (slot == count) {
    count++;
  }
  return save(nets, count) ? result : Result::NVS_ERROR;
}

bool remove(uint8_t index) {
  Network nets[MAX_NETWORKS];
  uint8_t count = load(nets, MAX_NETWORKS);
  if (index >= count) {
    return false;
  }
  for (uint8_t i = index; (i + 1U) < count; i++) {
    nets[i] = nets[i + 1U];
  }
  count--;
  return save(nets, count);
}

void clear() {
  Preferences prefs;
  if (prefs.begin(NAMESPACE, false)) {
    prefs.clear();
    prefs.end();
  }
}

const char *resultText(Result r) {
  switch (r) {
    case Result::ADDED:     return "saved";
    case Result::REPLACED:  return "password replaced";
    case Result::FULL:      return "list full - wifi del <n> first";
    case Result::BAD_SSID:  return "SSID must be 1 to 31 characters";
    case Result::BAD_PASS:  return "password must be 8 to 63 characters (or none for an open network)";
    case Result::NVS_ERROR: return "could not write NVS";
  }
  return "?";
}

}  // namespace wifi_store
