#pragma once
#include "Packet.h"
#include "Define.h"

#include <vector>
#include <atomic>

class User {
public:
    bool init() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        userSkt = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

        userHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
        auto bIOCPHandle = CreateIoCompletionPort((HANDLE)userSkt, userHandle, (UINT32)0, 0);


        SOCKADDR_IN addr;
        ZeroMemory(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(SERVER_TCP_PORT);
        inet_pton(AF_INET, SERVER_IP, &addr.sin_addr.s_addr);

        if (!connect(userSkt, (SOCKADDR*)&addr, sizeof(addr))) {
            HANDLE recvEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            USER_CONNECT_REQUEST_PACKET ucReq;
            ucReq.PacketId = (UINT16)PACKET_ID::USER_CONNECT_REQUEST;
            ucReq.PacketLength = sizeof(USER_CONNECT_REQUEST_PACKET);
            ucReq.userId = userId;
            ucReq.uuId = "";

            auto sendTCP = new OverlappedTCP;
            ZeroMemory(sendTCP, sizeof(OverlappedTCP));
            sendTCP->wsaBuf.len = sizeof(USER_CONNECT_REQUEST_PACKET);
            sendTCP->wsaBuf.buf = new char[sizeof(USER_CONNECT_REQUEST_PACKET)];
            CopyMemory(sendTCP->wsaBuf.buf, (char*)&ucReq, sizeof(USER_CONNECT_REQUEST_PACKET));
            sendTCP->taskType = TaskType::TCP_SEND;


            USER_CONNECT_RESPONSE_PACKET ucReS;
            ucReq.PacketId = (UINT16)PACKET_ID::USER_CONNECT_RESPONSE;
            ucReq.PacketLength = sizeof(USER_CONNECT_RESPONSE_PACKET);
            ucReq.userId = userId;
            ucReq.uuId = "";

            auto recvTCP = new OverlappedTCP;
            ZeroMemory(recvTCP, sizeof(OverlappedTCP));
            recvTCP->wsaBuf.len = sizeof(USER_CONNECT_REQUEST_PACKET);
            sendTCP->wsaBuf.buf = new char[sizeof(USER_CONNECT_REQUEST_PACKET)];
            CopyMemory(recvTCP->wsaBuf.buf, (char*)&ucReq, sizeof(USER_CONNECT_REQUEST_PACKET));
            recvTCP->taskType = TaskType::TCP_SEND;

            DWORD dwSendBytes = 0;
            DWORD dwRecvBytes = 0;
            int sCheck = WSASend(userSkt,
                &(sendTCP->wsaBuf),
                1,
                &dwSendBytes,
                0,
                (LPWSAOVERLAPPED)sendTCP,
                NULL);

            WSARecv(userSkt, &recvTCP->wsaBuf, 1, &dwRecvBytes, 0, (LPWSAOVERLAPPED) & (recvTCP), NULL);
            sendTCP->wsaOverlapped.hEvent = recvEvent;
            WaitForSingleObject(recvEvent, INFINITE);

            // GET USER UUID
            auto ucReqPacket = reinterpret_cast<USER_CONNECT_RESPONSE_PACKET*>(recvTCP->wsaBuf.buf);

            level = ucReqPacket->level;
            exp = ucReqPacket->currentExp;
            uuId = ucReqPacket->uuId;
        }

        equipment.resize(30);
        consumable.resize(30);
        material.resize(30);
    }

    bool CreateThread() {
        workThread = std::thread([this]() {WorkThread(); });
    }

