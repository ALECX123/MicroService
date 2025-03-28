#pragma once
#include "Micro_base.h"
struct ProcessInfor
{
	std::string ExeName;
	std::string Parameters;
	std::string Env;
	std::string Show;
	int PID = -1;
	bool Running = false;
};
struct Setting
{
	bool log = false;
};
class TaskManager
{
public:
	TaskManager();
	~TaskManager()
	{}
	void ConfigInit();
	void thread_findprocess();

	char current_proc_path[MAX_PATH] = { 0 };
	std::string current_proc_dir;
	std::vector<ProcessInfor> _process;
	bool autostart = true;
	bool _stop = false;
	std::thread _TaskThread;
	Setting _setting;

	static TaskManager _taskManager;
};

