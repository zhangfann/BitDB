#pragma once

#include "Statement.h"

#include <vector>
#include <string>
using namespace std;

// ColumnDefinition
class Column{
public:
    string name_;
};

enum CatalogType{
    TABLE_ENTRY=0,
};

class CatalogEntry{
public:
    CatalogType type_;
};

class StandardEntry: public CatalogEntry{

};

class TableCatalogEntry:public StandardEntry{
public:
    
    // ColumnList columns_;
    vector<Column> columns_;
    vector<Column> getColumns();
};


class Catalog{
public:
    static CatalogEntry*  getEntry(const string& name){

    }
};
// class Expression;
// class BindResult{
// public:
//     Expression* expression_;
//     BindResult(){}
//     BindResult(Expression* expr_ptr){
//         expression_= expr_ptr;
//     }
// };

