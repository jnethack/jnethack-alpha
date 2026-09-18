/* JNetHack Copyright */
/* (c) Issei Numata 1994-2000                                      */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2022            */
/* JNetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define JCONJ_BUFSZ     1024
#define JCONJ_MAX_VERB  800
#define JCONJ_MAX_SFX   100

#define STRNCMP2(x, y) strncmp((x), (y), strlen(y))

#define J_A     0
#define J_KA    (1*5)
#define J_SA    (2*5)
#define J_TA    (3*5)
#define J_NA    (4*5)
#define J_HA    (5*5)
#define J_MA    (6*5)
#define J_YA    (7*5)
#define J_RA    (8*5)
#define J_WA    (9*5)

#define J_GA    (10*5)
#define J_ZA    (11*5)
#define J_DA    (12*5)
#define J_BA    (13*5)
#define J_PA    (14*5)

static int cl = /* 全角ひらがなのバイト数 */
#ifdef ICUTF8
    3;
#else
    2;
#endif

static const char *hira_tab[] = {
    "あ", "い", "う", "え", "お", 
    "か", "き", "く", "け", "こ",
    "さ", "し", "す", "せ", "そ",
    "た", "ち", "つ", "て", "と",
    "な", "に", "ぬ", "ね", "の",
    "は", "ひ", "ふ", "へ", "ほ",
    "ま", "み", "む", "め", "も",
    "や", "い", "ゆ", "え", "よ",
    "ら", "り", "る", "れ", "ろ",
    "わ", "い", "う", "え", "お",
    "が", "ぎ", "ぐ", "げ", "ご",
    "ざ", "じ", "ず", "ぜ", "ぞ",
    "だ", "ぢ", "づ", "で", "ど",
    "ば", "び", "ぶ", "べ", "ぼ",
    "ぱ", "ぴ", "ぷ", "ぺ", "ぽ",
};

#define FIFTH   0 /* 五段 */
#define UPPER   1 /* 上一段 */
#define LOWER   2 /* 下一段 */
#define SAHEN   3 /* サ変 */
#define KAHEN   4 /* カ変 */

#define NORMAL  0 /* あける→～た */
#define HATSUON 1 /* 刻む→～んだ */
#define SOKUON  2 /* 打つ→～った */
#define ION     3 /* 浮く→～いた */

