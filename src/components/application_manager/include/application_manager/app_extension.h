/*
 * Copyright (c) 2013, Ford Motor Company
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

#ifndef SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_APP_EXTENSION_H_
#define SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_APP_EXTENSION_H_

#include <functional>
#include <memory>

#include "smart_objects/smart_object.h"

namespace resumption {
struct ResumptionRequest;
class ResumptionDataProcessor;
}  // namespace resumption

namespace application_manager {
namespace ns_smart = ns_smart_device_link::ns_smart_objects;

typedef int AppExtensionUID;

class AppExtension {
 public:
  explicit AppExtension(AppExtensionUID uid) : kUid_(uid) {}
  virtual ~AppExtension() {}
  AppExtensionUID uid() const {
    return kUid_;
  }

  /**
   * @brief SaveResumptionData method called by SDL when it saves resumption
   * data.
   * @param resumption_data data reference to data, that will be appended by
   * plugin
   */
  virtual void SaveResumptionData(ns_smart::SmartObject& resumption_data) = 0;

  /**
   * @brief ProcessResumption Method called by SDL during resumption.
   * @param resumption_data list of resumption data
   * @param subscriber callbacks for subscribing
   */
  virtual void ProcessResumption(
      const ns_smart::SmartObject& resumption_data) = 0;

  /**
   * @brief RevertResumption Method called by SDL during revert resumption.
   * @param subscriptions Subscriptions from which must discard. Expected that
   * SO contains the map of "IVI data" keys and "subscription bool flag" values
   */
  virtual void RevertResumption(const ns_smart::SmartObject& subscriptions) = 0;

 private:
  const AppExtensionUID kUid_;
};

typedef std::shared_ptr<AppExtension> AppExtensionPtr;

}  //  namespace application_manager

#endif  // SRC_COMPONENTS_APPLICATION_MANAGER_INCLUDE_APPLICATION_MANAGER_APP_EXTENSION_H_
