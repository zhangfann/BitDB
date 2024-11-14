#pragma once

#include "Catalog.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

using namespace std;

class ColumnBinding{
    public:
    int column_index_;
    int table_index_;
};

// Expression **************************************************************************************
enum LogicalType{
    INTEGER=0,
    STRING,
};

class Expression{
    public:
    LogicalType return_type_;

    virtual ~Expression() = default;
    virtual string toString()=0;
};
enum ExpressionClass{
    Constant=0,
    COLUMN_REF,
};
class ParsedExpression{
    public:
    ExpressionClass expression_class_;

    ParsedExpression(){}
    ParsedExpression(ExpressionClass exp_class):expression_class_(exp_class){}
    virtual ~ParsedExpression() = default;
    virtual string toString()=0;
};


class BoundExpression: public ParsedExpression{
    public:
    std::unique_ptr<Expression> expr_;

    BoundExpression(unique_ptr<Expression> expr): expr_(std::move(expr)){
    }

    string toString();
};

class Value{
    public:
    LogicalType type_;
    int int_value_;
    string str_value_;

    Value(){}
    Value(int v): int_value_(v), type_(LogicalType::INTEGER){}
    Value(string v): str_value_(v), type_(LogicalType::STRING){}

    string toString();
};

class ColumnRefExpression: public ParsedExpression{
    public:
    // TODO哪里取得
    vector<string> column_names_;
    Value value_;

    ColumnRefExpression(){}
    ColumnRefExpression(Value value): ParsedExpression(ExpressionClass::COLUMN_REF), value_(value){
        //TODO 目前这块也不知道怎么处理的
        column_names_.push_back(value_.str_value_);
    }
    const string& getTableName();



    string toString() override;

    string getColumnName();

};

class ConstantExpression: public ParsedExpression{
    public:
    Value value_;

    ConstantExpression(){}
    ConstantExpression(Value value): ParsedExpression(ExpressionClass::Constant), value_(value) {}

       string toString() override;
};

class BoundConstantExpression: public Expression{
    public:
    Value value_;

    BoundConstantExpression(Value value){
        value_= value;
        return_type_= value_.type_;

    }

    string toString() override;
};

class BoundColumnRefExpression: public Expression{
    public:
    ColumnBinding binding_;
    string col_name;

    BoundColumnRefExpression(string col_name, ColumnBinding binding): binding_(binding){

    }
    string toString() override;
};

// table相关结构

enum TableReferenceType{
    BASE_TABLE=0,
};

class TableCatalogEntry;
class BoundTableRef{
public:
    BoundTableRef(){

    }
    BoundTableRef(TableCatalogEntry* entry){

    }
};

class TableRef{
public:
    TableReferenceType type_;
    Value value_;

    TableRef(){
    }
    TableRef(Value v): value_(v), type_(TableReferenceType::BASE_TABLE){}
};

class BaseTableRef: public TableRef{
public:
    string table_name_;
};


// Node ********************************************************
enum QueryNodeType{
    SELECT_NODE=0,
};

class QueryNode{
    public:
    QueryNodeType type_;

    QueryNode(){}
    QueryNode(QueryNodeType type):type_(type){}
};

class SelectNode: public QueryNode{
    public:
    vector<unique_ptr<ParsedExpression>> select_list_;
    unique_ptr<TableRef> from_table_;

    SelectNode():QueryNode(QueryNodeType::SELECT_NODE){
        from_table_=nullptr;
    }
};

class BoundQueryNode{
    public:
    vector<string> names_;
    vector<LogicalType> types_; 

    virtual string toString()=0;
};

class BoundSelectNode: public BoundQueryNode{
public:
    vector<unique_ptr<Expression>> select_list_;
    unique_ptr<BoundTableRef> from_table_;

    string toString();
};



//class Schema{
//    public:
//        string schema_name_;// 列名
//};
//
//class Row{
//
//};
//// 数据表
//class Table{
//public:
//    vector<Schema> schema_list_; // 表头
//    vector<Row> rwo_list_; // 行
//};

enum StatementType {
    SELECT_STATEMENT =0, 
};

class SQLStatement{
    public:
        StatementType type_; //什么语句
        // select, 
};

class BoundStatement: public SQLStatement{
    public:
    vector<string> names_;

    unique_ptr<BoundQueryNode> node_;
    vector<LogicalType> types_;

    string toString();
};
class SelectStatement:public SQLStatement{
    public:
        string col_; // 待查询的列
        string table_; // 待查询的表

        QueryNode* node_;

};



class BindResult{
public:
    unique_ptr<Expression> expression_;
    BindResult(){}
    // BindResult(Expression* expr_ptr){
    //     expression_= expr_ptr;
    // }
    BindResult(unique_ptr<Expression> expr_ptr): expression_(std::move(expr_ptr)){}
};


bool test();
