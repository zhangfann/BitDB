#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "KV/Transaction/Utils/Testing.h"
// #include "KV/Transaction/Utils/base_test.hpp"
// #include "gtest/gtest.h"

#include "KV/Transaction/transaction_context.hpp"
#include "KV/Transaction/transaction_manager.hpp"
#include "KV/Transaction/types.hpp"

using namespace opossum;

// class TransactionManagerTest : public BaseTest {
//  protected:
//   void SetUp() override {}

//   void TearDown() override { TransactionManager::reset(); }

//   TransactionManager& manager() { return TransactionManager::get(); }
// };
  TransactionManager& manager() { return TransactionManager::get(); }
TEST(NonActiveTransactionCannotBeAborted) {
  auto context = manager().new_transaction_context();

  manager().prepare_commit(*context);

  EXPECT_TRUE(context->phase() == TransactionPhase::Committing);
  // EXPECT_THROW(manager().abort(*context), std::logic_error);

  END;
}

TEST( CommitShouldCommitAllFollowingPendingTransactions) {
  auto context_1 = manager().new_transaction_context();
  auto context_2 = manager().new_transaction_context();

  manager().prepare_commit(*context_1);
  manager().prepare_commit(*context_2);

  const auto prev_last_commit_id = manager().last_commit_id();

  manager().commit(*context_2);

  EXPECT_EQ(prev_last_commit_id , manager().last_commit_id());

  manager().commit(*context_1);

  EXPECT_EQ(context_2->commit_id() , manager().last_commit_id());

  END;
}

// Until the commit context is committed and the last commit id incremented,
// the commit context is held in a single linked list of shared_ptrs and hence not deleted.
TEST( CommitContextGetsOnlyDeletedAfterCommitting) {
  auto context_1 = manager().new_transaction_context();
  auto context_2 = manager().new_transaction_context();

  manager().prepare_commit(*context_1);
  manager().prepare_commit(*context_2);

  auto commit_context_1 = std::weak_ptr<CommitContext>(context_1->commit_context());
  auto commit_context_2 = std::weak_ptr<CommitContext>(context_2->commit_context());

  EXPECT_FALSE(commit_context_1.expired());
  EXPECT_FALSE(commit_context_2.expired());

  manager().commit(*context_2);
  context_2 = nullptr;

  EXPECT_FALSE(commit_context_1.expired());
  EXPECT_FALSE(commit_context_2.expired());

  manager().commit(*context_1);
  context_1 = nullptr;

  auto context_3 = manager().new_transaction_context();
  manager().prepare_commit(*context_3);

  EXPECT_TRUE(commit_context_1.expired());
  EXPECT_TRUE(commit_context_2.expired());

  END;
}

TEST( CallbackFiresWhenCommitted) {
  auto context_1 = manager().new_transaction_context();
  auto context_2 = manager().new_transaction_context();

  manager().prepare_commit(*context_1);
  manager().prepare_commit(*context_2);

  auto context_1_committed = false;
  auto callback_1 = [&context_1_committed](TransactionID) { context_1_committed = true; };

  auto context_2_committed = false;
  auto callback_2 = [&context_2_committed](TransactionID) { context_2_committed = true; };

  manager().commit(*context_2, callback_2);

  EXPECT_FALSE(context_2_committed);

  manager().commit(*context_1, callback_1);

  EXPECT_TRUE(context_1_committed);
  EXPECT_TRUE(context_2_committed);

  END;
}


int main(int argc, char** argv) {
  NonActiveTransactionCannotBeAborted();
  CommitShouldCommitAllFollowingPendingTransactions();
  CommitContextGetsOnlyDeletedAfterCommitting();
  CallbackFiresWhenCommitted();
  // PutTest();
//   PutMultipleTest();
}

// }  // namespace opossum
