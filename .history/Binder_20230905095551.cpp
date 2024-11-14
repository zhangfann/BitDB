
#include "Binder.h"
// #include "Statement.h"
// #include "Catalog.h"

#include <iostream>
#include <memory>
#include <vector>
using namespace std;






// Binder *********************************************************************************


std::unique_ptr<Expression> ExpressionBinder::bind(unique_ptr<ParsedExpression> &expr, LogicalType &result_type){
    auto err_msg= bind(expr, 0);
    auto bound_expr= (BoundExpression*) expr.get();

    unique_ptr<Expression> result= std::move(bound_expr->expr_);

    result_type= result->return_type_;

    return result;

}
string ExpressionBinder::bind(unique_ptr<ParsedExpression> &expr, int depth){
    auto &expression= *expr;

    BindResult result= bindExpression(expr, depth);

    // bind成功后, 使用bound后的expr替换原有的
    std::unique_ptr<BoundExpression> bound_expr_ptr= std::make_unique<BoundExpression>(result.expression_);
    expr= std::move(bound_expr_ptr);
    return string();
}

BindResult ExpressionBinder::bindExpression(unique_ptr<ParsedExpression> &expr, int depth){
    auto& expr_ref= *expr;
    if(expr_ref.expression_class_ == ExpressionClass::Constant){
        return bindExpression((ConstantExpression &)expr_ref, depth);
    }else if(expr_ref.expression_class_ == ExpressionClass::COLUMN_REF){
        return bindExpression((ColumnRefExpression &)expr_ref, depth);   
    }
}

// 常量
BindResult ExpressionBinder::bindExpression(ConstantExpression& expr, int depth){
    unique_ptr<BoundConstantExpression> bound_constant_expr_ptr= std::make_unique<BoundConstantExpression>(expr.value_);
    return BindResult(std::move(bound_constant_expr_ptr));
}

// 列名
BindResult ExpressionBinder::bindExpression(ColumnRefExpression& expr, int depth){
    BindResult result;
    result= binder_.bind_context_.bindColumn(expr, depth);
    return result;
}

BoundStatement Binder::bindStatement(SQLStatement& statement){
    if(statement.type_ == StatementType::SELECT_STATEMENT){
        return bind((SelectStatement &) statement);
    }
        
}

BoundStatement Binder::bind(SelectStatement& stmt){
    return bind(*stmt.node_);
    
}



// 分析select_list中的节点
BoundStatement Binder::bind(QueryNode& node){
    auto bound_node= bindNode(node);

    //?? 这里的结果没有绑定
    BoundStatement result;
    result.names_= bound_node->names_;
    result.types_= bound_node->types_;
    result.node_= std::move(bound_node);
    return result;
}

unique_ptr<BoundQueryNode> Binder::bindNode(QueryNode& node){
    unique_ptr<BoundQueryNode> result;
    if(node.type_ == QueryNodeType::SELECT_NODE){
        result = bindNode((SelectNode &) node);
    }

    return result;


}

unique_ptr<BoundQueryNode> Binder::bindNode(SelectNode& node){
    auto result= make_unique<BoundSelectNode>();

    // from子句
    if(node.from_table_){
        result->from_table_= bind(*node.from_table_);

    }

    // TODO select list子句, select *, 将*展开
    // vector<unique_ptr<ParsedExpression> > new_select_list;
    // expandStarExpressions(node.select_list_, new_select_list);

    // SelectBinder select_binder;
    ExpressionBinder expression_binder;
    for(int i=0; i < node.select_list_.size(); i++){
        LogicalType result_type;
        auto expr= expression_binder.bind(node.select_list_[i], result_type);
        result->select_list_.push_back(std::move(expr));
    }

    return std::move(result);

}

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
unique_ptr<BoundTableRef> Binder::bind(TableRef &ref){
    unique_ptr<BoundTableRef> result;
    if(ref.type_ == TableReferenceType::BASE_TABLE){
        result = bind((BaseTableRef &)ref);
    }

    return result;
}

unique_ptr<BoundTableRef> Binder::bind(BaseTableRef &ref){
    auto table_or_view= Catalog::getEntry(ref.table_name_);
    if(table_or_view->type_ == CatalogType::TABLE_ENTRY){
        auto table= (TableCatalogEntry*)table_or_view;
        //  这里bind table后 需要返回什么信息
        vector<string> colnames;
        for(auto col: table->getColumns()){
            colnames.push_back(col.name_);
        }
        string alias= ref.table_name_;
        int table_index= generateTableIndex();
        bind_context_.addBaseTable(table_index, alias, colnames);
        return make_unique<BoundTableRef>(table);
    }
}

int Binder::generateTableIndex(){
    return bound_tables_++;
}





BindResult BindContext::bindColumn(ColumnRefExpression& colref, int depth){
    auto binding= getBinding(colref.getTableName());
    return binding->bind(colref, depth);
}

Binding* BindContext::getBinding(const string& name){
    // 根据列名查找对应的列
    auto match= bindings_.find(name);
    return match->second.get();
}

void BindContext::addBaseTable(int table_index, string alias, vector<string> colnames){
    auto binding= make_unique<TableBinding>(table_index, colnames, alias);
    addBinding(alias, std::move(binding));
}

void BindContext::addBinding(string& alias, unique_ptr<Binding> binding){
    bindings_[alias]= std::move(binding);
}


BindResult Binding::bind(ColumnRefExpression &colref, int depth){
    string column_name= colref.getColumnName();
    int column_index= tryGetBindingIndex(column_name);
    ColumnBinding binding= getColumnBinding(column_index);

    std::unique_ptr<BoundColumnRefExpression> bound_column= std::make_unique<BoundColumnRefExpression>(colref.getColumnName(), binding);
    return BindResult(std::move(bound_column));
}

int Binding::tryGetBindingIndex(string column_name){
    auto entry= names_map_.find(column_name);
    if(entry == names_map_.end()){
        return -1;
    }
    auto column_index= entry->second;
    return column_index;
}


ColumnBinding Binding::getColumnBinding(int column_index){
    ColumnBinding binding;
    binding.column_index_= column_index;
    binding.table_index_= index_;
    return binding;
}