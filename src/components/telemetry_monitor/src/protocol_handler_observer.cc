/*
 * Copyright (c) 2014, Ford Motor Company
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

#include "telemetry_monitor/protocol_handler_observer.h"
#include "telemetry_monitor/protocol_handler_metric_wrapper.h"
#include "telemetry_monitor/telemetry_monitor.h"
#include "utils/date_time.h"

namespace telemetry_monitor {

SDL_CREATE_LOG_VARIABLE("TelemetryMonitor")

ProtocolHandlerObserver::ProtocolHandlerObserver(
    TelemetryMonitor* telemetry_monitor)
    : telemetry_monitor_(telemetry_monitor) {}

void ProtocolHandlerObserver::StartMessageProcess(
    uint32_t message_id, const date_time::TimeDuration& start_time) {
  if (message_id == 0) {
    return;
  }
  if (time_starts.find(message_id) != time_starts.end()) {
    SDL_LOG_DEBUG(
        "Already waiting for stop processing for Message ID: " << message_id);
    return;
  }
  time_starts[message_id] = start_time;
}

void ProtocolHandlerObserver::EndMessageProcess(
    std::shared_ptr<MessageMetric> m) {
  uint32_t message_id = m->message_id;
  std::map<uint32_t, date_time::TimeDuration>::const_iterator it =
      time_starts.find(message_id);
  if (it == time_starts.end()) {
    SDL_LOG_WARN("Cant find start time for message" << message_id);
    return;
  }
  m->begin = time_starts[message_id];
  m->end = date_time::getCurrentTime();
  auto metric = std::make_shared<ProtocolHandlerMecticWrapper>();
  metric->message_metric = m;
  metric->grabResources();
  telemetry_monitor_->SendMetric(metric);
}
}  // namespace telemetry_monitor
