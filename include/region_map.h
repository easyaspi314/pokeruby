#ifndef GUARD_REGION_MAP_H
#define GUARD_REGION_MAP_H

// Map Section IDs
#define MAPSEC_LITTLEROOT_TOWN 0
#define MAPSEC_OLDALE_TOWN 1
#define MAPSEC_DEWFORD_TOWN 2
#define MAPSEC_LAVARIDGE_TOWN 3
#define MAPSEC_FALLARBOR_TOWN 4
#define MAPSEC_VERDANTURF_TOWN 5
#define MAPSEC_PACIFIDLOG_TOWN 6
#define MAPSEC_PETALBURG_CITY 7
#define MAPSEC_SLATEPORT_CITY 8
#define MAPSEC_MAUVILLE_CITY 9
#define MAPSEC_RUSTBORO_CITY 10
#define MAPSEC_FORTREE_CITY 11
#define MAPSEC_LILYCOVE_CITY 12
#define MAPSEC_MOSSDEEP_CITY 13
#define MAPSEC_SOOTOPOLIS_CITY 14
#define MAPSEC_EVER_GRANDE_CITY 15
#define MAPSEC_ROUTE_101 0x10
#define MAPSEC_ROUTE_102 0x11
#define MAPSEC_ROUTE_103 0x12
#define MAPSEC_ROUTE_104 0x13
#define MAPSEC_ROUTE_105 0x14
#define MAPSEC_ROUTE_106 0x15
#define MAPSEC_ROUTE_107 0x16
#define MAPSEC_ROUTE_108 0x17
#define MAPSEC_ROUTE_109 0x18
#define MAPSEC_ROUTE_110 0x19
#define MAPSEC_ROUTE_111 0x1A
#define MAPSEC_ROUTE_112 0x1B
#define MAPSEC_ROUTE_113 0x1C
#define MAPSEC_ROUTE_114 0x1D
#define MAPSEC_ROUTE_115 0x1E
#define MAPSEC_ROUTE_116 0x1F
#define MAPSEC_ROUTE_117 0x20
#define MAPSEC_ROUTE_118 0x21
#define MAPSEC_ROUTE_119 0x22
#define MAPSEC_ROUTE_120 0x23
#define MAPSEC_ROUTE_121 0x24
#define MAPSEC_ROUTE_122 0x25
#define MAPSEC_ROUTE_123 0x26
#define MAPSEC_ROUTE_124 0x27
#define MAPSEC_ROUTE_125 0x28
#define MAPSEC_ROUTE_126 0x29
#define MAPSEC_ROUTE_127 0x2A
#define MAPSEC_ROUTE_128 0x2B
#define MAPSEC_ROUTE_129 0x2C
#define MAPSEC_ROUTE_130 0x2D
#define MAPSEC_ROUTE_131 0x2E
#define MAPSEC_ROUTE_132 0x2F
#define MAPSEC_ROUTE_133 0x30
#define MAPSEC_ROUTE_134 0x31
#define MAPSEC_UNDERWATER1 0x32
#define MAPSEC_UNDERWATER2 0x33
#define MAPSEC_UNDERWATER3 0x34
#define MAPSEC_UNDERWATER4 0x35
#define MAPSEC_UNDERWATER5 0x36
#define MAPSEC_GRANITE_CAVE 0x37
#define MAPSEC_MT_CHIMNEY 0x38
#define MAPSEC_SAFARI_ZONE 0x39
#define MAPSEC_BATTLE_TOWER 0x3A
#define MAPSEC_PETALBURG_WOODS 0x3B
#define MAPSEC_RUSTURF_TUNNEL 0x3C
#define MAPSEC_ABANDONED_SHIP 0x3D
#define MAPSEC_NEW_MAUVILLE 0x3E
#define MAPSEC_METEOR_FALLS_1 0x3F
#define MAPSEC_METEOR_FALLS_2 0x40
#define MAPSEC_MT_PYRE 0x41
#define MAPSEC_EVIL_TEAM_HIDEOUT 0x42
#define MAPSEC_SHOAL_CAVE 0x43
#define MAPSEC_SEAFLOOR_CAVERN 0x44
#define MAPSEC_UNDERWATER6 0x45
#define MAPSEC_VICTORY_ROAD 0x46
#define MAPSEC_MIRAGE_ISLAND 0x47
#define MAPSEC_CAVE_OF_ORIGIN 0x48
#define MAPSEC_SOUTHERN_ISLAND 0x49
#define MAPSEC_FIERY_PATH_1 0x4A
#define MAPSEC_FIERY_PATH_2 0x4B
#define MAPSEC_JAGGED_PASS_1 0x4C
#define MAPSEC_JAGGED_PASS_2 0x4D
#define MAPSEC_SEALED_CHAMBER 0x4E
#define MAPSEC_UNDERWATER7 0x4F
#define MAPSEC_SCORCHED_SLAB 0x50
#define MAPSEC_ISLAND_CAVE 0x51
#define MAPSEC_DESERT_RUINS 0x52
#define MAPSEC_ANCIENT_TOMB 0x53
#define MAPSEC_INSIDE_OF_TRUCK 0x54
#define MAPSEC_SKY_PILLAR 0x55
#define MAPSEC_SECRET_BASE 0x56
#define MAPSEC_UNK_0x57 0x57
#define MAPSEC_NONE 0x58

