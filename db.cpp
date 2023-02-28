#include "db.h"
#include "zerotrust.h"

#include "mysql.h"



void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  return ;        
}

int get_list(int depth, vector<MarkdownItem>& result_list){
	MYSQL *con = mysql_init(NULL);

	  if (con == NULL)
	  {
	      fprintf(stderr, "mysql_init() failed\n");
	    //   gLogger->error("mysql_init() failed");
	      return -1;
	  }

	  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");

	  if (mysql_real_connect(con, "localhost", "root", "dahuacloud",
	          "search_fandb", 0, NULL, 0) == NULL)
	  {
	      finish_with_error(con);
	  }

	    char cmd[MAX_STRING_LEN] = {0};
	    // snprintf(cmd, MAX_STRING_LEN, "select child_id from nested_list where parent_id like '%%s%';", word);
	    string sql_cmd="select id, title from markdown where depth ="+to_string(depth)+";";
	    // gLogger->info(cmd);
	    printf("cmd:%s", sql_cmd.c_str());

	  if (mysql_query(con, sql_cmd.c_str()))
	  {
	      finish_with_error(con);
	  }

	  MYSQL_RES *mysql_result = mysql_store_result(con);

	  if (mysql_result == NULL)
	  {
	      finish_with_error(con);
	  }

	  int num_fields = mysql_num_fields(mysql_result);

	  MYSQL_ROW row;
	  MYSQL_FIELD *field;

	   while ((row = mysql_fetch_row(mysql_result)))
	  {
	    //   AuditData data;
		   MarkdownItem markdown;
	      markdown.id=row[0] ? atoi(row[0]) : -1;
	      markdown.title=row[1] ? row[1] : "";
	    //   data.des_ip=row[1] ? row[1] : "NULL";
	    //   data.src_port=row[2] ? atoi(row[2]) : -1;
	    //   data.des_port=row[3] ? atoi(row[3]) : -1;
	    //   data.protocol=row[4] ? row[4] : "NULL";
	    //   data.data_size=row[5] ? atoi(row[5]) : -1;
	    //   data.operate_time=row[6] ? row[6] : "NULL";

	      result_list.push_back(markdown);


	  }


	  mysql_free_result(mysql_result);
	  mysql_close(con);

	  return 0;
}

