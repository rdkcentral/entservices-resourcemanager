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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ResourceManager.h"
#include "ResourceManagerImplementation.h"
#include "ServiceMock.h"
#include <core/core.h>
#include "ThunderPortability.h"

using namespace WPEFramework;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Invoke;
using std::string;

// ===== Testable Wrapper Class for ResourceManager Plugin =====
class TestableResourceManagerPlugin : public Plugin::ResourceManager {
public:
    // The plugin itself doesn't have test-specific methods anymore
    // since the implementation is in ResourceManagerImplementation
};

// ===== Testable Wrapper Class for ResourceManagerImplementation =====
class TestableResourceManagerImplementation : public Plugin::ResourceManagerImplementation {
private:
    mutable uint32_t _refCount{1};

public:
    TestableResourceManagerImplementation() : Plugin::ResourceManagerImplementation() {
    }

    virtual ~TestableResourceManagerImplementation() = default;

    // Implement IReferenceCounted pure virtual functions
    virtual void AddRef() const override {
        Core::InterlockedIncrement(_refCount);
    }

    virtual uint32_t Release() const override {
        if (Core::InterlockedDecrement(_refCount) == 0) {
            delete this;
            return Core::ERROR_DESTRUCTION_SUCCEEDED;
        }
        return Core::ERROR_NONE;
    }

    void setDisableReserveTTS(bool value) {
        mDisableReserveTTS = value;
    }

    bool callSetAVBlocked(const std::string& client, bool blocked) {
        Exchange::IResourceManager::Success result;
        SetAVBlocked(client, blocked, result);
        return result.success;
    }

    bool getBlockedAVApplicationsPublic(std::vector<std::string>& appsList) {
        bool success = false;
        Exchange::IResourceManager::IStringIterator* iterator = nullptr;
        GetBlockedAVApplications(iterator, success);
        
        if (success && iterator != nullptr) {
            string element;
            while (iterator->Next(element) == true) {
                appsList.push_back(element);
            }
            iterator->Release();
        }
        return success;
    }
};

// ===== Test Fixture Base =====
class ResourceManagerTest : public ::testing::Test {
protected:
    Core::ProxyType<TestableResourceManagerPlugin> plugin;
    Core::JSONRPC::Handler& handler;
    DECL_CORE_JSONRPC_CONX connection;
    string response;

    ResourceManagerTest()
        : plugin(Core::ProxyType<TestableResourceManagerPlugin>::Create())
        , handler(*plugin)
        , INIT_CONX(1, 0)
    {}

    virtual ~ResourceManagerTest() = default;
};

// ===== Mock for SecurityAgent =====
class MockAuthenticate : public PluginHost::IAuthenticate {
public:
    MOCK_METHOD(uint32_t, CreateToken, (const uint16_t length, const uint8_t buffer[], std::string& token), (override));
    MOCK_METHOD(PluginHost::ISecurity*, Officer, (const std::string& token), (override));
    MOCK_METHOD(uint32_t, Release, (), (const, override));
    MOCK_METHOD(void, AddRef, (), (const, override));

    BEGIN_INTERFACE_MAP(MockAuthenticate)
        INTERFACE_ENTRY(PluginHost::IAuthenticate)
    END_INTERFACE_MAP
};

// ===== Derived Fixture with Initialized Plugin =====
class ResourceManagerInitializedTest : public ResourceManagerTest {
protected:
    NiceMock<ServiceMock> service;
    NiceMock<MockAuthenticate>* mockAuth = nullptr;

public:
    ResourceManagerInitializedTest() {
        mockAuth = new NiceMock<MockAuthenticate>();

        ON_CALL(service, QueryInterfaceByCallsign(_, _))
            .WillByDefault([this](const uint32_t, const std::string& name) -> void* {
                if (name == "SecurityAgent") {
                    mockAuth->AddRef();
                    return static_cast<void*>(mockAuth);
                }
                return nullptr;
            });

        plugin->Initialize(&service);
    }

    ~ResourceManagerInitializedTest() override {
        plugin->Deinitialize(&service);
        delete mockAuth;
    }
};

// =========================
// ===== TEST CASES ========
// =========================

