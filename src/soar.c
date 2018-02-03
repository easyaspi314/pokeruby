#include "global.h"
#include "constants/songs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "soar.h"
#include "sound.h"
#include "task.h"
#include "trig.h"

#define DEBUG 1

// For now, just use the region map graphics
extern const u16 sRegionMapBkgnd_Pal[];
extern const u8 sRegionMapBkgnd_ImageLZ[];
extern const u8 sRegionMapBkgnd_TilemapLZ[];

static void CB2_LoadSoarGraphics(void);
static void SoarVBlankCallback(void);
static void SoarHBlankCallback(void);
static void CB2_HandleInput(void);
static void CB2_FadeOut(void);

static s32 sPlayerPosX;
static s32 sPlayerPosY;
static s32 sPlayerPosZ;
static u8 sPlayerYaw;
static u8 sPlayerPitch;
static s32 sTest;

// GPU affine params
static u32 sSoarBgAffine_X;
static u32 sSoarBgAffine_Y;
static s16 sSoarBgAffine_PA;
static s16 sSoarBgAffine_PB;
static s16 sSoarBgAffine_PC;
static s16 sSoarBgAffine_PD;

#define FIXED_POINT(iPart, fPart) (((iPart) << 8) | (fPart))

void CB2_InitSoar(void)
{
    sPlayerPosX = 32 << 8;
    sPlayerPosY = 32 << 8;
    sPlayerPosZ = 128 << 8;
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

//#define TO_SIGN_MAGNITUDE(n) ((n) >= 0 ? (n) : ((-n) | (1 << 15)))

static void UpdateAffineBGRegs(void)
{
    REG_BG2X = sSoarBgAffine_X;
    REG_BG2Y = sSoarBgAffine_Y;
    REG_BG2PA = sSoarBgAffine_PA;
    REG_BG2PB = sSoarBgAffine_PB;
    REG_BG2PC = sSoarBgAffine_PC;
    REG_BG2PD = sSoarBgAffine_PD;
}

static void CB2_LoadSoarGraphics(void)
{
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
        LZ77UnCompVram(sRegionMapBkgnd_ImageLZ, (void *)(VRAM + 0x8000));
        LZ77UnCompVram(sRegionMapBkgnd_TilemapLZ, (void *)(VRAM + 8 * 0x800));
        LoadPalette(sRegionMapBkgnd_Pal, 0x70, 64);

        // Set up video regs
        REG_DISPCNT = DISPCNT_MODE_1 | DISPCNT_BG2_ON;
        REG_BG2CNT = BGCNT_PRIORITY(0)
                   | BGCNT_256COLOR
                   | BGCNT_CHARBASE(2)
                   | BGCNT_SCREENBASE(8)
                   | BGCNT_AFF512x512;
        sSoarBgAffine_PA = FIXED_POINT(1,0);
        sSoarBgAffine_PB = FIXED_POINT(0,0);
        sSoarBgAffine_PC = FIXED_POINT(0,0);
        sSoarBgAffine_PD = FIXED_POINT(1,0);
        sSoarBgAffine_X = 0;
        sSoarBgAffine_Y = 0;
        UpdateAffineBGRegs();
        gMain.state++;
        break;
    case 2:
        // start fade in
#if DEBUG
        Menu_EraseScreen();
        //Menu_DrawStdWindowFrame(1, 1, 5, 5);
#endif
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, 0);
        gMain.state++;
        break;
    case 3:
        // continue fade in
        if (!UpdatePaletteFade())
        {
            SetMainCallback2(CB2_HandleInput);
            SetVBlankCallback(SoarVBlankCallback);
            SetHBlankCallback(SoarHBlankCallback);
            
            REG_IME = 0;
            REG_IE |= INTR_FLAG_VBLANK | INTR_FLAG_HBLANK;
            REG_IME = 1;
            REG_DISPSTAT |= DISPSTAT_VBLANK_INTR | DISPSTAT_HBLANK_INTR;
        }
        break;
    }
}

