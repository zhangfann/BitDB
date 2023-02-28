#include "AuditTaskQueue.h"

void AuditTaskQueue::insertTask(std::shared_ptr<AuditTask>& audit_task){
	const std::lock_guard<std::mutex> lock(m_mutex);
//	std::shared_ptr<AuditTask> cur_task= new AuditTask;
//	m_audit_task_queue.push_back(cur_task);
	std::map<string, std::shared_ptr<AuditTask>>::iterator it= m_audit_task_queue.find(audit_task->ip);
	if(it != m_audit_task_queue.end()){
		// 重复 不插入 删除任务
		m_audit_task_queue.erase(it);
	}else{
		m_audit_task_queue.insert(std::make_pair(audit_task->ip, audit_task));
		notifyWorkers();
	}
}

void AuditTaskQueue::notifyWorkers(){
	m_worker_notify.signal();
}

bool AuditTaskQueue::waitTask(int wait_time, std::shared_ptr<AuditTask>& audit_task){
//	if(m_worker_notify.wait()< 0){
//		return false;
//	}

	m_worker_notify.wait();
	return getTask(audit_task);
}

bool AuditTaskQueue::getTask(std::shared_ptr<AuditTask>& audit_task){
	const std::lock_guard<std::mutex> lock(m_mutex);
	std::shared_ptr<AuditTask> cur_task;
	bool isFound= false;
//	m_audit_task_queue.push_back(cur_task);
	std::map<string, std::shared_ptr<AuditTask>>::iterator it= m_audit_task_queue.begin();
	for(;it!=m_audit_task_queue.end();it++){
		if(it->second->task_status==norunning){
			isFound=true;

			cur_task= it->second;
			it->second->task_status=running;
			break;
		}

	}

	if(!isFound){
		return false;
	}

	audit_task= cur_task;
	return true;
}

bool AuditTaskQueue::deleteTask(std::shared_ptr<AuditTask>& audit_task){
	const std::lock_guard<std::mutex> lock(m_mutex);
	//	std::shared_ptr<AuditTask> cur_task= new AuditTask;
	//	m_audit_task_queue.push_back(cur_task);
	std::map<string, std::shared_ptr<AuditTask>>::iterator it= m_audit_task_queue.find(audit_task->ip);
	if(it== m_audit_task_queue.end()){
		return false;
	}

	m_audit_task_queue.erase(it);
	return true;

}
