//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// window_function_executor.h
//
// Identification: src/include/execution/executors/window_function_executor.h
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <vector>

#include "execution/executor_context.h"
#include "execution/executors/abstract_executor.h"
#include "execution/plans/window_plan.h"
#include "storage/table/tuple.h"
#include "type/value_factory.h"

namespace bustub {
// copy from aggregate
/** AggregateKey represents a key in an aggregation operation */
struct WindowKey {
  /** The group-by values */
  std::vector<Value> group_bys_;
  /**
   * Compares two aggregate keys for equality.
   * @param other the other aggregate key to be compared with
   * @return `true` if both aggregate keys have equivalent group-by expressions, `false` otherwise
   */
  auto operator==(const WindowKey &other) const -> bool {
    for (uint32_t i = 0; i < other.group_bys_.size(); i++) {
      if (group_bys_[i].CompareEquals(other.group_bys_[i]) != CmpBool::CmpTrue) {
        return false;
      }
    }
    return true;
  }
  WindowKey() { group_bys_.clear(); }
  explicit WindowKey(const Value &value) { group_bys_.push_back(value); }
  void AddKey(const Value &value) { group_bys_.push_back(value); }
  auto GetVector() -> std::vector<Value> * { return &group_bys_; }
};
// copy from aggregate
/** AggregateValue represents a value for each of the running aggregates */
struct WindowValue {
  /** The aggregate values */
  std::vector<Tuple> hashjoins_;
  WindowValue() { hashjoins_.clear(); }
  explicit WindowValue(const Tuple &tuple) { hashjoins_.push_back(tuple); }
  void AddTuple(const Tuple &tuple) { hashjoins_.push_back(tuple); }
  auto GetVector() -> std::vector<Tuple> * { return &hashjoins_; }
};
}  // namespace bustub

namespace std {

/** Implements std::hash on AggregateKey */
template <>
struct hash<bustub::WindowKey> {
  auto operator()(const bustub::WindowKey &agg_key) const -> std::size_t {
    size_t curr_hash = 0;
    for (const auto &key : agg_key.group_bys_) {
      if (!key.IsNull()) {
        curr_hash = bustub::HashUtil::CombineHashes(curr_hash, bustub::HashUtil::HashValue(&key));
      }
    }
    return curr_hash;
  }
};

}  // namespace std

namespace bustub {

// window需要一个hashtable，直接仿写aggregate的htable
class SimpleWindowHashTable {
 public:
  explicit SimpleWindowHashTable(const WindowFunctionType &window_function_type)
      : window_function_type_(window_function_type) {}

  /** @return The initial aggregate value for this aggregation executor */
  auto GenerateInitialWindowAggregateValue() -> Value {
    Value value;
    switch (window_function_type_) {
      case WindowFunctionType::CountStarAggregate:
        // Count start starts at zero.
        return ValueFactory::GetIntegerValue(0);
      case WindowFunctionType::CountAggregate:
      case WindowFunctionType::SumAggregate:
      case WindowFunctionType::MinAggregate:
      case WindowFunctionType::MaxAggregate:
      case WindowFunctionType::Rank:  // 注意windowfunctiontype多一个Rank类型
        // Others starts at null.
        return ValueFactory::GetNullValueByType(TypeId::INTEGER);
    }
    return {};
  }

  /**
   * TODO(Student)
   *
   * Combines the input into the aggregation result.
   * @param[out] result The output aggregate value
   * @param input The input value
   */
  auto CombineWindowAggregateValues(Value *result, const Value &input) -> Value {
    Value &old_val = *result;
    const Value &new_val = input;
    switch (window_function_type_) {
      //无论value是否为null，都要统计数目
      case WindowFunctionType::CountStarAggregate:
        old_val = old_val.Add(Value(TypeId::INTEGER, 1));
        break;
      case WindowFunctionType::CountAggregate:
        if (!new_val.IsNull()) {
          if (old_val.IsNull()) {
            old_val = ValueFactory::GetIntegerValue(0);
          }
          old_val = old_val.Add(Value(TypeId::INTEGER, 1));
        }
        break;
      case WindowFunctionType::SumAggregate:
        if (!new_val.IsNull()) {
          if (old_val.IsNull()) {
            old_val = new_val;
          } else {
            old_val = old_val.Add(new_val);
          }
        }
        break;
      case WindowFunctionType::MinAggregate:
        if (!new_val.IsNull()) {
          if (old_val.IsNull()) {
            old_val = new_val;
          } else {
            old_val = old_val.Min(new_val);
            // old_val = old_val.CompareLessThan(new_val) == CmpBool::CmpTrue ? old_val : new_val.Copy();
          }
        }
        break;
      case WindowFunctionType::MaxAggregate:
        if (!new_val.IsNull()) {
          if (old_val.IsNull()) {
            old_val = new_val;
          } else {
            old_val = old_val.Max(new_val);
            // old_val = old_val.CompareGreaterThan(new_val) == CmpBool::CmpTrue ? old_val : new_val.Copy();
          }
        }
        break;
      case WindowFunctionType::Rank:
        ++rank_count_;
        if (old_val.CompareEquals(new_val) != CmpBool::CmpTrue) {
          old_val = new_val;
          last_rank_count_ = rank_count_;
        }
        return ValueFactory::GetIntegerValue(last_rank_count_);
    }
    return old_val;
  }

