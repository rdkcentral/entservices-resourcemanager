# ResourceManager Plugin - Architecture Documentation

## Overview

The ResourceManager plugin is a WPEFramework (Thunder) plugin that provides centralized AV resource control and TTS (Text-to-Speech) resource reservation for RDK-based devices. It acts as a single gatekeeper for managing which applications are allowed AV output and which application holds the TTS resource, integrating with the Essos Resource Manager (ERM) library for hardware-level enforcement.

## System Architecture

### Component Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   Client Applications                       │
│        (Apps requiring AV/TTS resource management)         │
└────────────────────┬────────────────────────────────────────┘
                     │ JSON-RPC
                     ▼
┌─────────────────────────────────────────────────────────────┐
│             ResourceManager Plugin (Thunder)                │
│  ┌───────────────────────────────────────────────────────┐  │
│  │   ResourceManager JSONRPC Handler                     │  │
│  │   - setAVBlocked(appid, blocked)                      │  │
│  │   - getBlockedAVApplications()                        │  │
│  │   - reserveTTSResource(appid)                         │  │
│  │   - reserveTTSResourceForApps(appids[])               │  │
│  └───────────────────┬───────────────────────────────────┘  │
│                      │                                       │
│  ┌───────────────────▼───────────────────────────────────┐  │
│  │   ResourceManager Core Logic                          │  │
│  │   - RFC feature flag evaluation at startup            │  │
│  │   - AV blacklist map management                       │  │
│  │   - SecurityAgent token acquisition                   │  │
│  └───────────────────┬───────────────────────────────────┘  │
└──────────────────────┼──────────────────────────────────────┘
                       │
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              Essos Resource Manager (libessosrmgr)          │
│   EssRMgrCreate / EssRMgrAddToBlackList /                   │
│   EssRMgrRemoveFromBlackList / EssRMgrReserveTTS            │
└─────────────────────────────────────────────────────────────┘
```

### Component Interactions

1. **ResourceManager Plugin (Main)**: Entry point registered with Thunder framework
   - Manages plugin lifecycle (`Initialize` / `Deinitialize`)
   - Acquires a security token from `SecurityAgent` on startup
   - Registers four JSON-RPC method handlers for client communication
   - Reads RFC feature flags at construction time to enable/disable blacklist and TTS reservation

2. **Essos Resource Manager (`EssRMgr`)**: External hardware resource control library
   - `EssRMgrCreate()` initializes the ERM handle at plugin startup
   - `EssRMgrAddToBlackList` / `EssRMgrRemoveFromBlackList` enforce AV blocking per-app
   - `EssRMgrReserveTTS` handles TTS resource reservation
   - Conditionally compiled via `ENABLE_ERM` build flag

3. **RFC Configuration**: Runtime feature flag layer
   - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.Blacklist.Enable`
   - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.ReserveTTS.Enable`
   - Both are queried once at construction; results cached in `mDisableBlacklist` / `mDisableReserveTTS`

## Data Flow

### AV Block Operation (setAVBlocked)
```
Client → JSON-RPC setAVBlocked(appid, blocked=true)
  → RFC check (mDisableBlacklist)
    → EssRMgrAddToBlackList(mEssRMgr, appid)   [blocked=true]
    → EssRMgrRemoveFromBlackList(mEssRMgr, appid) [blocked=false]
  → response { success: true/false }
```

### AV Query Operation (getBlockedAVApplications)
```
Client → JSON-RPC getBlockedAVApplications()
  → EssRMgr query for current blacklist
  → response { clients: ["appid1", "appid2", ...] }
```

### TTS Reservation (reserveTTSResource / reserveTTSResourceForApps)
```
Client → JSON-RPC reserveTTSResource(appid)
  → RFC check (mDisableReserveTTS)
  → EssRMgrReserveTTS(mEssRMgr, appid)
  → response { success: true/false }
```

## Plugin Framework Integration

### WPEFramework/Thunder Integration

- **IPlugin Interface**: Lifecycle managed via `Initialize` / `Deinitialize`
- **JSONRPC**: Client-facing API exposed via Thunder controller; four methods registered at construction
- **In-Process Execution**: Plugin runs in-process (no `outofprocess` flag set); direct ERM library linkage
- **Service Registration**: Registered with `SERVICE_REGISTRATION(ResourceManager, 1, 0, 1)`
- **SecurityAgent**: Token acquired via `QueryInterfaceByCallsign<IAuthenticate>("SecurityAgent")` for secure Thunder calls

### Interface Hierarchy

```
ResourceManager (IPlugin, JSONRPC)
    ├─> setAVBlocked / getBlockedAVApplications   (AV control)
    ├─> reserveTTSResource / reserveTTSResourceForApps  (TTS control)
    └─> SecurityAgent (IAuthenticate)             (token acquisition)
```

## Dependencies and Interfaces

### External Dependencies

| Dependency | Purpose |
|---|---|
| WPEFramework Plugins (`${NAMESPACE}Plugins`) | Plugin framework base, JSONRPC, IPlugin |
| `libessosrmgr` (`-lessosrmgr`) | Essos Resource Manager hardware API |
| `rfcapi.h` / `RFC_ParamData_t` | RFC feature flag query |
| `SecurityAgent` (Thunder plugin) | Security token for authenticated calls |

### Helper Utilities Used

| File | Usage |
|---|---|
| `helpers/UtilsJsonRpc.h` | `LOGINFOMETHOD`, `returnResponse` macros |
| `helpers/UtilsgetRFCConfig.h` | `Utils::getRFCConfig()` for RFC queries |
| `helpers/UtilsLogging.h` | `LOGINFO`, `LOGERR` logging macros |

## Technical Implementation Details

### RFC-Gated Feature Flags
Both blacklist and TTS reservation are RFC-controlled. Both default to **disabled** at startup; only enabled if RFC returns `false` for the disable flag. This allows operators to control feature availability without redeployment.

### AV Blacklist State
- Internal `std::map<std::string, bool> mAppsAVBlacklistStatus` tracks per-app AV block state
- ERM is the authoritative store; map provides in-process cache
- Operations are synchronous; response reflects ERM return status directly

### Build Flags
| Flag | Effect |
|---|---|
| `ENABLE_ERM` | Activates EssRMgr linkage and AV/TTS logic |
| `ENABLE_L1TEST` | Enables ERM mock path for unit tests |
| `BUILD_ENABLE_ERM` | CMake toggle that sets `ENABLE_ERM` at build time |

### API Version
- Major: 1, Minor: 0, Patch: 1
- Registered via Thunder metadata system for version-aware interface management
