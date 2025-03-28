#include "ServiceThread.h"
#include "strings.h"
ServiceThread* ServiceThread::self = nullptr;
ServiceThread::ServiceThread(Service* service)
	: service_(service)
{
	self = this;
	memset(&status_, 0, sizeof(status_));
}
ServiceThread::~ServiceThread()
{
	setStatus(SERVICE_STOPPED);

	if (thread_.joinable())

		thread_.join();
	self = nullptr;
}

void ServiceThread::start()
{
	thread_ = std::thread(std::bind(&ServiceThread::run, this));
}

void ServiceThread::setStatus(DWORD status)
{
	status_.dwServiceType = SERVICE_WIN32;
	status_.dwControlsAccepted = 0;
	status_.dwCurrentState = status;
	status_.dwWin32ExitCode = NO_ERROR;
	status_.dwServiceSpecificExitCode = NO_ERROR;
	status_.dwWaitHint = 0;

	if (status == SERVICE_RUNNING)
	{
		status_.dwControlsAccepted =
			SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE |
			SERVICE_ACCEPT_POWEREVENT;
	}

	if (status != SERVICE_RUNNING && status != SERVICE_STOPPED)
		++status_.dwCheckPoint;
	else
		status_.dwCheckPoint = 0;

	if (!SetServiceStatus(status_handle_, &status_))
	{
		PLOG::logs->PLOGWARN("SetServiceStatus failed status:{}", status);
		return;
	}
}

void ServiceThread::doEvent(EventCallback callback, bool quit)
{
	std::unique_lock lock(event_lock);
	event_processed = false;
	callback();
}

void ServiceThread::run()
{
	SERVICE_TABLE_ENTRYW service_table[2];
	PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	service_table[0].lpServiceName = const_cast<wchar_t*>(asWide(service_->name()));
	service_table[0].lpServiceProc = ServiceThread::serviceMain;
	service_table[1].lpServiceName = nullptr;
	service_table[1].lpServiceProc = nullptr;

	if (!StartServiceCtrlDispatcherW(service_table))
	{
		std::scoped_lock lock(self->startup_lock);

		DWORD error_code = GetLastError();
		if (error_code == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			self->startup_state = State::RUNNING_AS_CONSOLE;
		}
		else
		{
			PLOG::logs->PLOGWARN("StartServiceCtrlDispatcherW failed: {}", error_code);
			self->startup_state = State::ERROR_OCCURRED;
		}

		self->startup_condition.notify_all();
	}
}

DWORD WINAPI ServiceThread::serviceControlHandler(
	DWORD control_code, DWORD event_type, LPVOID event_data, LPVOID /* context */)
{
	PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	switch (control_code)
	{
	case SERVICE_CONTROL_INTERROGATE:
		return NO_ERROR;

	case SERVICE_CONTROL_SESSIONCHANGE:
	{
		PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
		if (!self)
			return NO_ERROR;

		//SessionStatus session_status = static_cast<SessionStatus>(event_type);
		//SessionId session_id =
		//	reinterpret_cast<WTSSESSION_NOTIFICATION*>(event_data)->dwSessionId;

		//self->doEvent(std::bind(
		//	&Service::onSessionEvent, self->service_, session_status, session_id));
	}
	return NO_ERROR;

	case SERVICE_CONTROL_POWEREVENT:
	{
		if (!self)
			return NO_ERROR;

		//self->doEvent(std::bind(&Service::onPowerEvent, self->service_, event_type));
	}
	return NO_ERROR;

	case SERVICE_CONTROL_SHUTDOWN:
		PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	case SERVICE_CONTROL_STOP:
	{
		PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
		if (!self)
			return NO_ERROR;
		PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
		if (control_code == SERVICE_CONTROL_STOP)
			self->setStatus(SERVICE_STOP_PENDING);

		self->doEvent(std::bind(&Service::onStop, self->service_), true);
	}
	return NO_ERROR;

	default:
		return ERROR_CALL_NOT_IMPLEMENTED;
	}
}

void WINAPI ServiceThread::serviceMain(DWORD /* argc */, LPWSTR* /* argv */)
{
	if (!self)
		return;
	PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	// Start creating the MessageLoop instance.
	{
		std::scoped_lock lock(self->startup_lock);
		self->startup_state = State::SERVICE_MAIN_CALLED;
		self->startup_condition.notify_all();
	}

	// Waiting for the completion of the creation.
	{
		std::unique_lock lock(self->startup_lock);

		while (self->startup_state != State::MESSAGE_LOOP_CREATED)
			self->startup_condition.wait(lock);

		self->startup_state = State::RUNNING_AS_SERVICE;
	}

	self->status_handle_ = RegisterServiceCtrlHandlerExW(
		asWide(self->service_->name()), serviceControlHandler, nullptr);

	if (!self->status_handle_)
	{
		PLOG::logs->PLOGWARN("RegisterServiceCtrlHandlerExW failed");
		return;
	}
	PLOG::logs->PLOGWARN("{}:{}", __FILE__, __LINE__);
	self->setStatus(SERVICE_START_PENDING);
	//self->doEvent(std::bind(&Service::onStart, self->service_));
	self->setStatus(SERVICE_RUNNING);
}