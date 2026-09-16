# Events

## Status

| Status | Feature |
|:---:|---|
| ✅ | Subscribe and publish by event ID |
| ✅ | Snapshot subscriptions before dispatch |
| ✅ | Synchronous delivery |
| ❌ | Queue, priority, or schema registry |

`ST_EventSubscription` stores an event ID, callback, and user pointer. `ST_Event`
contains the event ID plus an opaque payload pointer and byte size.

The payload is borrowed for the callback only. The publisher owns and versions its
payload structure. Callbacks run synchronously on the publishing thread. A callback
error or exception produces `ST_RESULT_CALLBACK_FAILED`.
