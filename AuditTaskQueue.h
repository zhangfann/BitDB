#ifndef AUDIT_TASK_QUEUE
#define AUDIT_TASK_QUEUE

#include "semaphore.h"
#include "AuditTaskList.h"
#include <memory>
#include <map>
#include <string>
using namespace std;

class AuditTaskQueue{
public:
	void insertTask(std::shared_ptr<AuditTask>& audit_task);
	bool waitTask(int wait_time, std::shared_ptr<AuditTask>& audit_task);

	bool deleteTask(std::shared_ptr<AuditTask>& audit_task);
	void notifyWorkers();
	bool getTask(std::shared_ptr<AuditTask>& audit_task);
private:
	std::mutex m_mutex;
	std::map<string, std::shared_ptr<AuditTask>> m_audit_task_queue;
	semaphore m_worker_notify;
};

#endif
