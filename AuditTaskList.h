#ifndef AUDIT_TASK_LIST
#define AUDIT_TASK_LIST

#include <mutex>
#include <vector>
#include <string>
#include <memory>
using namespace std;
#include "AuditTask.h"
#include "db.h"


class AuditTaskList{
public:
	AuditTaskList();

    std::mutex m_mutex;
    vector<std::shared_ptr<AuditTask>> m_audit_task_list;
    void init();
    void add(string ip, string username);
    void remove(string ip);
    void print_all();
    int size();
    void getAll(std::vector<std::shared_ptr<AuditTask>>& task_list);
//    bool client_auth(string client_uuid, string client_auth_str);

    static AuditTaskList* instance();

    static AuditTaskList* m_pSingle;
};

#define gAuditTaskList AuditTaskList::instance()

#endif
