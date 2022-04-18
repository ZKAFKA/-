#include "Winsock.h"
#include "windows.h"
#include <iostream>
#include <string>
using namespace std;

//公网服务器地址设置**************************
#define RECV_PORT 12345	//接收端口
#define MAX_LISTEN 10   //最大连接数
//********************************************

#pragma comment(lib, "wsock32.lib")

SOCKET sockListen, sockConnect;
sockaddr_in masterAddr;		//master地址
sockaddr_in monitorAddr;	//monitor地址 

int addrLen;		//地址长度
char fileName[20];	//文件名
char order[20];		//命令
char rbuff[1024];	//接收缓冲区
char sbuff[1024];	//发送缓冲区

//***************函数声明***************
DWORD InitSock();
DWORD connectProcess();
DWORD sendOrder(char* order);
DWORD getFile(SOCKET datatcps, FILE* file);
//***************函数声明***************

DWORD InitSock() {
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "初始化失败" << endl;
		return -1;
	}
	sockListen = socket(AF_INET, SOCK_STREAM, 0);
	if (sockListen == SOCKET_ERROR) {
		cout << "创建失败" << endl;
		WSACleanup();
		return -1;
	}
	masterAddr.sin_family = AF_INET;
	masterAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	masterAddr.sin_port = htons(RECV_PORT);

	if (bind(sockListen, (struct sockaddr FAR*) & masterAddr, sizeof(masterAddr)) == SOCKET_ERROR) {
		cout << "绑定失败" << endl;
		return -1;
	}
	return 1;
}

//发送要执行的命令至服务端
DWORD sendOrder(char data[]) {
	int length = send(sockConnect, data, strlen(data), 0);
	if (length <= 0) {
		cout << "发送命令至服务端失败" << endl;
		closesocket(sockConnect);
		WSACleanup();
		return -1;
	}
	return 1;
}

DWORD connectProcess()
{
	//让套接字进入被动监听状态，参数2为请求队列的最大长度
	if (listen(sockListen, MAX_LISTEN) < 0) {
		cout << "监听失败" << endl;
		return -1;
	}
	cout << "服务器正在监听中…" << endl;
	addrLen = sizeof(SOCKADDR);
	while (1)
	{
		int cnt;
		char buff[50];						//用来存储经过字符串格式化的order
		
		while (TRUE)
		{
			sockConnect = accept(sockListen, (struct sockaddr FAR*) & monitorAddr, &addrLen);
			cout << "连接成功!" << endl;
			if (sockConnect == SOCKET_ERROR) {
				cout << "ERROR" << endl;
				continue;
			}
			//连接成功后进行初始化
			memset(order, 0, sizeof(order));
			memset(buff, 0, sizeof(buff));
			memset(rbuff, 0, sizeof(rbuff));
			memset(sbuff, 0, sizeof(sbuff));

			cout << "请输入执行命令：\n  get 获取日志；\n  clean 删除日志" << endl;
			cin >> order;
			if (strncmp(order, "get", 3) == 0)
			{
				sprintf_s(buff, order);
				sendOrder(buff);
				FILE* file{};
				errno_t x = _wfopen_s(&file, L"log.txt", L"wb+");	//用二进制的方式打开文件，wb表示打开或新建一个二进制文件（只允许写数据）  
				cout << x << endl;
				if (file == NULL) {
					cout << "打开或者新建log文件失败" << endl;
					continue;
				}
				memset(rbuff, '\0', sizeof(rbuff));
				while ((cnt = recv(sockConnect, rbuff, sizeof(rbuff), 0)) > 0) {
					int count = fwrite(rbuff, sizeof(rbuff[0]), cnt, file);
					cout << rbuff << endl;
				}
				fclose(file);								//关闭文件
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
			closesocket(sockConnect);	//关闭连接
		}
	}
	
	WSACleanup();				//释放Winsock
}

int main() {
	InitSock();
	connectProcess();
	return 1;
}