struct RegionMap
{
    u8 mapSectionName[20];
    /*0x14*/ u16 mapSectionId;
    /*0x16*/ u8 unk16;
    /*0x17*/ u8 everGrandeCityArea;
    /*0x18*/ u8 (*inputCallback)(void);
    struct Sprite *cursorSprite;
    struct Sprite *playerIconSprite;
    s32 bg2x;
    s32 bg2y;
    u32 unk2C;  //
    u32 unk30;  //  Map rotation parameters
    u32 unk34;  //  Likely a scrapped feature
    u32 unk38;  //
    s32 unk3C;
    s32 unk40;
    s32 unk44;
    s32 unk48;
    s32 unk4C;
    s32 unk50;
    u16 cursorPosX;
    u16 cursorPosY;
    u16 cursorTileTag;
    u16 cursorPaletteTag;
    s16 scrollX;
    s16 scrollY;
    s16 unk60;
    s16 unk62;
    u16 unk64;
    u16 unk66;
    u16 unk68;
    u16 unk6A;
    u16 unk6C;
    u16 unk6E;
    u16 playerIconTileTag;
    u16 playerIconPaletteTag;
    u16 unk74;
    u16 unk76;
    bool8 zoomed;
    u8 initStep;
    s8 unk7A;
    s8 cursorDeltaX;
    s8 cursorDeltaY;
    bool8 needUpdateVideoRegs;
    bool8 blinkPlayerIcon;
    bool8 playerIsInCave;
    /*0x080*/ u8 filler80[0x100];
    /*0x180*/ u8 cursorSmallImage[0x100];
    /*0x280*/ u8 cursorLargeImage[0x600];
};

void InitRegionMap(struct RegionMap *regionMap, bool8 zoomed);
void sub_80FA904(struct RegionMap *regionMap, bool8 zoomed);
bool8 sub_80FA940(void);
void FreeRegionMapIconResources(void);
u8 sub_80FAB60(void);
void sub_80FAEC4(void);
u8 sub_80FAFC0(void);
void UpdateRegionMapVideoRegs(void);
void sub_80FB2A4(s16, s16);
u16 GetRegionMapSectionAt_(u16 x, u16 y);
u16 sub_80FBA04(u16 mapSectionId);
void CreateRegionMapCursor(u16, u16);
void CreateRegionMapPlayerIcon(u16, u16);
const u8 *GetMapSectionName(u8 *dest, u16 mapSectionId, u16 length);
const u8 *CopyMapName(u8 *dest, u16 b);
const u8 *CopyLocationName(u8 *dest, u16 b);

#endif // GUARD_REGION_MAP_H
