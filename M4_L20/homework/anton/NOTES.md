# NOTES.md - Homework 20

## Why the setpoint survives a reboot

The setpoint is published to MQTT with retain=true. The broker stores the last
retained value on the topic and delivers it to any subscriber that connects
later. When the node reboots and reconnects, it receives the retained setpoint
immediately, so the value is restored without any action from the user.

If the broker restarts with persistence=false, the retained messages are lost.
The node then falls back to the default setpoint (21.0 C) until a new value is
published.

## Fault I caused

**Case**: Published a setpoint of "abc".
**Symptom**: The node printed "setpoint abc rejected" on Serial and published
an error message on the error topic.
**Hypotheses**:
1. strtof parsed "abc" as 0.0 and accepted it.
2. strtof returned 0.0 with endp == msg, and the range check rejected it.
**Evidence**: The end pointer was not advanced; the range check caught it.
**Conclusion**: The parser worked correctly; the input was invalid.
**Fix**: No code change needed. The validation is correct.