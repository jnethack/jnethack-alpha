/* NetHack 3.6	objects.c	$NHDT-Date: 1447313395 2015/11/12 07:29:55 $  $NHDT-Branch: master $:$NHDT-Revision: 1.49 $ */
/* Copyright (c) Mike Threepoint, 1989.                           */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * The data in this file is processed twice, to construct two arrays.
 * On the first pass, only object name and object description matter.
 * On the second pass, all object-class fields except those two matter.
 * 2nd pass is a recursive inclusion of this file, not a 2nd compilation.
 * The name/description array is also used by makedefs and lev_comp.
 *
 * #ifndef OBJECTS_PASS_2_
 * # define OBJECT(name,desc,foo,bar,glorkum) name,desc
 * struct objdescr obj_descr[] =
 * #else
 * # define OBJECT(name,desc,foo,bar,glorkum) foo,bar,glorkum
 * struct objclass objects[] =
 * #endif
 * {
 *   { OBJECT("strange object",NULL, 1,2,3) },
 *   { OBJECT("arrow","pointy stick", 4,5,6) },
 *   ...
 *   { OBJECT(NULL,NULL, 0,0,0) }
 * };
 * #define OBJECTS_PASS_2_
 * #include "objects.c"
 */

/* *INDENT-OFF* */
/* clang-format off */

#ifndef OBJECTS_PASS_2_
/* first pass */
struct monst { struct monst *dummy; };  /* lint: struct obj's union */
#include "config.h"
#include "obj.h"
#include "objclass.h"
#include "prop.h"
#include "skills.h"

#else /* !OBJECTS_PASS_2_ */
/* second pass */
#include "color.h"
#define COLOR_FIELD(X) X,
#endif /* !OBJECTS_PASS_2_ */

/* objects have symbols: ) [ = " ( % ! ? + / $ * ` 0 _ . */

/*
 *      Note:  OBJ() and BITS() macros are used to avoid exceeding argument
 *      limits imposed by some compilers.  The ctnr field of BITS currently
 *      does not map into struct objclass, and is ignored in the expansion.
 *      The 0 in the expansion corresponds to oc_pre_discovered, which is
 *      set at run-time during role-specific character initialization.
 */

#ifndef OBJECTS_PASS_2_
/* first pass -- object descriptive text */
#define OBJ(name,desc)  name, desc
#define OBJECT(obj,bits,prp,sym,prob,dly,wt, \
               cost,sdam,ldam,oc1,oc2,nut,color)  { obj }
#define None (char *) 0 /* less visual distraction for 'no description' */

NEARDATA struct objdescr obj_descr[] =
#else
/* second pass -- object definitions */
#define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl) \
  nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,dir,mtrl,sub /*SCO cpp fodder*/
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
  { 0, 0, (char *) 0, bits, prp, sym, dly, COLOR_FIELD(color) prob, wt, \
    cost, sdam, ldam, oc1, oc2, nut }
#ifndef lint
#define HARDGEM(n) (n >= 8)
#else
#define HARDGEM(n) (0)
#endif

NEARDATA struct objclass objects[] =
#endif
{
/* dummy object[0] -- description [2nd arg] *must* be NULL */
OBJECT(OBJ("奇妙な物体", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0),
       0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

/* weapons ... */
#define WEAPON(name,desc,kn,mg,bi,prob,wt,                \
               cost,sdam,ldam,hitbon,typ,sub,metal,color) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, mg, 1, 0, 0, 1, 0, 0, bi, 0, typ, sub, metal),  \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color)
#define PROJECTILE(name,desc,kn,prob,wt,                  \
                   cost,sdam,ldam,hitbon,metal,sub,color) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, PIERCE, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color)
#define BOW(name,desc,kn,prob,wt,cost,hitbon,metal,sub,color) \
    OBJECT(OBJ(name,desc),                                          \
           BITS(kn, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, sub, metal),      \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, 2, 2, hitbon, 0, wt, color)

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
   the extra damage is added on in weapon.c, not here! */

#define P PIERCE
#define S SLASH
#define B WHACK

/* missiles; materiel reflects the arrowhead, not the shaft */
PROJECTILE("矢", None,
           1, 55, 1, 2, 6, 6, 0,        IRON, -P_BOW, HI_METAL),
PROJECTILE("エルフの矢", "神秘的な矢",
           0, 20, 1, 2, 7, 6, 0,        WOOD, -P_BOW, HI_WOOD),
PROJECTILE("オークの矢", "粗末な矢",
           0, 20, 1, 2, 5, 6, 0,        IRON, -P_BOW, CLR_BLACK),
PROJECTILE("銀の矢", None,
           1, 12, 1, 5, 6, 6, 0,        SILVER, -P_BOW, HI_SILVER),
PROJECTILE("竹矢", "竹の矢",
           0, 15, 1, 4, 7, 7, 1,        METAL, -P_BOW, HI_METAL),
PROJECTILE("クロスボゥボルト", None,
           1, 55, 1, 2, 4, 6, 0,        IRON, -P_CROSSBOW, HI_METAL),

/* missiles that don't use a launcher */
WEAPON("投げ矢", None,
       1, 1, 0, 60,   1,   2,  3,  2, 0, P,   -P_DART, IRON, HI_METAL),
WEAPON("手裏剣", "星型の投げるもの",
       0, 1, 0, 35,   1,   5,  8,  6, 2, P,   -P_SHURIKEN, IRON, HI_METAL),
WEAPON("ブーメラン", None,
       1, 1, 0, 15,   5,  20,  9,  9, 0, 0,   -P_BOOMERANG, WOOD, HI_WOOD),

/* spears [note: javelin used to have a separate skill from spears,
   because the latter are primarily stabbing weapons rather than
   throwing ones; but for playability, they've been merged together
   under spear skill and spears can now be thrown like javelins] */
WEAPON("槍", None,
       1, 1, 0, 50,  30,   3,  6,  8, 0, P,   P_SPEAR, IRON, HI_METAL),
WEAPON("エルフの槍", "神秘的な槍",
       0, 1, 0, 10,  30,   3,  7,  8, 0, P,   P_SPEAR, WOOD, HI_WOOD),
WEAPON("オークの槍", "粗末な槍",
       0, 1, 0, 13,  30,   3,  5,  8, 0, P,   P_SPEAR, IRON, CLR_BLACK),
WEAPON("ドワーフの槍", "丈夫な槍",
       0, 1, 0, 12,  35,   3,  8,  8, 0, P,   P_SPEAR, IRON, HI_METAL),
WEAPON("銀の槍", None,
       1, 1, 0,  2,  36,  40,  6,  8, 0, P,   P_SPEAR, SILVER, HI_SILVER),
WEAPON("ジャベリン", "投げ槍",
       0, 1, 0, 10,  20,   3,  6,  6, 0, P,   P_SPEAR, IRON, HI_METAL),

/* spearish; doesn't stack, not intended to be thrown */
WEAPON("トライデント", None,
       1, 0, 0,  8,  25,   5,  6,  4, 0, P,   P_TRIDENT, IRON, HI_METAL),
        /* +1 small, +2d4 large */

/* blades; all stack */
WEAPON("短剣", None,
       1, 1, 0, 30,  10,   4,  4,  3, 2, P,   P_DAGGER, IRON, HI_METAL),
WEAPON("エルフの短剣", "神秘的な短剣",
       0, 1, 0, 10,  10,   4,  5,  3, 2, P,   P_DAGGER, WOOD, HI_WOOD),
WEAPON("オークの短剣", "粗末な短剣",
       0, 1, 0, 12,  10,   4,  3,  3, 2, P,   P_DAGGER, IRON, CLR_BLACK),
WEAPON("銀の短剣", None,
       1, 1, 0,  3,  12,  40,  4,  3, 2, P,   P_DAGGER, SILVER, HI_SILVER),
WEAPON("アサメ", None,
       1, 1, 0,  0,  10,   4,  4,  3, 2, S,   P_DAGGER, IRON, HI_METAL),
WEAPON("メス", None,
       1, 1, 0,  0,   5,   6,  3,  3, 2, S,   P_KNIFE, METAL, HI_METAL),
WEAPON("ナイフ", None,
       1, 1, 0, 20,   5,   4,  3,  2, 0, P|S, P_KNIFE, IRON, HI_METAL),
WEAPON("スティレット", None,
       1, 1, 0,  5,   5,   4,  3,  2, 0, P|S, P_KNIFE, IRON, HI_METAL),
/* 3.6: worm teeth and crysknives now stack;
   when a stack of teeth is enchanted at once, they fuse into one crysknife;
   when a stack of crysknives drops, the whole stack reverts to teeth */
WEAPON("ワームの歯", None,
       1, 1, 0,  0,  20,   2,  2,  2, 0, 0,   P_KNIFE, 0, CLR_WHITE),
WEAPON("クリスナイフ", None,
       1, 1, 0,  0,  20, 100, 10, 10, 3, P,   P_KNIFE, MINERAL, CLR_WHITE),

/* axes */
WEAPON("斧", None,
       1, 0, 0, 40,  60,   8,  6,  4, 0, S,   P_AXE, IRON, HI_METAL),
WEAPON("戦斧", "両刃の斧",       /* "double-bitted"? */
       0, 0, 1, 10, 120,  40,  8,  6, 0, S,   P_AXE, IRON, HI_METAL),

/* swords */
WEAPON("小剣", None,
       1, 0, 0,  8,  30,  10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL),
