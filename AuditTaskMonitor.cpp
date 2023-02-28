#include "AuditTaskMonitor.h"
#include "logger.h"
#include "zerotrust.h"
#include <thread>
#include <ctime>
#include <sstream>
#include <iostream>

#include <stdio.h>
// jsoncpp的头文件 在 /usr/include/jsoncpp中
#include <json/value.h>

#ifdef __APPLE__
    #include <json/json.h>
#else
    #include <jsoncpp/json/json.h>
#endif

using namespace std;

struct UploadAuditData{
	string date;
	string ip;
	string username;
	string cmd;
	string server_ip;
};

time_t str_to_time_t(const string& ATime, const string& AFormat="%Y-%m-%d %H:%M:%S"){
	struct tm timeDate;
	memset(&timeDate,0,sizeof(struct tm));
	strptime(ATime.c_str(), AFormat.c_str(),&timeDate);
	time_t  t1 = mktime(&timeDate);
	return t1;
}

time_t str_to_time_t_tmp(const string& ATime, const string& AFormat="%d-%d-%d %d:%d:%d")
{
    struct tm tm_Temp;
    time_t time_Ret;
    try
    {
        int i = sscanf(ATime.c_str(), AFormat.c_str(),// "%d/%d/%d %d:%d:%d" ,
                    &(tm_Temp.tm_year),
                    &(tm_Temp.tm_mon),
                    &(tm_Temp.tm_mday),
                    &(tm_Temp.tm_hour),
                    &(tm_Temp.tm_min),
                    &(tm_Temp.tm_sec));

        tm_Temp.tm_year -= 1900;
        tm_Temp.tm_mon--;
//        tm_Temp.tm_hour=0;
//        tm_Temp.tm_min=0;
//        tm_Temp.tm_sec=0;
//        tm_Temp.tm_isdst = 0;
        time_Ret = mktime(&tm_Temp);
        return time_Ret;
    } catch(...) {
        return 0;
    }
}
AuditTaskMonitor::AuditTaskMonitor(){
	m_destroy_flag= false;
}

