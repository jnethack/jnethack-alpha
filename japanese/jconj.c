/*
**
**	$Id: jconj.c,v 1.11 2005-06-10 22:02:04 argrath Exp $
**
*/

/* Copyright (c) Issei Numata 1994-2000 */
/* JNetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "hack.h"

#define EUC	0
#define SJIS	1
#define JIS	2

/* internal kcode */
/* IC=0 EUC */
/* IC=1 SJIS */
#define IC ((unsigned char)("Š¿"[0])==0x8a)

#define J_A	0
#define J_KA	(1*5)
#define J_SA	(2*5)
#define J_TA	(3*5)
#define J_NA	(4*5)
#define J_HA	(5*5)
#define J_MA	(6*5)
#define J_YA	(7*5)
#define J_RA	(8*5)
#define J_WA	(9*5)

#define J_GA	(10*5)
#define J_ZA	(11*5)
#define J_DA	(12*5)
#define J_BA	(13*5)
#define J_PA	(14*5)

static unsigned char hira_tab[][2]={
    {0xa4, 0xa2}, {0xa4, 0xa4}, {0xa4, 0xa6}, {0xa4, 0xa8}, {0xa4, 0xaa}, 
    {0xa4, 0xab}, {0xa4, 0xad}, {0xa4, 0xaf}, {0xa4, 0xb1}, {0xa4, 0xb3}, 
    {0xa4, 0xb5}, {0xa4, 0xb7}, {0xa4, 0xb9}, {0xa4, 0xbb}, {0xa4, 0xbd}, 
    {0xa4, 0xbf}, {0xa4, 0xc1}, {0xa4, 0xc4}, {0xa4, 0xc6}, {0xa4, 0xc8}, 
    {0xa4, 0xca}, {0xa4, 0xcb}, {0xa4, 0xcc}, {0xa4, 0xcd}, {0xa4, 0xce}, 
    {0xa4, 0xcf}, {0xa4, 0xd2}, {0xa4, 0xd5}, {0xa4, 0xd8}, {0xa4, 0xdb}, 
    {0xa4, 0xde}, {0xa4, 0xdf}, {0xa4, 0xe0}, {0xa4, 0xe1}, {0xa4, 0xe2}, 
    {0xa4, 0xe4}, {0xa4, 0xa4}, {0xa4, 0xe6}, {0xa4, 0xa8}, {0xa4, 0xe8}, 
    {0xa4, 0xe9}, {0xa4, 0xea}, {0xa4, 0xeb}, {0xa4, 0xec}, {0xa4, 0xed}, 
    {0xa4, 0xef}, {0xa4, 0xa4}, {0xa4, 0xa6}, {0xa4, 0xa8}, {0xa4, 0xaa}, 
    {0xa4, 0xac}, {0xa4, 0xae}, {0xa4, 0xb0}, {0xa4, 0xb2}, {0xa4, 0xb4}, 
    {0xa4, 0xb6}, {0xa4, 0xb8}, {0xa4, 0xba}, {0xa4, 0xbc}, {0xa4, 0xbe}, 
    {0xa4, 0xc0}, {0xa4, 0xc2}, {0xa4, 0xc5}, {0xa4, 0xc7}, {0xa4, 0xc9}, 
    {0xa4, 0xd0}, {0xa4, 0xd3}, {0xa4, 0xd6}, {0xa4, 0xd9}, {0xa4, 0xdc}, 
    {0xa4, 0xd1}, {0xa4, 0xd4}, {0xa4, 0xd7}, {0xa4, 0xda}, {0xa4, 0xdd},
};

#define FIFTH	0 /* ŒÜ’i */
#define UPPER	1 /* ãˆê’i */
#define LOWER	2 /* ‰ºˆê’i */
#define SAHEN	3 /* ƒT•Ï */
#define KAHEN	4 /* ƒJ•Ï */

