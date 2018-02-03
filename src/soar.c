#include "global.h"
#include "constants/songs.h"
#include "decompress.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "region_map.h"
#include "scanline_effect.h"
#include "soar.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "trig.h"

#define DEBUG 1

#define NOCASH_BREAKPOINT asm("mov r11, r11")

static void CB2_LoadSoarGraphics(void);
static void SoarVBlankCallback(void);
static void SoarHBlankCallback(void);
static void CB2_HandleInput(void);
static void PromptLandCB2(void);
static void ProcessYesNoCB2(void);
static void WarpCB2(void);
static void FadeOutVBlankCallback(void);
static void CB2_FadeOut(void);

extern const u8 sUnknown_083E7920[][3];

// For now, just use the region map graphics
extern const u16 sRegionMapBkgnd_Pal[];
extern const u8 sRegionMapBkgnd_ImageLZ[];
extern const u8 sRegionMapBkgnd_TilemapLZ[];

static const u8 sEonSpriteTiles[] = INCBIN_U8("graphics/intro/intro1_eon.4bpp.lz");
static const u16 sEonSpritePaletteData[16] = {0, RGB(4, 4, 16)};

static const struct OamData sEonSpriteOamData =
{
    .y = DISPLAY_HEIGHT / 2,
    .affineMode = 3,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,
    .shape = 1,
    .x = DISPLAY_WIDTH / 2,
    .matrixNum = 0,
    .size = 3,
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct SpriteTemplate sEonSpriteTemplate =
{
    .tileTag = 9999,
    .paletteTag = 9999,
    .oam = &sEonSpriteOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = NULL,
    .callback = SpriteCallbackDummy,
};

static const struct CompressedSpriteSheet sEonSpriteSheet = {sEonSpriteTiles, sizeof(sEonSpriteTiles), 9999};
static const struct SpritePalette sEonSpritePalette = {sEonSpritePaletteData, 9999};

static const struct {u8 left; u8 top; u8 right; u8 bottom;} sPopupWindowRect = {0, 0, 15, 3};

static s32 sPlayerPosX;
static s32 sPlayerPosY;
static s32 sPlayerPosZ;
static u8 sPlayerYaw;
static u8 sPlayerPitch;
static s32 sTest;

static u8 sEonSpriteId;

#define FIXED_POINT(iPart, fPart) (((iPart) << 8) | (fPart))

void CB2_InitSoar(void)
{
    sPlayerPosX = 32 << 8;
    sPlayerPosY = 32 << 8;
    sPlayerPosZ = 6 << 8;
    sPlayerYaw = 0;
    sPlayerPitch = 0;
    sTest = 0;

    // some of these may not be necessary, but I'm just being safe
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();

    // start fade out
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0);
    SetMainCallback2(CB2_LoadSoarGraphics);
    gMain.state = 0;
}

