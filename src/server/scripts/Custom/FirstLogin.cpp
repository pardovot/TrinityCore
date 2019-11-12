#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "DatabaseEnv.h"
#include "ScriptLoader.h"
#include "ScriptedCreature.h"
#include "CreatureAI.h"
#include <iostream>
#include "SQLOperation.h"
#include "Mail.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "string.h"
#include "ObjectMgr.h"

#define JOIN_QUE              "Join que"
#define LEAVE_QUE             "Leave que"
#define CURRENT_STATUS        "Que status"
#define GET_CLASS             "Get class"
#define HEALING_GEAR          "Choose Healing gear"
#define DPS_GEAR              "Choose DPS gear"
#define TANK_GEAR             "Choose Tanking gear"
#define SPEC_DRUID_FERAL      "Choose feral druid gear"
#define SPEC_DRUID_BALANCE    "Choose balance druid gear"

#define MENU_ID               2522

enum Options {
    HEALING_ACTION = 1,
    DPS_ACTION = 2,
    TANK_ACTION = 3,
    SPEC_DRUID_BALANCE_ACTION = 4,
    SPEC_DRUID_FERAL_ACTION = 5,
};

//enum WARRIOR_GEAR {
//    CHEST = 1,
//    FEET = 2,
//    HANDS = 3,
//    HANDS = 4,
//    HEAD = 5,
//    LEGS = 6,
//    SHOULDER = 7,
//    WAIST = 8,
//    WRIST = 9,
//    RING = 10,
//    TRINKET = 11,
//    WEAPON = 12
//};

/*
Classes uint8 values:

    CLASS_NONE = 0, // SKIP
    CLASS_WARRIOR = 1, // TITLE Warrior
    CLASS_PALADIN = 2, // TITLE Paladin
    CLASS_HUNTER = 3, // TITLE Hunter
    CLASS_ROGUE = 4, // TITLE Rogue
    CLASS_PRIEST = 5, // TITLE Priest
    CLASS_DEATH_KNIGHT = 6, // TITLE Death Knight
    CLASS_SHAMAN = 7, // TITLE Shaman
    CLASS_MAGE = 8, // TITLE Mage
    CLASS_WARLOCK = 9, // TITLE Warlock
    CLASS_DRUID = 11 // TITLE Druid
*/

/*
HEALER/TANK/DPS:
paladin - 2
druid - 11

HEALER/DPS:
priest - 5
shaman - 7

TANK/DPS:
warrior - 1
death knight - 6

DPS:
mage
rogue
hunter
warlock
*/

class FirstLogin : public PlayerScript {
private:
    int PALADIN_DPS_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int PALADIN_TANK_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int PALADIN_HEAL_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int DRUID_BALANCE_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int DRUID_FERAL_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int DRUID_TANK_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int DRUID_HEAL_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int PRIEST_DPS_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int PRIEST_HEAL_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int SHAMAN_DPS_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int SHAMAN_HEAL_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int WARRIOR_DPS_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int WARRIOR_TANK_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int DK_DPS_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};
    int DK_TANK_GEAR[9] = {43973, 43972, 43975, 43971, 43974, 43970, 43969, 24252, 41516};

    std::string DB_NAME = "gossip_menu";
    std::string DB_TABLE = "characters";
