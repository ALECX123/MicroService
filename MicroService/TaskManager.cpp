#include "TaskManager.h"
#include "SimpleIni.h"
TaskManager TaskManager::_taskManager;
TaskManager::TaskManager()
{

}


void TaskManager::ConfigInit()
{
	::GetModuleFileNameA(NULL, current_proc_path, MAX_PATH);
	current_proc_dir.append(current_proc_path);
	int separator_pos = current_proc_dir.rfind('\\');

	if (std::string::npos == separator_pos)
	{
		current_proc_dir = "";
	}
	else {
		current_proc_dir = current_proc_dir.substr(0, separator_pos);
	}
	CSimpleIniA ini;
	ini.SetUnicode();
	SI_Error rc = ini.LoadFile(std::string(current_proc_dir + "/process.ini").c_str());
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	//https ://blog.csdn.net/cpp_learner/article/details/128780799
	for (const auto& sections_it : sections) {
		if (std::string(sections_it.pItem) == "setting") {
			const CSimpleIniA::TKeyVal* pKeyVal = ini.GetSection(sections_it.pItem);
			if (nullptr != pKeyVal) {
				ProcessInfor _ProcessInfor;
				for (const auto& it : *pKeyVal) {
					if (std::string(it.first.pItem) == "log")
					{
						_setting.log = std::string(it.second) == "true";
					}
				}
			}
		}
		else
		{
			const CSimpleIniA::TKeyVal* pKeyVal = ini.GetSection(sections_it.pItem);
			if (nullptr != pKeyVal) {
				ProcessInfor _ProcessInfor;
				for (const auto& it : *pKeyVal) {
					if (std::string(it.first.pItem) == "ExeName")
					{
						_ProcessInfor.ExeName = it.second;
					}
					else if (std::string(it.first.pItem) == "Parameters")
					{
						_ProcessInfor.Parameters = it.second;

					}
					else if (std::string(it.first.pItem) == "Env")
					{
						_ProcessInfor.Env = it.second;

					}
					else if (std::string(it.first.pItem) == "Show")
					{
						_ProcessInfor.Show = it.second;

					}
				}
				_process.push_back(_ProcessInfor);
			}
		}
	}
}

void TaskManager::thread_findprocess()
{
	PLOG::logs->PLOGINFOR("thread_findprocess Sucessfully");
	int errortime = 0;
	while (!_stop)
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));//5s.
		for (auto& iter : _process) {
			if (iter.PID == -1)
			{
				iter.Running = false;
				break;
			}
			if (isExistProcess(iter.PID))
			{
				iter.Running = true;
			}
			else
			{
				iter.Running = false;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(5));//5s.
		for (auto& iter : _process) {
			if (!iter.Running)
			{
				PLOG::logs->PLOGINFOR("{} {}", iter.ExeName, iter.Parameters);
				HANDLE ProcessID;
				if (!StartPrcess(iter.ExeName, iter.Parameters, iter.Env, iter.Show == "true", ProcessID))
				{
					errortime++;
					PLOG::logs->PLOGError("Start ErrorCode :{} {}", GetLastError());
				}
				else
				{
					PLOG::logs->PLOGError("Start ProcessID :{}", ProcessID);
					iter.PID = GetProcessIdByHandle(ProcessID);
					iter.Running = true;
				}
			}
		}
	}
}