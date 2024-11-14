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

EncodingType _encoding_type;
std::shared_ptr<TableWrapper> _int_int_compressed;
std::shared_ptr<TableWrapper> _int_int_partly_compressed;
  void SetUp()  {
    _encoding_type = GetParam();

    auto int_int_7 = load_table("resources/test_data/tbl/int_int_shuffled.tbl", ChunkOffset{7});
    auto int_int_5 = load_table("resources/test_data/tbl/int_int_shuffled_2.tbl", ChunkOffset{5});

    ChunkEncoder::encode_chunks(int_int_7, {ChunkID{0}, ChunkID{1}}, SegmentEncodingSpec{_encoding_type});
    // partly compressed table
    ChunkEncoder::encode_chunks(int_int_5, {ChunkID{0}, ChunkID{1}}, SegmentEncodingSpec{_encoding_type});

    _int_int_compressed = std::make_shared<TableWrapper>(std::move(int_int_7));
    _int_int_compressed->never_clear_output();
    _int_int_compressed->execute();
    _int_int_partly_compressed = std::make_shared<TableWrapper>(std::move(int_int_5));
    _int_int_partly_compressed->never_clear_output();
    _int_int_partly_compressed->execute();
  }

std::shared_ptr<TableWrapper> load_and_encode_table(
      const std::string& path, const ChunkOffset chunk_size = ChunkOffset{2},
      const std::optional<std::vector<SortColumnDefinition>> sorted_by = std::nullopt) {
    const auto table = load_table(path, chunk_size);

    auto chunk_encoding_spec = ChunkEncodingSpec{};
    for (const auto& column_definition : table->column_definitions()) {
      if (encoding_supports_data_type(_encoding_type, column_definition.data_type)) {
        chunk_encoding_spec.emplace_back(_encoding_type);
      } else {
        chunk_encoding_spec.emplace_back(EncodingType::Unencoded);
      }
    }

    ChunkEncoder::encode_all_chunks(table, chunk_encoding_spec);

    if (sorted_by) {
      const auto chunk_count = table->chunk_count();
      for (auto chunk_id = ChunkID{0}; chunk_id < chunk_count; ++chunk_id) {
        const auto chunk = table->get_chunk(chunk_id);
        if (!chunk) {
          continue;
        }

        chunk->set_individually_sorted_by(*sorted_by);
      }
    }

    auto table_wrapper = std::make_shared<TableWrapper>(table);
    table_wrapper->never_clear_output();
    table_wrapper->execute();

    return table_wrapper;
  }

  std::shared_ptr<TableWrapper> get_int_float_op() {
    return load_and_encode_table("resources/test_data/tbl/int_float.tbl");
  }

  std::shared_ptr<TableWrapper> get_int_sorted_op() {
    return load_and_encode_table(
        "resources/test_data/tbl/int_sorted.tbl", ChunkOffset{4},
        std::make_optional(std::vector<SortColumnDefinition>{SortColumnDefinition(ColumnID(0), SortMode::Ascending)}));
  }

  std::shared_ptr<TableWrapper> get_int_only_null_op() {
    return load_and_encode_table(
        "resources/test_data/tbl/int_only_null.tbl", ChunkOffset{4},
        std::make_optional(std::vector<SortColumnDefinition>{SortColumnDefinition(ColumnID(0), SortMode::Ascending)}));
  }

  std::shared_ptr<TableWrapper> get_int_string_op() {
    return load_and_encode_table("resources/test_data/tbl/int_string.tbl");
  }

  std::shared_ptr<TableWrapper> get_int_float_with_null_op(const ChunkOffset chunk_size = ChunkOffset{2}) {
    return load_and_encode_table("resources/test_data/tbl/int_float_with_null.tbl", chunk_size);
  }

  std::shared_ptr<TableWrapper> get_table_op_filtered() {
    TableColumnDefinitions table_column_definitions;
    table_column_definitions.emplace_back("a", DataType::Int, false);
    table_column_definitions.emplace_back("b", DataType::Int, false);

    const auto table = std::make_shared<Table>(table_column_definitions, TableType::References);

    const auto test_table_part_compressed = _int_int_partly_compressed->get_output();

    auto pos_list = std::make_shared<RowIDPosList>();
    pos_list->emplace_back(ChunkID{2}, ChunkOffset{0});
    pos_list->emplace_back(ChunkID{1}, ChunkOffset{1});
    pos_list->emplace_back(ChunkID{1}, ChunkOffset{3});
    pos_list->emplace_back(ChunkID{0}, ChunkOffset{2});
    pos_list->emplace_back(ChunkID{2}, ChunkOffset{2});
    pos_list->emplace_back(ChunkID{0}, ChunkOffset{0});
    pos_list->emplace_back(ChunkID{0}, ChunkOffset{4});

    auto segment_a = std::make_shared<ReferenceSegment>(test_table_part_compressed, ColumnID{0}, pos_list);
    auto segment_b = std::make_shared<ReferenceSegment>(test_table_part_compressed, ColumnID{1}, pos_list);

    const auto segments = Segments({segment_a, segment_b});

    table->append_chunk(segments);
    auto table_wrapper = std::make_shared<TableWrapper>(std::move(table));
    table_wrapper->never_clear_output();
    table_wrapper->execute();

    return table_wrapper;
  }

  std::shared_ptr<TableWrapper> get_table_op_with_n_dict_entries(const int num_entries) {
    // Set up dictionary encoded table with a dictionary consisting of num_entries entries.
    auto table_column_definitions = TableColumnDefinitions{};
    table_column_definitions.emplace_back("a", DataType::Int, false);

    const auto table = std::make_shared<Table>(table_column_definitions, TableType::Data, ChunkOffset{100'000});

    for (int i = 0; i <= num_entries; i++) {
      table->append({i});
    }

    table->get_chunk(static_cast<ChunkID>(ChunkID{0}))->set_immutable();

    ChunkEncoder::encode_chunks(table, {ChunkID{0}}, SegmentEncodingSpec{_encoding_type});

    auto table_wrapper = std::make_shared<TableWrapper>(std::move(table));
    table_wrapper->never_clear_output();
    table_wrapper->execute();

    return table_wrapper;
  }

  std::shared_ptr<const Table> create_referencing_table_w_null_row_id(const bool references_dict_segment) {
    const auto table = load_table("resources/test_data/tbl/int_int_w_null_8_rows.tbl", ChunkOffset{4});

    if (references_dict_segment) {
      ChunkEncoder::encode_all_chunks(table, SegmentEncodingSpec{_encoding_type});
    }

    auto pos_list_a = std::make_shared<RowIDPosList>(
        RowIDPosList{RowID{ChunkID{0}, ChunkOffset{1}}, RowID{ChunkID{1}, ChunkOffset{0}},
                     RowID{ChunkID{0}, ChunkOffset{2}}, RowID{ChunkID{0}, ChunkOffset{3}}});
    auto ref_segment_a = std::make_shared<ReferenceSegment>(table, ColumnID{0}, pos_list_a);

    auto pos_list_b = std::make_shared<RowIDPosList>(RowIDPosList{NULL_ROW_ID, RowID{ChunkID{0}, ChunkOffset{0}},
                                                                  RowID{ChunkID{1}, ChunkOffset{2}},
                                                                  RowID{ChunkID{0}, ChunkOffset{1}}});
    auto ref_segment_b = std::make_shared<ReferenceSegment>(table, ColumnID{1}, pos_list_b);

    TableColumnDefinitions column_definitions;
    column_definitions.emplace_back("a", DataType::Int, true);
    column_definitions.emplace_back("b", DataType::Int, true);
    auto ref_table = std::make_shared<Table>(column_definitions, TableType::References);

    const auto segments = Segments({ref_segment_a, ref_segment_b});

    ref_table->append_chunk(segments);

    return ref_table;
  }

  void scan_for_null_values(const std::shared_ptr<AbstractOperator> in,
                            const std::map<PredicateCondition, std::vector<AllTypeVariant>>& tests) {
    for (const auto& test : tests) {
      const auto predicate_condition = test.first;
      const auto& expected = test.second;  // {12, 123}

      const auto column = get_column_expression(in, ColumnID{1});

      auto scan = create_table_scan(in, ColumnID{1}, predicate_condition, NULL_VALUE);
      scan->execute();

      ASSERT_COLUMN_EQ(scan->get_output(), ColumnID{0}, expected);
    }
  }

  void ASSERT_COLUMN_EQ(std::shared_ptr<const Table> table, const ColumnID& column_id,
                        std::vector<AllTypeVariant> expected) {
    const auto chunk_count = table->chunk_count();
    for (auto chunk_id = ChunkID{0}; chunk_id < chunk_count; ++chunk_id) {
      const auto chunk = table->get_chunk(chunk_id);

      const auto chunk_size = chunk->size();
      for (auto chunk_offset = ChunkOffset{0}; chunk_offset < chunk_size; ++chunk_offset) {
        const auto& segment = *chunk->get_segment(column_id);

        const auto found_value = segment[chunk_offset];
        const auto comparator = [found_value](const AllTypeVariant expected_value) {
          // returns equivalency, not equality to simulate std::multiset.
          // multiset cannot be used because it triggers a compiler / lib bug when built in CI
          return !(found_value < expected_value) && !(expected_value < found_value);
        };

        auto search = std::find_if(expected.begin(), expected.end(), comparator);

        ASSERT_TRUE(search != expected.end()) << found_value << " not found";
        expected.erase(search);
      }
    }

    ASSERT_EQ(expected.size(), 0u);
  }

  void scan_and_check_sorted_by(const std::shared_ptr<TableWrapper> table_wrapper) const {
    const auto scan_sorted = create_table_scan(table_wrapper, ColumnID{0}, PredicateCondition::GreaterThanEquals, 1234);
    scan_sorted->execute();
    const auto& result_table_sorted = scan_sorted->get_output();

    const auto chunk_count = result_table_sorted->chunk_count();
    for (auto chunk_id = ChunkID{0}; chunk_id < chunk_count; ++chunk_id) {
      const auto& actual_sorted_by = result_table_sorted->get_chunk(chunk_id)->individually_sorted_by();
      ASSERT_TRUE(!actual_sorted_by.empty());
      const auto expected_sorted_by =
          std::vector<SortColumnDefinition>{SortColumnDefinition(ColumnID{0}, SortMode::Ascending)};
      EXPECT_EQ(actual_sorted_by, expected_sorted_by);
    }
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