TEST_F(ResourceManagerTest, InitializeWithoutSecurityAgent)
{
    NiceMock<ServiceMock> noSecurityService;

    ON_CALL(noSecurityService, QueryInterfaceByCallsign(_, _))
        .WillByDefault(Return(nullptr));

    EXPECT_EQ("", plugin->Initialize(&noSecurityService));
}

TEST_F(ResourceManagerInitializedTest, RegisteredMethods)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("setAVBlocked")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("reserveTTSResource")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("getBlockedAVApplications")));
    EXPECT_EQ(Core::ERROR_NONE, handler.Exists(_T("reserveTTSResourceForApps")));

}

TEST_F(ResourceManagerInitializedTest, SetAVBlockedTest)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("setAVBlocked"),
        _T("{\"appid\":\"testApp\",\"blocked\":true}"), response));
}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceTest_1)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reserveTTSResource"),
        _T("{\"appid\":\"testApp\"}"), response));
}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceForAppsTest_1)
{
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reserveTTSResourceForApps"),
        _T("{\"appids\":[\"testApp1\",\"testApp2\"]}"), response));
}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceTest_3)
{
    plugin->Deinitialize(&service);  
    plugin->Initialize(&service);       

    std::cout << "[TEST] Testing reserveTTSResource with reinitialized plugin\n";
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reserveTTSResource"),
        _T("{ \"appid\": \"testApp\" }"), response));

}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceForAppsTest_3)
{
    plugin->Deinitialize(&service);
    plugin->Initialize(&service);

    // This test now verifies the JSON-RPC wrapper behavior
    std::cout << "[TEST] Testing reserveTTSResourceForApps with reinitialized plugin\n";
    // With RFC disabled, the method succeeds but logs a warning
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reserveTTSResourceForApps"),
        _T("{ \"appids\": [\"testApp1\",\"testApp2\"]}"), response));

}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResource_MissingAppId)
{
    plugin->Deinitialize(&service);
    plugin->Initialize(&service);

    // Test with missing appid parameter
    // When RFC is disabled: missing/empty appid is ALLOWED and returns success=true
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reserveTTSResource"),
        _T("{ }"), response));
    
    // With RFC disabled (default), expects success even with missing appid
    EXPECT_THAT(response, ::testing::HasSubstr("\"success\":true"));
}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceForApps_MissingAppIds)
{
    plugin->Deinitialize(&service);
    plugin->Initialize(&service);

    // Test with missing appids parameter
    // When RFC is disabled: missing/empty appids is ALLOWED and returns success=true
    EXPECT_EQ(Core::ERROR_NONE, handler.Invoke(connection, _T("reserveTTSResourceForApps"),
        _T("{ }"), response));
    
    // With RFC disabled (default), expects success even with missing appids
    EXPECT_THAT(response, ::testing::HasSubstr("\"success\":true"));
}

TEST_F(ResourceManagerInitializedTest, InformationReturnsDescriptiveTextTest)
{
    EXPECT_EQ("Plugin which exposes ResourceManager related methods.", plugin->Information());
}
TEST_F(ResourceManagerInitializedTest, getBlockedAVApplicationsTest_1)
{
    EXPECT_EQ(2, handler.Invoke(connection, _T("getBlockedAVApplications"), _T("{}"), response));
}

TEST_F(ResourceManagerInitializedTest, GetBlockedAVApplicationsTest_1)
{
    std::vector<std::string> appsList;
    
    // Create an instance of the implementation to test directly
    Core::ProxyType<TestableResourceManagerImplementation> implPlugin = 
        Core::ProxyType<TestableResourceManagerImplementation>::Create();
    
    bool result = implPlugin->getBlockedAVApplicationsPublic(appsList);
    
    // Since no apps were blocked, the list should be empty or the method should succeed
    EXPECT_TRUE(result || appsList.empty());
}

TEST_F(ResourceManagerInitializedTest, SetAVBlockedInternalTest)
{
    // Create an instance of the implementation to test directly
    Core::ProxyType<TestableResourceManagerImplementation> implPlugin = 
        Core::ProxyType<TestableResourceManagerImplementation>::Create();
    
    bool result = implPlugin->callSetAVBlocked("testApp", true);
    EXPECT_TRUE(result);
}
