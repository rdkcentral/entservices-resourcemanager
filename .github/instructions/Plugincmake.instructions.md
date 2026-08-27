---
description: Repo-specific CMake guidance for Thunder plugin build files in this project.
applyTo: "**/CMakeLists.txt"
---

# Plugin CMake Instructions (ResourceManager Pattern)

### 1) Core Naming and Target Model

### Requirement

Plugin CMake files in this repository follow a two-target model:

- Front plugin target: `${MODULE_NAME}` (`${NAMESPACE}${PLUGIN_NAME}`)
- Backend implementation target: `${PLUGIN_IMPLEMENTATION}` (`${MODULE_NAME}Implementation`)

Required naming pattern:

```cmake
set(PLUGIN_NAME ResourceManager)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)
```

Both targets are built as shared libraries and installed under:

```cmake
${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins
```

### 2) NAMESPACE Usage (Mandatory)

### Requirement

Inside plugin-level CMake files, use `${NAMESPACE}` for package discovery and target references. Do not hardcode namespace-specific package names in plugin CMake files.

This keeps the plugin compatible across namespace migrations.

### Correct Example

```cmake
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
    CompileSettingsDebug::CompileSettingsDebug
    ${NAMESPACE}Plugins::${NAMESPACE}Plugins
    ${NAMESPACE}Definitions::${NAMESPACE}Definitions)
```


### Incorrect Example

```cmake
set(MODULE_NAME Thunder${PLUGIN_NAME})

find_package(ThunderPlugins REQUIRED)
find_package(ThunderDefinitions REQUIRED)

target_link_libraries(${MODULE_NAME} 
    PRIVATE
    CompileSettingsDebug::CompileSettingsDebug
    ThunderPlugins::ThunderPlugins
    ThunderDefinitions::ThunderDefinitions)
```

### 3) Required Packages and Linking

### Requirement

Plugin CMake must include:

- `find_package(${NAMESPACE}Plugins REQUIRED)`
- `find_package(${NAMESPACE}Definitions REQUIRED)` for the front plugin target
- `find_package(CompileSettingsDebug CONFIG REQUIRED)`

Front plugin target (`${MODULE_NAME}`) links to:

- `CompileSettingsDebug::CompileSettingsDebug`
- `${NAMESPACE}Plugins::${NAMESPACE}Plugins`
- `${NAMESPACE}Definitions::${NAMESPACE}Definitions`

Implementation target (`${PLUGIN_IMPLEMENTATION}`) links to:

- `CompileSettingsDebug::CompileSettingsDebug`
- `${NAMESPACE}Plugins::${NAMESPACE}Plugins`
- optional extra libraries via `${PLUGIN_RESOURCE_MANAGER_EXTRA_LIBRARIES}`
- runtime feature-based libraries (`essosrmgr`, `rfcapi`) as applicable

### 4) Source and Compiler Settings

### Requirement

Maintain this source split:

- Front target sources: `ResourceManager.cpp`, `Module.cpp`
- Implementation target sources: `ResourceManagerImplementation.cpp`, `Module.cpp`

Both targets should keep C++11 settings:

```cmake
set_target_properties(<target> PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)
```

Front target keeps exception flags for `ResourceManager.cpp`:

```cmake
set_source_files_properties(ResourceManager.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")
```

### 5) Test and Feature Flags

### Requirement

Preserve conditional behavior used by this repo:

- `RDK_SERVICES_L1_TEST`:
    - include L1 test headers
    - define `ENABLE_L1TEST`
- `RDK_SERVICE_L2_TEST`:
    - optionally link `TestMocklib`
    - define `ENABLE_ERM`
- `BUILD_ENABLE_ERM`:
    - define `ENABLE_ERM`
    - link `essosrmgr`

RFC integration remains optional and guarded by discovery:

```cmake
find_library(RFC_LIBRARIES NAMES rfcapi)
if (RFC_LIBRARIES)
        find_path(RFC_INCLUDE_DIRS NAMES rfcapi.h REQUIRED)
        target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${RFC_INCLUDE_DIRS})
        target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${RFC_LIBRARIES})
        target_compile_definitions(${PLUGIN_IMPLEMENTATION} PRIVATE WITH_RFC)
endif ()
```

### 6) Config Generation and Install

### Requirement

At the end of plugin CMake, generate plugin config with:

```cmake
write_config(${PLUGIN_NAME})
```

Install both shared libraries with `install(TARGETS ...)` to the plugins destination.

### 7) Scope Note for Top-Level CMake

### Requirement

This instruction is primarily for plugin-level CMake updates. The repository root [CMakeLists.txt](../../CMakeLists.txt) still contains legacy `Thunder` package/config naming for project bootstrap and packaging variables.

Do not refactor those top-level legacy names unless explicitly requested as a dedicated migration task.


