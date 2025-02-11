#pragma once
#pragma once
#define WIN32_LEAN_AND_MEAN 

#include <winsock2.h>
#include <cstdint>

const UINT32 MAX_SOCK = 1024; // Set Max Socket Buf
const UINT32 MAX_RECV_DATA = 8096;

const short MAX_RETRY_COUNT = 3;

enum class TaskType {
	TCP_RECV,
	TCP_SEND,
	UDP_RECV,
	UDP_SEND
};

struct OverlappedEx {
	// 4 bytes
	TaskType taskType; // RECV, SEND
	WSAOVERLAPPED wsaOverlapped;
};

struct OverlappedTCP : OverlappedEx {
	// 16 bytes
	WSABUF wsaBuf; // TCP Buffer
};

struct OverlappedUDP : OverlappedEx {
	// 4 bytes
	int addrSize = sizeof(sockaddr_in);

	// 16 bytes
	WSABUF wsaBuf; // UDP Buffer
	sockaddr_in userAddr;  // Client Ip && Port Info
};