#define NORMAL	0 /* ‚ ‚¯‚é¨`‚½ */
#define SOKUON	1 /* ‚Þ¨`‚ñ‚¾ */
#define HATSUON	2 /* ‘Å‚Â¨`‚Á‚½ */
#define ION	3 /* •‚‚­¨`‚¢‚½ */

struct _jconj_tab {
    const char *main;
    int column;
/* 0: fifth conj. 1:upper conj. 2:lower conj. 3:SAHEN 4:KAHEN */
    int katsuyo_type;
/* 0: normal 1: sokuon 2: hatson 3: ion */
    int onbin_type;
} jconj_tab[] = {
/* ‚  */
    {"‚ ‚¯‚é", J_KA, LOWER, NORMAL},
    {"ŠJ‚¯‚é", J_KA, LOWER, NORMAL},
    {"‚¢‚ê‚é", J_RA, LOWER, NORMAL},
    {"“ü‚ê‚é", J_RA, LOWER, NORMAL},
    {"•‚‚­", J_KA, FIFTH, ION},
    {"‘Å‚Â", J_TA, FIFTH, HATSUON},
    {"’u‚­", J_KA, FIFTH, ION},
    {"”[‚ß‚é", J_MA, LOWER, NORMAL},
    {"—Ž‚¿‚é", J_TA, UPPER, NORMAL},
    {"—Ž‚·", J_SA, FIFTH, NORMAL},
    {"‚¨‚Ì‚Ì‚­", J_KA, FIFTH, ION},
/* ‚© */
    {"‘‚­", J_KA, FIFTH, ION},
    {"‚©‚¯‚é", J_KA, UPPER, NORMAL},
    {"‚©‚Ô‚é", J_RA, FIFTH, HATSUON},
    {"\‚¦‚é", J_A, LOWER, NORMAL},
    {"Šš‚Ý‚Â‚­", J_KA, FIFTH, ION},
    {"‚Þ", J_MA, FIFTH, SOKUON},
    {"’…‚é", J_KA, UPPER, NORMAL},
    {"—ˆ‚é", J_KA, KAHEN, NORMAL}, 
    {"Ó‚­", J_KA, FIFTH, ION}, 
    {"‰Á‚¦‚é", J_A, LOWER, NORMAL},
    {"‚±‚·‚é", J_RA, FIFTH, HATSUON},
    {"‚±‚Ü‚·", J_SA, FIFTH, NORMAL},
    {"ž‚Þ", J_MA, FIFTH, SOKUON},
    {"ŽE‚·", J_SA, FIFTH, NORMAL},
    {"‰ó‚·", J_SA, FIFTH, NORMAL},
/* ‚³ */
    {"•ù‚°‚é", J_KA, LOWER, NORMAL},
    {"ŽK‚Ñ‚é", J_BA, UPPER, NORMAL},
    {"Ž€‚Ê", J_NA, FIFTH, SOKUON},
    {"ŠŠ‚é", J_RA, FIFTH, HATSUON},
    {"‚·‚é", J_SA, SAHEN, NORMAL}, 
/* ‚½ */
    {"‚½‚¶‚ë‚®", J_GA, FIFTH, ION},
    {"o‚·", J_SA, FIFTH, NORMAL},
    {"H‚×‚é", J_HA, LOWER, NORMAL}, 
    {"Žg‚¤", J_WA, FIFTH, HATSUON},
    {"‚Â‚¯‚é", J_KA, LOWER, NORMAL},
    {"‚Â‚Ü‚¸‚­", J_KA, FIFTH, ION},
    {"o‚é", J_NA, LOWER, NORMAL},
    {"‰ð‚­", J_KA, FIFTH, ION},
    {"—n‚¯‚é", J_KA, LOWER, NORMAL},
    {"‚Æ‚Î‚·", J_SA, FIFTH, NORMAL},
    {"”ò‚Ô", J_BA, FIFTH, SOKUON},
    {"Žæ‚é", J_RA, FIFTH, HATSUON},
/* ‚È */
    {"“Š‚°‚é", J_GA, LOWER, NORMAL},
    {"–¼‚Ã‚¯‚é", J_KA, LOWER, NORMAL},
    {"ˆ¬‚é", J_RA, FIFTH, HATSUON},
    {"’E‚®", J_GA, FIFTH, ION},
    {"”G‚ç‚·", J_SA, FIFTH, NORMAL},
    {"“h‚é", J_RA, FIFTH, HATSUON},
    {"ˆù‚Þ", J_MA, FIFTH, SOKUON},
/* ‚Í */
    {"‚Í‚¢‚¸‚é", J_RA, FIFTH, HATSUON},
    {"—š‚­", J_KA, FIFTH, ION},
    {"‚Í‚³‚Þ", J_MA, FIFTH, SOKUON},
    {"‚Í‚¸‚·", J_SA, FIFTH, NORMAL},
    {"ŠO‚·", J_SA, FIFTH, NORMAL},
    {"‚Í‚ß‚é", J_MA, UPPER, NORMAL},
    {"Z‚·", J_SA, FIFTH, NORMAL},
    {"‚Ð‚Á‚©‚¯‚é", J_KA, LOWER, NORMAL},
    {"E‚¤", J_WA, FIFTH, HATSUON},
    {"“¥‚Þ", J_MA, FIFTH, SOKUON},
    {"U‚è‚©‚´‚·", J_SA, FIFTH, NORMAL},
    {"Œ@‚é", J_RA, FIFTH, HATSUON},
/* ‚Ü */
    {"Šª‚­", J_KA, FIFTH, ION},
    {"Žç‚é", J_RA, FIFTH, HATSUON},
    {"‰ñ‚·", J_SA, FIFTH, NORMAL},
    {"g‚É‚Â‚¯‚é", J_KA, LOWER, NORMAL},
    {"Ž‚Â", J_TA, FIFTH, HATSUON},
/* ‚â */
    {"Ä‚­", J_KA, FIFTH, ION},
    {"ŒÄ‚Ô", J_BA, FIFTH, SOKUON},
    {"“Ç‚Þ", J_MA, FIFTH, SOKUON},
    {"‚æ‚ë‚ß‚­", J_KA, FIFTH, ION},
/* ‚ç */
/* ‚í */
    {(void*)0, 0, 0, 0},
};

