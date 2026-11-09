---
description: Repo-specific guidance for plugin implementation communication and robustness patterns.
applyTo: "**/*.cpp,**/*.h"
---

# Implementation Instructions (ResourceManager Pattern)
  1. Communication Strategy Priority
  2. COM-RPC for Plugin Interfaces
  3. JSON-RPC Fallback for JSON-RPC-only Services
  4. On-Demand Access and Pointer Safety
  5. State and Concurrency Handling
  6. Error and Response Semantics

### 1) Communication Strategy Priority

### Requirement

Use this priority order for cross-component communication:

1. COM-RPC for Thunder plugin interfaces (Exchange contracts).
2. JSON-RPC only when target service is JSON-RPC-only.

In this repository:

- Front plugin `ResourceManager` acquires backend through COM-RPC (`IShell::Root<Exchange::IResourceManager>`).
- `ResourceManagerImplementation` uses JSON-RPC to call TextToSpeech `setACL` because that interaction is JSON-RPC-based.

### 2) COM-RPC for Plugin Interfaces

### Requirement

Use COM-RPC APIs such as:

- `QueryInterfaceByCallsign<T>()`
- `QueryInterface<T>()`
- `IShell::Root<T>()`

for Exchange interface communication.

### Example

```cpp
_resourceManager = _service->Root<Exchange::IResourceManager>(_connectionId, 5000, _T("ResourceManagerImplementation"));
```

### 3) JSON-RPC Fallback for JSON-RPC-only Services

### Requirement

Do not ban JSON-RPC universally. Use it when no Exchange interface path exists.

For this plugin implementation, TextToSpeech ACL configuration is performed via JSON-RPC:

```cpp
auto ttsClient = Utils::getThunderControllerClient("org.rdk.TextToSpeech.1");
if (ttsClient != nullptr) {
    ret = ttsClient->Invoke<JsonObject, JsonObject>(20000, "setACL", params, ttsResponse);
}
```

Prefer on-demand client acquisition rather than storing long-lived cross-plugin JSON-RPC clients.

### 4) On-Demand Access and Pointer Safety

### Requirement

When obtaining interfaces/pointers to external plugins/services:

- Acquire only when required by current operation.
- Validate pointer before use.
- Release COM-RPC interface pointers after use.
- Do not rely on stale interfaces across target plugin deactivation/restart.

### Correct Example

```cpp
auto other = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IOtherPlugin>("org.rdk.OtherPlugin");
if (other != nullptr) {
    other->PerformAction();
    other->Release();
}
```

### Incorrect Example

```cpp
void MyPlugin::Initialize() {
    _otherPlugin = _service->QueryInterfaceByCallsign<WPEFramework::Exchange::IOtherPlugin>("org.rdk.OtherPlugin");
}

void MyPlugin::DoSomething() {
    _otherPlugin->PerformAction(); // Risky if target plugin is deactivated
}
```

### 5) State and Concurrency Handling

### Requirement

Implementation methods that mutate shared state must protect critical regions.

For this repo pattern:

- Use `Core::CriticalSection` (here `_adminLock`) around shared state and external call orchestration.
- Keep state containers (`mAppsAVBlacklistStatus`) consistent with operation success.
- Reset/cleanup static instance and external resources in destructor.

### Example

```cpp
_adminLock.Lock();
try {
    // mutate shared state and perform guarded operations
} catch (const std::exception&) {
    // map exception to stable error behavior
}
_adminLock.Unlock();
```

### 6) Error and Response Semantics

### Requirement

Keep method outcomes deterministic:

- Initialize output fields early (for example, `result.success = false`).
- Convert errors/exceptions to stable `Core::hresult` return values.
- For RFC-disabled scenarios, return explicit success/fallback behavior as implemented.
- Validate nullable input arguments (for example, iterator pointers) before processing.

### Example

```cpp
Core::hresult ResourceManagerImplementation::ReserveTTSResourceForApps(IStringIterator* const appids, Exchange::IResourceManager::Success& result)
{
    if (appids == nullptr) {
        result.success = false;
        return Core::ERROR_NONE;
    }
    // continue normal flow
}
```
