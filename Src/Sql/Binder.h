#pragma once

#include "Statement.h"
#include "Catalog.h"

#include <iostream>
#include <memory>
#include <vector>
#include <map>
using namespace std;




class Binding{
    public:
    int index_; //table index
    string alias_; // 表名
    vector<string> names_; // 列名
    map<string, int> names_map_; // 列名->id

    Binding(int index, vector<string> colnames, string& alias):index_(index), alias_(alias), names_(std::move(colnames)){
        for(int i=0; i< names_.size(); i++){
            auto& name= names_[i];

            names_map_[name]=i;
        }
    }

    BindResult bind(ColumnRefExpression &colref, int depth);


    int tryGetBindingIndex(string column_name);

    ColumnBinding getColumnBinding(int column_index);
};

class TableBinding: public Binding{
public:
        TableBinding(int table_index, vector<string> colnames, string alias): Binding(table_index, colnames, alias){

        }
};



// Binder *********************************************************************************
class BindContext{
    public:
    map<string, unique_ptr<Binding>> bindings_;

    BindResult bindColumn(ColumnRefExpression& colref, int depth);

    Binding* getBinding(const string& name);

    void addBaseTable(int table_index, string alias, vector<string> colnames);

    void addBinding(string& alias, unique_ptr<Binding> binding);
};

class Binder{
public:
    //  何时初始化
    BindContext bind_context_;

    int bound_tables_; // 用于产生table index

    Binder():bound_tables_(0){

    }

    BoundStatement bindStatement(SQLStatement& statement);

    BoundStatement bind(SelectStatement& stmt);




    // 分析select_list中的节点
    BoundStatement bind(QueryNode& node);

    unique_ptr<BoundQueryNode> bindNode(QueryNode& node);

    unique_ptr<BoundQueryNode> bindNode(SelectNode& node);

    // void expandStarExpressions(vector<unique_ptr<ParsedExpression> > & select_list,
    //     vector<unique_ptr<ParsedExpression> >& new_select_list){
    //         for(auto& select_element: select_list){
    //             expandStarExpression(std::move(select_element), new_select_list);
    //         }
    // }

    // void expandStarExpression(unique_ptr<ParsedExpression> expr, 
    //     vector<unique_ptr<ParsedExpression> > & new_select_list){
            
    //     }


    // 分析 from_table
    unique_ptr<BoundTableRef> bind(TableRef &ref);

    unique_ptr<BoundTableRef> bind(BaseTableRef &ref);

    int generateTableIndex();
    
};
class ExpressionBinder{
    public:
    // 在何处初始化
    Binder binder_;

    std::unique_ptr<Expression> bind(unique_ptr<ParsedExpression> &expr, LogicalType &result_type);

    string bind(unique_ptr<ParsedExpression>&expr, int depth);

    BindResult bindExpression(std::unique_ptr<ParsedExpression> &expr, int depth);

    // 常量
    BindResult bindExpression(ConstantExpression& expr, int depth);

    // 列名
    BindResult bindExpression(ColumnRefExpression& expr, int depth);
};






