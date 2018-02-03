#include "global.h"
#include "constants/songs.h"
#include "decompress.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "soar.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "trig.h"

#define DEBUG 1

#define NOCASH_BREAKPOINT asm("mov r11, r11")

static void CB2_LoadSoarGraphics(void);
static void SoarVBlankCallback(void);
static void SoarHBlankCallback(void);
static void EonSpriteCallback(struct Sprite *sprite);
static void CB2_HandleInput(void);
static void CB2_FadeOut(void);

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

static u8 sEonSpriteId;

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

        LZ77UnCompVram(sEonSpriteTiles, (void *)(VRAM + 0x10000));
        // Set up video regs
        REG_DISPCNT = DISPCNT_MODE_1 | DISPCNT_BG2_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP;
        REG_BG2CNT = BGCNT_PRIORITY(0)
                   | BGCNT_256COLOR
                   | BGCNT_CHARBASE(2)
                   | BGCNT_SCREENBASE(8)
                   | BGCNT_AFF512x512
                   | BGCNT_WRAP;
        REG_BLDCNT = BLDCNT_EFFECT_LIGHTEN | BLDCNT_TGT1_BD | BLDCNT_TGT1_BG2;
        //REG_BLDY = 8;
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
            
            // set backdrop color
            ((u16 *)PLTT)[0] = RGB(8, 8, 20);
            
            LoadCompressedObjectPic(&sEonSpriteSheet);
            LoadSpritePalette(&sEonSpritePalette);  // why doesn't this work?
            sEonSpriteId = CreateSprite(&sEonSpriteTemplate, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, 0);
            
            // HACK! Please use the right palette
            gSprites[sEonSpriteId].oam.paletteNum = 0;
            ((u16 *)PLTT)[0x100 + 1] = RGB(4, 4, 16);
        }
        break;
    }
}

static void SoarVBlankCallback(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();

    REG_BG2X = 0;
    REG_BG2Y = 0;
    REG_BG2PA = 0;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 0;
    
    REG_DISPCNT &= ~DISPCNT_BG2_ON;
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

    lam= (10 << 8) * ((1 << 16) / (vcount - 32))>>12;  // .8*.16 /.12 = 20.12
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
        
        REG_IME = 0;
        REG_IE &= ~(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);
        REG_IME = 1;
        REG_DISPSTAT &= ~(DISPSTAT_VBLANK_INTR | DISPSTAT_HBLANK_INTR);

        SetMainCallback2(CB2_FadeOut);
        return;
    }

    /*
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
    */
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
    
    UpdateEonSpriteRotation(&gSprites[sEonSpriteId]);

    sinYaw = gSineTable[sPlayerYaw];
    cosYaw = gSineTable[sPlayerYaw + 64];

    //if (gMain.heldKeys & A_BUTTON)
    {
        sPlayerPosX += sinYaw / 4;
        sPlayerPosY -= cosYaw / 4;
    }
    /*
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
    */

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
    
    BuildOamBuffer();
    AnimateSprites();
}

static void CB2_FadeOut(void)
{
    //if (!UpdatePaletteFade())
        SetMainCallback2(sub_805469C);
}
