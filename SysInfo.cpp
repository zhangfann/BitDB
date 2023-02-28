#include "SysInfo.h"
#include <openssl/md5.h>

double get_memoccupy() //获取内存信息
{
    FILE *fd;
    char buff[256];
    MEM_OCCUPY mem_stat;

    fd = fopen("/proc/meminfo", "r");

    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", mem_stat.name1, &mem_stat.MemTotal);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", mem_stat.name2, &mem_stat.MemFree);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", mem_stat.name3, &mem_stat.Buffers);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu ", mem_stat.name4, &mem_stat.Cached);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lu", mem_stat.name5, &mem_stat.SwapCached);

    fclose(fd);     //关闭文件fd
	return (mem_stat.MemFree * 1.0 / ( mem_stat.MemTotal * 1.0  ));
}


int get_cpuoccupy(CPU_OCCUPY *cpust) //获取cpu占用率
{
    FILE *fd;
    char buff[256];
    CPU_OCCUPY *cpu_occupy;
    cpu_occupy = cpust;

    fd = fopen("/proc/stat", "r");
    fgets(buff, sizeof(buff), fd);

    sscanf(buff, "%s %u %u %u %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice, &cpu_occupy->system, &cpu_occupy->idle, &cpu_occupy->lowait, &cpu_occupy->irq, &cpu_occupy->softirq);

    fclose(fd);

    return 0;
}

float cal_cpuoccupy()
{
	double cpu_use = 0;
	unsigned long od, nd;
	CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;

	get_cpuoccupy(&cpu_stat1);
	usleep(100000);
	get_cpuoccupy(&cpu_stat2);

    od = (unsigned long)(cpu_stat1.user + cpu_stat1.nice + cpu_stat1.system + cpu_stat1.idle + cpu_stat1.lowait + cpu_stat1.irq + cpu_stat1.softirq);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (unsigned long)(cpu_stat2.user + cpu_stat2.nice + cpu_stat2.system + cpu_stat2.idle + cpu_stat2.lowait + cpu_stat2.irq + cpu_stat2.softirq);//第二次(用户+优先级+系统+空闲)的时间再赋给od
    double sum = nd - od;
    double idle = cpu_stat2.idle - cpu_stat1.idle;
    cpu_use = idle / sum;
    idle = cpu_stat2.user + cpu_stat2.system + cpu_stat2.nice - cpu_stat1.user - cpu_stat1.system - cpu_stat1.nice;
    cpu_use = idle / sum;
    return cpu_use;
}
/*
float get_netoccupy()
{
	FILE * stream;
	char buff[1024] = {0};
	int line_count = 0;
	int index = 0;

	unsigned long int total = 0;

	if((stream = popen("wc -l /proc/net/dev","r")) != nullptr)			//获取文本行数
	{
		if((fgets(buff,sizeof(buff),stream)) != nullptr)
		{
			index = strchr( ( const char*)buff, ' ') - buff;
			buff[index] = 0;
			line_count = atoi(buff);
		}
		memset(buff, 0, sizeof(buff));
	}
	pclose(stream);

	NET_OCCUPY * netdata = new NET_OCCUPY[line_count -3];

	for (size_t i = 0; i < 3; i++)
	{
		fgets( buff,sizeof(buff),stream);
	}

	memset(buff, 0, sizeof(buff));

	for (size_t i = 0; i < line_count - 3; i++)
	{
		if((fgets( buff,sizeof(buff),stream)) != nullptr)
		{
			sscanf(buff, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", netdata[i].name, netdata[i].rbyte, netdata[i].rpacks,
			netdata[i].nocare, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare,
			netdata[i].rbyte, netdata[i].rpacks, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare, netdata[i].nocare);
		}
	}

	for (size_t i = 0; i < line_count - 3; i++)
	{
		total = netdata[i].rbyte + netdata[i].tbytes;
	}

	return (1.0 * total / (2 * line_count));
}
*/

//float * get_netspeed()
void get_netspeed(float& net_in, float& net_out)
{
	FILE * stream;
	char buff[1024];
	int index = 0;
	int line_count = 0;
	int start_line = 0;
	int netdevnum = 0;
	float netspeed[2];

	if((stream = popen("sar -n DEV 1 1 > /etc/netspeed.txt","r")) != nullptr)	//网速信息保存到文本中
	{
		pclose(stream);
	}
	else
	{
		pclose(stream);
		return ;
	}

	if((stream = popen("wc -l /etc/netspeed.txt","r")) != nullptr)			//获取文本行数
	{
		if((fgets(buff,sizeof(buff),stream)) != nullptr)
		{
			index = strchr( ( const char*)buff, ' ') - buff;
			buff[index] = 0;
			line_count = atoi(buff);
		}
		memset(buff, 0, sizeof(buff));
	}
	pclose(stream);

	start_line = (line_count + 9)/2;			//确定从第几行开始存储数据
	netdevnum = line_count - start_line + 1;	//网卡数量

	stream = fopen("/etc/netspeed.txt", "r");

	for (size_t i = 0; i < start_line - 1; i++)			//读取前几行，不取数据
	{
		fgets( buff,sizeof(buff),stream);
	}

	memset(buff, 0, sizeof(buff));

	NET_OCCUPY * netdata = new NET_OCCUPY[netdevnum];

	for (size_t i = 0; i < netdevnum; i++)
	{
		if((fgets( buff,sizeof(buff),stream)) != nullptr)
		{
			sscanf(buff,"%s %s %lf %lf %lf %lf %lf %lf %lf %lf",
			netdata[i].type, netdata[i].name, &netdata[i].rpcks, &netdata[i].tpcks, &netdata[i].rkbs, &netdata[i].tkbs,
			&netdata[i].nocare, &netdata[i].nocare, &netdata[i].nocare, &netdata[i].nocare);
		}
	}
	fclose(stream);

	for (size_t i = 0; i < netdevnum; i++)
	{
		netspeed[0] += netdata[i].rkbs;
		netspeed[1] += netdata[i].tkbs;
	}

	delete [] netdata;
	net_in= netspeed[0];
	net_out= netspeed[1];

//	return netspeed;
	return ;
}

string get_disk_uuid()
{
	FILE* stream;
	string buff;
	char tmp[128] = {0};
	int index = 0, line_count = 0;

	stream = popen("ls -l /dev/disk/by-uuid/ | awk '{print $9}'", "r");

	while( fgets(tmp, sizeof(tmp),stream) != nullptr)
	{
		tmp[strlen(tmp) - 1] = 0;
		buff.append(tmp);
		memset(tmp, 0, sizeof(tmp));
	}
	return buff;
}

string get_cpumsg()
{
	FILE * stream;
	char tmp[128] = {0};
	string buff;
	stream = popen("cat /proc/cpuinfo | grep name | cut -f2 -d: | uniq -c", "r");

	while( fgets(tmp, sizeof(tmp), stream))
	{
		tmp[strlen(tmp) - 1] = 0;
		buff.append(tmp);
		memset(tmp, 0, sizeof(tmp));
	}
	return buff;
}

string getboardmsg()
{
	FILE* stream;
	char tmp[128];
	string buff;

	stream = popen("dmidecode -s system-serial-number", "r");
	while( fgets(tmp, sizeof(tmp), stream))
	{
		tmp[strlen(tmp) - 1] = 0;
		buff.append(tmp);
		memset(tmp, 0, sizeof(tmp));
	}
	cout << buff << endl;
	return buff;
}
// 上报系统信息
void sys_msg_to_platform(double& cpu, double& mem, float& net_in, float& net_out )
{
	double mem_usedata, cpu_usedata;
//	float *netspeed = new float[2];
//	float net_in, net_out;

	mem_usedata = get_memoccupy();//获取内存占用
	cpu_usedata = cal_cpuoccupy();//获取cpu占用
//	netload		= get_netoccupy();//获取网络负载   所有端口发送接收平均值  历史总和
	get_netspeed(net_in, net_out);

//	delete [] netspeed;

	cpu=cpu_usedata;
	mem= mem_usedata;

	printf("cpu:%.3f\nmem:%.3f\n", cpu, mem);
//	printf("rkbs:%.2f\ntkbs:%.2f\n", netspeed[0], netspeed[1]);
	printf("rkbs:%.2f\ntkbs:%.2f\n", net_in, net_out);
}

// 上报系统uuid
string sys_msg_md5_to_platform()
{
	string hardmsg,md5_string;
	MD5_CTX m;
	unsigned char md[16];
	char tmp[33] = {0};

	hardmsg = get_disk_uuid();
	hardmsg.append(get_cpumsg());
	hardmsg.append(getboardmsg());
	cout << hardmsg << endl;

	MD5_Init( &m);
	MD5_Update( &m, hardmsg.c_str(), hardmsg.size());
	MD5_Final( md, &m);

	for (size_t i = 0; i < 16; i++)
	{
		memset( tmp, 0, sizeof(tmp));
		sprintf( tmp, "%02x", md[i]);
		md5_string += tmp;
	}
	return md5_string;
}