WEAPON("エルフの小剣", "神秘的な小剣",
       0, 0, 0,  2,  30,  10,  8,  8, 0, P,   P_SHORT_SWORD, WOOD, HI_WOOD),
WEAPON("オークの小剣", "粗末な小剣",
       0, 0, 0,  3,  30,  10,  5,  8, 0, P,   P_SHORT_SWORD, IRON, CLR_BLACK),
WEAPON("ドワーフの小剣", "幅広の小剣",
       0, 0, 0,  2,  30,  10,  7,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL),
WEAPON("シミター", "曲った剣",
       0, 0, 0, 15,  40,  15,  8,  8, 0, S,   P_SCIMITAR, IRON, HI_METAL),
WEAPON("銀のサーベル", None,
       1, 0, 0,  6,  40,  75,  8,  8, 0, S,   P_SABER, SILVER, HI_SILVER),
WEAPON("幅広の剣", None,
       1, 0, 0,  8,  70,  10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, HI_METAL),
        /* +d4 small, +1 large */
WEAPON("エルフの幅広の剣", "神秘的な幅広の剣",
       0, 0, 0,  4,  70,  10,  6,  6, 0, S,   P_BROAD_SWORD, WOOD, HI_WOOD),
        /* +d4 small, +1 large */
WEAPON("長剣", None,
       1, 0, 0, 50,  40,  15,  8, 12, 0, S,   P_LONG_SWORD, IRON, HI_METAL),
WEAPON("両手剣", None,
       1, 0, 1, 22, 150,  50, 12,  6, 0, S,   P_TWO_HANDED_SWORD,
                                                            IRON, HI_METAL),
        /* +2d6 large */
WEAPON("刀", "侍の剣",
       0, 0, 0,  4,  40,  80, 10, 12, 1, S,   P_LONG_SWORD, IRON, HI_METAL),
/* special swords set up for artifacts */
WEAPON("大刀", "侍の長剣",
       0, 0, 1,  0,  60, 500, 16,  8, 2, S,   P_TWO_HANDED_SWORD,
                                                            METAL, HI_METAL),
        /* +2d6 large */
WEAPON("ルーンの剣", "神秘的な幅広の剣",
       0, 0, 0,  0,  40, 300,  4,  6, 0, S,   P_BROAD_SWORD, IRON, CLR_BLACK),
        /* +d4 small, +1 large; Stormbringer: +5d2 +d8 from level drain */

/* polearms */
/* spear-type */
WEAPON("パルチザン", "粗雑な長斧",
       0, 0, 1,  5,  80,  10,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL),
        /* +1 large */
WEAPON("ランサー", "柄付の長斧",
       0, 0, 1,  5,  50,   6,  4,  4, 0, P,   P_POLEARMS, IRON, HI_METAL),
        /* +d4 both */
WEAPON("スペタム", "フォーク付き長斧",
       0, 0, 1,  5,  50,   5,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL),
        /* +1 small, +d6 large */
WEAPON("グレイブ", "片刃長斧",
       0, 0, 1,  8,  75,   6,  6, 10, 0, S,   P_POLEARMS, IRON, HI_METAL),
WEAPON("ランス", None,
       1, 0, 0,  4, 180,  10,  6,  8, 0, P,   P_LANCE, IRON, HI_METAL),
        /* +2d10 when jousting with lance as primary weapon */
/* axe-type */
WEAPON("ハルバード", "曲ったまさかり",
       0, 0, 1,  8, 150,  10, 10,  6, 0, P|S, P_POLEARMS, IRON, HI_METAL),
        /* +1d6 large */
