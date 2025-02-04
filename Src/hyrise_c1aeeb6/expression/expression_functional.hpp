#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "abstract_expression.hpp"
#include "all_type_variant.hpp"
#include "arithmetic_expression.hpp"
#include "between_expression.hpp"
#include "binary_predicate_expression.hpp"
#include "case_expression.hpp"
#include "cast_expression.hpp"
#include "correlated_parameter_expression.hpp"
#include "exists_expression.hpp"
#include "extract_expression.hpp"
#include "function_expression.hpp"
#include "in_expression.hpp"
#include "interval_expression.hpp"
#include "is_null_expression.hpp"
#include "list_expression.hpp"
#include "logical_expression.hpp"
#include "lqp_column_expression.hpp"
#include "lqp_subquery_expression.hpp"
#include "placeholder_expression.hpp"
#include "pqp_column_expression.hpp"
#include "pqp_subquery_expression.hpp"
#include "../types.hpp"
#include "unary_minus_expression.hpp"
#include "value_expression.hpp"
#include "window_expression.hpp"
#include "window_function_expression.hpp"

/**
 * This file provides convenience methods to create (nested) Expression objects with little boilerplate.
 *
 * NOTE: functions suffixed with "_" (e.g. equals_()) to alert the unsuspecting reader to the fact this is something
 *       different than the equality check he might expect when reading "equals" *
 *
 * In Hyrise we say...
 *      case_(equals_(a, 123),
 *            b,
 *            case_(equals_(a, 1234),
 *                  a,
 *                  null_()))
 *
 * ...and it actually means:
 *     const auto value_123 = std::make_shared<ValueExpression>(123);
 *     const auto value_1234 = std::make_shared<ValueExpression>(1234);
 *     const auto a_eq_123 = std::make_shared<BinaryPredicateExpression>(PredicateCondition::Equals, int_float_a_expression, value_123);
 *     const auto a_eq_1234 = std::make_shared<BinaryPredicateExpression>(PredicateCondition::Equals, int_float_a_expression, value_1234);
 *     const auto null_value = std::make_shared<ValueExpression>(NullValue{});
 *     const auto case_a_eq_1234 = std::make_shared<CaseExpression>(a_eq_1234, int_float_a_expression, null_value);
 *     const auto case_a_eq_123 = std::make_shared<CaseExpression>(a_eq_123, int_float_b_expression, case_a_eq_1234);
 *
 * ...and I think that's beautiful.
 */

