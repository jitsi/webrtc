/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/congestion_controller/goog_cc/probe_controller.h"

#include <algorithm>
#include <initializer_list>
#include <memory>
#include <string>

#include "absl/strings/match.h"
#include "absl/types/optional.h"
#include "api/units/data_rate.h"
#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "logging/rtc_event_log/events/rtc_event_probe_cluster_created.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "system_wrappers/include/metrics.h"

namespace webrtc {

namespace {
// Maximum waiting time from the time of initiating probing to getting
// the measured results back.
constexpr TimeDelta kMaxWaitingTimeForProbingResult = TimeDelta::Seconds(1);

// Default probing bitrate limit. Applied only when the application didn't
// specify max bitrate.
constexpr DataRate kDefaultMaxProbingBitrate = DataRate::KilobitsPerSec(5000);

// If the bitrate drops to a factor `kBitrateDropThreshold` or lower
// and we recover within `kBitrateDropTimeoutMs`, then we'll send
// a probe at a fraction `kProbeFractionAfterDrop` of the original bitrate.
constexpr double kBitrateDropThreshold = 0.66;
constexpr TimeDelta kBitrateDropTimeout = TimeDelta::Seconds(5);
constexpr double kProbeFractionAfterDrop = 0.85;

// Timeout for probing after leaving ALR. If the bitrate drops significantly,
// (as determined by the delay based estimator) and we leave ALR, then we will
// send a probe if we recover within `kLeftAlrTimeoutMs` ms.
constexpr TimeDelta kAlrEndedTimeout = TimeDelta::Seconds(3);

// The expected uncertainty of probe result (as a fraction of the target probe
// This is a limit on how often probing can be done when there is a BW
// drop detected in ALR.
constexpr TimeDelta kMinTimeBetweenAlrProbes = TimeDelta::Seconds(5);

// bitrate). Used to avoid probing if the probe bitrate is close to our current
// estimate.
constexpr double kProbeUncertainty = 0.05;

// Use probing to recover faster after large bitrate estimate drops.
constexpr char kBweRapidRecoveryExperiment[] =
    "WebRTC-BweRapidRecoveryExperiment";

void MaybeLogProbeClusterCreated(RtcEventLog* event_log,
                                 const ProbeClusterConfig& probe) {
  RTC_DCHECK(event_log);
  if (!event_log) {
    return;
  }

  DataSize min_data_size = probe.target_data_rate * probe.target_duration;
  event_log->Log(std::make_unique<RtcEventProbeClusterCreated>(
      probe.id, probe.target_data_rate.bps(), probe.target_probe_count,
      min_data_size.bytes()));
}

}  // namespace

ProbeControllerConfig::ProbeControllerConfig(
    const FieldTrialsView* key_value_config)
    : first_exponential_probe_scale("p1", 3.0),
      second_exponential_probe_scale("p2", 6.0),
      further_exponential_probe_scale("step_size", 2),
      further_probe_threshold("further_probe_threshold", 0.7),
      alr_probing_interval("alr_interval", TimeDelta::Seconds(5)),
      alr_probe_scale("alr_scale", 2),
      network_state_estimate_probing_interval("network_state_interval",
                                              TimeDelta::PlusInfinity()),
      network_state_estimate_fast_rampup_rate("network_state_fast_rampup_rate",
                                              0),
      network_state_estimate_drop_down_rate("network_state_drop_down_rate", 0),
      network_state_probe_scale("network_state_scale", 1.0),
      network_state_probe_duration("network_state_probe_duration",
                                   TimeDelta::Millis(15)),

