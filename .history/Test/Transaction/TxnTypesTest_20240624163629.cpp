
#include "KV/Transaction/Txn.h"

#include <string>

#include "KV/Transaction/TxnProcessor.h"
#include "KV/Transaction/TxnTypes.h"
#include "KV/Transaction/Utils/Testing.h"

TEST(NoopTest) {
  TxnProcessor p(SERIAL);

  Txn* t = new Noop();
  EXPECT_EQ(INCOMPLETE, t->Status());

  p.NewTxnRequest(t);
  p.GetTxnResult();

  EXPECT_EQ(COMMITTED, t->Status());
  delete t;

  END;
}

TEST(PutTest) {
  TxnProcessor p(SERIAL);
  Txn* t;

    std::map<Key, Value> m ={{1,2}};
  p.NewTxnRequest(new Put(m));
  delete p.GetTxnResult();

    std::map<Key, Value> m1 ={{0,2}};
  p.NewTxnRequest(new Expect(m1));  // Should abort (no key '0' exists)
  t = p.GetTxnResult();
  EXPECT_EQ(ABORTED, t->Status());
  delete t;

  p.NewTxnRequest(new Expect("1", "1"));  // Should abort (wrong value for key)
  t = p.GetTxnResult();
  EXPECT_EQ(ABORTED, t->Status());
  delete t;

  p.NewTxnRequest(new Expect("1", "2"));  // Should commit
  t = p.GetTxnResult();
  EXPECT_EQ(COMMITTED, t->Status());
  delete t;

  END;
}

TEST(PutMultipleTest) {
  TxnProcessor p(SERIAL);
  Txn* t;

  map<Key, Value> m;
  for (int i = 0; i < 1000; i++)
    m[IntToString(i)] = IntToString(i*i);

  p.NewTxnRequest(new PutMultiple(m));
  delete p.GetTxnResult();

  p.NewTxnRequest(new ExpectMultiple(m));
  t = p.GetTxnResult();
  EXPECT_EQ(COMMITTED, t->Status());
  delete t;

  END;
}

int main(int argc, char** argv) {
  NoopTest();
  PutTest();
  PutMultipleTest();
}