#include "Statement.h"

bool test(){
    // select name from test_table
    SelectStatement select_statement= SelectStatement();
    select_statement.type_= StatementType::SELECT_STATEMENT;
    select_statement.col_= "name";
    select_statement.table_= "test_table";

//    Binder binder;
//    BoundStatement bound_statement= binder.bind(select_statement);
//
//    Planner planner;
//    ExecutionPlan plan= planner.plan(bound_statement);
//
//    Executor executor;
//    Result result= executor.execute(plan);

}

int main(){
    test();
}