/* NetHack 3.6	cmd.c	$NHDT-Date: 1446975462 2015/11/08 09:37:42 $  $NHDT-Branch: master $:$NHDT-Revision: 1.206 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2016            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "func_tab.h"

#ifdef ALTMETA
STATIC_VAR boolean alt_esc = FALSE;
#endif

struct cmd Cmd = { 0 }; /* flag.h */

extern const char *hu_stat[];  /* hunger status from eat.c */
extern const char *enc_stat[]; /* encumbrance status from botl.c */

#ifdef UNIX
/*
 * Some systems may have getchar() return EOF for various reasons, and
 * we should not quit before seeing at least NR_OF_EOFS consecutive EOFs.
 */
#if defined(SYSV) || defined(DGUX) || defined(HPUX)
#define NR_OF_EOFS 20
#endif
#endif

#define CMD_TRAVEL (char) 0x90
#define CMD_CLICKLOOK (char) 0x8F

#ifdef DEBUG
extern int NDECL(wiz_debug_cmd_bury);
extern int NDECL(wiz_debug_cmd_traveldisplay);
#endif

#ifdef DUMB /* stuff commented out in extern.h, but needed here */
extern int NDECL(doapply);            /**/
extern int NDECL(dorub);              /**/
extern int NDECL(dojump);             /**/
extern int NDECL(doextlist);          /**/
extern int NDECL(enter_explore_mode); /**/
extern int NDECL(dodrop);             /**/
extern int NDECL(doddrop);            /**/
extern int NDECL(dodown);             /**/
extern int NDECL(doup);               /**/
extern int NDECL(donull);             /**/
extern int NDECL(dowipe);             /**/
extern int NDECL(docallcnd);          /**/
extern int NDECL(dotakeoff);          /**/
extern int NDECL(doremring);          /**/
extern int NDECL(dowear);             /**/
extern int NDECL(doputon);            /**/
extern int NDECL(doddoremarm);        /**/
extern int NDECL(dokick);             /**/
extern int NDECL(dofire);             /**/
extern int NDECL(dothrow);            /**/
extern int NDECL(doeat);              /**/
extern int NDECL(done2);              /**/
extern int NDECL(vanquished);         /**/
extern int NDECL(doengrave);          /**/
extern int NDECL(dopickup);           /**/
extern int NDECL(ddoinv);             /**/
extern int NDECL(dotypeinv);          /**/
extern int NDECL(dolook);             /**/
extern int NDECL(doprgold);           /**/
extern int NDECL(doprwep);            /**/
extern int NDECL(doprarm);            /**/
extern int NDECL(doprring);           /**/
extern int NDECL(dopramulet);         /**/
extern int NDECL(doprtool);           /**/
extern int NDECL(dosuspend);          /**/
extern int NDECL(doforce);            /**/
extern int NDECL(doopen);             /**/
extern int NDECL(doclose);            /**/
extern int NDECL(dosh);               /**/
extern int NDECL(dodiscovered);       /**/
extern int NDECL(doclassdisco);       /**/
extern int NDECL(doset);              /**/
extern int NDECL(dotogglepickup);     /**/
extern int NDECL(dowhatis);           /**/
extern int NDECL(doquickwhatis);      /**/
extern int NDECL(dowhatdoes);         /**/
extern int NDECL(dohelp);             /**/
extern int NDECL(dohistory);          /**/
extern int NDECL(doloot);             /**/
extern int NDECL(dodrink);            /**/
extern int NDECL(dodip);              /**/
extern int NDECL(dosacrifice);        /**/
extern int NDECL(dopray);             /**/
extern int NDECL(dotip);              /**/
extern int NDECL(doturn);             /**/
extern int NDECL(doredraw);           /**/
extern int NDECL(doread);             /**/
extern int NDECL(dosave);             /**/
extern int NDECL(dosearch);           /**/
extern int NDECL(doidtrap);           /**/
extern int NDECL(dopay);              /**/
extern int NDECL(dosit);              /**/
extern int NDECL(dotalk);             /**/
extern int NDECL(docast);             /**/
extern int NDECL(dovspell);           /**/
extern int NDECL(dotele);             /**/
extern int NDECL(dountrap);           /**/
extern int NDECL(doversion);          /**/
extern int NDECL(doextversion);       /**/
extern int NDECL(doswapweapon);       /**/
extern int NDECL(dowield);            /**/
extern int NDECL(dowieldquiver);      /**/
extern int NDECL(dozap);              /**/
extern int NDECL(doorganize);         /**/
#endif                                /* DUMB */

static int NDECL(dosuspend_core); /**/

static int NDECL((*timed_occ_fn));

STATIC_PTR int NDECL(doprev_message);
STATIC_PTR int NDECL(timed_occupation);
STATIC_PTR int NDECL(doextcmd);
STATIC_PTR int NDECL(domonability);
STATIC_PTR int NDECL(dooverview_or_wiz_where);
STATIC_PTR int NDECL(dotravel);
STATIC_PTR int NDECL(doterrain);
STATIC_PTR int NDECL(wiz_wish);
STATIC_PTR int NDECL(wiz_identify);
STATIC_PTR int NDECL(wiz_map);
STATIC_PTR int NDECL(wiz_genesis);
STATIC_PTR int NDECL(wiz_where);
STATIC_PTR int NDECL(wiz_detect);
STATIC_PTR int NDECL(wiz_panic);
STATIC_PTR int NDECL(wiz_polyself);
STATIC_PTR int NDECL(wiz_level_tele);
STATIC_PTR int NDECL(wiz_level_change);
STATIC_PTR int NDECL(wiz_show_seenv);
STATIC_PTR int NDECL(wiz_show_vision);
STATIC_PTR int NDECL(wiz_smell);
STATIC_PTR int NDECL(wiz_mon_polycontrol);
STATIC_PTR int NDECL(wiz_show_wmodes);
STATIC_DCL void NDECL(wiz_map_levltyp);
STATIC_DCL void NDECL(wiz_levltyp_legend);
#if defined(__BORLANDC__) && !defined(_WIN32)
extern void FDECL(show_borlandc_stats, (winid));
#endif
#ifdef DEBUG_MIGRATING_MONS
STATIC_PTR int NDECL(wiz_migrate_mons);
#endif
STATIC_DCL int FDECL(size_monst, (struct monst *));
STATIC_DCL int FDECL(size_obj, (struct obj *));
STATIC_DCL void FDECL(count_obj, (struct obj *, long *, long *,
                                  BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(obj_chain, (winid, const char *, struct obj *,
                                  long *, long *));
STATIC_DCL void FDECL(mon_invent_chain, (winid, const char *, struct monst *,
                                         long *, long *));
STATIC_DCL void FDECL(mon_chain, (winid, const char *, struct monst *,
                                  long *, long *));
STATIC_DCL void FDECL(contained, (winid, const char *, long *, long *));
STATIC_PTR int NDECL(wiz_show_stats);
STATIC_DCL boolean FDECL(accept_menu_prefix, (int NDECL((*))));
#ifdef PORT_DEBUG
STATIC_DCL int NDECL(wiz_port_debug);
#endif
STATIC_PTR int NDECL(wiz_rumor_check);
STATIC_DCL char FDECL(cmd_from_func, (int NDECL((*))));
STATIC_PTR int NDECL(doattributes);
STATIC_PTR int NDECL(doconduct); /**/

STATIC_DCL void FDECL(enlght_line, (const char *, const char *, const char *,
                                    const char *));
STATIC_DCL char *FDECL(enlght_combatinc, (const char *, int, int, char *));
STATIC_DCL void FDECL(enlght_halfdmg, (int, int));
STATIC_DCL boolean NDECL(walking_on_water);
STATIC_DCL boolean FDECL(cause_known, (int));
STATIC_DCL char *FDECL(attrval, (int, int, char *));
STATIC_DCL void FDECL(background_enlightenment, (int, int));
STATIC_DCL void FDECL(characteristics_enlightenment, (int, int));
STATIC_DCL void FDECL(one_characteristic, (int, int, int));
STATIC_DCL void FDECL(status_enlightenment, (int, int));
STATIC_DCL void FDECL(attributes_enlightenment, (int, int));

static const char *readchar_queue = "";
static coord clicklook_cc;

STATIC_DCL char *NDECL(parse);
STATIC_DCL boolean FDECL(help_dir, (CHAR_P, const char *));

STATIC_PTR int
doprev_message(VOID_ARGS)
{
    return nh_doprev_message();
}

/* Count down by decrementing multi */
STATIC_PTR int
timed_occupation(VOID_ARGS)
{
    (*timed_occ_fn)();
    if (multi > 0)
        multi--;
    return multi > 0;
}

/* If you have moved since initially setting some occupations, they
 * now shouldn't be able to restart.
 *
 * The basic rule is that if you are carrying it, you can continue
 * since it is with you.  If you are acting on something at a distance,
 * your orientation to it must have changed when you moved.
 *
 * The exception to this is taking off items, since they can be taken
 * off in a number of ways in the intervening time, screwing up ordering.
 *
 *      Currently:      Take off all armor.
 *                      Picking Locks / Forcing Chests.
 *                      Setting traps.
 */
void
reset_occupations()
{
    reset_remarm();
    reset_pick();
    reset_trapset();
}

/* If a time is given, use it to timeout this function, otherwise the
 * function times out by its own means.
 */
void
set_occupation(fn, txt, xtime)
int NDECL((*fn));
const char *txt;
int xtime;
{
    if (xtime) {
        occupation = timed_occupation;
        timed_occ_fn = fn;
    } else
        occupation = fn;
    occtxt = txt;
    occtime = 0;
    return;
}

STATIC_DCL char NDECL(popch);

/* Provide a means to redo the last command.  The flag `in_doagain' is set
 * to true while redoing the command.  This flag is tested in commands that
 * require additional input (like `throw' which requires a thing and a
 * direction), and the input prompt is not shown.  Also, while in_doagain is
 * TRUE, no keystrokes can be saved into the saveq.
 */
#define BSIZE 20
static char pushq[BSIZE], saveq[BSIZE];
static NEARDATA int phead, ptail, shead, stail;

STATIC_OVL char
popch()
{
    /* If occupied, return '\0', letting tgetch know a character should
     * be read from the keyboard.  If the character read is not the
     * ABORT character (as checked in pcmain.c), that character will be
     * pushed back on the pushq.
     */
    if (occupation)
        return '\0';
    if (in_doagain)
        return (char) ((shead != stail) ? saveq[stail++] : '\0');
    else
        return (char) ((phead != ptail) ? pushq[ptail++] : '\0');
}

char
pgetchar() /* courtesy of aeb@cwi.nl */
{
    register int ch;

    if (!(ch = popch()))
        ch = nhgetch();
    return (char) ch;
}

/* A ch == 0 resets the pushq */
void
pushch(ch)
char ch;
{
    if (!ch)
        phead = ptail = 0;
    if (phead < BSIZE)
        pushq[phead++] = ch;
    return;
}

/* A ch == 0 resets the saveq.  Only save keystrokes when not
 * replaying a previous command.
 */
void
savech(ch)
char ch;
{
    if (!in_doagain) {
        if (!ch)
            phead = ptail = shead = stail = 0;
        else if (shead < BSIZE)
            saveq[shead++] = ch;
    }
    return;
}

/* here after # - now read a full-word command */
STATIC_PTR int
doextcmd(VOID_ARGS)
{
    int idx, retval;
    int NDECL((*func));

    /* keep repeating until we don't run help or quit */
    do {
        idx = get_ext_cmd();
        if (idx < 0)
            return 0; /* quit */

        func = extcmdlist[idx].ef_funct;
        if (iflags.menu_requested && !accept_menu_prefix(func)) {
/*JP
            pline("'m' prefix has no effect for this command.");
*/
            pline("'m'接頭辞はこのコマンドには無効．");
            iflags.menu_requested = FALSE;
        }
        retval = (*func)();
    } while (func == doextlist);

    return retval;
}

/* here after #? - now list all full-word commands */
int
doextlist(VOID_ARGS)
{
    register const struct ext_func_tab *efp;
    char buf[BUFSZ];
    winid datawin;

    datawin = create_nhwindow(NHW_TEXT);
    putstr(datawin, 0, "");
/*JP
    putstr(datawin, 0, "            Extended Commands List");
*/
    putstr(datawin, 0, "            拡張コマンド一覧");
    putstr(datawin, 0, "");
/*JP
    putstr(datawin, 0, "    Press '#', then type:");
*/
    putstr(datawin, 0, "    '#'を押したあとタイプせよ:");
    putstr(datawin, 0, "");

    for (efp = extcmdlist; efp->ef_txt; efp++) {
        Sprintf(buf, "    %-15s - %s.", efp->ef_txt, efp->ef_desc);
        putstr(datawin, 0, buf);
    }
    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);
    return 0;
}

#ifdef TTY_GRAPHICS
#define MAX_EXT_CMD 50 /* Change if we ever have > 50 ext cmds */

/*
 * This is currently used only by the tty port and is
 * controlled via runtime option 'extmenu'.
 * ``# ?'' is counted towards the limit of the number of commands,
 * so we actually support MAX_EXT_CMD-1 "real" extended commands.
 *
 * Here after # - now show pick-list of possible commands.
 */
int
extcmd_via_menu()
{
    const struct ext_func_tab *efp;
    menu_item *pick_list = (menu_item *) 0;
    winid win;
    anything any;
    const struct ext_func_tab *choices[MAX_EXT_CMD + 1];
    char buf[BUFSZ];
    char cbuf[QBUFSZ], prompt[QBUFSZ], fmtstr[20];
    int i, n, nchoices, acount;
    int ret, biggest;
    int accelerator, prevaccelerator;
    int matchlevel = 0;

    ret = 0;
    cbuf[0] = '\0';
    biggest = 0;
    while (!ret) {
        i = n = 0;
        any = zeroany;
        /* populate choices */
        for (efp = extcmdlist; efp->ef_txt; efp++) {
            if (!matchlevel || !strncmp(efp->ef_txt, cbuf, matchlevel)) {
                choices[i] = efp;
                if ((int) strlen(efp->ef_desc) > biggest) {
                    biggest = strlen(efp->ef_desc);
                    Sprintf(fmtstr, "%%-%ds", biggest + 15);
                }
                if (++i > MAX_EXT_CMD) {
#if defined(BETA)
                    impossible(
      "Exceeded %d extended commands in doextcmd() menu; 'extmenu' disabled.",
                               MAX_EXT_CMD);
#endif /* BETA */
                    iflags.extmenu = 0;
                    return -1;
                }
            }
        }
        choices[i] = (struct ext_func_tab *) 0;
        nchoices = i;
        /* if we're down to one, we have our selection so get out of here */
        if (nchoices == 1) {
            for (i = 0; extcmdlist[i].ef_txt != (char *) 0; i++)
                if (!strncmpi(extcmdlist[i].ef_txt, cbuf, matchlevel)) {
                    ret = i;
                    break;
                }
            break;
        }

        /* otherwise... */
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        accelerator = prevaccelerator = 0;
        acount = 0;
        for (i = 0; choices[i]; ++i) {
            accelerator = choices[i]->ef_txt[matchlevel];
            if (accelerator != prevaccelerator || nchoices < (ROWNO - 3)) {
                if (acount) {
                    /* flush extended cmds for that letter already in buf */
                    Sprintf(buf, fmtstr, prompt);
                    any.a_char = prevaccelerator;
                    add_menu(win, NO_GLYPH, &any, any.a_char, 0, ATR_NONE,
                             buf, FALSE);
                    acount = 0;
                }
            }
            prevaccelerator = accelerator;
            if (!acount || nchoices < (ROWNO - 3)) {
                Sprintf(prompt, "%s [%s]", choices[i]->ef_txt,
                        choices[i]->ef_desc);
            } else if (acount == 1) {
/*JP
                Sprintf(prompt, "%s or %s", choices[i - 1]->ef_txt,
*/
                Sprintf(prompt, "%s または %s", choices[i-1]->ef_txt,
                        choices[i]->ef_txt);
            } else {
/*JP
                Strcat(prompt, " or ");
*/
                Strcat(prompt," または ");
                Strcat(prompt, choices[i]->ef_txt);
            }
            ++acount;
        }
        if (acount) {
            /* flush buf */
            Sprintf(buf, fmtstr, prompt);
            any.a_char = prevaccelerator;
            add_menu(win, NO_GLYPH, &any, any.a_char, 0, ATR_NONE, buf,
                     FALSE);
        }
/*JP
        Sprintf(prompt, "Extended Command: %s", cbuf);
*/
        Sprintf(prompt, "拡張コマンド: %s", cbuf);
        end_menu(win, prompt);
        n = select_menu(win, PICK_ONE, &pick_list);
        destroy_nhwindow(win);
        if (n == 1) {
            if (matchlevel > (QBUFSZ - 2)) {
                free((genericptr_t) pick_list);
#if defined(BETA)
                impossible("Too many chars (%d) entered in extcmd_via_menu()",
                           matchlevel);
#endif
                ret = -1;
            } else {
                cbuf[matchlevel++] = pick_list[0].item.a_char;
                cbuf[matchlevel] = '\0';
                free((genericptr_t) pick_list);
            }
        } else {
            if (matchlevel) {
                ret = 0;
                matchlevel = 0;
            } else
                ret = -1;
        }
    }
    return ret;
}
#endif /* TTY_GRAPHICS */

/* #monster command - use special monster ability while polymorphed */
STATIC_PTR int
domonability(VOID_ARGS)
{
    if (can_breathe(youmonst.data))
        return dobreathe();
    else if (attacktype(youmonst.data, AT_SPIT))
        return dospit();
    else if (youmonst.data->mlet == S_NYMPH)
        return doremove();
    else if (attacktype(youmonst.data, AT_GAZE))
        return dogaze();
    else if (is_were(youmonst.data))
        return dosummon();
    else if (webmaker(youmonst.data))
        return dospinweb();
    else if (is_hider(youmonst.data))
        return dohide();
    else if (is_mind_flayer(youmonst.data))
        return domindblast();
    else if (u.umonnum == PM_GREMLIN) {
        if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
            if (split_mon(&youmonst, (struct monst *) 0))
                dryup(u.ux, u.uy, TRUE);
        } else
/*JP
            There("is no fountain here.");
*/
            pline("ここには泉はない．");
    } else if (is_unicorn(youmonst.data)) {
        use_unicorn_horn((struct obj *) 0);
        return 1;
    } else if (youmonst.data->msound == MS_SHRIEK) {
/*JP
        You("shriek.");
*/
        You("金切り声をあげた．");
        if (u.uburied)
/*JP
            pline("Unfortunately sound does not carry well through rock.");
*/
            pline("残念ながら音は岩をうまく伝わらない．");
        else
            aggravate();
    } else if (youmonst.data->mlet == S_VAMPIRE)
        return dopoly();
    else if (Upolyd)
/*JP
        pline("Any special ability you may have is purely reflexive.");
*/
        pline("あなたの持っている特殊能力はどれも受動的だ．");
    else
/*JP
        You("don't have a special ability in your normal form!");
*/
        You("普段の姿での特殊能力を持っていない！");
    return 0;
}

int
enter_explore_mode(VOID_ARGS)
{
    if (wizard) {
/*JP
        You("are in debug mode.");
*/
        You("すでにデバッグモードだ．");
    } else if (discover) {
/*JP
        You("are already in explore mode.");
*/
        You("すでに探検モードだ．");
    } else {
#ifdef SYSCF
#if defined(UNIX)
        if (!sysopt.explorers || !sysopt.explorers[0]
            || !check_user_string(sysopt.explorers)) {
/*JP
            You("cannot access explore mode.");
*/
            You("探検モードにアクセスできない．");
            return 0;
        }
#endif
#endif
        pline(
/*JP
        "Beware!  From explore mode there will be no return to normal game.");
*/
        "警告！発見モードに入ったら通常モードには戻れない．");
        if (paranoid_query(ParanoidQuit,
/*JP
                           "Do you want to enter explore mode?")) {
*/
                           "発見モードに移りますか？")) {
            clear_nhwindow(WIN_MESSAGE);
/*JP
            You("are now in non-scoring explore mode.");
*/
            You("スコアがのらない発見モードに移行した．");
            discover = TRUE;
        } else {
            clear_nhwindow(WIN_MESSAGE);
/*JP
            pline("Resuming normal game.");
*/
            pline("通常モードを続ける．");
        }
    }
    return 0;
}

STATIC_PTR int
dooverview_or_wiz_where(VOID_ARGS)
{
    if (wizard)
        return wiz_where();
    else
        dooverview();
    return 0;
}

/* ^W command - wish for something */
STATIC_PTR int
wiz_wish(VOID_ARGS) /* Unlimited wishes for debug mode by Paul Polderman */
{
    if (wizard) {
        boolean save_verbose = flags.verbose;

        flags.verbose = FALSE;
        makewish();
        flags.verbose = save_verbose;
        (void) encumber_msg();
    } else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_wish)));
    return 0;
}

/* ^I command - reveal and optionally identify hero's inventory */
STATIC_PTR int
wiz_identify(VOID_ARGS)
{
    if (wizard) {
        iflags.override_ID = (int) cmd_from_func(wiz_identify);
        if (display_inventory((char *) 0, TRUE) == -1)
            identify_pack(0, FALSE);
        iflags.override_ID = 0;
    } else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_identify)));
    return 0;
}

/* ^F command - reveal the level map and any traps on it */
STATIC_PTR int
wiz_map(VOID_ARGS)
{
    if (wizard) {
        struct trap *t;
        long save_Hconf = HConfusion, save_Hhallu = HHallucination;

        HConfusion = HHallucination = 0L;
        for (t = ftrap; t != 0; t = t->ntrap) {
            t->tseen = 1;
            map_trap(t, TRUE);
        }
        do_mapping();
        HConfusion = save_Hconf;
        HHallucination = save_Hhallu;
    } else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_map)));
    return 0;
}

/* ^G command - generate monster(s); a count prefix will be honored */
STATIC_PTR int
wiz_genesis(VOID_ARGS)
{
    if (wizard)
        (void) create_particular();
    else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_genesis)));
    return 0;
}

