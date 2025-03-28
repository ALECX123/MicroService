#ifndef MICRO_BASE_H
#define MICRO_BASE_H
#pragma warning(disable:4996)
#pragma comment(lib, "UserEnv.lib")
#pragma comment(lib, "WtsApi32.lib")
#pragma comment(lib, "advapi32.lib")
#define SVCNAME u"Window-Server"
#define SVC_ERROR   ((DWORD)0xC0020001L)

#include "base.h"
#include <Windows.h>
#include "scoped_object.h"
#include "strings.h"
#include "PLOG.h"
#include "Global.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <vector>
#include <tchar.h>
#include <strsafe.h>
#include <UserEnv.h>
#include <WtsApi32.h>
#include <sstream>
#include <map>
#include "TaskManager.h"
#endif // MICRO_BASE_H
