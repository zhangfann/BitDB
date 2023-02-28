#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include "ClientAuthList.h"
#include "Config.h"
#include "http_service.h"

int get_list(int depth, vector<MarkdownItem>& result_list);
int select_nested_list_by_id(int id, vector<int>&nested_list_int);
int select_title_content_by_id(int id, string& title, string& content);
string select_title_by_id(int id);
int mysql_query_word(string word, vector<int>& result);
static int read_client_auth_str_callback(void *data, int argc, char** argv, char** azColName);
int read_client_auth_str();
int add_client_auth_str(string client_uuid, string client_auth_str);
#endif