namespace hyrise {

class AbstractOperator;

/**
 * expression_"functional", since it supplies a functional-programming like interface to build nested expressions
 */
namespace expression_functional {

/**
 * @defgroup Turn expression-like things (Values, Expressions themselves) into expressions
 *
 * Mostly used internally in this file
 *
 * @{
 */
std::shared_ptr<AbstractExpression> to_expression(const std::shared_ptr<AbstractExpression>& expression);
std::shared_ptr<ValueExpression> to_expression(const AllTypeVariant& value);
/** @} */

std::shared_ptr<ValueExpression> value_(const AllTypeVariant& value);
std::shared_ptr<ValueExpression> null_();

namespace detail {

/**
 * @defgroup Static objects that create Expressions that have an enum member (e.g. PredicateCondition::Equals). Having
 * these eliminates the need to specify a function for each Expression-Enum-member combination.
 * @{
 */

// Helper for unary expressions (one argument).
//   Example: is_null_(argument) --> IsNullExpression(IsNull, argument)
template <auto t, typename E>
struct unary final {
  template <typename A>
  std::shared_ptr<E> operator()(const A& value) const {
    return std::make_shared<E>(t, to_expression(value));
  }
};

// Helper for expressions that have two arguments, but the first one is always nullptr.
//   Example: rank_(window) --> WindowFunctionExpression(WindowFunction::Rank, nullptr, window)
template <auto t, typename E>
struct pseudo_unary final {
  template <typename A>
  std::shared_ptr<E> operator()(const A& expression) const {
    return std::make_shared<E>(t, nullptr, to_expression(expression));
  }
};

// Helper for expressions that have two arguments, where the second one can be nullptr.
//   Example: sum_(column_a) --> WindowFunctionExpression(WindowFunction::Sum, column_a, nullptr)
template <auto t, typename E>
struct binary_defaulted final {
  template <typename A>
  std::shared_ptr<E> operator()(const A& lhs, const std::shared_ptr<AbstractExpression>& rhs = nullptr) const {
    return std::make_shared<E>(t, to_expression(lhs), rhs);
  }
};

// Helper for binary expressions (two arguments).
//   Example: or_(argument_1, argmuent_2) --> LogicalExpression(LogicalOperator::Or, argument_1, argmuent_2)
template <auto t, typename E>
struct binary final {
  template <typename A, typename B>
  std::shared_ptr<E> operator()(const A& lhs, const B& rhs) const {
    return std::make_shared<E>(t, to_expression(lhs), to_expression(rhs));
  }
};

// Helper for ternary expressions (three arguments).
//   Example: between_inclusive_(column_a, value_1, value_2) --> BetweenExpression(PredicateCondition::BetweenInclusive, column_a, value_1, value_2)  // NOLINT(whitespace/line_length)
template <auto t, typename E>
struct ternary final {
  template <typename A, typename B, typename C>
  std::shared_ptr<E> operator()(const A& left, const B& middle, const C& right) const {
    return std::make_shared<E>(t, to_expression(left), to_expression(middle), to_expression(right));
  }
};

/** @} */

}  // namespace detail

inline const detail::unary<PredicateCondition::IsNull, IsNullExpression> is_null_;
inline const detail::unary<PredicateCondition::IsNotNull, IsNullExpression> is_not_null_;

inline const detail::pseudo_unary<WindowFunction::CumeDist, WindowFunctionExpression> cume_dist_;
inline const detail::pseudo_unary<WindowFunction::DenseRank, WindowFunctionExpression> dense_rank_;
inline const detail::pseudo_unary<WindowFunction::PercentRank, WindowFunctionExpression> percent_rank_;
inline const detail::pseudo_unary<WindowFunction::Rank, WindowFunctionExpression> rank_;
inline const detail::pseudo_unary<WindowFunction::RowNumber, WindowFunctionExpression> row_number_;

inline const detail::binary_defaulted<WindowFunction::Sum, WindowFunctionExpression> sum_;
inline const detail::binary_defaulted<WindowFunction::Max, WindowFunctionExpression> max_;
inline const detail::binary_defaulted<WindowFunction::Min, WindowFunctionExpression> min_;
inline const detail::binary_defaulted<WindowFunction::Avg, WindowFunctionExpression> avg_;
inline const detail::binary_defaulted<WindowFunction::Count, WindowFunctionExpression> count_;
inline const detail::binary_defaulted<WindowFunction::CountDistinct, WindowFunctionExpression> count_distinct_;
inline const detail::binary_defaulted<WindowFunction::StandardDeviationSample, WindowFunctionExpression>
    standard_deviation_sample_;
inline const detail::binary_defaulted<WindowFunction::Any, WindowFunctionExpression> any_;

inline const detail::binary<ArithmeticOperator::Division, ArithmeticExpression> div_;
inline const detail::binary<ArithmeticOperator::Multiplication, ArithmeticExpression> mul_;
inline const detail::binary<ArithmeticOperator::Addition, ArithmeticExpression> add_;
inline const detail::binary<ArithmeticOperator::Subtraction, ArithmeticExpression> sub_;
inline const detail::binary<ArithmeticOperator::Modulo, ArithmeticExpression> mod_;
inline const detail::binary<PredicateCondition::Like, BinaryPredicateExpression> like_;
inline const detail::binary<PredicateCondition::NotLike, BinaryPredicateExpression> not_like_;
inline const detail::binary<PredicateCondition::Equals, BinaryPredicateExpression> equals_;
inline const detail::binary<PredicateCondition::NotEquals, BinaryPredicateExpression> not_equals_;
inline const detail::binary<PredicateCondition::LessThan, BinaryPredicateExpression> less_than_;
inline const detail::binary<PredicateCondition::LessThanEquals, BinaryPredicateExpression> less_than_equals_;
inline const detail::binary<PredicateCondition::GreaterThanEquals, BinaryPredicateExpression> greater_than_equals_;
inline const detail::binary<PredicateCondition::GreaterThan, BinaryPredicateExpression> greater_than_;
inline const detail::binary<LogicalOperator::And, LogicalExpression> and_;
inline const detail::binary<LogicalOperator::Or, LogicalExpression> or_;

inline const detail::ternary<PredicateCondition::BetweenInclusive, BetweenExpression> between_inclusive_;
inline const detail::ternary<PredicateCondition::BetweenLowerExclusive, BetweenExpression> between_lower_exclusive_;
inline const detail::ternary<PredicateCondition::BetweenUpperExclusive, BetweenExpression> between_upper_exclusive_;
inline const detail::ternary<PredicateCondition::BetweenExclusive, BetweenExpression> between_exclusive_;

template <typename... Args>
std::shared_ptr<LQPSubqueryExpression> lqp_subquery_(const std::shared_ptr<AbstractLQPNode>& lqp,
                                                     Args&&... parameter_id_expression_pairs) {
  if constexpr (sizeof...(Args) > 0) {
    // Correlated subquery
    return std::make_shared<LQPSubqueryExpression>(
        lqp, std::vector<ParameterID>{{std::forward<ParameterID>(parameter_id_expression_pairs.first)...}},
        std::vector<std::shared_ptr<AbstractExpression>>{{to_expression(
            std::forward<std::shared_ptr<AbstractExpression>>(parameter_id_expression_pairs.second))...}});
  } else {
    // Not correlated
    return std::make_shared<LQPSubqueryExpression>(lqp, std::vector<ParameterID>{},
                                                   std::vector<std::shared_ptr<AbstractExpression>>{});
  }
}

template <typename... Args>
std::shared_ptr<PQPSubqueryExpression> pqp_subquery_(const std::shared_ptr<AbstractOperator>& pqp,
                                                     const DataType data_type, const bool nullable,
                                                     Args&&... parameter_id_column_id_pairs) {
  if constexpr (sizeof...(Args) > 0) {
    // Correlated subquery
    return std::make_shared<PQPSubqueryExpression>(
        pqp, data_type, nullable,
        std::vector<std::pair<ParameterID, ColumnID>>{
            {std::make_pair(parameter_id_column_id_pairs.first, parameter_id_column_id_pairs.second)...}});
  } else {
    // Not correlated
    return std::make_shared<PQPSubqueryExpression>(pqp, data_type, nullable);
  }
}

template <typename... Args>
std::vector<std::shared_ptr<AbstractExpression>> expression_vector(Args&&... args) {
  return std::vector<std::shared_ptr<AbstractExpression>>({to_expression(args)...});
}

template <typename A, typename B, typename C>
std::shared_ptr<CaseExpression> case_(const A& a, const B& b, const C& c) {
  return std::make_shared<CaseExpression>(to_expression(a), to_expression(b), to_expression(c));
}

template <typename String, typename Start, typename Length>
std::shared_ptr<FunctionExpression> substr_(const String& string, const Start& start, const Length& length) {
  return std::make_shared<FunctionExpression>(
      FunctionType::Substring, expression_vector(to_expression(string), to_expression(start), to_expression(length)));
}

template <typename... Args>
std::shared_ptr<FunctionExpression> concat_(const Args... args) {
  return std::make_shared<FunctionExpression>(FunctionType::Concatenate, expression_vector(to_expression(args)...));
}

template <typename V>
std::shared_ptr<FunctionExpression> abs_(const V& v) {
  return std::make_shared<FunctionExpression>(FunctionType::Absolute, expression_vector(to_expression(v)));
}

template <typename... Args>
std::shared_ptr<ListExpression> list_(Args&&... args) {
  return std::make_shared<ListExpression>(expression_vector(std::forward<Args>(args)...));
}

template <typename V, typename S>
std::shared_ptr<InExpression> in_(const V& v, const S& s) {
  return std::make_shared<InExpression>(PredicateCondition::In, to_expression(v), to_expression(s));
}

template <typename V, typename S>
std::shared_ptr<InExpression> not_in_(const V& v, const S& s) {
  return std::make_shared<InExpression>(PredicateCondition::NotIn, to_expression(v), to_expression(s));
}

std::shared_ptr<ExistsExpression> exists_(const std::shared_ptr<AbstractExpression>& subquery_expression);
std::shared_ptr<ExistsExpression> not_exists_(const std::shared_ptr<AbstractExpression>& subquery_expression);

template <typename F>
std::shared_ptr<ExtractExpression> extract_(const DatetimeComponent datetime_component, const F& from) {
  return std::make_shared<ExtractExpression>(datetime_component, to_expression(from));
}

std::shared_ptr<PlaceholderExpression> placeholder_(const ParameterID parameter_id);
std::shared_ptr<LQPColumnExpression> lqp_column_(const std::shared_ptr<const AbstractLQPNode>& original_node,
                                                 const ColumnID original_column_id);
std::shared_ptr<PQPColumnExpression> pqp_column_(const ColumnID column_id, const DataType data_type,
                                                 const bool nullable, const std::string& column_name);

template <typename ReferencedExpression>
std::shared_ptr<CorrelatedParameterExpression> correlated_parameter_(const ParameterID parameter_id,
                                                                     const ReferencedExpression& referenced) {
  return std::make_shared<CorrelatedParameterExpression>(parameter_id, *to_expression(referenced));
}

std::shared_ptr<WindowFunctionExpression> count_star_(const std::shared_ptr<AbstractLQPNode>& lqp_node);

template <typename Argument>
std::shared_ptr<UnaryMinusExpression> unary_minus_(const Argument& argument) {
  return std::make_shared<UnaryMinusExpression>(to_expression(argument));
}

template <typename Argument>
std::shared_ptr<CastExpression> cast_(const Argument& argument, const DataType data_type) {
  return std::make_shared<CastExpression>(to_expression(argument), data_type);
}

std::shared_ptr<IntervalExpression> interval_(const int64_t duration, const DatetimeComponent unit);

std::shared_ptr<WindowExpression> window_(std::vector<std::shared_ptr<AbstractExpression>>&& partition_by_expressions,
                                          std::vector<std::shared_ptr<AbstractExpression>>&& order_by_expressions,
                                          std::vector<SortMode>&& sort_modes, FrameDescription frame_description);

}  // namespace expression_functional

}  // namespace hyrise
