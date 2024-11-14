#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "hyrise/utils/Testing.h"
#include "hyrise/utils/base_test.hpp"
// #include "gtest/gtest.h"

#include "hyrise/operators/abstract_read_only_operator.hpp"
// #include "hyrise/operators/get_table.hpp"
#include "hyrise/operators/print.hpp"
#include "hyrise/operators/table_scan.hpp"
// #include "hyrise/storage/storage_manager.hpp"
#include "hyrise/storage/table.hpp"
#include "hyrise/types.hpp"

// #include "hyrise/base_test.hpp"
#include "hyrise/expression/expression_functional.hpp"
// #include "hyrise/operators/abstract_read_only_operator.hpp"
#include "hyrise/operators/limit.hpp"
// #include "hyrise/operators/print.hpp"
#include "hyrise/operators/projection.hpp"
// #include "hyrise/operators/table_scan.hpp"
#include "hyrise/operators/table_scan/column_between_table_scan_impl.hpp"
#include "hyrise/operators/table_scan/column_is_null_table_scan_impl.hpp"
#include "hyrise/operators/table_scan/column_like_table_scan_impl.hpp"
#include "hyrise/operators/table_scan/column_vs_column_table_scan_impl.hpp"
#include "hyrise/operators/table_scan/column_vs_value_table_scan_impl.hpp"
#include "hyrise/operators/table_scan/expression_evaluator_table_scan_impl.hpp"
#include "hyrise/operators/table_wrapper.hpp"
#include "hyrise/storage/chunk_encoder.hpp"
#include "hyrise/storage/encoding_type.hpp"
#include "hyrise/storage/reference_segment.hpp"
#include "hyrise/storage/table.hpp"
#include "hyrise/utils/assert.hpp"

using namespace hyrise;
using namespace expression_functional;
// {
// class OperatorsTableScanTest : public BaseTest {
//  protected:
//   void SetUp() override {
//     std::shared_ptr<Table> test_table = load_table("src/test/tables/int_float.tbl", 2);
//     StorageManager::get().add_table("table_a", std::move(test_table));
//     _gt = std::make_shared<GetTable>("table_a");

//     std::shared_ptr<Table> test_table_dict = std::make_shared<Table>(5);
//     test_table_dict->add_column("a", "int");
//     test_table_dict->add_column("b", "int");
//     for (int i = 0; i <= 24; i += 2) test_table_dict->append({i, 100 + i});
//     test_table_dict->compress_chunk(0);
//     test_table_dict->compress_chunk(1);
//     StorageManager::get().add_table("table_dict", std::move(test_table_dict));

//     _gt_dict = std::make_shared<GetTable>("table_dict");

//     _gt->execute();
//     _gt_dict->execute();
//   }

//   std::shared_ptr<GetTable> _gt, _gt_dict;
// };
// 初始化
std::shared_ptr<GetTable> _gt, _gt_dict;
void SetUp()
{

    std::shared_ptr<Table> test_table = load_table("../Test/hyrise/tables/int_float.tbl", 2);
    StorageManager::get().add_table("table_a", std::move(test_table));
    _gt = std::make_shared<GetTable>("table_a");

    std::shared_ptr<Table> test_table_dict = std::make_shared<Table>(5);
    test_table_dict->add_column("a", "int");
    test_table_dict->add_column("b", "int");
    for (int i = 0; i <= 24; i += 2)
        test_table_dict->append({i, 100 + i});
    test_table_dict->compress_chunk(0);
    test_table_dict->compress_chunk(1);
    StorageManager::get().add_table("table_dict", std::move(test_table_dict));

    _gt_dict = std::make_shared<GetTable>("table_dict");

    _gt->execute();
    _gt_dict->execute();
}

MY_TEST(OperatorsTableScanTest_DoubleScan)
{
    SetUp();
    // 测试内容
    std::shared_ptr<Table> expected_result = load_table("../Test/hyrise/tables/int_float_filtered.tbl", 2);

    auto scan_1 = std::make_shared<TableScan>(_gt, "a", ">=", 1234);
    scan_1->execute();

    auto scan_2 = std::make_shared<TableScan>(scan_1, "b", "<", 457.9);
    scan_2->execute();

    EXPECT_TABLE_EQ(scan_2->get_output(), expected_result);
    // auto result = scan_2->get_output();
    // EXPECT_TRUE(scan_2->get_output() == expected_result);
    TESTING_END;
}


int main(int argc, char **argv)
{
    OperatorsTableScanTest_DoubleScan();

    // PutTest();
    //   PutMultipleTest();
}

// } // namespace opossum