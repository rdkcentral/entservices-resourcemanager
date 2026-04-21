# ResourceManager Plugin Architecture

## Overview
The ResourceManager plugin is a C++ implementation built on the WPEFramework (Thunder) platform that provides centralized resource management capabilities for RDK-based devices. It manages audio/video resource access control and text-to-speech (TTS) resource allocation through integration with the Essos Resource Manager (EssRMgr) library. The plugin follows a layered architecture with clear separation between the Thunder plugin interface, business logic, and low-level resource management operations.

## System Architecture

### Core Components

#### 1. Plugin Layer (`ResourceManager.h/cpp`)
- **Purpose**: Thunder plugin interface and JSON-RPC endpoint
- **Responsibilities**:
  - Plugin lifecycle management (Initialize, Deinitialize)
  - JSON-RPC method registration and routing
  - Security token management for inter-plugin communication
  - Request parameter validation and error handling
  - Response formatting and status reporting
- **Key Classes**:
  - `ResourceManager`: Main plugin class implementing `IPlugin` and JSON-RPC interfaces
  - Singleton pattern implementation for global access
- **API Methods**:
  - `setAVBlocked`: Block/unblock audio-video resources for specific applications
  - `getBlockedAVApplications`: Retrieve list of blocked applications
  - `reserveTTSResource`: Reserve TTS resources for a single application
  - `reserveTTSResourceForApps`: Reserve TTS resources for multiple applications

#### 2. Resource Management Layer
- **Purpose**: Integration with Essos Resource Manager for low-level resource control
- **Responsibilities**:
  - EssRMgr lifecycle management (creation and destruction)
  - Application blacklist management for A/V resources
  - TTS resource reservation coordination
  - Resource state tracking and synchronization
- **Key Components**:
  - `mEssRMgr`: Central EssRMgr instance handle
  - `mAppsAVBlacklistStatus`: In-memory cache of application blacklist states

#### 3. Configuration Layer
- **Purpose**: RFC-based configuration management
- **Responsibilities**:
  - Feature enable/disable control via RFC parameters
  - Runtime configuration loading during initialization
  - Dynamic behavior modification based on RFC settings
