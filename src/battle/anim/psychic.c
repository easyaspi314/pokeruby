#include "global.h"
#include "battle.h"
#include "battle_anim.h"
#include "palette.h"
#include "rom_8077ABC.h"
#include "sound.h"
#include "task.h"
#include "trig.h"
#include "constants/songs.h"

extern s16 gBattleAnimArgs[];
extern u8 gAnimBankAttacker;
extern u8 gAnimBankTarget;
extern u16 gBattle_BG1_X;
extern u16 gBattle_BG2_X;
extern u8 gObjectBankIDs[];

extern const union AffineAnimCmd *const gUnknown_083DA888[];
extern struct AffineAnimFrameCmd gUnknown_083DA8A4;
extern struct AffineAnimFrameCmd gUnknown_083DA8C4;

static void sub_80DB88C(struct Sprite *sprite);
static void sub_80DB8C0(struct Sprite *sprite);
static void sub_80DB92C(struct Sprite *sprite);
static void sub_80DB9E4(struct Sprite *sprite);
static void sub_80DBC00(struct Sprite *sprite);
static void sub_80DBC34(struct Sprite *sprite);
static void sub_80DBCD0(u8 taskId);
static void sub_80DBD58(u8 taskId);
void sub_80DBE98(u8 taskId);


void sub_80DB74C(struct Sprite *sprite)
{
    if (GetBankSide(gAnimBankAttacker) == SIDE_PLAYER || IsContest())
    {
        sprite->oam.priority = 2;
        sprite->subpriority = 200;
    }

    if (!IsContest())
    {
        u8 bankCopy;
        u8 bank = bankCopy = GetBankByIdentity(IDENTITY_OPPONENT_MON1);
        u8 identity = GetBankIdentity_permutated(bank);
        int var0 = 1;
        u8 toBG_2 = (identity ^ var0) != 0;

        if (IsAnimBankSpriteVisible(bank))
            sub_8076034(bank, toBG_2);

        bank = bankCopy ^ 2;
        if (IsAnimBankSpriteVisible(bank))
            sub_8076034(bank, toBG_2 ^ var0);
    }

    if (!IsContest() && IsDoubleBattle())
    {
        if (GetBankSide(gAnimBankAttacker) == SIDE_PLAYER)
        {
            sprite->pos1.x = 72;
            sprite->pos1.y = 80;
        }
        else
        {
            sprite->pos1.x = 176;
            sprite->pos1.y = 40;
        }
    }
    else
    {
        if (GetBankSide(gAnimBankAttacker) != SIDE_PLAYER)
            gBattleAnimArgs[0] = -gBattleAnimArgs[0];

        sprite->pos1.x = GetBankPosition(gAnimBankAttacker, 0) + gBattleAnimArgs[0];
        sprite->pos1.y = GetBankPosition(gAnimBankAttacker, 1) + gBattleAnimArgs[1];
    }

    if (IsContest())
        sprite->pos1.y += 9;

    sprite->data[0] = 256 + IndexOfSpritePaletteTag(gBattleAnimArgs[2]) * 16;
    sprite->callback = sub_80DB88C;
    sprite->callback(sprite);
}

static void sub_80DB88C(struct Sprite *sprite)
{
    REG_BLDALPHA = ((16 - sprite->data[3]) << 8) | sprite->data[3];

    if (sprite->data[3] == 13)
        sprite->callback = sub_80DB8C0;
    else
        sprite->data[3]++;
}

static void sub_80DB8C0(struct Sprite *sprite)
{
    u16 color;
    u16 startOffset;
    int i;

    if (++sprite->data[1] == 2)
    {
        sprite->data[1] = 0;
        startOffset = sprite->data[0];
        color = gPlttBufferFaded[startOffset + 8];

        for (i = 8; i > 0; i--)
            gPlttBufferFaded[startOffset + i] = gPlttBufferFaded[startOffset + i - 1];

        gPlttBufferFaded[startOffset + 1] = color;

        if (++sprite->data[2] == 16)
            sprite->callback = sub_80DB92C;
    }
}

static void sub_80DB92C(struct Sprite *sprite)
{
    REG_BLDALPHA = ((16 - sprite->data[3]) << 8) | sprite->data[3];

    if (--sprite->data[3] == -1)
    {
        if (!IsContest())
        {
            u8 bankCopy;
            u8 bank = bankCopy = GetBankByIdentity(IDENTITY_OPPONENT_MON1);

            if (IsAnimBankSpriteVisible(bank))
                gSprites[gObjectBankIDs[bank]].invisible = 0;

            bank = bankCopy ^ 2;
            if (IsAnimBankSpriteVisible(bank))
                gSprites[gObjectBankIDs[bank]].invisible = 0;
        }

        sprite->invisible = 1;
        sprite->callback = sub_80DB9E4;
    }
}

static void sub_80DB9E4(struct Sprite *sprite)
{
    if (!IsContest())
    {
        u8 bankCopy;
        u8 bank = bankCopy = GetBankByIdentity(IDENTITY_OPPONENT_MON1);
        u8 identity = GetBankIdentity_permutated(bank);
        int var0 = 1;
        u8 toBG_2 = (identity ^ var0) != 0;

        if (IsAnimBankSpriteVisible(bank))
            sub_8076464(toBG_2);

        bank = bankCopy ^ 2;
        if (IsAnimBankSpriteVisible(bank))
            sub_8076464(toBG_2 ^ var0);
    }

    sprite->callback = DestroyAnimSprite;
}

