
#include "KV/Transaction/Util.h"

#include <string>

TEST(BasicTest) {
    // Create an included bound with some data
    Bound includedBound(BoundType::Included, std::optional<std::vector<uint8_t>>({0x01, 0x02, 0x03}));

    // Create an excluded bound without data
    Bound excludedBound(BoundType::Excluded, std::nullopt);

  END;
}


int main(int argc, char** argv) {
  BasicTest();
}