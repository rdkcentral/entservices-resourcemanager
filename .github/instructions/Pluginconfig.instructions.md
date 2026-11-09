---
description: Repo-specific plugin configuration guidance for ResourceManager-style .conf.in and .config files.
applyTo: "**/*.config,**/*.conf.in"
---

# Plugin Configuration Instructions (ResourceManager Pattern)

### 1) Required Files and Roles

### Requirement

Each plugin configuration flow in this repository uses two files:

- `<PluginName>.conf.in`: template consumed by build/config tooling.
- `<PluginName>.config`: generated/maintained runtime configuration in map()/kv() style.

For this plugin, both files must stay aligned on key values such as callsign, mode, and locator.

### 2) Mandatory Fields

### Requirement

Each plugin must define these properties:

- autostart: Should reference the CMake variable and default to false in CMake cache.

- callsign: Must use org.rdk prefix and service name in PascalCase.
  - ResourceManager value: org.rdk.ResourceManager.

- precondition/preconditions: Declare required subsystems (ResourceManager uses Platform).

- startuporder: Optional; include only when configured.

- mode: Must be passed through from CMake variable.

- locator: Must point to backend implementation shared library using PLUGIN_IMPLEMENTATION.

### 3) Mode Determination

### Requirement

Mode behavior in this repo follows Thunder conventions and current plugin CMake defaults:

- Off means in-process.
- Unset mode defaults to in-process.
- Local means out-of-process.

Mode value is configured in plugin CMake through:

PLUGIN_RESOURCE_MANAGER_MODE

### 4) Required Root Configuration Shape

### Requirement

Both .conf.in and .config must define a root configuration object/map containing:

- mode
- locator

For ResourceManager:

- mode uses PLUGIN_RESOURCE_MANAGER_MODE
- locator uses lib${PLUGIN_IMPLEMENTATION}.so

### 5) ResourceManager Example (.conf.in)

ResourceManager.conf.in

```ini
precondition = ["Platform"]
callsign = "org.rdk.ResourceManager"
autostart = "@PLUGIN_RESOURCE_MANAGER_AUTOSTART@"
startuporder = "@PLUGIN_RESOURCE_MANAGER_STARTUPORDER@"

configuration = JSON()
rootobject = JSON()

rootobject.add("mode", "@PLUGIN_RESOURCE_MANAGER_MODE@")
rootobject.add("locator", "lib@PLUGIN_IMPLEMENTATION@.so")

configuration.add("root", rootobject)
```

### 6) ResourceManager Example (.config)

ResourceManager.config

```cmake
set (autostart ${PLUGIN_RESOURCE_MANAGER_AUTOSTART})
set (preconditions Platform)
set (callsign "org.rdk.ResourceManager")

if(PLUGIN_RESOURCE_MANAGER_STARTUPORDER)
set (startuporder ${PLUGIN_RESOURCE_MANAGER_STARTUPORDER})
endif()

map()
    key(root)
    map()
        kv(mode ${PLUGIN_RESOURCE_MANAGER_MODE})
        kv(locator lib${PLUGIN_IMPLEMENTATION}.so)
    end()
end()
ans(configuration)
```

### 7) CMake Variable Contract

### Requirement

Plugin config files must be consistent with plugin CMake variables:

- PLUGIN_RESOURCE_MANAGER_AUTOSTART
- PLUGIN_RESOURCE_MANAGER_STARTUPORDER
- PLUGIN_RESOURCE_MANAGER_MODE
- PLUGIN_IMPLEMENTATION

These are defined in plugin/CMakeLists.txt and consumed via write_config(PLUGIN_NAME).

### 8) Validation Checklist

### Requirement

When updating any plugin config template in this repo, verify:

- callsign format and value are correct.
- precondition/preconditions includes Platform when required.
- root.mode and root.locator are both present.
- locator uses implementation library name, not front plugin library.
- startuporder is optional and conditionally emitted in .config.
- .conf.in placeholders and .config variables use matching names.