static void CB2_LoadSoarGraphics(void)
{
    //const struct WindowTemplate *winTemplate = &gWindowTemplate_81E7224;

    switch (gMain.state)
    {
    case 0:
        // continue fade out
        if (!UpdatePaletteFade())
            gMain.state++;
        break;
    case 1:
        // Load graphics
        REG_DISPCNT = 0;
        DmaClearLarge32(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        LZ77UnCompVram(sRegionMapBkgnd_ImageLZ, (void *)(VRAM + 2 * 0x4000));
        LZ77UnCompVram(sRegionMapBkgnd_TilemapLZ, (void *)(VRAM + 2 * 0x800));
        LoadPalette(sRegionMapBkgnd_Pal, 0x70, 64);

        gPlttBufferUnfaded[0] = RGB(8, 8, 20);

        LZ77UnCompVram(sEonSpriteTiles, (void *)(VRAM + 0x10000));
        // Set up video regs
        REG_DISPCNT = DISPCNT_MODE_1 | DISPCNT_BG2_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP;
        //REG_BG0CNT = BGCNT_PRIORITY(0)
        //           | BGCNT_16COLOR
        //           | BGCNT_CHARBASE(2);
        REG_BG2CNT = BGCNT_PRIORITY(0)
                   | BGCNT_256COLOR
                   | BGCNT_CHARBASE(2)
                   | BGCNT_SCREENBASE(2)
                   | BGCNT_AFF512x512
                   | BGCNT_WRAP;
        REG_BLDCNT = BLDCNT_EFFECT_LIGHTEN | BLDCNT_TGT1_BD | BLDCNT_TGT1_BG2;
        gMain.state++;
        break;
    case 2:
        // start fade in
        Text_LoadWindowTemplate(&gWindowTemplate_81E7224);
        InitMenuWindow(&gWindowTemplate_81E7224);
        Menu_DrawStdWindowFrame(sPopupWindowRect.left, sPopupWindowRect.top, sPopupWindowRect.right, sPopupWindowRect.bottom);
        gMain.state++;
        break;
    case 3:
        // continue fade in
        //if (!UpdatePaletteFade())
        {
            DmaClear32(3, (void *)(VRAM + 0x6000), 0x2000);
            SetVBlankCallback(SoarVBlankCallback);
            SetHBlankCallback(SoarHBlankCallback);

            REG_IME = 0;
            REG_IE |= INTR_FLAG_VBLANK | INTR_FLAG_HBLANK;
            REG_IME = 1;
            REG_DISPSTAT |= DISPSTAT_VBLANK_INTR | DISPSTAT_HBLANK_INTR;
            
            // set backdrop color
            //((u16 *)PLTT)[0] = RGB(8, 8, 20);
            
            LoadCompressedObjectPic(&sEonSpriteSheet);
            LoadSpritePalette(&sEonSpritePalette);  // why doesn't this work?
            sEonSpriteId = CreateSprite(&sEonSpriteTemplate, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, 0);

            // HACK! Please use the right palette
            gSprites[sEonSpriteId].oam.paletteNum = 0;
            ((u16 *)PLTT)[0x100 + 1] = RGB(4, 4, 16);
        }
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, 0);
        gMain.state++;
        break;
    case 4:
        //if (!UpdatePaletteFade())
            SetMainCallback2(CB2_HandleInput);
        break;
    }
}

static void SoarVBlankCallback(void)
{
    LoadOam();
    //ProcessSpriteCopyRequests();
    UpdatePaletteFade();

    REG_BG2X = 0;
    REG_BG2Y = 0;
    REG_BG2PA = 0;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 0;

    REG_DISPCNT &= ~DISPCNT_BG2_ON;
    
    TransferPlttBuffer();
}

#define M7_D 128

static void SoarHBlankCallback(void)
{
    int sinYaw = gSineTable[sPlayerYaw];
    int cosYaw = gSineTable[sPlayerYaw + 64];

    int lam, lcf, lsf, lxr, lyr;
    int vcount = REG_VCOUNT;

    if (vcount < 32)
    {
        REG_DISPCNT &= ~DISPCNT_BG2_ON;
        REG_BLDY = vcount / 2;
        return;
    }
    else
    {
        REG_DISPCNT |= DISPCNT_BG2_ON;
        
        if (vcount <= 16 * 6)
            REG_BLDY = 16 - (vcount / 6);
        else
            REG_BLDY = 0;
    }

    lam= sPlayerPosZ * ((1 << 16) / (vcount - 32))>>12;  // .8*.16 /.12 = 20.12
    lcf= lam*cosYaw>>8;                     // .12*.8 /.8 = .12
    lsf= lam*sinYaw>>8;                     // .12*.8 /.8 = .12

    REG_BG2PA= lcf>>4;
    REG_BG2PC= lsf>>4;

    // Offsets
    // Note that the lxr shifts down first! 

    // horizontal offset
    lxr= 120*(lcf>>4);
    lyr= (M7_D*lsf)>>4;
    REG_BG2X= sPlayerPosX - lxr + lyr;

    // vertical offset
    lxr= 120*(lsf>>4);
    lyr= (M7_D*lcf)>>4; 
    REG_BG2Y= sPlayerPosY - lxr - lyr;
}

#define spTiltDir   data[0]
#define spTiltAngle data[1]

static void UpdateEonSpriteRotation(struct Sprite *sprite)
{
    const int TILT_MAX = 0x1000;
    const int TILT_STEP = 0x100;

    switch (sprite->spTiltDir)
    {
    case 0:
        if (sprite->spTiltAngle > 0)
            sprite->spTiltAngle -= TILT_STEP;
        else if (sprite->spTiltAngle < 0)
            sprite->spTiltAngle += TILT_STEP;
        break;
    case 1:
        if (sprite->spTiltAngle < TILT_MAX)
            sprite->spTiltAngle += TILT_STEP;
        break;
    case -1:
        if (sprite->spTiltAngle > -TILT_MAX)
            sprite->spTiltAngle -= TILT_STEP;
        break;
    }
    SetOamMatrixRotationScaling(sprite->oam.matrixNum, 1 << 8, 1 << 8, -sprite->spTiltAngle);
}

