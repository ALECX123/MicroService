#pragma once
#include <Windows.h>
#include <Service_win.h>
#include <functional>
class Service;
class ServiceThread
{
public:
	ServiceThread(Service* service);
	~ServiceThread();

	using EventCallback = std::function<void()>;

	void start();
	void setStatus(DWORD status);
	void doEvent(EventCallback callback, bool quit = false);

	static ServiceThread* self;

	enum class State
	{
		NOT_STARTED,
		ERROR_OCCURRED,
		SERVICE_MAIN_CALLED,
		MESSAGE_LOOP_CREATED,
		RUNNING_AS_CONSOLE,
		RUNNING_AS_SERVICE
	};

	std::condition_variable startup_condition;
	std::mutex startup_lock;
	State startup_state = State::NOT_STARTED;

	std::condition_variable event_condition;
	std::mutex event_lock;
	bool event_processed = false;

protected:
	// SimpleThread implementation.
	void run();

private:
	static void WINAPI serviceMain(DWORD argc, LPWSTR* argv);
	static DWORD WINAPI serviceControlHandler(
		DWORD control_code, DWORD event_type, LPVOID event_data, LPVOID context);

	std::thread thread_;
	Service* service_;
	SERVICE_STATUS_HANDLE status_handle_ = nullptr;
	SERVICE_STATUS status_;

};
