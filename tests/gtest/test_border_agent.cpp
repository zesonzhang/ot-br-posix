/*
 *    Copyright (c) 2017, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

#include "border_agent/border_agent.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common/code_utils.hpp"
#include "common/mainloop_manager.hpp"
#include "mdns/mdns.hpp"

namespace otbr {

class MockMdnsPublisher : public Mdns::Publisher
{
public:
    MOCK_METHOD(otbrError,
                PublishServiceImpl,
                (const std::string &aHostName,
                 const std::string &aName,
                 const std::string &aType,
                 const SubTypeList &aSubTypeList,
                 uint16_t           aPort,
                 const TxtData     &aTxtData,
                 ResultCallback   &&aCallback),
                (override));
    MOCK_METHOD(void,
                UnpublishService,
                (const std::string &aName, const std::string &aType, ResultCallback &&aCallback),
                (override));
    MOCK_METHOD(otbrError,
                PublishHostImpl,
                (const std::string &aName, const AddressList &aAddresses, ResultCallback &&aCallback),
                (override));
    MOCK_METHOD(void, UnpublishHost, (const std::string &aName, ResultCallback &&aCallback), (override));
    MOCK_METHOD(otbrError,
                PublishKeyImpl,
                (const std::string &aName, const KeyData &aKey, ResultCallback &&aCallback),
                (override));
    MOCK_METHOD(void, UnpublishKey, (const std::string &aName, ResultCallback &&aCallback), (override));
    MOCK_METHOD(void, SubscribeService, (const std::string &aType, const std::string &aInstanceName), (override));
    MOCK_METHOD(void, UnsubscribeService, (const std::string &aType, const std::string &aInstanceName), (override));
    MOCK_METHOD(void, SubscribeHost, (const std::string &aHostName), (override));
    MOCK_METHOD(void, UnsubscribeHost, (const std::string &aHostName), (override));
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(bool, IsStarted, (), (const, override));

protected:
    void OnServiceResolveFailedImpl(const std::string &aType,
                                    const std::string &aInstanceName,
                                    int32_t            aErrorCode) override
    {
    }
    void OnHostResolveFailedImpl(const std::string &aHostName, int32_t aErrorCode) override {}
    otbrError DnsErrorToOtbrError(int32_t aError) override { return OTBR_ERROR_NONE; }
};

static const char kTestThreadVersion[] = "1.2.0";
static const char kTestServerName[]    = "TestServer";

class BorderAgentTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        mBorderAgent = std::make_unique<BorderAgent>(mMockPublisher);
        mBorderAgent->SetEnabled(true);
    }

    void TearDown() override { mBorderAgent->SetEnabled(false); }

protected:
    std::unique_ptr<BorderAgent> mBorderAgent;
    MockMdnsPublisher            mMockPublisher;
    std::string                  mServiceInstanceName;
};

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;

TEST_F(BorderAgentTest, TestEnableAndPublishService)
{
    const char          kServiceName[] = "otbr-meshcop";
    const char          kServiceType[] = "_meshcop._udp";
    const uint16_t      kServicePort   = 12345;
    BorderAgent::TxtData kTxtData;

    EXPECT_CALL(mMockPublisher, PublishServiceImpl(_, _, StrEq(kServiceType), _, kServicePort, _, _))
        .WillOnce(DoAll(SaveArg<1>(&mServiceInstanceName), Return(OTBR_ERROR_NONE)));
    mBorderAgent->SetMeshCoPServiceValues(kServiceName, "", "", {}, {});
    mBorderAgent->HandleMdnsState(Mdns::Publisher::State::kReady);
    mBorderAgent->HandleBorderAgentMeshCoPServiceChanged(true, kServicePort, kTxtData);
    ASSERT_EQ(mServiceInstanceName, kServiceName);
}

TEST_F(BorderAgentTest, TestDisableAndUnpublishService)
{
    const char          kServiceName[] = "otbr-meshcop";
    const char          kServiceType[] = "_meshcop._udp";
    const uint16_t      kServicePort   = 12345;
    BorderAgent::TxtData kTxtData;

    EXPECT_CALL(mMockPublisher, PublishServiceImpl(_, _, StrEq(kServiceType), _, kServicePort, _, _))
        .WillOnce(DoAll(SaveArg<1>(&mServiceInstanceName), Return(OTBR_ERROR_NONE)));
    mBorderAgent->SetMeshCoPServiceValues(kServiceName, "", "", {}, {});
    mBorderAgent->HandleMdnsState(Mdns::Publisher::State::kReady);
    mBorderAgent->HandleBorderAgentMeshCoPServiceChanged(true, kServicePort, kTxtData);
    ASSERT_EQ(mServiceInstanceName, kServiceName);

    EXPECT_CALL(mMockPublisher, UnpublishService(mServiceInstanceName, StrEq(kServiceType), _));
    mBorderAgent->SetEnabled(false);
}

TEST_F(BorderAgentTest, TestSetMeshCopServiceValues)
{
    const char          kServiceName[] = "MyTestService";
    const char          kServiceType[] = "_meshcop._udp";
    const uint16_t      kServicePort   = 54321;
    BorderAgent::TxtData kTxtData;

    EXPECT_CALL(mMockPublisher, PublishServiceImpl(_, _, StrEq(kServiceType), _, kServicePort, _, _))
        .WillOnce(DoAll(SaveArg<1>(&mServiceInstanceName), Return(OTBR_ERROR_NONE)));
    mBorderAgent->SetMeshCoPServiceValues(kServiceName, "", "", {}, {});
    mBorderAgent->HandleMdnsState(Mdns::Publisher::State::kReady);
    mBorderAgent->HandleBorderAgentMeshCoPServiceChanged(true, kServicePort, kTxtData);
    ASSERT_EQ(mServiceInstanceName, kServiceName);
}

} // namespace otbr