#pragma once 

class ShowEntityBase {
    public:
    
    bool m_bDeath() {
        auto offset = Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "<m_bDeath>k__BackingField");
        return offset != size_t(-1) ? *(bool *) ((uintptr_t) this + offset) : false;
    }

    bool m_bSameCampType() {
        auto offset = Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "m_bSameCampType");
        return offset != size_t(-1) ? *(bool *) ((uintptr_t) this + offset) : false;
    }
    
    int m_Hp() {
        auto offset = Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "m_Hp");
        return offset != size_t(-1) ? *(int *) ((uintptr_t) this + offset) : 0;
    }
    
    int m_HpMax() {
        auto offset = Il2CppGetFieldOffset("Assembly-CSharp.dll", "Battle", "EntityBase", "m_HpMax");
        return offset != size_t(-1) ? *(int *) ((uintptr_t) this + offset) : 0;
    }
};

class ShowEntity : public ShowEntityBase {
    public:
    
    String *m_RoleName() {
        auto offset = Il2CppGetFieldOffset("Assembly-CSharp.dll", "", "ShowEntity", "m_RoleName");
        return offset != size_t(-1) ? *(String **) ((uintptr_t) this + offset) : nullptr;
    }
};

Vector3 AimEnemy,My;

std::map<std::string, uintptr_t> Fields;
std::map<std::string, uintptr_t> Methods;

static void *GetPlayerSelf(void *thiz){
    auto fn = reinterpret_cast<void *(*)(void *)>(Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "LogicBattleManager", "GetPlayerRealSelf"));
    return fn && thiz ? fn(thiz) : nullptr;
}

static Vector3 Player_position(void *thiz) {
    auto fn = reinterpret_cast<Vector3 (*)(void *)>(Il2CppGetMethodOffset("Assembly-CSharp.dll", "Battle", "LogicFighter", "get_Position"));
    return fn && thiz ? fn(thiz) : Vector3();
}

static int GetCampType(void *thiz,void *entity,bool benemy){
    auto fn = reinterpret_cast<int (*)(void *, void *, bool)>(Il2CppGetMethodOffset("Assembly-CSharp.dll", "", "LogicBattleManager", "GetCampType", 2));
    return fn && thiz && entity ? fn(thiz, entity, benemy) : 0;
}

static int Player_GetTotalHP(void *thiz){
    auto fn = reinterpret_cast<int (*)(void *)>(LogicFighter__GetTotalHp);
    return fn && thiz ? fn(thiz) : 0;
}


