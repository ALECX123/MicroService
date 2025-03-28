#include "Micro_base.h"
#include "ServiceController.h"
#include "Service_win.h"
ServiceController controller;

int __cdecl _tmain(int argc, TCHAR* argv[])
{
	// If command-line parameter is "install", install the service. 
	// Otherwise, the service is probably being started by the SCM.
	TaskManager::_taskManager.ConfigInit();
	PLOG::initLogging(TaskManager::_taskManager._setting.log);
	SetCurrentDirectoryA(TaskManager::_taskManager.current_proc_dir.c_str());
	PLOG::logs->PLOGINFOR("current_proc_dir:{}", TaskManager::_taskManager.current_proc_dir);
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED | ES_DISPLAY_REQUIRED);

	const wchar_t* path = argv[0];
	PLOG::logs->PLOGINFOR("current_path:{}", wstring2string(path).c_str());
	bool isadmin = IsRunAsAdmin();
	PLOG::logs->PLOGINFOR("IsRunAsAdmin:{}", isadmin);
	char name[1024] = "MicroService";
	if (lstrcmpi(argv[1], TEXT("install")) == 0) {
		if (controller.install(u"MicroService", u"MicroService", string2wstring(TaskManager::_taskManager.current_proc_dir) + string2wstring("\\MicroService.exe")))
		{
			PLOG::logs->PLOGINFOR("install Sucessfully");
		}
		else
		{
			PLOG::logs->PLOGINFOR("install Failed");
		}
	}
	else if (lstrcmpi(argv[1], TEXT("remove")) == 0)
	{
		if (controller.remove(u"MicroService"))
		{
			PLOG::logs->PLOGINFOR("remove Sucessfully");
		}
		else
		{
			PLOG::logs->PLOGINFOR("remove Failed");
		}
	}
	else if (lstrcmpi(argv[1], TEXT("help")) == 0)
	{
		std::cout << "exe [install|remove]" << std::endl;
	}
	else
	{
		Service service(SVCNAME);
		service.exec();
	}
	return 0;
}
