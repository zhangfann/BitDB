
#include "KV/Transaction/Util.h"

#include <string>
#include <optional>


#include <gtest/gtest.h>

class BoundTest: public::testing::Test{
  protected:
  
}
TEST(BasicTest) {

  uint8_t value1 = 10;
  uint8_t value2 = 20;
  std::vector<uint8_t> values = {value1, value2};

  Bound includedBound(Bound::Type::Included, values);
  Bound excludedBound(Bound::Type::Excluded, values);
  Bound unboundedBound(Bound::Type::Unbounded, values);

    // Check if the type is correctly set
  EXPECT_EQ(includedBound.get_type(), Bound::Type::Included);
  EXPECT_EQ(excludedBound.get_type(), Bound::Type::Excluded);
  EXPECT_EQ(unboundedBound.get_type(), Bound::Type::Unbounded);

  // Check if the value is correctly set
  EXPECT_EQ(includedBound.get_value(), std::vector<uint8_t>({10, 20}));
  EXPECT_EQ(excludedBound.get_value(), std::vector<uint8_t>({10, 20}));
  EXPECT_EQ(unboundedBound.get_value(), std::vector<uint8_t>({10, 20}));

  END;
}


int main(int argc, char** argv) {
  BasicTest();
}