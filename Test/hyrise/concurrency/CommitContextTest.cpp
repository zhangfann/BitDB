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

MY_TEST(CommitContextTest) {
  auto context = std::make_unique<CommitContext>(0u);

  TESTING_EXPECT_EQ(context->has_next(), false);

  TESTING_END;
}

MY_TEST(HasNextReturnsTrueAfterCallingGetOrCreateNext) {
  auto context = std::make_unique<CommitContext>(0u);

  context->get_or_create_next();

  TESTING_EXPECT_EQ(context->has_next(), true);

  TESTING_END;
}

MY_TEST(CidOfNextIncrementedByOne) {
  auto context = std::make_unique<CommitContext>(0u);

  auto next = context->get_or_create_next();

  TESTING_EXPECT_EQ(context->commit_id() + 1u, next->commit_id());

  TESTING_END;
}

// }  // namespace opossum


int main(int argc, char** argv) {
  CommitContextTest();
  HasNextReturnsTrueAfterCallingGetOrCreateNext();
  CidOfNextIncrementedByOne();
  // PutTest();
//   PutMultipleTest();
}