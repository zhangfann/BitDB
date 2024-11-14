#include <memory>
#include <string>
#include <utility>

#include "benchmark/benchmark.h"

#include "hyrise/operators/get_table.hpp"
#include "hyrise/operators/table_scan.hpp"
#include "hyrise/storage/storage_manager.hpp"
#include "hyrise/storage/table.hpp"
#include "hyrise/types.hpp"
#include "hyrise/benchmark/base_fixture.cpp"
#include "hyrise/benchmark/table_generator.hpp"

namespace opossum {

BENCHMARK_F(BenchmarkFixture, BM_TableScan)(benchmark::State& state) {
  clear_cache();
  auto warm_up = std::make_shared<TableScan>(_gt_a, "a", ">=", 7);
  warm_up->execute();
  while (state.KeepRunning()) {
    auto table_scan = std::make_shared<TableScan>(_gt_a, "a", ">=", 7);
    table_scan->execute();
  }
}

}  // namespace opossum

BENCHMARK_MAIN();