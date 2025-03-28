#ifndef BASE_SERVICE_H
#define BASE_SERVICE_H
#include <memory>
#include <mutex>
#include <Windows.h>
#include <iostream>
#include <xstring>
#include "ServiceThread.h"
#include "base.h"
#include "Micro_base.h"
class Service
{
public:
	Service(std::u16string_view name);
	virtual ~Service();

	void exec();

	const std::u16string& name() const { return name_; }
	std::condition_variable event_condition;
	std::mutex event_lock;
protected:
	friend class ServiceThread;
	void onStart();
	void onStop();
private:
	std::u16string name_;
	DISALLOW_COPY_AND_ASSIGN(Service);
};


#endif // BASE_SERVICE_H
