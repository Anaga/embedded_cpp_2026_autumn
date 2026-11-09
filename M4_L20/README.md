# Lesson 20 - MQTT and Network Diagnosis

Back on the ESP32-C3 Super Mini. Session 18 sent data to a public broker;
today the class runs its own broker and learns to find out why a message
did not arrive.

## What this lesson covers

- Publish / subscribe, topic levels, the `+` and `#` wildcards
- QoS 0 / 1 / 2 and what PubSubClient supports
- Retained messages, keepalive and the last will (`online` / `offline`)
- The callback rule: copy, set a flag, return - work in `loop()`
- Mosquitto with `broker/class.conf`, reading its log, MQTT Explorer
- Diagnosis: symptom -> hypothesis -> evidence -> conclusion

## Contents

| Folder | What |
|---|---|
| `slides/` | the deck and its generator |
| `broker/class.conf` | Mosquitto config for the class network and for home |
| `lab1-node/` | AHT10 node on the class broker - wiring, run, topics, lab 2 rules in its README |
| `homework/` | MQTT thermostat: task in `homework/README.md` |

## Before class

- Install MQTT Explorer (mqtt-explorer.com) if you do not have it.
- Build `lab1-node` once at home so the libraries are downloaded.
