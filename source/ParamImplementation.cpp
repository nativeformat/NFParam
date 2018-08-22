/*
 * Copyright (c) 2016 Spotify AB.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "ParamImplementation.h"

#include <cmath>
#include <sstream>

namespace nativeformat {
namespace param {

ParamImplementation::ParamImplementation(float default_value,
                                         float max_value,
                                         float min_value,
                                         const std::string &name)
    : _default_value(default_value), _max_value(max_value), _min_value(min_value), _name(name) {
  _events.push_back(createEvent<DummyEvent>(default_value));
}

ParamImplementation::~ParamImplementation() {}

float ParamImplementation::valueForTime(double time) {
  std::lock_guard<std::mutex> events_mutex(_events_mutex);
  auto current_iterator = iteratorForTime(time);

  if (current_iterator == _events.end()) {
    return defaultValue();
  }
  return std::min(std::max((*current_iterator)->valueAtTime(time), minValue()), maxValue());
}

void ParamImplementation::valuesForTimeRange(float *values,
                                             size_t values_count,
                                             double start_time,
                                             double end_time) {
  if (values_count == 0) {
    return;
  }
  if (start_time == end_time) {
    float value = valueForTime(start_time);
    for (int i = 0; i < values_count; ++i) {
      values[i] = value;
    }
    return;
  }
  std::lock_guard<std::mutex> events_mutex(_events_mutex);
  memset(values, 0, values_count * sizeof(float));
  auto event_it = iteratorForTime(start_time);
  double step = (end_time - start_time) / (values_count - 1);
  double current_time = start_time;
  for (int i = 0; i < values_count; ++i, current_time += step) {
    if (event_it != _events.end() && current_time >= (*event_it)->end_time &&
        (*event_it)->end_time != ParamEvent::INVALID_TIME && event_it != _events.end()) {
      event_it++;
    }
    values[i] =
        (event_it == _events.end()) ? defaultValue() : (*event_it)->valueAtTime(current_time);
  }
}

std::string ParamImplementation::name() {
  return _name;
}

float ParamImplementation::smoothedValueForTimeRange(double start_time,
                                                     double end_time,
                                                     size_t samples) {
  if (_smoothed_samples_buffer.size() < samples) {
    _smoothed_samples_buffer.resize(samples);
  }
  valuesForTimeRange(&_smoothed_samples_buffer[0], samples, start_time, end_time);
  float average_value = 0.0f;
  for (size_t i = 0; i < samples; ++i) {
    average_value += _smoothed_samples_buffer[i];
  }
  float output_value = average_value / static_cast<float>(samples);
  return output_value;
}

float ParamImplementation::cumulativeValueForTimeRange(double start_time,
                                                       double end_time,
                                                       double precision) {
  auto param_iter = iteratorForTime(start_time);
  auto end_iter = iteratorForTime(end_time);

  if (param_iter == _events.end()) {
    return 0;
  }

  // special case where the whole time range is covered by one event
  if (param_iter == end_iter)
    return param_iter->get()->cumulativeValue(start_time, end_time, precision);

  // start with cumulative value in the start region
  float cumulative_value = 0.0f;
  auto current_end_time = param_iter->get()->end_time;

  if (current_end_time != ParamEvent::INVALID_TIME) {
    cumulative_value += param_iter->get()->cumulativeValue(start_time, current_end_time, precision);
  }

  // now get everything between the start event and the second to last event
  for (++param_iter; param_iter != end_iter; ++param_iter) {
    cumulative_value += param_iter->get()->cumulativeValue(
        param_iter->get()->start_time, param_iter->get()->end_time, precision);
  }

  // now get remainder
  if (param_iter != _events.end()) {
    cumulative_value +=
        param_iter->get()->cumulativeValue(param_iter->get()->start_time, end_time, precision);
  }

  // if we went past the last event, use default
  if (current_end_time < end_time) {
    cumulative_value += (end_time - current_end_time) * defaultValue();
  }

  return cumulative_value;
}

float ParamImplementation::defaultValue() const {
  return _default_value;
}

float ParamImplementation::maxValue() const {
  return _max_value;
}

float ParamImplementation::minValue() const {
  return _min_value;
}

void ParamImplementation::setValue(float value) {
  setValueAtTime(value, 0.0);
}

void ParamImplementation::setValueAtTime(float value, double time) {
  std::lock_guard<std::mutex> events_mutex(_events_mutex);
  auto prev_it = prevEvent(time);
  auto event = createEvent<ValueAtTimeEvent>(value, time);
  addEvent(std::move(event), prev_it);
}

void ParamImplementation::linearRampToValueAtTime(float end_value, double end_time) {
  {
    std::lock_guard<std::mutex> events_mutex(_events_mutex);
    auto prev_it = prevEvent(end_time);
    auto event = createEvent<LinearRampEvent>(end_value, end_time);
    addEvent(std::move(event), prev_it);
  }

  // implicit setValueAtTime to maintain the end_value
  setValueAtTime(end_value, end_time);
}

void ParamImplementation::exponentialRampToValueAtTime(float end_value, double end_time) {
  {
    std::lock_guard<std::mutex> events_mutex(_events_mutex);
    auto prev_it = prevEvent(end_time);
    auto event = createEvent<ExponentialRampEvent>(end_value, end_time);
    addEvent(std::move(event), prev_it);
  }

  // implicit setValueAtTime to maintain the end_value
  setValueAtTime(end_value, end_time);
}

void ParamImplementation::setTargetAtTime(float target, double start_time, float time_constant) {
  std::lock_guard<std::mutex> events_mutex(_events_mutex);
  auto prev_it = prevEvent(start_time);
  auto event = createEvent<TargetAtTimeEvent>(target, start_time, time_constant);
  if (prev_it != _events.end()) {
    event->start_value = (*prev_it)->endValue();
  }
  addEvent(std::move(event), prev_it);
}

void ParamImplementation::setValueCurveAtTime(std::vector<float> values,
                                              double start_time,
                                              double duration) {
  std::lock_guard<std::mutex> events_mutex(_events_mutex);
  auto prev_it = prevEvent(start_time);
  auto event = createEvent<ValueCurveEvent>(values, start_time, duration);
  addEvent(std::move(event), prev_it);
}

void ParamImplementation::addCustomEvent(double start_time,
                                         double end_time,
                                         Anchor anchor,
                                         NF_AUDIO_PARAM_FUNCTION function) {
  std::lock_guard<std::mutex> events_mutex(_events_mutex);
  auto prev_it = prevEvent(start_time);
  auto event = createEvent<CustomParamEvent>(start_time, end_time, anchor, function);
  addEvent(std::move(event), prev_it);
}

std::list<std::unique_ptr<ParamEvent>>::iterator ParamImplementation::iteratorForTime(double time) {
  if (time < 0.0) {
    return _events.end();
  }
  for (auto it = _events.begin(); it != _events.end(); it++) {
    EVENT_PTR &event = *it;
    if ((event->end_time > time || event->end_time == ParamEvent::INVALID_TIME) &&
        event->start_time <= time) {
      return it;
    }
  }
  return _events.end();
}

std::list<std::unique_ptr<ParamEvent>>::iterator ParamImplementation::prevEvent(double time) {
  auto prev_it = _events.end();
  double prev_time = 0.0;
  for (auto it = _events.begin(); it != _events.end(); it++) {
    double t = ((*it)->anchor & Anchor::END) == Anchor::END ? (*it)->end_time : (*it)->start_time;
    if (t >= prev_time && t <= time) {
      prev_time = t;
      prev_it = it;
    }
  }
  return prev_it;
}

bool ParamImplementation::getRequiredTimeRange(const EVENT_PTR &event, double &start, double &end) {
  if (event->anchor == Anchor::NONE) {
    return false;
  }
  start = ((event->anchor & Anchor::START) == Anchor::START) ? event->start_time : event->end_time;
  end = ((event->anchor & Anchor::END) == Anchor::END) ? event->end_time : event->start_time;
  return true;
}

void ParamImplementation::checkOverlap(const EVENT_PTR &event) {
  double s1, e1, s2, e2;
  if (!getRequiredTimeRange(event, s1, e1)) {
    return;
  }
  for (const auto &e : _events) {
    if (getRequiredTimeRange(e, s2, e2)) {
      if (s1 < e2 && s2 < e1) {
        std::stringstream msg;
        msg << "New event with required time range " << s1 << " - " << e1
            << " conflicts with existing event with required time range " << s2 << " - " << e2;
        throw std::invalid_argument(msg.str());
      }
    }
  }
}

void ParamImplementation::updateTimes(EVENT_PTR &prev, EVENT_PTR &event) {
  Anchor prev_anchor = prev->anchor;
  if ((event->anchor & Anchor::START) == Anchor::NONE) {
    // If event does not have a fixed start, get it from previous event
    double prev_anchor_time = 0.0;
    if ((prev_anchor & Anchor::END) == Anchor::END) {
      prev_anchor_time = prev->end_time;
    } else {
      // New event has no fixed start and previous event has no fixed end
      // set duration of previous event to 0 and steal its end value
      prev_anchor_time = prev->start_time;
      prev->end_time = prev_anchor_time;
      event->start_value = prev->endValue();
    }
    event->start_time = prev_anchor_time;
  } else {
    // If event does have a fixed start, give it to previous event
    if ((prev_anchor & Anchor::END) == Anchor::NONE) {
      prev->end_time = event->start_time;
    }
  }
}

void ParamImplementation::addEvent(EVENT_PTR new_event, std::list<EVENT_PTR>::iterator prev_event) {
  checkOverlap(new_event);

  auto next_event = prev_event;
  if (prev_event != _events.end()) {
    updateTimes(*prev_event, new_event);
    ++next_event;
    if (next_event != _events.end()) {
      updateTimes(new_event, *next_event);
    }
  }
  invalidateCachedCumulativeValuesAfterTime(new_event->start_time);
  _events.insert(next_event, std::move(new_event));
}

void ParamImplementation::invalidateCachedCumulativeValuesAfterTime(double time) {
  for (auto &precision_map_pair : _cumulative_values_cache) {
    auto &precision_map = precision_map_pair.second;
    for (auto &map_pair : precision_map) {
      if (map_pair.first >= time) {
        precision_map.erase(map_pair.first);
      }
    }
  }
}

std::shared_ptr<Param> createParam(float default_value,
                                   float max_value,
                                   float min_value,
                                   const std::string &name) {
  return std::make_shared<ParamImplementation>(default_value, max_value, min_value, name);
}

}  // namespace param
}  // namespace nativeformat
