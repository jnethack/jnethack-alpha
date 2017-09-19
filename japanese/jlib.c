/*
**
**	$Id: jlib.c,v 1.14 2008-05-03 15:40:59 argrath Exp $
**
*/

/* Copyright (c) Issei Numata 1994-2000 */
/* JNetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <ctype.h>
#include "hack.h"

int xputc(CHAR_P);
int xputc2(int, int);


#define EUC	0
#define SJIS	1

/* internal kcode */
/* IC=0 EUC */
/* IC=1 SJIS */
#define IC ((unsigned char)("Š¿"[0])==0x8a)

/* default input kcode */
#ifndef INPUT_KCODE
# ifdef MSDOS
#  define INPUT_KCODE SJIS
# else
#  define INPUT_KCODE EUC
# endif
#endif

/* default output kcode */
#ifndef OUTPUT_KCODE
# ifdef MSDOS
#  define OUTPUT_KCODE SJIS
# else
#  define OUTPUT_KCODE EUC
# endif
#endif

#ifdef WIN32_PLATFORM_PSPC 
#define putchar(x) {}
#endif

static int	output_kcode = OUTPUT_KCODE;
static int	input_kcode = INPUT_KCODE;

/*
**	Kanji code library....
*/

/*
 * ˆø”‚ªŠ¿š(‚Ì1•¶š–Ú)‚©‚Ç‚¤‚©‚ğ’²‚×‚é
 */
int
is_kanji(c)
     unsigned c;
{
    if(IC == EUC)
      return (c & 0x80);
    else
      return ((unsigned int)c>=0x81 && (unsigned int)c<=0x9f)
	|| ((unsigned int)c>=0xe0 && (unsigned int)c<=0xfc);
}

void
setkcode(c)
     int c;
{
    if(c == 'E' || c == 'e' )
      output_kcode = EUC;
    else if(c == 'S' || c == 's')
      output_kcode = SJIS;
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
}
/*
**	EUC->SJIS
*/

unsigned char *
e2sj(s)
     unsigned char *s;
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
**	SJIS->EUC
*/
unsigned char *
sj2e(s)
     unsigned char *s;
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
**	translate string to internal kcode
*/
const char *
str2ic(s)
     const char *s;
{
    static unsigned char buf[1024];
    const unsigned char *up;
    unsigned char *p, *pp;

    if(!s)
      return s;

    buf[0] = '\0';

    if( IC==input_kcode ){
	strcpy((char *)buf, s);
	return (char *)buf;
    }

    p = buf;
    if( IC==EUC && input_kcode == SJIS ){
	while(*s){
	    up = s;
	    if(is_kanji(*up)){
		pp = sj2e((unsigned char *)s);
		*(p++) = pp[0];
		*(p++) = pp[1];
		s += 2;
	    }
	    else
	      *(p++) = (unsigned char)*(s++);
	}
    }
    else{
	strcpy((char *)buf, s);
	return (char *)buf;
    }

    *(p++) = '\0';
    return (char *)buf;
}

#ifdef SJIS_FILESYSTEM
/*
**	translate string to output kcode
*/
const char *
ic2str(s)
     const char *s;
{
    static unsigned char buf[1024];
    const unsigned char *up;
    unsigned char *p, *pp;

    if(!s)
      return s;

    buf[0] = '\0';

    p = buf;
    if( IC==EUC && output_kcode == SJIS ){
	while(*s){
	    up = s;
	    if( *up & 0x80 ){
		pp = e2sj((unsigned char *)s);
		*(p++) = pp[0];
		*(p++) = pp[1];
		s += 2;
	    }
	    else
	      *(p++) = (unsigned char)*(s++);
	}
    }
    else{
	strcpy((char *)buf, s);
	return (char *)buf;
    }

    *(p++) = '\0';
    return (char *)buf;
}
#endif /* MSDOS */

/*
**	primitive function
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
tty_cputc2(unsigned int c, unsigned int c2)
{
#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc2(c, c2);
#else
    putchar(c);
    putchar(c2);
#endif
} 

/* print out 1 byte character to tty (IC->output_kcode) */
static void
tty_jputc(unsigned int c)
{
#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc(c);
#else
    putchar(c);
#endif
}

/* print out 2 bytes character to tty (IC->output_kcode) */
static void
tty_jputc2(unsigned int c, unsigned int c2)
{
#if defined(NO_TERMS) && (defined(MSDOS) || defined(WIN32CON))
    xputc2(c, c2);
#else
    putchar(c);
    putchar(c2);
#endif
}

/*
 *  2ƒoƒCƒg•¶š‚ğƒoƒbƒtƒ@ƒŠƒ“ƒO‚µ‚È‚ª‚ço—Í‚·‚é
 *  Š¿šƒR[ƒh•ÏŠ·‚às‚¤
 */
int
jbuffer(
     unsigned int c,
     unsigned int *buf,
     void (*f1)(unsigned int),
     void (*f2)(unsigned int, unsigned int))
{
    static unsigned int ibuf[2];
    unsigned int c1, c2;
    unsigned char uc[2];
    unsigned char *p;

    if(!buf) buf = ibuf;
    if(!f1) f1 = tty_jputc;
    if(!f2) f2 = tty_jputc2;

    c = c & 0xff;

    if(!(buf[0]) && (is_kanji(c))){
	buf[1] = c;
	++buf[0];
	return 0;
    }
    else if(buf[0]){
	c1 = buf[1];
	c2 = c;

	if(IC == output_kcode)
	  ;
	else if(IC == EUC){
	    switch(output_kcode){
	      case SJIS:
		uc[0] = c1;
		uc[1] = c2;
		p = e2sj(uc);
		c1 = p[0];
		c2 = p[1];
		break;
	      default:
		impossible("Unknown kcode!");
		break;
	    }
	}
	else if(IC == SJIS){
	    uc[0] = c1;
	    uc[1] = c2;
	    p = sj2e(uc);
	    switch(output_kcode){
	      case EUC:
		break;
	      default:
		impossible("Unknown kcode!");
		break;
	    }
	}
	f2(c1, c2);
	buf[0] = 0;
	return 2;
    }
    else if(c){
	f1(c);
	return 1;
    }
    return -1;
}

/*
 *  2ƒoƒCƒg•¶š‚ğƒoƒbƒtƒ@ƒŠƒ“ƒO‚µ‚È‚ª‚ço—Í‚·‚é
 *  Š¿šƒR[ƒh•ÏŠ·‚Ís‚í‚È‚¢
 */
int
cbuffer(
     unsigned int c,
     unsigned int *buf,
     void (*f1)(unsigned int),
     void (*f2)(unsigned int, unsigned int))
{
    static unsigned int ibuf[2];

    if(!buf) buf = ibuf;
    if(!f1) f1 = tty_cputc;
    if(!f2) f2 = tty_cputc2;

    if(!(buf[0]) && is_kanji(c)){
	buf[1] = c;
	++buf[0];
	return 0;
    }
    else if(buf[0]){
	f2(buf[1], c);
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
    static unsigned int buf[2];
    jbuffer((unsigned int)(c & 0xff), buf, NULL, NULL);
}
void
cputchar(int c)
{
    static unsigned int buf[2];
    cbuffer((unsigned int)(c & 0xff), buf, NULL, NULL);
}

void
jputs(s)
     const char *s;
{
    while(*s)
      jputchar((unsigned char)*s++);
    jputchar('\n');
}

int
is_kanji2(s, pos)
     const char *s;
     int pos;
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
    if(pos<0)
      return 1;
    else
      return 0;
}

