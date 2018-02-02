#include "global.h"
#include "constants/songs.h"
#include "main.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "soar.h"
#include "sound.h"
#include "task.h"

// For now, just use the region map graphics
extern const u16 sRegionMapBkgnd_Pal[];
extern const u8 sRegionMapBkgnd_ImageLZ[];
extern const u8 sRegionMapBkgnd_TilemapLZ[];

static void CB2_LoadSoarGraphics(void);
 void SoarVBlankCB(void);
static void CB2_HandleInput(void);
static void CB2_FadeOut(void);

static u32 sBG2X;
static u32 sBG2Y;
static s16 sBG2PA;
static s16 sBG2PB;
static s16 sBG2PC;
static s16 sBG2PD;

void CB2_InitSoar(void)
{
    // some of these may not be necessary, but I'm just being safe
    ScanlineEffect_Stop();
    ResetTasks();
    ResetSpriteData();
    ResetPaletteFade();
    FreeAllSpritePalettes();

    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0);
    SetMainCallback2(CB2_LoadSoarGraphics);
    //SetVBlankCallback(NULL);
    gMain.state = 0;
}

static void UpdateAffineBGRegs(void)
{
    REG_BG2X = sBG2X;
    REG_BG2Y = sBG2Y;
    REG_BG2PA = sBG2PA;
    REG_BG2PB = sBG2PB;
    REG_BG2PC = sBG2PC;
    REG_BG2PD = sBG2PD;
}

static void CB2_LoadSoarGraphics(void)
{
    switch (gMain.state)
    {
    case 0:
        // fade out
        if (!UpdatePaletteFade())
            gMain.state++;
        break;
    case 1:
        // Load graphics
        REG_DISPCNT = 0;
        LZ77UnCompVram(sRegionMapBkgnd_ImageLZ, (void *)(VRAM + 0));
        LZ77UnCompVram(sRegionMapBkgnd_TilemapLZ, (void *)(VRAM + 8 * 0x800));
        LoadPalette(sRegionMapBkgnd_Pal, 0x70, 64);

        // Set up video regs
        REG_DISPCNT = DISPCNT_MODE_2 | DISPCNT_BG2_ON;
        REG_BG2CNT = BGCNT_PRIORITY(0)
                   | BGCNT_256COLOR
                   | BGCNT_CHARBASE(0)
                   | BGCNT_SCREENBASE(8)
                   | BGCNT_AFF512x512;
        sBG2PA = 0x100;
        sBG2PB = 0;
        sBG2PC = 0;
        sBG2PD = 0x100;
        sBG2X = 0;
        sBG2Y = 0;
        UpdateAffineBGRegs();
        gMain.state++;
        break;
    case 2:
        // start fade in
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, 0);
        gMain.state++;
        break;
    case 3:
        // continue fade in
        if (!UpdatePaletteFade())
        {
            SetMainCallback2(CB2_HandleInput);
            //SetVBlankCallback(SoarVBlankCB);
        }
        break;
    }
}

/*
static void SoarVBlankCB(void)
{
    //ScanlineEffect_InitHBlankDmaTransfer();
}
*/

static void CB2_HandleInput(void)
{
    if (gMain.newKeys & START_BUTTON)
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0);
        SetMainCallback2(CB2_FadeOut);
        return;
    }

    if (gMain.heldKeys & A_BUTTON)
    {
        sBG2PA += 0x10;
        sBG2PD += 0x10;
    }
    if (gMain.heldKeys & B_BUTTON)
    {
        sBG2PA -= 0x10;
        sBG2PD -= 0x10;
    }

    if (gMain.heldKeys & L_BUTTON)
    {
        sBG2PB += 0x100;
        sBG2PC += 0x100;
    }
    if (gMain.heldKeys & R_BUTTON)
    {
        sBG2PB -= 0x100;
        sBG2PC -= 0x100;
    }

    if (gMain.heldKeys & DPAD_UP)
        sBG2Y -= 0x100;
    if (gMain.heldKeys & DPAD_DOWN)
        sBG2Y += 0x100;
    if (gMain.heldKeys & DPAD_LEFT)
        sBG2X -= 0x100;
    if (gMain.heldKeys & DPAD_RIGHT)
        sBG2X += 0x100;
    UpdateAffineBGRegs();
}

static void CB2_FadeOut(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(sub_805469C);
}