extern unsigned char *e2sj(unsigned char *s);
extern unsigned char *sj2e(unsigned char *s);

/*
**	conjection verb word
**
**	Example
**	arg1	arg2	result
**	’E‚®	‚È‚¢	’E‚ª‚È‚¢
**	’E‚®	‚½	’E‚¢‚¾
**
*/
static char *
jconjsub( tab, jverb, sfx )
     struct _jconj_tab *tab;
     char *jverb;
     char *sfx;
{
    int len;
    unsigned char *p;
    static unsigned char tmp[1024];

    len = strlen(jverb);
    strcpy((char *)tmp, jverb );

    if(!strncmp(sfx, "‚Æ", 2)){
	strcat((char *)tmp, sfx);
	return (char *)tmp;
    }

    switch( tab->katsuyo_type ){
      case FIFTH:
	p = tmp + (len - 2);
	if(!strncmp(sfx, "‚È", 2)){
	    if(!IC){
		p[0] = 0xa4;
		p[1] = hira_tab[tab->column][1];
	    } else {
	      memcpy(p, e2sj(hira_tab[tab->column]), 2);
	    }

	    strcpy((char *)p + 2, sfx);
	    break;
	}
	else if(!strncmp(sfx, "‚½", 2) || !strncmp(sfx, "‚Ä", 2)){
	    switch( tab->onbin_type ){
	      case NORMAL:
		if(!IC){
		    p[1] = hira_tab[tab->column + 1][1];
		} else {
		    memcpy(p, e2sj(hira_tab[tab->column + 1]), 2);
		}
		break;
	      case SOKUON:
		if(!IC){
		    p[1] = 0xf3;
		} else {
		    memcpy(p, "‚ñ", 2);
		}
		break;
	      case HATSUON:
		if(!IC){
		    p[1] = 0xc3;
		} else {
		    memcpy(p, "‚Á", 2);
		}
		break;
	      case ION:
		if(!IC){
		    p[1] = 0xa4;
		} else {
		    memcpy(p, "‚¢", 2);
		}
		break;
	    }
	    strcpy((char *)p + 2, sfx);
	    if(tab->onbin_type == SOKUON ||
	       (tab->onbin_type == ION && tab->column >= J_GA)){
		if(!IC){
		  ++p[3];
		} else {
		  ++p[3];
		}
/*	  memcpy(p+2, e2sj(sj2e(p+2)+1), 2);*//* sj2e() returns ptr to char* */
	    }
	    break;
	}
	else if(!strncmp(sfx, "‚Î", 2)){
	    if(!IC){
		p[1] = hira_tab[tab->column + 3][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 3]), 2);
	    }
	    strcpy((char *)p + 2, sfx);
	}
	else if(!strncmp(sfx, "‚ê", 2)){
	    if(!IC){
		p[1]=hira_tab[tab->column + 3][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 3]), 2);
	    }
	    strcpy((char *)p + 2, sfx + 2);
	}
	else if(!strncmp(sfx, "‚Ü", 2)) {
	    if(!IC){
		p[1] = hira_tab[tab->column + 1][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 1]), 2);
	    }
	    strcpy((char *)p + 2, sfx);
	    break;
	}
	else if(!strncmp(sfx, "‚æ", 2)) {
	    if(!IC){
		p[1] = hira_tab[tab->column + 4][1];
	    } else {
		memcpy(p, e2sj(hira_tab[tab->column + 4]), 2);
	    }
	    strcpy((char *)p + 2, sfx + 2);
	    break;
	}
	break;
      case LOWER:
      case UPPER:
      case KAHEN:
	p = tmp + (len - 2);
	if(!strncmp(sfx, "‚Î", 2)){
	    strcpy((char *)p, "‚ê");
	    strcpy((char *)p + 2, sfx);
	}
	else if(!strncmp(sfx, "‚ê", 2) && tab->katsuyo_type == LOWER){
	    strcpy((char *)p, "‚ç");
	    strcpy((char *)p + 2, sfx);
	}
	else
	  strcpy((char *)p, sfx);
	break;
      case SAHEN:
	p = tmp + (len - 4);
	if(!strncmp(sfx, "‚È", 2) ||
	   !strncmp(sfx, "‚Ü", 2) ||
	   !strncmp(sfx, "‚½", 2) ||
	   !strncmp(sfx, "‚Ä", 2) ||
	   !strncmp(sfx, "‚æ", 2)){
	    strcpy((char *)p, "‚µ");
	    strcpy((char *)p + 2, sfx);
	}
	else if(!strncmp(sfx, "‚Î", 2) || !strncmp(sfx, "‚ê‚Î", 4)){
	    strcpy((char *)p, "‚·‚ê‚Î");
	}
	break;
    }
    return (char *)tmp;
}

