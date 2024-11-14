
#include "KV/Transaction/Util.h"

#include <string>
#include <optional>


#include "KV/Transaction1/Utils/Testing.h"

TEST(BasicTest) {

  uint8_t value1 = 10;
  uint8_t value2 = 20;
  std::vector<uint8_t> values = {value1, value2};

  Bound includedBound(Bound::Type::Included, values);
  Bound excludedBound(Bound::Type::Excluded, values);
  Bound unboundedBound(Bound::Type::Unbounded, values);

  

  END;
}


int main(int argc, char** argv) {
  BasicTest();
}