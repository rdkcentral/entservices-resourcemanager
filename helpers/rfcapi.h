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

/**
 * RFC API Stub Header - Fallback implementation for RFC configuration access
 * 
 * This header provides a minimal RFC API for standalone builds. In production,
 * this should be replaced with the actual RFC library implementation from essosrmgr.
 *
 * Functions defined here use weak symbols to allow runtime substitution with
 * the real RFC library implementation if available.
 */

#ifndef _RFCAPI_H_
#define _RFCAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/* RFC Data Types */
typedef enum {
    WDMP_SUCCESS = 0,
    WDMP_ERR_DEFAULT_VALUE = 1,
    WDMP_ERR_GENERAL = 2
} WDMP_STATUS;

typedef enum {
    WDMP_NONE = 0,
    WDMP_STRING = 1,
    WDMP_INT = 2,
    WDMP_BOOLEAN = 3
} WDMP_TYPE;

typedef struct {
    WDMP_TYPE type;
    char value[256];
} RFC_ParamData_t;

/**
 * Get RFC Parameter - Fallback implementation
 * 
 * @param callerId: Caller identifier (usually nullptr)
 * @param paramName: Name of the RFC parameter to query
 * @param paramData: Output parameter data structure
 * 
 * @return WDMP_ERR_DEFAULT_VALUE to indicate default/disabled state
 * 
 * This fallback stub returns WDMP_ERR_DEFAULT_VALUE to allow the plugin
 * to safely initialize when the RFC library is not available.
 */
static inline WDMP_STATUS getRFCParameter(
    char* callerId,
    const char* paramName,
    RFC_ParamData_t* paramData)
{
    (void)callerId;
    (void)paramName;
    (void)paramData;
    return WDMP_ERR_DEFAULT_VALUE;
}

#ifdef __cplusplus
}
#endif

#endif /* _RFCAPI_H_ */
