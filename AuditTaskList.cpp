#include "AuditTaskList.h"
#include <iostream>
using namespace std;

AuditTaskList* AuditTaskList::m_pSingle = NULL;
AuditTaskList::AuditTaskList(){

}

//从mysql读取
void AuditTaskList::init(){
	vector<AuditTask> result_list;
	get_audit_task(result_list);

	for(auto audit_task :result_list){
		std::shared_ptr<AuditTask> new_audit_task(new AuditTask());
		new_audit_task->ip=audit_task.ip;
		new_audit_task->username= audit_task.username;
		m_audit_task_list.push_back(new_audit_task);
	}

}

void AuditTaskList::add(string ip, string username){
	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto task_ptr: m_audit_task_list){
		if(task_ptr->ip.compare(ip) ==0 ){
			// uuid相同时更新
			task_ptr->username= username;
			return ;
		}
	}

	audit_task_write_to_db(ip, username);

	std::shared_ptr<AuditTask> new_audit_task(new AuditTask());
	new_audit_task->ip=ip;
	new_audit_task->username= username;
	m_audit_task_list.push_back(new_audit_task);
}

void AuditTaskList::remove(string ip){
	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto iter=m_audit_task_list.begin(); iter!= m_audit_task_list.end(); iter++){
		if(ip.compare((*iter)->ip) == 0){
			audit_task_remove_from_db(ip);

			m_audit_task_list.erase(iter);
			break;
		}
	}
}

void AuditTaskList::print_all(){
	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto task_ptr: m_audit_task_list){
		cout<< task_ptr->ip <<" "<< task_ptr->username <<endl;
	}
}

int AuditTaskList::size(){
	const std::lock_guard<std::mutex> lock(m_mutex);
	return  m_audit_task_list.size();
}

void AuditTaskList::getAll(std::vector<std::shared_ptr<AuditTask>>& task_list){

	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto task_ptr: m_audit_task_list){
		std::shared_ptr<AuditTask> tmp(new AuditTask());
		tmp->ip= task_ptr->ip;
		tmp->username= task_ptr->username;
		task_list.push_back(tmp);
	}
}

//bool AuditTaskList::client_auth(string client_uuid, string client_auth_str){
//
//	const std::lock_guard<std::mutex> lock(m_mutex);
//	for(auto iter=m_client_auth_list.begin(); iter!=m_client_auth_list.end();iter++){
//		if(0==iter->client_uuid.compare(client_uuid)){
//			// 存在uuid
//			if(0==iter->client_auth_str.compare(client_auth_str)){
//				//验证字符串相同
//				return true;
//			}else{
//				// 验证字符串不相同
//				return false;
//			}
//		}
//	}
//
//	// 不存在uuid
//	return false;
//}

AuditTaskList* AuditTaskList::instance(){
//    if(m_pSingle == NULL){
//        m_pSingle = new AuditTaskList();
//    }
//    return m_pSingle;
	static AuditTaskList audit_task_list_single;
	return &audit_task_list_single;
}
