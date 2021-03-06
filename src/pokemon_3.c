#include "global.h"
#include "asm.h"
#include "battle.h"
#include "data2.h"
#include "event_data.h"
#include "hold_effects.h"
#include "item.h"
#include "items.h"
#include "link.h"
#include "main.h"
#include "m4a.h"
#include "pokemon.h"
#include "rng.h"
#include "rom4.h"
#include "rtc.h"
#include "songs.h"
#include "sound.h"
#include "species.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"

#define EVO_FRIENDSHIP       0x0001 // Pokémon levels up with friendship ≥ 220
#define EVO_FRIENDSHIP_DAY   0x0002 // Pokémon levels up during the day with friendship ≥ 220
#define EVO_FRIENDSHIP_NIGHT 0x0003 // Pokémon levels up at night with friendship ≥ 220
#define EVO_LEVEL            0x0004 // Pokémon reaches the specified level
#define EVO_TRADE            0x0005 // Pokémon is traded
#define EVO_TRADE_ITEM       0x0006 // Pokémon is traded while it's holding the specified item
#define EVO_ITEM             0x0007 // specified item is used on Pokémon
#define EVO_LEVEL_ATK_GT_DEF 0x0008 // Pokémon reaches the specified level with attack > defense
#define EVO_LEVEL_ATK_EQ_DEF 0x0009 // Pokémon reaches the specified level with attack = defense
#define EVO_LEVEL_ATK_LT_DEF 0x000a // Pokémon reaches the specified level with attack < defense
#define EVO_LEVEL_SILCOON    0x000b // Pokémon reaches the specified level with a Silcoon personality value
#define EVO_LEVEL_CASCOON    0x000c // Pokémon reaches the specified level with a Cascoon personality value
#define EVO_LEVEL_NINJASK    0x000d // Pokémon reaches the specified level (special value for Ninjask)
#define EVO_LEVEL_SHEDINJA   0x000e // Pokémon reaches the specified level (special value for Shedinja)
#define EVO_BEAUTY           0x000f // Pokémon levels up with beauty ≥ specified value

struct Evolution
{
    u16 method;
    u16 param;
    u16 targetSpecies;
};

struct EvolutionData
{
    struct Evolution evolutions[5];
};

struct SpindaSpot
{
    u8 x, y;
    u16 image[16];
};

extern u8 gPlayerPartyCount;
extern struct Pokemon gPlayerParty[6];
extern u8 gEnemyPartyCount;
extern struct Pokemon gEnemyParty[6];
extern struct BattlePokemon gBattleMons[4];
extern u8 * const gItemEffectTable[];
extern u8 gUnknown_02024A60;
extern struct BattleEnigmaBerry gEnigmaBerries[];
extern struct EvolutionData gEvolutionTable[];
extern u16 gSpeciesToHoennPokedexNum[];
extern u16 gSpeciesToNationalPokedexNum[];
extern u16 gHoennToNationalOrder[];
extern u16 gSpeciesIdToCryId[];
extern u8 gUnknown_030041C0[];
extern u8 gUnknown_03004290[];
extern u8 gUnknown_020238CC[];
extern u8 gPlayerMonIndex;
extern u8 gEnemyMonIndex;
extern u8 gUnknown_02024C0B;
extern u8 gUnknown_02024E6C;
extern struct SpindaSpot gSpindaSpotGraphics[];
extern s8 gNatureStatTable[][5];
extern s8 gUnknown_082082FE[][3];
extern u16 gTrainerBattleOpponent;
extern u16 gBattleTypeFlags;
extern struct BaseStats gBaseStats[];
extern u32 gBitTable[];
extern u32 gExperienceTables[8][101];
extern u32 gTMHMLearnsets[][2];
extern const u16 *gLevelUpLearnsets[];
extern u8 gBattleMonForms[];
extern const u8 BattleText_Wally[];
extern const u16 gHMMoves[];
extern s8 gUnknown_083F7E28[];
extern u8 byte_2024C06;
extern const u8 BattleText_PreventedSwitch[];
extern u16 gUnknown_02024A6A[];
extern u8 gJapaneseNidoranNames[][11];

extern u8 gUnknown_082082F8[];
extern u8 gUnknown_083FFDB3[];
extern u8 gUnknown_083FFDD3[];
extern u8 gUnknown_083FEE5D[];
extern u8 gUnknown_083FEE92[];
extern u8 *gUnknown_08400F58[];

u8 CheckPartyHasHadPokerus(struct Pokemon *, u8);