/* ^O command - display dungeon layout */
STATIC_PTR int
wiz_where(VOID_ARGS)
{
    if (wizard)
        (void) print_dungeon(FALSE, (schar *) 0, (xchar *) 0);
    else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_where)));
    return 0;
}

/* ^E command - detect unseen (secret doors, traps, hidden monsters) */
STATIC_PTR int
wiz_detect(VOID_ARGS)
{
    if (wizard)
        (void) findit();
    else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_detect)));
    return 0;
}

/* ^V command - level teleport */
STATIC_PTR int
wiz_level_tele(VOID_ARGS)
{
    if (wizard)
        level_tele();
    else
/*JP
        pline("Unavailable command '%s'.",
*/
        pline("'%s'コマンドは使えない．",
              visctrl((int) cmd_from_func(wiz_level_tele)));
    return 0;
}

/* #monpolycontrol command - choose new form for shapechangers, polymorphees */
STATIC_PTR int
wiz_mon_polycontrol(VOID_ARGS)
{
    iflags.mon_polycontrol = !iflags.mon_polycontrol;
#if 0 /*JP*/
    pline("Monster polymorph control is %s.",
          iflags.mon_polycontrol ? "on" : "off");
#else
    pline("怪物の変化制御: %s",
          iflags.mon_polycontrol ? "オン" : "オフ");
#endif
    return 0;
}

/* #levelchange command - adjust hero's experience level */
STATIC_PTR int
wiz_level_change(VOID_ARGS)
{
    char buf[BUFSZ];
    int newlevel;
    int ret;

/*JP
    getlin("To what experience level do you want to be set?", buf);
*/
    getlin("経験レベルをいくつに設定しますか？", buf);
    (void) mungspaces(buf);
    if (buf[0] == '\033' || buf[0] == '\0')
        ret = 0;
    else
        ret = sscanf(buf, "%d", &newlevel);

    if (ret != 1) {
        pline1(Never_mind);
        return 0;
    }
    if (newlevel == u.ulevel) {
/*JP
        You("are already that experienced.");
*/
        You("すでにその経験レベルだ．");
    } else if (newlevel < u.ulevel) {
        if (u.ulevel == 1) {
/*JP
            You("are already as inexperienced as you can get.");
*/
            You("すでに可能な限りの最低の経験レベルだ．");
            return 0;
        }
        if (newlevel < 1)
            newlevel = 1;
        while (u.ulevel > newlevel)
/*JP
            losexp("#levelchange");
*/
            losexp("#levelchangeコマンドで");
    } else {
        if (u.ulevel >= MAXULEV) {
/*JP
            You("are already as experienced as you can get.");
*/
            You("すでに可能な限りの最大の経験レベルだ．");
            return 0;
        }
        if (newlevel > MAXULEV)
            newlevel = MAXULEV;
        while (u.ulevel < newlevel)
            pluslvl(FALSE);
    }
    u.ulevelmax = u.ulevel;
    return 0;
}

/* #panic command - test program's panic handling */
STATIC_PTR int
wiz_panic(VOID_ARGS)
{
/*JP
    if (yn("Do you want to call panic() and end your game?") == 'y')
*/
    if (yn("panic()関数を呼び出してゲームを終了させますか？") == 'y')
        panic("Crash test.");
    return 0;
}

/* #polyself command - change hero's form */
STATIC_PTR int
wiz_polyself(VOID_ARGS)
{
    polyself(1);
    return 0;
}

/* #seenv command */
STATIC_PTR int
wiz_show_seenv(VOID_ARGS)
{
    winid win;
    int x, y, v, startx, stopx, curx;
    char row[COLNO + 1];

    win = create_nhwindow(NHW_TEXT);
    /*
     * Each seenv description takes up 2 characters, so center
     * the seenv display around the hero.
     */
    startx = max(1, u.ux - (COLNO / 4));
    stopx = min(startx + (COLNO / 2), COLNO);
    /* can't have a line exactly 80 chars long */
    if (stopx - startx == COLNO / 2)
        startx++;

    for (y = 0; y < ROWNO; y++) {
        for (x = startx, curx = 0; x < stopx; x++, curx += 2) {
            if (x == u.ux && y == u.uy) {
                row[curx] = row[curx + 1] = '@';
            } else {
                v = levl[x][y].seenv & 0xff;
                if (v == 0)
                    row[curx] = row[curx + 1] = ' ';
                else
                    Sprintf(&row[curx], "%02x", v);
            }
        }
        /* remove trailing spaces */
        for (x = curx - 1; x >= 0; x--)
            if (row[x] != ' ')
                break;
        row[x + 1] = '\0';

        putstr(win, 0, row);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* #vision command */
STATIC_PTR int
wiz_show_vision(VOID_ARGS)
{
    winid win;
    int x, y, v;
    char row[COLNO + 1];

    win = create_nhwindow(NHW_TEXT);
    Sprintf(row, "Flags: 0x%x could see, 0x%x in sight, 0x%x temp lit",
            COULD_SEE, IN_SIGHT, TEMP_LIT);
    putstr(win, 0, row);
    putstr(win, 0, "");
    for (y = 0; y < ROWNO; y++) {
        for (x = 1; x < COLNO; x++) {
            if (x == u.ux && y == u.uy)
                row[x] = '@';
            else {
                v = viz_array[y][x]; /* data access should be hidden */
                if (v == 0)
                    row[x] = ' ';
                else
                    row[x] = '0' + viz_array[y][x];
            }
        }
        /* remove trailing spaces */
        for (x = COLNO - 1; x >= 1; x--)
            if (row[x] != ' ')
                break;
        row[x + 1] = '\0';

        putstr(win, 0, &row[1]);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* #wmode command */
STATIC_PTR int
wiz_show_wmodes(VOID_ARGS)
{
    winid win;
    int x, y;
    char row[COLNO + 1];
    struct rm *lev;
    boolean istty = !strcmp(windowprocs.name, "tty");

    win = create_nhwindow(NHW_TEXT);
    if (istty)
        putstr(win, 0, ""); /* tty only: blank top line */
    for (y = 0; y < ROWNO; y++) {
        for (x = 0; x < COLNO; x++) {
            lev = &levl[x][y];
            if (x == u.ux && y == u.uy)
                row[x] = '@';
            else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
                row[x] = '0' + (lev->wall_info & WM_MASK);
            else if (lev->typ == CORR)
                row[x] = '#';
            else if (IS_ROOM(lev->typ) || IS_DOOR(lev->typ))
                row[x] = '.';
            else
                row[x] = 'x';
        }
        row[COLNO] = '\0';
        /* map column 0, levl[0][], is off the left edge of the screen */
        putstr(win, 0, &row[1]);
    }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return 0;
}

/* wizard mode variant of #terrain; internal levl[][].typ values in base-36 */
STATIC_OVL void
wiz_map_levltyp(VOID_ARGS)
{
    winid win;
    int x, y, terrain;
    char row[COLNO + 1];
    boolean istty = !strcmp(windowprocs.name, "tty");

    win = create_nhwindow(NHW_TEXT);
    /* map row 0, levl[][0], is drawn on the second line of tty screen */
    if (istty)
        putstr(win, 0, ""); /* tty only: blank top line */
    for (y = 0; y < ROWNO; y++) {
        /* map column 0, levl[0][], is off the left edge of the screen;
           it should always have terrain type "undiggable stone" */
        for (x = 1; x < COLNO; x++) {
            terrain = levl[x][y].typ;
            /* assumes there aren't more than 10+26+26 terrain types */
            row[x - 1] = (char) ((terrain == 0 && !may_dig(x, y))
                                    ? '*'
                                    : (terrain < 10)
                                       ? '0' + terrain
                                       : (terrain < 36)
                                          ? 'a' + terrain - 10
                                          : 'A' + terrain - 36);
        }
        if (levl[0][y].typ != 0 || may_dig(0, y))
            row[x++] = '!';
        row[x] = '\0';
        putstr(win, 0, row);
    }

    {
        char dsc[BUFSZ];
        s_level *slev = Is_special(&u.uz);

        Sprintf(dsc, "D:%d,L:%d", u.uz.dnum, u.uz.dlevel);
        /* [dungeon branch features currently omitted] */
        /* special level features */
        if (slev) {
            Sprintf(eos(dsc), " \"%s\"", slev->proto);
            /* special level flags (note: dungeon.def doesn't set `maze'
               or `hell' for any specific levels so those never show up) */
            if (slev->flags.maze_like)
                Strcat(dsc, " mazelike");
            if (slev->flags.hellish)
                Strcat(dsc, " hellish");
            if (slev->flags.town)
                Strcat(dsc, " town");
            if (slev->flags.rogue_like)
                Strcat(dsc, " roguelike");
            /* alignment currently omitted to save space */
        }
        /* level features */
        if (level.flags.nfountains)
            Sprintf(eos(dsc), " %c:%d", defsyms[S_fountain].sym,
                    (int) level.flags.nfountains);
        if (level.flags.nsinks)
            Sprintf(eos(dsc), " %c:%d", defsyms[S_sink].sym,
                    (int) level.flags.nsinks);
        if (level.flags.has_vault)
            Strcat(dsc, " vault");
        if (level.flags.has_shop)
            Strcat(dsc, " shop");
        if (level.flags.has_temple)
            Strcat(dsc, " temple");
        if (level.flags.has_court)
            Strcat(dsc, " throne");
        if (level.flags.has_zoo)
            Strcat(dsc, " zoo");
        if (level.flags.has_morgue)
            Strcat(dsc, " morgue");
        if (level.flags.has_barracks)
            Strcat(dsc, " barracks");
        if (level.flags.has_beehive)
            Strcat(dsc, " hive");
        if (level.flags.has_swamp)
            Strcat(dsc, " swamp");
        /* level flags */
        if (level.flags.noteleport)
            Strcat(dsc, " noTport");
        if (level.flags.hardfloor)
            Strcat(dsc, " noDig");
        if (level.flags.nommap)
            Strcat(dsc, " noMMap");
        if (!level.flags.hero_memory)
            Strcat(dsc, " noMem");
        if (level.flags.shortsighted)
            Strcat(dsc, " shortsight");
        if (level.flags.graveyard)
            Strcat(dsc, " graveyard");
        if (level.flags.is_maze_lev)
            Strcat(dsc, " maze");
        if (level.flags.is_cavernous_lev)
            Strcat(dsc, " cave");
        if (level.flags.arboreal)
            Strcat(dsc, " tree");
        if (Sokoban)
            Strcat(dsc, " sokoban-rules");
        /* non-flag info; probably should include dungeon branching
           checks (extra stairs and magic portals) here */
        if (Invocation_lev(&u.uz))
            Strcat(dsc, " invoke");
        if (On_W_tower_level(&u.uz))
            Strcat(dsc, " tower");
        /* append a branch identifier for completeness' sake */
        if (u.uz.dnum == 0)
            Strcat(dsc, " dungeon");
        else if (u.uz.dnum == mines_dnum)
            Strcat(dsc, " mines");
        else if (In_sokoban(&u.uz))
            Strcat(dsc, " sokoban");
        else if (u.uz.dnum == quest_dnum)
            Strcat(dsc, " quest");
        else if (Is_knox(&u.uz))
            Strcat(dsc, " ludios");
        else if (u.uz.dnum == 1)
            Strcat(dsc, " gehennom");
        else if (u.uz.dnum == tower_dnum)
            Strcat(dsc, " vlad");
        else if (In_endgame(&u.uz))
            Strcat(dsc, " endgame");
        else {
            /* somebody's added a dungeon branch we're not expecting */
            const char *brname = dungeons[u.uz.dnum].dname;

            if (!brname || !*brname)
                brname = "unknown";
            if (!strncmpi(brname, "the ", 4))
                brname += 4;
            Sprintf(eos(dsc), " %s", brname);
        }
        /* limit the line length to map width */
        if (strlen(dsc) >= COLNO)
            dsc[COLNO - 1] = '\0'; /* truncate */
        putstr(win, 0, dsc);
    }

    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return;
}

/* temporary? hack, since level type codes aren't the same as screen
   symbols and only the latter have easily accessible descriptions */
static const char *levltyp[] = {
    "stone", "vertical wall", "horizontal wall", "top-left corner wall",
    "top-right corner wall", "bottom-left corner wall",
    "bottom-right corner wall", "cross wall", "tee-up wall", "tee-down wall",
    "tee-left wall", "tee-right wall", "drawbridge wall", "tree",
    "secret door", "secret corridor", "pool", "moat", "water",
    "drawbridge up", "lava pool", "iron bars", "door", "corridor", "room",
    "stairs", "ladder", "fountain", "throne", "sink", "grave", "altar", "ice",
    "drawbridge down", "air", "cloud",
    /* not a real terrain type, but used for undiggable stone
       by wiz_map_levltyp() */
    "unreachable/undiggable",
    /* padding in case the number of entries above is odd */
    ""
};

/* explanation of base-36 output from wiz_map_levltyp() */
STATIC_OVL void
wiz_levltyp_legend(VOID_ARGS)
{
    winid win;
    int i, j, last, c;
    const char *dsc, *fmt;
    char buf[BUFSZ];

    win = create_nhwindow(NHW_TEXT);
    putstr(win, 0, "#terrain encodings:");
    putstr(win, 0, "");
    fmt = " %c - %-28s"; /* TODO: include tab-separated variant for win32 */
    *buf = '\0';
    /* output in pairs, left hand column holds [0],[1],...,[N/2-1]
       and right hand column holds [N/2],[N/2+1],...,[N-1];
       N ('last') will always be even, and may or may not include
       the empty string entry to pad out the final pair, depending
       upon how many other entries are present in levltyp[] */
    last = SIZE(levltyp) & ~1;
    for (i = 0; i < last / 2; ++i)
        for (j = i; j < last; j += last / 2) {
            dsc = levltyp[j];
            c = !*dsc ? ' '
                   : !strncmp(dsc, "unreachable", 11) ? '*'
                      /* same int-to-char conversion as wiz_map_levltyp() */
                      : (j < 10) ? '0' + j
                         : (j < 36) ? 'a' + j - 10
                            : 'A' + j - 36;
            Sprintf(eos(buf), fmt, c, dsc);
            if (j > i) {
                putstr(win, 0, buf);
                *buf = '\0';
            }
        }
    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
    return;
}

/* #wizsmell command - test usmellmon(). */
STATIC_PTR int
wiz_smell(VOID_ARGS)
{
    int ans = 0;
    int mndx;  /* monster index */
    coord cc;  /* screen pos of unknown glyph */
    int glyph; /* glyph at selected position */

    cc.x = u.ux;
    cc.y = u.uy;
    mndx = 0; /* gcc -Wall lint */
    if (!olfaction(youmonst.data)) {
        You("are incapable of detecting odors in your present form.");
        return 0;
    }

    pline("You can move the cursor to a monster that you want to smell.");
    do {
        pline("Pick a monster to smell.");
        ans = getpos(&cc, TRUE, "a monster");
        if (ans < 0 || cc.x < 0) {
            return 0; /* done */
        }
        /* Convert the glyph at the selected position to a mndxbol. */
        glyph = glyph_at(cc.x, cc.y);
        if (glyph_is_monster(glyph))
            mndx = glyph_to_mon(glyph);
        else
            mndx = 0;
        /* Is it a monster? */
        if (mndx) {
            if (!usmellmon(&mons[mndx]))
                pline("That monster seems to give off no smell.");
        } else
            pline("That is not a monster.");
    } while (TRUE);
    return 0;
}

/* #wizrumorcheck command - verify each rumor access */
STATIC_PTR int
wiz_rumor_check(VOID_ARGS)
{
    rumor_check();
    return 0;
}

/* #terrain command -- show known map, inspired by crawl's '|' command */
STATIC_PTR int
doterrain(VOID_ARGS)
{
    winid men;
    menu_item *sel;
    anything any;
    int n;
    int which;

    /*
     * normal play: choose between known map without mons, obj, and traps
     *  (to see underlying terrain only), or
     *  known map without mons and objs (to see traps under mons and objs), or
     *  known map without mons (to see objects under monsters);
     * explore mode: normal choices plus full map (w/o mons, objs, traps);
     * wizard mode: normal and explore choices plus
     *  a dump of the internal levl[][].typ codes w/ level flags, or
     *  a legend for the levl[][].typ codes dump
     */
    men = create_nhwindow(NHW_MENU);
    any = zeroany;
    any.a_int = 1;
    add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
             "known map without monsters, objects, and traps",
             MENU_SELECTED);
    any.a_int = 2;
    add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
             "known map without monsters and objects",
             MENU_UNSELECTED);
    any.a_int = 3;
    add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
             "known map without monsters",
             MENU_UNSELECTED);
    if (discover || wizard) {
        any.a_int = 4;
        add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
                 "full map without monsters, objects, and traps",
                 MENU_UNSELECTED);
        if (wizard) {
            any.a_int = 5;
            add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
                     "internal levl[][].typ codes in base-36",
                     MENU_UNSELECTED);
            any.a_int = 6;
            add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
                     "legend of base-36 levl[][].typ codes",
                     MENU_UNSELECTED);
        }
    }
    end_menu(men, "View which?");

    n = select_menu(men, PICK_ONE, &sel);
    destroy_nhwindow(men);
    /*
     * n <  0: player used ESC to cancel;
     * n == 0: preselected entry was explicitly chosen and got toggled off;
     * n == 1: preselected entry was implicitly chosen via <space>|<enter>;
     * n == 2: another entry was explicitly chosen, so skip preselected one.
     */
    which = (n < 0) ? -1 : (n == 0) ? 1 : sel[0].item.a_int;
    if (n > 1 && which == 1)
        which = sel[1].item.a_int;
    if (n > 0)
        free((genericptr_t) sel);

    switch (which) {
    case 1: reveal_terrain(0, 0);   break; /* known map */
    case 2: reveal_terrain(0, 1);   break; /* known map with traps */
    case 3: reveal_terrain(0, 1|2); break; /* known map w/ traps & objs */
    case 4: reveal_terrain(1, 0);   break; /* full map */
    case 5: wiz_map_levltyp();      break; /* map internals */
    case 6: wiz_levltyp_legend();   break; /* internal details */
    default: break;
    }
    return 0; /* no time elapses */
}

/* -enlightenment and conduct- */
static winid en_win = WIN_ERR;
#if 0 /*JP*/
static const char You_[] = "You ", are[] = "are ", were[] = "were ",
                  have[] = "have ", had[] = "had ", can[] = "can ",
                  could[] = "could ";
#else
static const char You_[] = "あなたは", 
                  are[]  = "である",       were[]  = "であった",
                  have[] = "をもっている", had[]   = "をもっていた",
                  can[]  = "できる",       could[] = "できた",
                  iru[]  = "いる",         ita[]   = "いた";
#endif
#if 0 /*JP*//* not used */
static const char have_been[] = "have been ", have_never[] = "have never ",
                  never[] = "never ";
#endif

#if 0 /*JP*/
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line(prefix, final ? past : present, suffix, ps)
#else
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line(prefix, ps, suffix, final ? past : present)
#endif
#define you_are(attr, ps) enl_msg(You_, are, were, attr, ps)
#define you_have(attr, ps) enl_msg(You_, have, had, attr, ps)
#define you_can(attr, ps) enl_msg(You_, can, could, attr, ps)
/*JP
#define you_have_been(goodthing) enl_msg(You_, have_been, were, goodthing, "")
*/
#define you_have_been(goodthing) enl_msg(You_, are, were, goodthing, "")
#if 0 /*JP*/
#define you_have_never(badthing) \
    enl_msg(You_, have_never, never, badthing, "")
#else
#define you_have_never(badthing) \
    enl_msg(badthing, "ていない", "なかった", "", "")
#endif
#if 0 /*JP*/
#define you_have_X(something) \
    enl_msg(You_, have, (const char *) "", something, "")
#else
#define you_have_X(something) \
    enl_msg(something, "ている", "た", "", "")
#endif
#if 1 /*JP*/
#define you_are_ing(goodthing, ps) enl_msg(You_, iru, ita, goodthing, ps)
#endif

static void
enlght_line(start, middle, end, ps)
const char *start, *middle, *end, *ps;
{
    char buf[BUFSZ];

/*JP
    Sprintf(buf, " %s%s%s%s.", start, middle, end, ps);
*/
    Sprintf(buf, "%s%s%s%s．", start, middle, end, ps);
    putstr(en_win, 0, buf);
}

/* format increased chance to hit or damage or defense (Protection) */
static char *
enlght_combatinc(inctyp, incamt, final, outbuf)
const char *inctyp;
int incamt, final;
char *outbuf;
{
    const char *modif, *bonus;
    boolean invrt;
    int absamt;

    absamt = abs(incamt);
    /* Protection amount is typically larger than damage or to-hit;
       reduce magnitude by a third in order to stretch modifier ranges
       (small:1..5, moderate:6..10, large:11..19, huge:20+) */
    if (!strcmp(inctyp, "defense"))
        absamt = (absamt * 2) / 3;

    if (absamt <= 3)
/*JP
        modif = "small";
*/
        modif = "僅かな";
    else if (absamt <= 6)
/*JP
        modif = "moderate";
*/
        modif = "中程度の";
    else if (absamt <= 12)
/*JP
        modif = "large";
*/
        modif = "大きな";
    else
/*JP
        modif = "huge";
*/
        modif = "強大な";

#if 0 /*JP*/
    modif = !incamt ? "no" : an(modif); /* ("no" case shouldn't happen) */
#endif
/*JP
    bonus = (incamt >= 0) ? "bonus" : "penalty";
*/
    bonus = (incamt > 0) ? "ボーナス" : "ペナルティ";
    /* "bonus <foo>" (to hit) vs "<bar> bonus" (damage, defense) */
    invrt = strcmp(inctyp, "to hit") ? TRUE : FALSE;

#if 0 /*JP*/
    Sprintf(outbuf, "%s %s %s", modif, invrt ? inctyp : bonus,
            invrt ? bonus : inctyp);
#else
    Sprintf(outbuf, "%sに%s%s", inctyp, modif, bonus);
#endif
    if (final || wizard)
        Sprintf(eos(outbuf), " (%s%d)", (incamt > 0) ? "+" : "", incamt);

    return outbuf;
}

