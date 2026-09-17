# Complete minimal mod

The [first-mod guide](../guides/first-mod.md) contains the complete exported factory. A production project should split state and callbacks, retain registration handles, and implement cleanup:

```cpp
struct State { ST_StringView owner; ST_Registration command{}; };

static ST_Result ST_CALL Unload(const ST_HostApiV1* host, void* user) {
    auto* state = static_cast<State*>(user);
    if (state->command) host->release_registration(state->command);
    host->release_owner(state->owner);
    return ST_RESULT_OK;
}
```

Build as a Windows x64 DLL, keep exceptions inside callbacks, package with `mod.json`, and verify loading in `shroudtopia.log`. Add only the capabilities actually required.
