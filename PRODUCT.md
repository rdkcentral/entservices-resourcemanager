# ResourceManager Plugin Product Documentation

## Product Overview
The ResourceManager plugin is a centralized resource management solution designed for RDK (Reference Design Kit) based set-top boxes, gateways, and media devices. It provides audio-video resource access control and text-to-speech (TTS) resource reservation capabilities through a standardized JSON-RPC API interface, enabling efficient multi-application resource coordination and conflict prevention in embedded device environments.

## Key Features

### Audio-Video Resource Management
- **Application Blacklisting**: Control which applications can access audio and video resources
- **Dynamic Resource Control**: Runtime addition/removal of applications from blacklist
- **Blacklist Monitoring**: Query current list of blocked applications
- **State Persistence**: Maintains blacklist status across plugin lifecycle
- **RFC-Based Enable/Disable**: Feature toggle through remote configuration

### Text-to-Speech (TTS) Resource Management
- **Single Application Reservation**: Reserve TTS resources for individual applications
- **Multi-Application Reservation**: Batch reservation for multiple applications simultaneously
- **Resource Conflict Prevention**: Coordinated access to shared TTS pipeline
- **RFC-Based Enable/Disable**: Feature toggle through remote configuration
- **Flexible Reservation Model**: Supports various TTS usage patterns

### Operational Capabilities
- **Essos Resource Manager Integration**: Leverages EssRMgr library for low-level resource control
- **Security Token Management**: Secure inter-plugin communication
- **RFC Configuration**: Runtime feature control through Remote Feature Control system
- **Graceful Degradation**: Continues operation with informative messages when features are disabled
- **State Tracking**: Maintains accurate resource allocation state

## API Interface

### Core Methods

#### `setAVBlocked`
**Purpose**: Block or unblock audio-video resource access for specific applications
**Parameters**:
```json
{
  "appid": "string",    // Application identifier/callsign
  "blocked": boolean    // true to block, false to unblock
}
```
**Response**:
```json
{
  "success": boolean
}
```
**Use Cases**:
- Prevent background apps from interfering with foreground A/V playback
- Implement parental controls or content restrictions
- Manage resource conflicts in multi-app environments
- Emergency resource reclamation for critical applications

**Behavior**:
- Updates EssRMgr blacklist immediately
- Maintains internal state cache for quick queries
- Requires Blacklist RFC feature to be enabled
- Returns informative message when RFC is disabled

#### `getBlockedAVApplications`
**Purpose**: Retrieve list of applications currently blocked from A/V resources
**Parameters**: None
**Response**:
```json
{
  "clients": ["appId1", "appId2", ...],
  "success": boolean
}
```
**Use Cases**:
- Monitoring current resource access restrictions
- Auditing resource management policies
- Diagnostic and troubleshooting scenarios
- System status dashboards

**Behavior**:
- Returns only applications currently marked as blocked
- Fast retrieval from in-memory state cache
- No EssRMgr query overhead

#### `reserveTTSResource`
**Purpose**: Reserve text-to-speech resources for a single application
**Parameters**:
```json
{
  "appid": "string"    // Application identifier requiring TTS access
}
```
**Response**:
```json
{
  "success": boolean
}
```
**Use Cases**:
- Voice assistant applications preparing for speech output
- Accessibility features requiring TTS access
- Interactive voice response systems
- Audio description services

**Behavior**:
- Coordinates TTS resource allocation through EssRMgr
- Prevents conflicts with other TTS consumers
- Requires ReserveTTS RFC feature to be enabled
- Returns success status immediately

#### `reserveTTSResourceForApps`
**Purpose**: Reserve TTS resources for multiple applications in batch operation
**Parameters**:
```json
{
  "appids": ["app1", "app2", "app3", ...]    // Array of application identifiers
}
```
**Response**:
```json
{
  "success": boolean
}
```
**Use Cases**:
- Multi-app voice interaction scenarios
- Concurrent accessibility features
- Complex voice UI with multiple components
- Batch initialization for TTS-enabled applications

**Behavior**:
- Processes all application reservations atomically
- Efficient batch operation reduces overhead
- All-or-nothing reservation semantics
- Requires ReserveTTS RFC feature to be enabled

## Configuration Management

### RFC Parameters

#### Blacklist Feature Control
**Parameter**: `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.Blacklist.Enable`
**Type**: Boolean
**Default**: Disabled (false)
**Impact**:
- When enabled: `setAVBlocked` and `getBlockedAVApplications` function normally
- When disabled: Methods return success with informative message, no resource changes

#### TTS Reservation Feature Control
**Parameter**: `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.ReserveTTS.Enable`
**Type**: Boolean
**Default**: Disabled (false)
**Impact**:
- When enabled: `reserveTTSResource` and `reserveTTSResourceForApps` function normally
- When disabled: Methods return success with informative message, no resource allocation