WEAPON("バーディック", "長いまさかり",
       0, 0, 1,  4, 120,   7,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small, +2d4 large */
WEAPON("ヴォウジェ", "包丁付き竿",
       0, 0, 1,  4, 125,   5,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL),
        /* +d4 both */
WEAPON("ドワーフのつるはし", "幅広のつるはし",
       0, 0, 1, 13, 120,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_METAL),
/* curved/hooked */
WEAPON("フォシャール", "鎌付き竿",
       0, 0, 1,  6,  60,   5,  6,  8, 0, P|S, P_POLEARMS, IRON, HI_METAL),
WEAPON("ギザルム", "刈り込みがま",
       0, 0, 1,  6,  80,   5,  4,  8, 0, S,   P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small */
WEAPON("ビル・ギザルム", "鈎付き長斧",
       0, 0, 1,  4, 120,   7,  4, 10, 0, P|S, P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small */
/* other */
WEAPON("ルッツェルンハンマー", "二股の長斧",
       0, 0, 1,  5, 150,   7,  4,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL),
        /* +1d4 small */
WEAPON("ベック・ド・コルバン", "くちばし付き長斧",
       0, 0, 1,  4, 100,   8,  8,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL),

/* bludgeons */
WEAPON("メイス", None,
       1, 0, 0, 40,  30,   5,  6,  6, 0, B,   P_MACE, IRON, HI_METAL),
        /* +1 small */
WEAPON("モーニングスター", None,
       1, 0, 0, 12, 120,  10,  4,  6, 0, B,   P_MORNING_STAR, IRON, HI_METAL),
        /* +d4 small, +1 large */
WEAPON("ウォーハンマー", None,
       1, 0, 0, 15,  50,   5,  4,  4, 0, B,   P_HAMMER, IRON, HI_METAL),
        /* +1 small */
WEAPON("こん棒", None,
       1, 0, 0, 12,  30,   3,  6,  3, 0, B,   P_CLUB, WOOD, HI_WOOD),
WEAPON("ゴムホース", None,
       1, 0, 0,  0,  20,   3,  4,  3, 0, B,   P_WHIP, PLASTIC, CLR_BROWN),
WEAPON("六尺棒", "棒",
       0, 0, 1, 11,  40,   5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),
/* two-piece */
WEAPON("アキリス", "紐付のこん棒",
       0, 0, 0,  8,  15,   4,  6,  3, 0, B,   P_CLUB, IRON, HI_METAL),
WEAPON("フレイル", None,
       1, 0, 0, 40,  15,   4,  6,  4, 0, B,   P_FLAIL, IRON, HI_METAL),
        /* +1 small, +1d4 large */

/* misc */
WEAPON("鞭", None,
       1, 0, 0,  2,  20,   4,  2,  1, 0, 0,   P_WHIP, LEATHER, CLR_BROWN),

/* bows */
BOW("弓", None,                 1, 24, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("エルフの弓", "神秘的な弓", 0, 12, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("オークの弓", "粗末な弓",   0, 12, 30, 60, 0, WOOD, P_BOW, CLR_BLACK),
BOW("和弓", "長弓",             0,  0, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("スリング", None,           1, 40,  3, 20, 0, LEATHER, P_SLING, HI_LEATHER),
BOW("クロスボゥ", None,         1, 45, 50, 40, 0, WOOD, P_CROSSBOW, HI_WOOD),

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW

/* armor ... */
        /* IRON denotes ferrous metals, including steel.
         * Only IRON weapons and armor can rust.
         * Only COPPER (including brass) corrodes.
         * Some creatures are vulnerable to SILVER.
         */
#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,  \
              cost,ac,can,sub,metal,c)                   \
    OBJECT(OBJ(name, desc),                                         \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, blk, 0, 0, sub, metal),  \
           power, ARMOR_CLASS, prob, delay, wt,                     \
           cost, 0, 0, 10 - ac, can, wt, c)
#define HELM(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_HELM, metal, c)
#define CLOAK(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_CLOAK, metal, c)
#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c) \
    ARMOR(name, desc, kn, mgc, blk, power, prob, delay, wt, \
          cost, ac, can, ARM_SHIELD, metal, c)
#define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_GLOVES, metal, c)
#define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c)  \
    ARMOR(name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_BOOTS, metal, c)

/* helmets */
HELM("エルフの革帽子", "革帽子",
     0, 0,           0,  6, 1,  3,  8,  9, 0, LEATHER, HI_LEATHER),
HELM("オークの兜", "鉄の帽子",
     0, 0,           0,  6, 1, 30, 10,  9, 0, IRON, CLR_BLACK),
HELM("ドワーフの鉄兜", "固い帽子",
     0, 0,           0,  6, 1, 40, 20,  8, 0, IRON, HI_METAL),
HELM("フィドーラ", None,
     1, 0,           0,  0, 0,  3,  1, 10, 0, CLOTH, CLR_BROWN),
HELM("とんがり帽子", "円錐形の帽子",
     0, 1, CLAIRVOYANT,  3, 1,  4, 80, 10, 1, CLOTH, CLR_BLUE),
        /* name coined by devteam; confers clairvoyance for wizards,
           blocks clairvoyance if worn by role other than wizard */
HELM("低能帽", "円錐形の帽子",
     0, 1,           0,  3, 1,  4,  1, 10, 0, CLOTH, CLR_BLUE),
HELM("くぼんだ鍋", None,
     1, 0,           0,  2, 0, 10,  8,  9, 0, IRON, CLR_BLACK),
/* with shuffled appearances... */
HELM("兜", "羽兜",
     0, 0,           0, 10, 1, 30, 10,  9, 0, IRON, HI_METAL),
HELM("知性の兜", "模様入り兜",
     0, 1,           0,  6, 1, 50, 50,  9, 0, IRON, CLR_GREEN),
HELM("逆属性の兜", "とさかの兜",
     0, 1,           0,  6, 1, 50, 50,  9, 0, IRON, HI_METAL),
HELM("テレパシーの兜", "面頬付きの兜",
     0, 1,     TELEPAT,  2, 1, 50, 50,  9, 0, IRON, HI_METAL),

/* suits of armor */
/*
 * There is code in polyself.c that assumes (1) and (2).
 * There is code in obj.h, objnam.c, mon.c, read.c that assumes (2).
 *      (1) The dragon scale mails and the dragon scales are together.
 *      (2) That the order of the dragon scale mail and dragon scales
 *          is the the same as order of dragons defined in monst.c.
 */
#define DRGN_ARMR(name,mgc,power,cost,ac,color)  \
    ARMOR(name, None, 1, mgc, 1, power, 0, 5, 40,  \
          cost, ac, 0, ARM_SUIT, DRAGON_HIDE, color)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_ARMR("灰色ドラゴンの鱗鎧",     1, ANTIMAGIC,  1200, 1, CLR_GRAY),
DRGN_ARMR("銀色ドラゴンの鱗鎧",     1, REFLECTING, 1200, 1, DRAGON_SILVER),
#if 0 /* DEFERRED */
DRGN_ARMR("shimmering dragon scale mail", 1, DISPLACED, 1200, 1, CLR_CYAN),
#endif
DRGN_ARMR("赤色ドラゴンの鱗鎧",     1, FIRE_RES,    900, 1, CLR_RED),
DRGN_ARMR("白ドラゴンの鱗鎧",       1, COLD_RES,    900, 1, CLR_WHITE),
DRGN_ARMR("オレンジドラゴンの鱗鎧", 1, SLEEP_RES,   900, 1, CLR_ORANGE),
DRGN_ARMR("黒ドラゴンの鱗鎧",       1, DISINT_RES, 1200, 1, CLR_BLACK),
DRGN_ARMR("青ドラゴンの鱗鎧",       1, SHOCK_RES,   900, 1, CLR_BLUE),
DRGN_ARMR("緑ドラゴンの鱗鎧",       1, POISON_RES,  900, 1, CLR_GREEN),
DRGN_ARMR("黄色ドラゴンの鱗鎧",     1, ACID_RES,    900, 1, CLR_YELLOW),
/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer
   magical properties but are produced "naturally" */
DRGN_ARMR("灰色ドラゴンの鱗",       0, ANTIMAGIC,   700, 7, CLR_GRAY),
DRGN_ARMR("銀色ドラゴンの鱗",       0, REFLECTING,  700, 7, DRAGON_SILVER),
#if 0 /* DEFERRED */
DRGN_ARMR("shimmering dragon scales",  0, DISPLACED,   700, 7, CLR_CYAN),
#endif
DRGN_ARMR("赤色ドラゴンの鱗",       0, FIRE_RES,    500, 7, CLR_RED),
DRGN_ARMR("白ドラゴンの鱗",         0, COLD_RES,    500, 7, CLR_WHITE),
DRGN_ARMR("オレンジドラゴンの鱗",   0, SLEEP_RES,   500, 7, CLR_ORANGE),
DRGN_ARMR("黒ドラゴンの鱗",         0, DISINT_RES,  700, 7, CLR_BLACK),
DRGN_ARMR("青ドラゴンの鱗",         0, SHOCK_RES,   500, 7, CLR_BLUE),
DRGN_ARMR("緑ドラゴンの鱗",         0, POISON_RES,  500, 7, CLR_GREEN),
DRGN_ARMR("黄色ドラゴンの鱗",       0, ACID_RES,    500, 7, CLR_YELLOW),
#undef DRGN_ARMR
/* other suits */
ARMOR("鋼鉄の鎧", None,
      1, 0, 1,  0, 44, 5, 450, 600,  3, 2,  ARM_SUIT, IRON, HI_METAL),
ARMOR("水晶の鎧", None,
      1, 0, 1,  0, 10, 5, 450, 820,  3, 2,  ARM_SUIT, GLASS, CLR_WHITE),
ARMOR("青銅の鎧", None,
      1, 0, 1,  0, 25, 5, 450, 400,  4, 1,  ARM_SUIT, COPPER, HI_COPPER),
ARMOR("鉄片の鎧", None,
      1, 0, 1,  0, 62, 5, 400,  80,  4, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("帯金の鎧", None,
      1, 0, 1,  0, 72, 5, 350,  90,  4, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("ドワーフのミスリル服", None,
      1, 0, 0,  0, 10, 1, 150, 240,  4, 2,  ARM_SUIT, MITHRIL, HI_SILVER),
ARMOR("エルフのミスリル服", None,
      1, 0, 0,  0, 15, 1, 150, 240,  5, 2,  ARM_SUIT, MITHRIL, HI_SILVER),
ARMOR("鎖かたびら", None,
      1, 0, 0,  0, 72, 5, 300,  75,  5, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("オークの鎖かたびら", "粗末な鎖かたびら",
      0, 0, 0,  0, 20, 5, 300,  75,  6, 1,  ARM_SUIT, IRON, CLR_BLACK),
ARMOR("鱗の鎧", None,
      1, 0, 0,  0, 72, 5, 250,  45,  6, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("鋲付き革鎧", None,
      1, 0, 0,  0, 72, 3, 200,  15,  7, 1,  ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("鉄環の鎧", None,
      1, 0, 0,  0, 72, 5, 250, 100,  7, 1,  ARM_SUIT, IRON, HI_METAL),
ARMOR("オークの鉄環の鎧", "粗末な鉄環の鎧",
      0, 0, 0,  0, 20, 5, 250,  80,  8, 1,  ARM_SUIT, IRON, CLR_BLACK),
ARMOR("革鎧", None,
      1, 0, 0,  0, 82, 3, 150,   5,  8, 1,  ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("革の服", None,
      1, 0, 0,  0, 12, 0,  30,  10,  9, 0,  ARM_SUIT, LEATHER, CLR_BLACK),

/* shirts */
ARMOR("アロハシャツ", None,
      1, 0, 0,  0,  8, 0,   5,   3, 10, 0,  ARM_SHIRT, CLOTH, CLR_MAGENTA),
ARMOR("Ｔシャツ", None,
      1, 0, 0,  0,  2, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, CLR_WHITE),

/* cloaks */
CLOAK("ミイラの包帯", None,
      1, 0,          0,  0, 0,  3,  2, 10, 1,  CLOTH, CLR_GRAY),
        /* worn mummy wrapping blocks invisibility */
CLOAK("エルフのクローク", "陰気な外套",
      0, 1,    STEALTH,  8, 0, 10, 60,  9, 1,  CLOTH, CLR_BLACK),
CLOAK("オークのクローク", "粗末なマント",
      0, 0,          0,  8, 0, 10, 40, 10, 1,  CLOTH, CLR_BLACK),
CLOAK("ドワーフのクローク", "フードつきのクローク",
      0, 0,          0,  8, 0, 10, 50, 10, 1,  CLOTH, HI_CLOTH),
CLOAK("防水クローク", "つるつるしたクローク",
      0, 0,          0,  8, 0, 10, 50,  9, 2,  CLOTH, HI_CLOTH),
CLOAK("ローブ", None,
      1, 1,          0,  3, 0, 15, 50,  8, 2,  CLOTH, CLR_RED),
        /* robe was adopted from slash'em, where it's worn as a suit
           rather than as a cloak and there are several variations */
CLOAK("錬金術の仕事着", "エプロン",
      0, 1, POISON_RES,  9, 0, 10, 50,  9, 1,  CLOTH, CLR_WHITE),
CLOAK("革のクローク", None,
      1, 0,          0,  8, 0, 15, 40,  9, 1,  LEATHER, CLR_BROWN),
/* with shuffled appearances... */
CLOAK("守りのクローク", "ぼろぼろのケープ",
      0, 1, PROTECTION,  9, 0, 10, 50,  7, 3,  CLOTH, HI_CLOTH),
        /* cloak of protection is now the only item conferring MC 3 */
CLOAK("透明のクローク", "オペラクローク",
      0, 1,      INVIS, 10, 0, 10, 60,  9, 1,  CLOTH, CLR_BRIGHT_MAGENTA),
CLOAK("魔法を防ぐクローク", "装飾用の外套",
      0, 1,  ANTIMAGIC,  2, 0, 10, 60,  9, 1,  CLOTH, CLR_WHITE),
        /*  'cope' is not a spelling mistake... leave it be */
CLOAK("幻影のクローク", "布切れ",
      0, 1,  DISPLACED, 10, 0, 10, 50,  9, 1,  CLOTH, HI_CLOTH),

/* shields */
SHIELD("小さな盾", None,
       1, 0, 0,          0, 6, 0,  30,  3, 9, 0,  WOOD, HI_WOOD),
SHIELD("エルフの盾", "青と緑の盾",
       0, 0, 0,          0, 2, 0,  40,  7, 8, 0,  WOOD, CLR_GREEN),
SHIELD("ウルク・ハイの盾", "白の手の盾",
       0, 0, 0,          0, 2, 0,  50,  7, 9, 0,  IRON, HI_METAL),
SHIELD("オークの盾", "赤い目の盾",
       0, 0, 0,          0, 2, 0,  50,  7, 9, 0,  IRON, CLR_RED),
SHIELD("大きな盾", None,
       1, 0, 1,          0, 7, 0, 100, 10, 8, 0,  IRON, HI_METAL),
SHIELD("ドワーフの丸盾", "大きな丸盾",
       0, 0, 0,          0, 4, 0, 100, 10, 8, 0,  IRON, HI_METAL),
SHIELD("反射の盾", "銀色の磨かれた盾",
       0, 1, 0, REFLECTING, 3, 0,  50, 50, 8, 0,  SILVER, HI_SILVER),

/* gloves */
/* These have their color but not material shuffled, so the IRON must
 * stay CLR_BROWN (== HI_LEATHER) even though it's normally either
 * HI_METAL or CLR_BLACK.  All have shuffled descriptions.
 */
GLOVES("革の手袋", "古い手袋",
       0, 0,        0, 16, 1, 10,  8, 9, 0,  LEATHER, HI_LEATHER),
GLOVES("お手玉の小手", "詰めもののある手袋",
       0, 1, FUMBLING,  8, 1, 10, 50, 9, 0,  LEATHER, HI_LEATHER),
GLOVES("力の小手", "乗馬用の手袋",
       0, 1,        0,  8, 1, 30, 50, 9, 0,  IRON, CLR_BROWN),
GLOVES("器用さの小手", "フェンシングの小手",
       0, 1,        0,  8, 1, 10, 50, 9, 0,  LEATHER, HI_LEATHER),

/* boots */
BOOTS("かかとの低い靴", "散歩用の靴",
      0, 0,          0, 25, 2, 10,  8, 9, 0, LEATHER, HI_LEATHER),
BOOTS("鉄の靴", "固い靴",
      0, 0,          0,  7, 2, 50, 16, 8, 0, IRON, HI_METAL),
BOOTS("かかとの高い靴", "軍隊靴",
      0, 0,          0, 15, 2, 20, 12, 8, 0, LEATHER, HI_LEATHER),
/* with shuffled appearances... */
BOOTS("韋駄天の靴", "戦闘靴",
      0, 1,       FAST, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
BOOTS("水上歩行の靴", "ジャングルの靴",
      0, 1,   WWALKING, 12, 2, 15, 50, 9, 0, LEATHER, HI_LEATHER),
BOOTS("飛び跳ねる靴", "ハイキングの靴",
      0, 1,    JUMPING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
BOOTS("エルフの靴", "長靴",
      0, 1,    STEALTH, 12, 2, 15,  8, 9, 0, LEATHER, HI_LEATHER),
BOOTS("蹴り挙げ靴", "留め金のある靴",
      0, 1,          0, 12, 2, 50,  8, 9, 0, IRON, CLR_BROWN),
        /* CLR_BROWN for same reason as gauntlets of power */
BOOTS("つまずきの靴", "乗馬用の靴",
      0, 1,   FUMBLING, 12, 2, 20, 30, 9, 0, LEATHER, HI_LEATHER),
BOOTS("浮遊の靴", "雪靴",
      0, 1, LEVITATION, 12, 2, 15, 30, 9, 0, LEATHER, HI_LEATHER),
#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

/* rings ... */
#define RING(name,stone,power,cost,mgc,spec,mohs,metal,color) \
    OBJECT(OBJ(name, stone),                                          \
           BITS(0, 0, spec, 0, mgc, spec, 0, 0, 0,                    \
                HARDGEM(mohs), 0, P_NONE, metal),                     \
           power, RING_CLASS, 0, 0, 3, cost, 0, 0, 0, 0, 15, color)
RING("飾りの指輪", "木の指輪",
     ADORNED,                  100, 1, 1, 2, WOOD, HI_WOOD),
RING("強さの指輪", "花崗岩の指輪",
     0,                        150, 1, 1, 7, MINERAL, HI_MINERAL),
RING("体力の指輪", "オパールの指輪",
     0,                        150, 1, 1, 7, MINERAL, HI_MINERAL),
RING("命中の指輪", "土の指輪",
     0,                        150, 1, 1, 4, MINERAL, CLR_RED),
RING("攻撃の指輪", "珊瑚の指輪",
     0,                        150, 1, 1, 4, MINERAL, CLR_ORANGE),
RING("守りの指輪", "黒めのうの指輪",
     PROTECTION,               100, 1, 1, 7, MINERAL, CLR_BLACK),
        /* 'PROTECTION' intrinsic enhances MC from worn armor by +1,
           regardless of ring's enchantment; wearing a second ring of
           protection (or even one ring of protection combined with
           cloak of protection) doesn't give a second MC boost */
RING("回復の指輪", "月長石の指輪",
     REGENERATION,             200, 1, 0,  6, MINERAL, HI_MINERAL),
RING("探索の指輪", "虎目石の指輪",
     SEARCHING,                200, 1, 0,  6, GEMSTONE, CLR_BROWN),
RING("忍びの指輪", "ひすいの指輪",
     STEALTH,                  100, 1, 0,  6, GEMSTONE, CLR_GREEN),
RING("能力維持の指輪", "青銅の指輪",
     FIXED_ABIL,               100, 1, 0,  4, COPPER, HI_COPPER),
RING("浮遊の指輪", "めのうの指輪",
     LEVITATION,               200, 1, 0,  7, GEMSTONE, CLR_RED),
RING("飢餓の指輪", "トパーズの指輪",
     HUNGER,                   100, 1, 0,  8, GEMSTONE, CLR_CYAN),
RING("反感の指輪", "サファイアの指輪",
     AGGRAVATE_MONSTER,        150, 1, 0,  9, GEMSTONE, CLR_BLUE),
RING("争いの指輪", "ルビーの指輪",
     CONFLICT,                 300, 1, 0,  9, GEMSTONE, CLR_RED),
RING("警告の指輪", "ダイヤモンドの指輪",
     WARNING,                  100, 1, 0, 10, GEMSTONE, CLR_WHITE),
RING("耐毒の指輪", "真珠の指輪",
     POISON_RES,               150, 1, 0,  4, BONE, CLR_WHITE),
RING("耐炎の指輪", "鉄の指輪",
     FIRE_RES,                 200, 1, 0,  5, IRON, HI_METAL),
RING("耐冷の指輪", "真鍮の指輪",
     COLD_RES,                 150, 1, 0,  4, COPPER, HI_COPPER),
RING("耐電の指輪", "銅の指輪",
     SHOCK_RES,                150, 1, 0,  3, COPPER, HI_COPPER),
RING("自由行動の指輪", "ねじれた指輪",
     FREE_ACTION,              200, 1, 0,  6, IRON, HI_METAL),
RING("消化不良の指輪", "鋼鉄の指輪",
     SLOW_DIGESTION,           200, 1, 0,  8, IRON, HI_METAL),
RING("瞬間移動の指輪", "銀の指輪",
     TELEPORT,                 200, 1, 0,  3, SILVER, HI_SILVER),
RING("瞬間移動制御の指輪", "金の指輪",
     TELEPORT_CONTROL,         300, 1, 0,  3, GOLD, HI_GOLD),
RING("変化の指輪", "象牙の指輪",
     POLYMORPH,                300, 1, 0,  4, BONE, CLR_WHITE),
RING("変化制御の指輪", "エメラルドの指輪",
     POLYMORPH_CONTROL,        300, 1, 0,  8, GEMSTONE, CLR_BRIGHT_GREEN),
RING("透明の指輪", "針金の指輪",
     INVIS,                    150, 1, 0,  5, IRON, HI_METAL),
RING("可視の指輪", "婚約指輪",
     SEE_INVIS,                150, 1, 0,  5, IRON, HI_METAL),
RING("耐変化怪物の指輪", "光る指輪",
     PROT_FROM_SHAPE_CHANGERS, 100, 1, 0,  5, IRON, CLR_BRIGHT_CYAN),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob) \
    OBJECT(OBJ(name, desc),                                            \
           BITS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, IRON),        \
           power, AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL)
AMULET("遠視の魔除け",     "円形の魔除け", TELEPAT, 175),
AMULET("命の魔除け",       "球形の魔除け", LIFESAVED, 75),
AMULET("絞殺の魔除け",     "卵型の魔除け", STRANGLED, 135),
AMULET("安眠の魔除け",   "三角形の魔除け", SLEEPY, 135),
AMULET("耐毒の魔除け",   "四角錐の魔除け", POISON_RES, 165),
AMULET("性転換の魔除け",   "四角の魔除け", 0, 130),
AMULET("無変化の魔除け",   "凹面の魔除け", UNCHANGING, 45),
AMULET("反射の魔除け",   "六角形の魔除け", REFLECTING, 75),
AMULET("呼吸の魔除け",   "八角形の魔除け", MAGICAL_BREATHING, 65),
/* fixed descriptions; description duplication is deliberate;
 * fake one must come before real one because selection for
 * description shuffling stops when a non-magic amulet is encountered
 */
OBJECT(OBJ("偽物のイェンダーの魔除け",
           "イェンダーの魔除け"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, PLASTIC),
       0, AMULET_CLASS, 0, 0, 20, 0, 0, 0, 0, 0, 1, HI_METAL),
OBJECT(OBJ("イェンダーの魔除け", /* note: description == name */
           "イェンダーの魔除け"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, MITHRIL),
       0, AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL),
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, mrg, chg, 0, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat), \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color)
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,mat,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, chg, 1, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat),   \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color)
#define WEPTOOL(name,desc,kn,mgc,bi,prob,wt,cost,sdam,ldam,hitbon,sub,mat,clr)\
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, bi, 0, hitbon, sub, mat),    \
           0, TOOL_CLASS, prob, 0, wt, cost, sdam, ldam, hitbon, 0, wt, clr)
/* containers */
CONTAINER("大箱",                    None, 1, 0, 0, 40, 350,   8, WOOD, HI_WOOD),
CONTAINER("宝箱",                    None, 1, 0, 0, 35, 600,  16, WOOD, HI_WOOD),
CONTAINER("アイスボックス",          None, 1, 0, 0,  5, 900,  42, PLASTIC, CLR_WHITE),
CONTAINER("袋",                      "鞄", 0, 0, 0, 35,  15,   2, CLOTH, HI_CLOTH),
CONTAINER("防水袋",                  "鞄", 0, 0, 0,  5,  15, 100, CLOTH, HI_CLOTH),
CONTAINER("軽量化の鞄",              "鞄", 0, 1, 0, 20,  15, 100, CLOTH, HI_CLOTH),
CONTAINER("トリックの鞄",            "鞄", 0, 1, 1, 20,  15, 100, CLOTH, HI_CLOTH),
#undef CONTAINER

/* lock opening tools */
TOOL("万能鍵",                       "鍵", 0, 0, 0, 0, 80,  3, 10, IRON, HI_METAL),
TOOL("鍵開け器具",                   None, 1, 0, 0, 0, 60,  4, 20, IRON, HI_METAL),
TOOL("クレジットカード",             None, 1, 0, 0, 0, 15,  1, 10, PLASTIC, CLR_WHITE),
/* light sources */
TOOL("獣脂のろうそく",         "ろうそく", 0, 1, 0, 0, 20,  2, 10, WAX, CLR_WHITE),
TOOL("蜜蝋のろうそく",         "ろうそく", 0, 1, 0, 0,  5,  2, 20, WAX, CLR_WHITE),
TOOL("真鍮のランタン",               None, 1, 0, 0, 0, 30, 30, 12, COPPER, CLR_YELLOW),
TOOL("オイルランプ",             "ランプ", 0, 0, 0, 0, 45, 20, 10, COPPER, CLR_YELLOW),
TOOL("魔法のランプ",             "ランプ", 0, 0, 1, 0, 15, 20, 50, COPPER, CLR_YELLOW),
/* other tools */
TOOL("高価なカメラ",                 None, 1, 0, 0, 1, 15, 12,200, PLASTIC, CLR_BLACK),
TOOL("鏡",                       "ガラス", 0, 0, 0, 0, 45, 13, 10, GLASS, HI_SILVER),
TOOL("水晶玉",               "ガラスの球", 0, 0, 1, 1, 15,150, 60, GLASS, HI_GLASS),
TOOL("レンズ",                       None, 1, 0, 0, 0,  5,  3, 80, GLASS, HI_GLASS),
TOOL("目隠し",                       None, 1, 0, 0, 0, 50,  2, 20, CLOTH, CLR_BLACK),
TOOL("タオル",                       None, 1, 0, 0, 0, 50,  2, 50, CLOTH, CLR_MAGENTA),
TOOL("鞍",                           None, 1, 0, 0, 0,  5,200,150, LEATHER, HI_LEATHER),
TOOL("紐",                           None, 1, 0, 0, 0, 65, 12, 20, LEATHER, HI_LEATHER),
TOOL("聴診器",                       None, 1, 0, 0, 0, 25,  4, 75, IRON, HI_METAL),
TOOL("缶詰作成道具",                 None, 1, 0, 0, 1, 15,100, 30, IRON, HI_METAL),
TOOL("缶切り",                       None, 1, 0, 0, 0, 35,  4, 30, IRON, HI_METAL),
TOOL("脂の缶",                       None, 1, 0, 0, 1, 15, 15, 20, IRON, HI_METAL),
TOOL("人形",                         None, 1, 0, 1, 0, 25, 50, 80, MINERAL, HI_MINERAL),
        /* monster type specified by obj->corpsenm */
TOOL("魔法のマーカ",                 None, 1, 0, 1, 1, 15,  2, 50, PLASTIC, CLR_RED),
/* traps */
TOOL("地雷",                         None, 1, 0, 0, 0, 0, 300,180, IRON, CLR_RED),
TOOL("熊の罠",                       None, 1, 0, 0, 0, 0, 200, 60, IRON, HI_METAL),
/* instruments;
   "If tin whistles are made out of tin, what do they make foghorns out of?" */
TOOL("ブリキの笛",                   "笛", 0, 0, 0, 0,100, 3, 10, METAL, HI_METAL),
TOOL("魔法の笛",                     "笛", 0, 0, 1, 0, 30, 3, 10, METAL, HI_METAL),
TOOL("木のフルート",           "フルート", 0, 0, 0, 0,  4, 5, 12, WOOD, HI_WOOD),
TOOL("魔法のフルート",         "フルート", 0, 0, 1, 1,  2, 5, 36, WOOD, HI_WOOD),
TOOL("細工のほどこされたホルン", "ホルン", 0, 0, 0, 0,  5, 18, 15, BONE, CLR_WHITE),
TOOL("吹雪のホルン",             "ホルン", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE),
TOOL("炎のホルン",               "ホルン", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE),
TOOL("恵みのホルン",             "ホルン", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE),
        /* horn, but not an instrument */
TOOL("木の竪琴",                   "竪琴", 0, 0, 0, 0,  4, 30, 50, WOOD, HI_WOOD),
TOOL("魔法の竪琴",                 "竪琴", 0, 0, 1, 1,  2, 30, 50, WOOD, HI_WOOD),
TOOL("ベル",                         None, 1, 0, 0, 0,  2, 30, 50, COPPER, HI_COPPER),
TOOL("ラッパ",                       None, 1, 0, 0, 0,  4, 10, 15, COPPER, HI_COPPER),
TOOL("革の太鼓",                   "太鼓", 0, 0, 0, 0,  4, 25, 25, LEATHER, HI_LEATHER),
TOOL("地震の太鼓",                 "太鼓", 0, 0, 1, 1,  2, 25, 25, LEATHER, HI_LEATHER),
/* tools useful as weapons */
WEPTOOL("つるはし", None,
        1, 0, 0, 20, 100,  50,  6,  3, WHACK,  P_PICK_AXE, IRON, HI_METAL),
WEPTOOL("ひっかけ棒", "鉄のフック",
        0, 0, 0,  5,  30,  50,  2,  6, WHACK,  P_FLAIL,    IRON, HI_METAL),
WEPTOOL("ユニコーンの角", None,
        1, 1, 1,  0,  20, 100, 12, 12, PIERCE, P_UNICORN_HORN,
                                                           BONE, CLR_WHITE),
        /* 3.4.1: unicorn horn left classified as "magic" */
/* two unique tools;
 * not artifacts, despite the comment which used to be here
 */
OBJECT(OBJ("祈りの燭台", "燭台"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, GOLD),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 200, HI_GOLD),
OBJECT(OBJ("開放のベル", "銀のベル"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, SILVER),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 50, HI_SILVER),
#undef TOOL
#undef WEPTOOL

/* Comestibles ... */
#define FOOD(name, prob, delay, wt, unk, tin, nutrition, color)         \
    OBJECT(OBJ(name, None),                                       \
           BITS(1, 1, unk, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, tin), 0,     \
           FOOD_CLASS, prob, delay, wt, nutrition / 20 + 5, 0, 0, 0, 0, \
           nutrition, color)
/* All types of food (except tins & corpses) must have a delay of at least 1.
 * Delay on corpses is computed and is weight dependant.
 * Domestic pets prefer tripe rations above all others.
 * Fortune cookies can be read, using them up without ingesting them.
 * Carrots improve your vision.
 * +0 tins contain monster meat.
 * +1 tins (of spinach) make you stronger (like Popeye).
 * Meatballs/sticks/rings are only created from objects via stone to flesh.
 */
/* meat */
FOOD("乾し肉",           140,  2, 10, 0, FLESH, 200, CLR_BROWN),
FOOD("死体",               0,  1,  0, 0, FLESH,   0, CLR_BROWN),
FOOD("卵",                85,  1,  1, 1, FLESH,  80, CLR_WHITE),
FOOD("ミートボール",       0,  1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("ミートスティック",   0,  1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("大きな肉のかたまり", 0, 20,400, 0, FLESH,2000, CLR_BROWN),
/* special case because it's not mergable */
OBJECT(OBJ("meat ring", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, CLR_BROWN),
/* pudding 'corpses' will turn into these and combine;
   must be in same order as the pudding monsters */
FOOD("灰色ウーズの塊",     0,  2, 20, 0, FLESH,  20, CLR_GRAY),
FOOD("茶色プリンの塊",     0,  2, 20, 0, FLESH,  20, CLR_BROWN),
FOOD("緑スライムの塊",     0,  2, 20, 0, FLESH,  20, CLR_GREEN),
FOOD("黒プリンの塊",       0,  2, 20, 0, FLESH,  20, CLR_BLACK),

/* fruits & veggies */
FOOD("ワカメ",             0,  1,  1, 0, VEGGY,  30, CLR_GREEN),
FOOD("ユーカリの葉",       3,  1,  1, 0, VEGGY,  30, CLR_GREEN),
FOOD("りんご",            15,  1,  2, 0, VEGGY,  50, CLR_RED),
FOOD("オレンジ",          10,  1,  2, 0, VEGGY,  80, CLR_ORANGE),
FOOD("洋ナシ",            10,  1,  2, 0, VEGGY,  50, CLR_BRIGHT_GREEN),
FOOD("メロン",            10,  1,  5, 0, VEGGY, 100, CLR_BRIGHT_GREEN),
FOOD("バナナ",            10,  1,  2, 0, VEGGY,  80, CLR_YELLOW),
FOOD("にんじん",          15,  1,  2, 0, VEGGY,  50, CLR_ORANGE),
FOOD("トリカブト",         7,  1,  1, 0, VEGGY,  40, CLR_GREEN),
FOOD("にんにく",           7,  1,  1, 0, VEGGY,  40, CLR_WHITE),
/* name of slime mold is changed based on player's OPTION=fruit:something
   and bones data might have differently named ones from prior games */
FOOD("ねばねばカビ",      75,  1,  5, 0, VEGGY, 250, HI_ORGANIC),

/* people food */
FOOD("ロイヤルゼリー",     0,  1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("クリームパイ",      25,  1, 10, 0, VEGGY, 100, CLR_WHITE),
FOOD("キャンディ",        13,  1,  2, 0, VEGGY, 100, CLR_BROWN),
FOOD("占いクッキー",      55,  1,  1, 0, VEGGY,  40, CLR_YELLOW),
FOOD("パンケーキ",        25,  2,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("レンバス",          20,  2,  5, 0, VEGGY, 800, CLR_WHITE),
FOOD("クラム",            20,  3, 15, 0, VEGGY, 600, HI_ORGANIC),
FOOD("食料",             380,  5, 20, 0, VEGGY, 800, HI_ORGANIC),
FOOD("Ｋレーション",       0,  1, 10, 0, VEGGY, 400, HI_ORGANIC),
FOOD("Ｃレーション",       0,  1, 10, 0, VEGGY, 300, HI_ORGANIC),
/* tins have type specified by obj->spe (+1 for spinach, other implies
   flesh; negative specifies preparation method {homemade,boiled,&c})
   and by obj->corpsenm (type of monster flesh) */
FOOD("缶",                  75,  0, 10, 1, METAL,   0, HI_METAL),
#undef FOOD

/* potions ... */
#define POTION(name,desc,mgc,power,prob,cost,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, GLASS),      \
           power, POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color)
POTION("能力獲得の薬",     "ルビー色の薬",  1, 0, 42, 300, CLR_RED),
POTION("能力回復の薬",     "ピンク色の薬",  1, 0, 40, 100, CLR_BRIGHT_MAGENTA),
POTION("混乱の薬",       "オレンジ色の薬",  1, CONFUSION, 42, 100, CLR_ORANGE),
POTION("盲目の薬",             "黄色の薬",  1, BLINDED, 40, 150, CLR_YELLOW),
POTION("麻痺の薬",     "エメラルド色の薬",  1, 0, 42, 300, CLR_BRIGHT_GREEN),
POTION("加速の薬",           "暗緑色の薬",  1, FAST, 42, 200, CLR_GREEN),
POTION("浮遊の薬",         "シアン色の薬",  1, LEVITATION, 42, 200, CLR_CYAN),
POTION("幻覚の薬",             "水色の薬",  1, HALLUC, 40, 100, CLR_CYAN),
POTION("透明の薬",       "明るい青色の薬",  1, INVIS, 40, 150, CLR_BRIGHT_BLUE),
POTION("可視の薬",       "マゼンダ色の薬",  1, SEE_INVIS, 42, 50, CLR_MAGENTA),
POTION("回復の薬",           "赤紫色の薬",  1, 0, 57, 100, CLR_MAGENTA),
POTION("超回復の薬",         "暗褐色の薬",  1, 0, 47, 100, CLR_RED),
POTION("レベルアップの薬", "ミルク色の薬",  1, 0, 20, 300, CLR_WHITE),
POTION("啓蒙の薬",     "渦を巻いている薬",  1, 0, 20, 200, CLR_BROWN),
POTION("怪物を探す薬",   "泡だっている薬",  1, 0, 40, 150, CLR_WHITE),
POTION("物体を探す薬",   "煙がでている薬",  1, 0, 42, 150, CLR_GRAY),
POTION("魔力の薬",         "曇っている薬",  1, 0, 42, 150, CLR_WHITE),
POTION("睡眠の薬",       "沸騰している薬",  1, 0, 42, 100, CLR_GRAY),
POTION("完全回復の薬",           "黒い薬",  1, 0, 10, 200, CLR_BLACK),
POTION("変化の薬",             "金色の薬",  1, 0, 10, 200, CLR_YELLOW),
POTION("酔っぱらいの薬",       "茶色の薬",  0, 0, 42,  50, CLR_BROWN),
POTION("病気の薬",       "発泡している薬",  0, 0, 42,  50, CLR_CYAN),
POTION("フルーツジュース", "陰気な色の薬",  0, 0, 42,  50, CLR_BLACK),
POTION("酸の薬",                 "白い薬",  0, 0, 10, 250, CLR_WHITE),
POTION("油",                   "濃黒の薬",  0, 0, 30, 250, CLR_BROWN),
/* fixed description
 */
POTION("水",                   "無色の薬",  0, 0, 92, 100, CLR_CYAN),
#undef POTION

/* scrolls ... */
#define SCROLL(name,text,mgc,prob,cost) \
    OBJECT(OBJ(name, text),                                           \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),    \
           0, SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, HI_PAPER)
SCROLL("鎧に魔法をかける巻物",     "『へろへろっす』と書かれた巻物",  1,  63,  80),
SCROLL("鎧を破壊する巻物",       "『ぱーるあぱっち』と書かれた巻物",  1,  45, 100),
SCROLL("怪物を混乱させる巻物", "『せぶんすぎょざー』と書かれた巻物",  1,  53, 100),
SCROLL("怪物を怯えさせる巻物",     "『びーえすでー』と書かれた巻物",  1,  35, 100),
SCROLL("解呪の巻物",                 "『おろかぶー』と書かれた巻物",  1,  65,  80),
SCROLL("武器に魔法をかける巻物", "『あらほらさっさ』と書かれた巻物",  1,  80,  60),
SCROLL("怪物を作る巻物",             "『りなっくす』と書かれた巻物",  1,  45, 200),
SCROLL("怪物を飼いならす巻物",       "『げろげーろ』と書かれた巻物",  1,  15, 200),
SCROLL("虐殺の巻物",         "『ばくしょうもんだい』と書かれた巻物",  1,  15, 300),
SCROLL("光の巻物",               "『ちぇっくめいと』と書かれた巻物",  1,  90,  50),
SCROLL("瞬間移動の巻物",       "『おーくすだーびー』と書かれた巻物",  1,  55, 100),
SCROLL("金貨を探す巻物",           "『かねかねかね』と書かれた巻物",  1,  33, 100),
SCROLL("食料を探す巻物",         "『おぶちけいぞう』と書かれた巻物",  1,  25, 100),
SCROLL("識別の巻物",                   "『くっくー』と書かれた巻物",  1, 180,  20),
SCROLL("地図の巻物",                 "『じゃいすと』と書かれた巻物",  1,  45, 100),
SCROLL("記憶喪失の巻物",             "『まえをみろ』と書かれた巻物",  1,  35, 200),
SCROLL("炎の巻物",             "『どっくりびんきー』と書かれた巻物",  1,  30, 100),
SCROLL("大地の巻物",         "『よしのやぎゅどーん』と書かれた巻物",  1,  18, 200),
SCROLL("罰の巻物",                 "『すてーきみゃ』と書かれた巻物",  1,  15, 300),
SCROLL("充填の巻物",               "『じゃいしょく』と書かれた巻物",  1,  15, 300),
SCROLL("悪臭雲の巻物",           "『ちゃんぴお～ん』と書かれた巻物",  1,  15, 300),
    /* Extra descriptions, shuffled into use at start of new game.
     * Code in win/share/tilemap.c depends on SCR_STINKING_CLOUD preceding
     * these and on how many of them there are.  If a real scroll gets added
     * after stinking cloud or the number of extra descriptions changes,
     * tilemap.c must be modified to match.
     */
SCROLL(None,       "『はにゃ～ん』と書かれた巻物",  1,   0, 100),
SCROLL(None,       "『ぽちっとな』と書かれた巻物",  1,   0, 100),
SCROLL(None, "『しびびんしびびん』と書かれた巻物",  1,   0, 100),
SCROLL(None,     "『うしろをみろ』と書かれた巻物",  1,   0, 100),
SCROLL(None,         "『ETAOIN SHRDLU』と書かれた巻物",  1,   0, 100),
SCROLL(None,           "『LOREM IPSUM』と書かれた巻物",  1,   0, 100),
SCROLL(None,                 "『FNORD』と書かれた巻物",  1,   0, 100), /* Illuminati */
SCROLL(None,               "『KO BATE』と書かれた巻物",  1,   0, 100), /* Kurd Lasswitz */
SCROLL(None,         "『ABRA KA DABRA』と書かれた巻物",  1,   0, 100), /* traditional incantation */
SCROLL(None,          "『ASHPD SODALG』と書かれた巻物",  1,   0, 100), /* Portal */
SCROLL(None,               "『ZLORFIK』と書かれた巻物",  1,   0, 100), /* Zak McKracken */
SCROLL(None,         "『GNIK SISI VLE』と書かれた巻物",  1,   0, 100), /* Zak McKracken */
SCROLL(None,       "『HAPAX LEGOMENON』と書かれた巻物",  1,   0, 100),
SCROLL(None,     "『EIRIS SAZUN IDISI』と書かれた巻物",  1,   0, 100), /* Merseburg Incantations */
SCROLL(None,       "『PHOL ENDE WODAN』と書かれた巻物",  1,   0, 100), /* Merseburg Incantations */
SCROLL(None,                 "『GHOTI』と書かれた巻物",  1,   0, 100), /* pronounced as 'fish',
                                                        George Bernard Shaw */
SCROLL(None, "『MAPIRO MAHAMA DIROMAT』と書かれた巻物", 1, 0, 100), /* Wizardry */
SCROLL(None,     "『VAS CORP BET MANI』と書かれた巻物",  1,   0, 100), /* Ultima */
SCROLL(None,               "『XOR OTA』と書かれた巻物",  1,   0, 100), /* Aarne Haapakoski */
SCROLL(None,    "『STRC PRST SKRZ KRK』と書かれた巻物",  1,   0, 100), /* Czech and Slovak
                                                        tongue-twister */
    /* These must come last because they have special fixed descriptions.
     */
#ifdef MAIL
SCROLL("手紙の巻物",         "消印の押された巻物",  0,   0,   0),
#endif
SCROLL("白紙の巻物",           "ラベルのない巻物",  0,  28,  60),
#undef SCROLL

/* spellbooks ... */
/* expanding beyond 52 spells would require changes in spellcasting
   or imposition of a limit on number of spells hero can know because
   they are currently assigned successive letters, a-zA-Z, when learned */
#define SPELL(name,desc,sub,prob,delay,level,mgc,dir,color)  \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 0, 0, 0, mgc, 0, 0, 0, 0, 0, dir, sub, PAPER),       \
           0, SPBOOK_CLASS, prob, delay, 50, level * 100,               \
           0, 0, 0, level, 20, color)
SPELL("穴掘りの魔法書",   "羊皮紙の魔法書",
      P_MATTER_SPELL,      20,  6, 5, 1, RAY, HI_PAPER),
SPELL("矢の魔法書",       "子牛皮の魔法書",
      P_ATTACK_SPELL,      45,  2, 2, 1, RAY, HI_PAPER),
SPELL("火の玉の魔法書",   "ぼろぼろの魔法書",
      P_ATTACK_SPELL,      20,  4, 4, 1, RAY, HI_PAPER),
SPELL("冷気の魔法書",     "ページの折られた魔法書",
      P_ATTACK_SPELL,      10,  7, 4, 1, RAY, HI_PAPER),
SPELL("眠りの魔法書",     "まだらの魔法書",
      P_ENCHANTMENT_SPELL, 50,  1, 1, 1, RAY, HI_PAPER),
SPELL("死の指の魔法書",   "よごれた魔法書",
      P_ATTACK_SPELL,       5, 10, 7, 1, RAY, HI_PAPER),
SPELL("灯りの魔法書",     "布地の魔法書",
      P_DIVINATION_SPELL,  45,  1, 1, 1, NODIR, HI_CLOTH),
SPELL("怪物を探す魔法書", "leathery",
      P_DIVINATION_SPELL,  43,  1, 1, 1, NODIR, HI_LEATHER),
SPELL("回復の魔法書",     "白い魔法書",
      P_HEALING_SPELL,     40,  2, 1, 1, IMMEDIATE, CLR_WHITE),
SPELL("開錠の魔法書",     "ピンク色の魔法書",
      P_MATTER_SPELL,      35,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA),
SPELL("衝撃の魔法書",     "赤い魔法書",
      P_ATTACK_SPELL,      35,  2, 1, 1, IMMEDIATE, CLR_RED),
SPELL("混乱の魔法書",     "オレンジ色の魔法書",
      P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_ORANGE),
SPELL("盲目を癒す魔法書", "黄色い魔法書",
      P_HEALING_SPELL,     25,  2, 2, 1, IMMEDIATE, CLR_YELLOW),
SPELL("脱力の魔法書",     "ビロードの魔法書",
      P_ATTACK_SPELL,      10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("牛歩の魔法書",     "淡緑色の魔法書",
      P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN),
SPELL("施錠の魔法書",     "濃緑色の魔法書",
      P_MATTER_SPELL,      30,  3, 2, 1, IMMEDIATE, CLR_GREEN),
SPELL("怪物を造る魔法書", "青緑色の魔法書",
      P_CLERIC_SPELL,      35,  3, 2, 1, NODIR, CLR_BRIGHT_CYAN),
SPELL("食料を探す魔法書", "シアン色の魔法書",
      P_DIVINATION_SPELL,  30,  3, 2, 1, NODIR, CLR_CYAN),
SPELL("恐怖の魔法書",     "淡青の魔法書",
      P_ENCHANTMENT_SPELL, 25,  3, 3, 1, NODIR, CLR_BRIGHT_BLUE),
SPELL("千里眼の魔法書",   "濃青の魔法書",
      P_DIVINATION_SPELL,  15,  3, 3, 1, NODIR, CLR_BLUE),
SPELL("病気を癒す魔法書", "藍色の魔法書",
      P_HEALING_SPELL,     32,  3, 3, 1, NODIR, CLR_BLUE),
SPELL("魅了の魔法書",     "マゼンダ色の魔法書",
      P_ENCHANTMENT_SPELL, 20,  3, 3, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("速攻の魔法書",     "紫色の魔法書",
      P_ESCAPE_SPELL,      33,  4, 3, 1, NODIR, CLR_MAGENTA),
SPELL("霊感の魔法書",     "スミレ色の魔法書",
      P_DIVINATION_SPELL,  20,  4, 3, 1, NODIR, CLR_MAGENTA),
SPELL("浮遊の魔法書",     "黄褐色の魔法書",
      P_ESCAPE_SPELL,      20,  4, 4, 1, NODIR, CLR_BROWN),
SPELL("超回復の魔法書",   "ラシャの魔法書",
      P_HEALING_SPELL,     27,  5, 3, 1, IMMEDIATE, CLR_GREEN),
SPELL("能力回復の魔法書", "淡茶色の魔法書",
      P_HEALING_SPELL,     25,  5, 4, 1, NODIR, CLR_BROWN),
SPELL("透明の魔法書",     "濃茶色の魔法書",
      P_ESCAPE_SPELL,      25,  5, 4, 1, NODIR, CLR_BROWN),
SPELL("宝を探す魔法書",   "灰色の魔法書",
      P_DIVINATION_SPELL,  20,  5, 4, 1, NODIR, CLR_GRAY),
SPELL("解呪の魔法書",     "くしゃくしゃの魔法書",
      P_CLERIC_SPELL,      25,  5, 3, 1, NODIR, HI_PAPER),
SPELL("地図の魔法書",     "ほこりっぽい魔法書",
      P_DIVINATION_SPELL,  18,  7, 5, 1, NODIR, HI_PAPER),
SPELL("識別の魔法書",     "青銅の魔法書",
      P_DIVINATION_SPELL,  20,  6, 3, 1, NODIR, HI_COPPER),
SPELL("蘇生の魔法書",     "銅の魔法書",
      P_CLERIC_SPELL,      16,  8, 6, 1, IMMEDIATE, HI_COPPER),
SPELL("変化の魔法書",     "銀の魔法書",
      P_MATTER_SPELL,      10,  8, 6, 1, IMMEDIATE, HI_SILVER),
SPELL("瞬間移動の魔法書", "金の魔法書",
      P_ESCAPE_SPELL,      15,  6, 6, 1, IMMEDIATE, HI_GOLD),
SPELL("造魔の魔法書",     "きらびやかな魔法書",
      P_CLERIC_SPELL,      10,  7, 6, 1, NODIR, CLR_WHITE),
SPELL("無力化の魔法書",   "輝く魔法書",
      P_MATTER_SPELL,      15,  8, 7, 1, IMMEDIATE, CLR_WHITE),
SPELL("守りの魔法書",     "鉛色の魔法書",
      P_CLERIC_SPELL,      18,  3, 1, 1, NODIR, HI_PAPER),
SPELL("跳躍の魔法書",     "薄い色の魔法書",
      P_ESCAPE_SPELL,      20,  3, 1, 1, IMMEDIATE, HI_PAPER),
SPELL("軟化の魔法書",     "濃い色の魔法書",
      P_HEALING_SPELL,     15,  1, 3, 1, IMMEDIATE, HI_PAPER),
#if 0 /* DEFERRED */
/* from slash'em, create a tame critter which explodes when attacking,
   damaging adjacent creatures--friend or foe--and dying in the process */
SPELL("flame sphere",     "麻の魔法書",
      P_MATTER_SPELL,      20,  2, 1, 1, NODIR, CLR_BROWN),
SPELL("freeze sphere",    "固い表紙の魔法書",
      P_MATTER_SPELL,      20,  2, 1, 1, NODIR, CLR_BROWN),
#endif
/* books with fixed descriptions
 */
SPELL("白紙の魔法書", "真っ白な魔法書", P_NONE, 18, 0, 0, 0, 0, HI_PAPER),
/* tribute book for 3.6 */
OBJECT(OBJ("小説", "ペーパーバックの魔法書"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 0, 20, 0, 0, 0, 1, 20, CLR_BRIGHT_BLUE),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("死者の書", "パピルスの魔法書"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 20, 10000, 0, 0, 0, 7, 20, HI_PAPER),
#undef SPELL

/* wands ... */
#define WAND(name,typ,prob,cost,mgc,dir,metal,color) \
    OBJECT(OBJ(name, typ),                                              \
           BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, P_NONE, metal),    \
           0, WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color)
WAND("灯りの杖",             "ガラスの杖", 95, 100, 1, NODIR, GLASS, HI_GLASS),
WAND("扉探索の杖",
                        "バルサの杖", 50, 150, 1, NODIR, WOOD, HI_WOOD),
WAND("啓蒙の杖",               "水晶の杖", 15, 150, 1, NODIR, GLASS, HI_GLASS),
WAND("怪物を造る杖",             "楓の杖", 45, 200, 1, NODIR, WOOD, HI_WOOD),
WAND("願いの杖",                 "松の杖",  5, 500, 1, NODIR, WOOD, HI_WOOD),
WAND("単なる杖",                 "樫の杖", 25, 100, 0, IMMEDIATE, WOOD, HI_WOOD),
WAND("衝撃の杖",               "黒檀の杖", 75, 150, 1, IMMEDIATE, WOOD, HI_WOOD),
WAND("透明化の杖",           "大理石の杖", 45, 150, 1, IMMEDIATE, MINERAL, HI_MINERAL),
WAND("減速の杖",             "ブリキの杖", 50, 150, 1, IMMEDIATE, METAL, HI_METAL),
WAND("加速の杖",               "真鍮の杖", 50, 150, 1, IMMEDIATE, COPPER, HI_COPPER),
WAND("蘇生の杖",                 "銅の杖", 50, 150, 1, IMMEDIATE, COPPER, HI_COPPER),
WAND("変化の杖",                 "銀の杖", 45, 200, 1, IMMEDIATE, SILVER, HI_SILVER),
WAND("無力化の杖",         "プラチナの杖", 45, 200, 1, IMMEDIATE, PLATINUM, CLR_WHITE),
WAND("瞬間移動の杖",     "イリジウムの杖", 45, 200, 1, IMMEDIATE, METAL,
                                                             CLR_BRIGHT_CYAN),
WAND("開錠の杖",               "亜鉛の杖", 25, 150, 1, IMMEDIATE, METAL, HI_METAL),
WAND("施錠の杖",       "アルミニウムの杖", 25, 150, 1, IMMEDIATE, METAL, HI_METAL),
WAND("探査する杖",       "ウラニウムの杖", 30, 150, 1, IMMEDIATE, METAL, HI_METAL),
WAND("穴掘りの杖",               "鉄の杖", 55, 150, 1, RAY, IRON, HI_METAL),
WAND("矢の杖",                 "鋼鉄の杖", 50, 150, 1, RAY, IRON, HI_METAL),
WAND("炎の杖",               "六角形の杖", 40, 175, 1, RAY, IRON, HI_METAL),
WAND("吹雪の杖",                 "短い杖", 40, 175, 1, RAY, IRON, HI_METAL),
WAND("眠りの杖", "ルーン文字の書かれた杖", 50, 175, 1, RAY, IRON, HI_METAL),
WAND("死の杖",                   "長い杖",  5, 500, 1, RAY, IRON, HI_METAL),
WAND("雷の杖",                 "曲った杖", 40, 175, 1, RAY, IRON, HI_METAL),
/* extra descriptions, shuffled into use at start of new game */
WAND(None,                     "二叉の杖",  0, 150, 1, 0, WOOD, HI_WOOD),
WAND(None,     "くさびの打ちつけられた杖",  0, 150, 1, 0, IRON, HI_METAL),
WAND(None,           "宝石が埋められた杖",  0, 150, 1, 0, IRON, HI_MINERAL),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(name,prob,metal,worth) \
    OBJECT(OBJ(name, None),                                        \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, metal),    \
           0, COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, HI_GOLD)
COIN("金貨", 1000, GOLD, 1),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(name,desc,prob,wt,gval,nutr,mohs,glass,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0,                              \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, 0, nutr, color)
#define ROCK(name,desc,kn,prob,wt,gval,sdam,ldam,mgc,nutr,mohs,glass,color) \
    OBJECT(OBJ(name, desc),                                             \
           BITS(kn, 1, 0, 0, mgc, 0, 0, 0, 0,                           \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr, color)
GEM("ディリジウムの結晶", "白い石",  2, 1, 4500, 15,  5, GEMSTONE, CLR_WHITE),
GEM("ダイヤモンド",       "白い石",  3, 1, 4000, 15, 10, GEMSTONE, CLR_WHITE),
GEM("ルビー",             "赤い石",  4, 1, 3500, 15,  9, GEMSTONE, CLR_RED),
GEM("橙水晶",           "橙色の石",  3, 1, 3250, 15,  9, GEMSTONE, CLR_ORANGE),
GEM("サファイア",         "青い石",  4, 1, 3000, 15,  9, GEMSTONE, CLR_BLUE),
GEM("黒オパール",         "黒い石",  3, 1, 2500, 15,  8, GEMSTONE, CLR_BLACK),
GEM("エメラルド",         "緑の石",  5, 1, 2500, 15,  8, GEMSTONE, CLR_GREEN),
GEM("トルコ石",           "緑の石",  6, 1, 2000, 15,  6, GEMSTONE, CLR_GREEN),
GEM("黄水晶",           "黄色い石",  4, 1, 1500, 15,  6, GEMSTONE, CLR_YELLOW),
GEM("アクアマリン",       "緑の石",  6, 1, 1500, 15,  8, GEMSTONE, CLR_GREEN),
GEM("琥珀",           "茶褐色の石",  8, 1, 1000, 15,  2, GEMSTONE, CLR_BROWN),
GEM("トパーズ",       "茶褐色の石", 10, 1,  900, 15,  8, GEMSTONE, CLR_BROWN),
GEM("黒玉",               "黒い石",  6, 1,  850, 15,  7, GEMSTONE, CLR_BLACK),
GEM("オパール",           "白い石", 12, 1,  800, 15,  6, GEMSTONE, CLR_WHITE),
GEM("金緑石",           "黄色い石",  8, 1,  700, 15,  5, GEMSTONE, CLR_YELLOW),
GEM("ガーネット",         "赤い石", 12, 1,  700, 15,  7, GEMSTONE, CLR_RED),
GEM("アメジスト",         "紫の石", 14, 1,  600, 15,  7, GEMSTONE, CLR_MAGENTA),
GEM("ジャスパー",         "赤い石", 15, 1,  500, 15,  7, GEMSTONE, CLR_RED),
GEM("フルオライト",       "紫の石", 15, 1,  400, 15,  4, GEMSTONE, CLR_MAGENTA),
GEM("黒燿石",             "黒い石",  9, 1,  200, 15,  6, GEMSTONE, CLR_BLACK),
GEM("めのう",           "橙色の石", 12, 1,  200, 15,  6, GEMSTONE, CLR_ORANGE),
GEM("ひすい",             "緑の石", 10, 1,  300, 15,  6, GEMSTONE, CLR_GREEN),
GEM("白いガラス", "白い石",
    77, 1, 0, 6, 5, GLASS, CLR_WHITE),
GEM("青いガラス", "青い石",
    77, 1, 0, 6, 5, GLASS, CLR_BLUE),
GEM("赤いガラス", "赤い石",
    77, 1, 0, 6, 5, GLASS, CLR_RED),
GEM("茶褐色のガラス", "茶褐色の石",
    77, 1, 0, 6, 5, GLASS, CLR_BROWN),
GEM("橙色のガラス", "橙色の石",
    76, 1, 0, 6, 5, GLASS, CLR_ORANGE),
GEM("黄色のガラス", "黄色い石",
    77, 1, 0, 6, 5, GLASS, CLR_YELLOW),
GEM("黒色のガラス", "黒い石",
    76, 1, 0, 6, 5, GLASS, CLR_BLACK),
GEM("緑のガラス", "緑の石",
    77, 1, 0, 6, 5, GLASS, CLR_GREEN),
GEM("紫のガラス", "紫の石",
    77, 1, 0, 6, 5, GLASS, CLR_MAGENTA),

/* Placement note: there is a wishable subrange for
 * "gray stones" in the o_ranges[] array in objnam.c
 * that is currently everything between luckstones and flint
 * (inclusive).
 */
ROCK("幸せの石", "灰色の宝石", 0,  10,  10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY),
ROCK("重し", "灰色の宝石",     0,  10, 500,  1, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("試金石", "灰色の宝石",   0,   8,  10, 45, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("火打ち石", "灰色の宝石", 0,  10,  10,  1, 6, 6, 0, 10, 7, MINERAL, CLR_GRAY),
ROCK("石", None,               1, 100,  10,  0, 3, 3, 0, 10, 7, MINERAL, CLR_GRAY),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */
OBJECT(OBJ("岩", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 100, 0, 6000, 0, 20, 20, 0, 0, 2000, HI_MINERAL),
OBJECT(OBJ("像", None),
       BITS(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 900, 0, 2500, 0, 20, 20, 0, 0, 2500, CLR_WHITE),

OBJECT(OBJ("重い鉄球", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       BALL_CLASS, 1000, 0, 480, 10, 25, 25, 0, 0, 200, HI_METAL),
        /* +d4 when "very heavy" */
OBJECT(OBJ("鉄の鎖", None),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       CHAIN_CLASS, 1000, 0, 120, 0, 4, 4, 0, 0, 200, HI_METAL),
        /* +1 both l & s */

/* Venom is normally a transitory missile (spit by various creatures)
 * but can be wished for in wizard mode so could occur in bones data.
 */
OBJECT(OBJ("盲目の毒液", "毒液のしぶき"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_ORGANIC),
OBJECT(OBJ("酸の毒液", "毒液のしぶき"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_ORGANIC),
        /* +d6 small or large */

/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
OBJECT(OBJ(None, None),
       BITS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0), 0,
       ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
}; /* objects[] */

#ifndef OBJECTS_PASS_2_

/* perform recursive compilation for second structure */
#undef OBJ
#undef OBJECT
#define OBJECTS_PASS_2_
#include "objects.c"

/* clang-format on */
/* *INDENT-ON* */

void NDECL(objects_init);

/* dummy routine used to force linkage */
void
objects_init()
{
    return;
}

#endif /* !OBJECTS_PASS_2_ */

/*objects.c*/
