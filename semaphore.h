#pragma once
#include<mutex>
#include<condition_variable>
class semaphore {
public:
	semaphore(long count = 0) :count(count) {}
	void wait() {
		//cout << "wait" << endl;
		std::unique_lock<std::mutex>lock(mx);
		cond.wait(lock, [this]() {return count > 0; });
		//cout << "wait:" <<count<< endl;
		--count;
	}
	void signal() {
		
		std::unique_lock<std::mutex>lock(mx);
		++count;
		//cout << "signal:" <<count<< endl;
		cond.notify_one();
	}

private:
	std::mutex mx;
	std::condition_variable cond;
	long count;
};
