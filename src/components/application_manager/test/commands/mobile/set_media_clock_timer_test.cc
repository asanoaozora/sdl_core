/*
 * Copyright (c) 2016, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <string>

#include "application_manager/commands/mobile/set_media_clock_timer_request.h"

#include "gtest/gtest.h"
#include "application_manager/commands/command_request_test.h"
#include "application_manager/mock_application_manager.h"
#include "application_manager/mock_application.h"
#include "application_manager/mock_message_helper.h"
#include "application_manager/event_engine/event.h"
#include "application_manager/mock_hmi_interface.h"

namespace test {
namespace components {
namespace commands_test {
namespace mobile_commands_test {
namespace set_media_clock_timer_request {

namespace am = ::application_manager;
using am::commands::SetMediaClockRequest;
using am::commands::MessageSharedPtr;
using am::event_engine::Event;
using am::MockMessageHelper;
using ::testing::_;
using ::testing::Mock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace UpdateMode = mobile_apis::UpdateMode;

typedef SharedPtr<SetMediaClockRequest> SetMediaClockRequestPtr;

namespace {
const uint32_t kConnectionKey = 2u;
const uint32_t kCorrelationId = 2u;
const uint32_t kAppID = 2u;
const uint32_t kHours = 2u;
const uint32_t kMinutes = 26u;
const uint32_t kSeconds = 1u;
}  // namespace

class SetMediaClockRequestTest
    : public CommandRequestTest<CommandsTestMocks::kIsNice> {
 public:
  SetMediaClockRequestTest()
      : mock_message_helper_(*MockMessageHelper::message_helper_mock())
      , mock_app_(CreateMockApp()) {}

  void SetUp() OVERRIDE {
    ON_CALL(app_mngr_, application(kConnectionKey))
        .WillByDefault(Return(mock_app_));
    ON_CALL(*mock_app_, app_id()).WillByDefault(Return(kConnectionKey));
  }

  void TearDown() OVERRIDE {
    Mock::VerifyAndClearExpectations(&mock_message_helper_);
  }

  void ResultCommandExpectations(MessageSharedPtr msg,
                                 const std::string& info) {
    EXPECT_EQ((*msg)[am::strings::msg_params][am::strings::success].asBool(),
              true);
    EXPECT_EQ(
        (*msg)[am::strings::msg_params][am::strings::result_code].asInt(),
        static_cast<int32_t>(hmi_apis::Common_Result::UNSUPPORTED_RESOURCE));
    EXPECT_EQ((*msg)[am::strings::msg_params][am::strings::info].asString(),
              info);
  }

  MessageSharedPtr CreateMsgParams() {
    MessageSharedPtr msg = CreateMessage();
    (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;
    return msg;
  }

  void ExpectationsSetupHelper(bool is_media) {
    EXPECT_CALL(app_mngr_, application(kConnectionKey))
        .WillOnce(Return(mock_app_));
    EXPECT_CALL(*mock_app_, is_media_application()).WillOnce(Return(is_media));
    EXPECT_CALL(*mock_app_, app_id()).Times(0);
    EXPECT_CALL(app_mngr_, ManageMobileCommand(_, _));
  }

  MockMessageHelper& mock_message_helper_;
  MockAppPtr mock_app_;
};

TEST_F(SetMediaClockRequestTest,
       OnEvent_UIHmiSendUnsupportedResource_UNSUPPORTED_RESOURCE) {
  MessageSharedPtr msg = CreateMessage(smart_objects::SmartType_Map);
  (*msg)[am::strings::params][am::strings::connection_key] = kConnectionKey;

  utils::SharedPtr<SetMediaClockRequest> command =
      CreateCommand<SetMediaClockRequest>(msg);

  MessageSharedPtr ev_msg = CreateMessage(smart_objects::SmartType_Map);
  (*ev_msg)[am::strings::params][am::hmi_response::code] =
      hmi_apis::Common_Result::UNSUPPORTED_RESOURCE;
  (*ev_msg)[am::strings::msg_params][am::strings::app_id] = kConnectionKey;
  (*ev_msg)[am::strings::msg_params][am::strings::info] =
      "UI is not supported by system";

  Event event(hmi_apis::FunctionID::UI_SetMediaClockTimer);
  event.set_smart_object(*ev_msg);

  EXPECT_CALL(mock_hmi_interfaces_,
              GetInterfaceState(am::HmiInterfaces::HMI_INTERFACE_UI))
      .WillRepeatedly(Return(am::HmiInterfaces::STATE_NOT_RESPONSE));

  EXPECT_CALL(mock_message_helper_,
              HMIToMobileResult(hmi_apis::Common_Result::UNSUPPORTED_RESOURCE))
      .WillOnce(Return(mobile_apis::Result::UNSUPPORTED_RESOURCE));

  MessageSharedPtr ui_command_result;
  EXPECT_CALL(
      app_mngr_,
      ManageMobileCommand(_, am::commands::Command::CommandOrigin::ORIGIN_SDL))
      .WillOnce(DoAll(SaveArg<0>(&ui_command_result), Return(true)));

  command->on_event(event);

  ResultCommandExpectations(ui_command_result, "UI is not supported by system");
}

TEST_F(SetMediaClockRequestTest, Run_UpdateCountUp_SUCCESS) {
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::msg_params][am::strings::update_mode] =
      UpdateMode::COUNTUP;
  (*msg)[am::strings::msg_params][am::strings::start_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::start_time]
        [am::strings::minutes] = kMinutes;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::minutes] =
      kMinutes;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::seconds] =
      kSeconds;

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  EXPECT_CALL(app_mngr_, application(kConnectionKey))
      .WillOnce(Return(mock_app_));
  EXPECT_CALL(*mock_app_, is_media_application()).WillOnce(Return(true));
  EXPECT_CALL(*mock_app_, app_id()).WillOnce(Return(kAppID));
  EXPECT_CALL(app_mngr_, GetNextHMICorrelationID())
      .WillOnce(Return(kCorrelationId));
  ON_CALL(mock_hmi_interfaces_,
          GetInterfaceFromFunction(hmi_apis::FunctionID::UI_SetMediaClockTimer))
      .WillByDefault(Return(am::HmiInterfaces::HMI_INTERFACE_UI));
  ON_CALL(mock_hmi_interfaces_,
          GetInterfaceState(am::HmiInterfaces::HMI_INTERFACE_UI))
      .WillByDefault(Return(am::HmiInterfaces::STATE_AVAILABLE));
  EXPECT_CALL(app_mngr_, ManageHMICommand(_)).WillOnce(Return(true));

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_UpdateCountDown_SUCCESS) {
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::msg_params][am::strings::update_mode] =
      UpdateMode::COUNTDOWN;
  (*msg)[am::strings::msg_params][am::strings::start_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::start_time]
        [am::strings::minutes] = kMinutes;
  (*msg)[am::strings::msg_params][am::strings::start_time]
        [am::strings::seconds] = kSeconds;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::minutes] =
      kMinutes;

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  EXPECT_CALL(app_mngr_, application(kConnectionKey))
      .WillOnce(Return(mock_app_));
  EXPECT_CALL(*mock_app_, is_media_application()).WillOnce(Return(true));
  EXPECT_CALL(*mock_app_, app_id()).WillOnce(Return(kAppID));
  EXPECT_CALL(app_mngr_, GetNextHMICorrelationID())
      .WillOnce(Return(kCorrelationId));
  ON_CALL(mock_hmi_interfaces_,
          GetInterfaceFromFunction(hmi_apis::FunctionID::UI_SetMediaClockTimer))
      .WillByDefault(Return(am::HmiInterfaces::HMI_INTERFACE_UI));
  ON_CALL(mock_hmi_interfaces_,
          GetInterfaceState(am::HmiInterfaces::HMI_INTERFACE_UI))
      .WillByDefault(Return(am::HmiInterfaces::STATE_AVAILABLE));
  EXPECT_CALL(app_mngr_, ManageHMICommand(_)).WillOnce(Return(true));

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_UpdateCountUpWrongTime_Canceled) {
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::msg_params][am::strings::update_mode] =
      UpdateMode::COUNTUP;
  (*msg)[am::strings::msg_params][am::strings::start_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::start_time]
        [am::strings::minutes] = kMinutes;
  (*msg)[am::strings::msg_params][am::strings::start_time]
        [am::strings::seconds] = kSeconds;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::minutes] =
      kMinutes;

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  ExpectationsSetupHelper(true);

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_UpdateCountDownWrongTime_Canceled) {
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::msg_params][am::strings::update_mode] =
      UpdateMode::COUNTDOWN;
  (*msg)[am::strings::msg_params][am::strings::start_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::start_time]
        [am::strings::minutes] = kMinutes;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::hours] =
      kHours;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::minutes] =
      kMinutes;
  (*msg)[am::strings::msg_params][am::strings::end_time][am::strings::seconds] =
      kSeconds;

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  EXPECT_CALL(app_mngr_, application(kConnectionKey))
      .WillOnce(Return(mock_app_));
  EXPECT_CALL(*mock_app_, is_media_application()).WillOnce(Return(true));
  EXPECT_CALL(*mock_app_, app_id()).Times(0);
  EXPECT_CALL(app_mngr_, ManageMobileCommand(_, _));

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_NoStartTime_Canceled) {
  MessageSharedPtr msg = CreateMsgParams();
  (*msg)[am::strings::msg_params][am::strings::update_mode] =
      UpdateMode::COUNTDOWN;

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  ExpectationsSetupHelper(true);

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_NoUpdateMode_Canceled) {
  MessageSharedPtr msg = CreateMsgParams();

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  ExpectationsSetupHelper(true);

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_NotMediaApp_Canceled) {
  MessageSharedPtr msg = CreateMsgParams();

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  ExpectationsSetupHelper(false);

  command->Run();
}

TEST_F(SetMediaClockRequestTest, Run_InvalidApp_Canceled) {
  MessageSharedPtr msg = CreateMsgParams();

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  EXPECT_CALL(app_mngr_, application(kConnectionKey))
      .WillOnce(Return(MockAppPtr()));
  EXPECT_CALL(*mock_app_, is_media_application()).Times(0);
  EXPECT_CALL(*mock_app_, app_id()).Times(0);
  EXPECT_CALL(app_mngr_, ManageMobileCommand(_, _));

  command->Run();
}

TEST_F(SetMediaClockRequestTest, OnEvent_Success) {
  MessageSharedPtr msg = CreateMessage();
  (*msg)[am::strings::params][am::hmi_response::code] =
      mobile_apis::Result::SUCCESS;
  (*msg)[am::strings::msg_params] = SmartObject(smart_objects::SmartType_Map);

  EXPECT_CALL(mock_message_helper_,
              HMIToMobileResult(hmi_apis::Common_Result::SUCCESS))
      .WillOnce(Return(mobile_apis::Result::SUCCESS));

  EXPECT_CALL(
      app_mngr_,
      ManageMobileCommand(MobileResultCodeIs(mobile_apis::Result::SUCCESS), _));

  MockAppPtr app(CreateMockApp());
  EXPECT_CALL(app_mngr_, application(_)).WillRepeatedly(Return(app));

  Event event(hmi_apis::FunctionID::UI_SetMediaClockTimer);
  event.set_smart_object(*msg);

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));
  command->on_event(event);
}

TEST_F(SetMediaClockRequestTest, OnEvent_Canceled) {
  MessageSharedPtr msg = CreateMessage();

  SharedPtr<SetMediaClockRequest> command(
      CreateCommand<SetMediaClockRequest>(msg));

  EXPECT_CALL(app_mngr_, ManageMobileCommand(_, _)).Times(0);

  Event event(hmi_apis::FunctionID::UI_Slider);
  event.set_smart_object(*msg);

  command->on_event(event);
}

}  // namespace set_media_clock_timer_request
}  // namespace mobile_commands_test
}  // namespace commands_test
}  // namespace components
}  // namespace test
