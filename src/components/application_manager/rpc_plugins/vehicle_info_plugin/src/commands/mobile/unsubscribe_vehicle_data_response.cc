/*

 Copyright (c) 2018, Ford Motor Company
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

#include "vehicle_info_plugin/commands/mobile/unsubscribe_vehicle_data_response.h"
#include "smart_objects/smart_object.h"

namespace vehicle_info_plugin {
using namespace application_manager;
namespace commands {

SDL_CREATE_LOG_VARIABLE("Commands")

UnsubscribeVehicleDataResponse::UnsubscribeVehicleDataResponse(
    const application_manager::commands::MessageSharedPtr& message,
    const VehicleInfoCommandParams& params)
    : CommandResponseImpl(message,
                          params.application_manager_,
                          params.rpc_service_,
                          params.hmi_capabilities_,
                          params.policy_handler_) {}

UnsubscribeVehicleDataResponse::~UnsubscribeVehicleDataResponse() {}

void UnsubscribeVehicleDataResponse::Run() {
  SDL_LOG_AUTO_TRACE();

  // check if response false
  if (true == (*message_)[strings::msg_params].keyExists(strings::success)) {
    if ((*message_)[strings::msg_params][strings::success].asBool() == false) {
      SDL_LOG_ERROR("Success = false");
      SendResponse(false);
      return;
    }
  }
  SendResponse(true);
}

}  // namespace commands
}  // namespace vehicle_info_plugin
