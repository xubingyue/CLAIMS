/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /CLAIMS/stmt_handler/stmt_handler.cpp
 *
 *  Created on: Sep 23, 2015
 *      Author: fzh
 *		   Email: fzhedu@gmail.com
 *
 * Description:
 *
 */

#include <glog/logging.h>
#include <iostream>
#include <string>
#include "../stmt_handler/stmt_handler.h"

#include "../stmt_handler/create_projection_exec.h"
#include "../stmt_handler/drop_table_exec.h"
#include "../stmt_handler/show_exec.h"
#include "../utility/Timer.h"
namespace claims {
namespace stmt_handler {

StmtHandler::StmtHandler(string sql_stmt)
    : sql_stmt_(sql_stmt), stmt_exec_(NULL), sql_parser_(NULL) {}
StmtHandler::StmtHandler(string sql_stmt, ExecutedResult* exec_result)
    : sql_stmt_(sql_stmt), stmt_exec_(NULL), sql_parser_(NULL) {}
StmtHandler::~StmtHandler() {
  if (NULL != stmt_exec_) {
    delete stmt_exec_;
    stmt_exec_ = NULL;
  }
  if (NULL != sql_parser_) {
    delete sql_parser_;
    sql_parser_ = NULL;
  }
}
RetCode StmtHandler::GenerateStmtExec(AstNode* stmt_ast) {
  switch (stmt_ast->ast_node_type_) {
    case AST_SELECT_STMT: {
      stmt_exec_ = new SelectExec(stmt_ast, sql_stmt_);
      break;
    }
    case AST_INSERT_STMT: {
      stmt_exec_ = new InsertExec(stmt_ast);
      break;
    }
    case AST_LOAD_TABLE: {
      stmt_exec_ = new LoadExec(stmt_ast);
      break;
    }
    case AST_STMT_LIST: {
      AstStmtList* stmt_list = reinterpret_cast<AstStmtList*>(stmt_ast);
      GenerateStmtExec(stmt_list->stmt_);
      if (NULL != stmt_list) {
        LOG(WARNING) << "only support one sql statement!";
      }
      break;
    }
    case AST_SHOW_STMT: {
      stmt_exec_ = new ShowExec(stmt_ast);
      break;
    }
    case AST_CREATE_TABLE_LIST:
    case AST_CREATE_TABLE_LIST_SEL:
    case AST_CREATE_TABLE_SEL: {
      stmt_exec_ = new CreateTableExec(stmt_ast);
      break;
    }
    case AST_CREATE_PROJECTION:
    case AST_CREATE_PROJECTION_NUM: {
      stmt_exec_ = new CreateProjectionExec(stmt_ast);
      break;
    }
    case AST_DROP_TABLE: {
      stmt_exec_ = new DropTableExec(stmt_ast);
      break;
    }
    case AST_DELETE_STMT: {
      stmt_exec_ = new DeleteStmtExec(stmt_ast);
      break;
    }
    default: {
      LOG(ERROR) << "unknow statement type!" << std::endl;
      return rUnknowStmtType;
    }
  }
  return rSuccess;
}
RetCode StmtHandler::Execute(ExecutedResult* exec_result) {
  GETCURRENTTIME(start_time);
  RetCode ret = rSuccess;
  sql_parser_ = new Parser(sql_stmt_);
  AstNode* raw_ast = sql_parser_->GetRawAST();
  if (NULL == raw_ast) {
    exec_result->error_info_ = "Parser Error";
    exec_result->status_ = false;
    exec_result->result_ = NULL;
    return rParserError;
  }
  raw_ast->Print();
  ret = GenerateStmtExec(raw_ast);
  if (rSuccess != ret) {
    return ret;
  }
  ret = stmt_exec_->Execute(exec_result);
  if (rSuccess != ret) {
    return ret;
  }
  double exec_time_ms = GetElapsedTime(start_time);
  if (NULL != exec_result->result_)
    exec_result->result_->query_time_ = exec_time_ms / 1000.0;
  cout << "execute time: " << exec_time_ms / 1000.0 << " sec" << endl;
  return rSuccess;
}

}  // namespace stmt_handler
}  // namespace claims
