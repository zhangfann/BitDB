#include "AuditTaskServer.h"
#include "config.h"
#include "logger.h"
#include <vector>
#include <iostream>


AuditTaskServer::AuditTaskServer(){
	m_destroy_flag= false;
}

void AuditTaskServer::createThread(){
	//	m_thread = std::thread(thread_proc, stoi(gConfig->host_port));
	auto lambda_fun=[&]()->void{
		std::vector<std::shared_ptr<AuditTask>> currentTasks;
		while(this->m_destroy_flag != true){
			//TODO 检查任务处理间隔
			int audit_task_interval= gConfig->audit_task_interval;
			sleep(audit_task_interval);
			currentTasks.clear();
			this->processAndGetAuditTasks(currentTasks);
			this->executeAuditTasks(currentTasks);
		}
	};
	m_thread = std::thread(lambda_fun);
}

void AuditTaskServer::destroyThread(){
    m_thread.detach();
    pthread_cancel(m_thread.native_handle());

}

bool AuditTaskServer::start(){
	gLogger->info("AuditTaskServer::start");
	m_audit_task_queue= new AuditTaskQueue();
	int m_audit_thread_cnt= gConfig->audit_thread_cnt;
	m_audit_task_monitor_threads= new AuditTaskMonitor[m_audit_thread_cnt];
	for(int i=0; i< m_audit_thread_cnt;i++){
		m_audit_task_monitor_threads[i].start(m_audit_task_queue);
	}

	createThread();

}

bool AuditTaskServer::stop(){
	m_destroy_flag=true;
	// 停止线程 本线程 及monitor线程
	destroyThread();

	for(int i=0; i< m_audit_thread_cnt; i++){
		m_audit_task_monitor_threads[i].stop();
	}

	if(m_audit_task_queue != NULL){
		delete m_audit_task_queue;
		m_audit_task_queue= NULL;
	}
	if(m_audit_task_monitor_threads!=NULL){
		delete [] m_audit_task_monitor_threads;
		m_audit_task_monitor_threads=NULL;
	}
}

// 真正处理逻辑在上面
void AuditTaskServer::thread_proc(){
	std::vector<std::shared_ptr<AuditTask>> currentTasks;
	while(m_destroy_flag != true){
		//TODO 检查任务处理间隔
		int audit_task_interval= gConfig->audit_task_interval;
		sleep(audit_task_interval);
		this->processAndGetAuditTasks(currentTasks);
		this->executeAuditTasks(currentTasks);
	}
}

void AuditTaskServer::processAndGetAuditTasks(std::vector<shared_ptr<AuditTask>>& currentTasks){
	gLogger->info("AuditTaskServer::processAndGetAuditTasks");
	gAuditTaskList->getAll(currentTasks);

	cout<<"AuditTaskServer::processAndGetAuditTasks"<<endl;
	for(auto iter: currentTasks){
		cout<<"ip"<<iter->ip<<endl;

	}
	cout<<"currentTasks end"<<endl;

}

void AuditTaskServer::executeAuditTasks(std::vector<shared_ptr<AuditTask>>& currentTasks){
	gLogger->info("AuditTaskServer::executeAuditTasks");
	for(auto task: currentTasks){
		task->task_status= norunning;

		time_t now;
		tm* Tm;
		char time_string[20] = {0};

		time(&now);
		Tm = localtime(&now);
		snprintf(time_string, sizeof(time_string),"%4d-%02d-%02d %02d:%02d:%02d",
	            1900 + Tm->tm_year,
	            1 + Tm->tm_mon,
	            Tm->tm_mday,
	            Tm->tm_hour,
	            Tm->tm_min,
				Tm->tm_sec);
		int audit_task_interval= gConfig->audit_task_interval;
		now= now-audit_task_interval;
		Tm = localtime(&now);
		snprintf(time_string, sizeof(time_string),"%4d-%02d-%02d %02d:%02d:%02d",
				1900 + Tm->tm_year,
				1 + Tm->tm_mon,
				Tm->tm_mday,
				Tm->tm_hour,
				Tm->tm_min,
				Tm->tm_sec);
		task->starttime=string(time_string);

		m_audit_task_queue->insertTask(task);
	}


}

/*

 * */
