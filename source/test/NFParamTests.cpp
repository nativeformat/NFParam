#define CATCH_CONFIG_MAIN

#include <NFParam/Param.h>
#include <NFParam/ParamEvent.h>
#include "../source/WAAParamEvents.h"

#include <catch.hpp>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <vector>

TEST_CASE("order of setValueAtTime commands should not matter") {
  auto p1 = nativeformat::param::createParam(0, 1, -1, "testParam1");
  p1->setValueAtTime(0.25f, 1.0);
  p1->setValueAtTime(0.5f, 2.0);
  p1->setValueAtTime(0.75f, 3.0);

  auto p2 = nativeformat::param::createParam(0, 1, -1, "testParam2");
  p2->setValueAtTime(0.75f, 3.0);
  p2->setValueAtTime(0.25f, 1.0);
  p2->setValueAtTime(0.5f, 2.0);

  const size_t value_count = 8;
  std::vector<float> v1(value_count), v2(value_count);
  p1->valuesForTimeRange(v1.data(), value_count, 0.0, 4.0);
  p2->valuesForTimeRange(v2.data(), value_count, 0.0, 4.0);

  // check that v1 values are correct
  std::vector<float> expected_vals{0.0f, 0.0f, 0.25f, 0.25f, 0.5f, 0.5f, .75f, .75f};
  for (int i = 0; i < value_count; ++i) {
    CHECK(v1[i] == expected_vals[i]);
  }

  // check that v1 and v2 values match
  for (int i = 0; i < value_count; ++i) {
    CHECK(v2[i] == v1[i]);
  }
}

TEST_CASE("exception should be thrown for event conflicts") {
  auto p = nativeformat::param::createParam(0, 1, -1, "testParam");
  std::vector<float> curve{1.0f, 2.0f, 4.0f, 2.0f, 1.0f};

  std::string actual;
  std::string expected =
      "New event with required time range 0 - 2 conflicts "
      "with existing event with required time range 1 - 1";
  try {
    p->setValueAtTime(0.25f, 1.0);
    p->setValueCurveAtTime(curve, 0.0, 2.0);
  } catch (const std::exception &e) {
    actual = e.what();
  }
  CHECK(actual == expected);

  p = nativeformat::param::createParam(0, 1, -1, "testParam");
  actual = "";
  expected =
      "New event with required time range 1 - 1 conflicts with existing "
      "event with required time range 0 - 2";
  try {
    p->setValueCurveAtTime(curve, 0.0, 2.0);
    p->setValueAtTime(0.25f, 1.0);
  } catch (const std::exception &e) {
    actual = e.what();
  }
  CHECK(actual == expected);
}

TEST_CASE("param values should be clamped to min and max") {
  auto p = nativeformat::param::createParam(0, 1.0f, -1.0f, "testParam");
  p->setValueAtTime(-0.5f, 0.0);
  p->linearRampToValueAtTime(2.0f, 1.0);

  CHECK(p->valueForTime(0.0) == -0.5f);
  CHECK(p->valueForTime(1.0) == 1.0f);
}

TEST_CASE("full automation example from WAA spec should work") {
  // create curve for setCurveAtTime
  size_t curve_len = 44100;
  std::vector<float> curve(curve_len);
  for (int i = 0; i < curve_len; ++i) {
    curve[i] = std::sin((3.14159265 * i) / curve_len);
  }

  // calculate expected end point for setTargetAtTime
  float time_constant = 0.1f;
  float set_target_end_val = 0.5 + 0.3 * std::exp(-0.175 / time_constant);

  auto p = nativeformat::param::createParam(0, 1, -1, "testParam");
  p->setValueAtTime(0.2f, 0.0);
  p->setValueAtTime(0.3f, 0.1);
  p->setValueAtTime(0.4f, 0.2);
  p->linearRampToValueAtTime(1.0f, 0.3);
  p->linearRampToValueAtTime(0.8f, 0.325);
  p->setTargetAtTime(0.5f, 0.325, time_constant);
  p->setValueAtTime(0.552f, 0.5);
  p->exponentialRampToValueAtTime(0.75f, 0.6);
  p->exponentialRampToValueAtTime(0.05f, 0.7);
  p->setValueCurveAtTime(curve, 0.7, 0.3);

  size_t points = 1001;
  std::vector<float> x(points), y(points);
  p->valuesForTimeRange(y.data(), points, 0.0, 1.0);

  double ts = 1.0 / (points - 1);
  double t = 0;
  for (int i = 0; i < x.size(); ++i) {
    x[i] = t;
    t += ts;
  }

  // Write the generated output to a file for plotting
  std::ofstream output_file("paramAutomation.txt");
  for (int i = 0; i < x.size(); ++i) {
    output_file << x[i] << "\t" << y[i] << std::endl;
  }

  // Read test vector from file
  std::string path = "resources/paramAutomationExpected.txt";
  FILE *input_file = std::fopen(path.c_str(), "r");

  // Account for different build folder depths
  int i = 0;
  while (!input_file && i++ < 10) {
    path = "../" + path;
    input_file = std::fopen(path.c_str(), "r");
  }

  float tmp1, tmp2;
  for (int i = 0; i < x.size(); ++i) {
    fscanf(input_file, "%f\n%f\n", &tmp1, &tmp2);
    CHECK(y[i] == Approx(tmp2));
  }
}

