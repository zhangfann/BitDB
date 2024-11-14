//g++ -g -std=c++11 bitcask.cpp -o bitcask



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

#include "Bitcask.h"

// #include "KvStore.pb.h"
using namespace std;

std::string random_string(size_t length) {
	auto randchar = []() -> char {
		const char charset[] = "0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

std::string GetCurrentTimeStamp(int time_stamp_type = 0) {
	std::chrono::system_clock::time_point now =
			std::chrono::system_clock::now();

	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm *now_tm = std::localtime(&now_time_t);

	char buffer[128];
	strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", now_tm);

	std::ostringstream ss;
	ss.fill('0');

	std::chrono::milliseconds ms;
	std::chrono::microseconds cs;
	std::chrono::nanoseconds ns;

	switch (time_stamp_type) {
	case 0:
		ss << buffer;
		break;
	case 1:
		ms = std::chrono::duration_cast < std::chrono::milliseconds
				> (now.time_since_epoch()) % 1000;
		ss << buffer << ":" << ms.count();
		break;
	case 2:
		ms = std::chrono::duration_cast < std::chrono::milliseconds
				> (now.time_since_epoch()) % 1000;
		cs = std::chrono::duration_cast < std::chrono::microseconds
				> (now.time_since_epoch()) % 1000000;
		ss << buffer << ":" << ms.count() << ":" << cs.count() % 1000;
		break;
	case 3:
		ms = std::chrono::duration_cast < std::chrono::milliseconds
				> (now.time_since_epoch()) % 1000;
		cs = std::chrono::duration_cast < std::chrono::microseconds
				> (now.time_since_epoch()) % 1000000;
		ns = std::chrono::duration_cast < std::chrono::nanoseconds
				> (now.time_since_epoch()) % 1000000000;
		ss << buffer << ms.count() << cs.count() % 1000 << ns.count() % 1000;
		break;
	default:
		ss << buffer;
		break;
	}

	return ss.str();
}

unsigned long long timestamp_now() {
	time_t curtime = time(NULL);
	unsigned long long time = (unsigned long long) curtime;
	return time;
}
string timestamp_now_str() {
	time_t rawtime;
	struct tm *timeInfo;
	time(&rawtime);
	timeInfo = localtime(&rawtime);

	const int TIME_STRING_LENGTH = 20;
	char buffer[TIME_STRING_LENGTH];

	strftime(buffer, TIME_STRING_LENGTH, "%Y%m%d%H%M%S", timeInfo);
	cout << buffer << endl;
	stringstream ss;
	ss << buffer;
	return ss.str();
}

int getAllFilenameFromPath(string path, vector<string> &filename_list) {
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
//        printf ("%s\n", ent->d_name);
			filename_list.push_back(ent->d_name);
		}
		closedir(dir);
	} else {
		/* could not open directory */
		perror("");
		return -1;
	}
	return 0;
}


void DiskValueObject::print() {
	cout << "[DiskValueObject::print] data_length:" << data_length
			<< " timestamp:" << timestamp\
 << " key_length:" << key_length
			<< " value_length:" << value_length << " key:" << key\

			<< " value:" << value << endl;
}

void MemoryValueObject::print() {
	cout << "[MemoryValueObject::print] file_id:" << file_id << " value_pos:"
			<< value_pos << " timestamp:" << timestamp\
 << " key_length:"
			<< key_length << " value_length:" << value_length << " key:"
			<< key\
 << " value:" << value << endl;
}

bool KvStore::open() {
	// @recover 重建内存索引 打开全部日志文件
	// ; 打开新日志文件 准备写入
	// open(m_cur_fd)
	// m_file_list.add(m_cur_fd)
	// m_offset=0

	recover();

	string filename = m_data_dir + "KvStore_" + GetCurrentTimeStamp(3) + ".log";
	//m_cur_file.open(m_cur_filename, ios::out| ios::in);
	//m_cur_file.open(m_cur_filename);
	int fd = ::open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0645);
	if (fd < 0) {
		cout << "[open] file open error" << endl;
		return false;
	}
	m_cur_file.m_file_name = filename;
	m_cur_file.m_fd = fd;

	m_file_list.push_back(m_cur_file);

	m_offset = 0;
	m_value_count = 0;