bool8 HealStatusConditions(struct Pokemon *mon, u32 unused, u32 healMask, u8 battleId)
{
    u32 status = GetMonData(mon, MON_DATA_STATUS, 0);

    if (status & healMask)
    {
        status &= ~healMask;
        SetMonData(mon, MON_DATA_STATUS, (u8 *)&status);
        if (gMain.inBattle && battleId != 4)
            gBattleMons[battleId].status1 &= ~healMask;
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

u8 GetItemEffectParamOffset(u16 itemId, u8 effectByte, u8 effectBit)
{
    u8 *temp;
    u8 *itemEffect;
    u8 offset;
    int i;
    u8 j;
    u8 val;

    offset = 6;

    temp = gItemEffectTable[itemId - 13];

    if (!temp && itemId != ITEM_ENIGMA_BERRY)
        return 0;

    if (itemId == ITEM_ENIGMA_BERRY)
    {
        temp = gEnigmaBerries[gUnknown_02024A60].itemEffect;
    }

    itemEffect = temp;

    for (i = 0; i < 6; i++)
    {
        switch (i)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if (i == effectByte)
                return 0;
            break;
        case 4:
            val = itemEffect[4];
            if (val & 0x20)
                val &= 0xDF;
            j = 0;
            while (val)
            {
                if (val & 1)
                {
                    switch (j)
                    {
                    case 2:
                        if (val & 0x10)
                            val &= 0xEF;
                    case 0:
                        if (i == effectByte && (val & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 1:
                        if (i == effectByte && (val & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 3:
                        if (i == effectByte && (val & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 7:
                        if (i == effectByte)
                            return 0;
                        break;
                    }
                }
                j++;
                val >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        case 5:
            val = itemEffect[5];
            j = 0;
            while (val)
            {
                if (val & 1)
                {
                    switch (j)
                    {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                        if (i == effectByte && (val & effectBit))
                            return offset;
                        offset++;
                        break;
                    case 7:
                        if (i == effectByte)
                            return 0;
                        break;
                    }
                }
                j++;
                val >>= 1;
                if (i == effectByte)
                    effectBit >>= 1;
            }
            break;
        }
    }

    return offset;
}

void sub_803F324(int stat)
{
    gEnemyMonIndex = gUnknown_02024E6C;
    StringCopy(gUnknown_030041C0, gUnknown_08400F58[gUnknown_082082F8[stat]]);
    StringCopy(gUnknown_03004290, gUnknown_083FFDB3);
    get_battle_strings_(gUnknown_083FFDD3);
}

u8 *sub_803F378(u16 itemId)
{
    int i;
    u8 *itemEffect;

    if (itemId == ITEM_ENIGMA_BERRY)
    {
        if (gMain.inBattle)
        {
            itemEffect = gEnigmaBerries[gUnknown_02024E6C].itemEffect;
        }
        else
        {
            itemEffect = gSaveBlock1.enigmaBerry.itemEffect;
        }
    }
    else
    {
        itemEffect = gItemEffectTable[itemId - 13];
    }

    gUnknown_02024C0B = gUnknown_02024E6C;

    for (i = 0; i < 3; i++)
    {
        if (itemEffect[i] & 0xF)
            sub_803F324(i * 2);
        if (itemEffect[i] & 0xF0)
        {
            if (i)
            {
                sub_803F324(i * 2 + 1);
            }
            else
            {
                gPlayerMonIndex = gUnknown_02024E6C;
                get_battle_strings_(gUnknown_083FEE92);
            }
        }
    }

    if (itemEffect[3] & 0x80)
    {
        gPlayerMonIndex = gUnknown_02024E6C;
        get_battle_strings_(gUnknown_083FEE5D);
    }

    return gUnknown_020238CC;
}

u8 GetNature(struct Pokemon *mon)
{
    return GetMonData(mon, MON_DATA_PERSONALITY, 0) % 25;
}

u8 GetNatureFromPersonality(u32 personality)
{
    return personality % 25;
}

u16 GetEvolutionTargetSpecies(struct Pokemon *mon, u8 type, u16 evolutionItem)
{
    int i;
    u16 targetSpecies = 0;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u16 heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
    u8 level;
    u16 friendship;
    u8 beauty = GetMonData(mon, MON_DATA_BEAUTY, 0);
    u16 upperPersonality = personality >> 16;
    u8 holdEffect;

    if (heldItem == ITEM_ENIGMA_BERRY)
        holdEffect = gSaveBlock1.enigmaBerry.holdEffect;
    else
        holdEffect = ItemId_GetHoldEffect(heldItem);

    if (holdEffect == 38 && type != 3)
        return 0;

    switch (type)
    {
    case 0:
        level = GetMonData(mon, MON_DATA_LEVEL, 0);
        friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);

        for (i = 0; i < 5; i++)
        {
            switch (gEvolutionTable[species].evolutions[i].method)
            {
            case EVO_FRIENDSHIP:
                if (friendship >= 220)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_FRIENDSHIP_DAY:
                RtcCalcLocalTime();
                if (gLocalTime.hours >= 12 && gLocalTime.hours < 24 && friendship >= 220)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_FRIENDSHIP_NIGHT:
                RtcCalcLocalTime();
                if (gLocalTime.hours >= 0 && gLocalTime.hours < 12 && friendship >= 220)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL:
                if (gEvolutionTable[species].evolutions[i].param <= level)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_ATK_GT_DEF:
                if (gEvolutionTable[species].evolutions[i].param <= level)
                    if (GetMonData(mon, MON_DATA_ATK, 0) > GetMonData(mon, MON_DATA_DEF, 0))
                        targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_ATK_EQ_DEF:
                if (gEvolutionTable[species].evolutions[i].param <= level)
                    if (GetMonData(mon, MON_DATA_ATK, 0) == GetMonData(mon, MON_DATA_DEF, 0))
                        targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_ATK_LT_DEF:
                if (gEvolutionTable[species].evolutions[i].param <= level)
                    if (GetMonData(mon, MON_DATA_ATK, 0) < GetMonData(mon, MON_DATA_DEF, 0))
                        targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_SILCOON:
                if (gEvolutionTable[species].evolutions[i].param <= level && (upperPersonality % 10) <= 4)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_CASCOON:
                if (gEvolutionTable[species].evolutions[i].param <= level && (upperPersonality % 10) > 4)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_LEVEL_NINJASK:
                if (gEvolutionTable[species].evolutions[i].param <= level)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_BEAUTY:
                if (gEvolutionTable[species].evolutions[i].param <= beauty)
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    case 1:
        for (i = 0; i < 5; i++)
        {
            switch (gEvolutionTable[species].evolutions[i].method)
            {
            case EVO_TRADE:
                targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            case EVO_TRADE_ITEM:
                if (gEvolutionTable[species].evolutions[i].param == heldItem)
                {
                    heldItem = 0;
                    SetMonData(mon, MON_DATA_HELD_ITEM, (u8 *)&heldItem);
                    targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                }
                break;
            }
        }
        break;
    case 2:
    case 3:
        for (i = 0; i < 5; i++)
        {
            if (gEvolutionTable[species].evolutions[i].method == EVO_ITEM
             && gEvolutionTable[species].evolutions[i].param == evolutionItem)
            {
                targetSpecies = gEvolutionTable[species].evolutions[i].targetSpecies;
                break;
            }
        }
        break;
    }

    return targetSpecies;
}

u16 HoennPokedexNumToSpecies(u16 hoennNum)
{
    u16 species;

    if (!hoennNum)
        return 0;

    species = 0;

    while (species < 411 && gSpeciesToHoennPokedexNum[species] != hoennNum)
        species++;

    if (species == 411)
        return 0;

    return species + 1;
}

u16 NationalPokedexNumToSpecies(u16 nationalNum)
{
    u16 species;

    if (!nationalNum)
        return 0;

    species = 0;

    while (species < 411 && gSpeciesToNationalPokedexNum[species] != nationalNum)
        species++;

    if (species == 411)
        return 0;

    return species + 1;
}

u16 NationalToHoennOrder(u16 nationalNum)
{
    u16 hoennNum;

    if (!nationalNum)
        return 0;

    hoennNum = 0;

    while (hoennNum < 411 && gHoennToNationalOrder[hoennNum] != nationalNum)
        hoennNum++;

    if (hoennNum == 411)
        return 0;

    return hoennNum + 1;
}

u16 SpeciesToNationalPokedexNum(u16 species)
{
    if (!species)
        return 0;

    return gSpeciesToNationalPokedexNum[species - 1];
}

u16 SpeciesToHoennPokedexNum(u16 species)
{
    if (!species)
        return 0;

    return gSpeciesToHoennPokedexNum[species - 1];
}

u16 HoennToNationalOrder(u16 hoennNum)
{
    if (!hoennNum)
        return 0;

    return gHoennToNationalOrder[hoennNum - 1];
}

u32 SpeciesToCryId(u16 species)
{
    if (species <= 250)
        return species;

    if (species < 276)
        return 200;

    return gSpeciesIdToCryId[species - 276];
}

void unref_sub_803F938(u16 species, u32 personality, u8 *dest)
{
    if (species == SPECIES_SPINDA && dest != gUnknown_081FAF4C[0] && dest != gUnknown_081FAF4C[2])
    {
        int i;
        for (i = 0; i < 4; i++)
        {
            int j;
            u8 x = gSpindaSpotGraphics[i].x + ((personality & 0x0F) - 8);
            u8 y = gSpindaSpotGraphics[i].y + (((personality & 0xF0) >> 4) - 8);

            for (j = 0; j < 16; j++)
            {
                int k;
                s32 row = gSpindaSpotGraphics[i].image[j];

                for (k = x; k < x + 16; k++)
                {
                    u8 *val = dest + ((k / 8) * 32) + ((k % 8) / 2) + ((y >> 3) << 8) + ((y & 7) << 2);

                    if (row & 1)
                    {
                        if (k & 1)
                        {
                            if ((u8)((*val & 0xF0) - 0x10) <= 0x20)
                                *val += 0x40;
                        }
                        else
                        {
                            if ((u8)((*val & 0xF) - 0x01) <= 0x02)
                                *val += 0x04;
                        }
                    }

                    row >>= 1;
                }

                y++;
            }

            personality >>= 8;
        }
    }
}

void DrawSpindaSpots(u16 species, u32 personality, u8 *dest, u8 a4)
{
    if (species == SPECIES_SPINDA && a4)
    {
        int i;
        for (i = 0; i < 4; i++)
        {
            int j;
            u8 x = gSpindaSpotGraphics[i].x + ((personality & 0x0F) - 8);
            u8 y = gSpindaSpotGraphics[i].y + (((personality & 0xF0) >> 4) - 8);

            for (j = 0; j < 16; j++)
            {
                int k;
                s32 row = gSpindaSpotGraphics[i].image[j];

                for (k = x; k < x + 16; k++)
                {
                    u8 *val = dest + ((k / 8) * 32) + ((k % 8) / 2) + ((y >> 3) << 8) + ((y & 7) << 2);

                    if (row & 1)
                    {
                        if (k & 1)
                        {
                            if ((u8)((*val & 0xF0) - 0x10) <= 0x20)
                                *val += 0x40;
                        }
                        else
                        {
                            if ((u8)((*val & 0xF) - 0x01) <= 0x02)
                                *val += 0x04;
                        }
                    }

                    row >>= 1;
                }

                y++;
            }

            personality >>= 8;
        }
    }
}

void EvolutionRenameMon(struct Pokemon *mon, u16 oldSpecies, u16 newSpecies)
{
#ifdef BUGFIX_EVO_NAME
    u8 language;
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    language = GetMonData(mon, MON_DATA_LANGUAGE, &language);
    if (language == GAME_LANGUAGE && !StringCompareWithoutExtCtrlCodes(gSpeciesNames[oldSpecies], gStringVar1))
        SetMonData(mon, MON_DATA_NICKNAME, gSpeciesNames[newSpecies]);
#else
    GetMonData(mon, MON_DATA_NICKNAME, gStringVar1);
    if (!StringCompareWithoutExtCtrlCodes(gSpeciesNames[oldSpecies], gStringVar1))
        SetMonData(mon, MON_DATA_NICKNAME, gSpeciesNames[newSpecies]);
#endif
}

bool8 sub_803FBBC(void)
{
    bool8 retVal = FALSE;
    switch (gLinkPlayers[GetMultiplayerId()].lp_field_18)
    {
    case 0:
    case 3:
        retVal = FALSE;
        break;
    case 1:
    case 2:
        retVal = TRUE;
        break;
    }
    return retVal;
}

bool8 sub_803FBFC(u8 id)
{
    bool8 retVal = FALSE;
    switch (gLinkPlayers[id].lp_field_18)
    {
    case 0:
    case 3:
        retVal = FALSE;
        break;
    case 1:
    case 2:
        retVal = TRUE;
        break;
    }
    return retVal;
}

s32 sub_803FC34(u16 a1)
{
    s32 id;
    for (id = 0; id < MAX_LINK_PLAYERS; id++)
        if (gLinkPlayers[id].lp_field_18 == a1)
            break;
    return id;
}

u8 sub_803FC58(u16 trainer)
{
    return gTrainers[trainer].encounterMusic_gender & 0x7F;
}

u16 nature_stat_mod(u8 nature, u16 n, u8 statIndex)
{
    if (statIndex < 1 || statIndex > 5)
    {
        // should just be "return n", but it wouldn't match without this
        u16 retVal = n;
        retVal++;
        retVal--;
        return retVal;
    }

    switch (gNatureStatTable[nature][statIndex - 1])
    {
    case 1:
        return (u16)(n * 110) / 100;
    case -1:
        return (u16)(n * 90) / 100;
    }

    return n;
}

void AdjustFriendship(struct Pokemon *mon, u8 event)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES2, 0);
    u16 heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);
    u8 holdEffect;

    if (heldItem == ITEM_ENIGMA_BERRY)
    {
        if (gMain.inBattle)
        {
            holdEffect = gEnigmaBerries[0].holdEffect;
        }
        else
        {
            holdEffect = gSaveBlock1.enigmaBerry.holdEffect;
        }
    }
    else
    {
        holdEffect = ItemId_GetHoldEffect(heldItem);
    }

    if (species && species != SPECIES_EGG)
    {
        u8 friendshipLevel = 0;
        s16 friendship = GetMonData(mon, MON_DATA_FRIENDSHIP, 0);
        if (friendship > 99)
            friendshipLevel++;
        if (friendship > 199)
            friendshipLevel++;
        if ((event != 5 || !(Random() & 1))
         && (event != 3
          || ((gBattleTypeFlags & BATTLE_TYPE_TRAINER)
           && (gTrainers[gTrainerBattleOpponent].trainerClass == 24
            || gTrainers[gTrainerBattleOpponent].trainerClass == 25
            || gTrainers[gTrainerBattleOpponent].trainerClass == 32))))
        {
            s8 mod = gUnknown_082082FE[event][friendshipLevel];
            if (mod > 0 && holdEffect == HOLD_EFFECT_HAPPINESS_UP)
                mod = (150 * mod) / 100;
            friendship += mod;
            if (mod > 0)
            {
                if (GetMonData(mon, MON_DATA_POKEBALL, 0) == ITEM_LUXURY_BALL)
                    friendship++;
                if (GetMonData(mon, MON_DATA_MET_LOCATION, 0) == sav1_map_get_name())
                    friendship++;
            }
            if (friendship < 0)
                friendship = 0;
            if (friendship > 255)
                friendship = 255;
            SetMonData(mon, MON_DATA_FRIENDSHIP, (u8 *)&friendship);
        }
    }
}

void MonGainEVs(struct Pokemon *mon, u16 defeatedSpecies)
{
    u8 evs[NUM_STATS];
    u16 evIncrease = 0;
    u16 totalEVs = 0;
    u16 heldItem;
    u8 holdEffect;
    int i;

    for (i = 0; i < NUM_STATS; i++)
    {
        evs[i] = GetMonData(mon, MON_DATA_HP_EV + i, 0);
        totalEVs += evs[i];
    }

    for (i = 0; i < NUM_STATS; i++)
    {
        u8 hasHadPokerus;
        int multiplier;

        if (totalEVs >= MAX_TOTAL_EVS)
            break;

        hasHadPokerus = CheckPartyHasHadPokerus(mon, 0);

        if (hasHadPokerus)
            multiplier = 2;
        else
            multiplier = 1;

        switch (i)
        {
        case 0:
            evIncrease = gBaseStats[defeatedSpecies].evYield_HP * multiplier;
            break;
        case 1:
            evIncrease = gBaseStats[defeatedSpecies].evYield_Attack * multiplier;
            break;
        case 2:
            evIncrease = gBaseStats[defeatedSpecies].evYield_Defense * multiplier;
            break;
        case 3:
            evIncrease = gBaseStats[defeatedSpecies].evYield_Speed * multiplier;
            break;
        case 4:
            evIncrease = gBaseStats[defeatedSpecies].evYield_SpAttack * multiplier;
            break;
        case 5:
            evIncrease = gBaseStats[defeatedSpecies].evYield_SpDefense * multiplier;
            break;
        }

        heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, 0);

        if (heldItem == ITEM_ENIGMA_BERRY)
        {
            if (gMain.inBattle)
            {
                holdEffect = gEnigmaBerries[0].holdEffect;
            }
            else
            {
                holdEffect = gSaveBlock1.enigmaBerry.holdEffect;
            }
        }
        else
        {
            holdEffect = ItemId_GetHoldEffect(heldItem);
        }

        if (holdEffect == HOLD_EFFECT_MACHO_BRACE)
            evIncrease *= 2;

        if (totalEVs + (s16)evIncrease > MAX_TOTAL_EVS)
            evIncrease = ((s16)evIncrease + MAX_TOTAL_EVS) - (totalEVs + evIncrease);

        if (evs[i] + (s16)evIncrease > 255)
        {
            int val1 = (s16)evIncrease + 255;
            int val2 = evs[i] + evIncrease;
            evIncrease = val1 - val2;
        }

        evs[i] += evIncrease;
        totalEVs += evIncrease;
        SetMonData(mon, MON_DATA_HP_EV + i, &evs[i]);
    }
}

u16 GetMonEVCount(struct Pokemon *mon)
{
    int i;
    u16 count = 0;

    for (i = 0; i < NUM_STATS; i++)
        count += GetMonData(mon, MON_DATA_HP_EV + i, 0);

    return count;
}

void RandomlyGivePartyPokerus(struct Pokemon *party)
{
    u16 rnd = Random();
    if (rnd == 0x4000 || rnd == 0x8000 || rnd == 0xC000)
    {
        struct Pokemon *mon;

        do
        {
            do
            {
                rnd = Random() % PARTY_SIZE;
                mon = &party[rnd];
            }
            while (!GetMonData(mon, MON_DATA_SPECIES, 0));
        }
        while (GetMonData(mon, MON_DATA_IS_EGG, 0));

        if (!(CheckPartyHasHadPokerus(party, gBitTable[rnd])))
        {
            u8 rnd2;

            do
            {
                rnd2 = Random();
            }
            while (rnd2 == 0);

            if (rnd2 & 0xF0)
                rnd2 &= 0x07;

            rnd2 |= (rnd2 << 4);
            rnd2 &= 0xF3;
            rnd2++;

            SetMonData(&party[rnd], MON_DATA_POKERUS, &rnd2);
        }
    }
}

u8 CheckPartyPokerus(struct Pokemon *party, u8 selection)
{
    u8 retVal;

    int partyIndex = 0;
    unsigned curBit = 1;
    retVal = 0;

    if (selection)
    {
        do
        {
            if ((selection & 1) && (GetMonData(&party[partyIndex], MON_DATA_POKERUS, 0) & 0xF))
                retVal |= curBit;
            partyIndex++;
            curBit <<= 1;
            selection >>= 1;
        }
        while (selection);
    }
    else if (GetMonData(&party[0], MON_DATA_POKERUS, 0) & 0xF)
    {
        retVal = 1;
    }

    return retVal;
}

u8 CheckPartyHasHadPokerus(struct Pokemon *party, u8 selection)
{
    u8 retVal;

    int partyIndex = 0;
    unsigned curBit = 1;
    retVal = 0;

    if (selection)
    {
        do
        {
            if ((selection & 1) && GetMonData(&party[partyIndex], MON_DATA_POKERUS, 0))
                retVal |= curBit;
            partyIndex++;
            curBit <<= 1;
            selection >>= 1;
        }
        while (selection);
    }
    else if (GetMonData(&party[0], MON_DATA_POKERUS, 0))
    {
        retVal = 1;
    }

    return retVal;
}

void UpdatePartyPokerusTime(u16 days)
{
    int i;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, 0))
        {
            u8 pokerus = GetMonData(&gPlayerParty[i], MON_DATA_POKERUS, 0);
            if (pokerus & 0xF)
            {
                if ((pokerus & 0xF) < days || days > 4)
                    pokerus &= 0xF0;
                else
                    pokerus -= days;

                SetMonData(&gPlayerParty[i], MON_DATA_POKERUS, &pokerus);
            }
        }
    }
}

void PartySpreadPokerus(struct Pokemon *party)
{
    if ((Random() % 3) == 0)
    {
        int i;
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&party[i], MON_DATA_SPECIES, 0))
            {
                u8 pokerus = GetMonData(&party[i], MON_DATA_POKERUS, 0);
                u8 curPokerus = pokerus;
                if (pokerus)
                {
                    if (pokerus & 0xF)
                    {
                        // spread to adjacent party members
                        if (i != 0 && !(GetMonData(&party[i - 1], MON_DATA_POKERUS, 0) & 0xF0))
                            SetMonData(&party[i - 1], MON_DATA_POKERUS, &curPokerus);
                        if (i != (PARTY_SIZE - 1) && !(GetMonData(&party[i + 1], MON_DATA_POKERUS, 0) & 0xF0))
                        {
                            SetMonData(&party[i + 1], MON_DATA_POKERUS, &curPokerus);
                            i++;
                        }
                    }
                }
            }
        }
    }
}

