#pragma once

#include <memory>
#include <string>
#include <vector>

#include "KV/Transaction/Storage/table.hpp"
#include "KV/Transaction/types.hpp"
#include "gtest/gtest.h"

namespace opossum {

using Matrix = std::vector<std::vector<AllTypeVariant>>;

class BaseTest : public ::testing::Test {
  using Matrix = std::vector<std::vector<AllTypeVariant>>;

  // helper functions for _table_equal
  static BaseTest::Matrix _table_to_matrix(const Table &t);
  static void _print_matrix(const BaseTest::Matrix &m);

  // helper function for load_table
  template <typename T>
  static std::vector<T> _split(const std::string &str, char delimiter);

  // compares two tables with regard to the schema and content
  // but ignores the internal representation (chunk size, column type)
  static ::testing::AssertionResult _table_equal(const Table &tleft, const Table &tright, bool order_sensitive = false);

 protected:
  // creates a opossum table based from a file
  static std::shared_ptr<Table> load_table(const std::string &file_name, size_t chunk_size);
  static void EXPECT_TABLE_EQ(const Table &tleft, const Table &tright, bool order_sensitive = false);
  static void ASSERT_TABLE_EQ(const Table &tleft, const Table &tright, bool order_sensitive = false);

  static void EXPECT_TABLE_EQ(std::shared_ptr<const Table> tleft, std::shared_ptr<const Table> tright,
                              bool order_sensitive = false);
  static void ASSERT_TABLE_EQ(std::shared_ptr<const Table> tleft, std::shared_ptr<const Table> tright,
                              bool order_sensitive = false);

 public:
  virtual ~BaseTest();
};

}  // namespace opossum