TEST_CASE("automation example should work with reordered commands") {
  // create curve for setCurveAtTime
  size_t curve_len = 44100;
  std::vector<float> curve(curve_len);
  for (int i = 0; i < curve_len; ++i) {
    curve[i] = std::sin((3.14159265 * i) / curve_len);
  }

  // calculate expected end point for setTargetAtTime
  float time_constant = 0.1f;
  float set_target_end_val = 0.5 + 0.3 * std::exp(-0.175 / time_constant);

  auto p = nativeformat::param::createParam(0, 1, -1, "testParam");
  p->setValueAtTime(0.2f, 0.0);
  p->exponentialRampToValueAtTime(0.05f, 0.7);
  p->setValueAtTime(0.3f, 0.1);
  p->linearRampToValueAtTime(1.0f, 0.3);
  p->setValueAtTime(0.4f, 0.2);
  p->linearRampToValueAtTime(0.8f, 0.325);
  p->setValueAtTime(0.552f, 0.5);
  p->setTargetAtTime(0.5f, 0.325, time_constant);
  p->exponentialRampToValueAtTime(0.75f, 0.6);
  p->setValueCurveAtTime(curve, 0.7, 0.3);

  size_t points = 1001;
  std::vector<float> x(points), y(points);
  p->valuesForTimeRange(y.data(), points, 0.0, 1.0);

  double ts = 1.0 / (points - 1);
  double t = 0;
  for (int i = 0; i < x.size(); ++i) {
    x[i] = t;
    t += ts;
  }

  // Write the generated output to a file for plotting
  std::ofstream output_file("paramAutomation_reordered.txt");
  for (int i = 0; i < x.size(); ++i) {
    output_file << x[i] << "\t" << y[i] << std::endl;
  }

  // Read test vector from file
  std::string path = "resources/paramAutomationExpected.txt";
  FILE *input_file = std::fopen(path.c_str(), "r");

  // Account for different build folder depths
  int i = 0;
  while (!input_file && i++ < 10) {
    path = "../" + path;
    input_file = std::fopen(path.c_str(), "r");
  }

  float tmp1, tmp2;
  for (int i = 0; i < x.size(); ++i) {
    fscanf(input_file, "%f\n%f\n", &tmp1, &tmp2);
    CHECK(y[i] == Approx(tmp2));
  }
}

TEST_CASE("Events should return start or end value for times out of range") {
  nativeformat::param::LinearRampEvent lre(2.0, 2.0);
  nativeformat::param::ExponentialRampEvent ere(2.0, 1.0);
  nativeformat::param::ValueAtTimeEvent vte(5.0, 1.0);
  nativeformat::param::TargetAtTimeEvent tte(1.0, 1.0, 1.0);

  // check at t = -1, should be the start_value for all events
  CHECK(lre.valueAtTime(-1.0) == Approx(0.0));
  CHECK(ere.valueAtTime(-1.0) == Approx(0.0));
  CHECK(vte.valueAtTime(-1.0) == Approx(5.0));
  CHECK(tte.valueAtTime(-1.0) == Approx(0.0));

  // check at t -> infinity (30 is close enough)
  CHECK(lre.valueAtTime(30.0) == Approx(2.0));
  CHECK(ere.valueAtTime(30.0) == Approx(2.0));
  CHECK(vte.valueAtTime(30.0) == Approx(5.0));
  CHECK(tte.valueAtTime(30.0) == Approx(1.0));
}

TEST_CASE("A linear ramp event should integrate nicely") {
  nativeformat::param::LinearRampEvent event(2.0, 2.0);
  CHECK(event.cumulativeValue(0.0, 2.0) == Approx(2.0f));
}

TEST_CASE("The integral over a linear ramp should include initial value") {
  nativeformat::param::LinearRampEvent event(2.0, 2.0);
  event.start_value = 1.0;
  CHECK(event.cumulativeValue(0.0, 2.0) == Approx(3.0f));
}

TEST_CASE("A exponential ramp event should integrate nicely") {
  nativeformat::param::ExponentialRampEvent event(2.0, 1.0);
  event.start_value = 1.0;
  CHECK(event.cumulativeValue(0.0, 1.0) == Approx(2.442695f));
}

TEST_CASE("Value at time has the integral of the value at that time") {
  nativeformat::param::ValueAtTimeEvent event(5.0, 1.0);
  CHECK(event.cumulativeValue(0.0, 2.0) == Approx(10.0f));
}

