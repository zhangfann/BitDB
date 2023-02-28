#pragma once

/*
Set(k, v) M:[(k1, v1)] => M:[(k1, v1), (k,v)]
Get(k) M:[(k1, v1), (k,v)] => v
Remove(k) M:[(k1, v1), (k,v)] => M:[(k1, v1)]
compaction()
recover()
*/

#include <string>
#include <map>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <atomic>
#include <cstring>
#include <memory>

// timestamp_now
#include <iostream>
#include <time.h>
#include <sstream>

// 纳秒时间戳
#include <ctime>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>

// 读写文件
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// 获取目录文件名
#include <dirent.h>
#include <cstddef>
#include <cstdio>

// generate_n
#include <algorithm>

using namespace std;

std::string random_string(size_t length);
std::string GetCurrentTimeStamp(int time_stamp_type);
unsigned long long timestamp_now();

class DiskValueObject {
public:
	void print();

	int data_length;
	int timestamp;
	int key_length;
	int value_length;
	string key;
	string value;
};

class MemoryValueObject {
public:
	void print();

	int file_id;
	int value_pos;
	int timestamp;
	int key_length;
	int value_length;
	string key;
	string value;
};


class CFile {
public:
	int m_fd;
	string m_file_name;
};

class KvStore {
public:
	bool open(); // void start();
	bool close();
	int set(string key, string value);
	int set_no_compaction(string key, string value);
	int get(string key, string &value);
	int rm(string key);
	int compaction();
	void recover();
	void readLogEntry(CFile file, vector<DiskValueObject> &result);
	KvStore() {
		m_data_dir = "./KvStore/";
	}
	;
	~KvStore() {
		// m_cur_file.close();

	}
	;

	static KvStore* instance();
//private:
	map<string, MemoryValueObject> m_keydir;
	// char* m_cur_filename;// 文件名称 >> m_cur_file
	// ofstream m_cur_file;
	vector<CFile> m_file_list;
	CFile m_cur_file;
	string m_data_dir; // 日志目录
	// int m_fd; // 文件句柄 >> m_cur_file
	int m_value_count;
	std::atomic<uint64_t> m_offset;
};

#define gBitcaskService KvStore::instance()
