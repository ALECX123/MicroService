#include "ServiceController.h"

ServiceController::ServiceController()
{

}

ServiceController::ServiceController(SC_HANDLE sc_manager, SC_HANDLE service)
	: sc_manager_(sc_manager),
	service_(service)
{
	// Nothing
}

ServiceController::ServiceController(ServiceController&& other) noexcept
	: sc_manager_(std::move(other.sc_manager_)),
	service_(std::move(other.service_))
{
	// Nothing
}

ServiceController& ServiceController::operator=(ServiceController&& other) noexcept
{
	sc_manager_ = std::move(other.sc_manager_);
	service_ = std::move(other.service_);
	return *this;
}

ServiceController::~ServiceController()
{

}

bool ServiceController::isValid() const
{
	return sc_manager_.isValid() && service_.isValid();
}

bool ServiceController::open(std::u16string_view name)
{
	ScopedScHandle sc_manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!sc_manager.isValid())
	{
		PLOG::logs->PLOGWARN("OpenSCManagerW failed");
		return false;
	}

	ScopedScHandle service(OpenServiceW(sc_manager, asWide(name), SERVICE_ALL_ACCESS));
	if (!service.isValid())
	{
		PLOG::logs->PLOGWARN("OpenServiceW failed");
		return false;
	}
	sc_manager.release();
	service.release();
	return true;
}

bool ServiceController::install(std::u16string_view name,
	std::u16string_view display_name,
	const std::filesystem::path& file_path)
{
	ScopedScHandle sc_manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!sc_manager.isValid())
	{
		PLOG::logs->PLOGWARN("OpenSCManagerW failed");
		return false;
	}

	ScopedScHandle service(CreateServiceW(sc_manager,
		asWide(name),
		asWide(display_name),
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		file_path.c_str(),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr));
	if (!service.isValid())
	{
		PLOG::logs->PLOGWARN("CreateServiceW failed");
		return false;
	}

	SC_ACTION action;
	action.Type = SC_ACTION_RESTART;
	action.Delay = 60000; // 60 seconds

	SERVICE_FAILURE_ACTIONS actions;
	actions.dwResetPeriod = 0;
	actions.lpRebootMsg = nullptr;
	actions.lpCommand = nullptr;
	actions.cActions = 1;
	actions.lpsaActions = &action;

	if (!ChangeServiceConfig2W(service, SERVICE_CONFIG_FAILURE_ACTIONS, &actions))
	{
		PLOG::logs->PLOGWARN("ChangeServiceConfig2W failed");
		return false;
	}

	sc_manager.release();
	service.release();
	return true;
}

bool ServiceController::remove(std::u16string_view name)
{
	ScopedScHandle sc_manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!sc_manager.isValid())
	{
		PLOG::logs->PLOGWARN("OpenSCManagerW failed");
		return false;
	}

	ScopedScHandle service(OpenServiceW(sc_manager, asWide(name), SERVICE_ALL_ACCESS));
	if (!service.isValid())
	{
		PLOG::logs->PLOGWARN("OpenServiceW failed");
		return false;
	}

	if (!DeleteService(service))
	{
		PLOG::logs->PLOGWARN("DeleteService failed");
		return false;
	}

	service.reset();
	sc_manager.reset();

	static const int kMaxAttempts = 15;
	static const int kAttemptInterval = 100; // ms

	for (int i = 0; i < kMaxAttempts; ++i)
	{
		if (!isInstalled(name))
			return true;

		Sleep(kAttemptInterval);
	}

	return false;
}

// static
bool ServiceController::isInstalled(std::u16string_view name)
{
	ScopedScHandle sc_manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));
	if (!sc_manager.isValid())
	{
		PLOG::logs->PLOGWARN("OpenSCManagerW failed");
		return false;
	}

	ScopedScHandle service(OpenServiceW(sc_manager, asWide(name), SERVICE_QUERY_CONFIG));
	if (!service.isValid())
	{
		if (GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST)
		{
			PLOG::logs->PLOGWARN("OpenServiceW failed");
		}

		return false;
	}

	return true;
}

// static
bool ServiceController::isRunning(std::u16string_view name)
{
	ScopedScHandle sc_manager(OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));
	if (!sc_manager.isValid())
	{
		PLOG::logs->PLOGWARN("OpenSCManagerW failed");
		return false;
	}

	ScopedScHandle service(OpenServiceW(sc_manager, asWide(name), SERVICE_QUERY_STATUS));
	if (!service.isValid())
	{
		PLOG::logs->PLOGWARN("OpenServiceW failed");
		return false;
	}

	SERVICE_STATUS status;

	if (!QueryServiceStatus(service, &status))
	{
		PLOG::logs->PLOGWARN("QueryServiceStatus failed");
		return false;
	}

	return status.dwCurrentState != SERVICE_STOPPED;
}

void ServiceController::close()
{
	service_.reset();
	sc_manager_.reset();
}

bool ServiceController::setDescription(std::u16string_view description)
{
	SERVICE_DESCRIPTIONW service_description;
	service_description.lpDescription = const_cast<LPWSTR>(asWide(description));

	// Set the service description.
	if (!ChangeServiceConfig2W(service_, SERVICE_CONFIG_DESCRIPTION, &service_description))
	{
		PLOG::logs->PLOGWARN("ChangeServiceConfig2W failed");
		return false;
	}

	return true;
}
bool ServiceController::start()
{
	if (!StartServiceW(service_, 0, nullptr))
	{
		PLOG::logs->PLOGWARN("StartServiceW failed");
		return false;
	}

	SERVICE_STATUS status;
	if (QueryServiceStatus(service_, &status))
	{
		bool is_started = status.dwCurrentState == SERVICE_RUNNING;
		int number_of_attempts = 0;

		while (!is_started && number_of_attempts < 15)
		{
			Sleep(250);

			if (!QueryServiceStatus(service_, &status))
				break;

			is_started = status.dwCurrentState == SERVICE_RUNNING;
			++number_of_attempts;
		}
	}

	return true;
}

bool ServiceController::stop()
{
	SERVICE_STATUS status;

	if (!ControlService(service_, SERVICE_CONTROL_STOP, &status))
	{
		PLOG::logs->PLOGWARN("ControlService failed");
		return false;
	}

	bool is_stopped = status.dwCurrentState == SERVICE_STOPPED;
	int number_of_attempts = 0;

	while (!is_stopped && number_of_attempts < 15)
	{
		Sleep(250);

		if (!QueryServiceStatus(service_, &status))
			break;

		is_stopped = status.dwCurrentState == SERVICE_STOPPED;
		++number_of_attempts;
	}

	return is_stopped;
}