//==============================================================================
//
// Copyright 2017 The InsideLoop Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//==============================================================================

#include <benchmark/benchmark.h>

#include <vector>
#include <il/Array.h>

static void IlArray_Append(benchmark::State& state) {
  while (state.KeepRunning()) {
    il::Array<double> v{};
    v.append(0.0);
    benchmark::DoNotOptimize(v.data());
  }
}

static void StdArray_Append(benchmark::State& state) {
  while (state.KeepRunning()) {
    std::vector<double> v{};
    v.push_back(0.0);
    benchmark::DoNotOptimize(v.data());
  }
}

BENCHMARK(IlArray_Append);
BENCHMARK(StdArray_Append);