TEST_CASE("Target at time event should integrate nicely") {
  nativeformat::param::TargetAtTimeEvent event(1.0, 1.0, 1.0);
  event.start_value = 0.0;
  CHECK(event.cumulativeValue(0.0, 1.0) == Approx(0.3678794f));
}

TEST_CASE("The cumulative value over multiple events should work") {
  auto p = nativeformat::param::createParam(0.0f, 4.0f, 0.0f, "testParam");
  p->linearRampToValueAtTime(2.0f, 1.0);
  p->linearRampToValueAtTime(4.0f, 2.0);
  auto cv = p->cumulativeValueForTimeRange(0., 2.);
  INFO("cumulative value: " << cv);
  CHECK(p->cumulativeValueForTimeRange(0., 2.0f) == Approx(6.0));
}

TEST_CASE("CustomParamEvent should allow an arbitrary value function") {
  float default_val = 0.5;
  double event_end = 4.0;
  auto p = nativeformat::param::createParam(default_val, 16.0f, 0.0f, "testParam");
  p->addCustomEvent(
      0.0, event_end, nativeformat::param::Anchor::ALL, [](float t) { return t * t; });
  size_t count = 11;
  std::vector<float> values(count, 0.0);
  p->valuesForTimeRange(values.data(), count, 0.0, 5.0);

  // Check expected values within the custom event
  // Value should return to default for t > 4.0
  std::vector<float> expected_values{0.0, 0.25, 1.0, 2.25, 4.0, 6.25, 9.0, 12.25, 0.5, 0.5, 0.5};
  for (size_t i = 0; i < values.size(); ++i) {
    CHECK(values[i] == Approx(expected_values[i]));
  }

  // Check that defaults are preserved elsewhere
  float v = p->valueForTime(5.0);
  CHECK(v == Approx(default_val));

  // Check trapezoid rule integral
  float area = p->cumulativeValueForTimeRange(0.0, event_end);
  float expected_area = 64.0 / 3;
  CHECK(area == Approx(expected_area).epsilon(0.01));

  // Check that smoothed value is the average
  float avg = p->smoothedValueForTimeRange(0.0, 4.0, 1000);
  float expected_avg = expected_area / 4.0;
  CHECK(avg == Approx(expected_avg).epsilon(0.001));

  // Check that integral continues to work after final event
  double box_time = 2.0;
  area = p->cumulativeValueForTimeRange(0.0, event_end + box_time, 0.0001);
  expected_area += default_val * box_time;
  CHECK(area == Approx(expected_area).epsilon(0.01));
}

TEST_CASE("Param with no events should return default value") {
  float default_value = 0.7;
  size_t count = 4;
  auto p = nativeformat::param::createParam(default_value, 2.0f, -2.0f, "testParam");
  std::vector<float> values(count, 0.0);

  float v = p->valueForTime(1.0);
  p->valuesForTimeRange(values.data(), count, 0.0, 3.0);

  CHECK(v == Approx(default_value));
  for (auto val : values) {
    CHECK(val == Approx(default_value));
  }
}

TEST_CASE("valuesForTimeRange should return constant value when start == end") {
  auto p = nativeformat::param::createParam(0, 2.0f, -2.0f, "testParam");
  p->setValueAtTime(1.5f, 1.0);
  size_t count = 4;
  std::vector<float> values(count, 0.0);
  p->valuesForTimeRange(values.data(), count, 3.0, 3.0);
  for (auto val : values) {
    CHECK(val == Approx(values.front()));
  }
}

TEST_CASE("valuesForTimeRange should do nothing when count == 0") {
  auto p = nativeformat::param::createParam(0, 2.0f, -2.0f, "testParam");
  p->setValueAtTime(1.5f, 1.0);
  std::vector<float> values;
  p->valuesForTimeRange(values.data(), 0, 3.0, 3.0);
  for (auto val : values) {
    CHECK(val == Approx(0.0));
  }
}

TEST_CASE("integral should be 0 if start = end or start < 0") {
  auto p = nativeformat::param::createParam(0, 2.0f, -2.0f, "testParam");
  p->setValueAtTime(1.5f, 1.0);
  CHECK(p->cumulativeValueForTimeRange(1.0, 1.0) == Approx(0));
  CHECK(p->cumulativeValueForTimeRange(-2.0, -1.0) == Approx(0));
  CHECK(p->name() == "testParam");
}

TEST_CASE("setValue should set value for all times") {
  auto p = nativeformat::param::createParam(0, 2.0f, -2.0f, "testParam");
  float v = 1.13f;
  p->setValue(v);
  CHECK(p->valueForTime(0.0) == Approx(v));
  CHECK(p->valueForTime(10.0) == Approx(v));
  CHECK(p->valueForTime(100.0) == Approx(v));
}