static u16 sPrevMapSection;

static void UpdateMapSectionPopup(void)
{
    unsigned int mapSection = GetRegionMapSectionAt_((sPlayerPosX >> 8) / 8, (sPlayerPosY >> 8) / 8);

    if (mapSection != sPrevMapSection)
    {
        if (mapSection == MAPSEC_NONE)
        {
            REG_DISPCNT &= ~DISPCNT_BG0_ON;
        }
        else
        {
            REG_DISPCNT |= DISPCNT_BG0_ON;
            GetMapSectionName(gStringVar4, mapSection, 50);
            Menu_BlankWindowRect(sPopupWindowRect.left + 1, sPopupWindowRect.top + 1, sPopupWindowRect.right - 1, sPopupWindowRect.bottom - 1);
            Menu_PrintText(gStringVar4, 1, 1);
        }
        sPrevMapSection = mapSection;
    }
}

static const u8 sText_LandHere[] = _("Land here?\nA: Land    B: Cancel");

static void ExitSoar(void)
{
    PlaySE(SE_PC_OFF);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0);
    SetMainCallback2(CB2_FadeOut);
}

static void CB2_HandleInput(void)
{
    int sinYaw;
    int cosYaw;

    if ((gMain.newKeys & A_BUTTON) && sPrevMapSection != MAPSEC_NONE)
    {
        Menu_DisplayDialogueFrame();
        MenuPrintMessageDefaultCoords(sText_LandHere);
        SetMainCallback2(PromptLandCB2);
        return;
    }

    if (gMain.newKeys & START_BUTTON)
    {
        ExitSoar();
        return;
    }

    gSprites[sEonSpriteId].spTiltDir = 0;

    if (gMain.heldKeys & DPAD_LEFT)
    {
        sPlayerYaw--;
        gSprites[sEonSpriteId].spTiltDir = -1;
    }
    if (gMain.heldKeys & DPAD_RIGHT)
    {
        sPlayerYaw++;
        gSprites[sEonSpriteId].spTiltDir = 1;
    }
    if (gMain.heldKeys & DPAD_UP)
    {
        sPlayerPosZ += 0x10;
    }
    if (gMain.heldKeys & DPAD_DOWN)
    {
        sPlayerPosZ -= 0x10;
    }
    
    UpdateEonSpriteRotation(&gSprites[sEonSpriteId]);

    sinYaw = gSineTable[sPlayerYaw];
    cosYaw = gSineTable[sPlayerYaw + 64];

    sPlayerPosX += sinYaw / 8;
    sPlayerPosY -= cosYaw / 8;

    UpdateMapSectionPopup();

    BuildOamBuffer();
    //AnimateSprites();
}

static void PromptLandCB2(void)
{
    if (Menu_UpdateWindowText())
    {
        // TODO: Why does this yes/no menu screw everything up?
        //DisplayYesNoMenu(20, 8, 1);
        SetMainCallback2(ProcessYesNoCB2);
    }
    UpdateEonSpriteRotation(&gSprites[sEonSpriteId]);
    BuildOamBuffer();
    //AnimateSprites();
}

static void ErasePrompt(void)
{
    Menu_EraseWindowRect(0, 8, 29, 19);
}

static void ProcessYesNoCB2(void)
{
    switch (Menu_ProcessInputNoWrap_())
    {
    case 0:
        ErasePrompt();
        SetMainCallback2(WarpCB2);
        break;
    case -1:
    case 1:
        ErasePrompt();
        SetMainCallback2(CB2_HandleInput);
        break;
    }
    BuildOamBuffer();
}

static void WarpCB2(void)
{
    if (sUnknown_083E7920[sPrevMapSection][2] != 0)
        sub_8053538(sUnknown_083E7920[sPrevMapSection][2]);
    else
        warp1_set_2(sUnknown_083E7920[sPrevMapSection][0], sUnknown_083E7920[sPrevMapSection][1], -1);
    //ExitSoar();
    //sub_80865BC();
    Fldeff_FlyLand();
}

static void CB2_FadeOut(void)
{
    if (!gPaletteFade.active)
    {
        REG_IME = 0;
        REG_IE &= ~(INTR_FLAG_HBLANK);
        REG_IME = 1;
        REG_DISPSTAT &= ~(DISPSTAT_HBLANK_INTR);

        SetHBlankCallback(NULL);
        SetMainCallback2(sub_805469C);
    }
}