bool8 TryIncrementMonLevel(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u8 nextLevel = GetMonData(mon, MON_DATA_LEVEL, 0) + 1;
    if (GetMonData(mon, MON_DATA_EXP, 0) > gExperienceTables[gBaseStats[species].growthRate][nextLevel])
    {
        SetMonData(mon, MON_DATA_LEVEL, &nextLevel);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

u32 CanMonLearnTMHM(struct Pokemon *mon, u8 tm)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES2, 0);
    if (species == SPECIES_EGG)
    {
        return 0;
    }
    else if (tm < 32)
    {
        u32 mask = 1 << tm;
        return gTMHMLearnsets[species][0] & mask;
    }
    else
    {
        u32 mask = 1 << (tm - 32);
        return gTMHMLearnsets[species][1] & mask;
    }
}

u8 GetMoveRelearnerMoves(struct Pokemon *mon, u16 *moves)
{
    u16 learnedMoves[4];
    u8 numMoves = 0;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    int i, j, k;

    for (i = 0; i < 4; i++)
        learnedMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, 0);

    for (i = 0; i < 20; i++)
    {
        u16 moveLevel;

        if (gLevelUpLearnsets[species][i] == 0xFFFF)
            break;

        moveLevel = gLevelUpLearnsets[species][i] & 0xFE00;

        if (moveLevel <= (level << 9))
        {
            for (j = 0; j < 4 && learnedMoves[j] != (gLevelUpLearnsets[species][i] & 0x1FF); j++)
                ;

            if (j == 4)
            {
                for (k = 0; k < numMoves && moves[k] != (gLevelUpLearnsets[species][i] & 0x1FF); k++)
                    ;

                if (k == numMoves)
                    moves[numMoves++] = gLevelUpLearnsets[species][i] & 0x1FF;
            }
        }
    }

    return numMoves;
}