/* report half physical or half spell damage */
STATIC_OVL void
enlght_halfdmg(category, final)
int category;
int final;
{
    const char *category_name;
    char buf[BUFSZ];

    switch (category) {
    case HALF_PHDAM:
        category_name = "physical";
        break;
    case HALF_SPDAM:
        category_name = "spell";
        break;
    default:
        category_name = "unknown";
        break;
    }
    Sprintf(buf, " %s %s damage", (final || wizard) ? "half" : "reduced",
            category_name);
    enl_msg(You_, "take", "took", buf, from_what(category));
}

/* is hero actively using water walking capability on water (or lava)? */
STATIC_OVL boolean
walking_on_water()
{
    if (u.uinwater || Levitation || Flying)
        return FALSE;
    return (boolean) (Wwalking
                      && (is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)));
}

/* check whether hero is wearing something that player definitely knows
   confers the target property; item must have been seen and its type
   discovered but it doesn't necessarily have to be fully identified */
STATIC_OVL boolean
cause_known(propindx)
int propindx; /* index of a property which can be conveyed by worn item */
{
    register struct obj *o;
    long mask = W_ARMOR | W_AMUL | W_RING | W_TOOL;

    /* simpler than from_what()/what_gives(); we don't attempt to
       handle artifacts and we deliberately ignore wielded items */
    for (o = invent; o; o = o->nobj) {
        if (!(o->owornmask & mask))
            continue;
        if ((int) objects[o->otyp].oc_oprop == propindx
            && objects[o->otyp].oc_name_known && o->dknown)
            return TRUE;
    }
    return FALSE;
}

/* format a characteristic value, accommodating Strength's strangeness */
STATIC_OVL char *
attrval(attrindx, attrvalue, resultbuf)
int attrindx, attrvalue;
char resultbuf[]; /* should be at least [7] to hold "18/100\0" */
{
    if (attrindx != A_STR || attrvalue <= 18)
        Sprintf(resultbuf, "%d", attrvalue);
    else if (attrvalue > STR18(100)) /* 19 to 25 */
        Sprintf(resultbuf, "%d", attrvalue - 100);
    else /* simplify "18/ **" to be "18/100" */
        Sprintf(resultbuf, "18/%02d", attrvalue - 18);
    return resultbuf;
}

void
enlightenment(mode, final)
int mode;  /* BASICENLIGHTENMENT | MAGICENLIGHTENMENT (| both) */
int final; /* ENL_GAMEINPROGRESS:0, ENL_GAMEOVERALIVE, ENL_GAMEOVERDEAD */
{
    char buf[BUFSZ], tmpbuf[BUFSZ];

    Strcpy(tmpbuf, plname);
    *tmpbuf = highc(*tmpbuf); /* same adjustment as bottom line */
    /* as in background_enlightenment, when poly'd we need to use the saved
       gender in u.mfemale rather than the current you-as-monster gender */
#if 0 /*JP*/
    Sprintf(buf, "%s the %s's attributes:", tmpbuf,
            ((Upolyd ? u.mfemale : flags.female) && urole.name.f)
                ? urole.name.f
                : urole.name.m);
#else
    Sprintf(buf, "%sの%sの属性:",
            ((Upolyd ? u.mfemale : flags.female) && urole.name.f)
                ? urole.name.f
                : urole.name.m,
             tmpbuf);
#endif

    en_win = create_nhwindow(NHW_MENU);
    /* title */
    putstr(en_win, 0, buf); /* "Conan the Archeologist's attributes:" */
    /* background and characteristics; ^X or end-of-game disclosure */
    if (mode & BASICENLIGHTENMENT) {
        /* role, race, alignment, deities */
        background_enlightenment(mode, final);
        /* strength, dexterity, &c */
        characteristics_enlightenment(mode, final);
    }
    /* expanded status line information, including things which aren't
       included there due to space considerations--such as obvious
       alternative movement indicators (riding, levitation, &c), and
       various troubles (turning to stone, trapped, confusion, &c);
       shown for both basic and magic enlightenment */
    status_enlightenment(mode, final);
    /* remaining attributes; shown for potion,&c or wizard mode and
       explore mode ^X or end of game disclosure */
    if (mode & MAGICENLIGHTENMENT) {
        /* intrinsics and other traditional enlightenment feedback */
        attributes_enlightenment(mode, final);
    }
    display_nhwindow(en_win, TRUE);
    destroy_nhwindow(en_win);
    en_win = WIN_ERR;
}