struct _jconj_tab {
    const char *main;
    int column;
/* 0: fifth conj. 1:upper conj. 2:lower conj. 3:SAHEN 4:KAHEN */
    int katsuyo_type;
/* 0: normal 1: hatsuon 2: sokuon 3: ion */
    int onbin_type;
} static jconj_tab[] = {
/* あ */
    {"あける", J_KA, LOWER, NORMAL},
    {"開ける", J_KA, LOWER, NORMAL},
    {"いれる", J_RA, LOWER, NORMAL},
    {"入れる", J_RA, LOWER, NORMAL},
    {"浮く", J_KA, FIFTH, ION},
    {"動く", J_KA, FIFTH, ION},
    {"打つ", J_TA, FIFTH, SOKUON},
    {"置く", J_KA, FIFTH, ION},
    {"納める", J_MA, LOWER, NORMAL},
    {"落ちる", J_TA, UPPER, NORMAL},
    {"落す", J_SA, FIFTH, NORMAL},
    {"おののく", J_KA, FIFTH, ION},
/* か */
    {"輝く", J_KA, FIFTH, ION},
    {"書く", J_KA, FIFTH, ION},
    {"かける", J_KA, LOWER, NORMAL},
    {"かぶる", J_RA, FIFTH, SOKUON},
    {"構える", J_A, LOWER, NORMAL},
    {"噛みつく", J_KA, FIFTH, ION},
    {"刻む", J_MA, FIFTH, HATSUON},
    {"着る", J_KA, UPPER, NORMAL},
    {"来る", J_KA, KAHEN, NORMAL}, 
    {"砕く", J_KA, FIFTH, ION}, 
    {"加える", J_A, LOWER, NORMAL},
    {"こする", J_RA, FIFTH, SOKUON},
    {"こます", J_SA, FIFTH, NORMAL},
    {"込む", J_MA, FIFTH, HATSUON},
    {"殺す", J_SA, FIFTH, NORMAL},
    {"壊す", J_SA, FIFTH, NORMAL},
/* さ */
    {"捧げる", J_KA, LOWER, NORMAL},
    {"錆びる", J_BA, UPPER, NORMAL},
    {"死ぬ", J_NA, FIFTH, HATSUON},
    {"滑る", J_RA, FIFTH, SOKUON},
    {"する", J_SA, SAHEN, NORMAL}, 
/* た */
    {"たじろぐ", J_GA, FIFTH, ION},
    {"出す", J_SA, FIFTH, NORMAL},
    {"食べる", J_HA, LOWER, NORMAL}, 
    {"使う", J_WA, FIFTH, SOKUON},
    {"つける", J_KA, LOWER, NORMAL},
    {"付ける", J_KA, LOWER, NORMAL},
    {"つまずく", J_KA, FIFTH, ION},
    {"出る", J_NA, LOWER, NORMAL},
    {"解く", J_KA, FIFTH, ION},
    {"溶ける", J_KA, LOWER, NORMAL},
    {"とばす", J_SA, FIFTH, NORMAL},
    {"飛ぶ", J_BA, FIFTH, HATSUON},
    {"取る", J_RA, FIFTH, SOKUON},
/* な */
    {"投げる", J_GA, LOWER, NORMAL},
    {"名づける", J_KA, LOWER, NORMAL},
    {"握る", J_RA, FIFTH, SOKUON},
    {"にじみ出る", J_DA, LOWER, NORMAL},
    {"脱ぐ", J_GA, FIFTH, ION},
    {"濡らす", J_SA, FIFTH, NORMAL},
    {"塗る", J_RA, FIFTH, SOKUON},
    {"飲む", J_MA, FIFTH, HATSUON},
/* は */
    {"はいずる", J_RA, FIFTH, SOKUON},
    {"履く", J_KA, FIFTH, ION},
    {"はさむ", J_MA, FIFTH, HATSUON},
    {"はずす", J_SA, FIFTH, NORMAL},
    {"外す", J_SA, FIFTH, NORMAL},
    {"はめる", J_MA, LOWER, NORMAL},
    {"光る", J_RA, FIFTH, SOKUON},
    {"浸す", J_SA, FIFTH, NORMAL},
    {"ひっかける", J_KA, LOWER, NORMAL},
    {"ひっくり返す", J_SA, FIFTH, NORMAL},
    {"拾う", J_WA, FIFTH, SOKUON},
    {"踏む", J_MA, FIFTH, HATSUON},
    {"振りかざす", J_SA, FIFTH, NORMAL},
    {"震える", J_A, LOWER, NORMAL},
    {"掘る", J_RA, FIFTH, SOKUON},
/* ま */
    {"巻く", J_KA, FIFTH, ION},
    {"またたく", J_KA, FIFTH, ION},
    {"守る", J_RA, FIFTH, SOKUON},
    {"回す", J_SA, FIFTH, NORMAL},
    {"身につける", J_KA, LOWER, NORMAL},
    {"持つ", J_TA, FIFTH, SOKUON},
/* や */
    {"焼く", J_KA, FIFTH, ION},
    {"呼ぶ", J_BA, FIFTH, HATSUON},
    {"読む", J_MA, FIFTH, HATSUON},
    {"よろめく", J_KA, FIFTH, ION},
/* ら */
/* わ */
    {(void*)0, 0, 0, 0},
};