u8 GetLevelUpMovesBySpecies(u16 species, u16 *moves)
{
    u8 numMoves = 0;
    int i;

    for (i = 0; i < 20 && gLevelUpLearnsets[species][i] != 0xFFFF; i++)
         moves[numMoves++] = gLevelUpLearnsets[species][i] & 0x1FF;

     return numMoves;
}

u8 sub_8040574(struct Pokemon *mon)
{
    u16 learnedMoves[4];
    u16 moves[20];
    u8 numMoves = 0;
    u16 species = GetMonData(mon, MON_DATA_SPECIES2, 0);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, 0);
    int i, j, k;

    if (species == SPECIES_EGG)
        return 0;

    for (i = 0; i < 4; i++)
        learnedMoves[i] = GetMonData(mon, MON_DATA_MOVE1 + i, 0);

    for (i = 0; i < 20; i++)
    {
        u16 moveLevel;

        if (gLevelUpLearnsets[species][i] == 0xFFFF)
            break;

        moveLevel = gLevelUpLearnsets[species][i] & 0xFE00;

        if (moveLevel <= (level << 9))
        {
            for (j = 0; j < 4 && learnedMoves[j] != (gLevelUpLearnsets[species][i] & 0x1FF); j++)
                ;

            if (j == 4)
            {
                for (k = 0; k < numMoves && moves[k] != (gLevelUpLearnsets[species][i] & 0x1FF); k++)
                    ;

                if (k == numMoves)
                    moves[numMoves++] = gLevelUpLearnsets[species][i] & 0x1FF;
            }
        }
    }

    return numMoves;
}