/*ARGSUSED*/
/* display role, race, alignment and such to en_win */
STATIC_OVL void
background_enlightenment(unused_mode, final)
int unused_mode UNUSED;
int final;
{
    const char *role_titl, *rank_titl;
    int innategend, difgend, difalgn;
    char buf[BUFSZ], tmpbuf[BUFSZ];

    /* note that if poly'd, we need to use u.mfemale instead of flags.female
       to access hero's saved gender-as-human/elf/&c rather than current one */
    innategend = (Upolyd ? u.mfemale : flags.female) ? 1 : 0;
    role_titl = (innategend && urole.name.f) ? urole.name.f : urole.name.m;
    rank_titl = rank_of(u.ulevel, Role_switch, innategend);

    putstr(en_win, 0, ""); /* separator after title */
/*JP
    putstr(en_win, 0, "Background:");
*/
    putstr(en_win, 0, "背景情報:");

    /* if polymorphed, report current shape before underlying role;
       will be repeated as first status: "you are transformed" and also
       among various attributes: "you are in beast form" (after being
       told about lycanthropy) or "you are polymorphed into <a foo>"
       (with countdown timer appended for wizard mode); we really want
       the player to know he's not a samurai at the moment... */
    if (Upolyd) {
        struct permonst *uasmon = youmonst.data;

        tmpbuf[0] = '\0';
        /* here we always use current gender, not saved role gender */
        if (!is_male(uasmon) && !is_female(uasmon) && !is_neuter(uasmon))
/*JP
            Sprintf(tmpbuf, "%s ", genders[flags.female ? 1 : 0].adj);
*/
            Sprintf(tmpbuf, "%sの", genders[flags.female ? 1 : 0].adj);
#if 0 /*JP*/
        Sprintf(buf, "%sin %s%s form", !final ? "currently " : "", tmpbuf,
                uasmon->mname);
#else
        Sprintf(buf, "%s%s%sの姿", !final ? "今のところ" : "", tmpbuf,
                uasmon->mname);
#endif
        you_are(buf, "");
    }

    /* report role; omit gender if it's redundant (eg, "female priestess") */
    tmpbuf[0] = '\0';
    if (!urole.name.f
        && ((urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            || innategend != flags.initgend))
/*JP
        Sprintf(tmpbuf, "%s ", genders[innategend].adj);
*/
        Sprintf(tmpbuf, "%s", genders[innategend].adj);
    buf[0] = '\0';
    if (Upolyd)
#if 0 /*JP*/
        Strcpy(buf, "actually "); /* "You are actually a ..." */
#else
        Strcpy(buf, "実際には"); /* "あなたは実際には..." */
#endif
    if (!strcmpi(rank_titl, role_titl)) {
        /* omit role when rank title matches it */
#if 0 /*JP*/
        Sprintf(eos(buf), "%s, level %d %s%s", an(rank_titl), u.ulevel,
                tmpbuf, urace.noun);
#else
        Sprintf(eos(buf), "レベル%dの%sの%s%s", u.ulevel,
                tmpbuf, urace.adj, role_titl);
#endif
    } else {
#if 0 /*JP*/
        Sprintf(eos(buf), "%s, a level %d %s%s %s", an(rank_titl), u.ulevel,
                tmpbuf, urace.adj, role_titl);
#else
        Sprintf(eos(buf), "レベル%dの%sの%s%sの%s", u.ulevel,
                tmpbuf, urace.adj, role_titl, rank_titl);
#endif
    }
    you_are(buf, "");

    /* report alignment (bypass you_are() in order to omit ending period) */
#if 0 /*JP*/
    Sprintf(buf, " %s%s%s, %son a mission for %s",
            You_, !final ? are : were,
            align_str(u.ualign.type),
            /* helm of opposite alignment (might hide conversion) */
            (u.ualign.type != u.ualignbase[A_CURRENT]) ? "temporarily "
               /* permanent conversion */
               : (u.ualign.type != u.ualignbase[A_ORIGINAL]) ? "now "
                  /* atheist (ignored in very early game) */
                  : (!u.uconduct.gnostic && moves > 1000L) ? "nominally "
                     /* lastly, normal case */
                     : "",
            u_gname());
#else
    Sprintf(buf, "あなたは%sで, %s%sのための任務を行って%s．",
            align_str(u.ualign.type),
            /* helm of opposite alignment (might hide conversion) */
            (u.ualign.type != u.ualignbase[A_CURRENT]) ? "一時的に"
               /* permanent conversion */
               : (u.ualign.type != u.ualignbase[A_ORIGINAL]) ? "現在"
                  /* atheist (ignored in very early game) */
                  : (!u.uconduct.gnostic && moves > 1000L) ? "名義上"
                     /* lastly, normal case */
                     : "",
            u_gname(), !final ? iru : ita);
#endif
    putstr(en_win, 0, buf);
    /* show the rest of this game's pantheon (finishes previous sentence)
       [appending "also Moloch" at the end would allow for straightforward
       trailing "and" on all three aligned entries but looks too verbose] */
#if 0 /*JP*/
    Sprintf(buf, " who %s opposed by", !final ? "is" : "was");
#else
    Strcpy(buf, "あなたは");
#endif
    if (u.ualign.type != A_LAWFUL)
#if 0 /*JP*/
        Sprintf(eos(buf), " %s (%s) and", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
#else
        Sprintf(eos(buf), "%s(%s)および", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
#endif
    if (u.ualign.type != A_NEUTRAL)
#if 0 /*JP*/
        Sprintf(eos(buf), " %s (%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? " and" : "");
#else
        Sprintf(eos(buf), "%s(%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? "および" : "");
#endif
#if 0 /*JP*/
    if (u.ualign.type != A_CHAOTIC)
        Sprintf(eos(buf), " %s (%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
#else
    if (u.ualign.type != A_CHAOTIC)
        Sprintf(eos(buf), "%s(%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
#endif
#if 0 /*JP*/
    Strcat(buf, "."); /* terminate sentence */
#else
    Sprintf(eos(buf), "と対立して%s．", !final ? iru : ita);
#endif
    putstr(en_win, 0, buf);

    /* show original alignment,gender,race,role if any have been changed;
       giving separate message for temporary alignment change bypasses need
       for tricky phrasing otherwise necessitated by possibility of having
       helm of opposite alignment mask a permanent alignment conversion */
    difgend = (innategend != flags.initgend);
    difalgn = (((u.ualign.type != u.ualignbase[A_CURRENT]) ? 1 : 0)
               + ((u.ualignbase[A_CURRENT] != u.ualignbase[A_ORIGINAL])
                  ? 2 : 0));
    if (difalgn & 1) { /* have temporary alignment so report permanent one */
/*JP
        Sprintf(buf, "actually %s", align_str(u.ualignbase[A_CURRENT]));
*/
        Sprintf(buf, "実際には%s", align_str(u.ualignbase[A_CURRENT]));
        you_are(buf, "");
        difalgn &= ~1; /* suppress helm from "started out <foo>" message */
    }
    if (difgend || difalgn) { /* sex change or perm align change or both */
#if 0 /*JP*/
        Sprintf(buf, " You started out %s%s%s.",
                difgend ? genders[flags.initgend].adj : "",
                (difgend && difalgn) ? " and " : "",
                difalgn ? align_str(u.ualignbase[A_ORIGINAL]) : "");
#else
        Sprintf(buf, "あなたは%s%s%sで開始した．",
                difgend ? genders[flags.initgend].adj : "",
                (difgend && difalgn) ? "かつ" : "",
                difalgn ? align_str(u.ualignbase[A_ORIGINAL]) : "");
#endif
        putstr(en_win, 0, buf);
    }
}

/* characteristics: expanded version of bottom line strength, dexterity, &c */
STATIC_OVL void
characteristics_enlightenment(mode, final)
int mode;
int final;
{
    putstr(en_win, 0, ""); /* separator after background */
    putstr(en_win, 0,
/*JP
           final ? "Final Characteristics:" : "Current Characteristics:");
*/
           final ? "最終属性：" : "現在の属性：");

    /* bottom line order */
    one_characteristic(mode, final, A_STR); /* strength */
    one_characteristic(mode, final, A_DEX); /* dexterity */
    one_characteristic(mode, final, A_CON); /* constitution */
    one_characteristic(mode, final, A_INT); /* intelligence */
    one_characteristic(mode, final, A_WIS); /* wisdom */
    one_characteristic(mode, final, A_CHA); /* charisma */
}

/* display one attribute value for characteristics_enlightenment() */
STATIC_OVL void
one_characteristic(mode, final, attrindx)
int mode, final, attrindx;
{
    boolean hide_innate_value = FALSE, interesting_alimit;
    int acurrent, abase, apeak, alimit;
    const char *attrname, *paren_pfx;
    char subjbuf[BUFSZ], valubuf[BUFSZ], valstring[32];

    /* being polymorphed or wearing certain cursed items prevents
       hero from reliably tracking changes to characteristics so
       we don't show base & peak values then; when the items aren't
       cursed, hero could take them off to check underlying values
       and we show those in such case so that player doesn't need
       to actually resort to doing that */
    if (Upolyd) {
        hide_innate_value = TRUE;
    } else if (Fixed_abil) {
        if (stuck_ring(uleft, RIN_SUSTAIN_ABILITY)
            || stuck_ring(uright, RIN_SUSTAIN_ABILITY))
            hide_innate_value = TRUE;
    }
    switch (attrindx) {
    case A_STR:
/*JP
        attrname = "strength";
*/
        attrname = "強さ";
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER && uarmg->cursed)
            hide_innate_value = TRUE;
        break;
    case A_DEX:
/*JP
        attrname = "dexterity";
*/
        attrname = "素早さ";
        break;
    case A_CON:
/*JP
        attrname = "constitution";
*/
        attrname = "耐久力";
        break;
    case A_INT:
/*JP
        attrname = "intelligence";
*/
        attrname = "知力";
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_WIS:
/*JP
        attrname = "wisdom";
*/
        attrname = "賢さ";
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_CHA:
/*JP
        attrname = "charisma";
*/
        attrname = "魅力";
        break;
    default:
        return; /* impossible */
    };
    /* note: final disclosure includes MAGICENLIGHTENTMENT */
    if ((mode & MAGICENLIGHTENMENT) && !Upolyd)
        hide_innate_value = FALSE;

    acurrent = ACURR(attrindx);
    (void) attrval(attrindx, acurrent, valubuf); /* Sprintf(valubuf,"%d",) */
/*JP
    Sprintf(subjbuf, "Your %s ", attrname);
*/
    Sprintf(subjbuf, "あなたの%sは", attrname);

    if (!hide_innate_value) {
        /* show abase, amax, and/or attrmax if acurr doesn't match abase
           (a magic bonus or penalty is in effect) or abase doesn't match
           amax (some points have been lost to poison or exercise abuse
           and are restorable) or attrmax is different from normal human
           (while game is in progress; trying to reduce dependency on
           spoilers to keep track of such stuff) or attrmax was different
           from abase (at end of game; this attribute wasn't maxed out) */
        abase = ABASE(attrindx);
        apeak = AMAX(attrindx);
        alimit = ATTRMAX(attrindx);
        /* criterium for whether the limit is interesting varies */
        interesting_alimit =
            final ? TRUE /* was originally `(abase != alimit)' */
                  : (alimit != (attrindx != A_STR ? 18 : STR18(100)));
/*JP
        paren_pfx = final ? " (" : " (current; ";
*/
        paren_pfx = final ? " (" : " (現在; ";
        if (acurrent != abase) {
#if 0 /*JP*/
            Sprintf(eos(valubuf), "%sbase:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
#else
            Sprintf(eos(valubuf), "%s基本:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
#endif
            paren_pfx = ", ";
        }
        if (abase != apeak) {
#if 0 /*JP*/
            Sprintf(eos(valubuf), "%speak:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
#else
            Sprintf(eos(valubuf), "%s最大:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
#endif
            paren_pfx = ", ";
        }
        if (interesting_alimit) {
#if 0 /*JP*/
            Sprintf(eos(valubuf), "%s%slimit:%s", paren_pfx,
                    /* more verbose if exceeding 'limit' due to magic bonus */
                    (acurrent > alimit) ? "innate " : "",
                    attrval(attrindx, alimit, valstring));
#else
            Sprintf(eos(valubuf), "%s%s上限:%s", paren_pfx,
                    /* more verbose if exceeding 'limit' due to magic bonus */
                    (acurrent > alimit) ? "本来の" : "",
                    attrval(attrindx, alimit, valstring));
#endif
            /* paren_pfx = ", "; */
        }
        if (acurrent != abase || abase != apeak || interesting_alimit)
            Strcat(valubuf, ")");
    }
/*JP
    enl_msg(subjbuf, "is ", "was ", valubuf, "");
*/
    enl_msg(subjbuf, "だ", "だった", valubuf, "");
}

/* status: selected obvious capabilities, assorted troubles */
STATIC_OVL void
status_enlightenment(mode, final)
int mode;
int final;
{
    boolean magic = (mode & MAGICENLIGHTENMENT) ? TRUE : FALSE;
    int cap;
    char buf[BUFSZ], youtoo[BUFSZ];
    boolean Riding = (u.usteed
                      /* if hero dies while dismounting, u.usteed will still
                         be set; we want to ignore steed in that situation */
                      && !(final == ENL_GAMEOVERDEAD
/*JP
                           && !strcmp(killer.name, "riding accident")));
*/
                           && !strcmp(killer.name, "騎乗事故で")));
    const char *steedname = (!Riding ? (char *) 0
                      : x_monnam(u.usteed,
                                 u.usteed->mtame ? ARTICLE_YOUR : ARTICLE_THE,
                                 (char *) 0,
                                 (SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION),
                                 FALSE));

    /*\
     * Status (many are abbreviated on bottom line; others are or
     *     should be discernible to the hero hence to the player)
    \*/
    putstr(en_win, 0, ""); /* separator after title or characteristics */
/*JP
    putstr(en_win, 0, final ? "Final Status:" : "Current Status:");
*/
    putstr(en_win, 0, final ? "最終状態:" : "現在の状態:");

    Strcpy(youtoo, You_);
    /* not a traditional status but inherently obvious to player; more
       detail given below (attributes section) for magic enlightenment */
    if (Upolyd)
/*JP
        you_are("transformed", "");
*/
        you_are_ing("変化して", "");
    /* not a trouble, but we want to display riding status before maybe
       reporting steed as trapped or hero stuck to cursed saddle */
    if (Riding) {
#if 0 /*JP*/
        Sprintf(buf, "riding %s", steedname);
        you_are(buf, "");
#else
        Sprintf(buf, "%sに乗って", steedname);
        you_are_ing(buf, "");
#endif
/*JP
        Sprintf(eos(youtoo), "and %s ", steedname);
*/
        Sprintf(youtoo, "あなたと%sは", steedname);
    }
    /* other movement situations that hero should always know */
    if (Levitation) {
        if (Lev_at_will && magic)
/*JP
            you_are("levitating, at will", "");
*/
            you_are_ing("自分の意志で浮遊して", "");
        else
/*JP
            enl_msg(youtoo, are, were, "levitating", from_what(LEVITATION));
*/
            enl_msg(youtoo, "いる", "いた", "浮遊して", from_what(LEVITATION));
    } else if (Flying) { /* can only fly when not levitating */
/*JP
        enl_msg(youtoo, are, were, "flying", from_what(FLYING));
*/
        enl_msg(youtoo, "いる", "いた", "飛んで", from_what(FLYING));
    }
    if (Underwater) {
/*JP
        you_are("underwater", "");
*/
        enl_msg(You_, "いる", "いた", "水面下に", "");
    } else if (u.uinwater) {
/*JP
        you_are(Swimming ? "swimming" : "in water", from_what(SWIMMING));
*/
        enl_msg(You_, Swimming ? "泳いで" : "水中に", "いる", "いた", from_what(SWIMMING));
    } else if (walking_on_water()) {
        /* show active Wwalking here, potential Wwalking elsewhere */
#if 0 /*JP*/
        Sprintf(buf, "walking on %s",
                is_pool(u.ux, u.uy) ? "water"
                : is_lava(u.ux, u.uy) ? "lava"
                  : surface(u.ux, u.uy)); /* catchall; shouldn't happen */
        you_are(buf, from_what(WWALKING));
#else
        Sprintf(buf, "%sの上を歩いて",
                is_pool(u.ux, u.uy) ? "水"
                : is_lava(u.ux, u.uy) ? "溶岩"
                  : surface(u.ux, u.uy)); /* catchall; shouldn't happen */
        you_are_ing(buf, from_what(WWALKING));
#endif
    }
    if (Upolyd && (u.uundetected || youmonst.m_ap_type != M_AP_NOTHING))
        youhiding(TRUE, final);

    /* internal troubles, mostly in the order that prayer ranks them */
    if (Stoned)
/*JP
        you_are("turning to stone", "");
*/
        enl_msg("あなたは", "なりつつある", "なった", "石に", "");
    if (Slimed)
/*JP
        you_are("turning into slime", "");
*/
        enl_msg("あなたは", "なりつつある", "なった", "スライムに", "");
    if (Strangled) {
        if (u.uburied) {
/*JP
            you_are("buried", "");
*/
            you_are_ing("窒息して", "");
        } else {
/*JP
            Strcpy(buf, "being strangled");
*/
            Strcpy(buf, "首を絞められて");
            if (wizard)
                Sprintf(eos(buf), " (%ld)", (Strangled & TIMEOUT));
/*JP
            you_are(buf, from_what(STRANGLED));
*/
            enl_msg("あなたは", "いる", "いた", buf, from_what(STRANGLED));
        }
    }
    if (Sick) {
        /* prayer lumps these together; botl puts Ill before FoodPois */
        if (u.usick_type & SICK_NONVOMITABLE)
/*JP
            you_are("terminally sick from illness", "");
*/
            enl_msg("あなたは病気で致命的に気分が悪", "い", "かった", "", "");
        if (u.usick_type & SICK_VOMITABLE)
/*JP
            you_are("terminally sick from food poisoning", "");
*/
            enl_msg("あなたは食中毒で致命的に気分が悪", "い", "かった", "", "");
    }
    if (Vomiting)
/*JP
        you_are("nauseated", "");
*/
        enl_msg(You_, "吐き気が", "ある", "あった", "");
    if (Stunned)
/*JP
        you_are("stunned", "");
*/
        you_are("くらくら状態", "");
    if (Confusion)
/*JP
        you_are("confused", "");
*/
        you_are("混乱状態", "");
    if (Hallucination)
/*JP
        you_are("hallucinating", "");
*/
        you_are("幻覚状態", "");
    if (Blind) {
        /* from_what() (currently wizard-mode only) checks !haseyes()
           before u.uroleplay.blind, so we should too */
#if 0 /*JP*/
        Sprintf(buf, "%s blind",
                !haseyes(youmonst.data) ? "innately"
                : u.uroleplay.blind ? "permanently"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "deliberately"
                    : "temporarily");
#else
        Sprintf(buf, "%s盲目",
                !haseyes(youmonst.data) ? "生まれながらに"
                : u.uroleplay.blind ? "恒久的に"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "故意に"
                    : "一時的に");
#endif
        if (wizard && (Blinded & TIMEOUT) != 0L
            && !u.uroleplay.blind && haseyes(youmonst.data))
            Sprintf(eos(buf), " (%ld)", (Blinded & TIMEOUT));
        /* !haseyes: avoid "you are innately blind innately" */
        you_are(buf, !haseyes(youmonst.data) ? "" : from_what(BLINDED));
    }
    if (Deaf)
/*JP
        you_are("deaf", from_what(DEAF));
*/
        you_are("耳が聞こえない状態", from_what(DEAF));

    /* external troubles, more or less */
    if (Punished) {
        if (uball) {
/*JP
            Sprintf(buf, "chained to %s", ansimpleoname(uball));
*/
            Sprintf(buf, "%sにつながれて", ansimpleoname(uball));
        } else {
            impossible("Punished without uball?");
/*JP
            Strcpy(buf, "punished");
*/
            Strcpy(buf, "罰を受けて");
        }
/*JP
        you_are(buf, "");
*/
        you_are_ing(buf, "");
    }
    if (u.utrap) {
        char predicament[BUFSZ];
        struct trap *t;
        boolean anchored = (u.utraptype == TT_BURIEDBALL);

        if (anchored) {
/*JP
            Strcpy(predicament, "tethered to something buried");
*/
            Strcpy(predicament, "何か埋まっているものにつながれて");
        } else if (u.utraptype == TT_INFLOOR || u.utraptype == TT_LAVA) {
/*JP
            Sprintf(predicament, "stuck in %s", the(surface(u.ux, u.uy)));
*/
            Sprintf(predicament, "%sに埋まって", surface(u.ux, u.uy));
        } else {
#if 0 /*JP*/
            Strcpy(predicament, "trapped");
            if ((t = t_at(u.ux, u.uy)) != 0)
                Sprintf(eos(predicament), " in %s",
                        an(defsyms[trap_to_defsym(t->ttyp)].explanation));
#else
            predicament[0] = '\0';
            if ((t = t_at(u.ux, u.uy)) != 0)
                Sprintf(predicament, "%sに",
                        defsyms[trap_to_defsym(t->ttyp)].explanation);
            Strcat(predicament, "ひっかかって");
#endif
        }
        if (u.usteed) { /* not `Riding' here */
#if 0 /*JP*/
            Sprintf(buf, "%s%s ", anchored ? "you and " : "", steedname);
            *buf = highc(*buf);
            enl_msg(buf, (anchored ? "are " : "is "),
                    (anchored ? "were " : "was "), predicament, "");
#else
            Sprintf(buf, "%s%sは", anchored ? "あなたと" : "", steedname);
            enl_msg(buf, "いる", "いた" , predicament, "");
#endif
        } else
            you_are(predicament, "");
    } /* (u.utrap) */
    if (u.uswallow) {
/*JP
        Sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
*/
        Sprintf(buf, "%sに飲み込まれて", a_monnam(u.ustuck));
        if (wizard)
            Sprintf(eos(buf), " (%u)", u.uswldtim);
        you_are(buf, "");
    } else if (u.ustuck) {
#if 0 /*JP*/
        Sprintf(buf, "%s %s",
                (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
                a_monnam(u.ustuck));
        you_are(buf, "");
#else
        Sprintf(buf, "%s%s",
                a_monnam(u.ustuck),
                (Upolyd && sticks(youmonst.data)) ? "を捕まえて" : "に捕まって");
        you_are_ing(buf, "");
#endif
    }
    if (Riding) {
        struct obj *saddle = which_armor(u.usteed, W_SADDLE);

        if (saddle && saddle->cursed) {
            Sprintf(buf, "stuck to %s %s", s_suffix(steedname),
                    simpleonames(saddle));
            you_are(buf, "");
        }
    }
    if (Wounded_legs) {
        /* when mounted, Wounded_legs applies to steed rather than to
           hero; we only report steed's wounded legs in wizard mode */
        if (u.usteed) { /* not `Riding' here */
            if (wizard && steedname) {
#if 0 /*JP*/
                Strcpy(buf, steedname);
                *buf = highc(*buf);
                enl_msg(buf, " has", " had", " wounded legs", "");
#else
                enl_msg(buf, iru, ita, "は肢を怪我して", "");
#endif
            }
        } else {
#if 0 /*JP*/
            Sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
            you_have(buf, "");
#else
            Sprintf(buf, "%sを怪我して", makeplural(body_part(LEG)));
            you_are_ing(buf, "");
#endif
        }
    }
    if (Glib) {
#if 0 /*JP*/
        Sprintf(buf, "slippery %s", makeplural(body_part(FINGER)));
        you_have(buf, "");
#else
        Sprintf(buf, "%sがぬるぬるして", body_part(FINGER));
        enl_msg(buf, iru, ita, "", "");
#endif
    }
    if (Fumbling) {
        if (magic || cause_known(FUMBLING))
/*JP
            enl_msg(You_, "fumble", "fumbled", "", from_what(FUMBLING));
*/
            you_are_ing("不器用になって", from_what(FUMBLING));
    }
    if (Sleepy) {
        if (magic || cause_known(SLEEPY)) {
            Strcpy(buf, from_what(SLEEPY));
            if (wizard)
                Sprintf(eos(buf), " (%ld)", (HSleepy & TIMEOUT));
/*JP
            enl_msg("You ", "fall", "fell", " asleep uncontrollably", buf);
*/
            you_are_ing("眠って", buf);
        }
    }
    /* hunger/nutrition */
    if (Hunger) {
        if (magic || cause_known(HUNGER))
#if 0 /*JP*/
            enl_msg(You_, "hunger", "hungered", " rapidly",
                    from_what(HUNGER));
#else
            enl_msg("あなたはすぐに腹が減る状態", "である", "だった", "", "");
#endif
    }
    Strcpy(buf, hu_stat[u.uhs]); /* hunger status; omitted if "normal" */
    mungspaces(buf);             /* strip trailing spaces */
    if (*buf) {
#if 0 /*JP*/
        *buf = lowc(*buf); /* override capitalization */
        if (!strcmp(buf, "weak"))
            Strcat(buf, " from severe hunger");
        else if (!strncmp(buf, "faint", 5)) /* fainting, fainted */
            Strcat(buf, " due to starvation");
        you_are(buf, "");
#else
        Strcat(buf, "状態");
        you_are(buf, "");
#endif
    }
    /* encumbrance */
    if ((cap = near_capacity()) > UNENCUMBERED) {
#if 0 /*JP*/
        const char *adj = "?_?"; /* (should always get overridden) */

        Strcpy(buf, enc_stat[cap]);
        *buf = lowc(*buf);
        switch (cap) {
        case SLT_ENCUMBER:
            adj = "slightly";
            break; /* burdened */
        case MOD_ENCUMBER:
            adj = "moderately";
            break; /* stressed */
        case HVY_ENCUMBER:
            adj = "very";
            break; /* strained */
        case EXT_ENCUMBER:
            adj = "extremely";
            break; /* overtaxed */
        case OVERLOADED:
            adj = "not possible";
            break;
        }
        Sprintf(eos(buf), "; movement %s %s%s", !final ? "is" : "was", adj,
                (cap < OVERLOADED) ? " slowed" : "");
        you_are(buf, "");
#else
        Sprintf(buf, "荷物によって%s状態", enc_stat[cap]);
        you_are(buf, "");
#endif
    } else {
        /* last resort entry, guarantees Status section is non-empty
           (no longer needed for that purpose since weapon status added;
           still useful though) */
/*JP
        you_are("unencumbered", "");
*/
        you_are("荷物は邪魔にならない状態", "");
    }
    /* report being weaponless; distinguish whether gloves are worn */
    if (!uwep) {
#if 0 /*JP*/
        you_are(uarmg ? "empty handed" /* gloves imply hands */
                      : humanoid(youmonst.data)
                         /* hands but no weapon and no gloves */
                         ? "bare handed"
                         /* alternate phrasing for paws or lack of hands */
                         : "not wielding anything",
                "");
#else
        enl_msg(You_, "い", "かった", "武器を装備していな", "");
#endif
    /* two-weaponing implies a weapon (not other odd stuff) in each hand */
    } else if (u.twoweap) {
/*JP
        you_are("wielding two weapons at once", "");
*/
        you_are("二刀流", "");
    /* report most weapons by their skill class (so a katana will be
       described as a long sword, for instance; mattock and hook are
       exceptions), or wielded non-weapon item by its object class */
    } else {
        const char *what = weapon_descr(uwep);

#if 0 /*JP*/
        if (!strcmpi(what, "armor") || !strcmpi(what, "food")
            || !strcmpi(what, "venom"))
            Sprintf(buf, "wielding some %s", what);
        else
            Sprintf(buf, "wielding %s",
                    (uwep->quan == 1L) ? an(what) : makeplural(what));
        you_are(buf, "");
#else
        Sprintf(buf, "%sを装備して", what);
        enl_msg(You_, "いる", "いた", buf, "");
#endif
    }
    /* report 'nudity' */
    if (!uarm && !uarmu && !uarmc && !uarmg && !uarmf && !uarmh) {
        if (u.uroleplay.nudist)
#if 0 /*JP*/
            enl_msg(You_, "do", "did", " not wear any armor", "");
#else
            enl_msg(You_, "い", "かった", "何の鎧も装備しな", "");
#endif
        else
#if 0 /*JP*/
            you_are("not wearing any armor", "");
#else
            enl_msg(You_, "い", "かった", "何の鎧も装備していな", "");
#endif
    }
}

/* attributes: intrinsics and the like, other non-obvious capabilities */
void
attributes_enlightenment(unused_mode, final)
int unused_mode UNUSED;
int final;
{
    static NEARDATA const char if_surroundings_permitted[] =
        " if surroundings permitted";
    int ltmp, armpro;
    char buf[BUFSZ];

    /*\
     *  Attributes
    \*/
    putstr(en_win, 0, "");
/*JP
    putstr(en_win, 0, final ? "Final Attributes:" : "Current Attributes:");
*/
    putstr(en_win, 0, final ? "最終属性:" : "現在の属性:");

    if (u.uevent.uhand_of_elbereth) {
#if 0 /*JP*/
        static const char *const hofe_titles[3] = { "the Hand of Elbereth",
                                                    "the Envoy of Balance",
                                                    "the Glory of Arioch" };
#else
        static const char *const hofe_titles[3] = { "エルベレスの御手",
                                                    "調和の使者",
                                                    "アリオッチの名誉" };
#endif
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1], "");
    }

    /* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20)
/*JP
        you_are("piously aligned", "");
*/
      you_have("敬虔な信仰心", "");
    else if (u.ualign.record > 13)
/*JP
        you_are("devoutly aligned", "");
*/
      you_have("心からの信仰心", "");
    else if (u.ualign.record > 8)
/*JP
        you_are("fervently aligned", "");
*/
      you_have("熱心な信仰心", "");
    else if (u.ualign.record > 3)
/*JP
        you_are("stridently aligned", "");
*/
      you_have("おおげさな信仰心", "");
    else if (u.ualign.record == 3)
/*JP
        you_are("aligned", "");
*/
      you_have("普通の信仰心", "");
    else if (u.ualign.record > 0)
/*JP
        you_are("haltingly aligned", "");
*/
      you_have("躊躇しながらも信仰心", "");
    else if (u.ualign.record == 0)
/*JP
        you_are("nominally aligned", "");
*/
      you_have("形だけの信仰心", "");
    else if (u.ualign.record >= -3)
/*JP
        you_have("strayed", "");
*/
      you_are_ing("信仰に迷いをもって", "");
    else if (u.ualign.record >= -8)
/*JP
        you_have("sinned", "");
*/
      you_are_ing("罪を負って", "");
    else
/*JP
        you_have("transgressed", "");
*/
      you_are_ing("信仰から逸脱して", "");
    if (wizard) {
#if 0 /*JP*/
        Sprintf(buf, " %d", u.ualign.record);
        enl_msg("Your alignment ", "is", "was", buf, "");
#else
        Sprintf(buf, "あなたの属性値は%d", u.ualign.record);
        enl_msg(buf, "である", "だった", "", "");
#endif
    }

    /*** Resistances to troubles ***/
    if (Invulnerable)
/*JP
        you_are("invulnerable", from_what(INVULNERABLE));
*/
        you_are("不死身", from_what(INVULNERABLE));
    if (Antimagic)
/*JP
        you_are("magic-protected", from_what(ANTIMAGIC));
*/
        you_have("魔法防御能力", from_what(ANTIMAGIC));
    if (Fire_resistance)
/*JP
        you_are("fire resistant", from_what(FIRE_RES));
*/
        you_have("火への耐性", from_what(FIRE_RES));
    if (Cold_resistance)
/*JP
        you_are("cold resistant", from_what(COLD_RES));
*/
        you_have("寒さへの耐性", from_what(COLD_RES));
    if (Sleep_resistance)
/*JP
        you_are("sleep resistant", from_what(SLEEP_RES));
*/
        you_have("眠りへの耐性", from_what(SLEEP_RES));
    if (Disint_resistance)
/*JP
        you_are("disintegration-resistant", from_what(DISINT_RES));
*/
        you_have("粉砕への耐性", from_what(DISINT_RES));
    if (Shock_resistance)
/*JP
        you_are("shock resistant", from_what(SHOCK_RES));
*/
        you_have("電撃への耐性", from_what(SHOCK_RES));
    if (Poison_resistance)
/*JP
        you_are("poison resistant", from_what(POISON_RES));
*/
        you_have("毒への耐性", from_what(POISON_RES));
    if (Acid_resistance)
/*JP
        you_are("acid resistant", from_what(ACID_RES));
*/
        you_have("酸への耐性", from_what(ACID_RES));
    if (Drain_resistance)
/*JP
        you_are("level-drain resistant", from_what(DRAIN_RES));
*/
        you_have("レベルダウンへの耐性", from_what(DRAIN_RES));
    if (Sick_resistance)
/*JP
        you_are("immune to sickness", from_what(SICK_RES));
*/
        you_have("病気に対する免疫", from_what(SICK_RES));
    if (Stone_resistance)
/*JP
        you_are("petrification resistant", from_what(STONE_RES));
*/
        you_have("石化への耐性", from_what(STONE_RES));
    if (Halluc_resistance)
#if 0 /*JP*/
        enl_msg(You_, "resist", "resisted", " hallucinations",
                from_what(HALLUC_RES));
#else
        you_have("幻覚への耐性", from_what(HALLUC_RES));
#endif
    if (u.uedibility)
/*JP
        you_can("recognize detrimental food", "");
*/
        you_can("有害な食料を識別", "");

    /*** Vision and senses ***/
    if (!Blind && (Blinded || !haseyes(youmonst.data)))
        you_can("see", from_what(-BLINDED)); /* Eyes of the Overworld */
    if (See_invisible) {
        if (!Blind)
/*JP
            enl_msg(You_, "see", "saw", " invisible", from_what(SEE_INVIS));
*/
            enl_msg("あなたは透明なものを見られ", "る", "た", "", from_what(SEE_INVIS));
        else
#if 0 /*JP*/
            enl_msg(You_, "will see", "would have seen",
                    " invisible when not blind", from_what(SEE_INVIS));
#else
            enl_msg(You_, "る", "た",
                    "盲目でないときには透明なものを見られ", from_what(SEE_INVIS));
#endif
    }
    if (Blind_telepat)
/*JP
        you_are("telepathic", from_what(TELEPAT));
*/
        you_have("テレパシー", from_what(TELEPAT));
    if (Warning)
/*JP
        you_are("warned", from_what(WARNING));
*/
        you_have("警戒能力", from_what(WARNING));
    if (Warn_of_mon && context.warntype.obj) {
#if 0 /*JP*/
        Sprintf(buf, "aware of the presence of %s",
                (context.warntype.obj & M2_ORC) ? "orcs"
                : (context.warntype.obj & M2_ELF) ? "elves"
                : (context.warntype.obj & M2_DEMON) ? "demons" : something);
        you_are(buf, from_what(WARN_OF_MON));
#else
        Sprintf(buf, "%sの存在を感じる能力",
                (context.warntype.obj & M2_ORC) ? "オーク"
                : (context.warntype.obj & M2_ELF) ? "エルフ"
                : (context.warntype.obj & M2_DEMON) ? "悪魔" : something);
        you_have(buf, "");
#endif
    }
    if (Warn_of_mon && context.warntype.polyd) {
#if 0 /*JP*/
        Sprintf(buf, "aware of the presence of %s",
                ((context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF))
                    ? "humans and elves"
                    : (context.warntype.polyd & M2_HUMAN)
                          ? "humans"
                          : (context.warntype.polyd & M2_ELF)
                                ? "elves"
                                : (context.warntype.polyd & M2_ORC)
                                      ? "orcs"
                                      : (context.warntype.polyd & M2_DEMON)
                                            ? "demons"
                                            : "certain monsters");
        you_are(buf, "");
#else
        Sprintf(buf, "%sの存在を感じる能力",
                ((context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF))
                    ? "人間とエルフ"
                    : (context.warntype.polyd & M2_HUMAN)
                          ? "人間"
                          : (context.warntype.polyd & M2_ELF)
                                ? "エルフ"
                                : (context.warntype.polyd & M2_ORC)
                                      ? "オーク"
                                      : (context.warntype.polyd & M2_DEMON)
                                            ? "悪魔"
                                            : "ある種の怪物");
        you_have(buf, "");
#endif
    }
    if (Warn_of_mon && context.warntype.speciesidx) {
#if 0 /*JP*/
        Sprintf(buf, "aware of the presence of %s",
                makeplural(mons[context.warntype.speciesidx].mname));
        you_are(buf, from_what(WARN_OF_MON));
#else
        Sprintf(buf, "%sの存在を感じる能力",
                mons[context.warntype.speciesidx].mname);
        you_have(buf, from_what(WARN_OF_MON));
#endif
    }
    if (Undead_warning)
/*JP
        you_are("warned of undead", from_what(WARN_UNDEAD));
*/
        you_have("不死の生物への警戒能力", from_what(WARN_UNDEAD));
    if (Searching)
/*JP
        you_have("automatic searching", from_what(SEARCHING));
*/
        you_have("探査能力", from_what(SEARCHING));
    if (Clairvoyant)
/*JP
        you_are("clairvoyant", from_what(CLAIRVOYANT));
*/
        you_have("千里眼能力", from_what(CLAIRVOYANT));
    else if ((HClairvoyant || EClairvoyant) && BClairvoyant) {
        Strcpy(buf, from_what(-CLAIRVOYANT));
#if 0 /*JP*/
        if (!strncmp(buf, " because of ", 12))
            /* overwrite substring; strncpy doesn't add terminator */
            (void) strncpy(buf, " if not for ", 12);
        enl_msg(You_, "could be", "could have been", " clairvoyant", buf);
#else
        /*JP:「…によって」*/
        if (!strncmp(buf, "によって", 8))
            /*JP:「…がなければ」に書き換える*/
            strcpy(eos(buf) - 8, "がなければ");
        you_have("千里眼能力", buf);
#endif
    }
    if (Infravision)
/*JP
        you_have("infravision", from_what(INFRAVISION));
*/
        you_have("赤外線が見える視覚", from_what(INFRAVISION));
    if (Detect_monsters)
/*JP
        you_are("sensing the presence of monsters", "");
*/
        you_have("怪物を探す能力", "");
    if (u.umconf)
/*JP
        you_are("going to confuse monsters", "");
*/
        you_have("怪物を混乱させる能力", "");

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if (uleft && uleft->otyp == RIN_ADORNMENT)
            adorn += uleft->spe;
        if (uright && uright->otyp == RIN_ADORNMENT)
            adorn += uright->spe;
        /* the sum might be 0 (+0 ring or two which negate each other);
           that yields "you are charismatic" (which isn't pointless
           because it potentially impacts seduction attacks) */
#if 0 /*JP*/
        Sprintf(buf, "%scharismatic",
                (adorn > 0) ? "more " : (adorn < 0) ? "less " : "");
        you_are(buf, from_what(ADORNED));
#else
        Sprintf(buf, "魅力%sて",
                (adorn > 0) ? "が増加し" : (adorn < 0) ? "が減少し" : "的になっ");
        enl_msg(You_, "ている", "た", buf, "");
#endif
    }
    if (Invisible)
/*JP
        you_are("invisible", from_what(INVIS));
*/
        you_are("透明", from_what(INVIS));
    else if (Invis)
/*JP
        you_are("invisible to others", from_what(INVIS));
*/
        you_are("他人に対して透明", from_what(INVIS));
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis) && BInvis)
/*JP
        you_are("visible", from_what(-INVIS));
*/
        you_are("不透明", from_what(-INVIS));
    if (Displaced)
/*JP
        you_are("displaced", from_what(DISPLACED));
*/
        you_have("幻影能力", from_what(DISPLACED));
    if (Stealth)
/*JP
        you_are("stealthy", from_what(STEALTH));
*/
        you_have("人目を盗む能力", from_what(STEALTH));
    if (Aggravate_monster)
#if 0 /*JP*/
        enl_msg("You aggravate", "", "d", " monsters",
                from_what(AGGRAVATE_MONSTER));
#else
        you_are_ing("反感をかって", from_what(AGGRAVATE_MONSTER));
#endif
    if (Conflict)
/*JP
        enl_msg("You cause", "", "d", " conflict", from_what(CONFLICT));
*/
        you_are_ing("闘争を引き起こして", from_what(CONFLICT));

    /*** Transportation ***/
    if (Jumping)
/*JP
        you_can("jump", from_what(JUMPING));
*/
        you_can("跳躍することが", from_what(JUMPING));
    if (Teleportation)
/*JP
        you_can("teleport", from_what(TELEPORT));
*/
        you_can("瞬間移動が", from_what(TELEPORT));
    if (Teleport_control)
/*JP
        you_have("teleport control", from_what(TELEPORT_CONTROL));
*/
        you_have("瞬間移動の制御能力", from_what(TELEPORT_CONTROL));
    /* actively levitating handled earlier as a status condition */
    if (BLevitation) { /* levitation is blocked */
        long save_BLev = BLevitation;

        BLevitation = 0L;
        if (Levitation)
#if 0 /*JP*/
            enl_msg(You_, "would levitate", "would have levitated",
                    if_surroundings_permitted, "");
#else
            you_are("浮遊状態", "");
#endif
        BLevitation = save_BLev;
    }
    /* actively flying handled earlier as a status condition */
    if (BFlying) { /* flight is blocked */
        long save_BFly = BFlying;

        BFlying = 0L;
        if (Flying)
#if 0 /*JP*/
            enl_msg(You_, "would fly", "would have flown",
                    Levitation
                       ? "if you weren't levitating"
                       : (save_BFly == FROMOUTSIDE)
                          ? if_surroundings_permitted
                          /* both surroundings and [latent] levitation */
                          : " if circumstances permitted",
                    "");
#else
            you_can("飛ぶことが", "");
#endif
        BFlying = save_BFly;
    }
    /* actively walking on water handled earlier as a status condition */
    if (Wwalking && !walking_on_water())
/*JP
        you_can("walk on water", from_what(WWALKING));
*/
        you_can("水の上を歩くことが", from_what(WWALKING));
    /* actively swimming (in water but not under it) handled earlier */
    if (Swimming && (Underwater || !u.uinwater))
/*JP
        you_can("swim", from_what(SWIMMING));
*/
        you_can("泳ぐことが", from_what(SWIMMING));
    if (Breathless)
/*JP
        you_can("survive without air", from_what(MAGICAL_BREATHING));
*/
        you_can("空気なしで生き延びることが", from_what(MAGICAL_BREATHING));
    else if (Amphibious)
/*JP
        you_can("breathe water", from_what(MAGICAL_BREATHING));
*/
        you_can("水中で呼吸が", from_what(MAGICAL_BREATHING));
    if (Passes_walls)
/*JP
        you_can("walk through walls", from_what(PASSES_WALLS));
*/
        you_can("壁を通り抜けることが", from_what(PASSES_WALLS));

    /*** Physical attributes ***/
    if (Regeneration)
/*JP
        enl_msg("You regenerate", "", "d", "", from_what(REGENERATION));
*/
        you_have("再生能力", from_what(REGENERATION));
    if (Slow_digestion)
/*JP
        you_have("slower digestion", from_what(SLOW_DIGESTION));
*/
        enl_msg("食物の消化が遅", "い", "かった", "", from_what(SLOW_DIGESTION));
    if (u.uhitinc)
/*JP
        you_have(enlght_combatinc("to hit", u.uhitinc, final, buf), "");
*/
        you_have(enlght_combatinc("命中率", u.uhitinc, final, buf), "");
    if (u.udaminc)
/*JP
        you_have(enlght_combatinc("damage", u.udaminc, final, buf), "");
*/
        you_have(enlght_combatinc("ダメージ", u.udaminc, final, buf), "");
    if (u.uspellprot || Protection) {
        int prot = 0;

        if (uleft && uleft->otyp == RIN_PROTECTION)
            prot += uleft->spe;
        if (uright && uright->otyp == RIN_PROTECTION)
            prot += uright->spe;
        if (HProtection & INTRINSIC)
            prot += u.ublessed;
        prot += u.uspellprot;
        if (prot)
/*JP
            you_have(enlght_combatinc("defense", prot, final, buf), "");
*/
            you_have(enlght_combatinc("防御", prot, final, buf), "");
    }
    if ((armpro = magic_negation(&youmonst)) > 0) {
        /* magic cancellation factor, conferred by worn armor */
        static const char *const mc_types[] = {
#if 0 /*JP*/
            "" /*ordinary*/, "warded", "guarded", "protected",
#else
            "" /*ordinary*/, "衛られて", "護られて", "守られて",
#endif
        };
        /* sanity check */
        if (armpro >= SIZE(mc_types))
            armpro = SIZE(mc_types) - 1;
/*JP
        you_are(mc_types[armpro], "");
*/
        you_are_ing(mc_types[armpro], "");
    }
    if (Half_physical_damage)
        enlght_halfdmg(HALF_PHDAM, final);
    if (Half_spell_damage)
        enlght_halfdmg(HALF_SPDAM, final);
    /* polymorph and other shape change */
    if (Protection_from_shape_changers)
#if 0 /*JP*/
        you_are("protected from shape changers",
                from_what(PROT_FROM_SHAPE_CHANGERS));
#else
        you_have("変化怪物への耐性", from_what(PROT_FROM_SHAPE_CHANGERS));
#endif
    if (Unchanging) {
        const char *what = 0;

        if (!Upolyd) /* Upolyd handled below after current form */
/*JP
            you_can("not change from your current form",
*/
            you_are("現在の姿から変化できない状態",
                    from_what(UNCHANGING));
        /* blocked shape changes */
        if (Polymorph)
/*JP
            what = !final ? "polymorph" : "have polymorphed";
*/
            what = "変化して";
        else if (u.ulycn >= LOW_PM)
/*JP
            what = !final ? "change shape" : "have changed shape";
*/
            what = "姿を変えて";
        if (what) {
#if 0 /*JP*/
            Sprintf(buf, "would %s periodically", what);
            /* omit from_what(UNCHANGING); too verbose */
            enl_msg(You_, buf, buf, " if not locked into your current form",
                    "");
#else
            Sprintf(buf, "もし現在の姿に固定されていなければ定期的に%s", what);
            you_are_ing(buf, "");
#endif
        }
    } else if (Polymorph) {
/*JP
        you_are("polymorphing periodically", from_what(POLYMORPH));
*/
        you_are("定期的に変化して", from_what(POLYMORPH));
    }
    if (Polymorph_control)
/*JP
        you_have("polymorph control", from_what(POLYMORPH_CONTROL));
*/
        you_have("変化の制御能力", from_what(POLYMORPH_CONTROL));
    if (Upolyd && u.umonnum != u.ulycn) {
        /* foreign shape (except were-form which is handled below) */
/*JP
        Sprintf(buf, "polymorphed into %s", an(youmonst.data->mname));
*/
        Sprintf(buf, "%sに変化して", youmonst.data->mname);
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.mtimedone);
/*JP
        you_are(buf, "");
*/
        you_are_ing(buf, "");
    }
    if (lays_eggs(youmonst.data) && flags.female) /* Upolyd */
/*JP
        you_can("lay eggs", "");
*/
        you_can("卵を産むことが", "");
    if (u.ulycn >= LOW_PM) {
#if 0 /*JP*/
        /* "you are a werecreature [in beast form]" */
        Strcpy(buf, an(mons[u.ulycn].mname));
        if (u.umonnum == u.ulycn) {
            Strcat(buf, " in beast form");
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.mtimedone);
        }
#else
        /*JP:「あなたは[獣の姿の]○○人間である」*/
        buf[0] = '\0';
        if (u.umonnum == u.ulycn) {
            Strcpy(buf, "獣の姿の");
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.mtimedone);
        }
        Strcat(buf, mons[u.ulycn].mname);
#endif
        you_are(buf, "");
    }
    if (Unchanging && Upolyd) /* !Upolyd handled above */
/*JP
        you_can("not change from your current form", from_what(UNCHANGING));
*/
        enl_msg("今の姿から変化することができな", "い", "かった", "", from_what(UNCHANGING));
    if (Hate_silver)
        you_are("harmed by silver", "");
    /* movement and non-armor-based protection */
    if (Fast)
/*JP
        you_are(Very_fast ? "very fast" : "fast", from_what(FAST));
*/
        you_have(Very_fast ? "とても素早く行動する能力" : "素早く行動する能力", from_what(FAST));
    if (Reflecting)
/*JP
        you_have("reflection", from_what(REFLECTING));
*/
        you_have("反射能力", from_what(REFLECTING));
    if (Free_action)
/*JP
        you_have("free action", from_what(FREE_ACTION));
*/
        you_have("拘束されない能力", from_what(FREE_ACTION));
    if (Fixed_abil)
/*JP
        you_have("fixed abilities", from_what(FIXED_ABIL));
*/
        enl_msg("能力が変化しな", "い", "かった", "", from_what(FIXED_ABIL));
    if (Lifesaved)
/*JP
        enl_msg("Your life ", "will be", "would have been", " saved", "");
*/
        enl_msg("あなたの生命は保存されて", iru, ita, "", "");

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int) Luck);
#if 0 /*JP*/
        Sprintf(buf, "%s%slucky",
                ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
                Luck < 0 ? "un" : "");
#else
        Sprintf(buf, "%s%s",
                ltmp >= 10 ? "猛烈に" : ltmp >= 5 ? "とても" : "",
                Luck < 0 ? "不幸" : "幸福");
#endif
        if (wizard)
            Sprintf(eos(buf), " (%d)", Luck);
        you_are(buf, "");
    } else if (wizard)
/*JP
        enl_msg("Your luck ", "is", "was", " zero", "");
*/
        enl_msg("あなたの運はゼロ", "である", "だった", "", "");
    if (u.moreluck > 0)
/*JP
        you_have("extra luck", "");
*/
        you_have("さらなる幸運", "");
    else if (u.moreluck < 0)
/*JP
        you_have("reduced luck", "");
*/
        you_have("さらなる不運", "");
    if (carrying(LUCKSTONE) || stone_luck(TRUE)) {
        ltmp = stone_luck(0);
        if (ltmp <= 0)
/*JP
            enl_msg("Bad luck ", "does", "did", " not time out for you", "");
*/
            enl_msg("不運は時間切れにならな", "い", "かった", "", "");
        if (ltmp >= 0)
/*JP
            enl_msg("Good luck ", "does", "did", " not time out for you", "");
*/
            enl_msg("幸運は時間切れにならな", "い", "かった", "", "");
    }

    if (u.ugangr) {
#if 0 /*JP*/
        Sprintf(buf, " %sangry with you",
                u.ugangr > 6 ? "extremely " : u.ugangr > 3 ? "very " : "");
#else
        Sprintf(buf, "%sは%s怒って%s", u_gname(),
                u.ugangr > 6 ? "猛烈に" : u.ugangr > 3 ? "とても" : "", final ? ita : iru);
#endif
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.ugangr);
#if 0 /*JP*/
        enl_msg(u_gname(), " is", " was", buf, "");
#else
        enl_msg(buf, "", "", "", "");
#endif
    } else {
        /*
         * We need to suppress this when the game is over, because death
         * can change the value calculated by can_pray(), potentially
         * resulting in a false claim that you could have prayed safely.
         */
        if (!final) {
#if 0 /*JP*/
#if 0
            /* "can [not] safely pray" vs "could [not] have safely prayed" */
            Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
                    final ? "have " : "", final ? "ed" : "");
#else
            Sprintf(buf, "%ssafely pray", can_pray(FALSE) ? "" : "not ");
#endif
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.ublesscnt);
            you_can(buf, "");