bool do_audit_ssh(std::shared_ptr<AuditTask>& audit_task){
    char cmd[MAX_STRING_LEN] = {0};
    char result[MAX_STRING_LEN] = {0};
    bool ret= false;
	string shell_path=gConfig->shell_path;
	string id_rsa_path=gConfig->id_rsa_path;

	// 获取审计日志

	thread::id this_thread_id= this_thread::get_id();
	stringstream ss;
	ss << this_thread_id;
	string s_thread_id = ss.str();
	string audit_dir="/tmp/audit_ssh/"+s_thread_id;
	snprintf(cmd, MAX_STRING_LEN, "mkdir -p %s", audit_dir.c_str());
	gLogger->info(cmd);
	cout<<cmd<<endl;
	ret = getShellResult(cmd, result);
	if(!ret){
		cout <<"mkdir /tmp/audit_ssh 失败"<<endl;
		return false;
	}

	time_t t = time(0);
	char tmp[32]={NULL};
	strftime(tmp, sizeof(tmp), "%Y%m%d",localtime(&t));
	string s_tmp=tmp;
	string command_file="command_"+string(tmp)+".log";
	string command_path="/tmp/"+command_file;
	snprintf(cmd, MAX_STRING_LEN, "scp -i %s %s@%s:%s %s", \
					id_rsa_path.c_str(), audit_task->username.c_str(), audit_task->ip.c_str(), command_path.c_str(), audit_dir.c_str());
	gLogger->info(cmd);
	cout<<cmd<<endl;
	ret = getShellResult(cmd, result);
	if(!ret){
		cout <<"do_audit_ssh scp 失败"<<endl;
		return false;
	}

	// 解析审计日志
	string filename=audit_dir+"/"+command_file;
    vector<string> lines;
    string line;

    ifstream input_file(filename);
    if (!input_file.is_open()) {
    	gLogger->error("Could not open the file - {}",filename);
//        cerr << "Could not open the file - '"<< filename << "'" << endl;
        return false;
    }

    while (getline(input_file, line)){
        lines.push_back(line);
    }

    vector<struct UploadAuditData> result_list;
    for (const auto &i : lines){
//    	cout << i << endl;
    	Json::Reader reader;
    	Json::Value root;
    	if(reader.parse(i,root))
	  {
		  string parsed_date = root["date"].asString();
		  string parsed_logname = root["logname"].asString();
		  string parsed_user_ip = root["user_ip"].asString();
		  string parsed_cmd = root["cmd"].asString();

//		  cout<<"parsed_date" <<parsed_date<<"parsed_logname"<<parsed_logname\
				  <<"parsed_user_ip"<<parsed_user_ip<<"parsed_cmd"<<parsed_cmd<<endl;

		  time_t starttime= str_to_time_t(audit_task->starttime, "%Y-%m-%d %H:%M:%S");
		  time_t time_parsed_time= str_to_time_t(parsed_date, "%Y-%m-%d %H:%M:%S");
		  cout<<"starttime"<<starttime<<"time_parsed_time"<<time_parsed_time<<endl;
		  if(time_parsed_time>= starttime){
			  struct UploadAuditData tmp_upload_audit_data;
			  tmp_upload_audit_data.date= parsed_date;
			  tmp_upload_audit_data.ip= parsed_user_ip;
			  tmp_upload_audit_data.username= parsed_logname;
			  tmp_upload_audit_data.cmd= parsed_cmd;
			  tmp_upload_audit_data.server_ip= audit_task->ip.c_str();
			  result_list.push_back(tmp_upload_audit_data);
			  cout<<"Added"<<endl;
		  }

	  }else{
		  gLogger->error("parse json error {}",i);
	  }
    }

    // 上传数据
    // 如果获取数据为0 不上报
      if(0==result_list.size()){
    	  gLogger->info("upload_audit_data 0==result_list not execute");
    	  return true;
      }

      // for(auto iter=result_list.begin(); iter != result_list.end(); iter++){
      //   iter->print();
      // }

    	// 上传数据到平台
    	string audit_data="";
    	Json::Value json_root;
    	for(auto data:result_list){
    		Json::Value json_data;
    		json_data["date"]= Json::Value(data.date);
    		json_data["ip"]= Json::Value(data.ip);
    		json_data["username"]= Json::Value(data.username);
    		json_data["cmd"]= Json::Value(data.cmd);
    		json_data["serverIp"]= Json::Value(data.server_ip);

    		cout<<"date:" <<data.date<<" ip:"<<data.ip\
    				<<" username:"<<data.username<<" cmd:"<<data.cmd<<" serverIp:"<<data.server_ip<<endl;
    		json_root.append(Json::Value(json_data));
    	}
    //		ret_str= json_root.toStyledString();
    	Json::FastWriter writer;
    	audit_data = writer.write(json_root);
    	gLogger->info("audit_data:{}", audit_data);
    	audit_data_to_platform(audit_data);

    input_file.close();

	return true;
}

void AuditTaskMonitor::createThread(){
	//	m_thread = std::thread(thread_proc, stoi(gConfig->host_port));
	auto lambda_fun=[&]()->void{
		while(!this->m_destroy_flag){
			std::shared_ptr<AuditTask> task;
			if(this->m_audit_task_queue->waitTask(1000, task) == false){
				continue;
			}

			// 业务逻辑
			gLogger->info("AuditTaskMonitor::createThread waitTask {} {}", task->ip, task->username);

			do_audit_ssh(task);

			this->m_audit_task_queue->deleteTask(task);
		}
	};
	m_thread = std::thread(lambda_fun);
}

void AuditTaskMonitor::destroyThread(){
    m_thread.detach();
    pthread_cancel(m_thread.native_handle());

}

void AuditTaskMonitor::start(AuditTaskQueue* audit_task_queue){
	m_audit_task_queue= audit_task_queue;
	createThread();
}

void AuditTaskMonitor::stop(){
	m_destroy_flag=true;
	// 停止线程 本线程 及monitor线程
	destroyThread();
}

void AuditTaskMonitor::thread_proc(){
	while(!m_destroy_flag){
		std::shared_ptr<AuditTask> task;
		if(m_audit_task_queue->waitTask(1000, task) == false){
			continue;
		}

		m_audit_task_queue->deleteTask(task);
	}
}
