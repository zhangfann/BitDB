#pragma once

#include <memory>
#include <string>
#include <vector>

#include "hyrise/storage/table.hpp"
#include "hyrise/types.hpp"
// #include "gtest/gtest.h"

namespace opossum {
  // TODO 这里的改法就是 全部从BaseTest提取出来
  // 引入gtest， 然后我们自定义的EXPECT_TRUE改名



// class BaseTest : public ::testing::Test {
//   using Matrix = std::vector<std::vector<AllTypeVariant>>;
using Matrix = std::vector<std::vector<AllTypeVariant>>;
//   // helper functions for _table_equal
//   static BaseTest::Matrix _table_to_matrix(const Table &t);
//   static void _print_matrix(const BaseTest::Matrix &m);
  static Matrix _table_to_matrix(const Table &t);
  static void _print_matrix(const Matrix &m);

//   // helper function for load_table
//   template <typename T>
//   static std::vector<T> _split(const std::string &str, char delimiter);

//   // compares two tables with regard to the schema and content
//   // but ignores the internal representation (chunk size, column type)
//   static ::testing::AssertionResult _table_equal(const Table &tleft, const Table &tright, bool order_sensitive = false);

//  protected:
//   // creates a opossum table based from a file
//   static std::shared_ptr<Table> load_table(const std::string &file_name, size_t chunk_size);
//   static void EXPECT_TABLE_EQ(const Table &tleft, const Table &tright, bool order_sensitive = false);
//   static void ASSERT_TABLE_EQ(const Table &tleft, const Table &tright, bool order_sensitive = false);

//   static void EXPECT_TABLE_EQ(std::shared_ptr<const Table> tleft, std::shared_ptr<const Table> tright,
//                               bool order_sensitive = false);
//   static void ASSERT_TABLE_EQ(std::shared_ptr<const Table> tleft, std::shared_ptr<const Table> tright,
//                               bool order_sensitive = false);

//  public:
//   virtual ~BaseTest();
// };

std::shared_ptr<Table> load_table(const std::string &file_name, size_t chunk_size);

template <typename T>
std::vector<T> _split(const std::string &str, char delimiter);

void EXPECT_TABLE_EQ(const Table &tleft, const Table &tright, bool order_sensitive = false);

  // compares two tables with regard to the schema and content
  // but ignores the internal representation (chunk size, column type)
  static ::testing::AssertionResult _table_equal(const Table &tleft, const Table &tright, bool order_sensitive = false);

  // helper functions for _table_equal


}  // namespace opossum
