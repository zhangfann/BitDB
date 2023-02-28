#ifndef AUDIT_TASK_SERVER
#define AUDIT_TASK_SERVER

#include "AuditTaskQueue.h"
#include "AuditTaskMonitor.h"

class AuditTaskServer{

public:
	AuditTaskServer();

	bool start();
	bool stop();
	void thread_proc();

	void createThread();
	void destroyThread();

	void processAndGetAuditTasks(std::vector<shared_ptr<AuditTask>>& currentTasks);
	void executeAuditTasks(std::vector<shared_ptr<AuditTask>>& currentTasks);

protected:
	bool m_destroy_flag;
	AuditTaskQueue* m_audit_task_queue;
	AuditTaskMonitor* m_audit_task_monitor_threads;
	int m_audit_thread_cnt;
	std::thread m_thread;
};


#endif