int select_nested_list_by_id(int id, vector<int>&nested_list_int){
    MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
    //   gLogger->error("mysql_init() failed");
      return -1;
  }  

  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");

  if (mysql_real_connect(con, "localhost", "root", "dahuacloud", 
          "search_fandb", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }    

    char cmd[MAX_STRING_LEN] = {0};
    // snprintf(cmd, MAX_STRING_LEN, "select child_id from nested_list where parent_id like '%%s%';", word);
    string sql_cmd="select child_id from nested_list where parent_id ="+to_string(id)+";";
    // gLogger->info(cmd);
    printf("cmd:%s", sql_cmd.c_str());

  if (mysql_query(con, sql_cmd.c_str())) 
  {
      finish_with_error(con);
  }

  MYSQL_RES *mysql_result = mysql_store_result(con);

  if (mysql_result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(mysql_result);

  MYSQL_ROW row;
  MYSQL_FIELD *field;

   while ((row = mysql_fetch_row(mysql_result))) 
  { 
    //   AuditData data;
      int id=row[0] ? atoi(row[0]) : -1;
    //   data.des_ip=row[1] ? row[1] : "NULL";
    //   data.src_port=row[2] ? atoi(row[2]) : -1;
    //   data.des_port=row[3] ? atoi(row[3]) : -1;
    //   data.protocol=row[4] ? row[4] : "NULL";
    //   data.data_size=row[5] ? atoi(row[5]) : -1;
    //   data.operate_time=row[6] ? row[6] : "NULL";

      nested_list_int.push_back(id);


  }


  mysql_free_result(mysql_result);
  mysql_close(con);

  return 0;

}
int select_title_content_by_id(int id, string& title, string& content){
 MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
    //   gLogger->error("mysql_init() failed");
      return -1;
  }  

  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");

  if (mysql_real_connect(con, "localhost", "root", "dahuacloud", 
          "search_fandb", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }    

    char cmd[MAX_STRING_LEN] = {0};
    // snprintf(cmd, MAX_STRING_LEN, "select title from markdown where title like '%%s%';", word);
    string sql_cmd="select  title,content from markdown where id ="+to_string(id)+";";
    // gLogger->info(cmd);
    printf("cmd:%s", sql_cmd.c_str());

  if (mysql_query(con, sql_cmd.c_str())) 
  {
      finish_with_error(con);
  }

  MYSQL_RES *mysql_result = mysql_store_result(con);

  if (mysql_result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(mysql_result);

  MYSQL_ROW row;
  MYSQL_FIELD *field;

  row = mysql_fetch_row(mysql_result);
//   AuditData data;
    title=row[0] ? row[0] : "";
    content=row[1] ? row[1] : "";
//   data.des_ip=row[1] ? row[1] : "NULL";
//   data.src_port=row[2] ? atoi(row[2]) : -1;
//   data.des_port=row[3] ? atoi(row[3]) : -1;
//   data.protocol=row[4] ? row[4] : "NULL";
//   data.data_size=row[5] ? atoi(row[5]) : -1;
//   data.operate_time=row[6] ? row[6] : "NULL";


  mysql_free_result(mysql_result);
  mysql_close(con);

  return 0;

}
string select_title_by_id(int id)
// int main(int argc, char **argv)
{      
  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
    //   gLogger->error("mysql_init() failed");
      return "";
  }  

  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");

  if (mysql_real_connect(con, "localhost", "root", "dahuacloud", 
          "search_fandb", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }    

    char cmd[MAX_STRING_LEN] = {0};
    // snprintf(cmd, MAX_STRING_LEN, "select title from markdown where title like '%%s%';", word);
    string sql_cmd="select title from markdown where id ="+to_string(id)+";";
    // gLogger->info(cmd);
    printf("cmd:%s", sql_cmd.c_str());

  if (mysql_query(con, sql_cmd.c_str())) 
  {
      finish_with_error(con);
  }

  MYSQL_RES *mysql_result = mysql_store_result(con);

  if (mysql_result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(mysql_result);

  MYSQL_ROW row;
  MYSQL_FIELD *field;

  row = mysql_fetch_row(mysql_result);
//   AuditData data;
    string  title=row[0] ? row[0] : "";
//   data.des_ip=row[1] ? row[1] : "NULL";
//   data.src_port=row[2] ? atoi(row[2]) : -1;
//   data.des_port=row[3] ? atoi(row[3]) : -1;
//   data.protocol=row[4] ? row[4] : "NULL";
//   data.data_size=row[5] ? atoi(row[5]) : -1;
//   data.operate_time=row[6] ? row[6] : "NULL";


  mysql_free_result(mysql_result);
  mysql_close(con);

  return title;
}

// 获取数据
int mysql_query_word(string word, vector<int>& result)
// int main(int argc, char **argv)
{      
  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
      fprintf(stderr, "mysql_init() failed\n");
    //   gLogger->error("mysql_init() failed");
      return -1;
  }  

  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");

  if (mysql_real_connect(con, "localhost", "root", "dahuacloud", 
          "search_fandb", 0, NULL, 0) == NULL) 
  {
      finish_with_error(con);
  }    

    char cmd[MAX_STRING_LEN] = {0};
    // snprintf(cmd, MAX_STRING_LEN, "select id from markdown where title like '%%s%';", word);
    string sql_cmd="select id from markdown where title like '%"+word+"%';";
    // gLogger->info(cmd);
    printf("cmd:%s", sql_cmd.c_str());

  if (mysql_query(con, sql_cmd.c_str())) 
  {
      finish_with_error(con);
  }

  MYSQL_RES *mysql_result = mysql_store_result(con);

  if (mysql_result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(mysql_result);

  MYSQL_ROW row;
  MYSQL_FIELD *field;

  while ((row = mysql_fetch_row(mysql_result))) 
  { 
    //   AuditData data;
      int id=row[0] ? atoi(row[0]) : -1;
    //   data.des_ip=row[1] ? row[1] : "NULL";
    //   data.src_port=row[2] ? atoi(row[2]) : -1;
    //   data.des_port=row[3] ? atoi(row[3]) : -1;
    //   data.protocol=row[4] ? row[4] : "NULL";
    //   data.data_size=row[5] ? atoi(row[5]) : -1;
    //   data.operate_time=row[6] ? row[6] : "NULL";

      result.push_back(id);


  }

  mysql_free_result(mysql_result);
  mysql_close(con);

  return 0;
}

static int read_client_auth_str_callback(void *data, int argc, char** argv, char** azColName){
//    int i;
    // printf("%s: ", (const char*)data);
    ClientAuthInfo info;
    for(int i=0;i <argc; i++){
        // printf("%s = %s\n", azColName[i], argv[i]?argv[i]:"NULL");
//        monthly_sqlite_root[monthly_sqlite_len][azColName[i]]= argv[i]?argv[i]:"NULL";
        if(strcmp(azColName[i], "client_uuid")==0){
        	info.client_uuid=argv[i]?argv[i]:"NULL";
        }else if(strcmp(azColName[i], "client_auth_str")==0){
        	info.client_auth_str=argv[i]?argv[i]:"NULL";
        }
    }
    // printf("\n");
//    monthly_sqlite_len+=1;

	// 添加至本地客户端字符串列表
//    ClientAuthInfo info;
//	info.client_uuid=client_uuid;
//	info.client_auth_str=client_auth_str;
	gClientAuthList->add(info);
    return 0;
}

// 开机读取client_auth_str
int read_client_auth_str(){
	string db_path= gConfig->db_path;
    sqlite3 *db;
    int rc= sqlite3_open(db_path.c_str(), &db);
    if(rc){
        gLogger->error("sqlite3 open err");
        return -1;
    }else{
        gLogger->info("sqlite3 open succ");

    }

    const char* data="callback function called";
    char* zErrMsg=0;

    char *sql = "select client_uuid, client_auth_str from client_info";
    rc= sqlite3_exec(db, sql, read_client_auth_str_callback, (void*)data, &zErrMsg);
    if(rc != SQLITE_OK){
        gLogger->error("sql err {}", zErrMsg);
        sqlite3_free(zErrMsg);
    }else{
        gLogger->error("sql succ");
    }

    sqlite3_close(db);
    return 0;

}

static int add_client_auth_str_callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

// 运行中更新client_auth_str到数据库
int add_client_auth_str(string client_uuid, string client_auth_str){
	string db_path= gConfig->db_path;
	sqlite3 *db;
	int rc= sqlite3_open(db_path.c_str(), &db);
	if(rc){
		gLogger->error("sqlite3 open err");
		return -1;
	}else{
		gLogger->info("sqlite3 open succ");

	}

	const char* data="callback function called";
	char* zErrMsg=0;

	char sql[MAX_STRING_LEN] = {0};
	snprintf(sql, MAX_STRING_LEN, "insert into client_info (client_uuid, client_auth_str) \
			values (%s, %s)",\
			client_uuid.c_str(), client_auth_str.c_str());
//	char *sql = "insert into client_info client_uuid, client_auth_str from client_info";
	rc= sqlite3_exec(db, sql, add_client_auth_str_callback, (void*)data, &zErrMsg);
	if(rc != SQLITE_OK){
		gLogger->error("sql err {}", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return -1;
	}else{
		gLogger->info("sql succ");
	}

	sqlite3_close(db);
	return 0;
}