### Configuration Loading
- RFC parameters retrieved during plugin initialization
- Changes require plugin restart to take effect
- WDMP (Web Device Management Protocol) integration for parameter access
- Type-safe boolean parameter validation

## Security Features

### Access Control
- **Thunder Security Token**: Integration with SecurityAgent for secure communication
- **Token Generation**: Creates tokens for outbound inter-plugin calls
- **Payload Security**: Secure handling of authentication credentials
- **Error Handling**: Graceful degradation when SecurityAgent unavailable

### Resource Protection
- **Application-Level Control**: Fine-grained resource access management
- **State Validation**: Prevents invalid resource allocation attempts
- **Audit Trail**: Trackable resource management operations through logging

## Use Cases

### Multi-Application Audio-Video Management
- **Scenario**: Multiple streaming apps competing for A/V decoder resources
- **Solution**: Blacklist background apps when foreground app starts playback
- **Benefit**: Prevents decoder conflicts and ensures smooth user experience

### Voice Assistant Priority Management
- **Scenario**: Voice assistant needs exclusive TTS access during interaction
- **Solution**: Reserve TTS resource when assistant activates
- **Benefit**: Prevents TTS conflicts with other audio features

### Parental Control Integration
- **Scenario**: Restrict certain apps based on parental settings
- **Solution**: Dynamically add restricted apps to A/V blacklist
- **Benefit**: Runtime content control without app restart

### Accessibility Services
- **Scenario**: Screen reader needs reliable TTS access
- **Solution**: Reserve TTS resource for accessibility service
- **Benefit**: Ensures critical accessibility features always available

### Resource Conflict Resolution
- **Scenario**: Limited hardware decoders shared among apps
- **Solution**: Blacklist apps that exceed resource quotas
- **Benefit**: Fair resource distribution and system stability

## Integration Points

### Essos Resource Manager (EssRMgr)
- **Integration Type**: Native C library integration
- **API Used**: 
  - `EssRMgrCreate()`: Initialize resource manager
  - `EssRMgrDestroy()`: Clean up resource manager
  - `EssRMgrAddToBlackList()`: Block application A/V access
  - `EssRMgrRemoveFromBlackList()`: Restore application A/V access
- **Lifecycle**: Tied to plugin initialization/deinitialization
- **Error Handling**: Null checks and graceful failure modes

### RFC System Integration
- **Purpose**: Remote configuration and feature toggle
- **Protocol**: WDMP (Web Device Management Protocol)
- **Parameters**: Boolean feature flags
- **Validation**: Type checking and safe default handling

### Thunder Framework Integration
- **Plugin Registration**: Standard Thunder plugin lifecycle
- **JSON-RPC Interface**: Full Thunder dispatcher integration
- **Security Integration**: SecurityAgent for token management
- **Service Discovery**: Standard Thunder service registration

## Performance Characteristics

### Resource Usage
- **Memory Footprint**: Minimal - single instance with small state cache
- **CPU Utilization**: Low - synchronous operations with minimal processing
- **State Cache**: In-memory map for fast blacklist queries
- **No Persistence**: Stateless design, state not persisted across reboots

### Response Characteristics
- **Latency**: Sub-millisecond for most operations
- **Throughput**: Handles concurrent requests through Thunder dispatcher
- **Scalability**: Suitable for typical device application counts (<100 apps)
- **Reliability**: Direct EssRMgr integration ensures consistent state

## Deployment Considerations

### Build Requirements
- **ENABLE_ERM Flag**: Required for production builds with EssRMgr
- **Essos Library**: Must be available on target system
- **RFC Support**: rfcapi library for configuration management

### Runtime Requirements
- **Thunder Framework**: Running Thunder/WPEFramework instance
- **SecurityAgent**: Optional but recommended for security features
- **RFC Service**: For dynamic configuration support
- **EssRMgr**: Essos Resource Manager library loaded

### Feature Toggles
- Blacklist and TTS features independently controllable
- Default disabled for backward compatibility
- Enable through RFC parameters as needed per deployment

## Error Handling

### Graceful Degradation
- **EssRMgr Unavailable**: Returns error messages, no resource operations
- **RFC Disabled**: Returns success with informative messages
- **SecurityAgent Missing**: Continues without security token
- **Invalid Parameters**: Proper JSON-RPC error responses

### Logging and Diagnostics
- Comprehensive console logging for debugging
- Success/failure status for each operation
- RFC configuration status logged at initialization
- EssRMgr operation results logged

This centralized resource management approach ensures efficient hardware utilization, prevents resource conflicts, and provides flexible control mechanisms suitable for complex multi-application RDK device environments.
