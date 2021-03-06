/*
 * expr_node.h
 *  Created on: May 29, 2015 11:28:14 AM
 *      Author: fzh
 *       Email: fzhedu@gmail.com
 *   Copyright: Copyright (c) @ ECNU.DaSE
 * Description:
 */

#ifndef COMMON_EXPRESSION_EXPR_NODE_H_
#define COMMON_EXPRESSION_EXPR_NODE_H_
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include "../../common/Schema/Schema.h"
#include "./expr_type_cast.h"
#include "../../configure.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include "../../catalog/attribute.h"
#include "../../common/ids.h"
using claims::catalog::Attribute;
namespace claims {
namespace common {
#define MAX_ARGS 10
#define BASE_DATA_SIZE 64
typedef struct OperFuncInfoData {
  int args_num_;
  void* args_[MAX_ARGS];
  void* result_;
}* OperFuncInfo;
typedef void (*DataTypeOperFunc)(OperFuncInfo fcinfo);
typedef void (*AvgDivide)(void* sum_value, int64_t tuple_number, void* result);

enum ExprNodeType {
  t_qnode,
  t_qexpr_cal,
  t_qexpr_cmp,
  t_qexpr,
  t_qexpr_unary,
  t_qexpr_ternary,
  t_qcolcumns,
  t_qexpr_func,
  t_qexpr_case_when,
  t_qexpr_in,
  t_qexpr_date_add_sub,
};
enum OperType {
  oper_none,
  oper_add,
  oper_minus,
  oper_multiply,
  oper_divide,
  oper_mod,
  oper_and,
  oper_or,
  oper_xor,
  oper_not,
  oper_equal,
  oper_not_equal,
  oper_great,
  oper_great_equal,
  oper_less,
  oper_less_equal,
  oper_both_trim,
  oper_trailing_trim,
  oper_leading_trim,
  oper_like,
  oper_not_like,
  oper_upper,
  oper_substring,
  oper_negative,
  oper_case_when,
  oper_date_add_day,
  oper_date_add_week,
  oper_date_add_month,
  oper_date_add_year,  // quanter type have been changed to month*3 in
                       // transfromqual()
  oper_date_sub_day,
  oper_date_sub_week,
  oper_date_sub_month,
  oper_date_sub_year,

  oper_agg_sum,
  oper_agg_avg,
  oper_max,
  oper_min,
  oper_agg_count,
};
class ExprNode {
 public:
  ExprNode(ExprNodeType expr_node_type, data_type actual_type, string alias);
  ExprNode(ExprNodeType expr_node_type, data_type actual_type,
           data_type get_type, string alias);
  explicit ExprNode(ExprNode* expr);
  ExprNode() {}
  virtual ~ExprNode() {
    if (value_ != NULL) {
      free(value_);
      value_ = NULL;
    }
  }
  ExprNodeType expr_node_type_;
  data_type actual_type_, return_type_, get_type_;
  void* value_;     // store temporary calculation result
  int value_size_;  // the value size
  bool is_null_;
  TypeCastFunc type_cast_func_;
  std::string alias_;
  bool MoreExprEvaluate(vector<ExprNode*> condi, void* tuple, Schema* schema);
  virtual void* ExprEvaluate(void* tuple, Schema* schema) { return NULL; }
  virtual void InitExprAtLogicalPlan(
      data_type return_type, const std::map<std::string, int>& column_index,
      Schema* schema) {}
  virtual void InitExprAtPhysicalPlan() {}
  virtual ExprNode* ExprCopy() { return NULL; }
  bool IsEqualAttr(const Attribute& attr);
  Attribute ExprNodeToAttr(const int id, unsigned table_id = 1000000);

 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(const Archive& ar, const unsigned int version) {
    ar& expr_node_type_& actual_type_& get_type_& return_type_& value_size_&
        is_null_& alias_;
  }
};
}  // namespace common
}  // namespace claims
#endif  // COMMON_EXPRESSION_EXPR_NODE_H_
