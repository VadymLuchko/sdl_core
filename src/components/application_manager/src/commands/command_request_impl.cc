/*
 Copyright (c) 2016, Ford Motor Company
 All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following
 disclaimer in the documentation and/or other materials provided with the
 distribution.
 Neither the name of the Ford Motor Company nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "application_manager/commands/command_request_impl.h"

#include "application_manager/application_impl.h"
#include "application_manager/message_helper.h"
#include "application_manager/rpc_service.h"
#include "smart_objects/enum_schema_item.h"
#include "smart_objects/smart_object.h"

namespace application_manager {

namespace {
struct AppExtensionPredicate {
  AppExtensionUID uid;
  bool operator()(const ApplicationSharedPtr app) {
    return app ? (app->QueryInterface(uid).use_count() != 0) : false;
  }
};
}  // namespace

namespace commands {

SDL_CREATE_LOG_VARIABLE("Commands")

CommandRequestImpl::CommandRequestImpl(
    const MessageSharedPtr& message,
    ApplicationManager& application_manager,
    rpc_service::RPCService& rpc_service,
    HMICapabilities& hmi_capabilities,
    policy::PolicyHandlerInterface& policy_handler)
    : CommandImpl(message,
                  application_manager,
                  rpc_service,
                  hmi_capabilities,
                  policy_handler)
    , EventObserver(application_manager.event_dispatcher())
    , current_state_(RequestState::kAwaitingResponse) {}

CommandRequestImpl::~CommandRequestImpl() {
  CleanUp();
}

bool CommandRequestImpl::CheckPermissions() {
  return CommandImpl::CheckPermissions();
}

bool CommandRequestImpl::Init() {
  return CommandImpl::Init();
}

bool CommandRequestImpl::CleanUp() {
  return CommandImpl::CleanUp();
}

void CommandRequestImpl::Run() {}

uint32_t CommandRequestImpl::default_timeout() const {
  return CommandImpl::default_timeout();
}

uint32_t CommandRequestImpl::correlation_id() const {
  return CommandImpl::correlation_id();
}

int32_t CommandRequestImpl::function_id() const {
  return CommandImpl::function_id();
}

uint32_t CommandRequestImpl::connection_key() const {
  return CommandImpl::connection_key();
}

bool CommandRequestImpl::AllowedToTerminate() {
  return CommandImpl::AllowedToTerminate();
}

bool CommandRequestImpl::CheckAllowedParameters(
    const Command::CommandSource source) {
  SDL_LOG_AUTO_TRACE();

  // RegisterAppInterface should always be allowed
  if (mobile_apis::FunctionID::RegisterAppInterfaceID ==
      static_cast<mobile_apis::FunctionID::eType>(function_id())) {
    return true;
  }

  return CommandImpl::CheckAllowedParameters(source);
}

void CommandRequestImpl::SendMobileRequest(
    const mobile_apis::FunctionID::eType& function_id,
    smart_objects::SmartObjectSPtr msg,
    bool use_events) {
  smart_objects::SmartObject& request = *msg;

  const uint32_t mobile_correlation_id =
      application_manager_.GetNextMobileCorrelationID();

  request[strings::params][strings::correlation_id] = mobile_correlation_id;
  request[strings::params][strings::message_type] = MessageType::kRequest;
  if (use_events) {
    SDL_LOG_DEBUG("SendMobileRequest subscribe_on_event "
                  << function_id << " " << mobile_correlation_id);
    subscribe_on_event(function_id, mobile_correlation_id);
  }

  if (!rpc_service_.ManageMobileCommand(msg, SOURCE_SDL)) {
    SDL_LOG_ERROR("Unable to send request to mobile");
  }
}

void CommandRequestImpl::SetAllowedToTerminate(const bool allowed) {
  return CommandImpl::SetAllowedToTerminate(allowed);
}

bool CommandRequestImpl::ReplaceMobileWithHMIAppId(
    ns_smart_device_link::ns_smart_objects::SmartObject& message) {
  return CommandImpl::ReplaceMobileWithHMIAppId(message);
}

bool CommandRequestImpl::ReplaceHMIWithMobileAppId(
    ns_smart_device_link::ns_smart_objects::SmartObject& message) {
  return CommandImpl::ReplaceHMIWithMobileAppId(message);
}

void CommandRequestImpl::OnTimeOut() {}

void CommandRequestImpl::on_event(const event_engine::Event&) {}
void CommandRequestImpl::on_event(const event_engine::MobileEvent&) {}

void CommandRequestImpl::HandleTimeOut() {
  SDL_LOG_AUTO_TRACE();
  {
    sync_primitives::AutoLock auto_lock(state_lock_);
    if (RequestState::kProcessEvent == current_state()) {
      SDL_LOG_DEBUG("Current request state is: "
                    << current_state() << ". Timeout request ignored");
      return;
    }
    set_current_state(RequestState::kTimedOut);
  }

  OnTimeOut();
}

bool CommandRequestImpl::IsMobileResultSuccess(
    const mobile_apis::Result::eType result_code) {
  SDL_LOG_AUTO_TRACE();
  using namespace helpers;
  return Compare<mobile_apis::Result::eType, EQ, ONE>(
      result_code,
      mobile_apis::Result::SUCCESS,
      mobile_apis::Result::WARNINGS,
      mobile_apis::Result::WRONG_LANGUAGE,
      mobile_apis::Result::RETRY,
      mobile_apis::Result::SAVED,
      mobile_apis::Result::TRUNCATED_DATA);
}

bool CommandRequestImpl::IsHMIResultSuccess(
    const hmi_apis::Common_Result::eType result_code) {
  SDL_LOG_AUTO_TRACE();
  using namespace helpers;
  return Compare<hmi_apis::Common_Result::eType, EQ, ONE>(
      result_code,
      hmi_apis::Common_Result::SUCCESS,
      hmi_apis::Common_Result::WARNINGS,
      hmi_apis::Common_Result::WRONG_LANGUAGE,
      hmi_apis::Common_Result::RETRY,
      hmi_apis::Common_Result::SAVED,
      hmi_apis::Common_Result::TRUNCATED_DATA);
}

void CommandRequestImpl::HandleOnEvent(const event_engine::Event& event) {
  SDL_LOG_AUTO_TRACE();

  {
    sync_primitives::AutoLock auto_lock(state_lock_);
    if (RequestState::kTimedOut == current_state()) {
      SDL_LOG_DEBUG("current_state_ = kTimedOut");
      return;
    }
    set_current_state(RequestState::kProcessEvent);
  }

  const auto conn_key = connection_key();
  const auto corr_id = correlation_id();

  // Retain request instance to avoid object suicide after on_event()
  application_manager_.RetainRequestInstance(conn_key, corr_id);
  on_event(event);

  if (application_manager_.IsStillWaitingForResponse(conn_key, corr_id)) {
    SDL_LOG_DEBUG("Request (" << conn_key << ", " << corr_id
                              << ") is still waiting for repsonse");
    set_current_state(RequestState::kAwaitingResponse);
  }

  // Remove request instance from retained to destroy it safely if required
  application_manager_.RemoveRetainedRequest(conn_key, corr_id);
}

// FIXME(VSemenyuk): Duplicated code of HandleOnEvent(MobileEvent) and
// HandleOnEvent(Event) should be moved to a separate function
void CommandRequestImpl::HandleOnEvent(const event_engine::MobileEvent& event) {
  SDL_LOG_AUTO_TRACE();

  {
    sync_primitives::AutoLock auto_lock(state_lock_);
    if (RequestState::kTimedOut == current_state()) {
      SDL_LOG_DEBUG("current_state_ = kTimedOut");
      return;
    }
    set_current_state(RequestState::kProcessEvent);
  }

  const auto conn_key = connection_key();
  const auto corr_id = correlation_id();

  // Retain request instance to avoid object suicide after on_event()
  application_manager_.RetainRequestInstance(conn_key, corr_id);
  on_event(event);

  if (application_manager_.IsStillWaitingForResponse(conn_key, corr_id)) {
    SDL_LOG_DEBUG("Request (" << conn_key << ", " << corr_id
                              << ") is still waiting for repsonse");
    set_current_state(RequestState::kAwaitingResponse);
  }

  // Remove request instance from retained to destroy it safely if required
  application_manager_.RemoveRetainedRequest(conn_key, corr_id);
}

void CommandRequestImpl::OnUpdateTimeOut() {
  SDL_LOG_AUTO_TRACE();
  set_current_state(RequestState::kAwaitingResponse);
}

void CommandRequestImpl::StartAwaitForInterface(
    const HmiInterfaces::InterfaceID& interface_id) {
  sync_primitives::AutoLock lock(awaiting_response_interfaces_lock_);
  awaiting_response_interfaces_.insert(interface_id);
}

bool CommandRequestImpl::IsInterfaceAwaited(
    const HmiInterfaces::InterfaceID& interface_id) const {
  sync_primitives::AutoLock lock(awaiting_response_interfaces_lock_);

  return helpers::in_range(awaiting_response_interfaces_, interface_id);
}

void CommandRequestImpl::EndAwaitForInterface(
    const HmiInterfaces::InterfaceID& interface_id) {
  sync_primitives::AutoLock lock(awaiting_response_interfaces_lock_);
  awaiting_response_interfaces_.erase(interface_id);
}

bool CommandRequestImpl::IsPendingResponseExist() const {
  sync_primitives::AutoLock lock(awaiting_response_interfaces_lock_);
  return !awaiting_response_interfaces_.empty();
}

CommandRequestImpl::RequestState CommandRequestImpl::current_state() const {
  sync_primitives::AutoLock auto_lock(state_lock_);
  return current_state_;
}

void CommandRequestImpl::set_current_state(
    const CommandRequestImpl::RequestState state) {
  sync_primitives::AutoLock auto_lock(state_lock_);
  current_state_ = state;
}

}  // namespace commands
}  // namespace application_manager
