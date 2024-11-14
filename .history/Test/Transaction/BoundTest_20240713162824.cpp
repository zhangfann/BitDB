
#include "KV/Transaction/Util.h"

#include <string>
#include <optional>


#include <gtest/gtest.h>

TEST(BoundTest, BasicTest) {

  uint8_t value1 = 10;
  uint8_t value2 = 20;
  std::vector<uint8_t> values = {value1, value2};

  Bound includedBound(Bound::Type::Included, values);
  Bound excludedBound(Bound::Type::Excluded, values);
  Bound unboundedBound(Bound::Type::Unbounded, values);

    // Check if the type is correctly set
  ASSERT_EQ(includedBound.get_type(), Bound::Type::Included);
  ASSERT_EQ(excludedBound.get_type(), Bound::Type::Excluded);
  ASSERT_EQ(unboundedBound.get_type(), Bound::Type::Unbounded);

  // Check if the value is correctly set
  ASSERT_EQ(includedBound.get_value(), std::vector<uint8_t>({10, 20}));
  ASSERT_EQ(excludedBound.get_value(), std::vector<uint8_t>({10, 20}));
  ASSERT_EQ(unboundedBound.get_value(), std::vector<uint8_t>({10, 20}));

}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}