static void SoarVBlankCallback(void)
{
    //ScanlineEffect_InitHBlankDmaTransfer();
    //UpdateAffineBGRegs();
    REG_BG2X = 0;
    REG_BG2Y = 0;
    REG_BG2PA = 0;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 0;
}

#define SHIFT_AMT 12
#define M7_D 128

static void SoarHBlankCallback(void)
{
    int sinYaw = gSineTable[sPlayerYaw];
    int cosYaw = gSineTable[sPlayerYaw + 64];

    /*
    int vcount = REG_VCOUNT;
    int lam, xs, ys;

    //if (vcount > 32)
    //{
        
    lam = (10 << 8) * ((1 << 16) / vcount)>> (24-SHIFT_AMT);  // .8*.16/.12 = .12

    xs = 120*lam;
    ys = 120*lam;

    REG_BG2PA = (cosYaw * lam) >> SHIFT_AMT;
    REG_BG2PC = (sinYaw * lam) >> SHIFT_AMT;

    REG_BG2X = sPlayerPosX - ((xs * cosYaw - ys * sinYaw) >> SHIFT_AMT);
    REG_BG2Y = sPlayerPosY - ((xs * sinYaw + ys * cosYaw) >> SHIFT_AMT); 
    //}
    */
    int lam, lcf, lsf, lxr, lyr;

    lam= (10 << 8) * ((1 << 16) / REG_VCOUNT)>>12;  // .8*.16 /.12 = 20.12
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

#if DEBUG
void PrintDebugInfo(void)
{
    const u8 debugText[] = _("Test");
    Menu_PrintText(debugText, 2, 2);
}
#endif

static void CB2_HandleInput(void)
{
    int sinYaw;
    int cosYaw;

    if (gMain.newKeys & START_BUTTON)
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, 0);
        SetVBlankCallback(NULL);
        SetHBlankCallback(NULL);
        SetMainCallback2(CB2_FadeOut);
        return;
    }

    if (gMain.heldKeys & L_BUTTON)
        sPlayerYaw--;
    if (gMain.heldKeys & R_BUTTON)
        sPlayerYaw++;

    sinYaw = gSineTable[sPlayerYaw];
    cosYaw = gSineTable[sPlayerYaw + 64];

    if (gMain.heldKeys & DPAD_UP)
    {
        sPlayerPosX += sinYaw;
        sPlayerPosY -= cosYaw;
    }
    if (gMain.heldKeys & DPAD_DOWN)
    {
        sPlayerPosX -= sinYaw;
        sPlayerPosY += cosYaw;
    }
    if (gMain.heldKeys & DPAD_LEFT)
    {
        sPlayerPosX -= cosYaw;
        sPlayerPosY -= sinYaw;
    }
    if (gMain.heldKeys & DPAD_RIGHT)
    {
        sPlayerPosX += cosYaw;
        sPlayerPosY += sinYaw;
    }

    // Set which pixel will appear at the top left of the screen
    // This is adjusted so that the pixel at (sPlayerPosX, sPlayerPosY) is always
    // at the center of the screen.
    sSoarBgAffine_X = sPlayerPosX - ((DISPLAY_WIDTH / 2) * cosYaw - (DISPLAY_HEIGHT / 2) * sinYaw);
    sSoarBgAffine_Y = sPlayerPosY - ((DISPLAY_HEIGHT / 2) * cosYaw + (DISPLAY_WIDTH / 2) * sinYaw);
    
    // Adjust rotation parameters
    sSoarBgAffine_PA =  cosYaw;
    sSoarBgAffine_PB = -sinYaw;
    sSoarBgAffine_PC =  sinYaw;
    sSoarBgAffine_PD =  cosYaw;

#if DEBUG
    //PrintDebugInfo();
#endif

    //UpdateAffineBGRegs();
}

static void CB2_FadeOut(void)
{
    if (!UpdatePaletteFade())
        SetMainCallback2(sub_805469C);
}
