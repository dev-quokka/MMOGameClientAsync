#pragma once
#pragma comment(lib, "ws2_32.lib") // ���־󿡼� �������α׷��� �ϱ� ���� ��

#define SERVER_IP "127.0.0.1"
#define SERVER_TCP_PORT 8080
#define PACKET_SIZE 1024

#include "Packet.h"
#include "User.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <thread>
#include <iostream>
#include <string>
#include <vector>

int main() {


        bool outCheck = false;
        outCheck = true;

    while (outCheck) {
        std::cout << std::endl;
        uint8_t select;

        std::cout << "========================" << std::endl;
        std::cout << "===    1. �� ����    ===" << std::endl;
        std::cout << "===    2. �κ��丮   ===" << std::endl;
        std::cout << "===   3. ���̵� ��Ī ===" << std::endl;
        std::cout << "===    4. �α׾ƿ�   ===" << std::endl;
        std::cout << "========================" << std::endl;

        std::cin >> select;

        switch (select) {

        case 1 :{
                int checknum;
                std::cout << "���ϴ� �κ��丮 ��ȣ ������ ���͸� �����ּ���." << std::endl;
                std::cout << "1. ��� " << "2. �Һ� " << "3. ��� " << "4. �ڷΰ���" << std::endl;
                std::cin >> checknum;
                
                if (checknum == 1) {

                }
                else if (checknum == 2) {

                }
                else if (checknum == 3) {

                }

                else break;
            }
        
        case 2:
        {
            char recvBuffer[PACKET_SIZE];
            memset(recvBuffer, 0, PACKET_SIZE);

            RAID_MATCHING_REQUEST rmReq;
            rmReq.PacketId = (UINT16)PACKET_ID::RAID_MATCHING_REQUEST;
            rmReq.PacketLength = sizeof(RAID_MATCHING_REQUEST);
            rmReq.uuId = uuId;

            send(userSkt, (char*)&rmReq, sizeof(rmReq), 0);
            recv(userSkt, recvBuffer, PACKET_SIZE, 0);

            auto rmReqPacket = reinterpret_cast<RAID_MATCHING_RESPONSE*>(recvBuffer);

            if (rmReqPacket->insertSuccess) { // Mathing Success
                recv(userSkt, recvBuffer, PACKET_SIZE, 0);
                auto rrReqPacket = reinterpret_cast<RAID_READY_REQUEST*>(recvBuffer);

                uint8_t timer = rrReqPacket->timer; // Minutes
                uint8_t roomNum = rrReqPacket->roomNum; // If Max RoomNum Up to Short Range, Back to Number One
                uint8_t yourNum = rrReqPacket->yourNum;
                UINT16 udpPort = rrReqPacket->udpPort;   // Server UDP Port Num
                unsigned int mobHp = rrReqPacket->mobHp;
                char serverIP[16]; // Server IP Address
                CopyMemory(serverIP, rrReqPacket->serverIP, sizeof(rrReqPacket->serverIP));
                
                SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
                HANDLE iocp = CreateIoCompletionPort((HANDLE)udpSocket, NULL, (ULONG_PTR)userSkt, 0);

                sockaddr_in serverAddr;
                serverAddr.sin_family = AF_INET;
                serverAddr.sin_port = htons(udpPort);
                inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

                std::cout << "Matching Waitting Start" << std::endl;
            }
            else { // Server Matching Full
                std::cout << "Server Matching Full. Matching Fail" << std::endl;
            }

            std::cout << "�������� ������ �ƹ�Ű�� ������ ���͸� �����ּ���" << std::endl;
            std::string amukey = "";
            std::cin >> amukey;
            break;
        }
        case 3: {
            outCheck = false;
            break;
        }
        }
    }

    End();
}

// ========================== USER STRUCT ==========================

// 1. ������ �α��� �ϸ�, �� ������ �α��� üũ�Ŀ� ui�� �������� ������ �������ش�.
// 2. ���ӽ��� ��ư�� ������ ���������� mysql�� �ش� ���� ���� �����ͼ� ���� Ŭ�����Ϳ� �÷��ֱ� (�ش� ���� �������� ª�� ttl �����ϱ�) 
// ���𽺿� �ش� ���� UUID�� pk����صΰ� ������ ������ userinfo:pk, invenory:pk ������ ����. 

// ���� ������ ������ ó�� ��ٰ� ����. ���ӽ����� ������ ���������� uuid�� �����ؼ� ���� Ŭ�����Ϳ� UUID:"userId", "����uuid", pk������ �����صΰ�
// �������� �ش� �����͸� �����Ŀ� �ٷ� ���� ó��. ���� �ش� uuid�� ������ ���.
// uuid�� ������ ���� ���� �� ������ ���� �������ٰŰ�, ���� ���̵� ���� ������ ���Ƽ� ���ȼ� ����.

