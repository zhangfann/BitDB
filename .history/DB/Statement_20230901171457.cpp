#include "Statement.h"


#include <string>
#include <vector>
#include <memory>
using namespace std;
// Expression **************************************************************************************




    string BoundExpression::toString() {
        return expr_->toString();
    }



    string Value::toString(){
        if(type_==LogicalType::INTEGER){
            return to_string(int_value_);
        }
    }




       string ConstantExpression::toString()  {  // 覆盖基类的虚函数
       return "ConstantExpression:"+value_.toString();
    }



    string BoundConstantExpression::toString() {
        return "BoundConstantExpression:"+value_.toString();
    }


// Node ********************************************************


    string BoundSelectNode::toString() {
        string result_str;
        for(auto expr_ptr : select_list_){
            result_str+= expr_ptr->toString();
        }
        return "BoundSelectNode:"+result_str;
    }



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



    string BoundStatement::toString(){
        string str= node_->toString();
            return str;
    }


    const string& ColumnRefExpression::getTableName(){
        return column_names_[0];
    }
// table相关结构

//class Binder{
//    public:
//        bind(SQLStatement* sql_statement){
//            switch(sql_statement->type_){
//                case "select":
//                    BoundStatement bound_statement= BoundStatement();
//                    break;
//                default:
//                    break;
//            }
//        }
//};
//


string ColumnRefExpression::toString(){
    return "ColumnRefExpression:"+value_.toString();
}



string BoundColumnRefExpression::toString(){
    return "BoundColumnRefExpression:";
}

string ColumnRefExpression::getColumnName(){
    return column_names_.back();
}