      first_allocation_probe_scale("alloc_p1", 1),
      second_allocation_probe_scale("alloc_p2", 2),
      allocation_allow_further_probing("alloc_probe_further", false),
      allocation_probe_max("alloc_probe_max", DataRate::PlusInfinity()),
      min_probe_packets_sent("min_probe_packets_sent", 5),
      min_probe_duration("min_probe_duration", TimeDelta::Millis(15)),
      limit_probe_target_rate_to_loss_bwe("limit_probe_target_rate_to_loss_bwe",
                                          false),
      skip_if_estimate_larger_than_fraction_of_max(
          "skip_if_est_larger_than_fraction_of_max",
          0.0) {
  ParseFieldTrial(
      {&first_exponential_probe_scale, &second_exponential_probe_scale,
       &further_exponential_probe_scale, &further_probe_threshold,
       &alr_probing_interval, &alr_probe_scale, &first_allocation_probe_scale,
       &second_allocation_probe_scale, &allocation_allow_further_probing,
       &min_probe_duration, &network_state_estimate_probing_interval,
       &network_state_estimate_fast_rampup_rate,
       &network_state_estimate_drop_down_rate, &network_state_probe_scale,
       &network_state_probe_duration, &min_probe_packets_sent,
       &limit_probe_target_rate_to_loss_bwe,
       &skip_if_estimate_larger_than_fraction_of_max},
      key_value_config->Lookup("WebRTC-Bwe-ProbingConfiguration"));

  // Specialized keys overriding subsets of WebRTC-Bwe-ProbingConfiguration
  ParseFieldTrial(
      {&first_exponential_probe_scale, &second_exponential_probe_scale},
      key_value_config->Lookup("WebRTC-Bwe-InitialProbing"));
  ParseFieldTrial({&further_exponential_probe_scale, &further_probe_threshold},
                  key_value_config->Lookup("WebRTC-Bwe-ExponentialProbing"));
  ParseFieldTrial({&alr_probing_interval, &alr_probe_scale},
                  key_value_config->Lookup("WebRTC-Bwe-AlrProbing"));
  ParseFieldTrial(
      {&first_allocation_probe_scale, &second_allocation_probe_scale,
       &allocation_allow_further_probing, &allocation_probe_max},
      key_value_config->Lookup("WebRTC-Bwe-AllocationProbing"));
  ParseFieldTrial({&min_probe_packets_sent, &min_probe_duration},
                  key_value_config->Lookup("WebRTC-Bwe-ProbingBehavior"));
}

ProbeControllerConfig::ProbeControllerConfig(const ProbeControllerConfig&) =
    default;
ProbeControllerConfig::~ProbeControllerConfig() = default;

ProbeController::ProbeController(const FieldTrialsView* key_value_config,
                                 RtcEventLog* event_log)
    : enable_periodic_alr_probing_(false),
      in_rapid_recovery_experiment_(absl::StartsWith(
          key_value_config->Lookup(kBweRapidRecoveryExperiment),
          "Enabled")),
      event_log_(event_log),
      config_(ProbeControllerConfig(key_value_config)) {
  Reset(Timestamp::Zero());
}

ProbeController::~ProbeController() {}

std::vector<ProbeClusterConfig> ProbeController::SetBitrates(
    DataRate min_bitrate,
    DataRate start_bitrate,
    DataRate max_bitrate,
    Timestamp at_time) {
  if (start_bitrate > DataRate::Zero()) {
    start_bitrate_ = start_bitrate;
    estimated_bitrate_ = start_bitrate;
  } else if (start_bitrate_.IsZero()) {
    start_bitrate_ = min_bitrate;
  }

  // The reason we use the variable `old_max_bitrate_pbs` is because we
  // need to set `max_bitrate_` before we call InitiateProbing.
  DataRate old_max_bitrate = max_bitrate_;
  max_bitrate_ =
      max_bitrate.IsFinite() ? max_bitrate : kDefaultMaxProbingBitrate;

  switch (state_) {
    case State::kInit:
      if (network_available_)
        return InitiateExponentialProbing(at_time);
      break;

    case State::kWaitingForProbingResult:
      break;

    case State::kProbingComplete:
      // If the new max bitrate is higher than both the old max bitrate and the
      // estimate then initiate probing.
      if (!estimated_bitrate_.IsZero() && old_max_bitrate < max_bitrate_ &&
          estimated_bitrate_ < max_bitrate_) {
        // The assumption is that if we jump more than 20% in the bandwidth
        // estimate or if the bandwidth estimate is within 90% of the new
        // max bitrate then the probing attempt was successful.
        mid_call_probing_succcess_threshold_ =
            std::min(estimated_bitrate_ * 1.2, max_bitrate_ * 0.9);
        mid_call_probing_waiting_for_result_ = true;
        mid_call_probing_bitrate_ = max_bitrate_;

        RTC_HISTOGRAM_COUNTS_10000("WebRTC.BWE.MidCallProbing.Initiated",
                                   max_bitrate_.kbps());

        return InitiateProbing(at_time, {max_bitrate_}, false);
      }
      break;
  }
  return std::vector<ProbeClusterConfig>();
}

std::vector<ProbeClusterConfig> ProbeController::OnMaxTotalAllocatedBitrate(
    DataRate max_total_allocated_bitrate,
    Timestamp at_time) {
  const bool in_alr = alr_start_time_.has_value();
  const bool allow_allocation_probe = in_alr;

  if (state_ == State::kProbingComplete &&
      max_total_allocated_bitrate != max_total_allocated_bitrate_ &&
      estimated_bitrate_ < max_bitrate_ &&
      estimated_bitrate_ < max_total_allocated_bitrate &&
      allow_allocation_probe) {
    max_total_allocated_bitrate_ = max_total_allocated_bitrate;

    if (!config_.first_allocation_probe_scale)
      return std::vector<ProbeClusterConfig>();

    DataRate first_probe_rate = max_total_allocated_bitrate *
                                config_.first_allocation_probe_scale.Value();
    DataRate probe_cap = config_.allocation_probe_max.Get();
    first_probe_rate = std::min(first_probe_rate, probe_cap);
    std::vector<DataRate> probes = {first_probe_rate};
    if (config_.second_allocation_probe_scale) {
      DataRate second_probe_rate =
          max_total_allocated_bitrate *
          config_.second_allocation_probe_scale.Value();
      second_probe_rate = std::min(second_probe_rate, probe_cap);
      if (second_probe_rate > first_probe_rate)
        probes.push_back(second_probe_rate);
    }
    return InitiateProbing(at_time, probes,
                           config_.allocation_allow_further_probing.Get());
  }
  max_total_allocated_bitrate_ = max_total_allocated_bitrate;
  return std::vector<ProbeClusterConfig>();
}

std::vector<ProbeClusterConfig> ProbeController::OnNetworkAvailability(
    NetworkAvailability msg) {
  network_available_ = msg.network_available;

  if (!network_available_ && state_ == State::kWaitingForProbingResult) {
    state_ = State::kProbingComplete;
    min_bitrate_to_probe_further_ = DataRate::PlusInfinity();
  }

  if (network_available_ && state_ == State::kInit && !start_bitrate_.IsZero())
    return InitiateExponentialProbing(msg.at_time);
  return std::vector<ProbeClusterConfig>();
}

std::vector<ProbeClusterConfig> ProbeController::InitiateExponentialProbing(
    Timestamp at_time) {
  RTC_DCHECK(network_available_);
  RTC_DCHECK(state_ == State::kInit);
  RTC_DCHECK_GT(start_bitrate_, DataRate::Zero());

  // When probing at 1.8 Mbps ( 6x 300), this represents a threshold of
  // 1.2 Mbps to continue probing.
  std::vector<DataRate> probes = {config_.first_exponential_probe_scale *
                                  start_bitrate_};
  if (config_.second_exponential_probe_scale &&
      config_.second_exponential_probe_scale.GetOptional().value() > 0) {
    probes.push_back(config_.second_exponential_probe_scale.Value() *
                     start_bitrate_);
  }
  return InitiateProbing(at_time, probes, true);
}

std::vector<ProbeClusterConfig> ProbeController::SetEstimatedBitrate(
    DataRate bitrate,
    BandwidthLimitedCause bandwidth_limited_cause,
    Timestamp at_time) {
  bandwidth_limited_cause_ = bandwidth_limited_cause;
  if (bandwidth_limited_cause_ ==
          BandwidthLimitedCause::kLossLimitedBweDecreasing &&
      config_.limit_probe_target_rate_to_loss_bwe) {
    state_ = State::kProbingComplete;
  }
  if (bitrate < kBitrateDropThreshold * estimated_bitrate_) {
    time_of_last_large_drop_ = at_time;
    bitrate_before_last_large_drop_ = estimated_bitrate_;
  }
  estimated_bitrate_ = bitrate;

  if (mid_call_probing_waiting_for_result_ &&
      bitrate >= mid_call_probing_succcess_threshold_) {
    RTC_HISTOGRAM_COUNTS_10000("WebRTC.BWE.MidCallProbing.Success",
                               mid_call_probing_bitrate_.kbps());
    RTC_HISTOGRAM_COUNTS_10000("WebRTC.BWE.MidCallProbing.ProbedKbps",
                               bitrate.kbps());
    mid_call_probing_waiting_for_result_ = false;
  }
  std::vector<ProbeClusterConfig> pending_probes;
  if (state_ == State::kWaitingForProbingResult) {
    // Continue probing if probing results indicate channel has greater
    // capacity.
    RTC_LOG(LS_INFO) << "Measured bitrate: " << bitrate
                     << " Minimum to probe further: "
                     << min_bitrate_to_probe_further_;

    if (bitrate > min_bitrate_to_probe_further_) {
      pending_probes = InitiateProbing(
          at_time, {config_.further_exponential_probe_scale * bitrate}, true);
    }
  }

  return pending_probes;
}

void ProbeController::EnablePeriodicAlrProbing(bool enable) {
  enable_periodic_alr_probing_ = enable;
}

void ProbeController::SetAlrStartTimeMs(
    absl::optional<int64_t> alr_start_time_ms) {
  if (alr_start_time_ms) {
    alr_start_time_ = Timestamp::Millis(*alr_start_time_ms);
  } else {
    alr_start_time_ = absl::nullopt;
  }
}
void ProbeController::SetAlrEndedTimeMs(int64_t alr_end_time_ms) {
  alr_end_time_.emplace(Timestamp::Millis(alr_end_time_ms));
}

std::vector<ProbeClusterConfig> ProbeController::RequestProbe(
    Timestamp at_time) {
  // Called once we have returned to normal state after a large drop in
  // estimated bandwidth. The current response is to initiate a single probe
  // session (if not already probing) at the previous bitrate.
  //
  // If the probe session fails, the assumption is that this drop was a
  // real one from a competing flow or a network change.
  bool in_alr = alr_start_time_.has_value();
  bool alr_ended_recently =
      (alr_end_time_.has_value() &&
       at_time - alr_end_time_.value() < kAlrEndedTimeout);
  if (in_alr || alr_ended_recently || in_rapid_recovery_experiment_) {
    if (state_ == State::kProbingComplete) {
      DataRate suggested_probe =
          kProbeFractionAfterDrop * bitrate_before_last_large_drop_;
      DataRate min_expected_probe_result =
          (1 - kProbeUncertainty) * suggested_probe;
      TimeDelta time_since_drop = at_time - time_of_last_large_drop_;
      TimeDelta time_since_probe = at_time - last_bwe_drop_probing_time_;
      if (min_expected_probe_result > estimated_bitrate_ &&
          time_since_drop < kBitrateDropTimeout &&
          time_since_probe > kMinTimeBetweenAlrProbes) {
        RTC_LOG(LS_INFO) << "Detected big bandwidth drop, start probing.";
        // Track how often we probe in response to bandwidth drop in ALR.
        RTC_HISTOGRAM_COUNTS_10000(
            "WebRTC.BWE.BweDropProbingIntervalInS",
            (at_time - last_bwe_drop_probing_time_).seconds());
        last_bwe_drop_probing_time_ = at_time;
        return InitiateProbing(at_time, {suggested_probe}, false);
      }
    }
  }
  return std::vector<ProbeClusterConfig>();
}

void ProbeController::SetMaxBitrate(DataRate max_bitrate) {
  max_bitrate_ = max_bitrate;
}

void ProbeController::SetNetworkStateEstimate(
    webrtc::NetworkStateEstimate estimate) {
  if (config_.network_state_estimate_fast_rampup_rate > 0 &&
      estimated_bitrate_ < estimate.link_capacity_upper &&
      (!network_estimate_ ||
       estimate.link_capacity_upper >=
           config_.network_state_estimate_fast_rampup_rate *
               network_estimate_->link_capacity_upper)) {
    send_probe_on_next_process_interval_ = true;
  }
  if (config_.network_state_estimate_drop_down_rate > 0 && network_estimate_ &&
      !estimate.link_capacity_upper.IsZero() &&
      (estimated_bitrate_ > estimate.link_capacity_upper ||
       bandwidth_limited_cause_ ==
           BandwidthLimitedCause::kLossLimitedBweDecreasing) &&
      estimate.link_capacity_upper <=
          config_.network_state_estimate_drop_down_rate *
              network_estimate_->link_capacity_upper) {
    send_probe_on_next_process_interval_ = true;
  }

  network_estimate_ = estimate;
}

void ProbeController::Reset(Timestamp at_time) {
  network_available_ = true;
  state_ = State::kInit;
  min_bitrate_to_probe_further_ = DataRate::PlusInfinity();
  time_last_probing_initiated_ = Timestamp::Zero();
  estimated_bitrate_ = DataRate::Zero();
  network_estimate_ = absl::nullopt;
  start_bitrate_ = DataRate::Zero();
  max_bitrate_ = kDefaultMaxProbingBitrate;
  Timestamp now = at_time;
  last_bwe_drop_probing_time_ = now;
  alr_end_time_.reset();
  mid_call_probing_waiting_for_result_ = false;
  time_of_last_large_drop_ = now;
  bitrate_before_last_large_drop_ = DataRate::Zero();
  max_total_allocated_bitrate_ = DataRate::Zero();
  send_probe_on_next_process_interval_ = false;
  bandwidth_limited_cause_ = BandwidthLimitedCause::kDelayBasedLimited;
}

bool ProbeController::TimeForAlrProbe(Timestamp at_time) const {
  if (enable_periodic_alr_probing_ && alr_start_time_) {
    Timestamp next_probe_time =
        std::max(*alr_start_time_, time_last_probing_initiated_) +
        config_.alr_probing_interval;
    return at_time >= next_probe_time;
  }
  return false;
}

bool ProbeController::TimeForNetworkStateProbe(Timestamp at_time) const {
  if (config_.network_state_estimate_probing_interval->IsFinite() &&
      network_estimate_ && network_estimate_->link_capacity_upper.IsFinite() &&
      estimated_bitrate_ < network_estimate_->link_capacity_upper) {
    Timestamp next_probe_time = time_last_probing_initiated_ +
                                config_.network_state_estimate_probing_interval;
    return at_time >= next_probe_time;
  }
  return false;
}

std::vector<ProbeClusterConfig> ProbeController::Process(Timestamp at_time) {
  if (at_time - time_last_probing_initiated_ >
      kMaxWaitingTimeForProbingResult) {
    mid_call_probing_waiting_for_result_ = false;

    if (state_ == State::kWaitingForProbingResult) {
      RTC_LOG(LS_INFO) << "kWaitingForProbingResult: timeout";
      state_ = State::kProbingComplete;
      min_bitrate_to_probe_further_ = DataRate::PlusInfinity();
    }
  }
  if (estimated_bitrate_.IsZero() || state_ != State::kProbingComplete) {
    return {};
  }
  if (send_probe_on_next_process_interval_ || TimeForAlrProbe(at_time) ||
      TimeForNetworkStateProbe(at_time)) {
    DataRate suggested_probe = estimated_bitrate_ * config_.alr_probe_scale;
    if (config_.limit_probe_target_rate_to_loss_bwe &&
        bandwidth_limited_cause_ != BandwidthLimitedCause::kDelayBasedLimited) {
      suggested_probe = estimated_bitrate_;
    }
    return InitiateProbing(at_time, {suggested_probe}, true);
  }
  return std::vector<ProbeClusterConfig>();
}

std::vector<ProbeClusterConfig> ProbeController::InitiateProbing(
    Timestamp now,
    std::vector<DataRate> bitrates_to_probe,
    bool probe_further) {
  if (config_.skip_if_estimate_larger_than_fraction_of_max > 0) {
    DataRate network_estimate = network_estimate_
                                    ? network_estimate_->link_capacity_upper
                                    : DataRate::PlusInfinity();
    if (std::min(network_estimate, estimated_bitrate_) >
        config_.skip_if_estimate_larger_than_fraction_of_max * max_bitrate_) {
      return {};
    }
  }

  DataRate max_probe_bitrate = max_bitrate_;
  if (bandwidth_limited_cause_ ==
          BandwidthLimitedCause::kLossLimitedBweDecreasing &&
      config_.limit_probe_target_rate_to_loss_bwe) {
    max_probe_bitrate = std::min(estimated_bitrate_, max_bitrate_);
  }
  if (config_.network_state_estimate_probing_interval->IsFinite() &&
      network_estimate_ && network_estimate_->link_capacity_upper.IsFinite()) {
    if (network_estimate_->link_capacity_upper.IsZero()) {
      RTC_LOG(LS_INFO) << "Not sending probe, Network state estimate is zero";
      return {};
    }
    max_probe_bitrate =
        std::min(max_probe_bitrate, network_estimate_->link_capacity_upper *
                                        config_.network_state_probe_scale);
  }
  if (max_total_allocated_bitrate_ > DataRate::Zero()) {
    // If a max allocated bitrate has been configured, allow probing up to 2x
    // that rate. This allows some overhead to account for bursty streams,
    // which otherwise would have to ramp up when the overshoot is already in
    // progress.
    // It also avoids minor quality reduction caused by probes often being
    // received at slightly less than the target probe bitrate.
    max_probe_bitrate =
        std::min(max_probe_bitrate, max_total_allocated_bitrate_ * 2);
  }

  send_probe_on_next_process_interval_ = false;

  std::vector<ProbeClusterConfig> pending_probes;
  for (DataRate bitrate : bitrates_to_probe) {
    RTC_DCHECK(!bitrate.IsZero());

    if (bitrate > max_probe_bitrate) {
      bitrate = max_probe_bitrate;
      probe_further = false;
    }

    ProbeClusterConfig config;
    config.at_time = now;
    config.target_data_rate = bitrate;
    if (network_estimate_ &&
        config_.network_state_estimate_probing_interval->IsFinite()) {
      config.target_duration = config_.network_state_probe_duration;
    } else {
      config.target_duration = config_.min_probe_duration;
    }

    config.target_probe_count = config_.min_probe_packets_sent;
    config.id = next_probe_cluster_id_;
    next_probe_cluster_id_++;
    MaybeLogProbeClusterCreated(event_log_, config);
    pending_probes.push_back(config);
  }
  time_last_probing_initiated_ = now;
  if (probe_further) {
    state_ = State::kWaitingForProbingResult;
    min_bitrate_to_probe_further_ =
        (*(bitrates_to_probe.end() - 1)) * config_.further_probe_threshold;
  } else {
    state_ = State::kProbingComplete;
    min_bitrate_to_probe_further_ = DataRate::PlusInfinity();
  }
  return pending_probes;
}

}  // namespace webrtc