/*
**      conjection verb word
**
**      Example
**      arg1    arg2    result
**      脱ぐ    ない    脱がない
**      脱ぐ    た      脱いだ
**
*/
static const char *
jconjsub(struct _jconj_tab *tab, const char *jverb, const char *sfx)
{
    int len;
    unsigned char *p;
    static unsigned char tmp[JCONJ_BUFSZ];

    /* tabはjconj_tabの内容のみを受け取り、
     * jconj_tabの内容はこれ以上変更しないため、
     * tabの内容によってバッファオーバーフローすることはない
     */

    len = strlen(jverb);
    if (len > JCONJ_MAX_VERB || strlen(sfx) > JCONJ_MAX_SFX) /* 安全用 */
        return jverb;
    strcpy((char *)tmp, jverb );

    if(!STRNCMP2(sfx, "と")){
        strcat((char *)tmp, sfx);
        return (char *)tmp;
    }

    switch( tab->katsuyo_type ){
      case FIFTH:
        p = tmp + (len - cl);
        if(!STRNCMP2(sfx, "な")){
            memcpy(p, hira_tab[tab->column], cl);

            strcpy((char *)p + cl, sfx);
            break;
        }
        else if(!STRNCMP2(sfx, "た") || !STRNCMP2(sfx, "て")){
            switch( tab->onbin_type ){
              case NORMAL:
                memcpy(p, hira_tab[tab->column + 1], cl);
                break;
              case HATSUON:
                memcpy(p, "ん", cl);
                break;
              case SOKUON:
                memcpy(p, "っ", cl);
                break;
              case ION:
                memcpy(p, "い", cl);
                break;
            }
            strcpy((char *)p + cl, sfx);
            if(tab->onbin_type == HATSUON ||
               (tab->onbin_type == ION && tab->column >= J_GA)){
                  /*
                   * 2文字目を濁音にする。
                   * 清音の次が濁音であることに依存している。
                   * EUC-JP, SJIS, UTF-8 とも満たしている。
                   */
                  ++p[cl * 2 - 1];
            }
            break;
        }
        else if(!STRNCMP2(sfx, "ば")){
            memcpy(p, hira_tab[tab->column + 3], cl);
            strcpy((char *)p + cl, sfx);
        }
        else if(!STRNCMP2(sfx, "れ")){
            memcpy(p, hira_tab[tab->column + 3], cl);
            strcpy((char *)p + cl, sfx + cl);
        }
        else if(!STRNCMP2(sfx, "ま")) {
            memcpy(p, hira_tab[tab->column + 1], cl);
            strcpy((char *)p + cl, sfx);
            break;
        }
        else if(!STRNCMP2(sfx, "よ")) {
            memcpy(p, hira_tab[tab->column + 4], cl);
            strcpy((char *)p + cl, sfx + cl);
            break;
        }
        break;
      case LOWER:
      case UPPER:
      case KAHEN:
        p = tmp + (len - cl);
        if(!STRNCMP2(sfx, "ば")){
            strcpy((char *)p, "れ");
            strcpy((char *)p + cl, sfx);
        }
        else if(!STRNCMP2(sfx, "れば")){
            strcpy((char *)p, "れ");
            strcpy((char *)p + cl, sfx + cl);
        }
        else if(!STRNCMP2(sfx, "れ")){
          strcpy((char *)p, "られ");
             strcpy((char *)p + cl * 2, sfx + cl);
         }
          else
          strcpy((char *)p, sfx);
        break;
      case SAHEN:
        p = tmp + (len - cl * 2);
        if(!STRNCMP2(sfx, "な") ||
           !STRNCMP2(sfx, "ま") ||
           !STRNCMP2(sfx, "た") ||
           !STRNCMP2(sfx, "て") ||
           !STRNCMP2(sfx, "よ")){
            strcpy((char *)p, "し");
            strcpy((char *)p + cl, sfx);
        }
        else if(!STRNCMP2(sfx, "ば") || !STRNCMP2(sfx, "れば")){
            strcpy((char *)p, "すれば");
        }
        break;
    }
    return (char *)tmp;
}