#else /*JP*/
            Sprintf(buf, "あなたは安全に祈ることが");
            Strcat(buf, can_pray(FALSE) ? can : "できない");
            if (wizard)
              Sprintf(eos(buf), " (%d)", u.ublesscnt);
            enl_msg(buf, "", "", "", "");
#endif
        }
    }

    /* named fruit debugging (doesn't really belong here...) */
    if (wizard) {
        int fcount = 0;
        struct fruit *f;
        char buf2[BUFSZ];

        for (f = ffruit; f; f = f->nextf) {
/*JP
            Sprintf(buf, "Fruit %d ", ++fcount);
*/
            Sprintf(buf, "fruit %d は", ++fcount);
            Sprintf(buf2, "%s (id %d)", f->fname, f->fid);
/*JP
            enl_msg(buf, "is ", "was ", buf2, "");
*/
            enl_msg(buf, "だ", "だった", buf2, "");
        }
/*JP
        enl_msg("The current fruit ", "is ", "was ", pl_fruit, "");
*/
        enl_msg("現在の fruit は", "だ", "だった", pl_fruit, "");
        Sprintf(buf, "%d", flags.made_fruit);
/*JP
        enl_msg("The made fruit flag ", "is ", "was ", buf, "");
*/
        enl_msg("made fruit flag は", "だ", "だった", buf, "");
    }

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) { /* still in progress, or quit/escaped/ascended */
/*JP
            p = "survived after being killed ";
*/
            p = "死んだ後復活していた";
            switch (u.umortality) {
            case 0:
/*JP
                p = !final ? (char *) 0 : "survived";
*/
                p = !final ? (char *)0 : "生き延びた";
                break;
            case 1:
/*JP
                Strcpy(buf, "once");
*/
                Strcpy(buf, "一度");
                break;
            case 2:
/*JP
                Strcpy(buf, "twice");
*/
                Strcpy(buf, "二度");
                break;
            case 3:
/*JP
                Strcpy(buf, "thrice");
*/
                Strcpy(buf, "三度");
                break;
            default:
/*JP
                Sprintf(buf, "%d times", u.umortality);
*/
                Sprintf(buf, "%d回", u.umortality);
                break;
            }
        } else { /* game ended in character's death */
/*JP
            p = "are dead";
*/
            p = "死んでいる";
            switch (u.umortality) {
            case 0:
                impossible("dead without dying?");
            case 1:
                break; /* just "are dead" */
            default:
#if 0 /*JP*/
                Sprintf(buf, " (%d%s time!)", u.umortality,
                        ordin(u.umortality));
#else
                 Sprintf(buf, "(%d回！)", u.umortality);
#endif
                break;
            }
        }
        if (p)
/*JP
            enl_msg(You_, "have been killed ", p, buf, "");
*/
            enl_msg(You_, "死んでいる", p, buf, "");
    }
}

#if 0  /* no longer used */
STATIC_DCL boolean NDECL(minimal_enlightenment);

/*
 * Courtesy function for non-debug, non-explorer mode players
 * to help refresh them about who/what they are.
 * Returns FALSE if menu cancelled (dismissed with ESC), TRUE otherwise.
 */
STATIC_OVL boolean
minimal_enlightenment()
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    int genidx, n;
    char buf[BUFSZ], buf2[BUFSZ];
    static const char untabbed_fmtstr[] = "%-15s: %-12s";
    static const char untabbed_deity_fmtstr[] = "%-17s%s";
    static const char tabbed_fmtstr[] = "%s:\t%-12s";
    static const char tabbed_deity_fmtstr[] = "%s\t%s";
    static const char *fmtstr;
    static const char *deity_fmtstr;

    fmtstr = iflags.menu_tab_sep ? tabbed_fmtstr : untabbed_fmtstr;
    deity_fmtstr = iflags.menu_tab_sep ? tabbed_deity_fmtstr
                                       : untabbed_deity_fmtstr;
    any = zeroany;
    buf[0] = buf2[0] = '\0';
    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
/*JP
             "Starting", FALSE);
*/
             "開始", FALSE);

    /* Starting name, race, role, gender */
/*JP
    Sprintf(buf, fmtstr, "name", plname);
*/
    Sprintf(buf, fmtstr, "名前", plname);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
/*JP
    Sprintf(buf, fmtstr, "race", urace.noun);
*/
    Sprintf(buf, fmtstr, "種族", urace.noun);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
/*JP
    Sprintf(buf, fmtstr, "role",
*/
    Sprintf(buf, fmtstr, "職業",
            (flags.initgend && urole.name.f) ? urole.name.f : urole.name.m);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
/*JP
    Sprintf(buf, fmtstr, "gender", genders[flags.initgend].adj);
*/
    Sprintf(buf, fmtstr, "性別", genders[flags.initgend].adj);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Starting alignment */
/*JP
    Sprintf(buf, fmtstr, "alignment", align_str(u.ualignbase[A_ORIGINAL]));
*/
    Sprintf(buf, fmtstr, "属性", align_str(u.ualignbase[A_ORIGINAL]));
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Current name, race, role, gender */
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", FALSE);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
/*JP
             "Current", FALSE);
*/
             "現在", FALSE);
/*JP
    Sprintf(buf, fmtstr, "race", Upolyd ? youmonst.data->mname : urace.noun);
*/
    Sprintf(buf, fmtstr, "種族", Upolyd ? youmonst.data->mname : urace.noun);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
    if (Upolyd) {
/*JP
        Sprintf(buf, fmtstr, "role (base)",
*/
        Sprintf(buf, fmtstr, "職業(基本)",
                (u.mfemale && urole.name.f) ? urole.name.f
                                            : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
    } else {
/*JP
        Sprintf(buf, fmtstr, "role",
*/
        Sprintf(buf, fmtstr, "職業",
                (flags.female && urole.name.f) ? urole.name.f
                                               : urole.name.m);
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
    }
    /* don't want poly_gender() here; it forces `2' for non-humanoids */
    genidx = is_neuter(youmonst.data) ? 2 : flags.female;
/*JP
    Sprintf(buf, fmtstr, "gender", genders[genidx].adj);
*/
    Sprintf(buf, fmtstr, "性別", genders[genidx].adj);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
    if (Upolyd && (int) u.mfemale != genidx) {
/*JP
        Sprintf(buf, fmtstr, "gender (base)", genders[u.mfemale].adj);
*/
        Sprintf(buf, fmtstr, "性別(基本)", genders[u.mfemale].adj);
        add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
    }

    /* Current alignment */
/*JP
    Sprintf(buf, fmtstr, "alignment", align_str(u.ualign.type));
*/
    Sprintf(buf, fmtstr, "属性", align_str(u.ualign.type));
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

    /* Deity list */
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", FALSE);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
/*JP
             "Deities", FALSE);
*/
             "神", FALSE);
#if 0 /*JP*/
    Sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_CHAOTIC)               ? " (s,c)"
                : (u.ualignbase[A_ORIGINAL] == A_CHAOTIC) ? " (s)"
                : (u.ualign.type   == A_CHAOTIC)          ? " (c)" : "");
#else
    Sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_CHAOTIC)               ? " (初，現)"
                : (u.ualignbase[A_ORIGINAL] == A_CHAOTIC) ? " (初)"
                : (u.ualign.type   == A_CHAOTIC)          ? " (現)" : "");
#endif
/*JP
    Sprintf(buf, fmtstr, "Chaotic", buf2);
*/
    Sprintf(buf, fmtstr, "混沌", buf2);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

#if 0 /*JP*/
    Sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_NEUTRAL)               ? " (s,c)"
                : (u.ualignbase[A_ORIGINAL] == A_NEUTRAL) ? " (s)"
                : (u.ualign.type   == A_NEUTRAL)          ? " (c)" : "");
#else
    Sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_NEUTRAL)               ? " (初，現)"
                : (u.ualignbase[A_ORIGINAL] == A_NEUTRAL) ? " (初)"
                : (u.ualign.type   == A_NEUTRAL)          ? " (現)" : "");
#endif
/*JP
    Sprintf(buf, fmtstr, "Neutral", buf2);
*/
    Sprintf(buf, fmtstr, "中立", buf2);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

#if 0 /*JP*/
    Sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_LAWFUL)                ? " (s,c)"
                : (u.ualignbase[A_ORIGINAL] == A_LAWFUL)  ? " (s)"
                : (u.ualign.type   == A_LAWFUL)           ? " (c)" : "");
#else
    Sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
            (u.ualignbase[A_ORIGINAL] == u.ualign.type
             && u.ualign.type == A_LAWFUL)                ? " (初，現)"
                : (u.ualignbase[A_ORIGINAL] == A_LAWFUL)  ? " (初)"
                : (u.ualign.type   == A_LAWFUL)           ? " (現)" : "");
#endif
/*JP
    Sprintf(buf, fmtstr, "Lawful", buf2);
*/
    Sprintf(buf, fmtstr, "秩序", buf2);
    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

/*JP
    end_menu(tmpwin, "Base Attributes");
*/
    end_menu(tmpwin, "基本属性");
    n = select_menu(tmpwin, PICK_NONE, &selected);
    destroy_nhwindow(tmpwin);
    return (boolean) (n != -1);
}
#endif /*0*/

/* ^X command */
STATIC_PTR int
doattributes(VOID_ARGS)
{
    int mode = BASICENLIGHTENMENT;

    /* show more--as if final disclosure--for wizard and explore modes */
    if (wizard || discover)
        mode |= MAGICENLIGHTENMENT;

    enlightenment(mode, ENL_GAMEINPROGRESS);
    return 0;
}

