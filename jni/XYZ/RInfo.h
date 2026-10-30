#pragma once

struct PlayerDataZ {
    std::string Name;
    std::string UserID;
    std::string Verified;
    std::string Rank;
    std::string Star;
    std::string grade;
    int uiRank;
    int HeroID;
    int Spell;
    std::string Country;
    std::string Version;
    std::string Gender;
    std::string Elo;
    std::string MPL;
    std::string Ip;
    std::string RoleLevel;
    std::string BattleCount;
};

struct {
    PlayerDataZ PlayerB[10];
    PlayerDataZ PlayerR[10];
} RoomInfoZ;

void RoomInfoList2() {
    void *LogicBattleManager_Instance = nullptr;
    Il2CppGetStaticFieldValue("Assembly-CSharp.dll", "", "LogicBattleManager", "Instance", &LogicBattleManager_Instance);
    if (LogicBattleManager_Instance) {
        int iBattleState = GetBattleState();
        if (iBattleState == 2 || iBattleState == 3) {
            List<uintptr_t> *m_RoomPlayerInfo = GetBattlePlayerInfo();
            if (!m_RoomPlayerInfo) return;
            int PlayerB = 0, PlayerR = 0, iSelfCamp = -1;
            int m_uiID = *(int *)(SystemData_m_uiID);
            for (int i = 0; i < m_RoomPlayerInfo->getSize(); i++) {
                auto roomData = m_RoomPlayerInfo->getItems()[i];
                if (!roomData) continue;
                auto iCamp = *(int *) ((uintptr_t)roomData + RoomData_iCamp2);
                auto lUid = *(int *) ((uintptr_t)roomData + RoomData_lUid2);
                if (lUid != m_uiID) continue;
                iSelfCamp = iCamp;
                break;
            }
            for (int i = 0; i < m_RoomPlayerInfo->getSize(); i++) {
                auto roomData = m_RoomPlayerInfo->getItems()[i];
                if (!roomData) continue;
                auto iCamp = *(int *) ((uintptr_t)roomData + RoomData_iCamp2);
                auto lUid = *(int *) ((uintptr_t)roomData + RoomData_lUid2);
                auto uiZoneId = *(int *) ((uintptr_t)roomData + RoomData_uiZoneId2);
                auto *_sName = *(String **) ((uintptr_t)roomData + RoomData_sName2);
                if (iCamp == iSelfCamp) {
                    if (_sName) {
                        RoomInfoZ.PlayerB[PlayerB].Name = _sName->toString();
                    }
                    RoomInfoZ.PlayerB[PlayerB].UserID = to_string(lUid) + " (" + to_string(uiZoneId) + ")";
                    PlayerB++;
                } else {
                    if (_sName) {
                        RoomInfoZ.PlayerR[PlayerR].Name = _sName->toString();
                    }
                    RoomInfoZ.PlayerR[PlayerR].UserID = to_string(lUid) + " (" + to_string(uiZoneId) + ")";
                    
                    PlayerR++;
                    }
                }
            }
        }
  }  

