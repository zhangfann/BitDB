#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "hyrise/concurrency/Utils/Testing.h"
// #include "hyrise/concurrency/Utils/base_test.hpp"
// #include "gtest/gtest.h"

#include "hyrise/concurrency/transaction_context.hpp"
#include "hyrise/concurrency/transaction_manager.hpp"
#include "hyrise/types.hpp"

using namespace opossum;

// class TransactionManagerTest : public BaseTest {
//  protected:
//   void SetUp() override {}

//   void TearDown() override { TransactionManager::reset(); }

//   TransactionManager& manager() { return TransactionManager::get(); }
// };
  TransactionManager& manager() { return TransactionManager::get(); }
MY_TEST(NonActiveTransactionCannotBeAborted) {
  auto context = manager().new_transaction_context();

  manager().prepare_commit(*context);

  TESTING_EXPECT_TRUE(context->phase() == TransactionPhase::Committing);
  try
  {
    manager().abort(*context);
  }
  catch(const std::logic_error& e)
  {
    std::cerr << e.what() << '\n';    
  }
  
  // EXPECT_THROW(manager().abort(*context), std::logic_error);
TransactionManager::reset();
  TESTING_END;
}

MY_TEST( CommitShouldCommitAllFollowingPendingTransactions) {
  // 事务id是1
  auto context_1 = manager().new_transaction_context();
  // 事务id是2
  auto context_2 = manager().new_transaction_context();
  
  // commit id是1
  manager().prepare_commit(*context_1);
  // commit id是2
  manager().prepare_commit(*context_2);

  const auto prev_last_commit_id = manager().last_commit_id();

  manager().commit(*context_2);

  TESTING_EXPECT_EQ(prev_last_commit_id , manager().last_commit_id());

  manager().commit(*context_1);

  TESTING_EXPECT_EQ(context_2->commit_id() , manager().last_commit_id());

TransactionManager::reset();
  TESTING_END;
}

// Until the commit context is committed and the last commit id incremented,
// the commit context is held in a single linked list of shared_ptrs and hence not deleted.

MY_TEST( CommitContextGetsOnlyDeletedAfterCommitting) {
  auto context_1 = manager().new_transaction_context();
  auto context_2 = manager().new_transaction_context();

  manager().prepare_commit(*context_1);
  manager().prepare_commit(*context_2);

  auto commit_context_1 = std::weak_ptr<CommitContext>(context_1->commit_context());
  auto commit_context_2 = std::weak_ptr<CommitContext>(context_2->commit_context());

  TESTING_EXPECT_FALSE(commit_context_1.expired());
  TESTING_EXPECT_FALSE(commit_context_2.expired());

  manager().commit(*context_2);
  context_2 = nullptr;

  TESTING_EXPECT_FALSE(commit_context_1.expired());
  TESTING_EXPECT_FALSE(commit_context_2.expired());

  manager().commit(*context_1);
  context_1 = nullptr;

  auto context_3 = manager().new_transaction_context();
  manager().prepare_commit(*context_3);

  TESTING_EXPECT_TRUE(commit_context_1.expired());
  TESTING_EXPECT_TRUE(commit_context_2.expired());

TransactionManager::reset();
  TESTING_END;
}

MY_TEST( CallbackFiresWhenCommitted) {
  auto context_1 = manager().new_transaction_context();
  auto context_2 = manager().new_transaction_context();

  manager().prepare_commit(*context_1);
  manager().prepare_commit(*context_2);

  auto context_1_committed = false;
  auto callback_1 = [&context_1_committed](TransactionID) { context_1_committed = true; };

  auto context_2_committed = false;
  auto callback_2 = [&context_2_committed](TransactionID) { context_2_committed = true; };

  manager().commit(*context_2, callback_2);

  TESTING_EXPECT_FALSE(context_2_committed);

  manager().commit(*context_1, callback_1);

  TESTING_EXPECT_TRUE(context_1_committed);
  TESTING_EXPECT_TRUE(context_2_committed);

TransactionManager::reset();
  TESTING_END;
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
