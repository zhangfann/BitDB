#include "testing_assert.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hyrise/all_type_variant.hpp"
#include "hyrise/logical_query_plan/abstract_lqp_node.hpp"
#include "hyrise/storage/table.hpp"
#include "hyrise/storage/value_segment.hpp"

namespace hyrise {

bool check_lqp_tie(const std::shared_ptr<const AbstractLQPNode>& output, LQPInputSide input_side,
                   const std::shared_ptr<const AbstractLQPNode>& input) {
  auto outputs = input->outputs();
  for (const auto& output2 : outputs) {
    if (!output2) {
      return false;
    }

    if (output == output2 && output2->input(input_side) == input) {
      return true;
    }
  }

  return false;
}

}  // namespace hyrise