#include "Global.h"
#include "Micro_base.h"
int GetProcessIdByHandle(HANDLE ProcessID) {
	return (int)GetProcessId(ProcessID);
}
int GetThreadIdByHandle(HANDLE ProcessID) {
	return (int)GetThreadId(ProcessID);
}

BOOL isExistProcess(DWORD process_id)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return NULL;
	}
	PROCESSENTRY32 pe = { sizeof(pe) };
	for (BOOL ret = Process32First(hSnapshot, &pe); ret; ret = Process32Next(hSnapshot, &pe)) {

		if (pe.th32ProcessID == process_id)
		{
			return TRUE;
		}
	}
	CloseHandle(hSnapshot);
	return FALSE;
}

DWORD GetProcessidFromName(LPCTSTR name)
{
	PROCESSENTRY32 pe;
	DWORD id = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
			break;
		if (lstrcmp(pe.szExeFile, name) == 0)
		{

			id = pe.th32ProcessID;
			break;
		}
	}
	CloseHandle(hSnapshot);
	return id;
}

//启动进程
bool StartPrcess(std::string strProcessName, std::string parameters, std::string env, bool show, HANDLE& ProcessID)
{
	CHAR tszProcess[256] = { 0 };
	lstrcpyA(tszProcess, strProcessName.c_str());
	DWORD dwSessionID = 0;
	HANDLE hToken = NULL;
	bool GetSession = true;
	// 获得当前Session ID
	dwSessionID = ::WTSGetActiveConsoleSessionId();
	// 获得当前Session的用户令牌
	if (FALSE == ::WTSQueryUserToken(dwSessionID, &hToken))
	{
		PLOG::logs->PLOGError("WTSQueryUserToken");
		GetSession = false;
	}
	CloseHandle(hToken);
	if (GetSession)
	{
		return CreateUserProcess_USER(strProcessName.c_str(), parameters.c_str(), env.c_str(), ProcessID, show);
	}

	//启动程序
	SHELLEXECUTEINFOA shellInfo;
	memset(&shellInfo, 0, sizeof(SHELLEXECUTEINFOA));
	shellInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	shellInfo.hwnd = NULL;
	shellInfo.lpVerb = NULL;
	shellInfo.lpFile = tszProcess;						// 执行的程序名(绝对路径)
	shellInfo.lpParameters = parameters.c_str();
	shellInfo.lpDirectory = env.c_str();
	shellInfo.nShow = show ? SW_SHOWNORMAL : SW_HIDE;						//SW_SHOWNORMAL 全屏显示这个程序
	shellInfo.hInstApp = NULL;
	if (ShellExecuteExA(&shellInfo))
	{
		ProcessID = shellInfo.hProcess;
		return true;
	}
	else
		return false;
}

void string_replace(std::string& strBig, const std::string& strsrc, const std::string& strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}
/*
提升进程权限
*/
bool improvePv()
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken)) return false;
	if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid)) return false;
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, NULL, NULL, NULL)) return false;
	return true;
}

/*
关机
*/
bool powerOffProc()
{
	if (!improvePv() || !ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, SHTDN_REASON_MAJOR_APPLICATION)) return false;
	return true;
}

/*
注销
*/
bool logOffProc()
{
	if (!improvePv() || !ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, SHTDN_REASON_MAJOR_APPLICATION)) return false;
	return true;
}

/*
重启
*/
bool reBootProc()
{
	if (!improvePv() || !ExitWindowsEx(EWX_REBOOT | EWX_FORCE, SHTDN_REASON_MAJOR_APPLICATION)) return false;
	return true;
}

int split(std::string pszSrc, const char* flag, std::vector<std::string>& vecDat)
{
	if (pszSrc.empty() || !flag)
		return -1;

	std::string strContent, strTemp;
	strContent = pszSrc;
	std::string::size_type nBeginPos = 0, nEndPos = 0;
	while (true)
	{
		nEndPos = strContent.find(flag, nBeginPos);
		if (nEndPos == std::string::npos)
		{
			strTemp = strContent.substr(nBeginPos, strContent.length());
			if (!strTemp.empty())
			{
				vecDat.push_back(strTemp);
			}
			break;
		}
		strTemp = strContent.substr(nBeginPos, nEndPos - nBeginPos);
		nBeginPos = nEndPos + strlen(flag);
		vecDat.push_back(strTemp);
	}
	return vecDat.size();
}

std::vector<std::wstring> ws_split(const std::wstring& in, const std::wstring& delim) {
	std::wregex re{ delim };
	return std::vector<std::wstring> {
		std::wsregex_token_iterator(in.begin(), in.end(), re, -1),
			std::wsregex_token_iterator()
	};
}


