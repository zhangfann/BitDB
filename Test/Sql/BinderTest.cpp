#include "Binder.h"
#include "Statement.h"

#include <iostream>
#include <random>
#include <cassert>

auto tryBind(SQLStatement& statement){
    Binder binder;
    
    auto bounded_statement= binder.bindStatement(statement);
    return bounded_statement;
}

string printStatement(BoundStatement& bound_statement){
    std::cout<< bound_statement.toString() << std::endl;
    return bound_statement.toString();
}

void test_BindSelectValue(){
    // select 1

    SelectStatement statement= SelectStatement();
    statement.type_= StatementType::SELECT_STATEMENT;
    statement.col_= "";
    statement.table_="";

    // 产生随机数
        // 使用随机设备作为种子
    std::random_device rd;
    // 使用 Mersenne Twister 引擎
    std::mt19937 gen(rd());
    // 定义要生成的随机数的范围
    std::uniform_int_distribution<> dis(0, 100);

    // 生成随机数
    int random_number = dis(gen);

    auto expr= new ConstantExpression(Value(random_number));
    cout<<"aaa:"<<expr->expression_class_<<endl;

    auto node= new SelectNode();
    node->select_list_.push_back(expr);
    statement.node_= node;

    auto bounded_statement= tryBind(statement);
    string result= printStatement(bounded_statement);
	
	size_t found= result.find(to_string(random_number));
    assert(found!= string::npos);
}

void test_BindSelectFrom(){
    // 手动在catalog中建表

     SelectStatement statement= SelectStatement();
    statement.type_= StatementType::SELECT_STATEMENT;


    auto expr= new ColumnRefExpression(Value("x"));

    auto node= new SelectNode();
    node->select_list_.push_back(expr);
    statement.node_= node;

    auto table= make_unique<TableRef>(Value("y"));
    node->from_table_= std::move(table);

    auto bounded_statement= tryBind(statement);
    string result= printStatement(bounded_statement);

    size_t found= result.find("ColumnRefExpression:x");
    assert(found!= string::npos);
}

int main(){
    // select 1
//    test_BindSelectValue();

    // create table y (x int, z int, a int, b int, c int)
    // select x from y
    test_BindSelectFrom();
}
