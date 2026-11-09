---
description: Repo-specific guidance for plugin module declaration files.
applyTo: "**/Module.cpp,**/Module.h"
---


# Module Instructions (ResourceManager Pattern)

### 1) Module Name Convention

### Requirement

- Every plugin must define MODULE_NAME because Thunder uses it to identify the plugin.
- Every plugin must also define MODULE_NAME_DECLARATION() macro since it generates identifiers such as the module name string, SHA value, and version for the module, enabling the system to recognize and link it.
- The MODULE_NAME should always start with the prefix Plugin_.
- For this repository plugin, module name is `Plugin_ResourceManager`.

### 2) Module.h Requirements

### Requirement

In Module.h:

- Use `#pragma once`.
- Guard and define `MODULE_NAME` only when not already defined.
- Include plugin and tracing headers used by module-wide code.
- Keep `EXTERNAL` visibility macro pattern used in this repo.

### Example

1. In Module.h:

   ```cpp
   #pragma once
   #ifndef MODULE_NAME
   #define MODULE_NAME Plugin_ResourceManager
   #endif

   #include <plugins/plugins.h>
   #include <tracing/tracing.h>

   #undef EXTERNAL
   #define EXTERNAL
   ```

### 3) Module.cpp Requirements

### Requirement

In Module.cpp:

- Include only `Module.h`.
- Declare module metadata once using `MODULE_NAME_DECLARATION(BUILD_REFERENCE)`.
- Do not duplicate module name definitions in Module.cpp.

2. In Module.cpp:

   ```cpp
   #include "Module.h"

   MODULE_NAME_DECLARATION(BUILD_REFERENCE)
   ```

### 4) Validation Checklist

### Requirement

When updating module files, verify:

- `MODULE_NAME` starts with `Plugin_`.
- `MODULE_NAME` matches plugin identity (`Plugin_ResourceManager` for this repo).
- `MODULE_NAME_DECLARATION(BUILD_REFERENCE)` exists exactly once in Module.cpp.
- Module.h retains plugin/tracing includes and EXTERNAL macro pattern.