u16 SpeciesToPokedexNum(u16 species)
{
    if (IsNationalPokedexEnabled())
    {
        return SpeciesToNationalPokedexNum(species);
    }
    else
    {
        species = SpeciesToHoennPokedexNum(species);
        if (species <= 202)
            return species;
        return 0xFFFF;
    }
}

void ClearBattleMonForms(void)
{
    int i;
    for (i = 0; i < 4; i++)
        gBattleMonForms[i] = 0;
}

u16 sub_8040728(void)
{
    if (gBattleTypeFlags & BATTLE_TYPE_KYOGRE_GROUDON)
        return BGM_BATTLE34;
    if (gBattleTypeFlags & BATTLE_TYPE_REGI)
        return BGM_BATTLE36;
    if (gBattleTypeFlags & BATTLE_TYPE_LINK)
        return BGM_BATTLE20;
    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
    {
        switch (gTrainers[gTrainerBattleOpponent].trainerClass)
        {
        case 2:
        case 0x31:
            return BGM_BATTLE30;
        case 3:
        case 4:
        case 0x32:
        case 0x33:
            return BGM_BATTLE31;
        case 0x19:
            return BGM_BATTLE32;
        case 0x20:
            return BGM_BATTLE33;
        case 0x2E:
            if (!StringCompare(gTrainers[gTrainerBattleOpponent].trainerName, BattleText_Wally))
                return BGM_BATTLE20;
            return BGM_BATTLE35;
        case 0x18:
            return BGM_BATTLE38;
        default:
            return BGM_BATTLE20;
        }
    }
    return BGM_BATTLE27;
}

