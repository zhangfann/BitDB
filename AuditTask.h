#ifndef AUDIT_TASK_H
#define AUDIT_TASK_H

#include <string>
using namespace std;

enum AuditTaskStatus{
	norunning,
	running
};

class AuditTask{
public:
    //string client_uuid;
    //string client_auth_str;
	string ip;
	string username;
	AuditTaskStatus task_status;
	string starttime;
};

#endif
