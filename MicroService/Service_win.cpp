#include "Service_win.h"

//important
//install

//reporte

//uninstall
//

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//

Service::Service(std::u16string_view name) :
	name_(name)
{
}

Service::~Service() = default;

void Service::exec()
{
	std::unique_ptr<ServiceThread> service_thread = std::make_unique<ServiceThread>(this);
	// Waiting for the launch ServiceThread::serviceMain.
	{
		std::unique_lock lock(service_thread->startup_lock);
		service_thread->startup_state = ServiceThread::State::NOT_STARTED;

		// Starts handler thread.
		service_thread->start();

		while (service_thread->startup_state == ServiceThread::State::NOT_STARTED)
			service_thread->startup_condition.wait(lock);

		if (service_thread->startup_state == ServiceThread::State::ERROR_OCCURRED)
			return;
	}
	PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	// Now we can complete the registration of the service.
	{
		std::scoped_lock lock(service_thread->startup_lock);
		service_thread->startup_state = ServiceThread::State::MESSAGE_LOOP_CREATED;
		service_thread->startup_condition.notify_all();
	}
	TaskManager::_taskManager._TaskThread = std::thread(std::bind(&TaskManager::thread_findprocess, &TaskManager::_taskManager));

	for (;;)
	{
		PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
		std::unique_lock lock(event_lock);
		event_condition.wait(lock);
		break;
	}
	PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	service_thread.reset();
}

void Service::onStop()
{
	PLOG::logs->PLOGINFOR("Stopping service...");
	event_condition.notify_all();
	TaskManager::_taskManager._stop = true;
	PLOG::logs->PLOGINFOR("Service stopped");
}

void Service::onStart()
{
	PLOG::logs->PLOGINFOR("Starting service...");
}