void sub_80408BC(void)
{
    ResetMapMusic();
    m4aMPlayAllStop();
    PlayBGM(sub_8040728());
}

void current_map_music_set__default_for_battle(u16 song)
{
    ResetMapMusic();
    m4aMPlayAllStop();
    if (song)
        PlayNewMapMusic(song);
    else
        PlayNewMapMusic(sub_8040728());
}

const u16 *species_and_otid_get_pal(u16, u32, u32);

const u16 *pokemon_get_pal(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES2, 0);
    u32 otId = GetMonData(mon, MON_DATA_OT_ID, 0);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
    return species_and_otid_get_pal(species, otId, personality);
}

//Extracts the upper 16 bits of a 32-bit number
#define HIHALF(n) (((n) & 0xFFFF0000) >> 16)

//Extracts the lower 16 bits of a 32-bit number
#define LOHALF(n) ((n) & 0xFFFF)

const u16 *species_and_otid_get_pal(u16 species, u32 otId , u32 personality)
{
    u32 shinyValue;

    if (species > SPECIES_EGG)
        return gMonPaletteTable[0].data;

    shinyValue = HIHALF(otId) ^ LOHALF(otId) ^ HIHALF(personality) ^ LOHALF(personality);
    if (shinyValue < 8)
        return gMonShinyPaletteTable[species].data;
    else
        return gMonPaletteTable[species].data;
}