    void TCPSend() {
        USER_CONNECT_REQUEST_PACKET ucReq;
        ucReq.PacketId = (UINT16)PACKET_ID::USER_CONNECT_REQUEST;
        ucReq.PacketLength = sizeof(USER_CONNECT_REQUEST_PACKET);
        ucReq.userId = userId;
        ucReq.uuId = "";

        auto sendTCP = new OverlappedTCP;
        ZeroMemory(sendTCP, sizeof(OverlappedTCP));
        sendTCP->wsaBuf.len = sizeof(USER_CONNECT_REQUEST_PACKET);
        sendTCP->wsaBuf.buf = new char[sizeof(USER_CONNECT_REQUEST_PACKET)];
        CopyMemory(sendTCP->wsaBuf.buf, (char*)&ucReq, sizeof(USER_CONNECT_REQUEST_PACKET));
        sendTCP->taskType = TaskType::TCP_SEND;

        DWORD dwSendBytes = 0;
        int sCheck = WSASend(userSkt,
            &(sendTCP->wsaBuf),
            1,
            &dwSendBytes,
            0,
            (LPWSAOVERLAPPED)sendTCP,
            NULL);

    }
    void TCPRecv() {
        auto recvTCP = new OverlappedTCP;
        ZeroMemory(recvTCP, sizeof(OverlappedTCP));
        recvTCP->wsaBuf.len = 
        recvTCP->wsaBuf.buf = 
        recvTCP->taskType = TaskType::TCP_RECV;

        DWORD dwSendBytes = 0;
        int sCheck = WSASend(userSkt,
            &(sendTCP->wsaBuf),
            1,
            &dwSendBytes,
            0,
            (LPWSAOVERLAPPED)sendTCP,
            NULL);
    }

    void UDPSend() {

    }

    void UDPRecv() {

    }

    void WorkThread() {
        std::cout << "Start Work Thread" << std::endl;
        LPOVERLAPPED lpOverlapped = NULL;
        DWORD dwIoSize = 0;
        bool gqSucces = TRUE;

        while (WorkRun) {
            gqSucces = GetQueuedCompletionStatus(
                userHandle,
                &dwIoSize,
                nullptr,
                &lpOverlapped,
                INFINITE
            );

            auto overlapped = (OverlappedEx*)lpOverlapped;

            if (overlapped->taskType == TaskType::TCP_RECV) {
                auto tcp = (OverlappedTCP*)lpOverlapped;
                auto pakcetCheck = reinterpret_cast<PACKET_HEADER*>(tcp->wsaBuf.buf);
                
                UINT16 packetId = pakcetCheck->PacketId;

                if (overlapped->wsaOverlapped.hEvent) { // 동기 요청
                    SetEvent(overlapped->wsaOverlapped.hEvent); // 트리거 해제 후 처리

                    delete[] tcp->wsaBuf.buf;
                    delete tcp;

                }
                else { // 비동기 요청
                    delete[] tcp->wsaBuf.buf;
                    delete tcp;
                }

            }
            else if(overlapped->taskType == TaskType::TCP_SEND){
                auto tcp = (OverlappedTCP*)lpOverlapped;
                auto pakcetCheck = reinterpret_cast<PACKET_HEADER*>(tcp->wsaBuf.buf);

                UINT16 packetId = pakcetCheck->PacketId;

                if (overlapped->wsaOverlapped.hEvent) { // 동기 요청
                    SetEvent(overlapped->wsaOverlapped.hEvent); // 트리거 해제 후 처리
                    delete[] tcp->wsaBuf.buf;
                    delete tcp;
                }
                else { // 비동기 요청
                    delete[] tcp->wsaBuf.buf;
                    delete tcp;
                }

            }
            else if(overlapped->taskType == TaskType::UDP_RECV){ // 레이드 몹 hp 동기화 요청
                auto udp = (OverlappedUDP*)lpOverlapped;
                auto hp = reinterpret_cast<unsigned int*>(udp->wsaBuf.buf);
                mobHp.store(*hp);

                delete[] udp->wsaBuf.buf;
                delete udp;
            }
            else if(overlapped->taskType == TaskType::UDP_SEND) {
                auto udp = (OverlappedUDP*)lpOverlapped;
                auto pakcetCheck = reinterpret_cast<PACKET_HEADER*>(udp->wsaBuf.buf);

                UINT16 packetId = pakcetCheck->PacketId;

                delete[] udp->wsaBuf.buf;
                delete udp;
            }
        }
    }

    void RaidStart() {
        while (1) {
            
        }
    }

    void End() {
        WSACleanup();
    }

private:
    bool WorkRun = false;
    HANDLE userHandle;

    SOCKET userSkt;

    std::atomic<uint8_t> level;
    std::atomic<unsigned int> exp;
    std::atomic<unsigned int> mobHp;

    std::string uuId;
    std::string userId = "quokka";

    std::thread workThread;

    std::vector<Equipment> equipment;
    std::vector<Consumable> consumable;
    std::vector<Material> material;
};