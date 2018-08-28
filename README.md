<img alt="NFParam" src="NFParam.png" width="100%" max-width="888">

[![CircleCI](https://circleci.com/gh/spotify/NFParam/tree/master.svg?style=svg)](https://circleci.com/gh/spotify/NFParam/tree/master)
[![License](https://img.shields.io/github/license/spotify/NFParam.svg)](LICENSE)
[![Spotify FOSS Slack](https://slackin.spotify.com/badge.svg)](https://slackin.spotify.com)
[![Readme Score](http://readme-score-api.herokuapp.com/score.svg?url=https://github.com/spotify/nfparam)](http://clayallsopp.github.io/readme-score?url=https://github.com/spotify/nfparam)

A C++ library for defining and evaluating piecewise functions, inspired by the Web Audio API [AudioParam](https://webaudio.github.io/web-audio-api/#AudioParam) interface.

## Platform support
`NFParam` should compile and run on any platform with a C++11 compiler, but the following are tested in CI.

- [x] 📱 iOS 9.0+
- [x] 💻 OS X 10.11+
- [x] 🐧 Ubuntu Trusty 14.04+ (clang 3.9 or gcc 4.9)

## Raison D'être :thought_balloon:
When designing a cross platform player that could be used for complex mixing and effects, we required a library that worked in the same way that the Web Audio API [AudioParam](https://webaudio.github.io/web-audio-api/#AudioParam) worked but on none web based platforms. This led to the creation of this library, which is not only able to emulate the [AudioParam](https://webaudio.github.io/web-audio-api/#AudioParam) library but can also handle seeks into the centre of a function being evaluated due to its architecture not being a state machine. In addition to supporting everything [AudioParam](https://webaudio.github.io/web-audio-api/#AudioParam) supports, we have also added in some extra goodies such as `smoothedValueForTimeRange` and `cumulativeValueForTimeRange`.

## Architecture :triangular_ruler:
`NFParam` is designed as a C++11 interface to define a control curve and interact with it in real time. The API allows you to create a parameter and then begin to add control curves to execute at specific times. The library is thread safe and can be written or read from any thread. The system works by having a list of events, doing a binary search on that list to find the correct function to execute, then executing that function on the current time being requested.

## Installation
CMake 3.5 or later is required to generate the build.
```shell
brew install cmake
```
or
```shell
sudo apt-get install cmake
```

`NFParam` is a [CMake](https://cmake.org/) project, so to use it within a larger CMake project, simply add the following line to your `CMakeLists.txt` file:
```
add_subdirectory(NFParam)
``` 
Alternatively, you can compile NFParam as a standalone library.
The [ci build scripts](./ci) can be used to install all necessary dependencies, build the library, and run unit tests.

### For Linux
```shell
sh ci/linux.sh build
```

### For iOS/OSX
```shell
sh ci/osx.sh build
```

## Examples

### Create the "Audio Param Example" curve pictured below 

#### Create the `Param`
Specify the default value, min, max, and a name.
Until events are added, its value will be the default value for all times.
```
auto p = nativeformat::param::createParam(0, 1, -1, "testParam");
```

#### Add some events
The `setValueAtTime` command behaves as a step function.
The value specified will be maintained until the next event anchor.
```
p->setValueAtTime(0.2f, 0.0);
p->setValueAtTime(0.3f, 0.1);
p->setValueAtTime(0.4f, 0.2);
```

The linearRampToValueAtTime event computes the slope necessary to 
reach the target value from the previous anchor point by the specified time.
```
p->linearRampToValueAtTime(1.0f, 0.3);
p->linearRampToValueAtTime(0.8f, 0.325);
```

The setTargetAtTime command will expoenentially approach the target value
from the previous anchor with a rate specified by the time constant.
It does not have a fixed end time.
```
p->setTargetAtTime(0.5f, 0.325, time_constant);
p->setValueAtTime(0.552f, 0.5);
```

The `exponentialRampToValueAtTime` event computes the constant necessary to
reach the target value from the previous anchor point by the specified time.
```
p->exponentialRampToValueAtTime(0.75f, 0.6);
p->exponentialRampToValueAtTime(0.05f, 0.7);
```

The `setValueCurveAtTime` event linearly interpolates between the points provided on the user-defined curve.
It cannot overlap with any other command anchors.
```
size_t curve_len = 44100;
std::vector<float> curve(curve_len);
for (int i = 0; i < curve_len; ++i) {
  curve[i] = std::sin((3.14159265 * i) / curve_len);
}
p->setValueCurveAtTime(curve, 0.7, 0.3);
```
#### Retrieve some values from the `Param`
Finally, let's sample some values from the param we have defined!
```
size_t points = 1001;
std::vector<float> x(points), y(points);
p->valuesForTimeRange(y.data(), points, 0.0, 1.0);

double ts = 1.0 / (points - 1);
double t = 0;
for (int i = 0; i < x.size(); ++i) {
  x[i] = t;
  t += ts;
}
```

## Tests
[`NFParamTests.cpp`](source/test/NFParamTests.cpp) contains a number of test cases,
two of which generate TSV output files that should match the
[AudioParam automation example](https://webaudio.github.io/web-audio-api/#example1-AudioParam)
from the WAA spec. In the `resources` directory, there is a [script](resources/plot.sh) that will plot the unit test output data
if you have [gnuplot](http://gnuplot.info/) installed. The path where `paramAutomation.txt` is generated when the tests are run by the 
ci scripts varies by platform.

### Generating a plot
```
sh plot.sh ../build/source/test/Debug/paramAutomation.txt out.png
```
![](resources/paramAutomationExpected.png?raw=true)

## Contributing :mailbox_with_mail:
Contributions are welcomed, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License :memo:
The project is available under the [Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0) license.

### Acknowledgements
- Icon in readme banner is “[Settings](https://thenounproject.com/search/?q=parameter&i=1477820)” by Bharat from the Noun Project.

#### Contributors
* [Julia Cox](https://github.com/astrocox)
* [Justin Sarma](https://github.com/jsarma)
* [David Rubinstein](https://github.com/drubinstein)
* [Will Sackfield](https://github.com/8W9aG)
