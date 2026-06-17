/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license:
 * https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellAuras.h"
#include "SpellInfo.h"

namespace
{
constexpr char CONFIG_AUTO_GATHER_TRACKING_ENABLE[] = "AutoGatherTracking.Enable";
constexpr uint32 TRACK_RESOURCE_HERBS = uint32(1) << (LOCKTYPE_HERBALISM - 1);
constexpr uint32 TRACK_RESOURCE_MINING = uint32(1) << (LOCKTYPE_MINING - 1);
constexpr uint32 TRACK_RESOURCE_GATHERING_MASK = TRACK_RESOURCE_HERBS | TRACK_RESOURCE_MINING;

bool sAutoGatherTrackingEnabled = false;

bool IsAutoGatherTrackingEnabled()
{
    return sAutoGatherTrackingEnabled;
}

void LoadAutoGatherTrackingConfig()
{
    sAutoGatherTrackingEnabled = sConfigMgr->GetOption<bool>(CONFIG_AUTO_GATHER_TRACKING_ENABLE, false);
}

uint32 BuildAutoGatherTrackingMask(Player const* player)
{
    if (!player || !IsAutoGatherTrackingEnabled())
        return 0;

    uint32 mask = 0;

    if (player->HasSkill(SKILL_HERBALISM))
        mask |= TRACK_RESOURCE_HERBS;

    if (player->HasSkill(SKILL_MINING))
        mask |= TRACK_RESOURCE_MINING;

    return mask;
}

void RefreshAutoGatherTracking(Player* player)
{
    if (!player || !IsAutoGatherTrackingEnabled())
        return;

    uint32 const currentMask = player->GetUInt32Value(PLAYER_TRACK_RESOURCES);
    uint32 const autoMask = BuildAutoGatherTrackingMask(player);
    uint32 const newMask = (currentMask & ~TRACK_RESOURCE_GATHERING_MASK) | autoMask;

    if (newMask != currentMask)
        player->SetUInt32Value(PLAYER_TRACK_RESOURCES, newMask);
}

bool IsResourceTrackingAura(AuraApplication* aurApp)
{
    if (!aurApp || !aurApp->GetBase())
        return false;

    SpellInfo const* spellInfo = aurApp->GetBase()->GetSpellInfo();
    return spellInfo && spellInfo->HasAura(SPELL_AURA_TRACK_RESOURCES);
}
}

class AutoGatherTrackingWorldScript : public WorldScript
{
public:
    AutoGatherTrackingWorldScript()
        : WorldScript("AutoGatherTrackingWorldScript", { WORLDHOOK_ON_AFTER_CONFIG_LOAD })
    {
    }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        LoadAutoGatherTrackingConfig();
    }
};

class AutoGatherTrackingPlayerScript : public PlayerScript
{
public:
    AutoGatherTrackingPlayerScript()
        : PlayerScript("AutoGatherTrackingPlayerScript",
            {
                PLAYERHOOK_ON_LOGIN,
                PLAYERHOOK_ON_UPDATE_ZONE,
                PLAYERHOOK_ON_MAP_CHANGED,
                PLAYERHOOK_ON_SET_SKILL
            })
    {
    }

    void OnPlayerLogin(Player* player) override
    {
        RefreshAutoGatherTracking(player);
    }

    void OnPlayerMapChanged(Player* player) override
    {
        RefreshAutoGatherTracking(player);
    }

    void OnPlayerUpdateZone(Player* player, uint32 /*newZone*/, uint32 /*newArea*/) override
    {
        RefreshAutoGatherTracking(player);
    }

    void OnPlayerSetSkill(Player* player, uint32 skillId, uint32 /*value*/, uint32 /*max*/, uint32 /*step*/, uint32 /*newValue*/) override
    {
        if (skillId == SKILL_HERBALISM || skillId == SKILL_MINING)
            RefreshAutoGatherTracking(player);
    }
};

class AutoGatherTrackingUnitScript : public UnitScript
{
public:
    AutoGatherTrackingUnitScript()
        : UnitScript("AutoGatherTrackingUnitScript", true, { UNITHOOK_ON_AURA_REMOVE })
    {
    }

    void OnAuraRemove(Unit* unit, AuraApplication* aurApp, AuraRemoveMode /*mode*/) override
    {
        if (!IsResourceTrackingAura(aurApp))
            return;

        if (Player* player = unit ? unit->ToPlayer() : nullptr)
            RefreshAutoGatherTracking(player);
    }
};

void AddSC_auto_gather_tracking()
{
    new AutoGatherTrackingWorldScript();
    new AutoGatherTrackingPlayerScript();
    new AutoGatherTrackingUnitScript();
}