  /**
   * Inserts a value into the hash table and then combines it with the current aggregation.
   * @param agg_key the key to be inserted
   * @param agg_val the value to be inserted
   */
  auto WindowInsertCombine(const WindowKey &key, const Value &val) -> Value {
    if (htable_.count(key) == 0) {
      htable_.insert({key, GenerateInitialWindowAggregateValue()});
    }
    return CombineWindowAggregateValues(&htable_[key], val);
  }

  //省略iter，直接用unordermap自带的搜索
  auto Find(const WindowKey &key) -> Value { return htable_.find(key)->second; }

  void Clear() { htable_.clear(); }

 private:
  const WindowFunctionType window_function_type_;
  std::unordered_map<WindowKey, Value> htable_;
  uint32_t rank_count_ = 0;
  uint32_t last_rank_count_ = 0;
};

/**
 * The WindowFunctionExecutor executor executes a window function for columns using window function.
 *
 * Window function is different from normal aggregation as it outputs one row for each inputing rows,
 * and can be combined with normal selected columns. The columns in WindowFunctionPlanNode contains both
 * normal selected columns and placeholder columns for window functions.
 *
 * For example, if we have a query like:
 *    SELECT 0.1, 0.2, SUM(0.3) OVER (PARTITION BY 0.2 ORDER BY 0.3), SUM(0.4) OVER (PARTITION BY 0.1 ORDER BY 0.2,0.3)
 *      FROM table;
 *
 * The WindowFunctionPlanNode contains following structure:
 *    columns: std::vector<AbstractExpressionRef>{0.1, 0.2, 0.-1(placeholder), 0.-1(placeholder)}
 *    window_functions_: {
 *      3: {
 *        partition_by: std::vector<AbstractExpressionRef>{0.2}
 *        order_by: std::vector<AbstractExpressionRef>{0.3}
 *        functions: std::vector<AbstractExpressionRef>{0.3}
 *        window_func_type: WindowFunctionType::SumAggregate
 *      }
 *      4: {
 *        partition_by: std::vector<AbstractExpressionRef>{0.1}
 *        order_by: std::vector<AbstractExpressionRef>{0.2,0.3}
 *        functions: std::vector<AbstractExpressionRef>{0.4}
 *        window_func_type: WindowFunctionType::SumAggregate
 *      }
 *    }
 *
 * Your executor should use child executor and exprs in columns to produce selected columns except for window
 * function columns, and use window_agg_indexes, partition_bys, order_bys, functionss and window_agg_types to
 * generate window function columns results. Directly use placeholders for window function columns in columns is
 * not allowed, as it contains invalid column id.
 *
 * Your WindowFunctionExecutor does not need to support specified window frames (eg: 1 preceding and 1 following).
 * You can assume that all window frames are UNBOUNDED FOLLOWING AND CURRENT ROW when there is ORDER BY clause, and
 * UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING when there is no ORDER BY clause.
 *
 */
class WindowFunctionExecutor : public AbstractExecutor {
 public:
  /**
   * Construct a new WindowFunctionExecutor instance.
   * @param exec_ctx The executor context
   * @param plan The window aggregation plan to be executed
   */
  WindowFunctionExecutor(ExecutorContext *exec_ctx, const WindowFunctionPlanNode *plan,
                         std::unique_ptr<AbstractExecutor> &&child_executor);

  /** Initialize the window aggregation */
  void Init() override;

  /**
   * Yield the next tuple from the window aggregation.
   * @param[out] tuple The next tuple produced by the window aggregation
   * @param[out] rid The next tuple RID produced by the window aggregation
   * @return `true` if a tuple was produced, `false` if there are no more tuples
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return The output schema for the window aggregation plan */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

 private:
  auto MakeWindowKey(const Tuple *tuple, const std::vector<AbstractExpressionRef> &partition) -> WindowKey;
  auto MakeWindowValue(const Tuple *tuple, const AbstractExpressionRef &expr) -> Value;
  /** The window aggregation plan node to be executed */
  const WindowFunctionPlanNode *plan_;

  /** The child executor from which tuples are obtained */
  std::unique_ptr<AbstractExecutor> child_executor_;

  std::vector<SimpleWindowHashTable> window_hash_table_;
  std::deque<std::vector<Value>> tuples_;
};
}  // namespace bustub