const struct SpritePalette *sub_80409C8(u16, u32, u32);

const struct SpritePalette *sub_8040990(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES2, 0);
    u32 otId = GetMonData(mon, MON_DATA_OT_ID, 0);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
    return sub_80409C8(species, otId, personality);
}

const struct SpritePalette *sub_80409C8(u16 species, u32 otId , u32 personality)
{
    u32 shinyValue;

    shinyValue = HIHALF(otId) ^ LOHALF(otId) ^ HIHALF(personality) ^ LOHALF(personality);
    if (shinyValue < 8)
        return &gMonShinyPaletteTable[species];
    else
        return &gMonPaletteTable[species];
}

bool8 IsHMMove2(u16 move)
{
    int i = 0;
    while (gHMMoves[i] != 0xFFFF)
    {
        if (gHMMoves[i++] == move)
            return TRUE;
    }
    return FALSE;
}

bool8 sub_8040A3C(u16 species)
{
    return gBaseStats[species].bodyColor >> 7; // XXX: should this be a bitfield instead?
}

s8 sub_8040A54(struct Pokemon *mon, u8 a2)
{
    u8 nature = GetNature(mon);
    return gUnknown_083F7E28[nature * 5 + a2];
}

s8 sub_8040A7C(u32 personality, u8 a2)
{
    u8 nature = GetNatureFromPersonality(personality);
    return gUnknown_083F7E28[nature * 5 + a2];
}

bool8 IsOtherTrainer(u32, u8 *);

bool8 IsTradedMon(struct Pokemon *mon)
{
    u8 otName[8];
    u32 otId;
    GetMonData(mon, MON_DATA_OT_NAME, otName);
    otId = GetMonData(mon, MON_DATA_OT_ID, 0);
    return IsOtherTrainer(otId, otName);
}