//    m_cur_file.seekp(0, ios::end);
}

bool KvStore::close() {
	for (auto file : m_file_list) {
		::close(file.m_fd);
	}

}

int KvStore::set(string key, string value) {
//	i: k,v
//	; 写入日志文件
//	MemoryValueObject=crc 时间戳 key长度 value长度 k v
//	m_cur_fd.append(MemoryValueObject=crc 时间戳 key长度 value长度 k v) 追加日志文件
//		;写入文件
//			int数据块总长度 int时间戳 int:keysize char:key int:valuesize char:value
//		;
//			MemoryValueObject先序列化为string
//			然后string顺序写入m_cur_file
//		; 文件偏移
//		value位置= m_offset
//		m_offset=m_offset+MemoryValueObject.size
//	; 写入内存
//	m_keydir.set(k,(file_id=m_fd, value长度, value位置, 时间戳)) 写入内存
//	m_value_count++
//
//	; 文件切换
//	-value位置m_value_count>=100
//		//m_cur_fd.close 不能关闭 m_keydir还有数据需要使用
//		m_cur_fd.open(新的日志文件)
//		m_file_list.add(m_cur_fd)
//		m_offset=0

	auto iter = m_keydir.find(key);
	if (iter != m_keydir.end()) {
//	key已经存在 替换value的值
		m_keydir.erase(key);
//		return -1;
	}

	uint32_t timestamp = timestamp_now();
	uint32_t key_length = key.length();
	uint32_t value_length = value.length();

	//写入日志
	uint32_t data_buf_size = 4 * sizeof(uint32_t) + key.size() + value.size();
	char data_buf[data_buf_size];
	int pos = 0;
//    sprintf(data_buf+pos, "%d", data_buf_size);
	memcpy(data_buf + pos, (char*) &data_buf_size, sizeof(uint32_t));
	pos += sizeof(uint32_t);
//    sprintf(data_buf+pos, "%d", timestamp);
	memcpy(data_buf + pos, (char*) &timestamp, sizeof(uint32_t));
	pos += sizeof(uint32_t);
//    sprintf(data_buf+pos, "%d", key_length);
	memcpy(data_buf + pos, (char*) &key_length, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	memcpy(data_buf + pos, key.data(), key.size());
	pos += key.size();
//    sprintf(data_buf+pos, "%d", value_length);
	memcpy(data_buf + pos, (char*) &value_length, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	memcpy(data_buf + pos, value.data(), value.size());
	pos += value.size();
	// m_cur_file<< output<<endl;
	//!?fd_
//	int offset= m_offset;
	int file_offset = m_offset;
	int64_t cnt = 0;
	while (cnt < data_buf_size) {
		ssize_t n = pwrite(m_cur_file.m_fd, data_buf, data_buf_size - cnt,
				file_offset + cnt);
		if (n > 0) {
			cnt += n;
		} else if (n < 0) {
			if (errno != EINTR) {
				cnt = -1;
				break;
			}
		} else {
			break;
		}
	}
	m_offset.fetch_add(data_buf_size);

	// 写入内存
	MemoryValueObject memory_value_object;
	memory_value_object.file_id = m_cur_file.m_fd;
	memory_value_object.value_length = data_buf_size;
	memory_value_object.timestamp = timestamp;
	memory_value_object.value_pos = file_offset;
	m_keydir.insert(make_pair(key, memory_value_object));
	m_value_count++;
//	memory_value_object.print();


	// 切换文件
	if (m_value_count > 500) {

		compaction();
		m_value_count =0;
	}
//	if (m_value_count > 100) {
//		// close(m_fd);
//		// 重命名老文件
////        string filename="./KvStore_current.log";
////        string new_filename="./KvStore_current"+GetCurrentTimeStamp(3)+".log";
//		string filename = m_data_dir + "KvStore_" + GetCurrentTimeStamp(3)
//				+ ".log";
//		;
////        rename(filename.c_str(), new_filename.c_str());
//
//		int fd = ::open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0645);
//		m_cur_file.m_file_name = filename;
//		m_cur_file.m_fd = fd;
//
//		m_file_list.push_back(m_cur_file);
//		m_offset = 0;
//
//		compaction();
//	}

	return 0;
}

// 没有切换文件 没有compaction 用于测试 @test_compaction
int KvStore::set_no_compaction(string key, string value) {
//	i: k,v
//	; 写入日志文件
//	MemoryValueObject=crc 时间戳 key长度 value长度 k v
//	m_cur_fd.append(MemoryValueObject=crc 时间戳 key长度 value长度 k v) 追加日志文件
//		;写入文件
//			int数据块总长度 int时间戳 int:keysize char:key int:valuesize char:value
//		;
//			MemoryValueObject先序列化为string
//			然后string顺序写入m_cur_file
//		; 文件偏移
//		value位置= m_offset
//		m_offset=m_offset+MemoryValueObject.size
//	; 写入内存
//	m_keydir.set(k,(file_id=m_fd, value长度, value位置, 时间戳)) 写入内存
//	m_value_count++
//
//	; 文件切换
//	-value位置m_value_count>=100
//		//m_cur_fd.close 不能关闭 m_keydir还有数据需要使用
//		m_cur_fd.open(新的日志文件)
//		m_file_list.add(m_cur_fd)
//		m_offset=0
	uint32_t timestamp = timestamp_now();
	uint32_t key_length = key.length();
	uint32_t value_length = value.length();

	//写入日志
	uint32_t data_buf_size = 4 * sizeof(uint32_t) + key.size() + value.size();
	char data_buf[data_buf_size];
	int pos = 0;
//    sprintf(data_buf+pos, "%d", data_buf_size);
	memcpy(data_buf + pos, (char*) &data_buf_size, sizeof(uint32_t));
	pos += sizeof(uint32_t);
//    sprintf(data_buf+pos, "%d", timestamp);
	memcpy(data_buf + pos, (char*) &timestamp, sizeof(uint32_t));
	pos += sizeof(uint32_t);
//    sprintf(data_buf+pos, "%d", key_length);
	memcpy(data_buf + pos, (char*) &key_length, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	memcpy(data_buf + pos, key.data(), key.size());
	pos += key.size();
//    sprintf(data_buf+pos, "%d", value_length);
	memcpy(data_buf + pos, (char*) &value_length, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	memcpy(data_buf + pos, value.data(), value.size());
	pos += value.size();
	// m_cur_file<< output<<endl;
	//!?fd_
	int file_offset = m_offset.fetch_add(data_buf_size);
	int64_t cnt = 0;
	while (cnt < data_buf_size) {
		ssize_t n = pwrite(m_cur_file.m_fd, data_buf, data_buf_size - cnt,
				file_offset + cnt);
		if (n > 0) {
			cnt += n;
		} else if (n < 0) {
			if (errno != EINTR) {
				cnt = -1;
				break;
			}
		} else {
			break;
		}
	}

	// 写入内存
	MemoryValueObject memory_value_object;
	memory_value_object.file_id = m_cur_file.m_fd;
	memory_value_object.value_length = data_buf_size;
	memory_value_object.timestamp = timestamp;
	memory_value_object.value_pos = m_offset;
	m_keydir.insert(make_pair(key, memory_value_object));
	m_value_count++;

	// 切换文件
//		if (m_value_count > 100) {
//
//			compaction();
//		}

//	if (m_value_count > 100) {
//		// close(m_fd);
//		// 重命名老文件
////        string filename="./KvStore_current.log";
////        string new_filename="./KvStore_current"+GetCurrentTimeStamp(3)+".log";
//		string filename = m_data_dir + "KvStore_" + GetCurrentTimeStamp(3)
//				+ ".log";
//		;
////        rename(filename.c_str(), new_filename.c_str());
//
//		int fd = ::open(filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0645);
//		m_cur_file.m_file_name = filename;
//		m_cur_file.m_fd = fd;
//
//		m_file_list.push_back(m_cur_file);
//		m_offset = 0;
//
//		compaction();
//	}

	return 0;
}

int KvStore::get(string key, string &value) {
	auto iter = m_keydir.find(key);
	if (iter == m_keydir.end()) {
		return -1;
	}
	auto disk_object = iter->second;

	int file_id = disk_object.file_id;
	int value_pos = disk_object.value_pos;
	int value_length = disk_object.value_length;

	char data_buf[value_length];
	pread(file_id, data_buf, value_length, value_pos);

//	cout<<"[get] -------- value_pos" << value_pos <<endl;
	int pos = 0;
	uint32_t data_size;
//    char data_size_buf[sizeof(int)];
	memcpy(&data_size, data_buf + pos, sizeof(uint32_t));
//    data_size=atoi(data_size_buf);
	pos += sizeof(uint32_t);
//	cout<<"[get] data_size" << data_size <<endl;
	uint32_t timestamp;
//    char timestamp_buf[sizeof(int)];
	memcpy(&timestamp, data_buf + pos, sizeof(uint32_t));
//    timestamp= atoi(timestamp_buf);
	pos += sizeof(uint32_t);
//	cout<<"[get] timestamp" << timestamp <<endl;
	uint32_t key_size;
//    char key_size_buf[sizeof(int)];
	memcpy(&key_size, data_buf + pos, sizeof(uint32_t));
//    key_size= atoi(key_size_buf);
	pos += sizeof(uint32_t);
//	cout<<"[get] key_size" << key_size <<endl;
	// std::shared_ptr<char> key_buf(new char);
	// memcpy(key_buf, data_buf+pos, key_size);
	string key_buf = string(data_buf + pos, key_size);
	pos += key_size;
//	cout<<"[get] key_buf" << key_buf <<endl;
	uint32_t value_size;
//    char value_size_buf[sizeof(int)];
	memcpy(&value_size, data_buf + pos, sizeof(uint32_t));
//    value_size=atoi(value_size_buf);
	pos += sizeof(uint32_t);
//	cout<<"[get] value_size" << value_size <<endl;
	// std::shared_ptr<char> value_buf(new char);
	// memcpy(value_buf, data_buf+pos, value_size);
	string value_buf = string(data_buf + pos, value_size);
	pos += value_size;
//	cout<<"[get] value_buf" << value_buf <<endl;

//	cout << "[get] data_size:"<<data_size << " timestamp:" << timestamp << " key_size:" << key_size << " key_buf:" << key_buf
//			<< " value_size:" << value_size << " value_buf:" << value_buf << endl;

	value = value_buf;
	return 0;
}

int KvStore::rm(string key) {
	auto iter = m_keydir.find(key);
	if (iter != m_keydir.end()) {
		//存在
		m_keydir.erase(key);

		int timestamp = 0;
		int key_length = key.length();
		// int value_length= value.length();
		int value_length = 0;

		//写入日志
		int data_buf_size = 4 * sizeof(uint32_t) + key_length + value_length;
		char data_buf[data_buf_size];
		int pos = 0;
		memcpy(data_buf + pos, (uint32_t*) &data_buf_size, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, (uint32_t*) &timestamp, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, (uint32_t*) &key_length, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, key.data(), key.size());
		pos += key.size();
		memcpy(data_buf + pos, (uint32_t*) &value_length, sizeof(uint32_t));
		pos += sizeof(uint32_t);
//		 memcpy(data_buf+pos, value.data(), value.size());
//		 pos+=value.size();
		// m_cur_file<< output<<endl;
		int file_offset = m_offset.fetch_add(data_buf_size);
		int64_t cnt = 0;
		while (cnt < data_buf_size) {
			ssize_t n = pwrite(m_cur_file.m_fd, data_buf, data_buf_size - cnt,
					file_offset + cnt);
			if (n > 0) {
				cnt += n;
			} else if (n < 0) {
				if (errno != EINTR) {
					cnt = -1;
					break;
				}
			} else {
				break;
			}
		}

	} else {
		return 0;
	}

	return 0;
}

int KvStore::compaction() {
	// log_list 写入新的日志文件的数组
	vector<DiskValueObject> log_list;
	// merged_data_file 新的日志文件
	string merged_data_file_name = m_data_dir + "KvStore_"
			+ GetCurrentTimeStamp(3) + ".log";
	int merged_data_file_id = ::open(merged_data_file_name.c_str(),
			O_RDWR | O_CREAT | O_APPEND, 0645);
	//m_cur_file.open(m_cur_filename, ios::out| ios::in);
	//m_cur_file.open(m_cur_filename);
	m_cur_file.m_file_name = merged_data_file_name;
	m_cur_file.m_fd = merged_data_file_id;
	m_offset=0;
//	m_file_list.push_back(m_cur_file);
	if (merged_data_file_id < 0) {
		return false;
	}
	// m_fd= merged_data_file_id;

	// for data_file in m_file_list:
	//     for (crc 时间戳 key长度 value长度 key value) in data_file: 遍历每个条目
	//         -key不在m_keydir存在 -> key已经被删除
	//         -key在m_keydir存在
	//             -时间戳==m_key_dir[key].时间戳
	//                 ;追加入队列
	//                 log_list.add(crc 时间戳 key长度 value长度 key value)
	//             -时间戳!=m_key_dir[key].时间戳 ->老数据
	for (auto file : m_file_list) {
		vector<DiskValueObject> entryList;
		readLogEntry(file, entryList);
		for (auto entry : entryList) {
			auto foundKey = m_keydir.find(entry.key);
			if (foundKey == m_keydir.end()) {
				//         -key不在m_keydir存在 -> key已经被删除
				cout<<"key:"<<entry.key<<"key不在m_keydir存在: 被删除"<<endl;
				continue;
			} else {
				if (entry.timestamp == foundKey->second.timestamp) {
					log_list.push_back(entry);
					cout<<"key:"<<entry.key<<"内存时间:"<<foundKey->second.timestamp<<"磁盘时间:"<<entry.timestamp<<"key在m_keydir存在 时间相同: 新数据"<<endl;
				}else{
					cout<<"key:"<<entry.key<<"内存时间:"<<foundKey->second.timestamp<<"磁盘时间:"<<entry.timestamp<<"key在m_keydir存在 时间不同: 老数据"<<endl;
				}

			}
		}

	}
	// ; 将log_list序列化入磁盘
	// for log in log_list:
	//     new_value_pos= merged_data_file.append(log)
	//     ; 更新m_keydir
	//     m_keydir[log.key].value位置= new_value_pos
	//     m_keydir[log.key].file_id= new_file_id
	// => merged_data_file
//	int file_offset = 0;
	for (auto entry : log_list) {
		uint32_t timestamp = timestamp_now();
		uint32_t key_length = entry.key_length;
		uint32_t value_length = entry.value_length;

		//写入日志
		uint32_t data_buf_size = 4 * sizeof(uint32_t) + key_length + value_length;
		char data_buf[data_buf_size];
		uint32_t pos = 0;
		memcpy(data_buf + pos, (uint32_t*) &data_buf_size, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, (uint32_t*) &timestamp, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, (int*) &key_length, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, entry.key.data(), entry.key.size());
		pos += entry.key.size();
		memcpy(data_buf + pos, (uint32_t*) &value_length, sizeof(uint32_t));
		pos += sizeof(uint32_t);
		memcpy(data_buf + pos, entry.value.data(), entry.value.size());
		pos += entry.value.size();
		// m_cur_file<< output<<endl;
		int file_offset = m_offset;
		int64_t cnt = 0;
		while (cnt < data_buf_size) {
			ssize_t n = pwrite(m_cur_file.m_fd, data_buf, data_buf_size - cnt,
					file_offset + cnt);
			if (n > 0) {
				cnt += n;
			} else if (n < 0) {
				if (errno != EINTR) {
					cnt = -1;
					break;
				}
			} else {
				break;
			}
		}



		//     ; 更新m_keydir
		//     m_keydir[log.key].value位置= new_value_pos
		//     m_keydir[log.key].file_id= new_file_id
		auto found_key = m_keydir.find(entry.key);
		if (found_key != m_keydir.end()) {
			found_key->second.value_pos = file_offset;
			found_key->second.file_id = m_cur_file.m_fd;
			found_key->second.timestamp = timestamp;
		} else {
			MemoryValueObject memory_value_object;
			memory_value_object.file_id = m_cur_file.m_fd;
			memory_value_object.value_length = data_buf_size;
			memory_value_object.timestamp = timestamp;
			memory_value_object.value_pos = file_offset;
			m_keydir.insert(make_pair(entry.key, memory_value_object));
//			m_value_count++;
		}

		m_offset.fetch_add(data_buf_size);

	}



	// !? 删除旧文件 关闭m_file_list对应项
	for (auto file : m_file_list) {
//		if (file.m_fd != m_cur_file.m_fd) {
//			::close(file.m_fd);
//			remove(file.m_file_name.c_str());
//		}
		::close(file.m_fd);
		remove(file.m_file_name.c_str());
	}
	m_file_list.clear();
	m_file_list.push_back(m_cur_file);

}

void KvStore::readLogEntry(CFile file, vector<DiskValueObject> &result) {
	// 读取数据长度
	// 读取数据
	// 移动下一个偏移 >>读取数据长度

	// 读取数据长度
	int file_offset = 0;
	char data_size_buf[sizeof(uint32_t)];
	int n_char = 0;
	while ((n_char = pread(file.m_fd, data_size_buf, sizeof(uint32_t),
			file_offset)) && n_char > 0) {
		int offset = 0;
		uint32_t data_size;
		memcpy(&data_size, data_size_buf, sizeof(uint32_t));
		char data_buf[data_size];
		pread(file.m_fd, data_buf, data_size, file_offset);
		offset += sizeof(uint32_t);

		// 读取数据
		// int data_size;
		// memcpy(&data_size, data_buf+pos, sizeof(int));
		// offset+=sizeof(int);
		uint32_t timestamp;
//        char timestamp_buf[sizeof(int)];
		memcpy(&timestamp, data_buf + offset, sizeof(uint32_t));
//        timestamp= atoi(timestamp_buf);
		offset += sizeof(uint32_t);
		uint32_t key_size;
//        char key_size_buf[sizeof(uint32_t)];
		memcpy(&key_size, data_buf + offset, sizeof(uint32_t));
//        key_size= atoi(key_size_buf);
		offset += sizeof(uint32_t);
		// std::shared_ptr<char> key_buf(new char);
		// memcpy(key_buf, data_buf+pos, key_size);
		string key_buf = string(data_buf + offset, key_size);
		offset += key_size;
		uint32_t value_size;
//        char value_size_buf[sizeof(uint32_t)];
		memcpy(&value_size, data_buf + offset, sizeof(uint32_t));
//        value_size=atoi(value_size_buf);
		offset += sizeof(uint32_t);
		// std::shared_ptr<char> value_buf(new char);
		// memcpy(value_buf, data_buf+pos, value_size);
		string value_buf = string(data_buf + offset, value_size);
		offset += value_size;

		DiskValueObject disk_value_object;
		disk_value_object.timestamp = timestamp;
		disk_value_object.key_length = key_size;
		disk_value_object.key = key_buf;
		disk_value_object.value_length = value_size;
		disk_value_object.value = value_buf;
		disk_value_object.data_length = offset;
		result.push_back(disk_value_object);

		file_offset += data_size;
	}

}

void KvStore::recover() {
//	for fd in data_dir ; 打开全部data_dir下文件 日志由旧到新打开文件
//		data_file= open(fd)
//		m_fd_list.add(fd)
//		for (crc 时间戳 key长度 value长度 key value) in data_file: 遍历每个条目
//			-key不在m_keydir存在
//				-时间戳为0
//				-时间戳不为0
//					添加入m_keydir
//			-key在m_keydir存在
//				-时间戳为0
//					m_keydir删除
//				-时间戳不为0
//					更新m_keydir
	vector < string > filename_list;
	getAllFilenameFromPath(m_data_dir, filename_list);
	for (auto filename : filename_list) {

		std::size_t found = filename.find("KvStore_");
		if (found != string::npos) {
//			cout << "[recover] recover file:" << filename << endl;
			//		data_file= open(fd)
			//		m_fd_list.add(fd)
			string fullpath_filename = m_data_dir + filename;
			int file_id = ::open(fullpath_filename.c_str(),
					O_RDWR | O_CREAT | O_APPEND, 0645);
			CFile file;
			file.m_file_name = fullpath_filename;
			file.m_fd = file_id;
			m_file_list.push_back(file);

			vector<DiskValueObject> entryList;
			readLogEntry(file, entryList);
			int offset = 0;
			for (auto entry : entryList) {
//				cout << "[recover] read entry" << endl;
//				entry.print();
				auto foundKey = m_keydir.find(entry.key);
				if (foundKey == m_keydir.end()) {
					//			-key不在m_keydir存在
					//				-时间戳为0
					//				-时间戳不为0
					//					添加入m_keydir
					if (foundKey->second.timestamp == 0) {

					} else {
						MemoryValueObject memory_value_object;
						memory_value_object.file_id = file_id;
						memory_value_object.value_length = entry.data_length;
						memory_value_object.timestamp = entry.timestamp;
						memory_value_object.value_pos = offset;
//						cout << "[recover] key不在m_keydir 添加key" << endl;
//						memory_value_object.print();
						m_keydir.insert(
								make_pair(entry.key, memory_value_object));
//						m_value_count++;
					}
				} else {
					//			-key在m_keydir存在
					//				-时间戳为0
					//					m_keydir删除
					//				-时间戳不为0
					//					更新m_keydir
					if (entry.timestamp == 0) {
//						cout << "key在m_keydir存在 时间戳为0 删除key" << entry.key
//								<< endl;
						m_keydir.erase(entry.key);
					} else {
//						cout << "key在m_keydir存在 时间戳不为0 更新key" << entry.key
//								<< endl;
						MemoryValueObject memory_value_object;
						memory_value_object.file_id = file_id;
						memory_value_object.value_length = entry.data_length;
						memory_value_object.timestamp = entry.timestamp;
						memory_value_object.value_pos = offset;
						//memory_value_object.print();
						foundKey->second = memory_value_object;
//						m_keydir.insert(make_pair(foundKey->first, memory_value_object));
//						m_value_count++;
					}

				}
				offset += entry.data_length;

			}
		}

	}
}

//KvStore* KvStore::instance(){
//	static KvStore kv_store;
//	return &kv_store;
//}


//int main() {
////	test_basic_open_set_get(); // 基本打开 写入 获取操作
//	test_open_set_get(); // 写1000个 重新打开读
////	test_compaction(); //// 测试compaction
////	 test_recover();
////	test_readLogEntry(); // 写一个数据 读一个
////	test_readLogEntry_many(); // 写多个数据
//
////	test_readLogEntry_read_many(); // 多个数据读
//
//	return 0;
//}
//