public:
    FirstLogin() : PlayerScript("FirstLogin") {}

    void OnLogin(Player* player, bool firstLogin) override {
        auto result = CharacterDatabase.PQuery("SELECT " + DB_NAME + " FROM " + DB_TABLE + " WHERE guid=" + std::to_string(player->GetGUID().GetCounter()));
        bool value;
        if(result) {
            value = result->Fetch()[0].GetUInt8();
        }
        player->Say(std::to_string(value), LANG_UNIVERSAL);

        if(value) {
            uint8 playerClass = player->GetClass();

            if(playerClass == 2 || playerClass == 11) {
                SendGossipMenuDHT(player);
            } else if(playerClass == 5 || playerClass == 7) {
                SendGossipMenuHD(player);
            } else if(playerClass == 1 || playerClass == 6) {
                SendGossipMenuTD(player);
            } else {

            }
        }
    }

    bool SendItem(Player* player, uint32 itemId) {
        int32 count = 1;

        // Adding items
        uint32 noSpaceForCount = 0;

        // check space and find places
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
        if(msg != EQUIP_ERR_OK) {                              // convert to possible store amount
            count -= noSpaceForCount;
        }

        if(count == 0 || dest.empty()) {                        // can't add any 
            return false;
        }

        Item* item = player->StoreNewItem(dest, itemId, true, GenerateItemRandomPropertyId(itemId));
        item->SetBinding(true);

        if(count > 0 && item) {
            player->SendNewItem(item, 1, false, true);
        }

        return true;
    }

    void OnGossipSelect(Player* player, uint32 menu_id, uint32 /*sender*/, uint32 action) override {
        if(menu_id != MENU_ID) { // Not the menu coded here? stop.
            return;
        }

        switch(action) {
        case DPS_ACTION:
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);

            // If the class is Druid send another gossip to choose the spec(balance/feral).
            if(player->GetClass() == 11) {
                SelectDruidSpec(player);
            } else {
                switch(player->GetClass()) {

                    // Warrior gear
                case 1:
                    SendWarriorDPSGear(player);
                    break;

                    // Paladin gear.
                case 2:
                    SendPaladinDPSGear(player);
                    break;

                    // Priest gear.
                case 5:
                    SendPriestDPSGear(player);
                    break;

                    // DK gear.
                case 6:
                    SendDKDPSGear(player);
                    break;

                    // Shaman gear.
                case 7:
                    SendShamanDPSGear(player);
                    break;

                }
            }
            break;
        case HEALING_ACTION:
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);

            switch(player->GetClass()) {

                // Paladin gear.
            case 2:
                SendPaladinHealingGear(player);
                break;

                // Priest gear.
            case 5:
                SendPriestHealingGear(player);
                break;

                // Shaman gear.
            case 7:
                SendShamanHealingGear(player);
                break;

                // Druid gear.
            case 11:
                SendDruidHealingGear(player);
                break;
            }
            break;
        case TANK_ACTION:
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);
            switch(player->GetClass()) {

                // Warrior gear.
            case 1:
                SendWarriorTankingGear(player);
                break;

                // Paladin gear.
            case 2:
                SendPaladinTankingGear(player);
                break;

                // DK gear.
            case 6:
                SendDKTankingGear(player);
                break;

                // Druid gear.
            case 11:
                SendDruidTankingGear(player);
                break;
            }

            break;
        case SPEC_DRUID_BALANCE_ACTION:
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);
            SendDruidBalanceGear(player);
            break;
        case SPEC_DRUID_FERAL_ACTION:
            ClearGossipMenuFor(player);
            CloseGossipMenuFor(player);
            SendDruidFeralGear(player);
            break;
        }
    }

    void SendGossipMenuDHT(Player* player) {
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU_ID);
        ClearGossipMenuFor(player);

        // DPS gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, DPS_GEAR, GOSSIP_SENDER_MAIN, DPS_ACTION);
        SendGossipMenuFor(player, DPS_ACTION, player->GetGUID());

        // Healing gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, HEALING_GEAR, GOSSIP_SENDER_MAIN, HEALING_ACTION);
        SendGossipMenuFor(player, HEALING_ACTION, player->GetGUID());

        // Tank gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, TANK_GEAR, GOSSIP_SENDER_MAIN, TANK_ACTION);
        SendGossipMenuFor(player, TANK_ACTION, player->GetGUID());
    }

    void SendGossipMenuHD(Player* player) {
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU_ID);
        ClearGossipMenuFor(player);

        // DPS gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, DPS_GEAR, GOSSIP_SENDER_MAIN, DPS_ACTION);
        SendGossipMenuFor(player, DPS_ACTION, player->GetGUID());

        // Healing gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, HEALING_GEAR, GOSSIP_SENDER_MAIN, HEALING_ACTION);
        SendGossipMenuFor(player, HEALING_ACTION, player->GetGUID());
    }

    void SendGossipMenuTD(Player* player) {
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU_ID);
        ClearGossipMenuFor(player);
        // DPS gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, DPS_GEAR, GOSSIP_SENDER_MAIN, DPS_ACTION);
        SendGossipMenuFor(player, DPS_ACTION, player->GetGUID());

        // Tank gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, TANK_GEAR, GOSSIP_SENDER_MAIN, TANK_ACTION);
        SendGossipMenuFor(player, TANK_ACTION, player->GetGUID());
    }

    void SendGossipMenuDPS(Player* player) {
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU_ID);
        ClearGossipMenuFor(player);
        // DPS gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, DPS_GEAR, GOSSIP_SENDER_MAIN, DPS_ACTION);
        SendGossipMenuFor(player, DPS_ACTION, player->GetGUID());

        // Tank gear
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, TANK_GEAR, GOSSIP_SENDER_MAIN, TANK_ACTION);
        SendGossipMenuFor(player, TANK_ACTION, player->GetGUID());
    }

    void SelectDruidSpec(Player* player) {
        player->PlayerTalkClass->GetGossipMenu().SetMenuId(MENU_ID);
        ClearGossipMenuFor(player);

        // Select druid balance gear.
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, SPEC_DRUID_BALANCE, GOSSIP_SENDER_MAIN, SPEC_DRUID_BALANCE_ACTION);
        SendGossipMenuFor(player, SPEC_DRUID_BALANCE_ACTION, player->GetGUID());

        // Select druid feral gear.
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, SPEC_DRUID_FERAL, GOSSIP_SENDER_MAIN, SPEC_DRUID_FERAL_ACTION);
        SendGossipMenuFor(player, SPEC_DRUID_FERAL_ACTION, player->GetGUID());
    }

    void SetGossipMenuFalse(Player* player) {
        CharacterDatabase.PQuery("UPDATE " + DB_TABLE + " SET " + DB_NAME + "=0 WHERE guid = " + std::to_string(player->GetGUID()));

    }

    bool SendPaladinTankingGear(Player* player) {
        player->Say("Sending paladin tank gear", LANG_UNIVERSAL);
        for(int itemId : PALADIN_TANK_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendPaladinHealingGear(Player* player) {
        player->Say("Sending paladin heal gear", LANG_UNIVERSAL);
        for(int itemId : PALADIN_HEAL_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendPaladinDPSGear(Player* player) {
        player->Say("Sending paladin DPS gear", LANG_UNIVERSAL);
        for(int itemId : PALADIN_DPS_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendDruidHealingGear(Player* player) {
        player->Say("Sending druid heal gear", LANG_UNIVERSAL);
        for(int itemId : DRUID_HEAL_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendDruidTankingGear(Player* player) {
        player->Say("Sending druid tank gear", LANG_UNIVERSAL);
        for(int itemId : DRUID_TANK_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendDruidBalanceGear(Player* player) {
        player->Say("Sending druid DPS gear", LANG_UNIVERSAL);
        for(int itemId : DRUID_BALANCE_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendDruidFeralGear(Player* player) {
        player->Say("Sending druid DPS gear", LANG_UNIVERSAL);
        for(int itemId : DRUID_FERAL_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendPriestHealingGear(Player* player) {
        player->Say("Sending priest heal gear", LANG_UNIVERSAL);
        for(int itemId : PRIEST_HEAL_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendPriestDPSGear(Player* player) {
        player->Say("Sending priest DPS gear", LANG_UNIVERSAL);
        for(int itemId : PRIEST_DPS_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendShamanHealingGear(Player* player) {
        player->Say("Sending shaman heal gear", LANG_UNIVERSAL);
        for(int itemId : SHAMAN_HEAL_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendShamanDPSGear(Player* player) {
        player->Say("Sending shaman DPS gear", LANG_UNIVERSAL);
        for(int itemId : SHAMAN_DPS_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendWarriorTankingGear(Player* player) {
        player->Say("Sending warrior tank gear", LANG_UNIVERSAL);
        for(int itemId : WARRIOR_TANK_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendWarriorDPSGear(Player* player) {
        player->Say("Sending warrior DPS gear", LANG_UNIVERSAL);
        for(int itemId : WARRIOR_DPS_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendDKTankingGear(Player* player) {
        player->Say("Sending DK tank gear", LANG_UNIVERSAL);
        for(int itemId : DK_TANK_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }

    bool SendDKDPSGear(Player* player) {
        player->Say("Sending DK DPS gear", LANG_UNIVERSAL);
        for(int itemId : DK_DPS_GEAR) {
            SendItem(player, itemId);
        }
        SetGossipMenuFalse(player);
        return true;
    }
};

void AddSC_FirstLogin() {
    new FirstLogin();
}