// 3. ������ �׻� ������ ����ġ�� ���� Ȯ���� �����ϰ�, { ���, �Һ�, ��� }�� �̷���� �κ��丮�� Ȯ���� �����ϴ�.
// 4. �κ��丮�� ������ �߰�, ������ ����, ������ �̵��� �����ϸ� ��� �������� Ȯ�强���� �ٸ� �����۵�� ������ packet ����
// 5. ������ �� 2������ �̷���� 2��¥�� ���̵尡 �����ϴ�.
// 6. ���ݱ�ɰ� ���� �� hp�� Ȯ�� �����ϸ�, 2������ ���� ������ ������ ����Ǹ� ���� ������ �� ������ Ȯ�� �� �� �ִ�.

// ���߿� �߰� ���� : 
// ���̵� ��Ī �߰� ��� �� ���� ���� ���� ó��
// ���� ���� ó�� (������, ������ ��)


// ========================== REDIS CLUSTER SLOTS STRUCT ==========================
// 
// UUID:quokka, "Made UUID" , quokka pk num
// 
// 1. userInfo / pk,id,level,exp,lastlogin
// 2. inventory:equipment / inventory:pk, type:code:position, enhance...
// 3. inventory:else / inventory:pk, type:code:position, count


// If login success, Insert into Redis pk,id, level, exp,last_login

// Level Up Check
//std::pair<uint8_t, unsigned int> InGameUserManager::LevelUp(UINT16 connObjNum_, unsigned int expUp_) {
//	uint8_t currentLevel = inGmaeUser[connObjNum_]->currentLevel;
//	unsigned int currentExp = inGmaeUser[connObjNum_]->currentExp;
//	uint8_t levelUpCnt = 0;
//
//	if (enhanceProbabilities[currentLevel] <= currentExp + expUp_) { // LEVEL UP
//		currentExp += expUp_;
//		while (currentExp >= enhanceProbabilities[currentLevel]) {
//			currentExp -= enhanceProbabilities[currentLevel];
//			currentLevel++;
//			levelUpCnt++;
//		}
//	}
//	else { // Just Exp Up
//		currentLevel = 0;
//		currentExp = currentExp + expUp_;
//	}
//
//	return { levelUpCnt , currentExp }; // Level Up Increase, currenExp
//}

// �̰� Ŭ���̾�Ʈ�� �Űܼ� �ű⼭ üũ�ϱ�
    //try {
    //    auto existUserInfo = redis.exists("user:" + uuidCheck->uuId); // Check If a UserInfo Value Exists
    //    auto existUserInven = redis.exists("user:" + uuidCheck->uuId); // Check If a UserInventory Value Exists

    //    USERINFO_RESPONSE_PACKET urp;
    //    urp.PacketId = (UINT16)PACKET_ID::USERINFO_RESPONSE;
    //    urp.PacketLength = sizeof(USERINFO_RESPONSE_PACKET);

    //    if (existUserInfo > 0 && existUserInven>0) {
    //        /*urp.userInfo = ;
    //        urp.inventory = ;*/
    //        TempConnUser->PushSendMsg(sizeof(USERINFO_RESPONSE_PACKET), (char*)&urp);
    //    }

    //    else TempConnUser->PushSendMsg(sizeof(USERINFO_RESPONSE_PACKET), (char*)&urp); // Send Nullptr If User Does Not Exist In Redis (Connect Fail)
    //}
    //catch (const sw::redis::Error& e) {
    //    std::cerr << "Redis Error: " << e.what() << std::endl;
    //}

//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/uuid_generators.hpp>
//#include <boost/uuid/uuid_io.hpp>
//#include <iostream>
//#include <sstream>
//
//std::string GenerateCustomUUID(int pk, const std::string& id, int level) {
//    // ���� UUID ����
//    boost::uuids::random_generator generator;
//    boost::uuids::uuid uuid = generator();
//
//    // UUID�� ���� �����͸� ����
//    std::ostringstream oss;
//    oss << to_string(uuid) << "|pk:" << pk << "|id:" << id << "|level:" << level;
//
//    return oss.str();
//}
//
//int main() {
//    // ���� ������
//    int pk = 12345;
//    std::string id = "player123";
//    int level = 10;
//
//    // Ŀ���� UUID ���� �� ���
//    std::string customUUID = GenerateCustomUUID(pk, id, level);
//    std::cout << "Generated Custom UUID: " << customUUID << std::endl;
//
//    return 0;
//}


//<������ �ڵ�>
//void ParseCustomUUID(const std::string& customUUID, int& pk, std::string& id, int& level) {
//    size_t pkPos = customUUID.find("|pk:");
//    size_t idPos = customUUID.find("|id:");
//    size_t levelPos = customUUID.find("|level:");
//
//    if (pkPos == std::string::npos || idPos == std::string::npos || levelPos == std::string::npos) {
//        throw std::runtime_error("Invalid custom UUID format.");
//    }
//
//    // ������ ����
//    pk = std::stoi(customUUID.substr(pkPos + 4, idPos - (pkPos + 4)));
//    id = customUUID.substr(idPos + 4, levelPos - (idPos + 4));
//    level = std::stoi(customUUID.substr(levelPos + 7));
//}

