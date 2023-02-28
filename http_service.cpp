#include "http_service.h"
#include "kv_json.h"
#include "zerotrust.h"
#include "config.h"
#include "logger.h"
#include "db.h"

#include <set>
#include <mutex>

using namespace std;

std::mutex g_i_mutex;

void lock_test(){
	const std::lock_guard<std::mutex> lock(g_i_mutex);

}

// 分割字符串 str delim
bool split(const string& str, const string& delim, vector<string>& res) {
    if ("" == str) return true;
    //先将要切割的字符串从string类型转换为char*类型
    char* strs = new char[str.length() + 1]; //不要忘了
    strcpy(strs, str.c_str());

    char* d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char* p = strtok(strs, d);
    while (p) {
        string s = p; //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        p = strtok(NULL, d);
    }

    delete[] strs;
    delete[] d;
    return true;
}

std::vector<int> intersection (const std::vector<std::vector<int>> &vecs) {

    auto last_intersection = vecs[0];
    std::vector<int> curr_intersection;

    for (std::size_t i = 1; i < vecs.size(); ++i) {
        std::set_intersection(last_intersection.begin(), last_intersection.end(),
            vecs[i].begin(), vecs[i].end(),
            std::back_inserter(curr_intersection));
        std::swap(last_intersection, curr_intersection);
        curr_intersection.clear();
    }
    return last_intersection;
}



// callback ***************************************
void get_list_callback(const httplib::Request& req, httplib::Response& resp)
{
  (void)req;

	int depth = stoi(req.get_param_value("depth"));

	// 获取数据
	vector<MarkdownItem> result_list;
	get_list(depth, result_list);

	// 转换为json
	  string data="";
		Json::Value json_root;
	  for(auto item:result_list){
	    Json::Value json_data;
			json_data["id"]= Json::Value(item.id);
			json_data["title"]= Json::Value(item.title);

			json_root.append(Json::Value(json_data));

	  }
	//		ret_str= json_root.toStyledString();
		Json::FastWriter writer;
		data = writer.write(json_root);

	  cout<<"data:"<<data<<endl;

	  string body = data;

  resp.set_content(body.c_str(),body.size(),"application/json");
}

void search_callback(const httplib::Request& req, httplib::Response& resp)
{
  (void)req;

	string query_str = req.get_param_value("query_str");

  std::string body = query_str;

  // query分词=>[word]
  string shell_path = gConfig->shell_path;
  string python3_path= gConfig->python3_path;
  char cmd[MAX_STRING_LEN] = { 0 };
  char result[MAX_STRING_LEN] = { 0 };
  bool ret = false;
  snprintf(cmd, MAX_STRING_LEN, "%s %s/cut_words.py '%s'", python3_path.c_str(), shell_path.c_str(), \
	  query_str.c_str());
  cout << cmd << endl;
  gLogger->info(cmd);
  ret = getShellResult(cmd, result);
  if (!ret) {
	  cout << "cut_words 失败" << endl;
	  return ;
  }
  //result去除空格
   result[strlen(result)-1]='\0';
  //for word: [word]
//	  查询db
  vector<string> query_list;
  split(result, " ", query_list);

  // set<int> result_set;
  vector<vector<int>> result_list;
  for (auto word : query_list) {
    printf("word:%s ", word.c_str());
    vector<int> result;
     mysql_query_word(word, result);
     result_list.push_back(result);
    // for (auto res: result){
    //   result_set.insert(res);
    // }
  }

  // 求交集
  std::vector<int> intersection_result;
  intersection_result = intersection (result_list);


  // set<int>::iterator it;
  // for(it=result_set.begin();it!=result_set.end();it++)  //使用迭代器进行遍历
  // {
  //     printf("%d\n",*it);
  // }


  // 根据result_set 还原为title
  vector<MarkdownItem> markdown_item_vec;
  for(auto res: intersection_result){
    MarkdownItem item;
    item.id=res;
    // item.title= select_title_by_id(res);
    select_title_content_by_id(res, item.title, item.content);
    vector<int> nested_list_int;
    select_nested_list_by_id(res, nested_list_int);
    for(auto id: nested_list_int){
      // cout<<"nested_list_int:"<<id<<endl;
      MarkdownSubItem sub_item;
      sub_item.id=id;
      sub_item.title=select_title_by_id(id);
      item.nested_list.push_back(sub_item);
    }
    markdown_item_vec.push_back(item);
  }


  // 转换为json
  string data="";
	Json::Value json_root;
  for(auto item:markdown_item_vec){
    Json::Value json_data;
		json_data["id"]= Json::Value(item.id);
		json_data["title"]= Json::Value(item.title);
    json_data["content"]= Json::Value(item.content);
    json_data["nested_list"]= Json::Value();
    // cout<<item.title<<endl;
    for(auto sub_item:item.nested_list){
      // cout<<" "<<sub_item.title<<endl;
      Json::Value sub_json_data;
      sub_json_data["id"]= Json::Value(sub_item.id);
		  sub_json_data["title"]= Json::Value(sub_item.title);
      json_data["nested_list"].append(sub_json_data);
    }
		json_root.append(Json::Value(json_data));

  }
//		ret_str= json_root.toStyledString();
	Json::FastWriter writer;
	data = writer.write(json_root);

  cout<<"data:"<<data<<endl;

  body = data;
  resp.set_content(body.c_str(),body.size(),"application/json");
}

void index_callback(const httplib::Request& req, httplib::Response& resp)
{
  (void)req;
  std::string body = "111";
  resp.set_content(body.c_str(),body.size(),"text/html");
}

void add_callback(const httplib::Request& req, httplib::Response& resp)
{
  printf("add_callback\n");
		// json j;
		double a, b;
		string2num(req.get_param_value("a").c_str(), a);
		string2num(req.get_param_value("b").c_str(), b);
    std::cout<<"add_callback:"<<a<<b<<std::endl;
		double ans = a + b;
		std::string body = std::to_string(ans);
		resp.set_content(body.c_str(),body.size(),"text/plain");
}

void login_callback(const httplib::Request& req, httplib::Response& resp)
{
  printf("login_callback\n");
		// json j;
		std::string username = req.get_param_value("username");
		std::string password = req.get_param_value("password");
    std::cout<<username<<password<<std::endl;


    std::string config_username;
    std::string config_password;
//    read_(config_username, config_password);

    std::string body;
    if(username == config_username && password == config_password){
      // 验证通过
      // TODO 导航至配置ip页面
      body = "login ok";
		  
    }else{
      // 验证不通过
      body = "login error";
    }
    resp.set_content(body.c_str(),body.size(),"text/plain");
}
