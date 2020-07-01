/*
 * Copyright (c) 2018, Ford Motor Company
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

#include "sdl_rpc_plugin/commands/hmi/rc_is_ready_request.h"
#include "application_manager/rpc_service.h"

namespace sdl_rpc_plugin {
using namespace application_manager;

namespace commands {

RCIsReadyRequest::RCIsReadyRequest(
    const application_manager::commands::MessageSharedPtr& message,
    ApplicationManager& application_manager,
    rpc_service::RPCService& rpc_service,
    HMICapabilities& hmi_capabilities,
    policy::PolicyHandlerInterface& policy_handle)
    : RequestToHMI(message,
                   application_manager,
                   rpc_service,
                   hmi_capabilities,
                   policy_handle)
    , EventObserver(application_manager.event_dispatcher()) {}

RCIsReadyRequest::~RCIsReadyRequest() {}

void RCIsReadyRequest::Run() {
  LOG4CXX_AUTO_TRACE(logger_);
  subscribe_on_event(hmi_apis::FunctionID::RC_IsReady, correlation_id());
  SendRequest();
}

void RCIsReadyRequest::on_event(const event_engine::Event& event) {
  LOG4CXX_AUTO_TRACE(logger_);
  const smart_objects::SmartObject& message = event.smart_object();
  switch (event.id()) {
    case hmi_apis::FunctionID::RC_IsReady: {
      LOG4CXX_DEBUG(logger_, "Received RC_IsReady event");
      unsubscribe_from_event(hmi_apis::FunctionID::RC_IsReady);
      const bool is_available = app_mngr::commands::ChangeInterfaceState(
          application_manager_, message, HmiInterfaces::HMI_INTERFACE_RC);

      HMICapabilities& hmi_capabilities = hmi_capabilities_;
      hmi_capabilities.set_is_rc_cooperating(is_available);
      if (!is_available) {
        hmi_capabilities.set_rc_supported(false);
      }

      if (!app_mngr::commands::CheckAvailabilityHMIInterfaces(
              application_manager_, HmiInterfaces::HMI_INTERFACE_RC)) {
        hmi_capabilities_.UpdateRequestsRequiredForCapabilities(
            hmi_apis::FunctionID::RC_GetCapabilities);
        LOG4CXX_INFO(logger_,
                     "HmiInterfaces::HMI_INTERFACE_RC isn't available");
        return;
      }

      RequestCapabilities();
      break;
    }
    default: {
      LOG4CXX_ERROR(logger_, "Received unknown event" << event.id());
      return;
    }
  }
}

void RCIsReadyRequest::onTimeOut() {
  // Note(dtrunov): According to new requirment APPLINK-27956
  RequestCapabilities();
}

void RCIsReadyRequest::RequestCapabilities() {
  if (hmi_capabilities_.IsRequestsRequiredForCapabilities(
          hmi_apis::FunctionID::RC_GetCapabilities)) {
    std::shared_ptr<smart_objects::SmartObject> get_capabilities(
        MessageHelper::CreateModuleInfoSO(
            hmi_apis::FunctionID::RC_GetCapabilities, application_manager_));
    rpc_service_.ManageHMICommand(get_capabilities);
  }
}

}  // namespace commands

}  // namespace sdl_rpc_plugin
