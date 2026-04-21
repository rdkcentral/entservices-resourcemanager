/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2026 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#pragma once

#include <iostream>
#include <string>

#define LOGINFOMETHOD() { std::string json; parameters.ToString(json); std::cout << "params=" << json << std::endl; }
#define LOGTRACEMETHODFIN() { std::string json; response.ToString(json); std::cout << "response=" << json << std::endl; }

// "success" is kept for backward compatibility with existing ResourceManager APIs.
#define returnResponse(expression) \
    { \
        bool successBoolean = expression; \
        response["success"] = successBoolean; \
        LOGTRACEMETHODFIN(); \
        return (successBoolean ? WPEFramework::Core::ERROR_NONE : WPEFramework::Core::ERROR_GENERAL); \
    }
