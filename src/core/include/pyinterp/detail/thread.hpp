#pragma once
#include <thread>
#include <vector>

namespace pyinterp {
namespace detail {

/// Automates the cutting of vectors to be processed in thread.
///
/// @param worker Lambda function called in each thread launched
/// @param size Size of all vectors to be processed
/// @param num_threads The number of threads to use for the computation. If 0
/// all CPUs are used. If 1 is given, no parallel computing code is used at all,
/// which is useful for debugging.
/// @tparam Lambda Lambda function
template <typename Lambda>
void dispatch(Lambda worker, size_t size, size_t num_threads = 0) {
  if (num_threads == 1) {
    worker(0, size);
    return;
  }

  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
  }

  // List of threads responsible for parallelizing the calculation
  std::vector<std::thread> threads(num_threads);

  // Access index to the vectors required for calculation
  size_t start = 0, shift = size / num_threads;

  // Launch and join threads
  for (auto it = std::begin(threads); it != std::end(threads) - 1; ++it) {
    *it = std::thread(worker, start, start + shift);
    start += shift;
  }
  threads.back() = std::thread(worker, start, size);

  for (auto&& item : threads) {
    item.join();
  }
}

}  // namespace detail
}  // namespace pyinterp