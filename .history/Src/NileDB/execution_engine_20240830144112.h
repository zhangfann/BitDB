#pragma once
#include "catalog.cpp"
#include "parser.cpp"
#include <deque>

/* The execution engine that holds all execution operators that could be 
 * (sequential scan, index scan) which are access operators,
 * (filter, join, projection) that are relational algebra operators, 
 * other operator such as (sorting, aggrecations) 
 * or modification queries (delete, insert, update).
 * Each one of these operators implements a next() method that will produce the next record of the current table that
 * is being scanned filtered joined etc... This method has so many names ( volcano, pipleline, iterator ) as apposed to
 * building the entire output table and returning it all at once.
 * In order to initialize an operator we need some sorts of meta data that should be passed into it via the constructor,
 * The larger the project gets the more data we are going to need, Which might require using a wrapper class 
 * around the that data, But for now we are just going to pass everything to the constructor.
 */

class ExecutionEngine {
    public:
        ExecutionEngine(Catalog* catalog): catalog_(catalog)
    {}
        ~ExecutionEngine() {}

        bool create_table_handler(ASTNode* statement_root);

        bool insert_handler(ASTNode* statement_root);

        bool select_handler(ASTNode* statement_root){
            SelectStatementNode* select = reinterpret_cast<SelectStatementNode*>(statement_root);
            // handle filters later.
            // handle joins later ( only select values from the first table on the list ).

            auto table_ptr = select->tables_; 
            // did not find any tables.
            if(table_ptr == nullptr) return false;
            std::string table_name = table_ptr->token_.val_;
            TableSchema* schema = catalog_->getTableSchema(table_name);

            auto field_ptr = select->fields_;
            std::vector<std::string> fields;
            while(field_ptr != nullptr){
                std::string field_name = field_ptr->field_->token_.val_;
                // check valid column.
                if(!schema->isValidCol(field_name)) 
                    return false;

                fields.push_back(field_name);
                field_ptr = field_ptr->next_;
            }
            // handle duplicate fields later.

            TableIterator* it = schema->getTable()->begin();
            // print the schema at the top of the table
            schema->printTableHeader();
            while(it->advance()){
                Record r = it->getCurRecordCpy();
                std::vector<Value> values;
                int err = schema->translateToValues(r, values);
                if(err) return false;
                // filter the output based on the fields vector later.
                // our output is just printing for now.
                for(size_t i = 0; i < values.size(); ++i){
                    if(values[i].type_ == INT) 
                        std::cout << values[i].getIntVal();
                    else 
                        std::cout << values[i].getStringVal();

                    if(i < values.size() - 1 ) std::cout << " | ";
                }
                std::cout << std::endl;
            }
            return true;
        }

        bool delete_handler(ASTNode* statement_root){
            DeleteStatementNode* delete_statement = reinterpret_cast<DeleteStatementNode*>(statement_root);

            auto table_ptr = delete_statement->table_; 
            // did not find any tables.
            if(table_ptr == nullptr) return false;
            std::string table_name = table_ptr->token_.val_;
            TableSchema* schema = catalog_->getTableSchema(table_name);

            TableIterator* it = schema->getTable()->begin();
            while(it->advance()){
                RecordID rid = it->getCurRecordID();
                schema->getTable()->deleteRecord(rid);
            }
            // handle filters later.
            
            return true;
        }

        bool update_handler(ASTNode* statement_root){
            UpdateStatementNode* update_statement = reinterpret_cast<UpdateStatementNode*>(statement_root);

            std::string table_name = update_statement->table_->token_.val_;
            TableSchema* schema = catalog_->getTableSchema(table_name);
            // did not find any tables with that name.
            if(schema == nullptr) return false;
            auto field_ptr = update_statement->field_;
            std::string field_name = field_ptr->token_.val_;
            auto val_ptr = update_statement->expression_;
            std::string val_str = val_ptr->token_.val_;
            // check valid column.
            if(!schema->isValidCol(field_name)) 
                return false;
            // we consider int and string types for now.
            Type val_type = INVALID;
            if(val_ptr->category_ == STRING_CONSTANT) val_type = VARCHAR;
            else if(val_ptr->category_ == INTEGER_CONSTANT) val_type = INT;
            // invalid or not supported type;
            if( val_type == INVALID ) return false;
            Value val;
            if(val_type == INT) val = Value(stoi(val_str));
            else if(val_type == VARCHAR) val = Value(val);

            if(!schema->checkValidValue(field_name, val)) return false;


            TableIterator* it = schema->getTable()->begin();
            while(it->advance()){
                RecordID rid = it->getCurRecordID();
                // rid is not used for now.
                Record cpy = it->getCurRecordCpy();
                std::vector<Value> values;
                int err = schema->translateToValues(cpy, values);
                int idx = schema->getColIdx(field_name, val);
                if(idx < 0) return false;
                values[idx] = val;
                Record* new_rec = schema->translateToRecord(values);

                err = schema->getTable()->updateRecord(&rid, *new_rec);
                if(err) return false;
            }
            return true;
            // handle filters later.
        }


        bool execute(ASTNode* statement_root){
            if(!statement_root) return false;

            switch (statement_root->category_) {
                case CREATE_TABLE_STATEMENT:
                    return create_table_handler(statement_root);
                case INSERT_STATEMENT:
                    return insert_handler(statement_root);
                case SELECT_STATEMENT:
                    return select_handler(statement_root);
                case DELETE_STATEMENT:
                    return delete_handler(statement_root);
                case UPDATE_STATEMENT:
                    return update_handler(statement_root);
                default:
                    return false;
            }
        }
    private:
        Catalog* catalog_;
};