void
youhiding(via_enlghtmt, msgflag)
boolean via_enlghtmt; /* englightment line vs topl message */
int msgflag;          /* for variant message phrasing */
{
    char *bp, buf[BUFSZ];

/*JP
    Strcpy(buf, "hiding");
*/
    Strcpy(buf, "隠れ");
    if (youmonst.m_ap_type != M_AP_NOTHING) {
        /* mimic; hero is only able to mimic a strange object or gold
           or hallucinatory alternative to gold, so we skip the details
           for the hypothetical furniture and monster cases */
#if 0 /*JP*//*後ろに回す*/
        bp = eos(strcpy(buf, "mimicking"));
#endif
        if (youmonst.m_ap_type == M_AP_OBJECT) {
/*JP
            Sprintf(bp, " %s", an(simple_typename(youmonst.mappearance)));
*/
            Strcpy(buf, simple_typename(youmonst.mappearance));
        } else if (youmonst.m_ap_type == M_AP_FURNITURE) {
/*JP
            Strcpy(bp, " something");
*/
            Strcpy(buf, "何か");
        } else if (youmonst.m_ap_type == M_AP_MONSTER) {
/*JP
            Strcpy(bp, " someone");
*/
            Strcpy(buf, "何者か");
        } else {
            ; /* something unexpected; leave 'buf' as-is */
        }
#if 1 /*JP*//*ここで追加*/
        Strcat(buf, "のふりをし");
#endif
    } else if (u.uundetected) {
        bp = eos(buf); /* points past "hiding" */
        if (youmonst.data->mlet == S_EEL) {
            if (is_pool(u.ux, u.uy))
/*JP
                Sprintf(bp, " in the %s", waterbody_name(u.ux, u.uy));
*/
                Sprintf(bp, "%sの中に", waterbody_name(u.ux, u.uy));
        } else if (hides_under(youmonst.data)) {
            struct obj *o = level.objects[u.ux][u.uy];

            if (o)
/*JP
                Sprintf(bp, " underneath %s", ansimpleoname(o));
*/
                Sprintf(bp, "%sの下に", ansimpleoname(o));
        } else if (is_clinger(youmonst.data) || Flying) {
            /* Flying: 'lurker above' hides on ceiling but doesn't cling */
/*JP
            Sprintf(bp, " on the %s", ceiling(u.ux, u.uy));
*/
            Sprintf(bp, "%sに", ceiling(u.ux, u.uy));
        } else {
            /* on floor; is_hider() but otherwise not special: 'trapper' */
            if (u.utrap && u.utraptype == TT_PIT) {
                struct trap *t = t_at(u.ux, u.uy);

#if 0 /*JP*/
                Sprintf(bp, " in a %spit",
                        (t && t->ttyp == SPIKED_PIT) ? "spiked " : "");
#else
                Sprintf(bp, "%s落し穴の中に",
                        (t && t->ttyp == SPIKED_PIT) ? "トゲだらけの" : "");
#endif
            } else
/*JP
                Sprintf(bp, " on the %s", surface(u.ux, u.uy));
*/
                Sprintf(bp, "%sに", surface(u.ux, u.uy));
        }
#if 1 /*JP*//*ここで追加*/
        Strcat(buf, "隠れ");
#endif
    } else {
        ; /* shouldn't happen; will result in generic "you are hiding" */
    }

    if (via_enlghtmt) {
        int final = msgflag; /* 'final' is used by you_are() macro */

/*JP
        you_are(buf, "");
*/
        enl_msg(buf, "ている", "ていた", "", "");
    } else {
        /* for dohide(), when player uses '#monster' command */
#if 0 /*JP*/
        You("are %s %s.", msgflag ? "already" : "now", buf);
#else
        if (msgflag) {
            You("すでに%sている．", buf);
        } else {
            You("%sた．", buf);
        }
#endif
    }
}

/* KMH, #conduct
 * (shares enlightenment's tense handling)
 */
STATIC_PTR int
doconduct(VOID_ARGS)
{
    show_conduct(0);
    return 0;
}

void
show_conduct(final)
int final;
{
    char buf[BUFSZ];
    int ngenocided;

    /* Create the conduct window */
    en_win = create_nhwindow(NHW_MENU);
/*JP
    putstr(en_win, 0, "Voluntary challenges:");
*/
    putstr(en_win, 0, "自発的挑戦:");

    if (u.uroleplay.blind)
/*JP
        you_have_been("blind from birth");
*/
        you_have_been("生まれながらに盲目");
    if (u.uroleplay.nudist)
/*JP
        you_have_been("faithfully nudist");
*/
        you_have_been("忠実な裸族");

    if (!u.uconduct.food)
/*JP
        enl_msg(You_, "have gone", "went", " without food", "");
*/
        enl_msg("あなたは食事をし", "ていない", "なかった", "", "");
    /* But beverages are okay */
    else if (!u.uconduct.unvegan)
/*JP
        you_have_X("followed a strict vegan diet");
*/
        you_have_been("厳格な菜食主義者");
    else if (!u.uconduct.unvegetarian)
/*JP
        you_have_been("vegetarian");
*/
        you_have_been("菜食主義者");

    if (!u.uconduct.gnostic)
/*JP
        you_have_been("an atheist");
*/
        you_have_been("無神論者");

    if (!u.uconduct.weaphit) {
/*JP
        you_have_never("hit with a wielded weapon");
*/
        you_have_never("あなたは装備している武器で攻撃し");
    } else if (wizard) {
#if 0 /*JP*/
        Sprintf(buf, "used a wielded weapon %ld time%s", u.uconduct.weaphit,
                plur(u.uconduct.weaphit));
        you_have_X(buf);
#else
        Sprintf(buf, "あなたは%ld回装備した武器を使用し", u.uconduct.weaphit);
        you_have_X(buf);
#endif
    }
    if (!u.uconduct.killer)
/*JP
        you_have_been("a pacifist");
*/
        you_have_been("平和主義者");

    if (!u.uconduct.literate) {
/*JP
        you_have_been("illiterate");
*/
        you_have_never("あなたは読み書きし");
    } else if (wizard) {
#if 0 /*JP*/
        Sprintf(buf, "read items or engraved %ld time%s", u.uconduct.literate,
                plur(u.uconduct.literate));
        you_have_X(buf);
#else
        Sprintf(buf, "%ld回読んだり書いたりし", u.uconduct.literate);
        you_have_X(buf);
#endif
    }

    ngenocided = num_genocides();
    if (ngenocided == 0) {
/*JP
        you_have_never("genocided any monsters");
*/
        you_have_never("あなたは怪物を虐殺し");
    } else {
#if 0 /*JP*/
        Sprintf(buf, "genocided %d type%s of monster%s", ngenocided,
                plur(ngenocided), plur(ngenocided));
        you_have_X(buf);
#else
        Sprintf(buf, "%d種の怪物を虐殺し", ngenocided);
        you_have_X(buf);
#endif
    }

    if (!u.uconduct.polypiles) {
/*JP
        you_have_never("polymorphed an object");
*/
        you_have_never("あなたは物体を変化させ");
    } else if (wizard) {
#if 0 /*JP*/
        Sprintf(buf, "polymorphed %ld item%s", u.uconduct.polypiles,
                plur(u.uconduct.polypiles));
        you_have_X(buf);
#else
        Sprintf(buf, "%ld個の道具を変化させ", u.uconduct.polypiles);
        you_have_X(buf);
#endif
    }

    if (!u.uconduct.polyselfs) {
/*JP
        you_have_never("changed form");
*/
        you_have_never("あなたは変化し");
    } else if (wizard) {
#if 0 /*JP*/
        Sprintf(buf, "changed form %ld time%s", u.uconduct.polyselfs,
                plur(u.uconduct.polyselfs));
        you_have_X(buf);
#else
        Sprintf(buf, "%ld回姿を変え", u.uconduct.polyselfs);
        you_have_X(buf);
#endif
    }

    if (!u.uconduct.wishes) {
/*JP
        you_have_X("used no wishes");
*/
        you_have_never("あなたは願い事をし");
    } else {
#if 0 /*JP*/
        Sprintf(buf, "used %ld wish%s", u.uconduct.wishes,
                (u.uconduct.wishes > 1L) ? "es" : "");
        you_have_X(buf);
#else
        Sprintf(buf, "%ld回願い事をし", u.uconduct.wishes);
        you_have_X(buf);
#endif

        if (!u.uconduct.wisharti)
#if 0 /*JP*/
            enl_msg(You_, "have not wished", "did not wish",
                    " for any artifacts", "");
#else
            enl_msg("あなたは聖器を願", "っていない", "わなかった", "", "");
#endif
    }

    /* Pop up the window and wait for a key */
    display_nhwindow(en_win, TRUE);
    destroy_nhwindow(en_win);
    en_win = WIN_ERR;
}

#ifndef M
#ifndef NHSTDC
#define M(c) (0x80 | (c))
#else
#define M(c) ((c) -128)
#endif /* NHSTDC */
#endif
#ifndef C
#define C(c) (0x1f & (c))
#endif

static const struct func_tab cmdlist[] = {
    { C('d'), FALSE, dokick }, /* "D" is for door!...?  Msg is in dokick.c */
    { C('e'), TRUE, wiz_detect },
    { C('f'), TRUE, wiz_map },
    { C('g'), TRUE, wiz_genesis },
    { C('i'), TRUE, wiz_identify },
    { C('l'), TRUE, doredraw },    /* if number_pad is set */
    { C('n'), TRUE, donamelevel }, /* if number_pad is set */
    { C('o'), TRUE, dooverview_or_wiz_where }, /* depends on wizard status */
    { C('p'), TRUE, doprev_message },
    { C('r'), TRUE, doredraw },
    { C('t'), TRUE, dotele },
    { C('v'), TRUE, wiz_level_tele },
    { C('w'), TRUE, wiz_wish },
    { C('x'), TRUE, doattributes },
    { C('z'), TRUE, dosuspend_core },
    { 'a', FALSE, doapply },
    { 'A', FALSE, doddoremarm },
    { M('a'), TRUE, doorganize },
    { M('A'), TRUE, donamelevel }, /* #annotate */
    /*  'b', 'B' : go sw */
    { 'c', FALSE, doclose },
    { 'C', TRUE, docallcmd },
    { M('c'), TRUE, dotalk },
    { M('C'), TRUE, doconduct }, /* #conduct */
    { 'd', FALSE, dodrop },
    { 'D', FALSE, doddrop },
    { M('d'), FALSE, dodip },
    { 'e', FALSE, doeat },
    { 'E', FALSE, doengrave },
    { M('e'), TRUE, enhance_weapon_skill },
    { 'f', FALSE, dofire },
    /*  'F' : fight (one time) */
    { M('f'), FALSE, doforce },
    /*  'g', 'G' : multiple go */
    /*  'h', 'H' : go west */
    { 'h', TRUE, dohelp }, /* if number_pad is set */
    { 'i', TRUE, ddoinv },
    { 'I', TRUE, dotypeinv }, /* Robert Viduya */
    { M('i'), TRUE, doinvoke },
    /*  'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' : move commands */
    { 'j', FALSE, dojump }, /* if number_pad is on */
    { M('j'), FALSE, dojump },
    { 'k', FALSE, dokick }, /* if number_pad is on */
    { 'l', FALSE, doloot }, /* if number_pad is on */
    { M('l'), FALSE, doloot },
    /*  'n' prefixes a count if number_pad is on */
    { M('m'), TRUE, domonability },
    { 'N', TRUE, docallcmd }, /* if number_pad is on */
    { M('n'), TRUE, docallcmd },
    { M('N'), TRUE, docallcmd },
    { 'o', FALSE, doopen },
    { 'O', TRUE, doset },
    { M('o'), FALSE, dosacrifice },
    { M('O'), TRUE, dooverview }, /* #overview */
    { 'p', FALSE, dopay },
    { 'P', FALSE, doputon },
    { M('p'), TRUE, dopray },
    { 'q', FALSE, dodrink },
    { 'Q', FALSE, dowieldquiver },
    { M('q'), TRUE, done2 },
    { 'r', FALSE, doread },
    { 'R', FALSE, doremring },
    { M('r'), FALSE, dorub },
    { M('R'), FALSE, doride }, /* #ride */
/*JP
    { 's', TRUE, dosearch, "searching" },
*/
    { 's', TRUE, dosearch, "捜す" },
    { 'S', TRUE, dosave },
    { M('s'), FALSE, dosit },
    { 't', FALSE, dothrow },
    { 'T', FALSE, dotakeoff },
    { M('t'), TRUE, doturn },
    { M('T'), FALSE, dotip }, /* #tip */
    /*  'u', 'U' : go ne */
    { 'u', FALSE, dountrap }, /* if number_pad is on */
    { M('u'), FALSE, dountrap },
    { 'v', TRUE, doversion },
    { 'V', TRUE, dohistory },
    { M('v'), TRUE, doextversion },
    { 'w', FALSE, dowield },
    { 'W', FALSE, dowear },
    { M('w'), FALSE, dowipe },
    { 'x', FALSE, doswapweapon },
    { 'X', FALSE, dotwoweapon },
    /*  'y', 'Y' : go nw */
    { 'z', FALSE, dozap },
    { 'Z', TRUE, docast },
    { '<', FALSE, doup },
    { '>', FALSE, dodown },
    { '/', TRUE, dowhatis },
    { '&', TRUE, dowhatdoes },
    { '?', TRUE, dohelp },
    { M('?'), TRUE, doextlist },
#ifdef SHELL
    { '!', TRUE, dosh },
#endif
/*JP
    { '.', TRUE, donull, "waiting" },
*/
    { '.', TRUE, donull, "休憩する" },
/*JP
    { ' ', TRUE, donull, "waiting" },
*/
    { ' ', TRUE, donull, "休憩する" },
    { ',', FALSE, dopickup },
    { ':', TRUE, dolook },
    { ';', TRUE, doquickwhatis },
    { '^', TRUE, doidtrap },
    { '\\', TRUE, dodiscovered }, /* Robert Viduya */
    { '`', TRUE, doclassdisco },
    { '@', TRUE, dotogglepickup },
    { M('2'), FALSE, dotwoweapon },
    { WEAPON_SYM, TRUE, doprwep },
    { ARMOR_SYM, TRUE, doprarm },
    { RING_SYM, TRUE, doprring },
    { AMULET_SYM, TRUE, dopramulet },
    { TOOL_SYM, TRUE, doprtool },
    { '*', TRUE, doprinuse }, /* inventory of all equipment in use */
    { GOLD_SYM, TRUE, doprgold },
    { SPBOOK_SYM, TRUE, dovspell }, /* Mike Stephenson */
    { '#', TRUE, doextcmd },
    { '_', TRUE, dotravel },
    { 0, 0, 0, 0 }
};

struct ext_func_tab extcmdlist[] = {
/*JP
    { "adjust", "adjust inventory letters", doorganize, TRUE },
*/
    { "adjust", "持ち物一覧の調整", doorganize, TRUE },
/*JP
    { "annotate", "name current level", donamelevel, TRUE },
*/
    { "annotate", "現在の階に名前をつける", donamelevel, TRUE },
#if 0 /*JP*/
    { "chat", "talk to someone", dotalk, TRUE }, /* converse? */
#else
    { "chat", "誰かと話す", dotalk, TRUE }, /* converse? */
#endif
#if 0 /*JP*/
    { "conduct", "list voluntary challenges you have maintained", doconduct,
      TRUE },
#else
    { "conduct", "どういう行動をとったか見る", doconduct, TRUE },
#endif
/*JP
    { "dip", "dip an object into something", dodip, FALSE },
*/
    { "dip", "何かに物を浸す", dodip, FALSE },
#if 0 /*JP*/
    { "enhance", "advance or check weapon and spell skills",
      enhance_weapon_skill, TRUE },
#else
    { "enhance", "武器熟練度を高める", enhance_weapon_skill, TRUE },
#endif
/*JP
    { "exploremode", "enter explore mode", enter_explore_mode, TRUE },
*/
    { "exploremode", "探検モードに入る", enter_explore_mode, TRUE },
/*JP
    { "force", "force a lock", doforce, FALSE },
*/
    { "force", "鍵をこじあける", doforce, FALSE },
/*JP
    { "invoke", "invoke an object's powers", doinvoke, TRUE },
*/
    { "invoke", "物の特別な力を使う", doinvoke, TRUE },
/*JP
    { "jump", "jump to a location", dojump, FALSE },
*/
    { "jump", "他の位置に飛びうつる", dojump, FALSE },
/*JP
    { "loot", "loot a box on the floor", doloot, FALSE },
*/
    { "loot", "床の上の箱を開ける", doloot, TRUE },
/*JP
    { "monster", "use a monster's special ability", domonability, TRUE },
*/
    { "monster", "怪物の特別能力を使う", domonability, TRUE },
/*JP
    { "name", "name a monster or an object", docallcmd, TRUE },
*/
    { "name", "アイテムや物に名前をつける", docallcmd, TRUE },
/*JP
    { "offer", "offer a sacrifice to the gods", dosacrifice, FALSE },
*/
    { "offer", "神に供物を捧げる", dosacrifice, FALSE },
/*JP
    { "overview", "show an overview of the dungeon", dooverview, TRUE },
*/
    { "overview", "迷宮の概要を表示する", dooverview, TRUE },
/*JP
    { "pray", "pray to the gods for help", dopray, TRUE },
*/
    { "pray", "神に祈る", dopray, TRUE },
/*JP
    { "quit", "exit without saving current game", done2, TRUE },
*/
    { "quit", "セーブしないで終了", done2, TRUE },
/*JP
    { "ride", "ride (or stop riding) a monster", doride, FALSE },
*/
    { "ride", "怪物に乗る(または降りる)", doride, FALSE },
/*JP
    { "rub", "rub a lamp or a stone", dorub, FALSE },
*/
    { "rub", "ランプをこする", dorub, FALSE },
/*JP
    { "sit", "sit down", dosit, FALSE },
*/
    { "sit", "座る", dosit, FALSE },
/*JP
    { "terrain", "show map without obstructions", doterrain, TRUE },
*/
    { "terrain", "邪魔されずに地図を見る", doterrain, TRUE },
/*JP
    { "tip", "empty a container", dotip, FALSE },
*/
    { "tip", "入れ物を空にする", dotip, FALSE },
/*JP
    { "turn", "turn undead", doturn, TRUE },
*/
    { "turn", "アンデットを土に返す", doturn, TRUE },
/*JP
    { "twoweapon", "toggle two-weapon combat", dotwoweapon, FALSE },
*/
    { "twoweapon", "両手持ちの切り替え", dotwoweapon, FALSE },
/*JP
    { "untrap", "untrap something", dountrap, FALSE },
*/
    { "untrap", "罠をはずす", dountrap, FALSE },
#if 0 /*JP*/
    { "version", "list compile time options for this version of NetHack",
      doextversion, TRUE },
#else
    { "version", "コンパイル時のオプションを表示する", doextversion, TRUE },
#endif
/*JP
    { "wipe", "wipe off your face", dowipe, FALSE },
*/
    { "wipe", "顔を拭う", dowipe, FALSE },
/*JP
    { "?", "get this list of extended commands", doextlist, TRUE },
*/
    { "?", "この拡張コマンド一覧を表示する", doextlist, TRUE },
    /*
     * There must be a blank entry here for every entry in the table
     * below.
     */
    { (char *) 0, (char *) 0, donull, TRUE }, /* levelchange */
    { (char *) 0, (char *) 0, donull, TRUE }, /* lightsources */
#ifdef DEBUG_MIGRATING_MONS
    { (char *) 0, (char *) 0, donull, TRUE }, /* migratemons */
#endif
    { (char *) 0, (char *) 0, donull, TRUE }, /* monpolycontrol */
    { (char *) 0, (char *) 0, donull, TRUE }, /* panic */
    { (char *) 0, (char *) 0, donull, TRUE }, /* polyself */
#ifdef PORT_DEBUG
    { (char *) 0, (char *) 0, donull, TRUE }, /* portdebug */
#endif
    { (char *) 0, (char *) 0, donull, TRUE }, /* seenv */
    { (char *) 0, (char *) 0, donull, TRUE }, /* stats */
    { (char *) 0, (char *) 0, donull, TRUE }, /* timeout */
    { (char *) 0, (char *) 0, donull, TRUE }, /* vanquished */
    { (char *) 0, (char *) 0, donull, TRUE }, /* vision */
    { (char *) 0, (char *) 0, donull, TRUE }, /* wizsmell */
#ifdef DEBUG
    { (char *) 0, (char *) 0, donull, TRUE }, /* wizdebug_traveldisplay */
    { (char *) 0, (char *) 0, donull, TRUE }, /* wizdebug_bury */
#endif
    { (char *) 0, (char *) 0, donull, TRUE }, /* wizrumorcheck */
    { (char *) 0, (char *) 0, donull, TRUE }, /* wmode */
    { (char *) 0, (char *) 0, donull, TRUE }  /* sentinel */
};

