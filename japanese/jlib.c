/* JNetHack Copyright */
/* (c) Issei Numata 1994-2000                                      */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2022            */
/* JNetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <ctype.h>
#ifdef JAPANESETEST
#ifdef _MSC_VER
#include <windows.h>
#endif
#else
#ifdef WIN32
#include "win32api.h"
#endif
#include "hack.h"
#endif
#ifdef POSIX_ICONV
#include <iconv.h>
#endif

#ifndef JAPANESETEST
int xputc(int);
void xputc2(const unsigned char *);
#endif

#define EUC     0
#define SJIS    1
#define UTF8    2

#ifdef POSIX_ICONV
static const char* ccode[]={
    "EUC-JP-MS",
    "CP932",
    "UTF-8"
};
static const char* ccode_alt[]={
    "EUC-JP",
    "ShiftJIS",
    "UTF-8"
};
#endif

/* internal kcode */
/* IC=0 EUC */
/* IC=1 SJIS */
/* IC=2 UTF8 */
#define IC ((((unsigned char)("漢"[0])==0xe6) ? 2 : ((unsigned char)("漢"[0])==0x8a)))

/* default input kcode */
#ifndef INPUT_KCODE
# if (defined(MSDOS) || defined(WIN32)) && !defined(ICUTF8)
#  define INPUT_KCODE SJIS
# else
#  define INPUT_KCODE UTF8
# endif
#endif

/* default output kcode */
#ifndef OUTPUT_KCODE
# if defined(MSDOS) || defined(WIN32)
#  define OUTPUT_KCODE SJIS
# else
#  define OUTPUT_KCODE UTF8
# endif
#endif

#ifdef WIN32_PLATFORM_PSPC 
#define putchar(x) {}
#endif

static int      output_kcode = OUTPUT_KCODE;
static int      input_kcode = INPUT_KCODE;
#ifdef POSIX_ICONV
static iconv_t  output_dsc = 0;
static iconv_t  input_dsc = 0;
#endif

#ifdef JAPANESETEST
int rn2(int max)
{
  return 0;
}
#endif

/*
**      Kanji code library....
*/

/*
 * 引数が漢字(の1文字目)かどうかを調べる
 */
int
is_kanji(unsigned c)
{
    if(IC == SJIS)
      return ((unsigned int)c>=0x81 && (unsigned int)c<=0x9f)
        || ((unsigned int)c>=0xe0 && (unsigned int)c<=0xfc);
    else
      return (c & 0x80) ? 1 : 0;
}

void
setkcode(int c)
{
    if(c == 'E' || c == 'e' )
      output_kcode = EUC;
    else if(c == 'S' || c == 's')
      output_kcode = SJIS;
    else if(c == 'U' || c == 'u')
      output_kcode = UTF8;
    else if(c == 'I' || c == 'i')
#ifdef MSDOS
      output_kcode = SJIS;
#else
      output_kcode = IC;
#endif
    else{
        output_kcode = IC;
    }
    input_kcode = output_kcode;

#ifdef POSIX_ICONV
    if (output_dsc)
        iconv_close(output_dsc);
    output_dsc = iconv_open(ccode[output_kcode], ccode[IC]);
    if (output_dsc == (iconv_t)-1)
        output_dsc = iconv_open(ccode_alt[output_kcode], ccode[IC]);
    if (output_dsc == (iconv_t)-1)
        output_dsc = iconv_open(ccode[output_kcode], ccode_alt[IC]);
    if (output_dsc == (iconv_t)-1)
        output_dsc = iconv_open(ccode_alt[output_kcode], ccode_alt[IC]);
    if (output_dsc == (iconv_t)-1)
        output_dsc = 0;
    if(input_dsc)
        iconv_close(input_dsc);
    input_dsc = iconv_open(ccode[IC] ,ccode[input_kcode]);
    if (input_dsc == (iconv_t)-1)
        input_dsc = iconv_open(ccode_alt[IC] ,ccode[input_kcode]);
    if (input_dsc == (iconv_t)-1)
        input_dsc = iconv_open(ccode[IC] ,ccode_alt[input_kcode]);
    if (input_dsc == (iconv_t)-1)
        input_dsc = iconv_open(ccode_alt[IC] ,ccode_alt[input_kcode]);
    if (input_dsc == (iconv_t)-1)
        input_dsc = 0;
#endif
}
/*
**      EUC->SJIS
*/

unsigned char *
e2sj(unsigned char *s)
{
    unsigned char h,l;
    static unsigned char sw[2];

    h = s[0] & 0x7f;
    l = s[1] & 0x7f;

    sw[0] = ((h - 1) >> 1)+ ((h <= 0x5e) ? 0x71 : 0xb1);
    sw[1] = l + ((h & 1) ? ((l < 0x60) ? 0x1f : 0x20) : 0x7e);

    return sw;
}
/*
**      SJIS->EUC
*/
unsigned char *
sj2e(unsigned char *s)
{
    unsigned int h,l;
    static unsigned char sw[2];

    h = s[0];
    l = s[1];

    h = h + h - ((h <=0x9f) ? 0x00e1 : 0x0161);
    if( l<0x9f )
      l = l - ((l > 0x7f) ? 0x20 : 0x1f);
    else{
        l = l-0x7e;
        ++h;
    }
    sw[0] = h | 0x80;
    sw[1] = l | 0x80;
    return sw;
}

