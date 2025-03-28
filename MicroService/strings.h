#ifndef STRINGS_H
#define STRINGS_H
#include <xstring>
#include "Micro_base.h"
//char 有一个字节表示，wchar_t 宽体字符，由两个字符表示。char16_t，char32_t C++ 11 新增的字符类型，char16_t 占两个字节，char32_t 占四个字节。
inline const wchar_t* asWide(const char16_t* str)//宽字节，占两个字节
{
	return reinterpret_cast<const wchar_t*>(str);
}

inline const wchar_t* asWide(const std::u16string& str)
{
	return reinterpret_cast<const wchar_t*>(str.data());
}

inline const wchar_t* asWide(std::u16string_view str)
{
	return reinterpret_cast<const wchar_t*>(str.data());
}

//Converting a WChar string to a Ansi string
inline std::string WChar2Ansi(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
	if (nLen <= 0) return std::string("");
	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");
	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;
	std::string strTemp(pszDst);
	delete[] pszDst;
	return strTemp;
}
//需要手动释放
inline char* Wchar2Char(const wchar_t* wchar)
{
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wchar, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用
	char* psText; // psText为char*的临时数组，作为赋值给std::string的中间变量
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wchar, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用
	//string Name_first = psText;
	//delete[]psText;// psText的清除
	return psText;
}
//需要手动释放
inline wchar_t* Char2Wchar(const char* c)
{
	int slength = strlen(c) + 1;
	DWORD dwNum = MultiByteToWideChar(CP_OEMCP, NULL, c, slength, 0, 0);
	wchar_t* psText = new wchar_t[dwNum];
	memset(psText, 0, sizeof(WCHAR) * dwNum);
	MultiByteToWideChar(CP_OEMCP, 0, c, slength, psText, dwNum);
	//std::wstring r(buf);
	//delete[] buf;
	return psText;
}

inline std::string Wchar2String(const wchar_t* wchar)
{
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wchar, -1, NULL, 0, NULL, FALSE);// WideCharToMultiByte的运用
	char* psText; // psText为char*的临时数组，作为赋值给std::string的中间变量
	psText = new char[dwNum];
	WideCharToMultiByte(CP_OEMCP, NULL, wchar, -1, psText, dwNum, NULL, FALSE);// WideCharToMultiByte的再次运用
	std::string Name_first = psText;
	delete[]psText;// psText的清除
	return Name_first;
}

inline std::wstring Char2WString(const char* c)
{
	int slength = strlen(c) + 1;
	DWORD dwNum = MultiByteToWideChar(CP_OEMCP, NULL, c, slength, 0, 0);
	wchar_t* psText = new wchar_t[dwNum];
	memset(psText, 0, sizeof(WCHAR) * dwNum);
	MultiByteToWideChar(CP_OEMCP, 0, c, slength, psText, dwNum);
	std::wstring r(psText);
	delete[] psText;
	return r;
}


inline std::string wstring2string(std::wstring wstr)
{
	std::string result;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	if (len <= 0)return result;
	char* buffer = new char[len + 1];
	if (buffer == NULL)return result;
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}
inline std::wstring string2wstring(std::string str)
{
	std::wstring result;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	if (len < 0)return result;
	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == NULL)return result;
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';
	result.append(buffer);
	delete[] buffer;
	return result;
}
#endif