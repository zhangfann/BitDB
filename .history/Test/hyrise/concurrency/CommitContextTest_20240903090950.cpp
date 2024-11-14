#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hyrise/concurrency/Utils/Testing.h"
// #include "gtest/gtest.h"

#include "hyrise/concurrency/commit_context.hpp"
#include "hyrise/types.hpp"

using namespace opossum;
// namespace opossum {

// class CommitContextTest : public BaseTest {
//  protected:
//   // void SetUp() override {}
// };

TEST(CommitContextTest) {
  auto context = std::make_unique<CommitContext>(0u);

  EXPECT_EQ(context->has_next(), false);

  END;
}

TEST(HasNextReturnsTrueAfterCallingGetOrCreateNext) {
  auto context = std::make_unique<CommitContext>(0u);

  context->get_or_create_next();

  EXPECT_EQ(context->has_next(), true);

  END;
}

TEST(CidOfNextIncrementedByOne) {
  auto context = std::make_unique<CommitContext>(0u);

  auto next = context->get_or_create_next();

  EXPECT_EQ(context->commit_id() + 1u, next->commit_id());

  END;
}

// }  // namespace opossum


int main(int argc, char** argv) {
  CommitContextTest();
  HasNextReturnsTrueAfterCallingGetOrCreateNext();
  CidOfNextIncrementedByOne();
  // PutTest();
//   PutMultipleTest();
}