# ResourceManager Plugin - Product Documentation

## Product Overview

ResourceManager is a WPEFramework plugin that provides fine-grained media resource control for RDK-based devices. The plugin enables per-application AV output blocking and Text-to-Speech (TTS) resource reservation through the Essos Resource Manager (ERM), ensuring fair resource allocation across competing applications and preventing unauthorized media output.

## Key Features

### AV Output Blocking
- **Per-Application Control**: Block or allow audio/video output on a per-application basis
- **Real-time Enforcement**: Immediate AV blocking through ERM blacklist mechanism
- **Status Tracking**: Query list of currently blocked applications
- **RFC-Gated Feature**: Blacklist functionality toggled via RFC parameter (Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.Blacklist.Enable)

### TTS Resource Reservation
- **Resource Reservation**: Reserve and release Text-to-Speech resources to prevent conflicts
- **Multi-App Support**: Reserve resources for single or multiple applications simultaneously
- **RFC-Gated Feature**: TTS reservation controlled via RFC parameter (Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.ReserveTTS.Enable)
- **Automatic Release**: Resources released when application context ends

### Security & Integration
- **Security Token Integration**: Acquires security tokens from SecurityAgent plugin for authorized operations
- **JSONRPC Interface**: Standard JSON-RPC based API for seamless integration with RDK middleware
- **In-Process Execution**: Runs directly within Thunder framework for minimal latency

## Use Cases and Target Scenarios

### Parental Controls and Content Protection
**Scenario**: A set-top box needs to enforce parental controls that block AV output from specific applications during restricted times.

**Solution**:
- Use setAVBlocked() to block AV from restricted applications
- Query getBlockedAVApplications() to maintain UI state
- RFC feature flag can disable feature remotely if business logic changes

**Benefits**: Secure, hardware-enforced blocking through ERM

### Priority-Based TTS Resource Management
**Scenario**: Multiple applications compete for TTS resources; system services require guaranteed TTS availability.

**Solution**:
- Call reserveTTSResource() for critical system applications
- Use reserveTTSResourceForApps() to batch-reserve for multiple services
- Other applications gracefully degrade when resources unavailable

**Benefits**: Prevents TTS starvation of system services

### Multi-User Device Isolation
**Scenario**: A multi-user device needs to restrict media output based on user profile and time-of-day policies.

**Solution**:
- Maintain application-to-user mappings at application layer
- Use setAVBlocked() to enforce per-user blacklists
- SecurityAgent token ensures only authorized entities can modify blocking state

**Benefits**: Hardware-enforced user content separation

### Premium Application QoS
**Scenario**: Premium streaming applications require guaranteed TTS availability; lesser apps degrade gracefully.

**Solution**:
- Structure application initialization to call reserveTTSResource() for premium apps
- Non-premium applications check reservation status before attempting TTS operations
- Fail gracefully if resources unavailable

**Benefits**: Tiered quality-of-service guarantees

## API Capabilities and Integration

### JSON-RPC API Methods

#### AV Blocking Operations
```json
// Block/unblock AV output for an application
{
  "jsonrpc": "2.0",
  "method": "org.rdk.ResourceManager.1.setAVBlocked",
  "params": {
    "appid": "netflix",
    "blocked": true
  }
}

// Query blocked applications list
{
  "jsonrpc": "2.0",
  "method": "org.rdk.ResourceManager.1.getBlockedAVApplications",
  "params": {}
}
```

#### TTS Resource Reservation
```json
// Reserve TTS resource for single application
{
  "jsonrpc": "2.0",
  "method": "org.rdk.ResourceManager.1.reserveTTSResource",
  "params": {
    "appid": "voicecontrol"
  }
}

// Reserve TTS resources for multiple applications
{
  "jsonrpc": "2.0",
  "method": "org.rdk.ResourceManager.1.reserveTTSResourceForApps",
  "params": {
    "appids": ["voicecontrol", "accessibility"]
  }
}
```

### C++ Integration (Native)
```cpp
// Get ResourceManager interface
auto resourceMgr = service->QueryInterfaceByCallsign<Exchange::IResourceManager>(
    "org.rdk.ResourceManager");

// Set AV blocked status
bool blocked = true;
uint32_t result = resourceMgr->setAVBlocked(appId, blocked);
```

## Technical Integration Points

### Deployment Configuration
- **Autostart**: Configured to autostart with Thunder framework
- **In-Process Mode**: Executes within Thunder process (no separate process overhead)
- **ERM Dependency**: Requires libessosrmgr for AV blocking; gracefully disables if unavailable

### RFC Feature Flags
Two independent RFC parameters control feature availability:

| Parameter | Default | Purpose |
|-----------|---------|---------|
| Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.Blacklist.Enable | Disabled | Controls AV blocking feature |
| Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.ReserveTTS.Enable | Disabled | Controls TTS reservation feature |

### External Dependencies
- **libessosrmgr**: Essos Resource Manager library for hardware-level resource enforcement
- **SecurityAgent**: RDK plugin providing security token generation
- **WPEFramework**: Core plugin framework and JSONRPC infrastructure

## Version Information

- **API Version**: 1.0.1
- **Service Name**: org.rdk.ResourceManager
- **Interface**: IPlugin + JSONRPC
- **Execution Mode**: In-process (Thunder framework)