bool8 IsOtherTrainer(u32 otId, u8 *otName)
{
    if (otId == (gSaveBlock2.playerTrainerId[0] | (gSaveBlock2.playerTrainerId[1] << 8) | (gSaveBlock2.playerTrainerId[2] << 16) | (gSaveBlock2.playerTrainerId[3] << 24)))
    {
        int i;

        for (i = 0; otName[i] != EOS; i++)
            if (otName[i] != gSaveBlock2.playerName[i])
                return TRUE;
        return FALSE;
    }

    return TRUE;
}

void BoxMonRestorePP(struct BoxPokemon *);

void MonRestorePP(struct Pokemon *mon)
{
    BoxMonRestorePP(&mon->box);
}

void BoxMonRestorePP(struct BoxPokemon *boxMon)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (GetBoxMonData(boxMon, MON_DATA_MOVE1 + i, 0))
        {
            u16 move = GetBoxMonData(boxMon, MON_DATA_MOVE1 + i, 0);
            u16 bonus = GetBoxMonData(boxMon, MON_DATA_PP_BONUSES, 0);
            u8 pp = CalculatePPWithBonus(move, bonus, i);
            SetBoxMonData(boxMon, MON_DATA_PP1 + i, &pp);
        }
    }
}

void sub_8040B8C(void)
{
    byte_2024C06 = BATTLE_STRUCT->filler1_2[0x37];
    gUnknown_030041C0[0] = 0xFD;
    gUnknown_030041C0[1] = 4;
    gUnknown_030041C0[2] = BATTLE_STRUCT->filler1[0x34];
    gUnknown_030041C0[4] = EOS;
    if (!battle_side_get_owner(BATTLE_STRUCT->filler1[0x34]))
        gUnknown_030041C0[3] = pokemon_order_func(gUnknown_02024A6A[BATTLE_STRUCT->filler1[0x34]]);
    else
        gUnknown_030041C0[3] = gUnknown_02024A6A[BATTLE_STRUCT->filler1[0x34]];
    gUnknown_03004290[0] = 0xFD;
    gUnknown_03004290[1] = 4;
    gUnknown_03004290[2] = gUnknown_02024E6C;
    gUnknown_03004290[3] = pokemon_order_func(gUnknown_02024A6A[gUnknown_02024E6C]);
    gUnknown_03004290[4] = EOS;
    sub_8120FFC(BattleText_PreventedSwitch, gStringVar4);
}

void SetWildMonHeldItem(void)
{
    if (!(gBattleTypeFlags & (BATTLE_TYPE_LEGENDARY | BATTLE_TYPE_TRAINER)))
    {
        u16 rnd = Random() % 100;
        u16 species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES, 0);
        if (gBaseStats[species].item1 == gBaseStats[species].item2)
        {
            SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, (u8 *)&gBaseStats[species].item1);
            return;
        }

        if (rnd > 44)
        {
            if (rnd <= 94)
                SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, (u8 *)&gBaseStats[species].item1);
            else
                SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, (u8 *)&gBaseStats[species].item2);
        }
    }
}

bool8 IsShinyOtIdPersonality(u32, u32);

bool8 IsShiny(struct Pokemon *mon)
{
    u32 otId = GetMonData(mon, MON_DATA_OT_ID, 0);
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, 0);
    return IsShinyOtIdPersonality(otId, personality);
}

bool8 IsShinyOtIdPersonality(u32 otId, u32 personality)
{
    bool8 retVal = FALSE;
    u32 shinyValue = HIHALF(otId) ^ LOHALF(otId) ^ HIHALF(personality) ^ LOHALF(personality);
    if (shinyValue < 8)
        retVal = TRUE;
    return retVal;
}

u8 *sub_8040D08(void)
{
    u8 id = GetMultiplayerId();
    return gLinkPlayers[sub_803FC34(gLinkPlayers[id].lp_field_18 ^ 2)].name;
}

bool32 sub_8040D3C(u16 species, u8 *name, u8 language)
{
    bool32 retVal = FALSE;
    if (species == SPECIES_NIDORAN_M || species == SPECIES_NIDORAN_F)
    {
        u8 *speciesName;
        if (language == GAME_LANGUAGE)
        {
            speciesName = gSpeciesNames[species];
        }
        else
        {
            if (species == SPECIES_NIDORAN_M)
                speciesName = gJapaneseNidoranNames[0];
            else
                speciesName = gJapaneseNidoranNames[1];
        }
        if (!StringCompareWithoutExtCtrlCodes(name, speciesName))
            retVal = TRUE;
        else
            retVal = FALSE;
    }
    return retVal;
}

bool32 sub_8040D8C(u16 species, u8 *name)
{
    u8 language = GAME_LANGUAGE;
    if (name[0] == 0xFC && name[1] == 21)
        language = LANGUAGE_JAPANESE;
    return sub_8040D3C(species, name, language);
}

bool32 unref_sub_8040DAC(struct Pokemon *mon)
{
    u8 name[12];
    u16 species = GetMonData(mon, MON_DATA_SPECIES, 0);
    u8 language = GetMonData(mon, MON_DATA_LANGUAGE, 0);
    GetMonData(mon, MON_DATA_NICKNAME, name);
    return sub_8040D3C(species, name, language);
}