// 突破SESSION 0隔离创建SYSTEM进程
BOOL CreateUserProcess_SYSTEM(const char* lpszFileName, const char* parameters, const char* env, HANDLE& ProcessID, bool show)
{
	BOOL bRet = TRUE;
	DWORD dwSessionID = 0;
	HANDLE hToken = NULL;
	HANDLE hDuplicatedToken = NULL;
	LPVOID lpEnvironment = NULL;

	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	do
	{
		// 获得当前Session ID
		dwSessionID = ::WTSGetActiveConsoleSessionId();

		// 获得当前Session的用户令牌
		if (FALSE == ::OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
		{
			PLOG::logs->PLOGError("OpenProcessToken");
			bRet = FALSE;
			break;
		}

		// 复制令牌
		if (FALSE == ::DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL,
			SecurityIdentification, TokenPrimary, &hDuplicatedToken))
		{
			PLOG::logs->PLOGError("DuplicateTokenEx");
			bRet = FALSE;
			break;
		}
		if (FALSE == ::SetTokenInformation(hDuplicatedToken, TokenSessionId, &dwSessionID, sizeof(DWORD)))
		{
			PLOG::logs->PLOGError("SetTokenInformation:{}", GetLastError());
			bRet = FALSE;
			break;
		}
		STARTUPINFOA si;
		ZeroMemory(&si, sizeof(STARTUPINFOA));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFOA);
		//const wchar_t *desktopp = L"WinSta0\\Default";
		si.lpDesktop = (char*)"WinSta0\\Default";
		//si.lpDesktop = (LPWSTR)L"WinSta0\\Winlogon";
		si.wShowWindow = show ? SW_SHOWNORMAL : SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW /*|STARTF_USESTDHANDLES*/;

		// 创建用户Session环境
		if (FALSE == ::CreateEnvironmentBlock(&lpEnvironment,
			hDuplicatedToken, FALSE))
		{
			PLOG::logs->PLOGError("CreateEnvironmentBlock");
			bRet = FALSE;
			break;
		}

		SetCurrentDirectoryA(env);
		// 在复制的用户Session下执行应用程序，创建进程
		if (FALSE == ::CreateProcessAsUserA(hDuplicatedToken,
			lpszFileName, (char*)parameters, NULL, NULL, FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
			lpEnvironment, env, &si, &pi))
		{
			int error_code = GetLastError();
			if (error_code == 3)
			{
				PLOG::logs->PLOGError("file path not found");
			}
			else if (error_code == 193)
			{
				PLOG::logs->PLOGError("不是有效的 Win32 应用程序");
			}
			else
			{
				PLOG::logs->PLOGError("error_code:{}", error_code);
			}
			bRet = FALSE;
			break;
		}
	} while (FALSE);
	// 关闭句柄, 释放资源
	if (lpEnvironment)
	{
		::DestroyEnvironmentBlock(lpEnvironment);
	}
	if (hDuplicatedToken)
	{
		::CloseHandle(hDuplicatedToken);
	}
	if (hToken)
	{
		::CloseHandle(hToken);
	}
	ProcessID = pi.hProcess;
	return bRet;
}

// 突破SESSION 0隔离创建用户进程
BOOL CreateUserProcess_USER(const char* lpszFileName, const char* parameters, const char* env, HANDLE& ProcessID, bool show)
{
	BOOL bRet = TRUE;
	DWORD dwSessionID = 0;
	HANDLE hToken = NULL;
	HANDLE hDuplicatedToken = NULL;
	LPVOID lpEnvironment = NULL;
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	do
	{
		// 获得当前Session ID
		dwSessionID = ::WTSGetActiveConsoleSessionId();

		// 获得当前Session的用户令牌
		if (FALSE == ::WTSQueryUserToken(dwSessionID, &hToken))
		{
			PLOG::logs->PLOGError("WTSQueryUserToken");
			bRet = FALSE;
			break;
		}

		// 复制令牌
		if (FALSE == ::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL,
			SecurityIdentification, TokenPrimary, &hDuplicatedToken))
		{
			PLOG::logs->PLOGError("DuplicateTokenEx");
			bRet = FALSE;
			break;
		}

		// 创建用户Session环境
		if (FALSE == ::CreateEnvironmentBlock(&lpEnvironment,
			hDuplicatedToken, FALSE))
		{
			PLOG::logs->PLOGError("CreateEnvironmentBlock");
			bRet = FALSE;
			break;
		}
		STARTUPINFOA si;
		ZeroMemory(&si, sizeof(STARTUPINFOA));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFOA);
		si.lpDesktop = (char*)"WinSta0\\Default";
		si.wShowWindow = show ? SW_SHOWNORMAL : SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW /*|STARTF_USESTDHANDLES*/;

		SetCurrentDirectoryA(env);
		std::string command = std::string(lpszFileName) + " " + std::string(parameters);
		// 在复制的用户Session下执行应用程序，创建进程
		if (FALSE == ::CreateProcessAsUserA(hDuplicatedToken,
			NULL, (char*)command.c_str(), NULL, NULL, FALSE,
			NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT,
			lpEnvironment, env, &si, &pi))
		{
			int error_code = GetLastError();
			if (error_code == 3)
			{
				PLOG::logs->PLOGError("file path not found");
			}
			else if (error_code == 193)
			{
				PLOG::logs->PLOGError("不是有效的 Win32 应用程序");
			}
			else
			{
				PLOG::logs->PLOGError("error_code:{}", error_code);
			}
			bRet = FALSE;
			break;
		}

	} while (FALSE);
	// 关闭句柄, 释放资源
	if (lpEnvironment)
	{
		::DestroyEnvironmentBlock(lpEnvironment);
	}
	if (hDuplicatedToken)
	{
		::CloseHandle(hDuplicatedToken);
	}
	if (hToken)
	{
		::CloseHandle(hToken);
	}
	ProcessID = pi.hProcess;
	return bRet;
}


BOOL IsRunAsAdmin() {
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}
Cleanup:
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}
	return fIsRunAsAdmin;
}
