#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "KV/Transaction1/Utils/Testing.h"
// #include "gtest/gtest.h"

#include "KV/Transaction/commit_context.hpp"
#include "KV/Transaction/types.hpp"

using namespace opossum;
// namespace opossum {

// class CommitContextTest : public BaseTest {
//  protected:
//   // void SetUp() override {}
// };

TEST(CommitContext1Test) {
  auto context = std::make_unique<CommitContext>(0u);

  EXPECT_EQ(context->has_next(), false);
}

// TEST_F(CommitContextTest, HasNextReturnsTrueAfterCallingGetOrCreateNext) {
//   auto context = std::make_unique<CommitContext>(0u);

//   context->get_or_create_next();

//   EXPECT_EQ(context->has_next(), true);
// }

// TEST_F(CommitContextTest, CidOfNextIncrementedByOne) {
//   auto context = std::make_unique<CommitContext>(0u);

//   auto next = context->get_or_create_next();

//   EXPECT_EQ(context->commit_id() + 1u, next->commit_id());
// }

// }  // namespace opossum