/* there must be a placeholder in the table above for every entry here */
static const struct ext_func_tab debug_extcmdlist[] = {
/*JP
    { "levelchange", "change experience level", wiz_level_change, TRUE },
*/
    { "levelchange", "経験レベルを変える", wiz_level_change, TRUE},
/*JP
    { "lightsources", "show mobile light sources", wiz_light_sources, TRUE },
*/
    { "light sources", "移動光源を見る", wiz_light_sources, TRUE},
#ifdef DEBUG_MIGRATING_MONS
/*JP
    { "migratemons", "migrate n random monsters", wiz_migrate_mons, TRUE },
*/
    { "migratemons", "ランダムな怪物を何体か移住させる", wiz_migrate_mons, TRUE },
#endif
/*JP
    { "monpolycontrol", "control monster polymorphs", wiz_mon_polycontrol,
*/
    { "monpoly_control", "怪物への変化を制御する", wiz_mon_polycontrol,
      TRUE },
/*JP
    { "panic", "test panic routine (fatal to game)", wiz_panic, TRUE },
*/
    { "panic", "パニックルーチンをテストする(致命的)", wiz_panic, TRUE},
/*JP
    { "polyself", "polymorph self", wiz_polyself, TRUE },
*/
    { "polyself", "変化する", wiz_polyself, TRUE},
#ifdef PORT_DEBUG
/*JP
    { "portdebug", "wizard port debug command", wiz_port_debug, TRUE },
*/
    { "portdebug", "ウィザードポートデバッグコマンド", wiz_port_debug, TRUE },
#endif
/*JP
    { "seenv", "show seen vectors", wiz_show_seenv, TRUE },
*/
    { "seenv", "視線ベクトルを見る", wiz_show_seenv, TRUE},
/*JP
    { "stats", "show memory statistics", wiz_show_stats, TRUE },
*/
    { "stats", "メモリ状態を見る", wiz_show_stats, TRUE},
/*JP
    { "timeout", "look at timeout queue", wiz_timeout_queue, TRUE },
*/
    { "timeout", "時間切れキューを見る", wiz_timeout_queue, TRUE},
/*JP
    { "vanquished", "list vanquished monsters", dovanquished, TRUE },
*/
    { "vanquished", "倒した怪物の一覧を見る", dovanquished, TRUE },
/*JP
    { "vision", "show vision array", wiz_show_vision, TRUE },
*/
    { "vision", "視界配列を見る", wiz_show_vision, TRUE},
/*JP
    { "wizsmell", "smell monster", wiz_smell, TRUE },
*/
    { "wizsmell", "怪物の匂いを嗅ぐ", wiz_smell, TRUE },
#ifdef DEBUG
/*JP
    { "wizdebug_traveldisplay", "wizard debug: toggle travel display",
*/
    { "wizdebug_traveldisplay", "ウィザードデバッグ: 移動表示を切り替える",
      wiz_debug_cmd_traveldisplay, TRUE },
/*JP
    { "wizdebug_bury", "wizard debug: bury objs under and around you",
*/
    { "wizdebug_bury", "ウィザードでバッグ: 物をあなたの周りに埋める",
      wiz_debug_cmd_bury, TRUE },
#endif
/*JP
    { "wizrumorcheck", "verify rumor boundaries", wiz_rumor_check, TRUE },
*/
    { "wizrumorcheck", "噂の境界を検証する", wiz_rumor_check, TRUE },
/*JP
    { "wmode", "show wall modes", wiz_show_wmodes, TRUE },
*/
    { "wmode", "壁モードを見る", wiz_show_wmodes, TRUE},
    { (char *) 0, (char *) 0, donull, TRUE }
};

/*
 * Insert debug commands into the extended command list.  This function
 * assumes that the last entry will be the help entry.
 *
 * You must add entries in ext_func_tab every time you add one to the
 * debug_extcmdlist().
 */
void
add_debug_extended_commands()
{
    int i, j, k, n;

    /* count the # of help entries */
    for (n = 0; extcmdlist[n].ef_txt[0] != '?'; n++)
        ;

    for (i = 0; debug_extcmdlist[i].ef_txt; i++) {
        /* need enough room for "?" entry plus terminator */
        if (n + 2 >= SIZE(extcmdlist))
            panic("Too many debugging commands!");
        for (j = 0; j < n; j++)
            if (strcmp(debug_extcmdlist[i].ef_txt, extcmdlist[j].ef_txt) < 0)
                break;

        /* insert i'th debug entry into extcmdlist[j], pushing down  */
        for (k = n; k >= j; --k)
            extcmdlist[k + 1] = extcmdlist[k];
        extcmdlist[j] = debug_extcmdlist[i];
        n++; /* now an extra entry */
    }
}

STATIC_OVL char
cmd_from_func(fn)
int NDECL((*fn));
{
    int i;
    for (i = 0; i < SIZE(cmdlist); ++i)
        if (cmdlist[i].f_funct == fn)
            return cmdlist[i].f_char;
    return 0;
}

static const char template[] = "%-18s %4ld  %6ld";
static const char count_str[] = "                   count  bytes";
static const char separator[] = "------------------ -----  ------";

STATIC_OVL int
size_obj(otmp)
struct obj *otmp;
{
    int sz = (int) sizeof(struct obj);

    if (otmp->oextra) {
        sz += (int) sizeof(struct oextra);
        if (ONAME(otmp))
            sz += (int) strlen(ONAME(otmp)) + 1;
        if (OMONST(otmp))
            sz += (int) sizeof(struct monst);
        if (OMID(otmp))
            sz += (int) sizeof(unsigned);
        if (OLONG(otmp))
            sz += (int) sizeof(long);
        if (OMAILCMD(otmp))
            sz += (int) strlen(OMAILCMD(otmp)) + 1;
    }
    return sz;
}

STATIC_OVL void
count_obj(chain, total_count, total_size, top, recurse)
struct obj *chain;
long *total_count;
long *total_size;
boolean top;
boolean recurse;
{
    long count, size;
    struct obj *obj;

    for (count = size = 0, obj = chain; obj; obj = obj->nobj) {
        if (top) {
            count++;
            size += size_obj(obj);
        }
        if (recurse && obj->cobj)
            count_obj(obj->cobj, total_count, total_size, TRUE, TRUE);
    }
    *total_count += count;
    *total_size += size;
}

STATIC_OVL void
obj_chain(win, src, chain, total_count, total_size)
winid win;
const char *src;
struct obj *chain;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0, size = 0;

    count_obj(chain, &count, &size, TRUE, FALSE);
    *total_count += count;
    *total_size += size;
    Sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

STATIC_OVL void
mon_invent_chain(win, src, chain, total_count, total_size)
winid win;
const char *src;
struct monst *chain;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0, size = 0;
    struct monst *mon;

    for (mon = chain; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, TRUE, FALSE);
    *total_count += count;
    *total_size += size;
    Sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

STATIC_OVL void
contained(win, src, total_count, total_size)
winid win;
const char *src;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0, size = 0;
    struct monst *mon;

    count_obj(invent, &count, &size, FALSE, TRUE);
    count_obj(fobj, &count, &size, FALSE, TRUE);
    count_obj(level.buriedobjlist, &count, &size, FALSE, TRUE);
    count_obj(migrating_objs, &count, &size, FALSE, TRUE);
    /* DEADMONSTER check not required in this loop since they have no
     * inventory */
    for (mon = fmon; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, FALSE, TRUE);
    for (mon = migrating_mons; mon; mon = mon->nmon)
        count_obj(mon->minvent, &count, &size, FALSE, TRUE);

    *total_count += count;
    *total_size += size;

    Sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

STATIC_OVL int
size_monst(mtmp)
struct monst *mtmp;
{
    int sz = (int) sizeof(struct monst);

    if (mtmp->mextra) {
        sz += (int) sizeof(struct mextra);
        if (MNAME(mtmp))
            sz += (int) strlen(MNAME(mtmp)) + 1;
        if (EGD(mtmp))
            sz += (int) sizeof(struct egd);
        if (EPRI(mtmp))
            sz += (int) sizeof(struct epri);
        if (ESHK(mtmp))
            sz += (int) sizeof(struct eshk);
        if (EMIN(mtmp))
            sz += (int) sizeof(struct emin);
        if (EDOG(mtmp))
            sz += (int) sizeof(struct edog);
        /* mextra->mcorpsenm doesn't point to more memory */
    }
    return sz;
}

STATIC_OVL void
mon_chain(win, src, chain, total_count, total_size)
winid win;
const char *src;
struct monst *chain;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count, size;
    struct monst *mon;

    for (count = size = 0, mon = chain; mon; mon = mon->nmon) {
        count++;
        size += size_monst(mon);
    }
    *total_count += count;
    *total_size += size;
    Sprintf(buf, template, src, count, size);
    putstr(win, 0, buf);
}

/*
 * Display memory usage of all monsters and objects on the level.
 */
static int
wiz_show_stats()
{
    char buf[BUFSZ];
    winid win;
    long total_obj_size = 0, total_obj_count = 0;
    long total_mon_size = 0, total_mon_count = 0;

    win = create_nhwindow(NHW_TEXT);
    putstr(win, 0, "Current memory statistics:");
    putstr(win, 0, "");
    Sprintf(buf, "Objects, size %d", (int) sizeof(struct obj));
    putstr(win, 0, buf);
    putstr(win, 0, "");
    putstr(win, 0, count_str);

    obj_chain(win, "invent", invent, &total_obj_count, &total_obj_size);
    obj_chain(win, "fobj", fobj, &total_obj_count, &total_obj_size);
    obj_chain(win, "buried", level.buriedobjlist, &total_obj_count,
              &total_obj_size);
    obj_chain(win, "migrating obj", migrating_objs, &total_obj_count,
              &total_obj_size);
    mon_invent_chain(win, "minvent", fmon, &total_obj_count, &total_obj_size);
    mon_invent_chain(win, "migrating minvent", migrating_mons,
                     &total_obj_count, &total_obj_size);

    contained(win, "contained", &total_obj_count, &total_obj_size);

    putstr(win, 0, separator);
    Sprintf(buf, template, "Total", total_obj_count, total_obj_size);
    putstr(win, 0, buf);

    putstr(win, 0, "");
    putstr(win, 0, "");
    Sprintf(buf, "Monsters, size %d", (int) sizeof(struct monst));
    putstr(win, 0, buf);
    putstr(win, 0, "");

    mon_chain(win, "fmon", fmon, &total_mon_count, &total_mon_size);
    mon_chain(win, "migrating", migrating_mons, &total_mon_count,
              &total_mon_size);

    putstr(win, 0, separator);
    Sprintf(buf, template, "Total", total_mon_count, total_mon_size);
    putstr(win, 0, buf);

#if defined(__BORLANDC__) && !defined(_WIN32)
    show_borlandc_stats(win);
#endif

    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);
    return 0;
}

void
sanity_check()
{
    obj_sanity_check();
    timer_sanity_check();
    mon_sanity_check();
    light_sources_sanity_check();
}

#ifdef DEBUG_MIGRATING_MONS
static int
wiz_migrate_mons()
{
    int mcount = 0;
    char inbuf[BUFSZ];
    struct permonst *ptr;
    struct monst *mtmp;
    d_level tolevel;

    getlin("How many random monsters to migrate? [0]", inbuf);
    if (*inbuf == '\033')
        return 0;
    mcount = atoi(inbuf);
    if (mcount < 0 || mcount > (COLNO * ROWNO) || Is_botlevel(&u.uz))
        return 0;
    while (mcount > 0) {
        if (Is_stronghold(&u.uz))
            assign_level(&tolevel, &valley_level);
        else
            get_level(&tolevel, depth(&u.uz) + 1);
        ptr = rndmonst();
        mtmp = makemon(ptr, 0, 0, NO_MM_FLAGS);
        if (mtmp)
            migrate_to_level(mtmp, ledger_no(&tolevel), MIGR_RANDOM,
                             (coord *) 0);
        mcount--;
    }
    return 0;
}
#endif

#define unctrl(c) ((c) <= C('z') ? (0x60 | (c)) : (c))
#define unmeta(c) (0x7f & (c))

/* called at startup and after number_pad is twiddled */
void
reset_commands(initial)
boolean initial;
{
    static const char sdir[] = "hykulnjb><",
                      sdir_swap_yz[] = "hzkulnjb><",
                      ndir[] = "47896321><",
                      ndir_phone_layout[] = "41236987><";
    static const int ylist[] = {
        'y', 'Y', C('y'), M('y'), M('Y'), M(C('y'))
    };
    const struct func_tab *cmdtmp;
    boolean flagtemp;
    int c, i, updated = 0;

    if (initial) {
        updated = 1;
        for (i = 0; i < SIZE(cmdlist); i++) {
            c = cmdlist[i].f_char & 0xff;
            Cmd.commands[c] = &cmdlist[i];
        }
        Cmd.num_pad = FALSE;
        Cmd.pcHack_compat = Cmd.phone_layout = Cmd.swap_yz = FALSE;
    } else {
        /* basic num_pad */
        flagtemp = iflags.num_pad;
        if (flagtemp != Cmd.num_pad) {
            Cmd.num_pad = flagtemp;
            ++updated;
        }
        /* swap_yz mode (only applicable for !num_pad) */
        flagtemp = (iflags.num_pad_mode & 1) ? !Cmd.num_pad : FALSE;
        if (flagtemp != Cmd.swap_yz) {
            Cmd.swap_yz = flagtemp;
            ++updated;
            /* Cmd.swap_yz has been toggled;
               perform the swap (or reverse previous one) */
            for (i = 0; i < SIZE(ylist); i++) {
                c = ylist[i] & 0xff;
                cmdtmp = Cmd.commands[c];              /* tmp = [y] */
                Cmd.commands[c] = Cmd.commands[c + 1]; /* [y] = [z] */
                Cmd.commands[c + 1] = cmdtmp;          /* [z] = tmp */
            }
        }
        /* MSDOS compatibility mode (only applicable for num_pad) */
        flagtemp = (iflags.num_pad_mode & 1) ? Cmd.num_pad : FALSE;
        if (flagtemp != Cmd.pcHack_compat) {
            Cmd.pcHack_compat = flagtemp;
            ++updated;
            /* pcHack_compat has been toggled */
            c = M('5') & 0xff;
            cmdtmp = Cmd.commands['5'];
            Cmd.commands['5'] = Cmd.commands[c];
            Cmd.commands[c] = cmdtmp;
            c = M('0') & 0xff;
            Cmd.commands[c] = Cmd.pcHack_compat ? Cmd.commands['I'] : 0;
        }
        /* phone keypad layout (only applicable for num_pad) */
        flagtemp = (iflags.num_pad_mode & 2) ? Cmd.num_pad : FALSE;
        if (flagtemp != Cmd.phone_layout) {
            Cmd.phone_layout = flagtemp;
            ++updated;
            /* phone_layout has been toggled */
            for (i = 0; i < 3; i++) {
                c = '1' + i;                           /* 1,2,3 <-> 7,8,9 */
                cmdtmp = Cmd.commands[c];              /* tmp = [1] */
                Cmd.commands[c] = Cmd.commands[c + 6]; /* [1] = [7] */
                Cmd.commands[c + 6] = cmdtmp;          /* [7] = tmp */
                c = (M('1') & 0xff) + i;  /* M-1,M-2,M-3 <-> M-7,M-8,M-9 */
                cmdtmp = Cmd.commands[c]; /* tmp = [M-1] */
                Cmd.commands[c] = Cmd.commands[c + 6]; /* [M-1] = [M-7] */
                Cmd.commands[c + 6] = cmdtmp;          /* [M-7] = tmp */
            }
        }
    } /*?initial*/

    if (updated)
        Cmd.serialno++;
    Cmd.dirchars = !Cmd.num_pad
                       ? (!Cmd.swap_yz ? sdir : sdir_swap_yz)
                       : (!Cmd.phone_layout ? ndir : ndir_phone_layout);
    Cmd.alphadirchars = !Cmd.num_pad ? Cmd.dirchars : sdir;

    Cmd.move_W = Cmd.dirchars[0];
    Cmd.move_NW = Cmd.dirchars[1];
    Cmd.move_N = Cmd.dirchars[2];
    Cmd.move_NE = Cmd.dirchars[3];
    Cmd.move_E = Cmd.dirchars[4];
    Cmd.move_SE = Cmd.dirchars[5];
    Cmd.move_S = Cmd.dirchars[6];
    Cmd.move_SW = Cmd.dirchars[7];
}

STATIC_OVL boolean
accept_menu_prefix(cmd_func)
int NDECL((*cmd_func));
{
    if (cmd_func == dopickup || cmd_func == dotip
        || cmd_func == doextcmd || cmd_func == doextlist)
        return TRUE;
    return FALSE;
}

