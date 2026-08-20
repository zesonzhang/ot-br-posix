# Telemetry Push Design

## Overview

Currently `ot-br-posix` exposes telemetry information through a `TelemetryData`
property on the DBus interface.  Consumers must poll this property to obtain
updates.  To support immediate event reporting and periodic snapshots, we will
extend the DBus API so that the border router actively pushes serialized
telemetry records.

## Goals

- Emit full `TelemetryData` snapshots every two minutes.
- Immediately report important network events (e.g., role change, partition
  change, or address update).
- Reuse existing protobuf definition `threadnetwork.TelemetryData` as the payload
  format.
- Keep the current `TelemetryData` property for backwards compatibility.

## Non-Goals

- Providing reliability guarantees or buffering when the consumer is offline.
- Changing the telemetry protobuf schema.

## DBus API Changes

1. **Add `TelemetryUpdate` signal**
   - Define a new DBus signal in `src/dbus/server/introspect.xml`.
   - Signature: `ay` (array of bytes) containing a serialized
     `threadnetwork::TelemetryData` message.
   - The existing `TelemetryData` property remains read-only; its
     `EmitsChangesSignal` annotation may be set to `true` but the signal will be
     delivered through `TelemetryUpdate`.

2. **Emit helper**
   - Implement a helper in `dbus_thread_object_rcp.cpp` that retrieves telemetry
     data using `TelemetryRetriever`, serializes it to a byte array, and calls
     `g_dbus_emit_signal()` to send `TelemetryUpdate`.
   - Export a small utility (`EmitTelemetryUpdate`) so other modules can trigger
     a push.

## Periodic Snapshot

- Create a timer in the main event loop (e.g., in `otbr-agent/main.cpp`) that
  fires every two minutes.
- Timer callback invokes `EmitTelemetryUpdate` to push the current snapshot.
- Timer is armed only when DBus is enabled and telemetry is configured.

## Event-Driven Reports

- Hook existing OTBR events such as:
  - Network role or state changes.
  - Thread network partition ID changes.
  - Detected IPv6 address additions/removals.
- Each hook calls `EmitTelemetryUpdate` to push a snapshot immediately after the
  event occurs.
- If desired, the helper can accept a flag to emit partial reports for specific
  events.  The initial implementation sends the full snapshot to reduce API
  complexity.

## Thread Daemon Integration

- Thread daemon listens for the `TelemetryUpdate` signal on the DBus interface.
- Upon reception, it parses the protobuf payload and forwards it to the cloud
  backend.
- Existing polling logic may remain for backward compatibility but can be
  removed once push-based updates are fully adopted.

## Testing

1. Unit-test the new signal emission helper by verifying that
   `TelemetryUpdate` is produced when the helper is invoked.
2. Add an integration test that subscribes to the DBus signal, triggers a role
   change in a simulated environment, and confirms that the signal is received.
3. Ensure the periodic timer emits updates at the configured interval in a
   standalone test environment (could use a mock GLib main loop).

## Future Work

- Add optional persistence so that telemetry events are queued if the daemon is
  unavailable.
- Consider a gRPC or socket-based reporter for platforms that do not use DBus.
- Provide configuration options for snapshot interval and event types through
  command-line flags or configuration files.