int
is_kanji1(s, pos)
     const char *s;
     int pos;
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
** 8ƒrƒbƒgƒXƒ‹[‚Èisspace()
*/
int
isspace_8(c)
     int c;
{
    unsigned int *up;

    up = (unsigned int *)&c;
    return *up<0x80 ? isspace(*up) : 0;
}
/*
** split string(str) including japanese before pos and return to
** str1, str2.
*/
void
split_japanese( str, str1, str2, pos )
     char *str;
     char *str1;
     char *str2;
     int pos;
{
    int len, i, j, k, mlen;
    char *pstr;
    char *pnstr;
    int	sq_brac;

retry:
    len = strlen((char *)str);

    if( len < pos ){
	strcpy(str1,str);
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
	    if(!strncmp(str+i-j,"@",2)){
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
	else if(!strncmp(str+i-j, "n", 2) ||
		!strncmp(str+i-j, "j", 2) ||
		!strncmp(str+i-j, "p", 2)){
	    pos -= 2;
	    goto retry;
	}
	else if(!strncmp(str+i-j,"H",2) ||
		!strncmp(str+i-j,"A",2) ||
		!strncmp(str+i-j,"B",2) ||
		!strncmp(str+i-j,"C",2) ||
		!strncmp(str+i-j,"D",2)){
	    pos -= 2;
	    goto retry;
	}
    }

    if(i == j){
	/* •ªŠ„“_‚ª‚È‚©‚Á‚½ */
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
}

void 
jrndm_replace(c)
     char *c;
{
    unsigned char cc[3];

    if(IC==SJIS)
      memcpy(cc, (char *)sj2e(c), 2);
    else
      memcpy(cc, c, 2);

    cc[0] &= 0x7f;
    cc[1] &= 0x7f;

    switch(cc[0]){
      case 0x21:
	cc[1] = rn2(94) + 0x21;
	break;
      case 0x23:
	if(cc[1] <= 0x39) /* ‚O`‚X */
	  cc[1] = rn2(10) + 0x30;
	else if(cc[1] <= 0x5A) /* ‚``‚y */
	  cc[1] = rn2(26) + 0x41;
	else if(cc[2] <= 0x7A) /* ‚`‚š */
	  cc[1] = rn2(26) + 0x61;
	break;
      case 0x24:
      case 0x25:
	cc[1] = rn2(83) + 0x21; /* ‚ `‚ñ or ƒA`ƒ“ */
	break;
      case 0x26:
	if(cc[1] <= 0x30)
	  cc[1] = rn2(24) + 0x21; /* ƒŸ`ƒ¶ ƒMƒŠƒVƒƒ•¶š */
	else
	  cc[1] = rn2(24) + 0x41; /* ƒ¿`ƒÖ ƒMƒŠƒVƒƒ•¶š */
	break;
      case 0x27:
	if(cc[1] <= 0x40)
	  cc[1] = rn2(33) + 0x21; /* „@`„` ƒƒVƒA•¶š */
	else
	  cc[1] = rn2(33) + 0x51; /* „p`„‘ ƒƒVƒA•¶š */
	break;
      case 0x4f:
	cc[1] = rn2(51) + 0x21; /* ˜@` ˜r */
	break;
      case 0x74:
	cc[1] = rn2(4) + 0x21; /* êŸ ê  ê¡ ê¢ ‚Ì4•¶š*/
	break;
      default:
	if(cc[0] >= 0x30 && cc[1] <= 0x74)
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

/*
 * "put off"‚ğ‘ÎÛ‚É‚æ‚Á‚Ä“KØ‚É˜a–ó‚·‚é
 */
const char *
joffmsg(otmp, joshi)
register struct obj *otmp;
const char **joshi;
{
    static char buf[BUFSZ];

    *joshi = "‚ğ";

    if(otmp->oclass == RING_CLASS){
	Sprintf(buf, "%s‚©‚ç‚Í‚¸‚·", body_part(FINGER));
	return buf;
    }
    if( otmp->oclass == AMULET_CLASS){
	return "‚Í‚¸‚·";
    }
    else if(is_helmet(otmp))
      return "æ‚é";
    else if(is_gloves(otmp))
      return "‚Í‚¸‚·";
    else if(otmp->oclass == WEAPON_CLASS||is_shield(otmp)){
	*joshi = "‚Ì";
	return "‘•”õ‚ğ‰ğ‚­";
    }
    else if(is_suit(otmp))
      return "’E‚®";
    else
      return "‚Í‚¸‚·";
}

/*
 * "put on"‚ğ‘ÎÛ‚É‚æ‚Á‚Ä“KØ‚É˜a–ó‚·‚é
 */
const char *
jonmsg(otmp, joshi)
register struct obj *otmp;
const char **joshi;
{
    static char buf[BUFSZ];

    *joshi = "‚ğ";

    if(otmp->oclass == RING_CLASS){
	Sprintf(buf, "%s‚É‚Í‚ß‚é", body_part(FINGER));
	return buf;
    }
    else if(otmp->oclass == AMULET_CLASS)
      return "g‚É‚Â‚¯‚é";
    else if(is_gloves(otmp))
      return "g‚É‚Â‚¯‚é";
    else if(is_shield(otmp)){
	*joshi = "‚Å";
	return "g‚ğç‚é";
    }
    else if(is_helmet(otmp))
      return "‚©‚Ô‚é";
    else if(otmp->oclass == WEAPON_CLASS){
	Sprintf(buf, "%s‚É‚·‚é", body_part(HAND));
	return buf;
    }
    else if(is_boots(otmp))
      return "—š‚­";
    else if(is_suit(otmp))
      return "’…‚é";
    else
      return "g‚É‚Â‚¯‚é";
}

/*
 * ”Œ‚ğ•Ô‚·
 */
const char *
numeral(obj)
register struct obj *obj;
{
    switch(obj->oclass){
      case WEAPON_CLASS:
      case WAND_CLASS:
	if(obj->otyp==SHURIKEN)
	  return "–‡";
	else
	  return "–{";
      case ARMOR_CLASS:
	return "’…";
      case GEM_CLASS:
      case ROCK_CLASS:
      case BALL_CLASS:
	return "ŒÂ";
      case SCROLL_CLASS:
	return "–‡";
      case SPBOOK_CLASS:
	return "û";
      case POTION_CLASS:
      case RING_CLASS:
      case AMULET_CLASS:
      case FOOD_CLASS:
	if(obj->quan < 10L)
	  return "‚Â";
	else
	  return "";
      case COIN_CLASS:
	return "–‡";
      default:
	switch(obj->otyp){
	  case CREDIT_CARD:
	  case TOWEL:
	  case BLINDFOLD:
	    return "–‡";
	  case SKELETON_KEY:
	  case TALLOW_CANDLE:
	  case WAX_CANDLE:
	  case PICK_AXE:
	  case UNICORN_HORN:
	  case LEASH:
	  case STETHOSCOPE:
	  case MAGIC_MARKER:
	    return "–{";
	  case CRYSTAL_BALL:
	    return "ŒÂ";
	    break;
	  default:
	    if(obj->quan < 10L)
	      return "‚Â";
	    else
	      return "";
	}
    }
}

/*-------------------------------------------------------------------------
	‘SŠp•¶š‚Ì‚©‚·‚êƒpƒ^[ƒ“
  -------------------------------------------------------------------------*/
static const char *ro0 =
	"                        E  ECD  D‚Œ                E      "
	"            RJ    TJ    OƒGƒ}  ^ƒ‚b()|]|]            "
	"]  b      dEE          e  f   (  )            [  ]    {  "
	"}           q  r          u  v  imjn|‚Œ    {|^_|F"
	"|  ^<    >  ƒ„`äEdB  ›{        Œ  K‚b‚x‚rb"
	"‚bb    K^‚//    ~  ‚b‚        ™  ()‚b›  ›  <>    ƒR™"
	"   ÈÚ¢  ÉP¤  ~F„¦|->|<-bÈbÉ                  "
	"                            ¼ß½ß¼½(    )  ‚µ‚i||Ü    "
	"                            O/  /‚–|  ¨<>É¤ƒRß        "
	"                                    ^QbQ    ‚  ¤      "
	"<<ƒ>>„/   `‚ƒ`‚ƒdE    ç                              K‚`"
	"K^||b‚ÁI|)‚s{õa                  ()‚b                "
	"                                            ()‚bLbÜÚÜ½Ú{"
	"u½Ü‚P /‚‚R‚¹                            ¢È‚d‚R( Ü‚k )"
	"‚eßu„¥‚b  ||„§‚Œ  ‚Œ  |<  |_  ‚m‚u||‚u‚b()‚e½‚nƒ‚o  ‚ƒÜPb"
	"‚i‚µ /_VV‚m^_ÉbPÚ                        ‚  ‚Œ‚    ‚ƒ‚Œ"
	"‚ƒ  ‚Œ|‚  b‚E‚ŒE‚Œ‚Œ|<    ‚||‚’||‚ƒ  ‚Œ‚‚ƒ‚Œ‚Œ  R  {|"
	"‚µ||A  ‚–vv^><‚–ƒm|Ú                    ‚ß‚¨ƒÇR‚µRM‚ÁM‚Â"
	"]  ‚ñƒ‰‚å  ‚Ì‚æƒJR‚©  ƒL‚³‚«  <   ‚­  |+õ‚¯  ƒj[‚±  {‚ƒ‚³  "
	"l   ‚µ  ƒi‚X‚·  µt+‚¹  ‚Ä‚y‚»  ƒi‚±‚½  ‚T‚Â‚¿      ½[‚Â  [ ("
	"‚Ä  ¼  ‚Æ  ƒi  b‚±‚ß‚Ì‚ê‚íJ)ƒm‚æ‚¯‚Í  ‚Í  ‚Ä‰³‚Ğ  ‚Ğ  ƒn¬‚Ó  "
	"‚Ó  ^_‚Ö  ‚Ö  ‚Í‚Ü‚Ù  ‚Ù  ƒL‚æ7+İ‚·‚µ‚Ì()‚µƒL‚Á{‚Â{‚Á  ‚è  "
	"B„¥‚ã‚¤‚Á‚¢ƒm‚ë  ƒI‚í‚V‚Á‚ÁƒHƒI‚Â‚Ì‚é‚é‚ñ‚Æƒi/v`            "
	"                                C|ƒtƒmƒm‚Œƒm‚Œƒ  ƒ›~„¦Û‚sÛ"
	"{  „©  ƒi“ƒJ  „©ƒL  ƒmƒtƒN  ƒmƒNƒP  ƒj ]ƒR  ƒi++ƒT  Jƒ“ƒV  "
	"ƒtlƒX  ƒ„µƒZ  Rƒmƒ\  ƒNƒƒ^  ƒiçƒ`  JƒmJƒmƒc  §ƒe  bR"
	"ƒg  {ƒm|Qƒtƒ˜³•s    ƒmRƒn  ƒn  „¥‚kƒq  ƒq  Pƒmƒt  ƒt  ^_"
	"ƒw  ƒw  „©¬ƒz  ƒz  RƒtSßÚRƒmRŠ±µ{  „©  „¢ÛƒRÛƒRßƒRß"
	"ƒt‚ŒƒmƒmƒŒ‚ŒƒmƒR™„¡ƒtuƒtƒLƒGƒAƒGƒ‰ƒtRƒmƒE  ƒi„¢ƒNƒm        "
	"                        ¢È‚d‚RbPÚÈ‚e‚k‚VÚ||„§‚n|b  |<  "
	"/ _‚u‚m||Èƒjß‚b()ƒ¡„¢‚e‚c>_ÚPbÉb‘‚^_b{Ü          "
	"                            |3B                                 "
	"                                                                "
	"                                                                "
	"                                                                "
	"                ¢È„C½‚d‚RbP„L  ‚e‚k„E  „K  Ü‚Á||‚u„I  |<  "
	"J|„P‚m‚u||„§‚b()ƒ¡„¢‚e½( ÜPb‚–ƒm‘„O^_‚kÛ‚µb||Û„W„Y„]  "
	"„]||b½-)  |O„¥¼„§                                            "
	"                ‚  ( ‚„]„s|      ‚ƒ|„u  „{                  "
	"            „ƒ  „s                                  Û  „Š  „  "
	"„                                                              "
	"        |  ‚Œ                  „   „Ÿ  „   „Ÿ  „Ÿ„ „Ÿ  „   „¡  "
	"„¢  „£  „¤  „¥  „¦  „§  „¨  „©  „¥  „¦  „§  „¨  „©  „¥  „¦  „§  "
	"„¨  „©  ";

static const char *ro1 =
	"¼˜±ˆŸ  —Œ\B ‰Â˜³ˆßóSáá  —‡    á¡  ¼  ‹T  œ¦ˆŸ‰®  ‰®  ‹ã“ú"
	"èè  ŒË  ‹›Q–Øh™Ê“y‘ì“l‹y  ›~™Ç—Š’–SH‘ä…{…  ‹›èœ÷  ŒI•Ä"
	"ƒl‡›~—›ö‰‚ˆÀ  “ú‰¹ˆÀ  –å‰¹ŠvˆÀ–ØŒûl  ›š  ƒC—§ˆß  èè  š˜ˆä‹|‘å"
	"‰Ñ—•è  ¦¡è°  ‰¹  ˆÑ  “ú–ÜŠï      “c  “c‹¤‘½  …è°…èè“cŒˆÏ  "
	"˜³  ˆİ  èè  ‹M  ™·–î# ”˜³  ˆ½  Œ  —L  Šô  |  mƒq‰v  –Æ  ‰Ñ‹Œ"
	"Ÿ  ˜°  ‹›ãƒ€™X‚d™Åˆö  ŠL  š˜‘å—ˆö‹|bHŒ‡p  ™X  ‰A  Š®  ¡‰]"
	"‹}  ‰¹ˆõŒû¡Î  ›~˜°H  ™qÊ˜°  H  ™Å  ’í’¹‹K  ŒÜ  Îè°‚dÎšD  "
	"‹•  ŒûŠLİ  ˆÑ  ‹›™Ö—˜V™ÊŠù•á  ’Üƒ€–å‰¤‘¸  “ñƒ€ŒR  ‰J‰]”C  H¨"
	"–”  ‹{˜CŠL—Œiœc“ú‰›—R  ‘v  …  ‰i  ‰g  ‰p  ”TMƒq•Åƒq•Å‰›  sèè"
	"Œ¾‰i‹à™[–é  ›öŸt”ªM”nÚ¬ŒZŒ¾ˆÕ‘–œø–åŒZ‰Ä  ™Ê  ™c  š˜åÍ™·—‘å\"
	"›~—œM³™ÇS‰‚  à§  ”ªŒû“Ğ  ‰Î  ‰Î‹Œ‰Î¼–k“ùåÍ  …æ³–LF™Ç  ‰€  "
	"åÍ  ‹à”ª™Ç’¹M  •û  ƒV‚¿¶’j™  •vší•Ä‘åœeå›öSb  “ú‰¤‰©  ‹æŒ‡"
	"‹æŸtH“yŒö‰H‰œ  ’¹  ‹æ’¹‹¤—R™c  ’†  ‰Î  ˆÓ  ›™ŠˆÓ  ˆÓ  áT  ‹“y"
	"‚Ä‚y‰‚  ³™ÅˆöS“úM‹}  —§“ú‚sƒgƒCƒq”½  ‰Â  ‰Á˜¼¼  Œ\d—ÍŒûŒûÊ"
	"‹g‰Á•Sšç—‰Ææ³  •S•ª‰Ñ“lƒR–”“c–Ø‰Á–ØšFŒ‡‰Â’ól  ‰¤‰ÂšD  –Øç‰Ñ‰Æ"
	"ŒÅ  ‰»  ‰Â  ‰Á  ‰½  ‘  ‰Ê  ’–”Œ¾‰Ê‰Ø  ‰»ŠL‰Á  šD  ‰J  ’•¶‰ä”°"
	"‰ä  èœ÷ƒI  ™—Rbl‰åˆğ’‰ä‰ÁŠL‰åè°H‰ä‰Á”n‚Ö/|‚Ö‰]Šp  Œû  ‹S  "
	"ˆß  œM‰ñ¬ší–”“y–ˆ  ŠD  ˆß  œOœ÷Œû“ŒÈ¼‹S“l–ˆ  Z‰ú–ˆ  ™Ê‰Î“c‰î"
	"”ä”’…‰ï‰î  ‰ğ  –å  ŠF  –Ú  æ¯™{ˆå—Í—[ƒgˆå  ‹g  ˆ³Šİ¦ŠùªŠùˆ³  "
	"Î¡Œ|MsŒ\Œ¾ˆå‹àæ¯œˆå—¢  º’Œ\“y˜j–Øs’–œ‹àŠbá`  Šq  –¼šç"
	"›öŠsL  Šo  Še•Aˆå  ”„ŸtÇ  Îè°Ç  Œ©  —p  Ô  ÔŒğ‹  –åŠeé­  "
	"“ù’†š  ‹uR”’–Ø‹q•Å™ö•ÅŒT  —§  Œ˜  “c˜j”ö  ‹›H‰P™¯ŠQ  ˆÕ  ¬‡"
	"ã  ã  ˆÕ  œ  ˆÕ  ˆÕ  ÔŠQ–Ú  ‹›Œ˜Œû\‰Ô  ‰Ø  Šv•ï–Øé”’™X‹T  "
	"‰Y  •ƒ‹à‹àŒ“•  b’¹–Ø•S–µ  é  ‹|•ÄƒƒŠŠ   H  ‘ìŒîƒCŒû™kŒ³‹¤  "
	"Š±ƒŠr  è°—ÍŒÈ  šô  r  —ˆŒ³  ›~ŒûŒ©  ‚s\‘ìŠ±‹ø  ™÷  ŠÑ  Š´  "
	"šô  æ  ŠÃ  ˜j  Š¯  ¦Œ‡è°Œ‡Š±  •v  ŠÔ  y  ˆ£  “ù  bM–Ú  Š±  "
	"Š¯  ŠÔ  à§  ‹R‘ì‰HŒŠ±MŠÄŠ®  è°Œ©Œ¾“Œ“cŠLˆ£  ŠÄ  –å  –å  –å  "
	"‹Œ  ‘ìèèHŠ¯äqŠ¯‹ã  ¡Œû™ÊŠ±Œµ  ‰¤Œ³›ö•i–Ú¯RÎKŒ³Šå  ™Êè°Œ³•Å"
	"•F•ÅŒ´•Å~  x  –ï  ‹g  Œû‘å‘´  ‘å‰ÂŠì  Šï  Rxƒ•z›ôœùŒÈSŒR  "
	"–Ø™{•û‘´¯Ú‘´Œ–Ø‘´–Ø  Šô  ›â  æ³ŸtƒŸƒƒVŸƒ›ôœ÷ƒl‹Ò‰Ñ—›Šó  …ŒÈ"
	"H  •vŒ©Œ¾ŒÈŠL  ‘–ŒÈÔ‹ãŒõŒRH™{”nŠïŒZƒ€‹ø\ˆ×  ‹`  —xŠ  ‹•œ÷"
	"x  ‹^  ‘´Œ‡‹`  ƒqƒ}ƒl—r‰ä‹`  Œ¾‹X‹`  ™¯  ™¯  Šv™¯mŒûŒî  Œ_  "
	"–Ø‹g–µ  Œ¾‹gÎè–ØŒß‰Ñ  ‹™ÅŠe  ‹p  åg‚d›£  ‹Ò  ƒNƒXƒC‹ãƒC–Ø”T  "
	"‹y  ˜C  ƒR™·ƒˆS‹¼–Ø‚T’š  ‹y  —§  ‹v‰Î‰¤‹ŒŠ‹ãg‹|‹y  ‹y  …\"
	"…‡b“úŒßƒL“yƒ€›™ŒÃ™·‚d‹  ˆ  è  ‹–Øåg  Œ¾Œß‘«‹‹à‹‹›  Œä¦"
	"ƒN“c˜³—¹‹œ  Œû¬‹¤  •v  ‹ª  ‹¥™Y—§ŒZ“ù  ™ƒ—Íˆ™·‰¤‘¦‰KŒû\“ÛŒü"
	"èí  •v  ‹|’‹|˜j‹  –}S‹¤¬•v  F¼‹ª  ŒZ  ‰¤  •v  ‹ª  ”]™±—Íˆ"
	"“¯  ‹ª  ‘¦  ‹àèí‹½‰¹‹½HŒh”nƒC™Å‹^  ™Á™Y“ú‹Ä–Ø›™‹å“c”˜²‹€‰¤  "
	"“¯  •Äçä¿  ä¿—Í“y™¯™c\‹à›å™Ê‚s‹ÒŒ‡‹àŒ‡‰¤¡—Ñ¦lƒ€˜]  b…‹Ò  "
	"äŸ  ƒl¡‹Ö  Œ¾ä¿‹Ò  ‘S‹ÊŒû¡‹à¯ƒi L‹ï  ™¯  ™·ƒ‹å  ‰¤‹v–î‹ŒÃ“Ï"
	"g‹æ”n‹æ”n‹u”n‹å–Ú’Uâ[  ågŒàH  ŒŠHâ[  â[  â[  â[  ’†ƒLß  ‹àì"
	"›™Ñ›™o‹ü  ŒŠ‹ü…“úŠv‰»…ÔŒŠŒ\”\  ˆØ  ‹v•Ä¼–Ø…•i–”–Ø‹àH“®  "
	"›š‰Ed  Œ¾ìŒN—rÔ  ŒN  Œ\ƒg‰Áˆßƒl  Œn  ‰» Š§  Œû™YŒË¼“y‰¤‰¤Œ\"
	"ŒY  Ø  œOœcœe–”v—R›ö  ‹}  ã©ˆÕ  è°”T‹å¼’U‹Œ\  •v  “cŒ\–Ş|"
	"…  …–”…•ÄÔ…ŒT  –”“yŒY  ’  Œ¾\Œ¾|ŒhŒ¾Ô–”–”•Å•v’¹‰]  ™Å  "
	"‹›‹ågæ³‘ìœ÷Ôè”’•û¬“ús  ‘C–ØlƒPší™rŒ_  ›~”ª…‹gM  Œ¾ší“ú  "
	"ƒC‹g  Šª  Œš  ƒˆ”“  ‰›  é  š˜Šªb“y—Œ“œMãäS  Œ§Œnè  Šª  "
	"‰›  è°  –´  ‘å  “ìŒ¢Î  ÎŒ©…Œ–Ú¦ŒËŒ–ÚŒZŒ¾Œ“bŠLÔŠ±’Ç  ‹àŒš"
	"‰›  “ú•Å”n‰›êb‰›™Yƒe™Ê”’™ÊŠ¸›ôƒt‹|Œº™÷  Œ´  ˜³›ô‰¤Œ©…ŒºMŒºOŒû"
	"Œ¾•F¯  ç•½ŒÅ  èmŒûŒÁš˜ŒÃ—ŒÃq‰ZƒR  ›öÔ‹|‰ZP›™ŒÃ¼–ØŒÃŸ˜ŒÓ"
	"‰Z  ”SŒÓšğ  ŒŸtŒÃŒŒÇ  åg™XŒ¾šğ‘«šğ‹àŒÃŒËè°ŒÙ•Å“¤x‰¤  œ_  ŒÜ  "
	"Š±ƒPƒ†ŒûŒÜŒû—Œà›ôšç‰µ]¬Œá–ØŒá‹×  ‰¤ŒÓ‘´ÎŒ¾ŒáŒ¾ŒàŒ¾Ç“ÑŒÓ‰³  "
	"‹›—¢˜Z•ƒŒğ  ƒ†–îƒ†–îK  ˜³™Yƒnƒ€H—ÍŒğ—Íƒ€™¯™Êq™ƒR™cŒû™ÊÎŒûŒò"
	"“y˜´“y@—qq‚µq“y›~ƒ€‚sÛƒG–œ‹¤ŒÈ“yh›öƒ€›ö›š›öè®‹|ƒ€¬˜j¬r"
	"˜´  ‹å  ‹ó  ƒG¼“úˆó“úŒõ—™  ˜´  Œğ  X  ™i  ƒVHƒV‹¤ƒVƒVJƒV™i"
	"“c’†’æÛÎX‰Ñ‚•ÄN…H…ƒ€…Œğ…‰ªãÎˆä“y‚T~ŒŒƒ€Œ‹ó‚ŒM˜´"
	"äŠìœe§s‹›Œ¾™iHŠLŠL™iŒğB “ÑF‹àLÎL‹à‰ª–å‡šçƒH•Å‰Ñ“ú˜³Œü"
	"]’¹‰ªl|‹—ÍŒû–œ‚ÖŒû‹  l  ƒV‹‰Æ  Ôˆ”™¯ŒÃŒZˆå  ‹‹g‹§‹Ê‰ÑE"
	"“Ñ–Â—¢  Œ¾Œ¢ƒV­Œ—v‘]Š¢–ÜSš  ŒšD”’  “ü  ~”äƒq•Åƒwƒ‰ˆö–Ø"
	"“y\æ¶¯—¨¬¯æ¶¯“ú“ú”ä–Ø¯–Ø¢©  ›ö¯…ŠÃ“ú‚q‰]‹SŸƒjƒC¶"
	"–”  ˆò  R·ƒiƒGã³¶–ØŠƒV­‰¤·Î­Œ¾“á‹àŠL¹ˆß“y˜¸›ö¿¿  Ó  "
	"è°  ™f  æ  ‹gœ÷ŠîŠ¦ƒˆ—h  ÑœcƒI’šÑ  –Øœ÷~ÊÄ  ›İ‰Î‰Ñƒc›™‹"
	"Î™¾ŸÎ—[¦Ä¦…“cÑ  ˆßœ÷Ôœ÷B ÕÄ  “y  –ØË”ñ  ŠLË‰å  ”½  "
	"”½  ŠE  _  ƒ—LŒû“VRŠïŠï  ÎŠï˜H’¹ƒC“áÑ  Œû“áó  “úî›£Œû  "
	"ŒŠ“áR  …  ‹àÌ–Ø—‹›Œ\¢  ¥ƒqM  ›™‹ĞÕ  ›İ—[Å  @  –Ø‚µ‰ÑŸt"
	"Y  ™¾è°”’‘‹›Â•Ê‰û‹àÂ‹›Œğ„Š||“ú¼“ñß˜¢  ƒ€œc™ÛQ  U  Ì¼"
	"–Øœ÷‰Î•Ä‰¤û•F¶–ÚœOZ…“V’^  •vŠL“ÑˆòH  Ô‹Òa“úŸeœ÷YmƒCq"
	"i  ƒC—™R  =]  ä’†ûi•CŒû\Û—‘ä—sŸ——¹ˆê›™€‹Ğ  ”’‹ĞmS"
	"“cS|  m–”q¼‘´‹Ò•û–çƒq“ú–Øxã  Ÿeƒq[+EEt  ƒl~‰Ñƒ€›ô¬…"
	"Ÿ…ŒxŒ|ƒ€pjŒ©Œ¾iŒ¾›Œ¾®Œ¾uŒ¾™üŸŠLŠLˆÕŸè°Hi~•Ä˜­ƒˆ"
	"ˆÈ  ›  ‹Œ™X›~q“y¡›ôS›  “ú›Œ‡  ›ô  –è‘ä‰J  ¢‹Ê›ö›Î §¬"
	"™c  ‚S“l”’–Ú  ãhƒV—[›ö”äHœTæ}Æ“c–Â’|“ñÔ—RŒŠ˜Z‰J‰º‚µ{Œûµ"
	"KŠÛ–î•v—¾Š  ÑSƒV“ú‹o  ›ö¸‹ÒŠL•v  •”  Š  ’ÏvŸ–Ø”V  ›  "
	"cˆ…‚‡  —^  g¡ÉEÔ¼—]“lÒ  ƒl“y…­“y‰E“¢Ë˜j‘ì  ’›€"
	"‰å  ƒCÌ™¯  ›™”ª–ØÙ‰ÎÙ¯¡“ÑÙ‰ÑÚ‹àˆÕ‰E  f  ‹|‰HáS‰¤  ¨–”"
	"¡  pƒL‹  Ÿeéç  ‰¤é‰ÑdŒd‘–æ“Ñ  ©•Sù  –”  ŒZ  O¡ó  "
	"–Ø‹j…ó‰J§š˜l–”  ‹g  ¦  ‹–Şìc¿œcHS‡  B  ‰Ñ”T‰Ñ‰Î…“~"
	"l  ‰H”’©‘å’O  ‹S  ŒŒ  —´  è°Œ¾‘«AÔ¨ü  “Ñ  “ÑB”¹–Ø“Ñ‹SƒC\"
	"dåˆò  ˆê{œe•Dœ÷  –µ–Ø™q\~><áVŒ¢…]ç—¢‹à[ƒz–”Ÿe  ˜Ï  f  "
	"ƒlŒZ…h•Ä  ›x  ›x  “yRsƒ{ƒ{  ˆò  Rˆò“ú•vw  —§ˆò‘C  ”nˆòè°  "
	"‚  “ú‹å‚  Ÿe{‹  y”¹‰[  @  …“Ô›İ  ‘¸  “Ñ‹ì•Åšç™{ƒl“ŒË‹Ò"
	"“úÒ  Ò  ›ö“ù…ÒÒ  ãä    ”  Œ¾ÒŠ—Í—]–”‚­ƒm›ö—\—]  ”@S"
	"‹à•—]  ä  Ü  Œ”  ™·‹Òƒ`œO“ŒûÑ  —§™g¹  ®|«‘å—§——¹Ñ  "
	"¡  ƒn˜«¬  Œü  ›ö“y›ö–Ø›öÆÍœc—¹…­  ¢  ®è‘–  “ú¡“ú  “ú¢"
	"¹•i–ØŒö–ØÑ–ØÍ–ØÅ¢  Ñ  •à  Ÿ”‘Š‰Î‹Äè°  º  ›ö³­–ÚÎÑÎÅ"
	"ƒl—r‰Ñ›•‰¹‘šî  •Ä¯…¢¬Œ¹  «  Å  sd®ˆßŒ¾ŒöŒ¾³Œ¾¢Œ¾—r"
	"æ³  ®ˆõ«“Ñ‹à³‹àd‹à“¶Í  ŠvÑ„¥Ûƒiƒ—¹…‰Ñ  ™k™{æ  ¬  ä  "
	"åõ  —åõ®’İÂ  —J  šç–Ø–Øä‘ˆ  Œ¢  “cŠ‰Ñåõå  Œ¾åõ“Ñåõ‹à’è‘®  "
	"’¼  H‹Ğ®  ’¼  Ÿe’¼‰Îå†…Æ¨ÆƒN”bŠp’—Ç  H’’C¡›™‹ã’‡\Œ¾  "
	"ƒˆ–”’CŒû—’CZ  ”Ô  ƒn‚¶^  ’C  h‹Ò“ú  —Ñˆ`  ƒˆ–”ç–Ø—Rb›öœc"
	"‹ï’åƒl\‰Ñ  …\‹  S  V  hŒ©Œ¾œc©ƒI—§è°  ‹à\‰J’Cƒm‚ÖƒC“ñ"
	"“  ›ö  çmƒˆ¡‘´•CÚ  bŒŒ¾  \  Ô  Švni  Œ¾æœc•Å“Ñ“áš˜ƒ"
	"™Ê¡“¤  ŒûŒ‡ç\‹Ğ  è°  ƒt˜«‰ÎŒ‡–Ú‚•Ä™¾‰H‘²ˆ£  æ³  “Ñ™¾‹àè°‹à‚"
	"—L  ‰¤§œ—LR@R‚•Ä¼–Ø‹æ‘–ääè°‹  –Øœc–Ø¹Š¯  ”ç•Å­è°‹  "
	"“o  K  ’š{‚¹“ù—Š  “c‹v“ú•DÈ  s  ŠÛ—Í—¶³  ¶  œú  ³¼‘©³"
	"“ú¶“úÂ–ØÈ–Ø¼~‰ºÂ  ‹¶‹‰¤¬M•ÄÂ¨‰¤m›™§ˆßl  Œ¾¬ÜŒ¾"
	"Œ¾ÂÜ  “Ñ¯‰¤ŒÂ‘ˆ•¶  ‰ÑŒZŒŠëè°–”›ö‹Ğ¬Ì¬ƒ{‹Ò  “ú  –Ø‹ÒŒû  "
	"Ó  ’|Ì…Ó•vŒ‰¤ŠL“y–’‘«–’‘«ÓÎ•Åµ“o  ¨  ¨><‹Ò  Œ¾ŸtŒŠØ"
	"’|‘¦Œ¾ŒZ‰Jƒˆ…FpŒÃ’’PƒCR‹™Y\ƒCãŒû˜j  —R¡¬‘åƒml|’Pœ÷ŒË‰H"
	"’F  –Ø‘S–Ø’O”’…œ÷  æ  ™¾  ‘Ö  ‘O  î  •û•DŒŠ‰å’|‘O…ò…œ÷—rŸ"
	"Œò—[ƒM”ª›ö  Œ¾‘SŠLœ÷‘«œ÷’F  Šª  ‹àœ÷‹àæ–ål‹›—rŒl|—r‹ga  "
	"ƒ^Œ¢˜¢‰¤ƒl’P…‘PŒ‘P•Ä—Ğ¥‘]ñ“yRŠÌ  ”ª“ú“c“ú—Ñ•DŠ  •D[•D‘©"
	"Î‘^ƒlŠ‰ÑŠ•ÄŠ“y……Š‹›‰ÑŒ¾ËŠ  ñ  ‰P  ‘]  ‘q  –”  ‹Ææ‡ŒË"
	"ˆß  m  ‰Ñ  ‘å  –Ø  ›™‘]™·‹Ğ•¨S‘ŠS—R–”›â  ç\”a  •i–Ø’UŠ±‹È“ú"
	"’P‰Ê‘q  ‘‚  ‘‚  ‰Î•iƒN›š›ö—R–Ø–ÚŒŠS•Ä‘‚…Œö…@¨Œö‘  ‘s  €œO"
	"‘q  •i–Ø‘sˆß“y•D“V  ‘‚  ‹à‘q‰J‘Š”n”aÛ  ‘]  ‘]  ‘   •è  ŠL‘]  "
	"‘«  ‘¥  ŠL  ¯™Å©S‘«  R‹‘¥  Œû•D‘©  ’J  ›™âZŠL‰ú•û–î…”„˜³˜¸"
	"ƒl—RŠÃŠ‘O  ƒiqqŒnU¡ˆõ  –Ø¡‘·  –ç  —[œc‘å  ‘¾  Œ¾›€‚  —L  "
	"—  ¶Œ’š  –Ø›€M›€–Ø¶›€  ”n‘¾”n’PƒC–{“yy•¶¡§¡‘ãR™À‹Ğ˜  "
	"‘äS”\ŒFˆÙœ÷•v“ú•v…‘Ñ  Œ‘ä‘Ş  ‘ä  ‘ãˆß‘ãŠL¯  è®  æ³  ‘ã•‹›ü"
	"ƒCœTƒ€Œûlƒi’|’í“Ñ¥¥•ÅŠå’¹—³  —´  è‘æ³  ›~ƒ‚ƒ‚  Ú  Î  Ú  "
	"ƒˆy‰¤æ³Œ¾ƒ‚‹ààVå†  Œ¾á¨  ‹Ğ  ’ÑŒûƒnŒû™Åm’UK  ™Ê  è°¡ŒŒZ"
	"ŒÈ‹¤‹–”R  –Ø•üƒwŒû—¢  ‹›á–Ø‘¸Œ¾è°œO  ‘áVŒû•v“y’U’R’Uç–Ø“ú  "
	"•vŒ‡‰Š  r  RŠD–î“¤—§§’|’P…’è¨  Œ’U•D’Œ¾‰„‹à’iš˜¡˜·  ‹|’P"
	"•Ä‹Ò“úà§˜·  ‚dŸt“c—ÍŒ¾‰Š’¼  –îŒû“y–ç‹|–ç¨S’m“ú–ç  ›ö’m‰Ñè°’¼  "
	"Š¼’’m›™—r”n–ç’}–ØŒº“cƒPTT’|–}’{  æ³  ‰Ñ¸ŒŠŠƒz˜¢—¤—r–ÚƒL\"
	"ƒC’†—R  ’†S—R  Ú’U–Øåå  ’†  ˆ£ˆßŒ¾å“Ñ¡‹àõ”nå–Ø‰J’–Ò  "
	"’š  Ò  ŠL  ˆê˜«™X–kü  Œû¢—´  ‹Ğè‹Ğ’·›ö’š‹|’š‹|’·üœcœe¼’¥  "
	"’›  \ä‘ìŒ’©  •Ğ¢“c’š–Ú’›¨SŒ’·Œä’¢Œ¾üŒ¾¢‘–¢‘«’›‹à’›"
	"‚d  ’š•Å‰G”’‘©—Í•à  –Ú  Œ“VƒV‚ñ‰¤œc”CŠL‹à^“Œ  ãä  ‘à  –Øè°–Ø’Ç"
	";g  ’Ç  ›öáTáT  ‰Æ  –Ø•ê‘J–Ø‹K“c  Ó  –ØÎ\çˆ’¹  …–”‹à™ö–Øt"
	"‹M  “y•½ˆŸšâ—ù…—RƒC”ªŒû‹Ğ‹àÙè°’¹˜³’š  ’à  ’å  ’í  èŠLŒû‰¤"
	"¥  •D  —§‹Ğ›ö›ö’ìœMp’¢’í    ’ì  ¥  ’í  ƒV’šÎ’èƒl’å‰Ñ’æ"
	"…’éM’ìŒ¾’šŒ¾’é‘«’éåh    “Ñ‘å‹à’š–Ú  “ò  ¤  ƒˆ„¤¼¤  ”’Ù"
	"’|—R¤  ‹à¤ã  ÜŒûU  U  ÔU¸  ‹à¸‹ÈM“y^‘å  ›™  ›öèšî¬"
	"o  ãŠÃŠLèÔ‰]^•Åè‘ü•§‰]› Ÿt“a  “úo‰JbŒZ  Œû“y•¨  —]  —Î"
	"›™Ò‘–  ‚S  –Ø“y“x  á “¤–Æ  ŠLÒ—]  Ò  ‹à“xÎÎ–œ“z—Í›ö“ù\ã"
	"—–”“zS“  ®ŒZšç  “Œ  ƒmƒt›ö  ä§  “‚  ‘å’·Î  ’¹  R’¹‘ì  Ÿt  "
	"“ƒ  ‘©  ’›  õ  “Œ  ŸMŠÊ™¯ä  õ  ‰Î’š‰Î“oƒˆ  ›ö“¤õ  ›  ‡  "
	"“¯  •Ä“‚…[Š  d  “’  Ÿì  Œ¾¡—_  OŒû‘«ŒB’›  G  ‹à“o“‘  “¤•Å"
	"”n  –å  “®  d—Í™c  ®“y“¹¡“¶  “¶  “¯  “¶  —§—¢Œ“¯‹e  ñ  ‹à“¯"
	"ã‰º’¹  ™·á“ú¡œeS”„    f–Ú‰Ñ™X”n  •ê  ’  Œ¾”„–Ø–œÛ  ™[]"
	"ŒŠ‘å’i  ›™—R’¹œTè  ’ˆ  ¼  ´Ã“Ú  Rƒ‚‹  ‹¼“Ô  æ³  ‚  “Ô•Å"
	"šî‰E‰_  ‹à“Ô‘å¦‰N“™clƒP‰º~  è³  –À  “ï  “Ş  ‹àšDU  ”nì…‹T"
	"“c–”™c“ì  ÔŒ‡•vè°—  ˆê  ›™ƒq“ñœT›•  ƒq™¯’C  “à  ’H++  Œû  "
	"›t‚µƒm_—Œû›™…èê  p˜À—pnS”E  ù  ¢  ƒl›•S’š™°š•`•cŠÛ  "
	"Œßƒ¡S”O  ‘R  ‘R  •ÄèƒA  œM¼˜³‚y—Ñ“y‘´ˆßƒc‹¥”_  …“àƒ€ƒqƒc‹¥"
	"”_  ‹È’CiŒ©–”’–¤  ”b  ”Ô  â  ”b  ”ç  ’Ü  ‰¤”bÎ”ç”g—”n  ”b  "
	"‚d‚T”ñ  ”­  OI”ñ  ŠL¼–Ø•s•sM•Ğ”Ú–kŒŒs”ñÔ“ÑŒÈˆÊŒû—§Œû—–^"
	"–ˆ  –^  –^  ŠL  –`ˆõmçŠL—§—§ŒûŒ¾  ’‹T‰Ñ•½–îˆøH  m”’ƒˆƒ†ê  "
	"”’  ”’  ”’  “ú‰P”‘  •Ä”’M”’ê  ”’  –\  ”œ  –\  ê  ’U“V”nà©—F  "
	"™  ‘Š  Î’JÒ  ãä  ã  —¶  ”Ô  Œ™{‰Î“c¹“cƒm_‹à–{”­  á   “Ñ”­"
	"é•—F‘ãœ÷æi  —F  ”°  –å”°‹ã’¹V  ‚  ’‡è°  ”¼  ”¼  ƒL™Ê–””¼”½"
	"‹Ğ–}“Š”Ê‰¤•¶”½  :e  –}  •Ğ”½)e  ‰¤ƒŠ“c”¼•q  MŸt”×  ŠL”½ça  ‰Ñ•Ä"
	"‰Î•Å•ª•ÅH”½–Æ  –Æ  ”Î”ÊM”ÊÎ”Ô  –’’™·”ñ“cƒ˜¡  —ŒÈ›ö”ä”ç  "
	"”ñSŒË”ñ”ä  ”ç  ”ñ•¶ãƒq•K  ›ö”ç›öxÎ”Ú•K  ”ñ  ”\  Œ”b”ç  ”ñ  "
	"•¤ŠLç„  –k]E¡  ’Ê‰±‘´”ç—p  ›™–Ñœe¼”ä  “c©‰¤”ä›™–Ú—r•v”©  –Ø“~"
	"”Ú  ™·™X‰º‚dé•”ä›öœc½  äŒ  Œ¡‹|•SS  —¢  ãä  “c  –Ø‰ï—b—à§"
	"…‰N”’  ‰Hœc•\  ŒÕœc•[  …  •[  •[‰Z¼¦ˆß  Œ¾•½æ¶Ù›ö’©•c  ›ö•¸"
	"‰Ñ­“c  ‹à•c‹à•º¦  ’Š‹›˜VŒûˆ—Ñ™•  •º  Â•p•‰•ª•n  •à•Å–ˆ¼"
	"›óŠ¢ƒn˜¢ƒC¡•Œ  ‘å“V—›â•y  “¯“cƒi‹Ğ›ö•t•z  •v  •ú  •ƒ‹Ò•À“ú›t  "
	"”ª˜§•t  •{“÷ågˆİ•v  Œ¾•©ŠL•  ‘–ƒgŠ  •t  –ˆ  –³  ³œT–³‘C•áä¤"
	"–³  —§ŒûŒ\¡•—  ‘ú’‘ù  ˜H  ‘ãŒ¢Œû“cœešç‹Ğ“cŒ”½ƒl“cŒšçƒlšç•œ  "
	"ì  ’¢ˆäƒ€  •¤  ƒCƒ€‹–Ü‹›•t”ª“–Ü  æÊ  æÊ  æÊ  •ª  —Ñ‰ÎšS  •Ä•ª"
	"•ÄˆÙ…•ª‰J•ª˜³˜§–å¨“à  ›ó  ‹u  ›   Ç‹ĞŠ±  ÇœO•¸  aHÇ  –åË"
	"”ä“y–Ø¦•SŠLç„  ç„  ç„  àßÎ–œƒŠÇ–Ú–Îœù“à”äG  –’šç›ö„¢G  G  "
	"“  ”½  G  X  –Æ—Í—–Æƒ€œOŠv•Ö‹x•ğÉ•á‹à•áš˜•á•á  ~­—p\•á  "
	"Ô•áŒb  ”œ•ª”œ“y”œ¬œT  ”œ“ú’Oƒˆê  äƒ•ú  •ò  ŒÈ™¯ƒ–ØK”½˜¢•v"
	"‹Ê  RšçRšç•ü  ›ö•ï•ï  •ò  •û¼–œƒiŒ  ‹  •ï  ‹  Î•ï…ˆ§•ï  "
	"•û  –¾  ˆ§  ’  •Û  Œ¾•û‹Èæ¯‚  ‹à  H•ï’¹  •ü’¹”V  ˜³™¼Ó  —§ƒ"
	"“y•û—•û‹Ğ–`–SS–S  ŒË•û‹¤  –S‰¤ŠÃ•ğ•î•ò“ú–Ú…•ûŒ•ûœd  Œ¾–^æ¶á¤"
	"ŠL  ‹à–´•û  ƒŒ¢‹K•ÅƒRƒq”ü  ‚Œ˜¤•  ”ü  –Øƒg‹¼—¤  ‰Ñœc‹àŒûš—Í"
	"Ÿt  Ÿe‘ä‹ü  W  ‰î™Á–Ø  ”Ô‰H™{  •ªM–ƒè–ƒÎ–ƒ‹S›ö—Ñ—¢  —–¢“ú–¢"
	"–Ø¼Ÿx  —¢  ^  ”œ•z”œ  –Ø›—‹›—L–Ø³‹›‘¸‘C  ˜³¬ƒ“Vƒk˜§––  –Ø  "
	"––  Œî  s  …’–ƒ˜CƒN‰º™Ö  —¼  ™Ö  ™Ö  –¢  –Ø  ‹S–¢ŒÈ  ‘´  Rb"
	"›~•K›~•K‘t  Š  ”O  ”h  —­•Ä–Ñ  –Ú–¯–µ¼—[  û™Àƒ€‹—\ƒI‰J–±"
	"•’¹–Ø‹—•D——Ç™k’U—[ƒ’@‡“úŒ–¾M•Ä  ‹à–¼ƒ’¹—Š‹ƒqŠD•èƒN™Y"
	"–Ø›å…›å–Ê  §•S”–Ê”œ  ”œ  •è  –S—qM‚àƒ‚–Ğ  –S–Ú…ã¦–¢–Ñ‰Æ  "
	"M”\‘åàÒ•“úŒ–ØH™¯  H›ó›—  ŒË‘å•Än¢ŠL–åƒ–åS…•¶F]  ™¯n"
	"ƒZ‚â‘ä  ˜³  •ƒ–ë¨  —¢—\›•  “V  ™Ê  œeŸtÙ  Šy  Ú  ‘«è°—§Â‰K  "
	"”  Œ­  ‘O  ‘O  —R  –ú  Œ¾‘OÔ‘Oƒè°ƒC‰E—J  ’j  ƒi–”—L  ›ôR¿  "
	"ˆ¤•S¨  ‰EŒ–Ø—R—E  áT  U  UŒ¢“c  ƒl‰Eƒl’JŒ¾GŸà  ”b  ‚  Lè°"
	"é­’ƒ^ƒNƒ}’šƒz  ‚T‚ÓŒ¾  ˜Z  —\•Å—f  ›ô—Í—šî’J  ›ö—pä  ŠÊ  „  "
	"—B  ä  —r  —r  —e  —e  Œb‹óã´‰¤”¼Œõè°¢–Ø—e  ¼—Œ¾ŠÊáT  ŠÊ  "
	"ä  H  —~  ˆó  ’JŒ‡šî  ’J  ‰H—§‰HˆÙ’è  ˆÛ  —İ  ‰Ê  –¢  —ˆ  ‘©•Å"
	"‰J“cŠe  …Še—Œ  “ÑŠeã‚µ‰K  •—  èŒ­ŠÄ  ŠÄ  èŒ  bŒ©‰ÑƒŠjä›™•œ"
	"–Øq—˜–Ø‰¤—¢‰¤‹×›ö—˜—¢  ƒl—¢b‘‹×è°æ“yãä  ‘²›ô˜³ƒj—¥  ‹  “cŠe"
	"‹à  [  —¯  [  ““c[  •Ä—§šç¶‰¹b—§  ˜C  ågv•ûTKåg’jƒt˜«ç  "
	"˜µ  ™cRæšç˜µ  •Ä“lS  ‹  ê‹  ›ö˜µ˜µ  æšç—Ê  ¯  Œ¾‹˜µ  ’U—¢"
	"æšç—ß  “ƒi…ƒˆ˜Ó  ™Ê—¢–Ø  Ÿ”—Ñ•Ä‘C—Ñ  b•iÔ˜Ó•Ä‘C•Ä‘C­‘C—¯  "
	"—¢  –ß  “c…•Ä‘å¡  —ß  —ñ  —ß  –œ—Í—Ì  —ß  ‰¤—ßƒl‚µ—ß  ‹à—ßŠ¼è®"
	"‰J—ß‰J•À­  •—ß™Ê—Ñ™Ê—ÑŸeƒŠ­—Í—ñ  —ñˆß›öŒ“–’S•Ä‘C˜A  ‰Î“Œ—õ  "
	"…“Œ‘  ˜A  Ô  ‹à“ŒƒŒû‹›  ˜D  ‰ÎŒËŠLŠe‘«Še‰J˜H—Í  —  ›ö˜Y‰¤œO"
	"—ÇŒ•Ä—˜Y  —Ç  ›™‰J‹  —Ç  —³  “yƒq—´¨’ê‹—Ç  ˜³”ª­  ƒlƒˆŒ—Í"
	"‹àƒˆ˜Ó  ˆÏ  ‰ÑƒŒ¾ã•s³ŠL—L—Íˆˆ½  –Ø™¾A’¹Œİ–œ’U  ‹›™öŒ¾‘î‚  "
	"™Î  ˆ¶  œ^  ˆ¶  ˆ¶  ";

static const char *ro2 =
	"œT  ³™¯•s  ƒw‚Œ‚èƒ‹    ˆä      ƒm_ç–k˜¨  “û  ‚Œ  Û  ‘ˆ  É—\"
	"˜Ÿ  ’šƒG][  H‹zˆê  ˜³™{‹  ‘î  ‰ñŠl^^”T  ™ÊlƒCƒgƒCƒJƒCäƒC“"
	"ƒCnYç‰î  ˜´  ¸  ŒÃ  •¤  ‹å  ›€  ›~’š‹g  ‘½  é  ‘î  ’›  ‘ú  "
	"•S  —L  —r  –Ø˜¸‡ûá¶  Œ©  áá  lŠe  –Æ  áT  —¢‰À—˜  ’í  Ô  "
	"Šï  ‹  ‹ü  ™Z  ‹ó  ‘²  ‹w™¾f  ¹  Â  ‘ì  ”Ú  •{  –å  ™_  ™·  "
	"ƒR–”‘\‡ŠF  •F  ˆÕ  ŒÌ  Ò  š  ™`  ‹S  Œğ  ê  ™½  •íÂ–½      "
	"ê  ˜K  Šì  à¨  ‹Ä  ‘Ö  ‘Ö  “¶  æÉ  ˜j“c™@  è²  ”_  ŠÄ  Ä      "
	"–²  •i  “ï  —í  Œµ      ƒ‹  ™X  ‰P™YŒZ  –Æ    èí  ‰J  ‘O  ”ª‚T"
	"–kˆÙƒ¡ ]™c–¤™¿  —R™c™c“¯—RŒò™f–Æ  ˆêÊ™k™\™kŒ³æ³  ‰PÙ–‹      "
	"ší  Œİ  ’†  …  ŒZ  —ñ  ŒÅ  ‹  âh  ™X  ågˆ–Ø™{”C™{c™{‚kv™˜²"
	"n  ƒ`ƒŠšíƒŠ–Ü  ‹“û  ãƒŠšğ  —˜  ›ó™ƒ›İ    ‘©  Šï  ˆÕ  ‘O“"
	"æ¯  ˜©  ä§  ‘ƒ  •[  ™@  ™@™ƒ™@“™ƒ  ç„  Ä  h  h  ‹å—Í¢—Í‹g—Í"
	"•v—Í›İ  ‹—Í•  ˜J‰ÎÓ  ‘ƒ      œc  ŒM      è°  ƒmƒt–Ün‹¥™¯“c™¯"
	"•á™¯™¯  šğ•ïƒq‚”‚kƒ¡™·b™·y™·‹M™·  P‚k™·•i‹ã\œOì“ù‚¹œO\\  "
	"”¹y‰º˜³ƒ¡|]–ï  ƒ^™Å’J™ÅŠª  ƒmP™Êœc™Ê‘¥™Ê‰Ä™Ê  ™Êz™ÊÆƒ€  Q  "
	"Z  Ç  ‰Px“ú–”    ƒ’šƒ“ƒ”ªƒ“üƒ˜°ƒ‹ƒ‰åƒ‹Òƒ˜´ƒEƒˆò"
	"ƒ“àƒ•ª•¶ƒƒÚƒ‰iƒ‰ÂšçƒƒŒºƒ‰ZƒbƒŸƒ™{ƒ\ƒŠƒ“zƒo"
	"ƒ•tƒ•ïƒŒ\ƒ–œœúˆ½ƒŠƒŒğƒ‹¤ƒ‡ƒŸÚ‘üƒ¼ƒ‘îƒ˜VŒü  ƒs"
	"‰Â  ‰ä  Šó  Œá  X  F  Œ¢  •á  ˜M  ‰—  ˆ³  ‰µ  ‹ó  è°ƒ–”::‘ì  "
	"‰Š  ‰P  ”O  –ß  ˜b  ‘í  ‹q  Še  ™÷  ˆİ  ’é  H  §  ’@  ’P  ’é  "
	"“ì  ™`  h  —º  ‰G  L  ·  ‰Ä  ˜V“ú’  áÁ  ™½  Â  Ó  ‘°      "
	"–ƒ  ˜A  šã  Ší  ‰c  ŸŠpz  ’©  –³  ˆÓ  ‹Ö  ãç  â¬  •i–Ø‚  ”J  "
	"•@  —«          ‰  ‹½Œü‰d  Œµ  Ší  İ      G      åY  ‘¡      "
	"ê–  ƒR™š˜‰»š˜—ßš˜•ûš˜—Lš˜Œáš˜Kš˜Šªš˜ˆ½š˜èèš˜ˆõš˜êš˜  š˜›Şš˜ˆ£"
	"“y“ü“y‰º“yR“yŒ‡“y‹Ò“y~“y•s“yŠÃ˜¨  ‘ã“y“y”ç“y•t“y•ï“yˆå“y¯“ys"
	"“yŠ“y›óã‰º“yáá“yŠp“y•á“y¡“y¡“yˆŸ“y‰Ô“y‘²“y•ü“y¼“yšD“yä“y•Û"
	"“y‰G“y‰Î“yŠC‰PŸt“y“y³“ya“y–ì“yŠÔ“y‹•“y‘¸“y‰œ‰ó  “yš¥“y’˜‘Â  "
	"—i  ‰}  ‰b  á¶  œA  •i  –n  “Ü  ë  —´  ‘s  ”„˜±“¤  •DŒšâ  q  "
	"‹vƒ^šç      ƒˆŸe—Ñ—[‰Ê‘½•v  ‘å  ‘å\‘å‚T‘å  —§‰Â–’‘ål‘å‘åŒ\›ô‘å"
	"‘s‘åÒ  U  ‰œ  «  ™½  —Š±—Ù—  ƒC—–Ï  —”ä—’U—•ê—ˆÎ—r—"
	"—œO—”C’›  ‰ä  —Œ¹—“ß  —  —’j—ˆ¢—pˆ¶  æ  æ  ”Ú  —Ñ—"
	"”û  M  ™i  ã  ‰Px—”nà  ™½  í      •[  ŠÔ  ŠÕ  ‹ª  šd  ç„  "
	"’j—’j——  •@  —Š  ìåõ    ‘š  —¹  ”Tqq  š  “zqˆå  ‹ŠÛ›ôq"
	"—‘  Šw  •¶qù  ˜³  ›~ƒq›~b’C  –Æ      è°  ¥          ŠÑ  Q  "
	"”œ  ‰Hœc‰PÙˆ£  ŠL  ŠL  ¡«  ê  ‹Æ¡ƒP¬r  ‘å‹ã›—œc™Ê  ›™›ö"
	"›™”ä›™  ›™•Ä›™ŠL›™  ›™›ó›™  ›™  RƒiR‚µR“RŒî‹y  ¡  •ªRR—"
	"R—RRR”’R•½R–¯R•ÙRè‡  R›‰ä      RXRÑ’¹  R’J{  "
	"ˆ³  ‰ª  šò  ‹à  ‹ü  ©  è°  ‘ˆ  Ršç˜Ó  ˜Ó  ŠÃŒ‡•i  â[  ”û  ‹S  "
	"·  ã  ™½  a  Í  ‹Ä  “o  ‹`  ™@  –  —²  ‹^  äo      é°  “^  "
	"    Œµ  <<  H˜¸ŒÈ  @”b‹Ğƒˆ‹Ğ‹Ğ¸“z‹Ğ”’‹Ğ    ‹Ğè°‹Ğ‰®‹Ğèè‹Ğ’å"
	"‹Ğ–»‹Ğš ™Ö  ‰¹œT‹Ğ“¶Æ‹Ğ••‹Ğ++TTœO”ƒ€  –ƒ›ô˜³™Ê›ö—r›ö‘¥›ö‘Š›ö‰Ä"
	"›öŠù›öŸu›öœc›ö‰©›öz›ö¡›ö–n›öá¢›ö–³›ö‰ğ›öâg›öá¸›öè°›ö  ›ö’®    "
	"œM—Rƒi\œO  œO              ®  ‹|  “z‹|‹|¨•ü  šF  ’e  ¢  Œ¾‹|"
	"–’‹|ƒˆƒ†æ³  ƒˆ  œ_‰Êƒm  ‹gœcƒC  œe•ûœe¶œeŠœe•¤œe‰ñœe¯œe›İœe{"
	"œe  “k  ”ñ  œe  œecœeŠÊœe  ¡  ‹Ò  Œß  ‰N  ›—  šî¬’¼  •ªS‘ä  "
	"İ  ŒÃ  ‹å  “ò  “áS™°š’U  ”’  •¤  •½  ‰›  ‰i  Œ\u”CSŠe  ‹xS"
	"™±  —ÍˆŒİ  Œõ  ŸS›  ŒŒ  {  ã  “¯  —rSŒ  Û  ‹ï  ¢  ‘©  "
	"Ñ  ˆò  š  –Æ  —W  —˜  ™å  ˜±S‹G  Œb  Šª  ‘²  ™¾  È  ü  ’·  "
	"ã¦  M  ™ö  Ÿ¥  c  tSH  §  ¯  é  š  ‘¥  ›İ  –¯¼“úšçŸu  "
	"‹C  ’  ‹S  Œ“  Œ´SáÁ  ñ  ‘q  ê  —O  ŒI  Œ˜  N  Q  a  a  "
	"]  K  ä  ‘¢  ê  “®  “½  •[  —f  Šì      “[  Œi  Å  šd  ”õ  "
	"”n  è{  –³  K  ‰œ  Šå  ‰ù  ‰ğ  ‹Î  •i  Œ¾  —Ñ  è°  âg  ù  –  "
	"—Š  Í  Í  œ“  è°  áØ  ãá  —ö  œT  •è  •è  •è  œ÷  œ÷  œ÷  rœ÷"
	"è°œ÷œ÷  í  ‹Y  œ÷  ŒËûƒL‚µƒLŠ±ƒLƒƒLHƒL³“  –ï  ‰¤  ší  œ÷  "
	"—\  ’Ü  “l  ”²  ‰º  •s  —c  ŠÃ  \  “zè‡èÜ  Œ¾  è  ”q  ”¼  "
	"•t  •¤  •ê  ‹ã—Í—§  Še  ‹g  ‹¤  ‰H  Œ\  è  å  ‘¶  ƒŒšñ  Û  "
	"‰Px“ú“y–é  Šï  ‹Ó  æ  ‚  §è™¯  ‘ì  ’è  –½  –å  –ß  æ³  ŠF  "
	"“Œ  á¡  §  _  ç‰P–ë  ™`  —[ŠÊŠ¦Œ™i  ·  ã  ‘q  ãá  “‡  ‰H  "
	"ê  ›Á  ·  ê  ‰HœcŠo  z  êŸ  á¢  ˜µ  ™§  Š´  æ³  ‹×  ˜·  K  "
	"’B  ç„  —‹  ~  äo‹“äoƒLÄ  º  ‘ä  šæ  •o  ——  û  œA  “A  ”ë  "
	"‹“  ŠyÙåõ  ›Á  ^  “ï  …Œ¾Ç  –”  –”ƒP¼  ¼  ƒC¼“c¼Œğ¼•ú  "
	"‘©¼—]¼—]¼®¼‹Ğ¼‚»”  ™@¼Ç€…Œ¾Šp“lr“lÎ‹Ò’f  •û’O•ûs"
	"•û—§•û–Ñ•û¶•û[•û”Ô•û”Ôƒ`ƒ‚Ù  ‘Š±“ú  “ú“V“ú˜º“ú•¶–Ø“ú“ú“ò‰i  "
	"“ú‰K“ú–ÜˆÀ  “úŒõW  “ú’›“úŠó‘  “úŒá“ú“ú’C“ú¬Ü  Í  ”ñ  ŒR  "
	"‰p  ŒR  é  ä  –»  ’U  i  êŸ  “Ö  Ç“ú‰Ø  ˜µ  ˆ¤  –Ö  œA  L  "
	"‹`  ‹¤  ‚dŒû\œ÷“ú“õŒoŒ—Ç‘´  –Ö  —´  ‰Jâƒz  s––”T–Ø–Ø“ü–Ø—Í"
	"–Ø“–ØŠ±–ØŒÈ–ØH–ØœT–ØR–Ø–œ–Ø‰¤–Ø  –ØŒö–Ø—\–Ø­–Ø•ª–Ø•û–ØŒË–Ø¡"
	"–ØœOU‰Á–Ø‰Â–Ø‰û‘©  –Ø‘ü–Ø™·–Ø‹å–ØŠ–Ø“á–ØË–Ø–Øo–Ø•ï–Ø•t–Ø—§"
	"–Ø‰Z–Ø‘\–ØŠ±–Ø‹§–Ø‰H–Ø‘C–Øšğ–Øl–ØŠ–Ø[–Ø‘¶–Ø–´–Ø“––ØŠp–ØÛ–Ø’¹"
	"–Ø–Øˆò–Ø›à–Ø¿–Ø“ß–Ø’ì–Øšæ–Ø•º–Ø›t—Ñ–}–Ø˜C—Ñ‰º–Ø˜±–Øµ–ØŒN–Ø‹"
	"–Ø‘´–ØÙR  –Ø‘–ØŒ”–Ø‰ª–Ø‹ó©  –Ø¨–Øœû–Ø@–Øˆò–Øf–Ø¨–ØR–Ø  "
	"–Ø’m–Ø‘ì–Ø®–Ø”O–Ø•{–Ø•À–Ø–å–Ø“T–ØŠ–Ø˜Ó–Ø”T–ØŠF–ØŒÓ–ØH–Ø¨–ØŒ_"
	"–Øò–ØÒ–Ør–Ø’é–Øæ³—Ñ–µ–Ø–ë–Ø™`–Ø•û–Øj–Øº–Ø¢–ØM‰h  –Ø‹S–Øæ¯"
	"–Ø‚–Øv–Øœ–Ø·–ØŠ¦–Øñ–Ø“‡–Ø‰H–Ø”Ê–Ø”Ù–Ø‰Î–Øê–Ø–»–ØÓ–Ø—e–Ø—¯"
	"–Ø—³–ØŠsŠy  –Øœc–ØŒ\’Å  –ØHŠp“l–Øa–Ø]–ØŠ–Ø™½–ØÊ–Ø‘ƒ–Øê—Ñ‘å"
	"–Ø–§–Ø–¨–Ø‰i–Ø˜K–ØŠ¸–ØŠÑ–ØŠì–ØÅ–Ø»–ØŸ}–Øä@–Ø“o–Ø“¶–ØêŸ–Ø”ü–Ø’¹"
	"–ØŒ¾–ØˆÓ–ØŒh–Ø•ú–Ø™@–Øš¥–Øç„–ØŸ@–ØŠÏ–Ø™º–Øè°–Ø”J–Ø•o–Ø–Ö–Ø‰–Ø•i"
	"–ØÙ–ØŠy­èƒ–Ø—ğ–Ø‰d–Ø§–Ø  –Ø‰J–ØŒ¾–Ø——ŠÊœc–ØŠÏááŒ‡ŠóŒ‡“  ŠïŒ‡"
	"ˆù  Œ‡  ‰PŒ‡Œ“Œ‡™½Œ‡‰HŒ‡‹•Œ‡™@Œ‡äoŒ‡è°Œ‡t›â—[–œŸe–”ŸešîŸeœcŸe‰›"
	"Ÿe›tŸeœûŸe—§ŸeˆõŸeäŸešãŸešdŸe•oŸeœTŸeœT™{–”Ÿt  ”„Ÿt™½Ÿt‰N”–ˆ["
	"­–Ñ–Ñ‹–Ñ  –Ñ  –Ñ‰Š–ƒ–Ñ˜·–Ñ–S–¯‚µŸƒ•ªŸƒˆöŸƒ•ÄH…R  “y  ‰¤  "
	"‹Ò  Œİ  ~  S  s  •ª  “ú  ”½  –”  –Ø  ¢  ‰›  O  ŒÃ  l  ˆö  "
	"Ë  Š  ›€  è  “c  –R  –¯  •½  –Ú  ˆÎ  s’ó™±  ŒŒ  ‡  Œõ  é  "
	"{  “ğ”@¼  —ñ  Š®  Œ  G  ˆò  šñ  ÜŸ‹‰„  ’í  šæ  “ú“y‰‚  ƒVƒŠ"
	"•Äì”Ÿ  ‘´  ‹à  ŒÅ  æ  ‘²  Ÿ”¼®  à¥  È  Ÿ”Íœû  @  ‰—  “y™X"
	"˜Ó  y  ˆİ  ¼“y‰Õ  šô  à§  c  ŒR  Ÿ”’UH  Ÿ”Ÿ•ò  §  ’à  ”q  "
	"Ÿ£¹–Ê  –u  –  ™`  •ûqh  áá  –@MWŸ©š˜  É  J  ñ  ‘q  ™Õ  "
	"‰P  Œ‘×“‚  ‰Yê‹ƒÓ–»     Šù  ŸÌ  —Ü—W‹–  ˆ£  ›’  Q  ‘©Œ‡‘Ñ  "
	"’£  Š  ‰j  ‹¥ƒ€êb  êŸ  ›™  ƒzŒ~  ~  q  ö  ö  ‘  U  “¶  "
	"”Ô  œd  ˆŸ  —õ  ˜µ  ‰œ  Š²  Ÿ”•iK  Œ¾  ŠL  —ë  Ä          ¢  "
	"”J  •o  –l  –Ö  ™o  R  æË  –\  —{  —«  —¶      Š²  ’–  —ğ  á¸  "
	"ãç  à]œ\èŒ  Ê  ——í…Œ¾—[‰Î‰Î­‰Î™c‰Î™c‰Î‹‰Î“á‰Î•¸‰Î•ï‰Îˆö‹x  "
	"‰Îå‰ÎŠe³  ‰Îšç‰Î©‰Î—§‰Îšô‹–¤à†  ‹å  ‰Î  ‰Îc‰Îà§‰Îä‰Îd‰Îà"
	"‰Î‘§‰Îv‰ÎˆÑÂ  ‰Î–åŠì  ‰ÎœT‰ÎêŸ‰Î“Ö‰Î”Ô‰Î˜µ‰Î‰œ‰ÎšÊ‰Î‹‰Î’B‰Îá¶"
	"‰Îæ³Kè°‰ÎÙ‰Îá¸‰ÎèŒ•°  ƒˆ›š’Ü”b—F  ˆ×  ƒ˜§à©Š=|  à«  à«š¥•Ğœû"
	"•Ğ”ƒ‹‹Œá‹‰Ñ—˜‹‹ˆ‹‚˜S  ‹”ƒ‹‹`–Ş  Ë  ‰N  ’†  ‰Î  b  "
	"•¤  Še  ¯  Œğ  ‹·  ƒŒ¿  Šï  ™Z  Â  ¹  ‘²  Œò  §  ¯  ˆØ  "
	"œ  ›’  ”œ  •Œ¢™Î  ‘\  å†  ”J  b  ‘l  Œ¢  £  ‰Á  ‘ã  ›•  ”ç  "
	"”’  ¨  ‘ú  Še  ˆŸ  —Ç  ˜Y  ŒÕ  Œá  ”ñ  –@  ƒR–”ŒR  •K  ‹¥›İ–`  "
	"™`  •ó  ‹S  ‰¤ŠL”n  ‰¤ŠÊ‹Í  Í  ”ü  ç„‹Ê‰Ä  —´  àü  —  šğ‰Zh‰Z"
	"Š¢\Š¢çŠ¢ŒöŠ¢“ÔŠ¢•ªŠ¢–ÑŠ¢•SŠ¢pŠ¢¼Š¢HŠ¢—Ğ™½Š¢êŠ¢–²Š¢Š¢  Š¢  "
	"®ŠÃX¶ƒ}—p“c’šƒc“c“c‰î“cˆä“c•v“cœc“cƒ€“cƒ€“cˆß“c“cŠe“c‘“c—]"
	"“cŠï“c®‹®  “cšæ“cõô  “c  —Û  ›ö’š›ö‹v›öR›ö‰î›ö–Ş›ö‰Á›öŠÃ›öŒº"
	"›öŸ›öŠ›ö’U›ö“~›ö•ï›öˆÎ›ö‘S›ö—r›ö›İ›öu›ö”Û›öˆ¢›öˆÏ›öŒÅ›ö‘²›ö‰Š"
	"›ö”Ú–ƒ  –ƒ  ›ö•—›öä›ö™`›öM›ö‹s›öÒ›ö‘q›ö”Ê›ö—¯›öÍ›ö—İ›öÈ›öŠÔ"
	"›öá¢›ö™§›ö“a›öäİ›ö‹^›ö—{›öè°›ö  ›öÏ›ö—ğ›ö‘N›öè°›ö“^    á “Vá Ÿt"
	"”’ƒq”’™X”’”½”’‘å”’Œğ”’Š®”’Í”’”’æ¯”ç•ïˆò”çŒR”ç”çŒRä”ç˜°M‹M"
	"—rM‡MœûMãäM…MågM“’M“ M–Ú™a–Ú›—–Ú­–Ú  –ÚŒº–Ú“ò|‹ïŸ–Ú"
	"–ÚŸ–Ú–¢•v–Ú–Ú–´–Ú’í–Úˆ³–Ú™Z–Ú‘––ÚÂ–Ú”Ú–Ú  –ÚK–ÚÒ–ÚãáÁ  –»  "
	"“°  –Ú  –ÚŠ¸–Ú‹M–Úˆ¤–Úè°–Ú™@–ÚŒÛ–ÚŒ¾–Ú–ÖáØÇ’¼•i–Úå†–µ¡ƒ€–î–îˆÏ"
	"ÎHÎØÎ”äÎœAÎŠÎäİÎŒ\Î‘²Î‰Ô”gÎÎ•üÎ—§Îœ_Î“úÎ’åÎr"
	"Î›İÎæ¯ÎœÎ·Î‘CÎ“WÎ”nÎÓÎ•iÎºÎÓÎêÎêŸÎ“oÎ‰œÎ‹`"
	"ÎácÎ‹^Î—ÑÎÙ¦–¤¦i¦¦o¦“á¦•K¦œT¦‘´¦œ_¦Œ_¦ˆò¦Šì"
	"¦Ä¦šd¦–L¦åõ’  b’çƒˆ‰Ñ”ä‰Ñ‰›‰Ñ‹‰Ñ  ‰Ñ––‰ÑÛ‰ÑÑ‰Ñ‘´‰Ñ’¼"
	"‰Ñü‰Ñ‰ñ¦‰ñ‰Ñ™f‰Ñ‰P‰Ñ‚‰Ñˆò‰Ñ—e‰ÑŒb‰ÑÒ‰Ñš¥‰ÑÎ‰Ñ‹}‰Ñ‹T‰ÑåõŒŠ‹|"
	"ŒŠˆäŒŠ—cŒŠƒ^ŒŠ’›ŒŠŒNŒŠŒŠšDŒŠ  ŒŠŠÊŒŠÈŒŠ•úŒŠ‘lŒŠ—²ŒŠ‹ŒŠæÌŒŠ‰Ñ"
	"—§\—§ç—§•ª—§–Ñ—§è—§’š—§  —§•S—§áá—§‘©—§ˆÕ—§—ĞŠÛ  –Ü  ’Ü  ”b  "
	"‰Á  è  ¶  ‘ä  ”Ã  –{  –î  ‹§  â  ›òœO{  ›š  ‘S  æ  ‰„  ˜C  "
	"šñ  Œ©  ì  ¿  ¬  ›Ş  Y  ‹Û  ©  ‹‘  ‹ó  “š  œû  ›â  à¥  ‘ˆ  "
	"•  šñ  c  Œò  ŠÃŒ‡™÷  æ³  ™i  t  Š  –ª  ©  ŒI  —´  Ó  ‘°  "
	"’¤  •L  ˆ§  —À  È  à  ‹M  ‰©  ‘Ö  ¼‘Œ¾  ãç  “š™@šæ  ŠÄ  ”  "
	"Šø  —¯  Ÿì  Ÿì  —Š  ”³  ”³  ê  —£  •Ä\•Ä”ä•ÄŒË‰œ–œ•Ä‡Ÿ•Ä•Ä§"
	"•Ä“¯•Ä•S•ÄXƒ^•Ä•Ä—À•Ä—Ç•Ä‘²•Ä@•Ä‰Ô•Ä_•Är•Ä‰Æ•Ä—p•Ä–ƒ•Ä”œŠŸ  "
	"•Äù•Ääİ•ÄG•Ä  …‚µ…˜°…¡…‰]…”ä…•¶…™c…LD……¢…‘ä…’š"
	"…”¼…ƒ…Œõ…s…  …^”@……‰g…›ó…›İ…G…¿…‘Ã…˜C…”E…Šï"
	"ŒË……Šª…Ñ…áV…‘ì…œû…‰¶…ü…ŠÊ›åŒn…˜Ó…–ß…Š¯…™÷…¨…¢"
	"…’i…’vÑ  …¨…ˆĞ…‰vŒ§Œn…É…·c  …J…W…’Ç…  …œc…‹­"
	"–ƒ……™Ö…•[…•ö…˜K…—İ…˜A…‰_…–å…U…êŸ…”Ô…˜µ…K…‘\…  "
	"…Œp…ù…Ä…Ä…•o…h…ŠÄŒ‹•Å…^…æÌ…–ni•Å…‰d…–Æ…œ÷…œ÷"
	"“Å……——ŠÊHŠÊšíŠÊ  ŠÊŠLŠÊ  ŠÊ“ÜŠÊè°™cƒŠ±  ™c–S•s  ŒÃ  –¯  ‰‚  "
	"‘ì  —Ñ  “¢  Œ¦  ŒF  –‹  Šv”nŠvŠï—r™Y—r  ·‰N—r—r—ß—rŒN—r  —r  "
	"—r”ü—r”ü—r˜·—r  —rŒ¾x‰H™¾‰H—§‰H‡‰H—r‰H”ñ‰H‘O‰HG‰H‰£‰HêŸ‰H”Ô”ò"
	"˜V|˜V–Ñ˜VŠ‰Ñ–¢ãÎ‰]ãÎ”bãÎ˜CãÎ•ãÎJ¨‰Î¨~¨‰K¨—ß¨ã¨—Ræ  "
	"’m¨¨’è¨œO]¨º¨¨  ¨  ¨”J¨  ƒˆƒLƒqãä’·ãäãäìŒHŒ–SŒ“y"
	"Œ“àŒ“úŒ–ŞŒbŒ•DŒ“áŒŒ—RŒ˜¡Œ”¼Œ‰iŒšğŒŒõŒ›İŒ¿Œ’C"
	"Œ•áŒ–éŒ¶Œ“TŒ”ÚŒ”ñŒ•{Œ›óŒŒšŒvŒ¯Œ›İŒäkŒMŒé­Œê"
	"ŒÓŒ—·Œ›İŒš ŒáŒ’‚ŒºŒäŒvŒ”ÔŒ‘²Œ‘\Œ¶ŒŒ¾Œ“aŒç„"
	"ŒŠåŒ™@ŒÄŒùŒ‰Œ‘lŒŠ‹Œá¸Œ‘ …“÷b•èº  Š`‰Pl‰PœO‰Pt"
	"‰P’j‰P˜Z‰Pè°‡ããäq•áMŒöM•ûM‰ÂM—RM›tM‘qM™ÕMáM‘‚M“¶"
	"M‹`Mš¥M–ÖM˜DMá¸MŒËŠv¯–LF›£  ++˜§Ù  –S  Œ³  Ÿt  Ù  •ª  "
	"ˆÈ  ‹  ‹å  ™f  Š  “~  •ê  –ˆ  ”Ã  •t  •½  •ï  ‰K  –Ú  ––  —§  "
	"ˆö  ‰ñ  Še  ›ô  é  {  ”@  ‘¶  ‡  •š  äŠ  –¼  “ˆˆÊ  ‰„  ‰ä  "
	"ŠÜ  šñ  ›İ  Œà  ¹  •  šá  —]’ƒ“e  “¤  ”E  G  —˜  —Ç  ‰‚  é  "
	"’†‰¤©  f  ‘²  ¼  È  Â  ›â  ’·  ”g  ”ñ  Ÿ¢ä™–A  •ü  ‘åœOäk  "
	"—½  —Ñ  ‚dƒR‰È  ™ö  ™ö  Š¥  ŒR  ŒÓ  ä  ’i  ’é  ”b  •Û  â[  –ñ  "
	"{  šD  ‰¥  ‘¾MŒ“  ‚  ä‹åÀ  ’˜˜Vã  C  J  `  È  ©  Ó  "
	"Õ  h  ê    Q  ‘`  ‘°  ‘Ñ  ™´  ‰Hœc™  w  êŸ  ‘  q  S  "
	"~  —P  ‰·  Ÿeèê‘\  ˜j“c‹›  –²€ãç  š¥  h  ˜K¼”÷  ç„  —a  —‹  "
	"—Å  ãÎÌÄ  ‘   º  –e  ãÎâ[Œ|·Ù  êt  ‰y  åD  ‰Ñ‹›•p  —Š  –åè°"
	"á¸  —´  ‘N  ã{  —…  ãµ‹Òsåg•¶†ŒÕåg‰˜’  ’ˆø’Œö’R’“l’“à"
	"’ƒL’ŠÃ’‹u’ŒÃ’Š’—R’—ß’äİ’•ï’‰ñ’ã’–}’‹¤’Œğ’é’˜V"
	"’‰„’Œ©’Œà’™¯’’C’™[’‰„’›t’—]’áT’—˜’ˆÕ’ˆ¶’Šª’Â’Í"
	"’ü’”ñ’“c’ˆİ’šD’‰È’ˆÕ’ŒÓ’c’ƒi’šç’G’™`’q’ä’ê†"
	"Œu  ’–»’˜Y’Â’»’“~’—¦’í’è°’Í’·’“°’–‹’”œ’˜K’êŸ"
	"’•i’”Ô’‰ğå™Œ‡’Œ¾’¹’ác’–z’  ’‰h’Ç’ù’t’æ³å³M’  "
	"’  ’  ’…ŒŒ‰NŒŒnsŒºsŒás  sè°ƒlœcˆ£  ¡ˆßˆ£  ƒl“úƒlpåÑ”C"
	"ƒl“àƒlšíƒlœcƒl’Uƒl›•ƒl”’ƒl”¼ƒl•ïŠ_•êˆßƒlŒ\ƒl•šã‰ºƒlsˆß™g‹ˆß"
	"ƒlŒNšáˆß—   åİŒTƒlˆÕ”ñˆßƒl”Úƒl‹ĞƒlÈƒlŒRƒlGƒl•Ûˆß  ‰·  ƒlJƒl‘Ş"
	"ƒlŒÕƒl‹­•\  ·ˆßƒlKƒl˜Kƒlšdƒl’Pƒlácç„ˆßƒlùƒlŠÄƒl‹gƒl•ÌƒleƒlèŒ"
	"ƒl§™k  æG‘æG•úæG”nŒ©  Œ©èŒ©›ŞŒ©ÒŒ©™`Œ©æ¯Œ©™iŒ©  Šo  ——  Œ©æÌ"
	"Œ©è°Šp‰ZŠpŸŠpŠp—rŠpäŠpå†Œ¾ƒgŒ¾ŒîŒ¾Š±Œ¾HŒ¾‰»Œ¾‰åŒ¾“àŒ¾‰ÂŒ¾ŒÃ"
	"Œ¾ŠŒ¾‘äŒ¾Œ¾  Œ¾ŠDŒ¾ŠëŒ¾@Œ¾{Œ¾éŒ¾’›Œ¾ãÎŒ¾–ˆŒ¾‰úŒ¾‹¶Œ¾Œ¾áT"
	"Œ¾ÑŒ¾›ŞŒ¾‹Œ¾à¥Œ¾‰PŒ¾’èŒ¾jŒ¾‰¹Œ¾ŠFŒ¾™öŒ¾èèŒ¾‹sŒ¾éŒ¾ŒRŒ¾•—Œ¾G"
	"Œ¾äkŒ¾šFŒ¾Š¦Œ¾‰vŒ¾™aŒ¾“cŒ¾•KŒ¾ÓŒ¾ŠÊŒ¾™½Œ¾ŠvŒ¾ºŒ¾¤Œ¾™ÖŒ¾”œŒ¾‰Ø"
	"Œ¾à¨Œ¾ŠôŒ¾–µŒ¾“oŒ¾‘ÖŒ¾‘ÖŒ¾‘Œ¾  Œ¾•iŒ¾ç„Œ¾KŒ¾Œ­Œ¾äoŒ¾æÌŒ¾‰è°’N"
	"Œ¾–ÆŒ¾åõŒ¾œ÷Œ¾è°Œ¾^’J‰åŠQ’J’J  R“¤“¤ˆ¶Œ˜“¤—H“¤•s—¹æ³Œæ³Òœc  "
	"æ¶Ëæ¶¢æ¶Šeæ¶‹xæ¶•Sæ¶—¢æ¶™Zæ¶”äæ¶”äæ¶”œŠLœ÷ŠL‚ŠL¡ŠL‘äŠLŸŠL“ó"
	"ŠL“óŠL–R”ƒ  ŠL  ŠLœû”ƒ  ŠL˜ÒŠLŠ¦ŠLŒ“ŠLêŠL·ŠLÂŠL^ŠL•ŠL–ÓŠLŒ¾"
	"ŠLá¶ŠLÄŠL•èŠL¯ŠL•iŠLæÌÔ”½ÔÒ”V  ‘–‚S‘–’¿‘–Ñ‘«x‘«~‘«•v‘«‰Á"
	"‘«û‘«Î‘«¸‘«”ç‘«Œ¢‘«Šë‘«–}‘«¯‘«æ‘«‹Ç‘«‘©‘«—Ç‘«‘–‘«‰Ê‘«‹‘«œû"
	"‘«’m‘«_‘«d‘«™`‘«—E‘«  ‘«Š¦‘«·‘«‘q‘«Ò‘«‰P‘«Ê‘«]‘«‘«@‘«—¼"
	"‘«•L‘«™Î‘«‘¸‘«”ü‘«•i‘«’˜‘«å†‘«ç„‘«Ä‘«šæ‘«¿‘«“A‘«o‘«–å‘«çW‘«ãá"
	"g‹|g–{g–LgSg”ügŠågœäÔ‚µÔ–ïÔ  Ô¸Ô‰ÂÔœcÔ®ÔŠÔŠe"
	"Ô›İÔ–ëÔæÔ–ÆÔáVÔ–”Ô—¼Ô—¼Ô•vÔ‘tÔ“cÔšçÔåÍÔ’Ô“WÔá"
	"ÔêÔ­Ô‹ªÔŠ´ÔùÔÙÔ—ğÔá¸h  ›™hh‘©h  hŒ¾ˆê  ’†  ™c  "
	"À  —R  ŠO  ¢  ‰ñ  @  –’  ¼  ‹  „  ˆò  Ñ  ’æ  ‰Î  ‰Y  —  "
	"ˆÏ  “y™Xh  ›ó  Š‰  ƒR–”c  U  “Ñ  ’å  ™`  “ì  ™i  ŒÕ  Â  “Ø  "
	"êŸ  ä@  Ò  ‰ğ  àh  äİ  Œƒ  •Ó  •Ó  —…  “Ô  ŠÃ  ‹u  ¢  ’æ  ’J  "
	"ŒË”b›t  ™ö  ä  ‰ñ  šd  •Ä‘C“Ñ’š“Ñ›—“ÑŸt“ÑŠÃ“Ñ‰Ñ“Ñ–¼“Ñˆç“Ñ’æ“ÑÌ"
	"“Ñ‘²“Ñ—Ñ“ÑM“Ñˆã“ÑM“Ñœc“Ñæ³“Ñ–L“Ñà“Ñåõ“Ñ  ‰Ñ—R‰ÑK—Ğ  ‹à“‹à”ª"
	"‹à”ª‹à—Í‹àn‹à³‹à–ç‹àÙ‹à‹Ò‹à­‹àÚ‹à‰N‹à”½‹àœø‹àŠÃ‹à‹‹àŒº‹à‹å"
	"‹à›€‹àˆÎ‹à“c‹à•ï‹àÎ“Bs‹àé‹à‘S‹àã‹à‰K‹àšñ‹àG‹àÑ‹àäŠ‹à˜±‹àˆ¶"
	"‹à‹‹àŒÅ‹àáV‹àœû‹àà¥‹à–”‹à–F‹à‰Ô‹à•‹à–”‹àc‹à™÷‹à™`‹àr‹à‰v‹à‚"
	"‹àáÁ‹à—e‹à‘—‹à­‹àŒ˜‹àa‹àh‹à›’‹à‘°‹à™Ö‹àœc‹à˜A‹à˜K‹àœ¦‹à‘‹à“Ö"
	"‹àêŸ‹à”Ô‹à˜µ‹àˆ£‹àè²‹àœ÷‹àœ÷‹àác‹àˆò‹à  ‹àšæ‹àœA‹àÙ‹à—¶‹à‘l‹àá¸"
	"‹àŒË‹àê‹àè°‹àãá‹à^‹à^‹à—…‹à…‹àÇ‹à  –åˆê–å‰º–åR–å•¶–å…–åb"
	"–ås–å‹Ê–åŒ\–å‹¤–å˜C–å‰—–å‰P–å‰‚–åˆ½–åŠˆ–åã–åL–åÒ–åj–å  –åá²"
	"–å”n–å  –åšd–å’B–åç„ç  –ï  Œ³  ~  ”ç  •S  —L  •¸  ‰P  šñ  ¡“y"
	"šñ  •à  õ  ‚  æ  c  ‰v  ˆõ  ‹S  ™@  ‹  ‰B  ”n  …  —´  ƒˆ…"
	"è®  ‚dbŠè°è°  –îè°›ôè°ƒlW‘²è°‰Jè°üè°‰J•ï‰JÑ‰J‰„‰JŸ‰J™Z‰J¨"
	"‰JŸ‰J”ñ‰J—Ñ‰J‰p‰J—¯‰Jˆú‰JU‰Jç„‰JÄ‰Jæ¼‰J‰y‰_‘ß‰J›Ş‰J—ğ‰_ˆ¤Âà¥"
	"”ñ–Ê•ï–ÊŒ©–Ê‰}Šv—ÍŠv³ŠvnŠv“àŠv‰›Šv’UŠv”çŠv––Šv•¸ŠvŒ\Šv–}Švã"
	"ŠvŒBŠv™±ŠvHŠv_Švä§Šv—pŠv’BŠv‘JŠv•Ìƒ†ƒèè‰P”ñ  ÄèêÄèê‰¹Œ©‰¹¢"
	"‰¹Ù˜´•ÅŒö•Å›İ•Åb•Å‹g•ÅŠÜ•Å“Ã•Å‰Ê•Å•F•Åv•Å˜·•Å“ú•Å•p”Úá¸•Åè°•Å"
	"ãá•Å‰º•——§•—•—‘ä•—‹ï•[•—•—•[Œ¢•—H“ÔHšîHŒğHŒüH‘ÃH•áH—]H‰P"
	"H–FHœûH‰ŠH›óHŒÓHŸeH‹SH—¯HMH‰¤H™ÖHšãH‹MHŠôHêŸH’F"
	"Håj‹ãññˆ½šç”n–”™q”n”n•¶”nl”nj”n›€”n‘ä”n“z”nˆå”nŒğ”nŠe”nB"
	"”nÛ”nZ”n—R”n‘´”nè°”n›ó”nG”nÇ”n”a”n™½”nQ”n•æ”n•[”n—İ”n‹ª”nêŸ"
	"”nK”n™@”næ”ná¸”n™b”nåõ”nè°”nˆ”n—íœŠ±œŸtœŠeœ”Úœ˜Kœå†œ¶"
	"œ–L‚•i’·œcé•–çé••ûé•–Ñé•™fé•¢é•šçé••¤é••ïé•‹Èé•‹gé•¼é•™Öé•{"
	"é•ˆ£é••oé•‘l||++é¦sé¦‹¤é¦™Zé¦¡é¦  ‹¥ƒqƒ™c”’‹S‹SŒ¢ˆÏ‹S‹Sã¦‹S—¼"
	"‹S  ‹S‰}‹›•û‹›“á‹›•½‹›•ï‹›Î‹›“~‹›ˆÀ‹›Šë‹›|‹›‹x‹›Œn‹›¹‹›Ñ‹›•á"
	"‹›—˜‹›Šó‹›áT‹›ˆÕ‹›™Z‹›©‹›áV‹›”ñ‹›Q‹›‰—‹›ŒÕ‹›”O‹›–”‹›™÷‹›c‹›v"
	"‹›U‹›t‹›¢‹›šç‹›j‹›ˆĞ‹›M‹›M‹›  ‹›t‹›—¯‹›_‹›N‹›Â‹›Í‹›•["
	"‹›Šì‹›‘\‹›–L‹›—{‹›á¸’¹™{’¹™{“ü’¹‰å’¹™Ê’¹™Ê’¹’¹ší›—’¹ŒŠ’¹‰›’¹’¹‰Î"
	"ŒÃ’¹’¹Š’¹’¹›€—ß’¹Œğ’¹‡’¹–´’¹s’¹¨’¹‰ä’¹‰ä’¹Šp’¹Œ’¹›Ş’¹ŠL’¹"
	"Ì’¹‹’¹“Œ’¹“Œ’¹”Ú’¹–é’¹™ö’¹ŒR’¹’¹  ‰H’¹’¹  –~’¹œ’¹ã’¹Ò’¹áÁ’¹"
	"^’¹ŠÊ’¹·’¹’¹–µ’¹Å’¹”Ô’¹˜µ’¹’¹  ‰d’¹è°’¹…’¹è}êb™÷êbŠÄ­  "
	"­å­•Ä­Œà­‘´­‹Û    ­Ë˜Òšçêm•v”•vêm  êm•ï–ƒ”ñ‰©  ‰Ñ–Üêtè"
	"êt‹×•¡•o•è•—c•‹g•‹•®•‰¹•”÷•‰}•æÌ‹Ğ  êƒ  êƒ•áèê  "
	"Âê†Çê†“¤”çŒÛ“~áV—p‘l—R•@Š±Ä  •  êƒqê‹åêŠê¢ê—ßê¯êŒ_"
	"êŒáê‘«ê‰®êâZê™ö‡—´‹T  ‡˜Ó‹Ä  áÁ  ƒ^ŠÊƒ^ŠÊâg  b–¤";

/*
 * Š¿šŒğ‚¶‚è•¶‚Ì•¶š‚ğÁ‚·
 */
static int
kanji2index(c1, c2)
unsigned char c1, c2;
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#if (('')==0x8181)
	/* SJIS */
	if (c1 >= 0xe0) c1 -= 0x40;
	c1 -= 0x81;
	if (c2 >= 0x80) c2--;
	c2 -= 0x40;
	return ((int)c1 * (0xfc - 0x40 + 1 - 1)) + (int)c2;
#else
	/* EUC */
    	return (((int)c1 & 0x7f) - 0x21) * (0x7e - 0x21 + 1) +
	       (((int)c2 & 0x7f) - 0x21);
#endif
#pragma GCC diagnostic pop
}

int
jrubout(engr, nxt, use_rubout, select_rnd)
     char *engr;
     int nxt;
     int use_rubout;
     int select_rnd;
{
    int j;
    unsigned char *s;
    const unsigned char *p;

    if(is_kanji2(engr, nxt)){
	return 1;
      }

    s = (unsigned char *)&engr[nxt];
    if (*s == ' ') return 1;

    if(!is_kanji1(engr, nxt)) return 0;

    j = kanji2index(engr[nxt], engr[nxt + 1]);
    if (j >= 0x0000 && j <= 0x02B1) p = &ro0[j << 2];
    else if (j >= 0x0582 && j <= 0x1116) p = &ro1[(j - 0x0582) << 2];
    else if (j >= 0x1142 && j <= 0x1E7F) p = &ro2[(j - 0x1142) << 2];
    else p = "H";
    
    if (p[2] != ' ' || p[3] != ' ') p += select_rnd * 2;
    engr[nxt] = *p++;
    engr[nxt + 1] = *p;

    return 1;
}

static struct trans_verb trans_verb_list[] = {
    {"adjust",	"‚Ç‚ê", "‚ğ", "’²®‚·‚é"},
    {"call",	"‚Ç‚ê", "‚ğ", "ŒÄ‚Ô"},
    {"charge",	"‚Ç‚ê", "‚ğ", "[“U‚·‚é"},
    {"dip into", "‚Ç‚ê", "‚É", "Z‚·"},
    {"dip",	"‚Ç‚ê", "‚ğ", "Z‚·"},
    {"drink",	"‚Ç‚ê", "‚ğ", "ˆù‚Ş"},
    {"drop",	"‚Ç‚ê", "‚ğ", "’u‚­"},
    {"eat",	"‚Ç‚ê", "‚ğ", "H‚×‚é"},
    {"grease",	"‚Ç‚ê", "‚É", "“h‚é"},
    {"identify", "‚Ç‚ê", "‚ğ", "¯•Ê‚·‚é"},
    {"invoke",	"‚Ç", "‚Ì", "–‚—Í‚ğg‚¤"},
    {"name",	"‚Ç‚ê", "‚ğ", "–¼‚Ã‚¯‚é"},
    {"put on",	"‚Ç‚ê", "‚ğ", "g‚É‚Â‚¯‚é"},
    {"read",	"‚Ç‚ê", "‚ğ", "“Ç‚Ş"},
    {"ready",	"‚Ç‚ê", "‚ğ", "‘•“U‚·‚é"},
    {"remove",	"‚Ç‚ê", "‚ğ", "‚Í‚¸‚·"},
    {"rub",	"‚Ç‚ê", "‚ğ", "‚±‚·‚é"},
    {"rub on the stone", "‚Ç‚ê", "‚ğ", "Î‚Å‚±‚·‚é"},
    {"sacrifice", "‚Ç‚ê", "‚ğ", "•ù‚°‚é"},
    {"take off", "‚Ç", "‚Ì", "‘•”õ‚ğ‰ğ‚­"},
    {"throw",	"‚Ç‚ê", "‚ğ", "“Š‚°‚é"},
    {"tin",	"‚Ç‚ê", "‚ğ", "ŠÊ‹l‚ß‚É‚·‚é"},
    {"untrap with", "‚Ç‚ê", "‚ğ", "g‚Á‚Ä‰ğœ‚·‚é"},
    {"use or apply", "‚Ç‚ê", "‚ğ", "g‚¤"},
    {"wear",	"‚Ç‚ê", "‚ğ", "g‚É‚Â‚¯‚é"},
    {"write on", "‚Ç‚ê", "‚É", "‘‚­"},
    {"write with", "‚Ç‚ê", "‚ğ", "g‚Á‚Ä‘‚­"},
    {"zap",	"‚Ç‚ê", "‚ğ", "U‚è‚©‚´‚·"},
    {(void *)0, (void *)0},
};

static struct trans_verb wield = 
    {"wield",	"‚Ç‚ê", "‚ğ", (void *)0}
;

static struct trans_verb dummyverb = 
    {(void *)0,	"‚Ç‚ê", "‚ğ", (void *)0}
;

struct trans_verb
*trans_verb(const char *en){
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

    return &dummyverb;;
}
