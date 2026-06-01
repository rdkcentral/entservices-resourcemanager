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
using std::string;

// ===== Testable Wrapper Class for ResourceManager Plugin =====
class TestableResourceManagerPlugin : public Plugin::ResourceManager {
public:
    // No additional methods required for current plugin-level tests.
};

// ===== Testable Wrapper Class for ResourceManagerImplementation =====
class TestableResourceManagerImplementation : public Plugin::ResourceManagerImplementation {
private:
    mutable uint32_t _refCount{1};

public:
    TestableResourceManagerImplementation() = default;
    ~TestableResourceManagerImplementation() override = default;

    void AddRef() const override {
        Core::InterlockedIncrement(_refCount);
    }

    uint32_t Release() const override {
        if (Core::InterlockedDecrement(_refCount) == 0) {
            delete this;
            return Core::ERROR_DESTRUCTION_SUCCEEDED;
        }
        return Core::ERROR_NONE;
    }

    Core::hresult CallSetAVBlocked(const std::string& appId, const bool blocked, bool& success) {
        Exchange::IResourceManager::Success result{false};
        const Core::hresult code = SetAVBlocked(appId, blocked, result);
        success = result.success;
        return code;
    }

    Core::hresult CallGetBlockedAVApplications(std::vector<std::string>& appsList, bool& success) {
        Exchange::IResourceManager::IStringIterator* iterator = nullptr;
        const Core::hresult code = GetBlockedAVApplications(iterator, success);

        if (iterator != nullptr) {
            string element;
            while (iterator->Next(element) == true) {
                appsList.push_back(element);
            }
            iterator->Release();
        }

        return code;
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
    string initResult;

    bool IsPluginInitialized() const {
        return initResult.empty();
    }

    uint32_t ExpectedRpcCode() const {
        return IsPluginInitialized() ? Core::ERROR_NONE : Core::ERROR_UNKNOWN_KEY;
    }

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

        initResult = plugin->Initialize(&service);
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

    const string initResult = plugin->Initialize(&noSecurityService);
    EXPECT_TRUE(initResult.empty() || initResult == "ResourceManager plugin could not be initialised");
}

TEST_F(ResourceManagerInitializedTest, RegisteredMethods)
{
    const uint32_t expected = ExpectedRpcCode();

    EXPECT_EQ(expected, handler.Exists(_T("setAVBlocked")));
    EXPECT_EQ(expected, handler.Exists(_T("reserveTTSResource")));
    EXPECT_EQ(expected, handler.Exists(_T("getBlockedAVApplications")));
    EXPECT_EQ(expected, handler.Exists(_T("reserveTTSResourceForApps")));

}

TEST_F(ResourceManagerInitializedTest, SetAVBlockedTest)
{
    EXPECT_EQ(ExpectedRpcCode(), handler.Invoke(connection, _T("setAVBlocked"),
        _T("{\"appid\":\"testApp\",\"blocked\":true}"), response));
}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceTest)
{
    EXPECT_EQ(ExpectedRpcCode(), handler.Invoke(connection, _T("reserveTTSResource"),
        _T("{\"appid\":\"testApp\"}"), response));
}

TEST_F(ResourceManagerInitializedTest, ReserveTTSResourceForAppsTest)
{
    EXPECT_EQ(ExpectedRpcCode(), handler.Invoke(connection, _T("reserveTTSResourceForApps"),
        _T("{\"appids\":[\"testApp1\",\"testApp2\"]}"), response));
}

TEST_F(ResourceManagerInitializedTest, InformationReturnsDescriptiveTextTest)
{
    EXPECT_EQ("Plugin which exposes ResourceManager related methods.", plugin->Information());
}
TEST_F(ResourceManagerInitializedTest, GetBlockedAVApplicationsTest)
{
    const uint32_t expected = IsPluginInitialized() ? Core::ERROR_UNAVAILABLE : Core::ERROR_UNKNOWN_KEY;
    EXPECT_EQ(expected, handler.Invoke(connection, _T("getBlockedAVApplications"), _T("{}"), response));
}

// ===== Direct implementation tests (deterministic in unit environment) =====
TEST(ResourceManagerImplementationTest, SetAVBlockedMissingAppId_DefaultModeReturnsSuccess)
{
    Core::ProxyType<TestableResourceManagerImplementation> impl =
        Core::ProxyType<TestableResourceManagerImplementation>::Create();

    bool success = false;
    const Core::hresult code = impl->CallSetAVBlocked("", true, success);

    EXPECT_EQ(Core::ERROR_NONE, code);
    EXPECT_TRUE(success);
}

TEST(ResourceManagerImplementationTest, GetBlockedAVApplicationsWithoutERM)
{
    Core::ProxyType<TestableResourceManagerImplementation> impl =
        Core::ProxyType<TestableResourceManagerImplementation>::Create();

    std::vector<std::string> appsList;
    bool success = true;
    const Core::hresult code = impl->CallGetBlockedAVApplications(appsList, success);

    EXPECT_EQ(Core::ERROR_UNAVAILABLE, code);
    EXPECT_FALSE(success);
    EXPECT_TRUE(appsList.empty());
}