/* “®ŽŒ‚Ì•Ï‰» */
const char *
jconj( jverb, sfx )
     const char *jverb;
     const char *sfx;
{
    struct _jconj_tab *tab;
    int len;

    len = strlen(jverb);
    for( tab = jconj_tab; tab->main != (void*)0; ++tab){
	if(!strcmp(jverb, tab->main)){
	    return jconjsub(tab, jverb, sfx);
	}
    }

    for( tab = jconj_tab; tab->main != (void*)0; ++tab){
	if(len - strlen(tab->main) > 0 &&
	   !strcmp(jverb + (len - strlen(tab->main)), tab->main)){
	    return jconjsub(tab, jverb, sfx);
	}
    }

#ifdef JAPANESETEST
    fprintf( stderr, "I don't know such word \"%s\"\n");
#endif
    return jverb;
}

/* ‰Â”\ */
const char *
jcan(jverb)
     const char *jverb;
{
    const char *ret;
    static unsigned char tmp[1024];

    int len = strlen(jverb);
    if(!strcmp(jverb + len - 4, "‚·‚é")){
	strncpy(tmp, jverb, len - 4);
	strcpy(tmp + len - 4, "‚Å‚«‚é");
	return tmp;
    } else {
	ret = jconj(jverb, "‚ê‚é");
	return ret;
    }
}