- **RFC Parameters**:
  - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.Blacklist.Enable`
  - `Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.Resourcemanager.ReserveTTS.Enable`

#### 4. Module Layer (`Module.h/cpp`)
- **Purpose**: Plugin module definition and Thunder framework integration
- **Responsibilities**:
  - Module registration with Thunder framework
  - Plugin metadata versioning (Major: 1, Minor: 0, Patch: 1)
  - Build system integration

#### 5. Helper Utilities (`helpers/`)
- **Purpose**: Reusable utility components
- **Components**:
  - `UtilsgetRFCConfig.h`: RFC parameter retrieval and validation
  - `UtilsJsonRpc.h`: JSON-RPC helper functions and macros

## Architecture Patterns

### Layered Architecture
The system follows a three-tier architecture:
1. **Presentation Layer**: JSON-RPC interface for external communication and request handling
2. **Business Logic Layer**: Resource management operations, state management, and blacklist control
3. **Integration Layer**: EssRMgr integration, RFC configuration, and security token management

### Singleton Pattern
Centralized plugin instance management:
- Single global instance accessible via `ResourceManager::_instance`
- Thread-safe access to shared resource management state
- Consistent state across all client connections

### Conditional Compilation Pattern
Feature-based compilation for flexibility:
- `ENABLE_ERM`: Enables EssRMgr integration for production builds
- `ENABLE_L1TEST`: Enables test-specific functionality
- Graceful degradation when features are disabled

## Interface Architecture

### JSON-RPC Interface
**Endpoint**: `org.rdk.ResourceManager`
**Version**: 1.0.1

**Methods**:

#### `setAVBlocked`
- **Purpose**: Control audio-video resource access for applications
- **Parameters**:
  - `appid` (string): Application identifier/callsign
  - `blocked` (boolean): Block (true) or unblock (false) A/V resources
- **Functionality**: Updates EssRMgr blacklist and maintains internal state cache

#### `getBlockedAVApplications`
- **Purpose**: Retrieve list of currently blocked applications
- **Response**: Array of application identifiers with blocked A/V resources
- **Use Case**: Monitoring and auditing resource access control

#### `reserveTTSResource`
- **Purpose**: Reserve TTS resources for a single application
- **Parameters**:
  - `appid` (string): Application identifier requiring TTS access
- **Functionality**: Coordinates TTS resource allocation through EssRMgr

#### `reserveTTSResourceForApps`
- **Purpose**: Reserve TTS resources for multiple applications simultaneously
- **Parameters**:
  - `appids` (array of strings): List of application identifiers
- **Functionality**: Batch TTS resource reservation for efficient multi-app scenarios

### Thunder Framework Integration
- **IPlugin Interface**: Standard Thunder plugin lifecycle management
- **IDispatcher Interface**: JSON-RPC method routing and invocation
- **IAuthenticate Interface**: Security token generation for secure communication

## Security Architecture

### Access Control
- Thunder framework security token integration
- SecurityAgent integration for token-based authentication
- Secure inter-plugin communication channels

### Resource Protection
- Application-level resource access control through blacklisting
- Centralized authorization for critical A/V and TTS resources
- State validation before resource allocation

## Resource Management Model

### Audio-Video Resource Control
- **Blacklist Mechanism**: Prevent specific applications from accessing A/V resources
- **Add to Blacklist**: `EssRMgrAddToBlackList` - Blocks application A/V access
- **Remove from Blacklist**: `EssRMgrRemoveFromBlackList` - Restores application A/V access
- **State Persistence**: In-memory tracking of blacklist status per application

### TTS Resource Management
- **Single Application Reservation**: Reserve TTS for individual app contexts
- **Multi-Application Reservation**: Batch reservation for concurrent TTS requirements
- **Resource Coordination**: Prevents TTS resource conflicts between applications

## Configuration Architecture

### RFC-Based Feature Control
- **Blacklist Feature Toggle**: Runtime enable/disable of A/V blacklist functionality
- **TTS Reservation Toggle**: Runtime enable/disable of TTS resource management
- **Default Behavior**: Features disabled by default for backward compatibility
- **Configuration Loading**: RFC parameters read during plugin initialization

### Graceful Degradation
- Non-fatal error handling when EssRMgr is unavailable
- Informative error messages when RFC features are disabled
- Conditional execution paths based on feature availability

## Build Architecture

### Compilation Options
- **ENABLE_ERM**: Activates EssRMgr integration for production environments
- **ENABLE_L1TEST**: Includes test harness support and mock interfaces
- **Conditional Dependencies**: Essos library linked only when ERM is enabled

### CMake Build System
- Modular component building with clear dependency management
- Cross-platform compatibility for various RDK device types
- Integrated L1/L2 testing framework support

## Threading Model

### Synchronous Operations
- JSON-RPC method handlers execute synchronously
- State updates are immediately consistent
- Blocking operations limited to fast EssRMgr calls

### Thread Safety Considerations
- Single-threaded plugin instance access pattern
- State cache protected by Thunder framework dispatcher
- No explicit mutex requirements for current implementation

## Integration Points

### Essos Resource Manager (EssRMgr)
- Low-level resource management library integration
- Native C API for blacklist and resource operations
- Lifecycle management (create/destroy) tied to plugin lifecycle

### RFC System Integration
- Remote Feature Control for persistent configuration
- Runtime parameter retrieval through WDMP (Web Device Management Protocol)
- Boolean parameter validation and type safety

### Thunder Security Framework
- SecurityAgent integration for token generation
- Token-based authentication for inter-plugin calls
- Secure payload handling for external communication

This architecture ensures efficient resource management, secure access control, and flexible configuration while maintaining compatibility with the broader Thunder/RDK ecosystem.