/* 動詞の変化 */
const char *
jconj(const char *jverb,const char *sfx)
{
    struct _jconj_tab *tab;
    size_t len;
    struct _jconj_tab *best = (void *)0;
    size_t bestlen = 0;

    len = strlen(jverb);
    for( tab = jconj_tab; tab->main != (void*)0; ++tab){
        if(!strcmp(jverb, tab->main)){
            return jconjsub(tab, jverb, sfx);
        }
    }

    for( tab = jconj_tab; tab->main != (void*)0; ++tab){
        size_t mlen = strlen(tab->main);
        if(len > mlen && !strcmp(jverb + (len - mlen), tab->main)){
            if(!best || mlen > bestlen){
                best = tab;
                bestlen = mlen;
            }
        }
    }
    if(best)
        return jconjsub(best, jverb, sfx);

#ifdef JAPANESETEST
    fprintf( stderr, "I don't know such word \"%s\"\n", jverb);
#endif
    return jverb;
}

/* 可能 */
const char *
jcan(const char *jverb)
{
    const char *ret;
    static char tmp[JCONJ_BUFSZ];

    int len = strlen(jverb);
    if (len > JCONJ_MAX_VERB) /* 安全用 */
        return jverb;
    int prev = len - cl * 2; /* 2文字前 */
    if(prev >= 0 && !strcmp(jverb + prev, "する")){
        strncpy(tmp, jverb, prev);
        strcpy(tmp + prev, "できる");
        return tmp;
    } else {
        ret = jconj(jverb, "れる");
        return ret;
    }
}

/* 不可能 */
const char *
jcannot(const char *jverb)
{
    static char tmp[JCONJ_BUFSZ];

    int len = strlen(jverb);
    if (len > JCONJ_MAX_VERB) /* 安全用 */
        return jverb;
    int prev = len - cl * 2; /* 2文字前 */
    if(prev >= 0 && !strcmp(jverb + prev, "する")){
        strncpy(tmp, jverb, prev);
        strcpy(tmp + prev, "できない");
        return tmp;
    } else {
        return jconj(jverb, "れない");
    }
}

/* 過去 */
const char *
jpast(const char *jverb)
{
    return jconj(jverb, "た");
}

/* 敬体 */
const char *
jpolite(const char *jverb)
{
    return jconj(jverb, "ます");
}


/*
**      conjection of adjective word
**
**      Example:
**
**      形容詞的用法       副詞的用法
**
**      赤い            -> 赤く         (形容詞)
**      綺麗な          -> 綺麗に       (形容動詞)
**      綺麗だ          -> 綺麗に       (形容動詞)
*/
const char *
jconj_adj(const char *jadj)
{
    int len;
    static unsigned char tmp[JCONJ_BUFSZ];

    len = strlen(jadj);
    if (len < cl || len > JCONJ_MAX_VERB) /* 安全用 */
        return jadj;
    strcpy((char *)tmp, jadj);

    if(!strcmp((char *)tmp + len - cl, "い")){
        strcpy((char *)tmp + len - cl, "く");
    } else if(!strcmp((char *)tmp + len - cl, "だ") ||
              !strcmp((char *)tmp + len - cl, "な") ||
              !strcmp((char *)tmp + len - cl, "の")){
        strcpy((char *)tmp + len - cl, "に");
    }

    return (char *)tmp;
}


#ifdef JAPANESETEST
int
main(void)
{
    struct _jconj_tab *tab;

    for(tab = jconj_tab; tab->main != (void*)0; ++tab){
        printf("%-10s ない %s\n", tab->main, jconj(tab->main, "ない"));
        printf("%-10s ます %s\n", tab->main, jconj(tab->main, "ます"));
        printf("%-10s た   %s\n", tab->main, jconj(tab->main, "た"));
        printf("%-10s れば %s\n", tab->main, jconj(tab->main, "れば"));
        printf("%-10s とき %s\n", tab->main, jconj(tab->main, "とき"));
        printf("%-10s よう %s\n", tab->main, jconj(tab->main, "よう"));
        printf("%-10s %s\n", tab->main, jcan(tab->main));
        printf("%-10s %s\n", tab->main, jcannot(tab->main));
    }
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "ない"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "ます"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "た"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "れば"));
    printf("%s\n", jconj("徹夜でnethackの翻訳をする", "とき"));
    return 0;
}
#endif
