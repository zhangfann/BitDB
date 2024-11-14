#include "ClientAuthList.h"
#include <iostream>
using namespace std;

ClientAuthList* ClientAuthList::m_pSingle = NULL;
ClientAuthList::ClientAuthList(){

}


void ClientAuthList::add(ClientAuthInfo client_auth_info){
	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto& info: m_client_auth_list){
		if(info.client_uuid.compare(client_auth_info.client_uuid) ==0 ){
			// uuid相同时更新
			info.client_auth_str= client_auth_info.client_auth_str;
			return ;
		}
	}
	m_client_auth_list.push_back(client_auth_info);
}

void ClientAuthList::remove(string client_uuid){
	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto iter=m_client_auth_list.begin(); iter!= m_client_auth_list.end(); iter++){
		if(client_uuid.compare(iter->client_uuid) == 0){
			m_client_auth_list.erase(iter);
			break;
		}
	}
}

void ClientAuthList::print_all(){
	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto& info: m_client_auth_list){
		cout<< info.client_uuid <<" "<< info.client_auth_str <<endl;
	}
}

int ClientAuthList::size(){
	const std::lock_guard<std::mutex> lock(m_mutex);
	return  m_client_auth_list.size();
}

bool ClientAuthList::client_auth(string client_uuid, string client_auth_str){

	const std::lock_guard<std::mutex> lock(m_mutex);
	for(auto iter=m_client_auth_list.begin(); iter!=m_client_auth_list.end();iter++){
		if(0==iter->client_uuid.compare(client_uuid)){
			// 存在uuid
			if(0==iter->client_auth_str.compare(client_auth_str)){
				//验证字符串相同
				return true;
			}else{
				// 验证字符串不相同
				return false;
			}
		}
	}

	// 不存在uuid
	return false;
}

ClientAuthList* ClientAuthList::instance(){
    if(m_pSingle == NULL){
        m_pSingle = new ClientAuthList();
    }
    return m_pSingle;
}