void sub_80DBA4C(struct Sprite *sprite)
{
    if (sprite->data[0] == 0)
    {
        int arg3 = gBattleAnimArgs[3];
        u8 var0 = 0;
        if (arg3 == 0)
            var0 = 1;

        if (!IsContest() && IsDoubleBattle())
        {
            if (GetBankSide(gAnimBankAttacker) == SIDE_PLAYER)
            {
                sprite->pos1.x = 72 - gBattleAnimArgs[0];
                sprite->pos1.y = gBattleAnimArgs[1] + 80;
            }
            else
            {
                sprite->pos1.x = gBattleAnimArgs[0] + 176;
                sprite->pos1.y = gBattleAnimArgs[1] + 40;
            }
        }
        else
        {
            if (gBattleAnimArgs[2] == 0)
                sub_80787B0(sprite, var0);
            else
                sub_8078764(sprite, var0);
        }

        sprite->data[0]++;
    }
    else
    {
        if (sprite->animEnded || sprite->affineAnimEnded)
            move_anim_8074EE0(sprite);
    }
}

void sub_80DBAF4(struct Sprite *sprite)
{
    sprite->pos1.x = GetBankPosition(gAnimBankAttacker, 2);
    sprite->pos1.y = GetBankPosition(gAnimBankAttacker, 3);

    if (GetBankSide(gAnimBankAttacker) != SIDE_PLAYER)
    {
        StartSpriteAnim(sprite, 1);
        sprite->pos1.x -= 40;
        sprite->pos1.y += 10;
        sprite->data[1] = -1;
    }
    else
    {
        sprite->pos1.x += 40;
        sprite->pos1.y -= 10;
        sprite->data[1] = 1;
    }

    StoreSpriteCallbackInData(sprite, DestroyAnimSprite);
    sprite->callback = sub_8078600;
}

void sub_80DBB70(struct Sprite *sprite)
{
    s16 x = sub_807A100(gAnimBankAttacker, 1) /  2;
    s16 y = sub_807A100(gAnimBankAttacker, 0) / -2;

    if (GetBankSide(gAnimBankAttacker) == SIDE_OPPONENT)
    {
        x = -x;
    }

    sprite->pos1.x = GetBankPosition(gAnimBankAttacker, 2) + x;
    sprite->pos1.y = GetBankPosition(gAnimBankAttacker, 3) + y;

    if (sprite->pos1.y < 16)
    {
        sprite->pos1.y = 16;
    }

    StoreSpriteCallbackInData(sprite, sub_80DBC00);
    sprite->callback = sub_8078600;
}

static void sub_80DBC00(struct Sprite *sprite)
{
    sprite->oam.affineMode = 1;
    sprite->affineAnims = gUnknown_083DA888;
    sprite->data[0] = 0;
    InitSpriteAffineAnim(sprite);
    sprite->callback = sub_80DBC34;
}

static void sub_80DBC34(struct Sprite *sprite)
{
    switch (sprite->data[0])
    {
    case 0:
        if (sprite->affineAnimEnded)
        {
            FreeOamMatrix(sprite->oam.matrixNum);
            sprite->oam.affineMode = 0;
            sprite->data[1] = 18;
            sprite->data[0]++;
        }
        break;
    case 1:
        if (--sprite->data[1] == -1)
        {
            DestroyAnimSprite(sprite);
        }
        break;
    }
}

void sub_80DBC94(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    u8 spriteId = GetAnimBankSpriteId(0);
    task->data[0] = spriteId;
    sub_80798F4(task, spriteId, &gUnknown_083DA8A4);
    task->func = sub_80DBCD0;
}

static void sub_80DBCD0(u8 taskId)
{
    if (!sub_807992C(&gTasks[taskId]))
    {
        DestroyAnimVisualTask(taskId);
    }
}

void sub_80DBCFC(u8 taskId)
{
    struct Task *task = &gTasks[taskId];
    u8 spriteId = GetAnimBankSpriteId(0);
    task->data[0] = spriteId;
    task->data[1] = 0;
    task->data[2] = 0;
    task->data[3] = GetBankSide(gAnimBankAttacker) != SIDE_PLAYER ? 4 : 8;

    sub_80798F4(task, task->data[0], &gUnknown_083DA8C4);
    task->func = sub_80DBD58;
}

static void sub_80DBD58(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->data[1])
    {
    case 0:
        sub_807992C(task);
        if (++task->data[2] > 19)
            task->data[1]++;
        break;
    case 1:
        if (task->data[3] != 0)
        {
            gSprites[task->data[0]].pos2.y -= 8;
            task->data[3]--;
        }
        else
        {
            gSprites[task->data[0]].invisible = 1;
            gSprites[task->data[0]].pos1.x = 272;
            sub_8078F40(task->data[0]);
            DestroyAnimVisualTask(taskId);
        }
        break;
    }
}

void sub_80DBE00(u8 taskId)
{
    u16 var0, var1;

    struct Task *task = &gTasks[taskId];

    task->data[3] = 16;
    task->data[4] = 0;
    task->data[13] = GetBankPosition(gAnimBankAttacker, 2);
    task->data[14] = GetBankPosition(gAnimBankAttacker, 3);

    var0 = sub_807A100(gAnimBankAttacker, 1) / 3;
    var1 = sub_807A100(gAnimBankAttacker, 0) / 3;
    task->data[12] = var0 > var1 ? var0 : var1;

    REG_BLDCNT = 0x3F40;
    REG_BLDALPHA = 0x10;

    task->func = sub_80DBE98;
}
