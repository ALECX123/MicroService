#ifndef _GLOBAL_H
#define _GLOBAL_H
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <vector>
#include <functional>
#include <thread>
#include <regex>
DWORD GetProcessidFromName(LPCTSTR name);
int GetProcessIdByHandle(HANDLE ProcessID);
int GetThreadIdByHandle(HANDLE ProcessID);
BOOL isExistProcess(DWORD process_id);
void string_replace(std::string& strBig, const std::string& strsrc, const std::string& strdst);
//启动进程
bool StartPrcess(std::string strProcessName, std::string parameters, std::string env, bool show, HANDLE& ProcessID);
/*提升进程权限*/
bool improvePv();
/*关机*/
bool powerOffProc();
/*注销*/
bool logOffProc();
/*重启*/
bool reBootProc();

int split(std::string pszSrc, const char* flag, std::vector<std::string>& vecDat);
std::vector<std::wstring> ws_split(const std::wstring& in, const std::wstring& delim);

BOOL IsRunAsAdmin();
BOOL CreateUserProcess_USER(const char* lpszFileName, const char* parameters, const char* env, HANDLE& ProcessID, bool show);
BOOL CreateUserProcess_SYSTEM(const char* lpszFileName, const char* parameters, const char* env, HANDLE& ProcessID, bool show);
#endif // _GLOBAL_H