void
rhack(cmd)
register char *cmd;
{
    boolean do_walk, do_rush, prefix_seen, bad_command,
        firsttime = (cmd == 0);

    iflags.menu_requested = FALSE;
#ifdef SAFERHANGUP
    if (program_state.done_hup)
        end_of_input();
#endif
    if (firsttime) {
        context.nopick = 0;
        cmd = parse();
    }
    if (*cmd == '\033') {
        context.move = FALSE;
        return;
    }
    if (*cmd == DOAGAIN && !in_doagain && saveq[0]) {
        in_doagain = TRUE;
        stail = 0;
        rhack((char *) 0); /* read and execute command */
        in_doagain = FALSE;
        return;
    }
    /* Special case of *cmd == ' ' handled better below */
    if (!*cmd || *cmd == (char) 0377) {
        nhbell();
        context.move = FALSE;
        return; /* probably we just had an interrupt */
    }

    /* handle most movement commands */
    do_walk = do_rush = prefix_seen = FALSE;
    context.travel = context.travel1 = 0;
    switch (*cmd) {
    case 'g':
        if (movecmd(cmd[1])) {
            context.run = 2;
            do_rush = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case '5':
        if (!Cmd.num_pad)
            break; /* else FALLTHRU */
    case 'G':
        if (movecmd(lowc(cmd[1]))) {
            context.run = 3;
            do_rush = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case '-':
        if (!Cmd.num_pad)
            break; /* else FALLTHRU */
    /* Effects of movement commands and invisible monsters:
     * m: always move onto space (even if 'I' remembered)
     * F: always attack space (even if 'I' not remembered)
     * normal movement: attack if 'I', move otherwise.
     */
    case 'F':
        if (movecmd(cmd[1])) {
            context.forcefight = 1;
            do_walk = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case 'm':
        if (movecmd(cmd[1]) || u.dz) {
            context.run = 0;
            context.nopick = 1;
            if (!u.dz)
                do_walk = TRUE;
            else
                cmd[0] = cmd[1]; /* "m<" or "m>" */
        } else
            prefix_seen = TRUE;
        break;
    case 'M':
        if (movecmd(lowc(cmd[1]))) {
            context.run = 1;
            context.nopick = 1;
            do_rush = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case '0':
        if (!Cmd.num_pad)
            break;
        (void) ddoinv(); /* a convenience borrowed from the PC */
        context.move = FALSE;
        multi = 0;
        return;
    case CMD_CLICKLOOK:
        if (iflags.clicklook) {
            context.move = FALSE;
            do_look(2, &clicklook_cc);
        }
        return;
    case CMD_TRAVEL:
        if (flags.travelcmd) {
            context.travel = 1;
            context.travel1 = 1;
            context.run = 8;
            context.nopick = 1;
            do_rush = TRUE;
            break;
        }
        /*FALLTHRU*/
    default:
        if (movecmd(*cmd)) { /* ordinary movement */
            context.run = 0; /* only matters here if it was 8 */
            do_walk = TRUE;
        } else if (movecmd(Cmd.num_pad ? unmeta(*cmd) : lowc(*cmd))) {
            context.run = 1;
            do_rush = TRUE;
        } else if (movecmd(unctrl(*cmd))) {
            context.run = 3;
            do_rush = TRUE;
        }
        break;
    }

    /* some special prefix handling */
    /* overload 'm' prefix to mean "request a menu" */
    if (prefix_seen && cmd[0] == 'm') {
        /* (for func_tab cast, see below) */
        const struct func_tab *ft = Cmd.commands[cmd[1] & 0xff];
        int NDECL((*func)) = ft ? ((struct func_tab *) ft)->f_funct : 0;

        if (func && accept_menu_prefix(func)) {
            iflags.menu_requested = TRUE;
            ++cmd;
        }
    }

    if ((do_walk || do_rush) && !context.travel && !dxdy_moveok()) {
        /* trying to move diagonally as a grid bug;
           this used to be treated by movecmd() as not being
           a movement attempt, but that didn't provide for any
           feedback and led to strangeness if the key pressed
           ('u' in particular) was overloaded for num_pad use */
        You_cant("get there from here...");
        context.run = 0;
        context.nopick = context.forcefight = FALSE;
        context.move = context.mv = FALSE;
        multi = 0;
        return;
    }

    if (do_walk) {
        if (multi)
            context.mv = TRUE;
        domove();
        context.forcefight = 0;
        return;
    } else if (do_rush) {
        if (firsttime) {
            if (!multi)
                multi = max(COLNO, ROWNO);
            u.last_str_turn = 0;
        }
        context.mv = TRUE;
        domove();
        return;
    } else if (prefix_seen && cmd[1] == '\033') { /* <prefix><escape> */
        /* don't report "unknown command" for change of heart... */
        bad_command = FALSE;
    } else if (*cmd == ' ' && !flags.rest_on_space) {
        bad_command = TRUE; /* skip cmdlist[] loop */

        /* handle all other commands */
    } else {
        register const struct func_tab *tlist;
        int res, NDECL((*func));

#if 0
        /* obsolete - scan through the cmdlist array looking for *cmd */
        for (tlist = cmdlist; tlist->f_char; tlist++) {
            if ((*cmd & 0xff) != (tlist->f_char & 0xff))
                continue;
#else
        /* current - use *cmd to directly index cmdlist array */
        if ((tlist = Cmd.commands[*cmd & 0xff]) != 0) {
#endif
            if (u.uburied && !tlist->can_if_buried) {
/*JP
                You_cant("do that while you are buried!");
*/
                You("埋まっている時にそんなことはできない！");
                res = 0;
            } else {
                /* we discard 'const' because some compilers seem to have
                   trouble with the pointer passed to set_occupation() */
                func = ((struct func_tab *) tlist)->f_funct;
                if (tlist->f_text && !occupation && multi)
                    set_occupation(func, tlist->f_text, multi);
                res = (*func)(); /* perform the command */
            }
            if (!res) {
                context.move = FALSE;
                multi = 0;
            }
            return;
        }
        /* if we reach here, cmd wasn't found in cmdlist[] */
        bad_command = TRUE;
    }

    if (bad_command) {
        char expcmd[10];
        register char c, *cp = expcmd;

        while ((c = *cmd++) != '\0'
               && (int) (cp - expcmd) < (int) (sizeof expcmd - 3)) {
            if (c >= 040 && c < 0177) {
                *cp++ = c;
            } else if (c & 0200) {
                *cp++ = 'M';
                *cp++ = '-';
                *cp++ = c & ~0200;
            } else {
                *cp++ = '^';
                *cp++ = c ^ 0100;
            }
        }
        *cp = '\0';
        if (!prefix_seen || !iflags.cmdassist
/*JP
            || !help_dir(0, "Invalid direction key!"))
*/
            || !help_dir(0, "無効な方向指定です！"))
/*JP
            Norep("Unknown command '%s'.", expcmd);
*/
            Norep("'%s'コマンド？", expcmd);
    }
    /* didn't move */
    context.move = FALSE;
    multi = 0;
    return;
}

/* convert an x,y pair into a direction code */
int
xytod(x, y)
schar x, y;
{
    register int dd;

    for (dd = 0; dd < 8; dd++)
        if (x == xdir[dd] && y == ydir[dd])
            return dd;
    return -1;
}

/* convert a direction code into an x,y pair */
void
dtoxy(cc, dd)
coord *cc;
register int dd;
{
    cc->x = xdir[dd];
    cc->y = ydir[dd];
    return;
}

/* also sets u.dz, but returns false for <> */
int
movecmd(sym)
char sym;
{
    register const char *dp = index(Cmd.dirchars, sym);

    u.dz = 0;
    if (!dp || !*dp)
        return 0;
    u.dx = xdir[dp - Cmd.dirchars];
    u.dy = ydir[dp - Cmd.dirchars];
    u.dz = zdir[dp - Cmd.dirchars];
#if 0 /* now handled elsewhere */
    if (u.dx && u.dy && NODIAG(u.umonnum)) {
        u.dx = u.dy = 0;
        return 0;
    }
#endif
    return !u.dz;
}

/* grid bug handling which used to be in movecmd() */
int
dxdy_moveok()
{
    if (u.dx && u.dy && NODIAG(u.umonnum))
        u.dx = u.dy = 0;
    return u.dx || u.dy;
}

/* decide whether a character (user input keystroke) requests screen repaint */
boolean
redraw_cmd(c)
char c;
{
    return (boolean) (c == C('r') || (Cmd.num_pad && c == C('l')));
}

/*
 * uses getdir() but unlike getdir() it specifically
 * produces coordinates using the direction from getdir()
 * and verifies that those coordinates are ok.
 *
 * If the call to getdir() returns 0, Never_mind is displayed.
 * If the resulting coordinates are not okay, emsg is displayed.
 *
 * Returns non-zero if coordinates in cc are valid.
 */
int
get_adjacent_loc(prompt, emsg, x, y, cc)
const char *prompt, *emsg;
xchar x, y;
coord *cc;
{
    xchar new_x, new_y;
    if (!getdir(prompt)) {
        pline1(Never_mind);
        return 0;
    }
    new_x = x + u.dx;
    new_y = y + u.dy;
    if (cc && isok(new_x, new_y)) {
        cc->x = new_x;
        cc->y = new_y;
    } else {
        if (emsg)
            pline1(emsg);
        return 0;
    }
    return 1;
}

int
getdir(s)
const char *s;
{
    char dirsym;
    int is_mov;

retry:
    if (in_doagain || *readchar_queue)
        dirsym = readchar();
    else
/*JP
        dirsym = yn_function((s && *s != '^') ? s : "In what direction?",
*/
        dirsym = yn_function((s && *s != '^') ? s : "どの方向？",
                             (char *) 0, '\0');
    /* remove the prompt string so caller won't have to */
    clear_nhwindow(WIN_MESSAGE);

    if (redraw_cmd(dirsym)) { /* ^R */
        docrt();              /* redraw */
        goto retry;
    }
    savech(dirsym);

    if (dirsym == '.' || dirsym == 's') {
        u.dx = u.dy = u.dz = 0;
    } else if (!(is_mov = movecmd(dirsym)) && !u.dz) {
        boolean did_help = FALSE, help_requested;

        if (!index(quitchars, dirsym)) {
            help_requested = (dirsym == '?');
            if (help_requested || iflags.cmdassist) {
                did_help =
                    help_dir((s && *s == '^') ? dirsym : 0,
                             help_requested ? (const char *) 0
/*JP
                                            : "Invalid direction key!");
*/
                                            : "無効な方向指定です！");
                if (help_requested)
                    goto retry;
            }
            if (!did_help)
/*JP
                pline("What a strange direction!");
*/
                pline("ずいぶんと奇妙な方向だ！");
        }
        return 0;
    } else if (is_mov && !dxdy_moveok()) {
/*JP
        You_cant("orient yourself that direction.");
*/
        You_cant("向きに自分自身を指定できない．");
        return 0;
    }
    if (!u.dz && (Stunned || (Confusion && !rn2(5))))
        confdir();
    return 1;
}

STATIC_OVL boolean
help_dir(sym, msg)
char sym;
const char *msg;
{
    char ctrl;
    winid win;
    static const char wiz_only_list[] = "EFGIOVW";
    char buf[BUFSZ], buf2[BUFSZ], *explain;

    win = create_nhwindow(NHW_TEXT);
    if (!win)
        return FALSE;
    if (msg) {
        Sprintf(buf, "cmdassist: %s", msg);
        putstr(win, 0, buf);
        putstr(win, 0, "");
    }
    if (letter(sym)) {
        sym = highc(sym);
        ctrl = (sym - 'A') + 1;
        if ((explain = dowhatdoes_core(ctrl, buf2))
            && (!index(wiz_only_list, sym) || wizard)) {
            Sprintf(buf, "Are you trying to use ^%c%s?", sym,
                    index(wiz_only_list, sym)
                        ? ""
                        : " as specified in the Guidebook");
            putstr(win, 0, buf);
            putstr(win, 0, "");
            putstr(win, 0, explain);
            putstr(win, 0, "");
            putstr(win, 0, "To use that command, you press");
            Sprintf(buf, "the <Ctrl> key, and the <%c> key at the same time.",
                    sym);
            putstr(win, 0, buf);
            putstr(win, 0, "");
        }
    }
    if (NODIAG(u.umonnum)) {
        putstr(win, 0, "Valid direction keys in your current form are:");
        Sprintf(buf, "             %c   ", Cmd.move_N);
        putstr(win, 0, buf);
        putstr(win, 0, "             |   ");
        Sprintf(buf, "          %c- . -%c", Cmd.move_W, Cmd.move_E);
        putstr(win, 0, buf);
        putstr(win, 0, "             |   ");
        Sprintf(buf, "             %c   ", Cmd.move_S);
        putstr(win, 0, buf);
    } else {
        putstr(win, 0, "Valid direction keys are:");
        Sprintf(buf, "          %c  %c  %c", Cmd.move_NW, Cmd.move_N,
                Cmd.move_NE);
        putstr(win, 0, buf);
        putstr(win, 0, "           \\ | / ");
        Sprintf(buf, "          %c- . -%c", Cmd.move_W, Cmd.move_E);
        putstr(win, 0, buf);
        putstr(win, 0, "           / | \\ ");
        Sprintf(buf, "          %c  %c  %c", Cmd.move_SW, Cmd.move_S,
                Cmd.move_SE);
        putstr(win, 0, buf);
    };
    putstr(win, 0, "");
    putstr(win, 0, "          <  up");
    putstr(win, 0, "          >  down");
    putstr(win, 0, "          .  direct at yourself");
    if (msg) {
        /* non-null msg means that this wasn't an explicit user request */
        putstr(win, 0, "");
        putstr(win, 0,
/*JP
               "(Suppress this message with !cmdassist in config file.)");
*/
               "(このメッセージを表示したくない場合は設定ファイルに !cmdassist を設定してください．)");
    }
    display_nhwindow(win, FALSE);
    destroy_nhwindow(win);
    return TRUE;
}

void
confdir()
{
    register int x = NODIAG(u.umonnum) ? 2 * rn2(4) : rn2(8);

    u.dx = xdir[x];
    u.dy = ydir[x];
    return;
}

const char *
directionname(dir)
int dir;
{
    static NEARDATA const char *const dirnames[] = {
        "west",      "northwest", "north",     "northeast", "east",
        "southeast", "south",     "southwest", "down",      "up",
    };

    if (dir < 0 || dir >= SIZE(dirnames))
        return "invalid";
    return dirnames[dir];
}

int
isok(x, y)
register int x, y;
{
    /* x corresponds to curx, so x==1 is the first column. Ach. %% */
    return x >= 1 && x <= COLNO - 1 && y >= 0 && y <= ROWNO - 1;
}

static NEARDATA int last_multi;

/*
 * convert a MAP window position into a movecmd
 */
const char *
click_to_cmd(x, y, mod)
int x, y, mod;
{
    int dir;
    static char cmd[4];
    cmd[1] = 0;

    if (iflags.clicklook && mod == CLICK_2) {
        clicklook_cc.x = x;
        clicklook_cc.y = y;
        cmd[0] = CMD_CLICKLOOK;
        return cmd;
    }

    x -= u.ux;
    y -= u.uy;

    if (flags.travelcmd) {
        if (abs(x) <= 1 && abs(y) <= 1) {
            x = sgn(x), y = sgn(y);
        } else {
            u.tx = u.ux + x;
            u.ty = u.uy + y;
            cmd[0] = CMD_TRAVEL;
            return cmd;
        }

        if (x == 0 && y == 0) {
            /* here */
            if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)
                || IS_SINK(levl[u.ux][u.uy].typ)) {
                cmd[0] = mod == CLICK_1 ? 'q' : M('d');
                return cmd;
            } else if (IS_THRONE(levl[u.ux][u.uy].typ)) {
                cmd[0] = M('s');
                return cmd;
            } else if ((u.ux == xupstair && u.uy == yupstair)
                       || (u.ux == sstairs.sx && u.uy == sstairs.sy
                           && sstairs.up)
                       || (u.ux == xupladder && u.uy == yupladder)) {
                return "<";
            } else if ((u.ux == xdnstair && u.uy == ydnstair)
                       || (u.ux == sstairs.sx && u.uy == sstairs.sy
                           && !sstairs.up)
                       || (u.ux == xdnladder && u.uy == ydnladder)) {
                return ">";
            } else if (OBJ_AT(u.ux, u.uy)) {
                cmd[0] =
                    Is_container(level.objects[u.ux][u.uy]) ? M('l') : ',';
                return cmd;
            } else {
                return "."; /* just rest */
            }
        }

        /* directional commands */

        dir = xytod(x, y);

        if (!m_at(u.ux + x, u.uy + y)
            && !test_move(u.ux, u.uy, x, y, TEST_MOVE)) {
            cmd[1] = Cmd.dirchars[dir];
            cmd[2] = '\0';
            if (IS_DOOR(levl[u.ux + x][u.uy + y].typ)) {
                /* slight assistance to the player: choose kick/open for them
                 */
                if (levl[u.ux + x][u.uy + y].doormask & D_LOCKED) {
                    cmd[0] = C('d');
                    return cmd;
                }
                if (levl[u.ux + x][u.uy + y].doormask & D_CLOSED) {
                    cmd[0] = 'o';
                    return cmd;
                }
            }
            if (levl[u.ux + x][u.uy + y].typ <= SCORR) {
                cmd[0] = 's';
                cmd[1] = 0;
                return cmd;
            }
        }
    } else {
        /* convert without using floating point, allowing sloppy clicking */
        if (x > 2 * abs(y))
            x = 1, y = 0;
        else if (y > 2 * abs(x))
            x = 0, y = 1;
        else if (x < -2 * abs(y))
            x = -1, y = 0;
        else if (y < -2 * abs(x))
            x = 0, y = -1;
        else
            x = sgn(x), y = sgn(y);

        if (x == 0 && y == 0) /* map click on player to "rest" command */
            return ".";

        dir = xytod(x, y);
    }

    /* move, attack, etc. */
    cmd[1] = 0;
    if (mod == CLICK_1) {
        cmd[0] = Cmd.dirchars[dir];
    } else {
        cmd[0] = (Cmd.num_pad
                     ? M(Cmd.dirchars[dir])
                     : (Cmd.dirchars[dir] - 'a' + 'A')); /* run command */
    }

    return cmd;
}

STATIC_OVL char *
parse()
{
#ifdef LINT /* static char in_line[COLNO]; */
    char in_line[COLNO];
#else
    static char in_line[COLNO];
#endif
    register int foo;
    boolean prezero = FALSE;

    multi = 0;
    context.move = 1;
    flush_screen(1); /* Flush screen buffer. Put the cursor on the hero. */

#ifdef ALTMETA
    alt_esc = iflags.altmeta; /* readchar() hack */
#endif
    if (!Cmd.num_pad || (foo = readchar()) == 'n')
        for (;;) {
            foo = readchar();
            if (foo >= '0' && foo <= '9') {
                multi = 10 * multi + foo - '0';
                if (multi < 0 || multi >= LARGEST_INT)
                    multi = LARGEST_INT;
                if (multi > 9) {
                    clear_nhwindow(WIN_MESSAGE);
/*JP
                    Sprintf(in_line, "Count: %d", multi);
*/
                    Sprintf(in_line, "数: %d", multi);
                    pline1(in_line);
                    mark_synch();
                }
                last_multi = multi;
                if (!multi && foo == '0')
                    prezero = TRUE;
            } else
                break; /* not a digit */
        }
#ifdef ALTMETA
    alt_esc = FALSE; /* readchar() reset */
#endif

    if (foo == '\033') { /* esc cancels count (TH) */
        clear_nhwindow(WIN_MESSAGE);
        multi = last_multi = 0;
    } else if (foo == DOAGAIN || in_doagain) {
        multi = last_multi;
    } else {
        last_multi = multi;
        savech(0); /* reset input queue */
        savech((char) foo);
    }

    if (multi) {
        multi--;
        save_cm = in_line;
    } else {
        save_cm = (char *) 0;
    }
    /* in 3.4.3 this was in rhack(), where it was too late to handle M-5 */
    if (Cmd.pcHack_compat) {
        /* This handles very old inconsistent DOS/Windows behaviour
           in a different way: earlier, the keyboard handler mapped
           these, which caused counts to be strange when entered
           from the number pad. Now do not map them until here. */
        switch (foo) {
        case '5':
            foo = 'g';
            break;
        case M('5'):
            foo = 'G';
            break;
        case M('0'):
            foo = 'I';
            break;
        default:
            break; /* as is */
        }
    }

    in_line[0] = foo;
    in_line[1] = '\0';
    if (foo == 'g' || foo == 'G' || foo == 'm' || foo == 'M' || foo == 'F'
        || (Cmd.num_pad && (foo == '5' || foo == '-'))) {
        foo = readchar();
        savech((char) foo);
        in_line[1] = foo;
        in_line[2] = 0;
    }
    clear_nhwindow(WIN_MESSAGE);
    if (prezero)
        in_line[0] = '\033';
    return in_line;
}

#ifdef HANGUPHANDLING
/* some very old systems, or descendents of such systems, expect signal
   handlers to have return type `int', but they don't actually inspect
   the return value so we should be safe using `void' unconditionally */
/*ARGUSED*/
void
hangup(sig_unused) /* called as signal() handler, so sent at least one arg */
int sig_unused UNUSED;
{
    if (program_state.exiting)
        program_state.in_moveloop = 0;
    nhwindows_hangup();
#ifdef SAFERHANGUP
    /* When using SAFERHANGUP, the done_hup flag it tested in rhack
       and a couple of other places; actual hangup handling occurs then.
       This is 'safer' because it disallows certain cheats and also
       protects against losing objects in the process of being thrown,
       but also potentially riskier because the disconnected program
       must continue running longer before attempting a hangup save. */
    program_state.done_hup++;
    /* defer hangup iff game appears to be in progress */
    if (program_state.in_moveloop && program_state.something_worth_saving)
        return;
#endif /* SAFERHANGUP */
    end_of_input();
}

void
end_of_input()
{
#ifdef NOSAVEONHANGUP
#ifdef INSURANCE
    if (flags.ins_chkpt && program_state.something_worth_saving)
        program_statue.preserve_locks = 1; /* keep files for recovery */
#endif
    program_state.something_worth_saving = 0; /* don't save */
#endif

#ifndef SAFERHANGUP
    if (!program_state.done_hup++)
#endif
        if (program_state.something_worth_saving)
            (void) dosave0();
    if (iflags.window_inited)
        exit_nhwindows((char *) 0);
    clearlocks();
    terminate(EXIT_SUCCESS);
    /*NOTREACHED*/ /* not necessarily true for vms... */
    return;
}
#endif /* HANGUPHANDLING */

char
readchar()
{
    register int sym;
    int x = u.ux, y = u.uy, mod = 0;

    if (*readchar_queue)
        sym = *readchar_queue++;
    else
        sym = in_doagain ? pgetchar() : nh_poskey(&x, &y, &mod);

#ifdef NR_OF_EOFS
    if (sym == EOF) {
        register int cnt = NR_OF_EOFS;
        /*
         * Some SYSV systems seem to return EOFs for various reasons
         * (?like when one hits break or for interrupted systemcalls?),
         * and we must see several before we quit.
         */
        do {
            clearerr(stdin); /* omit if clearerr is undefined */
            sym = pgetchar();
        } while (--cnt && sym == EOF);
    }
#endif /* NR_OF_EOFS */

    if (sym == EOF) {
#ifdef HANGUPHANDLING
        hangup(0); /* call end_of_input() or set program_state.done_hup */
#endif
        sym = '\033';
#ifdef ALTMETA
    } else if (sym == '\033' && alt_esc) {
        /* iflags.altmeta: treat two character ``ESC c'' as single `M-c' */
        sym = *readchar_queue ? *readchar_queue++ : pgetchar();
        if (sym == EOF || sym == 0)
            sym = '\033';
        else if (sym != '\033')
            sym |= 0200; /* force 8th bit on */
#endif                   /*ALTMETA*/
    } else if (sym == 0) {
        /* click event */
        readchar_queue = click_to_cmd(x, y, mod);
        sym = *readchar_queue++;
    }
    return (char) sym;
}

STATIC_PTR int
dotravel(VOID_ARGS)
{
    /* Keyboard travel command */
    static char cmd[2];
    coord cc;

    if (!flags.travelcmd)
        return 0;
    cmd[1] = 0;
    cc.x = iflags.travelcc.x;
    cc.y = iflags.travelcc.y;
    if (cc.x == -1 && cc.y == -1) {
        /* No cached destination, start attempt from current position */
        cc.x = u.ux;
        cc.y = u.uy;
    }
/*JP
    pline("Where do you want to travel to?");
*/
    pline("どこに移動する？");
/*JP
    if (getpos(&cc, TRUE, "the desired destination") < 0) {
*/
    if (getpos(&cc, TRUE, "移動先") < 0) {
        /* user pressed ESC */
        return 0;
    }
    iflags.travelcc.x = u.tx = cc.x;
    iflags.travelcc.y = u.ty = cc.y;
    cmd[0] = CMD_TRAVEL;
    readchar_queue = cmd;
    return 0;
}

#ifdef PORT_DEBUG
extern void NDECL(win32con_debug_keystrokes);
extern void NDECL(win32con_handler_info);

int
wiz_port_debug()
{
    int n, k;
    winid win;
    anything any;
    int item = 'a';
    int num_menu_selections;
    struct menu_selection_struct {
        char *menutext;
        void NDECL((*fn));
    } menu_selections[] = {
#ifdef WIN32
        { "test win32 keystrokes (tty only)", win32con_debug_keystrokes },
        { "show keystroke handler information (tty only)",
          win32con_handler_info },
#endif
        { (char *) 0, (void NDECL((*) )) 0 } /* array terminator */
    };

    num_menu_selections = SIZE(menu_selections) - 1;
    if (num_menu_selections > 0) {
        menu_item *pick_list;
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        for (k = 0; k < num_menu_selections; ++k) {
            any.a_int = k + 1;
            add_menu(win, NO_GLYPH, &any, item++, 0, ATR_NONE,
                     menu_selections[k].menutext, MENU_UNSELECTED);
        }
        end_menu(win, "Which port debugging feature?");
        n = select_menu(win, PICK_ONE, &pick_list);
        destroy_nhwindow(win);
        if (n > 0) {
            n = pick_list[0].item.a_int - 1;
            free((genericptr_t) pick_list);
            /* execute the function */
            (*menu_selections[n].fn)();
        }
    } else
        pline("No port-specific debug capability defined.");
    return 0;
}
#endif /*PORT_DEBUG*/

/*
 *   Parameter validator for generic yes/no function to prevent
 *   the core from sending too long a prompt string to the
 *   window port causing a buffer overflow there.
 */
char
yn_function(query, resp, def)
const char *query, *resp;
char def;
{
    char qbuf[QBUFSZ];

    iflags.last_msg = PLNMSG_UNKNOWN; /* most recent pline is clobbered */

    /* maximum acceptable length is QBUFSZ-1 */
    if (strlen(query) < QBUFSZ)
        return (*windowprocs.win_yn_function)(query, resp, def);

    /* caller shouldn't have passed anything this long */
    paniclog("Query truncated: ", query);
    (void) strncpy(qbuf, query, QBUFSZ - 1 - 3);
    Strcpy(&qbuf[QBUFSZ - 1 - 3], "...");
    return (*windowprocs.win_yn_function)(qbuf, resp, def);
}

/* for paranoid_confirm:quit,die,attack prompting */
boolean
paranoid_query(be_paranoid, prompt)
boolean be_paranoid;
const char *prompt;
{
    boolean confirmed_ok;

    /* when paranoid, player must respond with "yes" rather than just 'y'
       to give the go-ahead for this query; default is "no" unless the
       ParanoidConfirm flag is set in which case there's no default */
    if (be_paranoid) {
        char qbuf[QBUFSZ], ans[BUFSZ];
        const char *promptprefix = "", *responsetype = ParanoidConfirm
                                                           ? "(yes|no)"
                                                           : "(yes) [no]";
        int trylimit = 6; /* 1 normal, 5 more with "Yes or No:" prefix */

        /* in addition to being paranoid about this particular
           query, we might be even more paranoid about all paranoia
           responses (ie, ParanoidConfirm is set) in which case we
           require "no" to reject in addition to "yes" to confirm
           (except we won't loop if response is ESC; it means no) */
        do {
            Sprintf(qbuf, "%s%s %s", promptprefix, prompt, responsetype);
            getlin(qbuf, ans);
            (void) mungspaces(ans);
            confirmed_ok = !strcmpi(ans, "yes");
            if (confirmed_ok || *ans == '\033')
                break;
            promptprefix = "\"Yes\" or \"No\": ";
        } while (ParanoidConfirm && strcmpi(ans, "no") && --trylimit);
    } else
        confirmed_ok = (yn(prompt) == 'y');

    return confirmed_ok;
}

int
dosuspend_core()
{
#ifdef SUSPEND
    /* Does current window system support suspend? */
    if ((*windowprocs.win_can_suspend)()) {
        /* NB: SYSCF SHELLERS handled in port code. */
        dosuspend();
    } else
#endif
        Norep("Suspend command not available.");
    return 0;
}

/*cmd.c*/
