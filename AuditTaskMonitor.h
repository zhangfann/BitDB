#ifndef AUDIT_TASK_MONITOR
#define AUDIT_TASK_MONITOR

#include "AuditTaskQueue.h"
#include <thread>
#include "config.h"
#include "http_service.h"

#define MAX_STRING_LEN 1024
class AuditTaskMonitor{

public:
	AuditTaskMonitor();

	void start(AuditTaskQueue* audit_task_queue);
	void stop();
	void thread_proc();


private:

	void createThread();
	void destroyThread();

	AuditTaskQueue* m_audit_task_queue;
	bool m_destroy_flag;
	std::thread m_thread;
};

#endif