/*
**      translate string to internal kcode
*/
char *
str2ic(const char *s)
{
    static unsigned char buf[2048];
    const unsigned char *up;
    unsigned char *p;

    if(!s)
      return (char *)0;

    buf[0] = '\0';

    if( IC==input_kcode ){
        strncpy((char *)buf, s, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        return (char *)buf;
    }

    p = buf;
#ifdef POSIX_ICONV
    if (input_dsc) {
        size_t src_len, dst_len;
        up = (unsigned char *)s;
        src_len = strlen(s);
        dst_len = sizeof(buf) - 1;
        if (iconv(input_dsc, (char**)&up, &src_len,
                (char**)&p, &dst_len) == (size_t)-1){
            strncpy((char *) buf, s, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            return (char *)buf;
        }
        *p = '\0';
        return (char *)buf;
    } else {
        strncpy((char *) buf, s, sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        return (char *)buf;
    }
#elif defined(WIN32)
    /* 仕様上これを使うのは入力UTF8出力SJISの場合のみ */
    {
        wchar_t wbuf[2048];
        int len;
        int len2;

        len = MultiByteToWideChar(
            CP_UTF8,
            0,
            s,
            -1,
            wbuf,
            sizeof wbuf / sizeof(wbuf[0]));
        if (len == 0)
            return (char *)s;

        len2 = WideCharToMultiByte(
            932,
            0,
            wbuf,
            -1,
            (LPSTR)buf,
            sizeof buf,
            NULL,
            NULL);
        if (len2 == 0)
            return (char *)s;

        return (char *)buf;
    }
#else
#error need POSIX_ICONV or WIN32
#endif
}

/* UTF8文字列を内部コードに */
char *
utf8toic(const char *s)
{
  /* 入力コードをUTF8に固定してstr2icを使う */
  int k = input_kcode;
  char *ret;
#ifdef POSIX_ICONV
  iconv_t d = input_dsc;
  input_dsc = iconv_open(ccode[IC], ccode[UTF8]);
  if (input_dsc == (iconv_t)-1)
      input_dsc = iconv_open(ccode_alt[IC], ccode[UTF8]);
  if (input_dsc == (iconv_t)-1)
      input_dsc = iconv_open(ccode[IC], ccode_alt[UTF8]);
  if (input_dsc == (iconv_t)-1)
      input_dsc = iconv_open(ccode_alt[IC], ccode_alt[UTF8]);
  if (input_dsc == (iconv_t)-1)
      input_dsc = 0;
#endif
  input_kcode = UTF8;
  ret = str2ic(s);
  input_kcode = k;
#ifdef POSIX_ICONV
  if (input_dsc)
      iconv_close(input_dsc);
  input_dsc = d;
#endif
  return ret;
}

/*
**      primitive function
*/

/* print out 1 byte character to tty (no conversion) */
static void
tty_cputc(unsigned int c)
{
#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc(c);
#else
    putchar(c);
#endif
} 

/* print out 2 bytes character to tty (no conversion) */
static void
tty_cputc2(unsigned char *str)
{
#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc2(str);
#else
    while(*str){
      putchar(*str++);
    }
#endif
} 

/*
  1文字の長さを返す
  */
int
charlen(unsigned int c)
{
#ifdef ICUTF8
    if(c >= 0xf0){
        return 4;
    }
    if(c >= 0xe0){
        return 3;
    }
    if(c >= 0xc0){
        return 2;
    }
    if(c >= 0x80){
        /* 起きないはず */
        return 1;
    }
    return 1;
#else
    return is_kanji(c) + 1;
#endif
}

/*
 * 文字列の表示長を計算する
 * 多バイト文字は2文字分としてカウントする
 */
int
displen(const char *s)
{
    int len = 0;
    const unsigned char *p = (const unsigned char *)s;
    int maxlen = strlen(s);

    for (int i = 0; i < maxlen; ) {
        int l = charlen(p[i]);
        if (l >= 2) {
            len += 2;
        } else {
            len++;
        }
        i += l;
    }

    return len;
}

/*
 *  2バイト文字をバッファリングしながら出力する
 *  漢字コード変換も行う
 */
int
jbuffer(
     unsigned int c,
     void (*f1)(unsigned int),
     void (*f2)(unsigned char *))
{
#ifdef ICUTF8
    static unsigned char buf[8];
    static int bufcnt = 0;
    static int buflen;
    int cnt;

    if(!f1) f1 = tty_cputc;
    if(!f2) f2 = tty_cputc2;

    c = c & 0xff;

    if(bufcnt == 0){
        cnt = charlen(c);
        if(cnt == 1){
            f1(c);
            return 1;
        }
        buflen = cnt;
        buf[bufcnt++] = c;
        return 0;
    }

    buf[bufcnt++] = c;
    if(bufcnt < buflen){
        return 0;
    }

    buf[bufcnt] = '\0';
    f2(buf);
    bufcnt = 0;
    return buflen;
#else
    static unsigned int buf[2];
    unsigned int c1, c2;
#ifndef POSIX_ICONV
    unsigned char uc[2];
    unsigned char *p;
#endif
    unsigned char f2buf[16];

    if(!f1) f1 = tty_cputc;
    if(!f2) f2 = tty_cputc2;

    c = c & 0xff;

    if(!(buf[0]) && (is_kanji(c))){
        buf[1] = c;
        ++buf[0];
        return 0;
    }
    else if(buf[0]){
        c1 = buf[1];
        c2 = c;

#ifdef POSIX_ICONV
        if(IC == output_kcode)
        {
            f2buf[0] = c1;
            f2buf[1] = c2;
            f2buf[2] = '\0';
            f2(f2buf);
        }
        else if (output_dsc) {
            char buf_in[2], buf_out[16];
            char *src = buf_in, *dst=buf_out;
            size_t src_len = 2, dst_len = sizeof(buf_out);
            *buf_in = c1;
            *(buf_in + 1) = c2;
            if (iconv(output_dsc, &src,
                      &src_len, &dst, &dst_len) == (size_t)-1) {
                f2buf[0] = c1;
                f2buf[1] = c2;
                f2buf[2] = '\0';
                f2(f2buf);
            } else {
                *dst = '\0';
                dst = buf_out;
                while(*dst) f1(*(dst++));
            }
        }
#else /*WIN32*/
        /* SJIS to SJIS only */
        f2buf[0] = c1;
        f2buf[1] = c2;
        f2buf[2] = '\0';
        f2(f2buf);
#endif
        buf[0] = 0;
        return 2;
    }
    else if(c){
        f1(c);
        return 1;
    }
    return -1;
#endif
}

/*
 *  2バイト文字をバッファリングしながら出力する
 *  漢字コード変換は行わない
 */
int
cbuffer(unsigned int c)
{
    static unsigned int ibuf[2];
    unsigned char f2buf[16];
    unsigned int *buf = ibuf;
    void (*f1)(unsigned int) = tty_cputc;
    void (*f2)(unsigned char *) = tty_cputc2;

    if (output_kcode == UTF8) {
        if (c) {
            f1(c);
            return 1;
        }
    }
    else if(!(buf[0]) && is_kanji(c)){
        buf[1] = c;
        ++buf[0];
        return 0;
    }
    else if(buf[0]){
        f2buf[0] = buf[1];
        f2buf[1] = c;
        f2buf[2] = '\0';
        f2(f2buf);
        buf[0] = 0;
        return 2;
    }
    else if(c){
        f1(c);
        return 1;
    }
    return -1;
}

void
jputchar(int c)
{
    jbuffer((unsigned int)(c & 0xff), NULL, NULL);
}
void
cputchar(int c)
{
    cbuffer((unsigned int)(c & 0xff));
}

void
jputstr(const char *s)
{
    while(*s)
      jputchar((unsigned char)*s++);
}

void
jputs(const char *s)
{
    jputstr(s);
    jputchar('\n');
}

int
is_kanji2(const char *s,int pos)
{
    unsigned char *str;

    str = (unsigned char *)s;
    while(pos > 0 && *str){
        if(is_kanji(*str)){
            str += 2;
            pos -= 2;
        }
        else{
            ++str;
            --pos;
        }
    }
    if(pos < 0)
      return 1;
    else
      return 0;
}

int
is_kanji1(const char *s,int pos)
{
    unsigned char *str;

    str = (unsigned char *)s;
    while(*str && pos>0){
        if(is_kanji(*str)){
            str+=2;
            pos-=2;
        }
        else{
            ++str;
            --pos;
        }
    }
    if(!pos && is_kanji(*str))
      return 1;
    else
      return 0;
}

/*
 * 漢字の先頭位置まで何バイト戻る必要があるかを計算する
 */
int
offset_in_kanji(const char *ss, int pos)
{
    const unsigned char *s = (const unsigned char *)ss;
    static int mask[7] = {
        0,
        0xc0,
        0xe0,
        0xf0,
        0xf8,
        0xfc,
        0xfe,
    };
    if (IC == UTF8) {
        int c = 1;
        int i;

        /* 先頭なら常に0 */
        if (pos == 0) {
            return 0;
        }
        
        pos--;
        /* 直前の文字はASCII */
        if ((s[pos] & 0x80) == 0x00) {
            return 0;
        }

        for (i = pos; i >= 0; i--) {
            if ((s[i] & 0xc0) == 0xc0)
                break;
            c++;
        }

        if (i < 0 || c > 6 || s[i] < mask[c]) {
            return 0;
        } else {
            return c;
        }
    } else {
        return is_kanji2((char *)s, pos);
    }
}

/*
 * 漢字文字列を指定された位置で切り詰める
 */
void
truncate_japanese(char *ss, int pos)
{
    int len = strlen(ss);

    if (pos > len) {
        return;
    }
    int offset = offset_in_kanji(ss, pos);
    int newlen = pos - offset;

    if (newlen < 0) {
        newlen = 0;
    }

    ss[newlen] = '\0';
}

/**
 * サイズに収まるように文字列をコピーする
 * 切り詰めるときは、漢字の途中で切らないようにする
 * dest_sizeは少なくとも2バイトはある
 */
void
strscpy_japanese(
    char *dest,
    const char *src,
    size_t dest_size)
{
    size_t src_len = strlen(src);
    if (src_len < dest_size) {
        strcpy(dest, src);
        return;
    }

    size_t copy_len = dest_size - 1;
    strncpy(dest, src, copy_len);
    dest[copy_len] = '\0';
    truncate_japanese(dest, copy_len);
}

/*
** 8ビットスルーなisspace()
*/
int
isspace_8(int c)
{
    unsigned int uc = (unsigned int) c;

    return (uc < 0x80) ? isspace((int) uc) : 0;
}

static void
split_japanese_utf8(
    char *str,
    char *str1,
    char *str2,
    int pos)
{
    int i = 0;
    int width = 0; /* 表示上の位置 */
    unsigned char *s = (unsigned char *)str;

    if (strlen(str) < pos) {
        strcpy(str1, str);
        *str2 = '\0';
        return;
    }
    while (s[i]) {
        int cl = charlen(s[i]);
        int w = (cl >= 2) ? 2 : 1;
        if (width + w > pos)
            break;
        i += cl;
        width += w;
    }

    memcpy(str1, str, i);
    str1[i] = '\0';
    if (i == 0) {
        /*分割点がない(起きないはず)*/
        str2[0] = '\0';
    } else {
        strcpy(str2, str + i);
    }
}


/*
** strをposより手前の位置で分割し、str1とstr2にコピーする。
** str1, str2 はどちらもstr以上のサイズで呼び出し元で準備される。
*/
void
split_japanese(char *str, char *str1, char *str2, int pos)
{
    int len, i, j, k, mlen;
    char *pstr;
    char *pnstr;
    int sq_brac;

#ifdef ICUTF8
    split_japanese_utf8(str, str1, str2, pos);
#else
retry:
    len = strlen((char *)str);

    if( len < pos ){
        strcpy(str1, str);
        *str2 = '\0';
        return;
    }

    if(pos > 20)
      mlen = 20;
    else
      mlen = pos;

    i = pos;
    if(is_kanji2(str, i))
      --i;

/* 1:
** search space character
*/
    j = 0;
    while( j<mlen ){
        if(isspace_8(str[i-j])){
            --j;
            goto found;
        }
        else if(is_kanji1(str,i-j)){
            if(!strncmp(str+i-j,"　",2)){
                j -= 2;
                goto found;
            }
        }
        ++j;
    }

/* 3:
** search second bytes of japanese
*/
    j = 0;
    sq_brac = 0;
    while( j<mlen ){
        if(str[i-j] == '['){
            sq_brac = 0;
            goto found;
        }
        else if(str[i-j] == ']')
          sq_brac = 1;
        
        if(!sq_brac && is_kanji1(str,i-j)){
            goto found;
        }

        ++j;
    }
    if(pos > 2){
        pos -= 2;
        goto retry;
    }
found:

    if(pos > 2){
        if((str[i-j] == ']' ||
            str[i-j] == ')' ||
            str[i-j] == '}')){
            pos -= 2;
            goto retry;
        }
        else if(!strncmp(str+i-j, "］", 2) ||
                !strncmp(str+i-j, "）", 2) ||
                !strncmp(str+i-j, "｝", 2)){
            pos -= 2;
            goto retry;
        }
        else if(!strncmp(str+i-j,"？",2) ||
                !strncmp(str+i-j,"、",2) ||
                !strncmp(str+i-j,"。",2) ||
                !strncmp(str+i-j,"，",2) ||
                !strncmp(str+i-j,"．",2)){
            pos -= 2;
            goto retry;
        }
    }

    if(i == j){
        /* 分割点がなかった */
        strcpy(str1, str);
        str2[0] = '\0';
        return;
    }

    pstr = str;

    pnstr = str1;
    for( k=0 ; k<i-j ; ++k )
      *(pnstr++) = *(pstr++);
    *(pnstr++) = '\0';

    pnstr = str2;
    for( ; str[k] ; ++k )
      *(pnstr++) = *(pstr++);
    *(pnstr++) = '\0';
#endif
}

void 
jrndm_replace(char *c)
{
    unsigned char cc[3];

    if (IC == UTF8)
      return;

    if(IC==SJIS)
      memcpy(cc, (char *)sj2e((unsigned char *)c), 2);
    else
      memcpy(cc, c, 2);

    cc[0] &= 0x7f;
    cc[1] &= 0x7f;

    switch(cc[0]){
      case 0x21:
        cc[1] = rn2(94) + 0x21;
        break;
      case 0x23:
        if(cc[1] <= 0x39) /* ０～９ */
          cc[1] = rn2(10) + 0x30;
        else if(cc[1] <= 0x5A) /* Ａ～Ｚ */
          cc[1] = rn2(26) + 0x41;
        else if(cc[1] <= 0x7A) /* ａ～ｚ */
          cc[1] = rn2(26) + 0x61;
        break;
      case 0x24:
      case 0x25:
        cc[1] = rn2(83) + 0x21; /* あ～ん or ア～ン */
        break;
      case 0x26:
        if(cc[1] <= 0x30)
          cc[1] = rn2(24) + 0x21; /* Α～Ω ギリシャ文字 */
        else
          cc[1] = rn2(24) + 0x41; /* α～ω ギリシャ文字 */
        break;
      case 0x27:
        if(cc[1] <= 0x40)
          cc[1] = rn2(33) + 0x21; /* А～Я ロシア文字 */
        else
          cc[1] = rn2(33) + 0x51; /* а～я ロシア文字 */
        break;
      case 0x4f:
        cc[1] = rn2(51) + 0x21; /* 蓮～ 腕 */
        break;
      case 0x74:
        cc[1] = rn2(4) + 0x21; /* 堯 槇 遙 瑤 の4文字*/
        break;
      default:
        if(cc[0] >= 0x30 && cc[0] <= 0x74)
          cc[1] = rn2(94) + 0x21;
        break;
    }

    cc[0] |= 0x80;
    cc[1] |= 0x80;

    if(IC==SJIS)
      memcpy(c, (char *)e2sj(cc), 2);
    else
      memcpy(c, cc, 2);
}

#ifndef JAPANESETEST
/*
 * "put off"を対象によって適切に和訳する
 */
const char *
joffmsg(register struct obj *otmp,const char **joshi)
{
    static char buf[BUFSZ];

    *joshi = "を";

    if(otmp->oclass == RING_CLASS){
        Sprintf(buf, "%sからはずす", body_part(FINGER));
        return buf;
    }
    if( otmp->oclass == AMULET_CLASS){
        return "はずす";
    }
    else if(is_helmet(otmp))
      return "取る";
    else if(is_gloves(otmp))
      return "はずす";
    else if(is_boots(otmp))
      return "脱ぐ";
    else if(otmp->oclass == WEAPON_CLASS||is_shield(otmp)){
        *joshi = "の";
        return "装備を解く";
    }
    else if(is_suit(otmp))
      return "脱ぐ";
    else
      return "はずす";
}

/*
 * "put on"を対象によって適切に和訳する
 */
const char *
jonmsg(register struct obj *otmp,const char **joshi)
{
    static char buf[BUFSZ];

    *joshi = "を";

    if(otmp->oclass == RING_CLASS){
        Sprintf(buf, "%sにはめる", body_part(FINGER));
        return buf;
    }
    else if(otmp->oclass == AMULET_CLASS)
      return "身につける";
    else if(is_gloves(otmp))
      return "身につける";
    else if(is_shield(otmp)){
        *joshi = "で";
        return "身を守る";
    }
    else if(is_helmet(otmp))
      return "かぶる";
    else if(otmp->oclass == WEAPON_CLASS){
        Sprintf(buf, "%sにする", body_part(HAND));
        return buf;
    }
    else if(is_boots(otmp))
      return "履く";
    else if(is_suit(otmp))
      return "着る";
    else if(otmp->otyp == TOWEL){
        Sprintf(buf, "%sに巻く", body_part(HEAD));
        return buf;
    }
    else
      return "身につける";
}

/*
 * 数詞を返す
 */
const char *
numeral(register struct obj *obj)
{
    switch(obj->oclass){
      case WEAPON_CLASS:
      case WAND_CLASS:
        if(obj->otyp==SHURIKEN)
          return "枚";
        else
          return "本";
      case ARMOR_CLASS:
        return "着";
      case GEM_CLASS:
      case ROCK_CLASS:
      case BALL_CLASS:
        return "個";
      case SCROLL_CLASS:
        return "枚";
      case SPBOOK_CLASS:
        return "冊";
      case POTION_CLASS:
      case RING_CLASS:
      case AMULET_CLASS:
      case FOOD_CLASS:
        if(obj->quan < 10L)
          return "つ";
        else
          return "";
      case COIN_CLASS:
        return "枚";
      default:
        switch(obj->otyp){
          case CREDIT_CARD:
          case TOWEL:
          case BLINDFOLD:
            return "枚";
          case SKELETON_KEY:
          case TALLOW_CANDLE:
          case WAX_CANDLE:
          case PICK_AXE:
          case UNICORN_HORN:
          case LEASH:
          case STETHOSCOPE:
          case MAGIC_MARKER:
            return "本";
          case CRYSTAL_BALL:
            return "個";
          default:
            if(obj->quan < 10L)
              return "つ";
            else
              return "";
        }
    }
}
#endif

/*-------------------------------------------------------------------------
        全角文字のかすれパターン
  -------------------------------------------------------------------------*/
static const char *ro0 =
        "                        ・  ・，．  ．ｌ                ・      "
        "            ヽ゛    ゝ゛    ＾エマ  ／メＣ()－‐－‐            "
        "‐  ｜      ‥・・          ‘  ’   (  )            [  ]    {  "
        "}           〈  〉          「  」  （［）］－ｌ    ＋－／＼－："
        "－  ＝／<    >  ＜＝＞＝～∽・‥。  ○＋        ′  ゜ＣＹ＝Ｓ｜"
        "Ｃ｜    ゜／≠//    ×  Ｃａ        ☆  ()Ｃ○  ○  <>  ◇  コ凵"
        "□  ∧∠△  ∨￣▽  ×：＝┬－->－<-｜∧｜∨＝                  "
        "                            ⊂≡⊃≡⊂＝⊃＝(    )  しＪ||⌒    "
        "                            ＾/  /ｖ－  ＝→＝<>∨▽コ≡        "
        "                                    ／＿｜＿    ｏ  ▽  ＝  ＝  "
        "<<＜>>＞/   ～ｃ～ｃ‥・    ∫                              ゜Ａ"
        "゜／||＝｜っ！|)Ｔ＋†＝∥                  ()Ｃ                "
        "                                            ()Ｃ´｜⌒∠⌒⊃∠＋"
        "「⊃⌒ｏ￣ /ｏ３ｏ∋                            △∧Ｅ３( ⌒Ｌ )"
        "Ｆ≡「├Ｃ  ||┤ｌ  ｌ  |<  |_  ＮＶ||ＶＣ()Ｆ⊃ＯメＰ  ｃ⌒￣｜"
        "Ｊし /＼VVＮ／＼∨｜￣∠                        ｏ  ｌｏ    ｃｌ"
        "ｃ  ｌ－ｏ  ｜ｎ・ｌ・ｌｌ|<    ｎ||ｒ||ｃ  ｌｏｃｌｌ  ヽ  ＋－"
        "し||、  ｖvv／><ｖノ－∠                    めおιヽしヽ｀っ｀つ"
        "‐  んラょ  のよカヽか  キさき  <   く  |+†け  ニーこ  ＋ｃさ  "
        "l   し  ナ９す  七t+せ  てＺそ  ナこた  ５つち      ⊃ーつ  ー ("
        "て  ⊂  と  ナ  ｜こめのれわJ)ノよけは  は  て乙ひ  ひ  ハ小ふ  "
        "ふ  ／＼へ  へ  はまほ  ほ  キよ7+∂すしの()しキっ＋つ＋っ  り  "
        "。├ｏ上うっいノろ  オわ７っっォオつのるるんとナ/v～            "
        "                                ，－フノノｌノｌヮ  ワ宀┬⊥Ｔ⊥"
        "＋  ┼  ナ刀カ  ＝┼キ  ノフク  ノクケ  ニ ]コ  ナ++サ  ゛ンシ  "
        "フ人ス  ヤ七セ  ヽノソ  クメタ  ナ千チ  ゛ノ゛ノツ  ＝〒テ  ｜ヽ"
        "ト  ＋ノ－＿フメ亠不    ノヽハ  ハ  ├Ｌヒ  ヒ  ￣ノフ  フ  ／＼"
        "ヘ  ヘ  ┼小ホ  ホ  ヽフヾ≡∠ヽノヽ干七＋  ┼  ┐⊥コ⊥コ≡コ≡"
        "＝フｌノノレｌノコ凵┌フ「フキエアエラフヽノウ  ナ┐クノ        "
        "                        △∧Ｅ３｜￣∠∧ＦＬ７∠||┤Ｏ－｜  |<  "
        "/ ＼ＶＮ||∧ニ≡Ｃ()Γ┐ＦＤ>_∠￣｜∨｜￠ｏ／＼｜＋⌒          "
        "                            |3B                                 "
        "                                                                "
        "                                                                "
        "                                                                "
        "                △∧Г⊃Ｅ３｜￣Л  ＦＬЕ  К  ⌒っ||ＶИ  |<  "
        "J|ПＮＶ||┤Ｃ()Γ┐Ｆ⊃( ⌒￣｜ｖノ￠О／＼Ｌ⊥し｜||⊥ЦШЬ  "
        "Ь||｜⊃-)  |O├⊂┤                                            "
        "                ｏ  ( ｏЬг－      ｃ－е  к                  "
        "            с  г                                  ⊥  ш  ь  "
        "ь                                                              "
        "        －  ｌ                  │  ─  │  ─  ─│─  │  ┌  "
        "┐  ┘  └  ├  ┬  ┤  ┴  ┼  ├  ┬  ┤  ┴  ┼  ├  ┬  ┤  "
        "┴  ┼  ";

static const char *ro1 =
        "西亞亜  女圭B 可亠衣受心矣  女合    癸  西  亀  惡亜屋  屋  九日"
        "韋  戸  魚参木辛厂土卓斗及  宀夘女且虫亡食台糸旬糸  魚占戈  栗米"
        "ネ合宀女广奄安  日音安  門音革安木口人  尹  イ立衣  韋  囗井弓大"
        "禾女戊  示寸隹  音  尉  日勿奇      田  田共多  糸隹糸韋田月委  "
        "亠  胃  韋  貴  匚矢# ＃亠  或  月  有  幾  －  士ヒ益  免  禾旧"
        "次  于  魚弱ム儿Ｅ卩因  貝  囗大女因弓｜食欠壬  儿  陰  完  今云"
        "急  音員口寸石  宀于？  冫￢于  ？  卩  弟鳥規  五  石隹Ｅ∃咼  "
        "虚  口貝爵  尉  魚曼女老厂既甫  爪ム門王尊  二ム軍  雨云任  食耳"
        "又  宮呂貝女景彡日央由  宋  水  永  曳  英  乃皿ヒ頁ヒ頁央  行韋"
        "言永金兌夜  广殳八皿馬尺小兄言易走戉門兄夏  厂  冂  囗袁匚女大申"
        "宀女廴正夘心奄  爰  八口寅  火  火旧火西北廿袁  糸豕豊色夘  園  "
        "袁  金八夘鳥皿  方  シち生男凵  夫夬米大彳主广心甲  日王黄  区欠"
        "区殳工土公羽奥  鳥  区鳥共由冂  中  火  意  尸至意  意  甬  牛土"
        "てＺ奄  正卩因心日皿急  立日Ｔトイヒ反  可  加仂西  圭仕力口口￢"
        "吉加百夂女家豕  百分禾斗コ又田木加木哥欠可汀人  王可咼  木千禾家"
        "固  化  可  加  何  草  果  虫又言果華  化貝加  咼  雨  虫文我伐"
        "我  手戈オ  凵由臣人牙芋虫我加貝牙隹食我加馬へ/|へ云角  口  鬼  "
        "衣  廴回小夬又土毎  灰  衣  廾戈口刀己攵鬼斗毎  杙戒毎  厂火田介"
        "比白糸会介  解  門  皆  目  豈几亥力夕ト亥  吉  圧岸恨既根既圧  "
        "石寸芸皿行圭言亥金豈骨亥里  声香虫圭土亘木市虫万金鈎畫  赫  名夂"
        "广郭広  覚  各柊亥  売殳隻  石隹隻  見  用  赤  車交享  門各鬲  "
        "廿中字  丘山白木客頁咢頁卦  立  堅  田亘尾  魚秋臼勹害  易  小合"
        "舌  舌  易  骨  易  易  車害目  魚堅口十花  華  革包木朱白儿亀  "
        "浦  父金金兼歯  甲鳥木百矛  宣  弓米メリ刈  ？  卓乞イ口冖元共  "
        "干リ甚  隹力己  奐  甚  女∴元  宀口見  Ｔ十卓干串  咸  貫  感  "
        "奐  取  甘  亘  官  示欠隹欠干  夫  間  准  哀  廿  臣皿目  干  "
        "官  間  爰  牛山卓羽月干舟監完  隹見言東田貝哀  監  門  門  門  "
        "旧  卓韋食官舍官九  今口厂干厳  王元广品目艮山石習元雁  厂隹元頁"
        "彦頁原頁止  支  厄  吉  口大其  大可喜  奇  山支メ布幺戍己心軍  "
        "木几方其艮旡其月木其木  幾  帚  豕殳メ气シ气幺戈ネ斤禾李希  糸己"
        "？  夫見言己貝  走己車九光軍食几馬奇兄ム串申為  義  女支且  虚戈"
        "支  疑  其欠義  ヒマネ氏羊我義  言宜義  勹  勹  革勹士口乞  契  "
        "木吉矛  言吉石占木午禾  去卩各  却  虍Ｅ屮  斤  クスイ九イ木乃  "
        "及  呂  コ匚ヨ心求攵木５丁  及  立  久火王求穴九身弓及  及  糸十"
        "糸合｜日午キ土ム尸古匚Ｅ巨  処  手  巨木虍  言午足巨金居魚  御示"
        "ク田亠了亨  口小共  夫  喬  凶兀立兄廿  凵メ力∴匚王即卯口十呑向"
        "竟  夫  弓虫弓亘去  凡心共小夫  孝攵喬  兄  王  夫  喬  脳匈力∴"
        "同  喬  即  金竟郷音郷食敬馬イ卩疑  卉兀日尭木￥尸句田＃亟朽王  "
        "同  米千菫  菫力土勹冂十金帛厂Ｔ斤欠金欠王今林示人ム肋  臣糸斤  "
        "茵  ネ今禁  言菫斤  全玉口今金艮ナ L具  勹  匚メ句  王久矢巨古苫"
        "身区馬区馬丘馬句目旦禺  虍呉食  穴工禺  禺  禺  禺  中キ節  金川"
        "尸肖尸出屈  穴屈水日革化糸車穴圭能  畏  久米西木糸品又木金秋動  "
        "尹右重  言川君羊車  君  圭ト加衣ネ  系  化頃刊  口兀戸攵土王王圭"
        "刑  切  廾彡彳又思由广  急  舌自易  隹乃句攵旦京圭  夫  田圭尤旨"
        "糸  糸又糸米車糸卦  又土刑  虫  言十言旨敬言車又又頁夫鳥云  卩  "
        "魚京虍豕卓戈車手白方小日行  舛木人ケ夬决契  宀八糸吉皿  言夬日  "
        "イ牛使  巻  建  ヨ＃刀  央  宣  囗巻臣土女兼廴聿心  県系手  巻  "
        "央  隹  牟  大  南犬石  石見糸月目示戸月目兄言兼臣貝車干追  金建"
        "央  日頁馬央鹵央兀テ厂白厂敢幺フ弓玄咸  原  亠幺王見糸玄舟玄三口"
        "言彦艮  千平固  占士口乎囗古女古子瓜コ  广車弓瓜￣尸古攵木古沽胡"
        "瓜  粘胡夸  月殳古月孤  虍儿言夸足夸金古戸隹雇頁豆支王  彑  五  "
        "干ケユ口五口女呉幺夂卸従小吾木吾禽  王胡其石言吾言呉言隻酉胡乙  "
        "魚里六父交  ユ矢ユ矢幸  亠兀ハム工力交力ム勹厂子凵コ冂口厂石口侯"
        "土亢土后女子子し子土宀ムＴ⊥エ万共己土辛广ム广尹广隶弓ム小亘小荒"
        "亢  句  空  エ攵日印日光吏  亢  交  更  冓  シ工シ共シ告シ巷シ冓"
        "田中呈旱石更禾高米康糸工糸ム糸交糸岡耒井土５止月月ム月空高月舟亢"
        "芒川彳〒行魚言冓工貝貝冓交B 酉孝金広石広金岡門合夂ヰ工頁禾日亠向"
        "江鳥岡l|去力口万へ口豪  考  シ豪家  車∴麦勹古兄亥  牛吉匡玉禾殺"
        "酉告告鳴里  言犬シ鹿月要曽瓦勿心忽  月咼白  入  止比ヒ頁ヘラ因木"
        "土申豸艮女昏小艮豸艮氏日日比木艮木困昆  广艮糸甘日Ｒ云鬼此ニイ左"
        "又  允  山差ナエ羌左木且シ少王差石少言乍金貝沙衣土从广坐坐  責  "
        "隹  冉  取  吉戈基寒ヨ女辛  采彡オ丁采  木戈止戚斉  巛火禾ツ尸牛"
        "石卆此石夕示斉示糸田采  衣戈車戈B 祭斉  土  木才非  貝才牙  反  "
        "反  界  神  メ有口天山奇奇  石奇路鳥イ乍肖  口乍窄  日咋屮月冊  "
        "穴乍朿  糸  金昔木女魚圭世  是ヒ皿  尸巾祭  巛夕最  察  木し禾殳"
        "産  卆隹白早魚青別拐金青魚交ш||日西二≡个  ム彡凵⊥参  散  昔攵"
        "木戈火米王冊彦生目廾算糸天虫賛  夫貝酉允食  車斤斬日歹戈什士イ子"
        "司  イ吏朿  =]  丈中冊司匹口十⊥女台女市次女了一尸死巾  白巾士心"
        "田心旨  士又子攵其斤方也ヒ日木支上  歹ヒ[+EE師  ネ止禾ム幺小糸氏"
        "此糸月支月旨ム壬祝見言司言寺言式言志言咨次貝貝易此隹食司止米亊ヨ"
        "以  寺  旧儿宀子土寸幺心寺  日寺欠  幺  冶台雨  爾玉广寺石滋〒小"
        "冂  ４斗白目時  舌辛シ夕广比工弋諳哉田鳴竹二車由穴六雨下し＋口七"
        "幸丸矢夫女疾至  采心シ日黍  广失斤貝夫  部  條  佃思此木之  屎  "
        "芯∴糸高合  与  身寸舎拾赤攵余斗者  ネ土糸少土右討射亘卓庶  虫它"
        "牙  イ昔勹  尸八木勺火勺艮寸酉勺禾尺金易右  叔  弓羽若心王  耳又"
        "寸  壬キ牛  歹朱守  王朱禾重月重走取酉  自百需  又  兄  三寸受  "
        "木桔糸受雨而囗人又  吉  示  京尤川…攸彡秋心合  州  禾乃禾火糸冬"
        "粛  羽白自大丹  鬼  血  龍  隹言足就車耳周  酉  酉州隼木酉鬼イ十"
        "仕主允  一＋彳疋戈  矛木冫十止><畄犬糸従千里金充ホ又歹  佰  叔  "
        "ネ兄糸宿米  孰  孰  土山行ボボ  允  山允日夫舜  立允舛  馬允隹  "
        "盾  日句盾  歹旬享  准隼閏  后  糸屯巛  尊  酉享川頁夂几ネ刀戸斤"
        "日者署  者  广廿糸者者  聿  署  諸  言者且力余又くノ广予余  如心"
        "金助余  昜  賞  券  匚斤チ廾刀口肖  立冏昌  尚旨将大立女女昌肖  "
        "寸  ハ亅小  向  广土广木广敞章彡了水少  召  尚手走  日升日  日召"
        "昌品木公木肖木章木焦召  肖  歩  沐相火尭隹  昭  广正少目石肖石焦"
        "ネ羊禾尓音早夭  米庄糸召小月昌  将  焦  行重尚衣言公言正言召言羊"
        "豕  尚員将酉金正金重金童章  革肖├⊥ナメ了水禾  冖几乗  成  昜  "
        "襄  女襄尚吊青  憂  夂木木丈争  犬  田且禾襄丞  言襄酉襄金定属  "
        "直  食巾式  直  歹直火蜀糸哉耳哉ク巴角虫良  食虫辰寸尸九仲申言  "
        "ヨ又辰口女辰浸  番  ハじ真  辰  辛斤日  林∴秦  ヨ又冗木由甲广彡"
        "具貞ネ申禾  糸申巨  心  新  辛見言彡自オ立￥隹  金十雨辰ノへイ二"
        "刀  广  千士ヨ寸其匹尺  臣月言  十  車  革刃司  言取彡頁酉乍囗メ"
        "厂寸豆  口欠千申巾  隹  フ亅火欠目垂米卆羽卒哀  豕  酉卆金隹金垂"
        "有  王而骨有山宗山高米攵木区走芻芻隹居  木彡木昌官  皮頁少隹居  "
        "登  習  丁＋せ廿頼  田久日疋妻  市  丸力女生正  生  戌  正攵束正"
        "日生日青木妻木西止下青  牛生牛王成皿米青耳王士尸制衣四  言成折言"
        "言青折  酉星王月青争文  禾兄月危隹又广巾小昔成ボ斤  日  木斤口  "
        "責  竹昔糸責夫月王貝土亦足亦足責石頁七刀出  妾  耳><斤  言殳穴切"
        "竹即言兄雨ヨ糸色壬古虫単イ山牛兀十イ上口亘  由寸小大ノl|単戈戸羽"
        "巽  木全木丹白水戈  先  卆  替  前  扇  方疋穴牙竹前糸泉糸戈羊次"
        "月泉夕ヰ舟八广  言全貝戈足戈巽  巻  金戈金先門人魚羊月l|羊吉斬  "
        "タ犬个王ネ単糸善月善米厘唱曽朔土山且昔  八日田日林疋且  疋充疋束"
        "石楚ネ且禾且米且土糸糸且魚禾言斥且  朔  臼  曽  倉  又  業取合戸"
        "衣  士  禾  大  木  尸曽匚巾物心相心由又帚  千申蚤  品木旦干曲日"
        "単果倉  曹  曹  火品ク尹广由木目穴心米曹糸公糸宗耳公早  壮  死廾"
        "倉  品木壮衣土疋天  曹  金倉雨相馬蚤象  曽  曽  蔵  戊  貝曽告  "
        "足  則  貝  艮卩自心足  朿京則  口疋束  谷  尸禹貝戒方矢糸売亠从"
        "ネ由甘且前  ナ子子系酋寸員  木寸孫  也  夕彡大  太  言它垂  有  "
        "女  左月丁  木它舟它木左它  馬太馬単イ本土准文寸而寸代山丗巾侍  "
        "台心能熊異戈夫日夫水帯  月台退  台  代衣代貝艮  隶  豕  代黒魚周"
        "イ弋ム口人ナ竹弟酉是是頁雁鳥竜  龍  占早豕  宀モモ  尺  石  尺  "
        "ヨ准王豕言モ金澤蜀  言若耳  巾  虫肖口ハ口卩仁旦幸  厂  隹寸月兄"
        "己共巨又山  木朋ヘ口里  魚雪木尊言隹廾  早畄口夫土旦坦旦冗木日  "
        "夫欠炎  甚  山灰矢豆立而竹単糸定耳  月旦疋虫言延金段囗寸亶  弓単"
        "米斤日爰亶  Ｅ殳田力言炎直  矢口土也弓也耳心知日也  广知禾隹直  "
        "至攵虫知尸羊馬也筑木玄田ケTT竹凡畜  豕  禾失穴至ホ个女商羊目キ十"
        "イ中由  中心由  尺旦木主主  中  哀衣言主酉寸金寿馬主木雨猪渚者  "
        "丁  者  貝  一亅儿北周  口世龍  巾占巾長广丁弓丁弓長周彡彳攵徴  "
        "兆  申昜卓月朝  片世田丁目兆耳心月長月昜虫世言周言世走召足兆金兆"
        "Ｅ  丁頁烏白束力歩  目  月天シん王彡任貝金真東  聿  隊  木隹木追"
        ";g  追  广甬甬  家  木母国扣木規田  責  木石十辷鳥  糸又金咢木春"
        "貴  土平亜壺女需糸由イ八口巾金勺隹鳥亠丁氏  亭  貞  弟  占貝口王"
        "是  疋  立巾广氏广廷廴壬弔￥弟  氏  廷  是  弟  シ丁石定ネ貞禾呈"
        "糸帝舟廷言丁言帝足帝乕  氏  酉大金丁目  尼  商  ヨ推商攵商  白勺"
        "竹由商  金商弱  折口散  散  車散失  金失曲皿土真大  尸  广占夭小"
        "塵  舌甘貝占車云真頁占只仏云屏殳殿  日出雨甲兄  口土物  余  女石"
        "尸者走  ４  木土度  癶豆免  貝者余  者  金度石氏石万奴力广廿十上"
        "女又奴心到  尚兄夂  東  ノフ广  荅  唐  大長石  鳥  山鳥卓  殳  "
        "塔  束  兆  寿  東  次皿缶勹昜  寿  火丁火登ヨ  广豆寿  寺  合  "
        "同  米唐糸充至  重  湯  滕  言寸誉  三口足沓兆  秀  金登淘  豆頁"
        "馬  門  動  重力冂  尚土道寸童  童  同  童  立里月同菊  首  金同"
        "上下鳥  匚若日寸彳心売  持  叔目禾儿馬  母  虫  言売木万象  凵[]"
        "穴大段  尸由鳥弋占  宙  西  清静頓  山モ享  享攵屯  豕  盾  屯頁"
        "夭右雲  金屯大示丑刀冂人ケ下止  雉  迷  難  奈  金咼酋  馬川糸亀"
        "田又冂￥南  車欠夫隹女  一  尸ヒ二弋尓  ヒ勹辰  内  虫工++  口  "
        "孚しノ＼女口尸水韭  壬仟女壬刃心忍  需  爾  ネ尓心丁匆忽描苗丸  "
        "午ヰ今心念  然  然  米占ア  廴西亠Ｚ林土其衣ツ凶農  糸内ムヒツ凶"
        "農  曲辰司見又虫巳  巴  番  鞘  巴  皮  爪  王巴石皮波女馬  巴  "
        "Ｅ５非  発  抂扞非  貝攵木不不皿片卑北月月市非車酉己位口立口女某"
        "毎  某  某  貝  冒員士冗貝立立口言  虫亀禾平矢引秋  仁白ヨユ専  "
        "白  白  白  日臼泊  米白舟白専  白  暴  莫  暴  専  旦天馬爻友  "
        "凵  相  石谷者  聿  舌  慮  番  月几火田昌田ノ＼金本発  癶  酉発"
        "髟友代戈詈  友  伐  門伐九鳥新  高  虫合隹  半  半  キ￥厂又半反"
        "巾凡投般王文反  :e  凡  片反)e  王リ田半敏  舟殳蕃  貝反軋  禾米"
        "火頁分頁食反免  免  釆香般皿般石番  亦虫匚非田ヰ丕  女己广比皮  "
        "非心戸非比  皮  非文上ヒ必  广皮广支石卑必  非  能  月巴皮  非  "
        "弗貝辟  北]E升  通桶其皮用  尸毛彳攵比  田昆王比尸目羊夫畠  木冬"
        "卑  匚儿下Ｅ髟比广彡漆  芟  月寸弓百心  里  聿  田  木会女臣女爰"
        "糸丑白  羽彡表  虎彡票  水  票  票瓜西示衣  言平豸勺广朝苗  广丙"
        "禾少田  金苗金兵示  虫至魚老口∴林杉武  兵  渉頻負分貧  歩頁毎攵"
        "并瓦ハ个イ寸阜  大天女帚冨  同田ナ巾广付布  夫  放  父斤並日孚  "
        "八乂付  府肉虍胃夫  言普自貝武  走ト♀  付  毎  無  正弋無舛甫荀"
        "無  立口圭寸風  凧虫茸  路  代犬口田彳夂巾田月反ネ田月夂ネ夂復  "
        "川  弔井ム  弗  イム牛勿魚付八刀勿  賁  賁  賁  分  林火售  米分"
        "米異糸分雨分亠乂門耳内  并  丘  屏  敝巾干  敝廾丙  ∥工敝  門才"
        "比土木※百貝辟  辟  辟  珀石万リ敝目茂戍内比扁  亦夂广┐扁  扁  "
        "刀  反  扁  更  免力女免ム廾革便休呆舎甫金甫囗甫甫  止少用申甫  "
        "車甫恵  莫分莫土莫小弋  莫日丹ヨ専  苙ロ放  奉  己勹ロ木幸反个夫"
        "玉  山夂山夂朋  广包包  奉  方攵万ナ月  去  包  享  石包糸逢包  "
        "方  明  逢  虫  保  言方曲豈≠  金  食包鳥  朋鳥之  亠匸旁  立ロ"
        "土方女方巾冒亡心亡  戸方共  亡王甘呆俸奉日目糸方月方彭  言某豸皃"
        "貝  金牟方  ロ犬規頁コヒ美  ｌ丶黒  美  木ト牛攵陸  禾彡金口字力"
        "殳  歹台屈  晃  介卉木  番羽几  分皿麻手麻石麻鬼广林里  女未日未"
        "木攵毋  里  真  莫布莫  木尢魚有木正魚尊舛  亠小ロ天ヌ乂末  木  "
        "末  乞  尽  糸虫麻呂ク下曼  両  曼  曼  未  木  鬼未己  其  山甲"
        "宀必宀必奏  衰  念  派  女少米毛氏  目民矛攵夕  冊丗ム牛予オ雨務"
        "武鳥木京女疋女良冖旦夕ロ叩合日月明皿米  金名ロ鳥女至牛ヒ灰戊ク兀"
        "木帛糸帛面  而百麦面莫  莫  戊  亡女子皿もモ孟  亡目糸罔未毛家  "
        "信諸十大默黒日月木工勹  食并尢  戸大米刃世貝門ロ門心糸文F]  勹刃"
        "セや台  亠  父耶耳  里予尓  天  厂  彳殳勺  楽  尺  足隹立青卯  "
        "数  遣  前  前  由  愈  言前車前ロ隹イ右憂  男  ナ又有  幺山攸  "
        "愛百耳  右月木由勇  甬  酋  酋犬田  ネ右ネ谷言秀游  巴  垂  広隹"
        "鬲虫タクマ丁ホ  ５ふ言  六  予頁庸  幺力女夭谷  广用昜  缶  推  "
        "唯  昜  羊  羊  容  容  月甲空羔王半光隹世木容  西女言缶甬  缶  "
        "昜  食  欲  印  谷欠夭  谷  羽立羽異定  維  累  果  未  来  束頁"
        "雨田各  糸各洛  酉各舌し卯  風  闌椚監  監  闌  臣見禾リ史丈尸復"
        "木子利木王里王禽广利里  ネ里甲早禽隹先土聿  卒幺亠ニ律  京  田各"
        "金  充  留  充  刀田充  米立夂生音甲立  呂  虍思方TK虍男フ亅冗  "
        "亰  冂山先夂亰  米斗朶  京  鼡  广亰亰  先夂量  艮  言京亰  旦里"
        "先夂令  刀ナ糸ヨ侖  厂里木  沐林米舛林  臣品車侖米舛米舛鹿舛留  "
        "里  戻  田糸米大今  令  列  令  万力領  令  王令ネし令  金令款隶"
        "雨令雨並鹿  歯令厂林厂林歹リ少力列  列衣广兼亦心米舛連  火東廉  "
        "糸東聡  連  車  金東ロ口魚  魯  火戸貝各足各雨路力  女  广郎王廾"
        "良月米女郎  良  尸雨牛  良  竜  土ヒ龍耳虫鼡良  亠八鹿  ネヨ月力"
        "金ヨ侖  委  禾ロ言舌不正貝有力∴或  木卆就鳥互万旦  魚咢言宅高  "
        "厥  宛  弯  宛  宛  ";

static const char *ro2 =
        "弋  正勹不  ヘｌりル    井      ノ＼千北乖  乳  ｌ  象  争  舎予"
        "弌  丁エ][  工吸一  亠几京  宅  回且人^^乃  厂人イトイカイ丈イ刀"
        "イ刃什千介  亢  失  古  弗  句  它  宀丁吉  多  朱  宅  兆  凧  "
        "百  有  羊  木从合冊盡  見  矣  人且仔  免  甬  里佳利  弟  車  "
        "奇  居  屈  兒  空  卒  仇卆叔  昌  青  卓  卑  府  門  兩  匚  "
        "コ又曾合皆  彦  易  故  者  忽  兪  鬼  交  専  區  倣敖命      "
        "専  婁  喜  爲  尭  替  替  童  賈  亘田僉  雋  農  監  斉      "
        "夢  品  難  麗  厳      ル  儿  臼兀兄  免  克  竟  雨  前  八５"
        "北異Γ ]冂巳卅  由冂冂同由月♯冉免  一￢冖兔冖元豕  臼勺幕      "
        "夬  互  中  水  兄  列  固  京  禀  儿  虍処木几任几皇几Ｌ」凵亟"
        "刃  チリ夬リ勿  去刀冊  舌リ夸  利  并刄巛  克  束  奇  易  前刀"
        "豈  乘  荅  巣  票  僉  僉刄僉刀刄  辟  斉  辛  辛  句力召力吉力"
        "夫力巛  京力助  労火責  巣      彡  勲      隹  ノフ勿刃凶勹田勹"
        "甫勹勹  夸包ヒｔＬΓ匚甲匚准匚貴匚  ￣Ｌ匚品九十廾川廿せ廾十十  "
        "隼准下亠Γ|]厄  タ卩谷卩巻  ノ￣厂彡厂則厂夏厂  厂斯厂敞ム  参  "
        "算  隻  臼支日又    ロ丁ロ刀ロ八ロ入ロ于ロ牛ロ牙ロ斤ロ亢ロ孔ロ允"
        "ロ内ロ分文ロロ尺ロ永ロ可夂ロロ玄ロ瓜ロ甲ロ此ロ几ロ申ロ且ロ奴ロ出"
        "ロ付ロ包ロ圭ロ万戌或ロ至ロ交ロ共ロ合ロ次尺只ロ西ロ宅ロ老向  ロ行"
        "可  我  希  吾  更  孝  犬  甫  弄  於  圧  卸  空  隹ロ又::卓  "
        "炎  臼  念  戻  話  啄  客  各  咸  胃  帝  秋  而  叩  単  帝  "
        "南  兪  刺  亮  烏  臭  差  夏  老日虫  眞  區  敖  責  族      "
        "麻  連  壹  器  営  此角斯  朝  無  意  禁  肅  筮  品木高  寧  "
        "鼻  劉          燕  郷向嬰  厳  器  爵      雑      藝  贈      "
        "齧  コ凵囗化囗令囗方囗有囗吾囗幸囗巻囗或囗韋囗員囗専囗  囗巫囗哀"
        "土入土下土山土欠土斤土止土不土甘乖  代土土皮土付土包土亥土艮土行"
        "土至土并上下土矣土角土甫土寸土寸土亜土花土卒土朋土西土咼土昜土保"
        "土烏土火土海臼殳土時土正土斬土野土間土虚土尊土奥壊  土嗇土著堕  "
        "擁  厭  叡  盡  廣  品  墨  曇  壌  龍  壮  売亞豆  疋月壺  尋  "
        "久タ夂      ヨ歹林夕果多夫  大  大十大５大  立可亦大四大大圭幺大"
        "壮大者  酋  奥  将  區  女干女勺女  イ女妄  女比女旦女母女夷羊女"
        "女廾女任兆  我  女月沙女那  女  女男女阿女壬宛  取  取  卑  林女"
        "眉  皿  冓  弱  臼支女馬焉  區  常      票  間  閑  喬  單  辟  "
        "男女男女女  鼻  頼  嬢襄    霜  了  乃子子  字  奴子亥  享丸幺子"
        "卵  学  文子需  亠  宀ヒ宀臣辰  免      隹  是          貫  寝  "
        "莫  羽彡臼勺哀  貝  貝  克寸将  専  業寸ケ小甚  大九尢彡厂  尸广"
        "尸比尸  尸米尸貝尸  尸并尸  尸  山ナ山し山刀山乞及  今  分山山女"
        "山由山氏山白山平山民山弁山占合  山寺我      山更山肖鳥  山谷＋  "
        "圧  岡  竒  金  屈  昆  隹  争  山夂侖  侖  甘欠品  禺  眉  鬼  "
        "差  弱  區  斬  章  尭  登  義  僉  獄  隆  疑  與      魏  顛  "
        "    厳  <<  工从己  后巴氏巾ヨ巾巾失奴巾白巾    巾隹巾屋巾韋巾貞"
        "巾冥巾國曼  音弋巾童敞巾封巾++TT廾＃ム  麻幺亠厂广羊广則广相广夏"
        "广既广殷广彡广黄广斯广寸广墨广發广無广解广稟广盧广隹广  广聴    "
        "廴由ナ十廾  廾              式  弓  奴弓弓耳朋  哥  弾  爾  言弓"
        "亦弓ヨユ豕  ヨ  彑果ノ  吉彡イ  彳方彳生彳且彳弗彳回彳艮彳巛彳旬"
        "彳  徒  非  彳  彳皇彳缶彳  寸  斤  午  丑  尢  夭小直  分心台  "
        "在  古  句  尼  乍心匆忽旦  白  弗  平  央  永  圭志任心各  休心"
        "匈  力∴互  光  次心寺  血  旬  舌  同  羊心月  旱  具  困  束  "
        "肖  允  字  免  邑  利  吝  亞心季  恵  巻  卒  卆  妻  周  長  "
        "罔  皿  咢  衍  皇  春心秋  而  星  宣  忽  則  巛  民攵日夂殷  "
        "気  穀  鬼  兼  原心眞  朔  倉  専  涌  栗  堅  康  参  斬  斬  "
        "従  習  昜  造  専  動  匿  票  庸  喜      甜  景  焦  單  備  "
        "馬  閔  無  幸  奥  雁  懐  解  勤  品  言  林  隹  稟  需  満  "
        "頼  栽  栽  恣  隹  瞿  聶  恋  弋  戊  戊  戊  戈  戈  戈  甚戈"
        "隹戈戈  戦  戯  戈  戸冊キしキ干キロキ工キ叉刀  厄  王  夬  戈  "
        "予  爪  斗  抜  下  不  幼  甘  申  奴手合手折  言  占  拝  半  "
        "付  弗  母  九力立  各  吉  共  羽  圭  手  丞  存  ロ月夾  旱  "
        "臼支日土夜  奇  欣  取  垂  制手勹  卓  定  命  門  戻  豕  皆  "
        "東  癸  而  柔  千臼耶  兪  夕缶寒拳冓  差  弱  倉  聶  島  羽  "
        "専  崔  執  専  羽彡覚  斯  堯  發  亰  勞  感  豕  禽  亶  幸  "
        "達  辟  雷  捫  與挙與キ斉  室  台  壽  賓  覧  箭  廣  鄭  罷  "
        "挙  楽樂襄  崔  賛  難  糸言隻  又  又ケ攵  攵  イ攵田攵交攵放  "
        "束攵余攵余攵尚攵巾攵高攴数  僉攵敝死糸言角斗甚斗石斤断  方丹方市"
        "方立方毛方生方充方番方番チモ无  早干日  日天日仄日文木日日尼永  "
        "日卯日勿安  日光晋  日兆日希書  日吾日告日辰日成折  析  非  軍  "
        "英  軍  宣  昜  冥  旦  進  堯  敦  敝日華  亰  愛  蒙  廣  広  "
        "義  共  Ｅ口申戈日匂月出月良其  蒙  龍  雨鞘ホ  市末乃木木入木力"
        "木刀木干木己木工木弋木山木万木王木  木公木予木少木分木方木戸木升"
        "木廾朸加木可木拐束  木只木匚木句木且木乍木斥木氏木出木包木付木立"
        "木瓜木曾木干木匡木羽木舛木夸木考木至木充木存木牟木当木角木旱木鳥"
        "木告木允木巵木攸木那木廷木壽木兵木孚林凡木呂林下木亞木七木君木享"
        "木其木勺朿  木国木券木岡木空昆  木昏木戔木宗木允木叔木妾木朿木  "
        "木知木卓木尚木念木府木並木門木典木至木侖木乃木皆木胡木秋木耳木契"
        "木泉木者木甚木帝木豕林矛木耶木兪木方木柬木室木世木皿栄  木鬼木豈"
        "木高木貢木骨木差木寒木朔木島木羽木般木匪木火木専木冥木旁木容木留"
        "木竜木郭楽  木彡木圭椎  木皐角斗木斬木従木衰木區木戚木巣木専林大"
        "木密木蜜木永木婁木敢木貫木喜木最木晶木毳木隋木登木童木堯木美木鳥"
        "木言木意木敬木放木僉木嗇木辟木檗木観木匱木隹木寧木賓木蒙木縁木品"
        "木樂木楽椚閭木歴木嬰木擧木  木雨木言木覧缶彡木観矣欠希欠盗  奇欠"
        "飲  欠  臼欠兼欠區欠羽欠虚欠僉欠與欠隹欠師帚夕万歹又歹夭歹彡歹央"
        "歹孚歹戔歹立歹員歹昜歹壹歹單歹賓歹弋歹弋几又殳  売殳區殳丑＃毎充"
        "少毛毛求毛  毛  毛炎麻毛亶毛亡民＝し气分气因气米工水山  土  王  "
        "斤  互  止  心  市  分  日  反  又  木  世  央  弘  古  四  因  "
        "斥  且  它  占  田  乏  民  平  目  夷  行汀匈  血  合  光  朱  "
        "旬  汝如西  列  完  月  宏  允  夾  折沂延  弟  壽  日土奄  シリ"
        "米川函  其  金  固  肴  卒  沐松尚  爭  妻  沐析戔  宗  於  土儿"
        "侖  准  胃  西土苛  奐  爰  皇  軍  沐旦秋  沐泄泉  而  亭  拝  "
        "泪沙面  勃  満  兪  方子刺  矣  法皿晃洸囗  宰  辱  朔  倉  叟  "
        "臼  月泰唐  浦専泣旁冥  頃  既  淮  涙邑許  哀  將  参  束欠帯  "
        "張  條  泳  凶ム鹵  堯  尸  ホ月止  止  尋  潜  潜  早  攻  童  "
        "番  彭  亜  廉  亰  奥  幹  沐品幸  言  貝  零  斉          爾  "
        "寧  賓  僕  蒙  冩  審  賤  暴  養  劉  慮      幹  猪  歴  盧  "
        "肅  濔彌闌  斂  漉麗糸言夕火火少火冂火冂火巨火乍火丙火包火因休  "
        "火丞火各正  火夂火昆火立火奐巨巳煕  句  火  火皇火爰火昜火重火熏"
        "火息火貢火尉敖  火門喜  火弋火堯火敦火番火亰火奥火毀火遂火達火盡"
        "火豕習隹火樂火盧火闌焚  ヨ尹爪巴友  為  メ乂爻且=|  爿  爿嗇片戔"
        "片買牛氏牛吾牛禾利牛牛∴牛高牢  牛買牛義尤  才  丑  中  火  甲  "
        "弗  各  艮  交  狭  ロ月攸  奇  兒  青  昌  卒  侯  而  星  畏  "
        "骨  將  莫  黒犬厥  曾  蜀  寧  獣  鼠  犬  瀬  加  代  尓  皮  "
        "白  耳  凧  各  亜  良  郎  虎  吾  非  法  コ又軍  必  凶巛冒  "
        "兪  宝  鬼  王貝馬  王缶僅  章  美  辟玉夏  龍  珱  女  夸瓜辛瓜"
        "瓦十瓦千瓦公瓦屯瓦分瓦毛瓦百瓦姿瓦西瓦秋瓦厘區瓦専瓦夢瓦瓦  瓦  "
        "尚甘更生マ用田丁ツ田田介田井田夫田彡田ム田ム田衣田時田各田書田余"
        "田奇田尚彊  田壽田寿畳  田  塁  广丁广久广山广介广尤广加广甘广玄"
        "广此广且广旦广冬广包广夷广全广羊广巛广志广否广阿广委广固广卒广炎"
        "广卑麻  麻  广風广昜广兪广皿广虐广脊广倉广般广留广章广累广妻广間"
        "广發广勞广殿广萬广疑广養广隹广  广積广歴广鮮广隹广顛    癶天癶殳"
        "白ヒ白儿白反白大白交白完白告析白白豈皮包允皮軍皮皮軍芻皮于皿去皿"
        "羊皿合皿戔皿聿皿水皿虍皿湯皿蕩皿目兮目尢目少目  目玄目尼旨具此目"
        "目此目未夫目目牟目弟目圧目兒目走目青目卑目  目幸目者目舌眞  冥  "
        "堂  目  目敢目貴目愛目隹目僉目鼓目言目蒙瞿隻直品目蜀矛今ム矢矢委"
        "石工石切石比石廣石且石萬石圭石卒石花波石石朋石立石彑石日石貞石甚"
        "石巛石豈石骨石差石舛石展石馬石旁石品石声石責石専石堯石登石奥石義"
        "石當石疑石林石樂示巳示司示氏示出示乍示必示弋示其示彑示契示允示喜"
        "示斉示單示豊示襄虫  甲虫千ヨ禾比禾央禾巨禾  禾末禾旱禾肖禾其禾直"
        "禾周禾回示回禾冉禾臼禾高禾允禾容禾恵禾犀禾嗇禾歳禾急禾亀禾襄穴弓"
        "穴井穴幼穴タ穴兆穴君穴告穴咼穴  穴缶穴妻穴放穴鼠穴隆穴遂穴賣穴禾"
        "立十立千立分立毛立占立丁立  立百立矣立束立易立厘丸  勿  爪  巴  "
        "加  占  生  台  氾  本  矢  匡  筐  幵廾旬  尹  全  先  延  呂  "
        "夾  見  作  攸  成  巫  拑  菌  昆  拒  空  答  戔  帚  爭  争  "
        "服  夾  皇  侯  甘欠咸  豕  冓  師  衰  蓑  昆  栗  龍  責  族  "
        "彫  畢  逢  梁  妻  焉  貴  黄  替  西早言  肅  答僉壽  監  数  "
        "旗  留  滕  滕  頼  筏  筏  龠  離  米十米比米戸奥万米合次米米而"
        "米同米百米更タ米米梁米良米卒米宗米花米柔米甚米家米用米麻米莫粥  "
        "米需米萬米雑米  糸し糸于糸寸糸云糸比糸文糸冂糸広札糸糸世糸台糸丁"
        "糸半糸ヰ糸光糸行糸  糸戎如糸糸曳糸并糸巛糸秀糸攸糸妥糸呂糸忍糸奇"
        "戸糸糸巻糸采糸畄糸卓糸戔糸恩糸周糸缶帛系糸侖糸戻糸官糸咸糸耳糸世"
        "糸段糸致紗  糸昏糸威糸益県系糸宰糸差縦  糸辱糸晋糸追糸  糸彡糸強"
        "麻糸糸曼糸票糸崩糸婁糸累糸連糸雲糸門糸散糸堯糸番糸亰糸幸糸曾糸  "
        "糸継糸需糸斉糸斉糸賓糸辛糸監結頁糸賛糸賣糸墨絞頁糸嬰糸免糸戈糸戈"
        "毒糸糸覧缶工缶夬缶  缶貝缶  缶曇缶隹冂メ干  冂亡不  古  民  奄  "
        "卓  林  討  絹  熊  幕  革馬革奇羊兀羊  差丑羊氏羊令羊君羊  羊  "
        "羊美羊美羊亶羊  羊言支羽卆羽立羽合羽羊羽非羽前羽扁羽殴羽堯羽番飛"
        "老旨老毛老至禾未耒云耒巴耒呂耒助耒辱耳火耳止耳卯耳令耳舌耳由取  "
        "知耳耳定耳廾従耳声耳耳  耳  耳寧耳  ヨキヒ聿長聿聿川月工月亡月土"
        "月内月日月尤月甲月疋月乍月氏月由月丕月半月永月夸月光月巛月攸月辰"
        "月甫月夜月左月典月卑月非月府月并月建月思月星月巛月臾月皿月鬲月専"
        "月旁月旅月巛月國月雪月窒月室月昜月貢月番月卒月曾月左月言月殿月辟"
        "月雁月僉月斉月需月燕月鼠月葛月盧月蔵糸肉臣戊室  至秦臼人臼廾臼春"
        "臼男臼六臼隹合舌舌氏舍甫舟公舟方舟可舟由舟孚舟倉舟叟舟雪舟曹舟童"
        "舟義舟嗇舟蒙舟魯舟盧舟戸革艮豊色屮  ++乂勺  亡  元  殳  勺  分  "
        "以  巨  句  冉  且  冬  母  毎  氾  付  平  包  卯  目  末  立  "
        "因  回  各  幺  朱  旬  如  存  合  伏  芒  名  刀∴位  延  我  "
        "含  夾  巛  呉  沙  助  壯  余茶兎  豆  忍  秀  利  良  奄  宣  "
        "中王昆  叔  卒  松  妻  青  帚  長  波  非  泙苹泡  朋  大廾臾  "
        "凌  林  Ｅコ科  咢  咢  冠  軍  胡  芻  段  帝  巴  保  禺  約  "
        "施  咼  翁  太皿兼  高  苙句座  著老弱  修  辱  秦  席  昆  旁  "
        "祭  宿  専  庶  参  疏  族  帯  匐  羽彡棘  舜  堯  早  尋  心  "
        "止  猶  温  歹韭曾  亘田魚  夢死肅  嗇  辛  婁攵微  辟  預  雷  "
        "稜  耒昔斉  蔵  室  貌  耒禺芸執樂  黎  謁  薀  禾魚頻  頼  門隹"
        "盧  龍  鮮  縵  羅  上七斤市虍文号虎虍汚虫  虫引虫公虫山虫斗虫内"
        "虫キ虫甘虫丘虫古虫且虫由虫令虫萬虫包虫回虫舌虫凡虫共虫交虫朱虫老"
        "虫延虫見虫呉虫勹虫辰虫兌虫延虫孚虫余虫甬虫利虫易虫宛虫巻虫青虫析"
        "虫周虫非虫田虫胃虫咼虫科虫易虫胡虫皇虫ナ虫夂虫扁虫兪虫子虫昜虫黽"
        "蛍  虫冥虫郎虫敖虫悉虫冬虫率虫常虫隹虫赦虫執虫堂虫幕虫莫虫婁虫堯"
        "虫品虫番虫解蝎欠虫言虫聖虫當虫奔虫  虫栄虫隻虫需虫春虫豕蟲皿虫  "
        "虫  虫  虫糸血丑血刃行玄行吾行  行隹ネ彡哀  今衣哀  ネ日ネ壬衽任"
        "ネ内ネ夬ネ彡ネ旦ネ尓ネ白ネ半ネ包茅柔母衣ネ圭ネ伏上下ネ行衣冏求衣"
        "ネ君壯衣裏  袿卦ネ易非衣ネ卑ネ巾ネ妻ネ軍ネ扁ネ保衣  温  ネ辱ネ退"
        "ネ虎ネ強表  執衣ネ習ネ婁ネ單ネ単ネ當辟衣ネ需ネ監ネ吉ネ蔑ネ親ネ闌"
        "ネ擧冖  襾早襾放襾馬見  見占見巫見者見兪見豈見冓見  覚  覧  見賣"
        "見隹角瓜角此角氏角羊角昜角蜀言ト言乞言干言工言化言牙言内言可言古"
        "言且言台言氏言  言灰言危言后言旬言朱言兆言耒言毎言戒言狂言告言甬"
        "言肖言巫言享言爭言臼言定言柬言音言皆言咢言韋言虐言宣言軍言風言扁"
        "言臾言哥言寒言益言兮言田言必言旁言缶言區言革言声言商言曼言莫言華"
        "言爲言幾言矛言登言替言替言早言  言品言辟言幸言遣言與言賣言燕隹誰"
        "言免言襄言戈言隹言賛谷牙害谷谷  山豆豆宛堅豆幽豆不了豕拳豕者彡  "
        "豸才豸召豸各豸休豸百豸里豸兒豸比豸比豸莫貝戈貝盾貝今貝台貝此貝弐"
        "貝弐貝乏買  貝  貝戔買  貝來貝寒貝兼貝専貝執貝敖貝賛貝武貝盲貝言"
        "貝盡貝斉貝戊貝庄貝品貝賣赤反赤者之  走４走珍走肖足支足止足夫足加"
        "足冊足石足失足皮足犬足危足凡足艮足先足局足束足良足走足果足居足戔"
        "足知足柔足重足兪足勇足  足寒足差足倉足脊足臼足戚足従足庶足宗足両"
        "足畢足厥足尊足美足品足著足蜀足辟足斉足壽足質足鄭足塵足門足躙足聶"
        "身弓身本身豊身朶身美身雁身應車し車厄車  車失車可車彡車式車至車各"
        "車巛車耶車取車免車畄車又車両車両車夫車奏車田車夂車袁車穀車展車雪"
        "車専車鹿車喬車感車需車樂車歴車盧辛  尸辛辛束辛  辛言一  中  冂  "
        "沼  由  外  爾  回  后  亦  西  求  巡  允  肖  呈  火  浦  浴  "
        "委  土儿辛  并  渇  コ又皇  酋  酉  貞  兪  南  冓  虎  敖  豚  "
        "堯  隋  犀  解  濾  萬  激  辺  辺  羅  屯  甘  丘  召  呈  谷  "
        "戸巴孚  咢  芻  回  單  米舛酉丁酉尢酉殳酉甘酉禾酉名酉育酉呈酉昔"
        "酉卒酉林酉皿酉医酉皿酉彡酉豕酉豊酉熏酉襄酉  禾由禾幸厘  金刀金八"
        "金八金力金刃金叉金也金勺金斤金少金尺金丑金反金戉金甘金巨金玄金句"
        "金它金夷金田金包金石釘行金朱金全金舌金卯金夾金秀金肖金芒金亞金宛"
        "金京金固金畄金戔金爭金又金芳金花金武金又金皇金咸金兪金甚金益金高"
        "金眞金容金送金鹿金堅金斬金宿金將金族金曼金彡金連金婁金惡金早金敦"
        "金堯金番金亰金哀金雋金戈金戈金當金允金  金壽金廣金樂金慮金鼠金盧"
        "金戸金龠金隹金聶金賛金賛金羅金糸金隻金  門一門下門山門文門水門甲"
        "門市門玉門圭門共門呂門於門臼門奄門或門活門舌門臭門者門柬門  門盍"
        "門馬門  門單門達門辟千  厄  元  止  皮  百  有  丙  臼  夾  升土"
        "夾  歩  寿  垂  取  皇  益  員  鬼  僉  遂  隠  馬  糸  龍  ヨ水"
        "隶  Ｅ臣且隹隹  矢隹幺隹ネ集卒隹雨隹周隹雨包雨肖雨延雨沛雨兒雨妾"
        "雨沾雨非雨林雨英雨留雨淫雨散雨辟雨斉雨貍雨謁雲逮雨巫雨歴雲愛青爭"
        "告非面包面見面厭革力革叉革刃革内革央革旦革皮革末革丙革圭革凡革上"
        "革沓革匈革秋革柔革荅革用革達革遷革蔑ユヰ韋臼非  斉韭斉韭音見音召"
        "音勺亢頁公頁巛頁臣頁吉頁含頁禿頁果頁彦頁思頁亶頁日頁頻卑盧頁隹頁"
        "聶頁下風立風風台風具票風風票犬風食屯食夭食交食向食妥食甫食余食臼"
        "食芳食戔食炎食并食胡食歹食鬼食留食皿食王食曼食壹食貴食幾食堯食巽"
        "食號九首首或香夂馬又冫馬馬文馬四馬史馬它馬台馬奴馬亥馬交馬各馬州"
        "馬旱馬浸馬由馬其馬隹馬并馬扁馬塞馬蚤馬區馬参馬墓馬票馬累馬喬馬堯"
        "馬幸馬僉馬取馬盧馬冀馬襄馬隹馬∴馬麗骨干骨殳骨各骨卑骨婁骨蜀骨左"
        "骨豊高品長彡髟也髟方髟毛髟冉髟召髟夂髟弗髟包髟曲髟吉髟松髟曼髟須"
        "髟哀髟賓髟鼠||++鬥市鬥共鬥兒鬥寸鬥  凶ヒロ冂白鬼鬼犬委鬼鬼罔鬼両"
        "鬼  鬼厭魚方魚乍魚平魚包魚石魚冬魚安魚危魚旨魚休魚系魚沙魚肖魚甫"
        "魚利魚希魚甬魚易魚兒魚昆魚畄魚非魚参魚於魚虎魚念魚又魚咸魚皇魚思"
        "魚酋魚春魚世魚夂魚柬魚威魚皿魚皿魚  魚師魚留魚神魚康魚敖魚章魚票"
        "魚喜魚曾魚豊魚養魚盧鳥几鳥几入鳥牙鳥厂鳥厂鳥鳥夬尢鳥穴鳥央鳥鳥火"
        "古鳥氏鳥至鳥鳥它令鳥交鳥合鳥牟鳥行鳥耳鳥我鳥我鳥角鳥月鳥巫鳥貝鳥"
        "昔鳥享鳥東鳥東鳥卑鳥夜鳥咢鳥軍鳥鳥  羽鳥鳥  盆鳥骨鳥弱鳥脊鳥眞鳥"
        "真鳥缶鳥執鳥庶鳥矛鳥焦鳥番鳥亰鳥鳥  嬰鳥隹鳥糸鳥占図鹵咸鹵監鹿  "
        "鹿主鹿米鹿呉鹿其鹿菌    鹿射來夂麥夫麦夫麥  麥包麻非黄  禾勿黎占"
        "黎禽黒今黒出黒占黒幼黒吉黒京黒尚黒音黒微黒厭黒賣巾  黹  黹甫韭  "
        "敖黽敝黽豆皮鼓冬畄用鼠由鼻干斉  歯  齒ヒ齒句齒且齒召齒令齒艮齒契"
        "齒吾齒足齒屋齒禹齒咢合龍亀  合侖尭  眞  タ缶タ缶稟  臣巳";

/*
 * 漢字交じり文の文字を消す
 */
static int
kanji2index(
    unsigned char c1,
    unsigned char c2)
{
    if (IC == SJIS) {
        /* SJIS */
        if (c1 >= 0xe0) c1 -= 0x40;
        c1 -= 0x81;
        if (c2 >= 0x80) c2--;
        c2 -= 0x40;
        return ((int)c1 * (0xfc - 0x40 + 1 - 1)) + (int)c2;
    } else {
        /* EUC */
        return (((int)c1 & 0x7f) - 0x21) * (0x7e - 0x21 + 1) +
               (((int)c2 & 0x7f) - 0x21);
    }
}

int
jrubout(
    char *engr,
    int nxt,
    int use_rubout,
    int select_rnd)
{
    int j;
    unsigned char *s;
    const unsigned char *p;

    if (IC == UTF8) {
        int offset = offset_in_kanji(engr, nxt);
        /* 非漢字の場合 */
        if (offset == 0 && !is_kanji((unsigned char) engr[nxt])) {
            return 0;
        }
        /*JP:TODO:漢字はそのまま通す*/
        return 1;
    }

    if(is_kanji2(engr, nxt)){
        return 1;
      }

    s = (unsigned char *)&engr[nxt];
    if (*s == ' ') return 1;

    if(!is_kanji1(engr, nxt)) return 0;

    j = kanji2index(engr[nxt], engr[nxt + 1]);
    if (j >= 0x0000 && j <= 0x02B1)
        p = (unsigned char *)&ro0[j << 2];
    else if (j >= 0x0582 && j <= 0x1116)
        p = (unsigned char *)&ro1[(j - 0x0582) << 2];
    else if (j >= 0x1142 && j <= 0x1E7F)
        p = (unsigned char *)&ro2[(j - 0x1142) << 2];
    else p = (unsigned char *)"？？";
    
    if (p[2] != ' ' || p[3] != ' ') p += select_rnd * 2;
    engr[nxt] = *p++;
    engr[nxt + 1] = *p;

    return 1;
}

#ifndef JAPANESETEST
static struct trans_verb trans_verb_list[] = {
    {"adjust",  "どれ", "を", "調整する"},
    {"call",    "どれ", "を", "呼ぶ"},
    {"charge",  "どれ", "を", "充填する"},
    {"destroy", "どれ", "を", "壊す"},
    {"dip into", "どれ", "に", "浸す"},
    {"dip",     "どれ", "を", "浸す"},
    {"drink",   "どれ", "を", "飲む"},
    {"drop",    "どれ", "を", "置く"},
    {"eat",     "どれ", "を", "食べる"},
    {"grease",  "どれ", "に", "塗る"},
    {"identify", "どれ", "を", "識別する"},
    {"invoke",  "ど", "の", "魔力を使う"},
    {"name",    "どれ", "を", "名づける"},
    {"open"  ,  "どれ", "を", "開ける"},
    {"put on",  "どれ", "を", "身につける"},
    {"read",    "どれ", "を", "読む"},
    {"ready",   "どれ", "を", "装填する"},
    {"remove",  "どれ", "を", "はずす"},
    {"rub",     "どれ", "を", "こする"},
    {"rub on the stone", "どれ", "を", "石でこする"},
    {"rub the royal jelly on", "どれ", "に", "ロイヤルゼリーを塗る"},
    {"sacrifice", "どれ", "を", "捧げる"},
    {"split",   "どれ", "を", "分割する"},
    {"stash",   "どれ", "を", "入れる"},
    {"take off", "ど", "の", "装備を解く"},
    {"throw",   "どれ", "を", "投げる"},
    {"tin",     "どれ", "を", "缶詰めにする"},
    {"tip",     "どれ", "を", "ひっくり返す"},
    {"untrap with", "どれ", "を", "使って解除する"},
    {"use or apply", "どれ", "を", "使う"},
    {"wear",    "どれ", "を", "身につける"},
    {"write on", "どれ", "に", "書く"},
    {"write with", "どれ", "を", "使って書く"},
    {"zap",     "どれ", "を", "振りかざす"},
    {(void *)0, (void *)0, (void *)0, (void *)0},
};

static struct trans_verb wield = 
    {"wield",   "どれ", "を", (void *)0}
;

static struct trans_verb dummyverb = 
    {(void *)0, "どれ", "を", (void *)0}
;

struct trans_verb
*trans_verb(const char *en)
{
    struct trans_verb *list = trans_verb_list;

    if(!strcmp(en, wield.en)){
        wield.jp = body_part(HANDED);
        return &wield;
    }
    

    while(list->en){
        if(!strcmp(en, list->en)){
            return list;
        }
        list++;
    }

    dummyverb.en = en;
    dummyverb.jp = en;

    return &dummyverb;
}
#endif