/* •s‰Â”\ */
const char *
jcannot(jverb)
     const char *jverb;
{
    static unsigned char tmp[1024];

    int len = strlen(jverb);
    if(!strcmp(jverb + len - 4, "‚·‚é")){
	strncpy(tmp, jverb, len-4);
	strcpy(tmp +len-4, "‚Å‚«‚È‚¢");
	return tmp;
    } else {
	return jconj(jverb, "‚ê‚È‚¢");
    }
}

/* ‰ß‹Ž */
const char *
jpast(jverb)
     const char *jverb;
{
    return jconj(jverb, "‚½");
}

/* Œh‘Ì */
const char *
jpolite(jverb)
     const char *jverb;
{
    return jconj(jverb, "‚Ü‚·");
}


/*
**	conjection of adjective word
**
**	Example:
**
**	Œ`—eŽŒ“I—p–@	   •›ŽŒ“I—p–@
**
**	Ô‚¢		-> Ô‚­		(Œ`—eŽŒ)
**	ãY—í‚È		-> ãY—í‚É	(Œ`—e“®ŽŒ)
**	ãY—í‚¾		-> ãY—í‚É	(Œ`—e“®ŽŒ)
*/
const char *
jconj_adj( jadj )
     const char *jadj;
{
    int len;
    static unsigned char tmp[1024];

    strcpy((char *)tmp, jadj);
    len = strlen((char *)tmp);

    if(!strcmp((char *)tmp + len - 2, "‚¢")){
	strcpy((char *)tmp + len - 2, "‚­");
    } else if(!strcmp((char *)tmp + len - 2, "‚¾") ||
	      !strcmp((char *)tmp + len - 2, "‚È") ||
	      !strcmp((char *)tmp + len - 2, "‚Ì")){
	strcpy((char *)tmp + len - 2, "‚É");
    }

    return (char *)tmp;
}


#ifdef JAPANESETEST
unsigned char
*e2sj(unsigned char *s)
{
    return *s;
}

unsigned char
*sj2e(unsigned char *s)
{
    return *s;
}

void
main()
{
    struct _jconj_tab *tab;

    for(tab = jconj_tab; tab->main != (void*)0; ++tab){
	printf("%-10s ‚È‚¢ %s\n", tab->main, jconj(tab->main, "‚È‚¢"));
	printf("%-10s ‚Ü‚· %s\n", tab->main, jconj(tab->main, "‚Ü‚·"));
	printf("%-10s ‚½   %s\n", tab->main, jconj(tab->main, "‚½"));
	printf("%-10s ‚ê‚Î %s\n", tab->main, jconj(tab->main, "‚ê‚Î"));
	printf("%-10s ‚Æ‚« %s\n", tab->main, jconj(tab->main, "‚Æ‚«"));
	printf("%-10s ‚æ‚¤ %s\n", tab->main, jconj(tab->main, "‚æ‚¤"));
	printf("%-10s %s\n", tab->main, jcan(tab->main));
	printf("%-10s %s\n", tab->main, jcannot(tab->main));
    }
    printf("%s\n", jconj("“O–é‚Ånethack‚Ì–|–ó‚ð‚·‚é", "‚È‚¢"));
    printf("%s\n", jconj("“O–é‚Ånethack‚Ì–|–ó‚ð‚·‚é", "‚Ü‚·"));
    printf("%s\n", jconj("“O–é‚Ånethack‚Ì–|–ó‚ð‚·‚é", "‚½"));
    printf("%s\n", jconj("“O–é‚Ånethack‚Ì–|–ó‚ð‚·‚é", "‚ê‚Î"));
    printf("%s\n", jconj("“O–é‚Ånethack‚Ì–|–ó‚ð‚·‚é", "‚Æ‚«"));
}
#endif
