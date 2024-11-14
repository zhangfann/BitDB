#ifndef CLIENT_AUTH_LIST
#define CLIENT_AUTH_LIST

#include <mutex>
#include <vector>
#include <string>
using namespace std;

class ClientAuthInfo{
public:
    string client_uuid;
    string client_auth_str;
};

class ClientAuthList{
public:
	ClientAuthList();

    std::mutex m_mutex;
    vector<ClientAuthInfo> m_client_auth_list;
    void add(ClientAuthInfo client_auth_info);
    void remove(string client_uuid);
    void print_all();
    int size();
    bool client_auth(string client_uuid, string client_auth_str);

    static ClientAuthList* instance();

    static ClientAuthList* m_pSingle;
};

#define gClientAuthList ClientAuthList::instance()

#endif
