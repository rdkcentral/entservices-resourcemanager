/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2024 RDK Management
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

#if defined(RDK_SERVICES_L1_TEST) || defined(RDK_SERVICE_L2_TEST)
// L1/L2 test builds inject mock RFC symbols via compiler -include; avoid fallback header conflicts.
#else
#include "rfcapi.h"
#endif

namespace Utils {
inline bool getRFCConfig(const char* paramName, RFC_ParamData_t& paramOutput)
{
    WDMP_STATUS wdmpStatus = getRFCParameter(nullptr, paramName, &paramOutput);
    if (wdmpStatus == WDMP_SUCCESS || wdmpStatus == WDMP_ERR_DEFAULT_VALUE) {
        return true;
    }
    return false;
}
}
