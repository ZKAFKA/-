#include "Winsock.h"
#include "windows.h"
#include <iostream>
#include <string>
using namespace std;

//������������ַ����**************************
#define RECV_PORT 12345	//���ն˿�
#define MAX_LISTEN 10   //���������
//********************************************

#pragma comment(lib, "wsock32.lib")

SOCKET sockListen, sockConnect;
sockaddr_in masterAddr;		//master��ַ
sockaddr_in monitorAddr;	//monitor��ַ 

int addrLen;		//��ַ����
char fileName[20];	//�ļ���
char order[20];		//����
char rbuff[1024];	//���ջ�����
char sbuff[1024];	//���ͻ�����

//***************��������***************
DWORD InitSock();
DWORD connectProcess();
DWORD sendOrder(char* order);
DWORD getFile(SOCKET datatcps, FILE* file);
//***************��������***************

DWORD InitSock() {
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "��ʼ��ʧ��" << endl;
		return -1;
	}
	sockListen = socket(AF_INET, SOCK_STREAM, 0);
	if (sockListen == SOCKET_ERROR) {
		cout << "����ʧ��" << endl;
		WSACleanup();
		return -1;
	}
	masterAddr.sin_family = AF_INET;
	masterAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	masterAddr.sin_port = htons(RECV_PORT);

	if (bind(sockListen, (struct sockaddr FAR*) & masterAddr, sizeof(masterAddr)) == SOCKET_ERROR) {
		cout << "��ʧ��" << endl;
		return -1;
	}
	return 1;
}

//����Ҫִ�е������������
DWORD sendOrder(char data[]) {
	int length = send(sockConnect, data, strlen(data), 0);
	if (length <= 0) {
		cout << "���������������ʧ��" << endl;
		closesocket(sockConnect);
		WSACleanup();
		return -1;
	}
	return 1;
}

DWORD connectProcess()
{
	//���׽��ֽ��뱻������״̬������2Ϊ������е���󳤶�
	if (listen(sockListen, MAX_LISTEN) < 0) {
		cout << "����ʧ��" << endl;
		return -1;
	}
	cout << "���������ڼ����С�" << endl;
	addrLen = sizeof(SOCKADDR);
	while (1)
	{
		int cnt;
		char buff[50];						//�����洢�����ַ�����ʽ����order
		
		while (TRUE)
		{
			sockConnect = accept(sockListen, (struct sockaddr FAR*) & monitorAddr, &addrLen);
			cout << "���ӳɹ�!" << endl;
			if (sockConnect == SOCKET_ERROR) {
				cout << "ERROR" << endl;
				continue;
			}
			//���ӳɹ�����г�ʼ��
			memset(order, 0, sizeof(order));
			memset(buff, 0, sizeof(buff));
			memset(rbuff, 0, sizeof(rbuff));
			memset(sbuff, 0, sizeof(sbuff));

			cout << "������ִ�����\n  get ��ȡ��־��\n  clean ɾ����־" << endl;
			cin >> order;
			if (strncmp(order, "get", 3) == 0)
			{
				sprintf_s(buff, order);
				sendOrder(buff);
				FILE* file{};
				errno_t x = _wfopen_s(&file, L"log.txt", L"wb+");	//�ö����Ƶķ�ʽ���ļ���wb��ʾ�򿪻��½�һ���������ļ���ֻ����д���ݣ�  
				cout << x << endl;
				if (file == NULL) {
					cout << "�򿪻����½�log�ļ�ʧ��" << endl;
					continue;
				}
				memset(rbuff, '\0', sizeof(rbuff));
				while ((cnt = recv(sockConnect, rbuff, sizeof(rbuff), 0)) > 0) {
					int count = fwrite(rbuff, sizeof(rbuff[0]), cnt, file);
					cout << rbuff << endl;
				}
				fclose(file);								//�ر��ļ�
			}
			else if (strncmp(order, "clean", 5) == 0)
			{
				sprintf_s(buff, order);
				sendOrder(buff);
				recv(sockConnect, rbuff, sizeof(rbuff), 0);
				cout << rbuff << endl;
			}
			else
			{
				cout << "Unrecognised Order" << endl;
			}
			closesocket(sockConnect);	//�ر�����
		}
	}
	
	WSACleanup();				//�ͷ�Winsock
}

int main() {
	InitSock();
	connectProcess();
	return 1;
}
