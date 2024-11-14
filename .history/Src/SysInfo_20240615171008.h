#ifndef SYS_INFO_H
#define SYS_INFO_H

#include "httplib.h"

#include <string>
#include <vector>

using namespace std;

#include "zerotrust.h"

#include "Config.h"
#include "ClientAuthList.h"
#include "Logger.h"
#include <mutex>

typedef struct MEMPACKED         //储存内存信息
{
  char name1[20];
  unsigned long MemTotal;
  char name2[20];
  unsigned long MemFree;
  char name3[20];
  unsigned long Buffers;
  char name4[20];
  unsigned long Cached;
  char name5[20];
  unsigned long SwapCached;
}MEM_OCCUPY;

typedef struct CPUPACKED         //储存内存信息
{
  char name[20];
  unsigned int user;
  unsigned int nice;
  unsigned int system;
  unsigned int idle;
  unsigned int lowait;
  unsigned int irq;
  unsigned int softirq;
}CPU_OCCUPY;
/*
typedef struct NETPACKED
{
  char name[20];
  unsigned long int rbyte;
  unsigned long int rpacks;
  unsigned long int nocare;
  unsigned long int tbytes;
  unsigned long int tpacks;
}NET_OCCUPY;
*/

typedef struct NETPACKED
{
  char type[20];
  char name[20];
  double rpcks;
  double tpcks;
  double rkbs;
  double tkbs;
  double nocare;
}NET_OCCUPY;

//void sys_msg_to_platform();
string sys_msg_md5_to_platform();

void sys_msg_to_platform(double& cpu, double& mem, float& net_in, float& net_out );
//void get_netspeed(float& net_in, float& net_out);

int simple_add(int a, int b);
#endif
