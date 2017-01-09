//==============================================================================
//
//                                  InsideLoop
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.txt for details.
//
//==============================================================================

#include <random>
#include <unordered_map>

#include <il/HashMap.h>
#include <il/Timer.h>
#include <il/format.h>

#include <google/dense_hash_map>
#include <llvm/ADT/DenseMap.h>

int main() {
  il::Timer timer{};
//  const il::int_t n = 1 << 26;
  const il::int_t n = 1 << 26;
//  il::int_t n = 10;
  const bool inside_loop = true;
  il::int_t sum_il = 0;
  const bool llvm = false;
  il::int_t sum_llvm = 0;
  const bool google = true;
  il::int_t sum_google = 0;
  const bool stl = false;
  il::int_t sum_stl = 0;

  std::default_random_engine engine{};
  std::uniform_int_distribution<il::int_t> dist{
      0, std::numeric_limits<il::int_t>::max() / 16};
  il::Array<il::int_t> v{n};
  for (il::int_t k = 0; k < n; ++k) {
    v[k] = 8 * dist(engine) + 3;
  }
  il::print("{:>20}: {:>10n}\n", "n", n);

  //////////////////////////////////////////////////////////////////////////////
  // Timing Inside Loop
  //////////////////////////////////////////////////////////////////////////////
  if (inside_loop) {
    timer.start();
    il::HashMap<il::int_t, il::int_t> map{2 * n};
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      il::int_t i = map.search(key);
      if (!map.found(i)) {
        map.insert(key, k, il::io, i);
      } else {
        map.value(i) = k;
      }
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time insert IL", timer.elapsed());

    timer.reset();
    timer.start();
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      const il::int_t i = map.search(key);
      sum_il += map.value(i);
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time search IL", timer.elapsed());
    il::print("{:>20}: {:>10n}\n", "Number of slots", map.capacity());
    timer.reset();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Timing LLVM
  //////////////////////////////////////////////////////////////////////////////
  if (llvm) {
    timer.start();
    llvm::DenseMap<il::int_t, il::int_t> map_llvm(n);
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      map_llvm[key] = k;
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time insert LLVM", timer.elapsed());

    timer.reset();
    timer.start();
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      sum_llvm += map_llvm[key];
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time search LLVM", timer.elapsed());
    timer.reset();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Timing Google Dense Hash
  //////////////////////////////////////////////////////////////////////////////
  if (google) {
    timer.start();
    google::dense_hash_map<il::int_t, il::int_t> map_google{
        static_cast<std::size_t>(n)};
    map_google.set_empty_key(std::numeric_limits<il::int_t>::max());
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      map_google[key] = k;
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time insert Google", timer.elapsed());

    timer.reset();
    timer.start();
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      sum_google += map_google[key];
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time search Google", timer.elapsed());
    il::print("{:>20}: {:>10n}\n", "Number of slots", map_google.bucket_count());
    timer.reset();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Timing STL
  //////////////////////////////////////////////////////////////////////////////
  if (stl) {
    timer.start();
    std::unordered_map<il::int_t, il::int_t> map_stl{};
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      map_stl[key] = k;
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time insert STL", timer.elapsed());

    timer.reset();
    timer.start();
    for (il::int_t k = 0; k < n; ++k) {
      const il::int_t key = v[k];
      sum_stl += map_stl[key];
    }
    timer.stop();

    il::print("{:>20}: {:>10} s\n", "Time search STL", timer.elapsed());
    timer.reset();
  }

  il::print("\n{} {} {}\n", sum_il, sum_google, sum_stl);

  return 0;
}