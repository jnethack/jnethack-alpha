/* NetHack 3.6	cmd.c	$NHDT-Date: 1523306904 2018/04/09 20:48:24 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.281 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2013. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2018            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
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
#endif /* DUMB */

static int NDECL(dosuspend_core); /**/

static int NDECL((*timed_occ_fn));

STATIC_PTR int NDECL(doherecmdmenu);
STATIC_PTR int NDECL(dotherecmdmenu);
STATIC_PTR int NDECL(doprev_message);
STATIC_PTR int NDECL(timed_occupation);
STATIC_PTR int NDECL(doextcmd);
STATIC_PTR int NDECL(dotravel);
STATIC_PTR int NDECL(doterrain);
STATIC_PTR int NDECL(wiz_wish);
STATIC_PTR int NDECL(wiz_identify);
STATIC_PTR int NDECL(wiz_intrinsic);
STATIC_PTR int NDECL(wiz_map);
STATIC_PTR int NDECL(wiz_makemap);
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
STATIC_PTR int NDECL(wiz_intrinsic);
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
STATIC_DCL int FDECL(size_monst, (struct monst *, BOOLEAN_P));
STATIC_DCL int FDECL(size_obj, (struct obj *));
STATIC_DCL void FDECL(count_obj, (struct obj *, long *, long *,
                                  BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(obj_chain, (winid, const char *, struct obj *,
                                  BOOLEAN_P, long *, long *));
STATIC_DCL void FDECL(mon_invent_chain, (winid, const char *, struct monst *,
                                         long *, long *));
STATIC_DCL void FDECL(mon_chain, (winid, const char *, struct monst *,
                                  BOOLEAN_P, long *, long *));
STATIC_DCL void FDECL(contained_stats, (winid, const char *, long *, long *));
STATIC_DCL void FDECL(misc_stats, (winid, long *, long *));
STATIC_PTR int NDECL(wiz_show_stats);
STATIC_DCL boolean FDECL(accept_menu_prefix, (int NDECL((*))));
#ifdef PORT_DEBUG
STATIC_DCL int NDECL(wiz_port_debug);
#endif
STATIC_PTR int NDECL(wiz_rumor_check);
STATIC_PTR int NDECL(doattributes);

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

STATIC_DCL void FDECL(add_herecmd_menuitem, (winid, int NDECL((*)),
                                             const char *));
STATIC_DCL char FDECL(here_cmd_menu, (BOOLEAN_P));
STATIC_DCL char FDECL(there_cmd_menu, (BOOLEAN_P, int, int));
STATIC_DCL char *NDECL(parse);
STATIC_DCL void FDECL(show_direction_keys, (winid, CHAR_P, BOOLEAN_P));
STATIC_DCL boolean FDECL(help_dir, (CHAR_P, int, const char *));

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
        if (!wizard && (extcmdlist[idx].flags & WIZMODECMD)) {
/*JP
            You("can't do that.");
*/
            pline("それはできません．");
            return 0;
        }
        if (iflags.menu_requested && !accept_menu_prefix(func)) {
#if 0 /*JP*/
            pline("'%s' prefix has no effect for the %s command.",
                  visctrl(Cmd.spkeys[NHKF_REQMENU]),
                  extcmdlist[idx].ef_txt);
#else
            pline("'%s'接頭辞は%sコマンドには無効．",
                  visctrl(Cmd.spkeys[NHKF_REQMENU]),
                  extcmdlist[idx].ef_txt);
#endif
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
    char ch = cmd_from_func(doextcmd);

    datawin = create_nhwindow(NHW_TEXT);
    putstr(datawin, 0, "");
/*JP
    putstr(datawin, 0, "            Extended Commands List");
*/
    putstr(datawin, 0, "            拡張コマンド一覧");
    putstr(datawin, 0, "");
    if (ch) {
#if 0 /*JP*/
        Sprintf(buf, "    Press '%s', then type:",
                visctrl(ch));
#else
        Sprintf(buf, "    '%s'を押したあとタイプせよ:",
                visctrl(ch));
#endif
        putstr(datawin, 0, buf);
        putstr(datawin, 0, "");
    }

    for (efp = extcmdlist; efp->ef_txt; efp++) {
        if (!wizard && (efp->flags & WIZMODECMD))
            continue;
        Sprintf(buf, "   %-15s %c %s.",
                efp->ef_txt,
                (efp->flags & AUTOCOMPLETE) ? '*' : ' ',
                efp->ef_desc);
        putstr(datawin, 0, buf);
    }
    putstr(datawin, 0, "");
/*JP
    putstr(datawin, 0, "    Commands marked with a * will be autocompleted.");
*/
    putstr(datawin, 0, "    *付きのコマンドは自動補完されます．");
    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);
    return 0;
}

#ifdef TTY_GRAPHICS
#define MAX_EXT_CMD 200 /* Change if we ever have more ext cmds */

/*
 * This is currently used only by the tty interface and is
 * controlled via runtime option 'extmenu'.  (Most other interfaces
 * already use a menu all the time for extended commands.)
 *
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
    int ret, len, biggest;
    int accelerator, prevaccelerator;
    int matchlevel = 0;
    boolean wastoolong, one_per_line;

    ret = 0;
    cbuf[0] = '\0';
    biggest = 0;
    while (!ret) {
        i = n = 0;
        any = zeroany;
        /* populate choices */
        for (efp = extcmdlist; efp->ef_txt; efp++) {
            if (!(efp->flags & AUTOCOMPLETE)
                || (!wizard && (efp->flags & WIZMODECMD)))
                continue;
            if (!matchlevel || !strncmp(efp->ef_txt, cbuf, matchlevel)) {
                choices[i] = efp;
                if ((len = (int) strlen(efp->ef_desc)) > biggest)
                    biggest = len;
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
        if (nchoices  <= 1) {
            ret = (nchoices == 1) ? (int) (choices[0] - extcmdlist) : -1;
            break;
        }

        /* otherwise... */
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        Sprintf(fmtstr, "%%-%ds", biggest + 15);
        prompt[0] = '\0';
        wastoolong = FALSE; /* True => had to wrap due to line width
                             * ('w' in wizard mode) */
        /* -3: two line menu header, 1 line menu footer (for prompt) */
        one_per_line = (nchoices < ROWNO - 3);
        accelerator = prevaccelerator = 0;
        acount = 0;
        for (i = 0; choices[i]; ++i) {
            accelerator = choices[i]->ef_txt[matchlevel];
            if (accelerator != prevaccelerator || one_per_line)
                wastoolong = FALSE;
            if (accelerator != prevaccelerator || one_per_line
                || (acount >= 2
                    /* +4: + sizeof " or " - sizeof "" */
                    && (strlen(prompt) + 4 + strlen(choices[i]->ef_txt)
                        /* -6: enough room for 1 space left margin
                         *   + "%c - " menu selector + 1 space right margin */
                        >= min(sizeof prompt, COLNO - 6)))) {
                if (acount) {
                    /* flush extended cmds for that letter already in buf */
                    Sprintf(buf, fmtstr, prompt);
                    any.a_char = prevaccelerator;
                    add_menu(win, NO_GLYPH, &any, any.a_char, 0, ATR_NONE,
                             buf, FALSE);
                    acount = 0;
                    if (!(accelerator != prevaccelerator || one_per_line))
                        wastoolong = TRUE;
                }
            }
            prevaccelerator = accelerator;
            if (!acount || one_per_line) {
#if 0 /*JP:T*/
                Sprintf(prompt, "%s%s [%s]", wastoolong ? "or " : "",
                        choices[i]->ef_txt, choices[i]->ef_desc);
#else
                Sprintf(prompt, "%s%s [%s]", wastoolong ? "または" : "",
                        choices[i]->ef_txt, choices[i]->ef_desc);
#endif
            } else if (acount == 1) {
#if 0 /*JP:T*/
                Sprintf(prompt, "%s%s or %s", wastoolong ? "or " : "",
                        choices[i - 1]->ef_txt, choices[i]->ef_txt);
#else
                Sprintf(prompt, "%s%sまたは%s", wastoolong ? "または" : "",
                        choices[i - 1]->ef_txt, choices[i]->ef_txt);
#endif
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
int
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

STATIC_PTR int
wiz_makemap(VOID_ARGS)
{
    /* FIXME: doesn't handle riding */
    if (wizard) {
        struct monst *mtmp;

        rm_mapseen(ledger_no(&u.uz));
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
            if (mtmp->isshk)
                setpaid(mtmp);
        if (Punished) {
            ballrelease(FALSE);
            unplacebc();
        }
        check_special_room(TRUE);
        dmonsfree();
        savelev(-1, ledger_no(&u.uz), FREE_SAVE);
        mklev();
        vision_reset();
        vision_full_recalc = 1;
        cls();
        (void) safe_teleds(TRUE);
        if (Punished) {
            unplacebc();
            placebc();
        }
        docrt();
        flush_screen(1);
    }
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
    char buf[BUFSZ] = DUMMY;
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
            row[x - 1] = (char) ((terrain == STONE && !may_dig(x, y))
                                    ? '*'
                                    : (terrain < 10)
                                       ? '0' + terrain
                                       : (terrain < 36)
                                          ? 'a' + terrain - 10
                                          : 'A' + terrain - 36);
        }
        x--;
        if (levl[0][y].typ != STONE || may_dig(0, y))
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

/* #wizinstrinsic command to set some intrinsics for testing */
STATIC_PTR int
wiz_intrinsic(VOID_ARGS)
{
    if (wizard) {
        extern const struct propname {
            int prop_num;
            const char *prop_name;
        } propertynames[]; /* timeout.c */
        static const char wizintrinsic[] = "#wizintrinsic";
        static const char fmt[] = "You are%s %s.";
        winid win;
        anything any;
        char buf[BUFSZ];
        int i, j, n, p, amt, typ;
        long oldtimeout, newtimeout;
        const char *propname;
        menu_item *pick_list = (menu_item *) 0;

        any = zeroany;
        win = create_nhwindow(NHW_MENU);
        start_menu(win);
        for (i = 0; (propname = propertynames[i].prop_name) != 0; ++i) {
            p = propertynames[i].prop_num;
            if (p == HALLUC_RES) {
                /* Grayswandir vs hallucination; ought to be redone to
                   use u.uprops[HALLUC].blocked instead of being treated
                   as a separate property; letting in be manually toggled
                   even only in wizard mode would be asking for trouble... */
                continue;
            }
            if (p == FIRE_RES) {
                any.a_int = 0;
                add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, "--", FALSE);
            }
            any.a_int = i + 1; /* +1: avoid 0 */
            oldtimeout = u.uprops[p].intrinsic & TIMEOUT;
            if (oldtimeout)
                Sprintf(buf, "%-27s [%li]", propname, oldtimeout);
            else
                Sprintf(buf, "%s", propname);
            add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
        }
        end_menu(win, "Which intrinsics?");
        n = select_menu(win, PICK_ANY, &pick_list);
        destroy_nhwindow(win);

        amt = 30; /* TODO: prompt for duration */
        for (j = 0; j < n; ++j) {
            i = pick_list[j].item.a_int - 1; /* -1: reverse +1 above */
            p = propertynames[i].prop_num;
            oldtimeout = u.uprops[p].intrinsic & TIMEOUT;
            newtimeout = oldtimeout + (long) amt;
            switch (p) {
            case SICK:
            case SLIMED:
            case STONED:
                if (oldtimeout > 0L && newtimeout > oldtimeout)
                    newtimeout = oldtimeout;
                break;
            }

            switch (p) {
            case BLINDED:
                make_blinded(newtimeout, TRUE);
                break;
#if 0       /* make_confused() only gives feedback when confusion is
             * ending so use the 'default' case for it instead */
            case CONFUSION:
                make_confused(newtimeout, TRUE);
                break;
#endif /*0*/
            case DEAF:
                make_deaf(newtimeout, TRUE);
                break;
            case HALLUC:
                make_hallucinated(newtimeout, TRUE, 0L);
                break;
            case SICK:
                typ = !rn2(2) ? SICK_VOMITABLE : SICK_NONVOMITABLE;
                make_sick(newtimeout, wizintrinsic, TRUE, typ);
                break;
            case SLIMED:
                Sprintf(buf, fmt,
                        !Slimed ? "" : " still", "turning into slime");
                make_slimed(newtimeout, buf);
                break;
            case STONED:
                Sprintf(buf, fmt,
                        !Stoned ? "" : " still", "turning into stone");
                make_stoned(newtimeout, buf, KILLED_BY, wizintrinsic);
                break;
            case STUNNED:
                make_stunned(newtimeout, TRUE);
                break;
            case VOMITING:
                Sprintf(buf, fmt, !Vomiting ? "" : " still", "vomiting");
                make_vomiting(newtimeout, FALSE);
                pline1(buf);
                break;
            default:
                pline("Timeout for %s %s %d.", propertynames[i].prop_name,
                      oldtimeout ? "increased by" : "set to", amt);
                incr_itimeout(&u.uprops[p].intrinsic, amt);
                break;
            }
            context.botl = 1; /* probably not necessary... */
        }
        if (n >= 1)
            free((genericptr_t) pick_list);
        doredraw();
    } else
        pline("Unavailable command '%s'.",
              visctrl((int) cmd_from_func(wiz_intrinsic)));
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
    start_menu(men);
    any = zeroany;
    any.a_int = 1;
    add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
/*JP
             "known map without monsters, objects, and traps",
*/
             "怪物，物，罠なしの地図",
             MENU_SELECTED);
    any.a_int = 2;
    add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
/*JP
             "known map without monsters and objects",
*/
             "怪物，物なしの地図",
             MENU_UNSELECTED);
    any.a_int = 3;
    add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
/*JP
             "known map without monsters",
*/
             "怪物なしの地図",
             MENU_UNSELECTED);
    if (discover || wizard) {
        any.a_int = 4;
        add_menu(men, NO_GLYPH, &any, 0, 0, ATR_NONE,
/*JP
                 "full map without monsters, objects, and traps",
*/
                 "怪物，物，罠なしの完全な地図",
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
/*JP
    end_menu(men, "View which?");
*/
    end_menu(men, "どれを見る？");

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
    case 1: /* known map */
        reveal_terrain(0, TER_MAP);
        break;
    case 2: /* known map with known traps */
        reveal_terrain(0, TER_MAP | TER_TRP);
        break;
    case 3: /* known map with known traps and objects */
        reveal_terrain(0, TER_MAP | TER_TRP | TER_OBJ);
        break;
    case 4: /* full map */
        reveal_terrain(1, TER_MAP);
        break;
    case 5: /* map internals */
        wiz_map_levltyp();
        break;
    case 6: /* internal details */
        wiz_levltyp_legend();
        break;
    default:
        break;
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
#if 0 /*JP*/
    boolean invrt;
#endif
    int absamt;

    absamt = abs(incamt);
    /* Protection amount is typically larger than damage or to-hit;
       reduce magnitude by a third in order to stretch modifier ranges
       (small:1..5, moderate:6..10, large:11..19, huge:20+) */
#if 0 /*JP*/
    if (!strcmp(inctyp, "defense"))
#else
    if (!strcmp(inctyp, "防御"))
#endif
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
#if 0 /*JP*/
    invrt = strcmp(inctyp, "to hit") ? TRUE : FALSE;
#endif

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
/*JP
        category_name = "physical";
*/
        category_name = "物理";
        break;
    case HALF_SPDAM:
/*JP
        category_name = "spell";
*/
        category_name = "呪文";
        break;
    default:
/*JP
        category_name = "unknown";
*/
        category_name = "不明";
        break;
    }
#if 0 /*JP*/
    Sprintf(buf, " %s %s damage", (final || wizard) ? "half" : "reduced",
            category_name);
    enl_msg(You_, "take", "took", buf, from_what(category));
#else
    Sprintf(buf, " %sダメージを%s", (final || wizard) ? "半減" : "減少",
            category_name);
    enl_msg(You_, "している", "していた", buf, from_what(category));
#endif
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
/*JP
        you_are(buf, "");
*/
        you_are_ing(buf, "");
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

    /* report alignment (bypass you_are() in order to omit ending period);
       adverb is used to distinguish between temporary change (helm of opp.
       alignment), permanent change (one-time conversion), and original */
#if 0 /*JP*/
    Sprintf(buf, " %s%s%s, %son a mission for %s",
            You_, !final ? are : were,
            align_str(u.ualign.type),
            /* helm of opposite alignment (might hide conversion) */
            (u.ualign.type != u.ualignbase[A_CURRENT])
               /* what's the past tense of "currently"? if we used "formerly"
                  it would sound like a reference to the original alignment */
               ? (!final ? "currently " : "temporarily ")
               /* permanent conversion */
               : (u.ualign.type != u.ualignbase[A_ORIGINAL])
                  /* and what's the past tense of "now"? certainly not "then"
                     in a context like this...; "belatedly" == weren't that
                     way sooner (in other words, didn't start that way) */
                  ? (!final ? "now " : "belatedly ")
                  /* atheist (ignored in very early game) */
                  : (!u.uconduct.gnostic && moves > 1000L)
                     ? "nominally "
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
/*JP
        you_are(buf, "");
*/
        enl_msg(buf, "ている", "ていた", "", "");
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

/* characteristics: expanded version of bottom line strength, dexterity, &c;
   [3.6.1: now includes all status info (except things already shown in the
   'background' section), primarily so that blind players can suppress the
   status line(s) altogether and use ^X feedback on demand to view HP, &c] */
STATIC_OVL void
characteristics_enlightenment(mode, final)
int mode;
int final;
{
    char buf[BUFSZ];
    int hp = Upolyd ? u.mh : u.uhp;
    int hpmax = Upolyd ? u.mhmax : u.uhpmax;

    putstr(en_win, 0, ""); /* separator after background */
    putstr(en_win, 0,
/*JP
           final ? "Final Characteristics:" : "Current Characteristics:");
*/
           final ? "最終属性：" : "現在の属性：");

    if (hp < 0)
        hp = 0;
/*JP
    Sprintf(buf, "%d hit points (max:%d)", hp, hpmax);
*/
    Sprintf(buf, "%dヒットポイント(最大:%d)", hp, hpmax);
    you_have(buf, "");

/*JP
    Sprintf(buf, "%d magic power (max:%d)", u.uen, u.uenmax);
*/
    Sprintf(buf, "%d魔力ポイント(最大:%d)", u.uen, u.uenmax);
    you_have(buf, "");

    Sprintf(buf, "%d", u.uac);
/*JP
    enl_msg("Your armor class ", "is ", "was ", buf, "");
*/
    enl_msg("あなたの防御値は", "である", "であった", buf, "");

    if (Upolyd) {
        switch (mons[u.umonnum].mlevel) {
        case 0:
            /* status line currently being explained shows "HD:0" */
/*JP
            Strcpy(buf, "0 hit dice (actually 1/2)");
*/
            Strcpy(buf, "HD0(実際には1/2)");
            break;
        case 1:
/*JP
            Strcpy(buf, "1 hit die");
*/
            Strcpy(buf, "HD1");
            break;
        default:
/*JP
            Sprintf(buf, "%d hit dice", mons[u.umonnum].mlevel);
*/
            Sprintf(buf, "HD%d", mons[u.umonnum].mlevel);
            break;
        }
    } else {
        /* flags.showexp does not matter */
        /* experience level is already shown in the Background section */
#if 0 /*JP*/
        Sprintf(buf, "%-1ld experience point%s",
                u.uexp, plur(u.uexp));
#else
        Sprintf(buf, "経験値%-1ldポイント",
                u.uexp);
#endif
    }
    you_have(buf, "");

    /* this is shown even if the 'time' option is off */
/*JP
    Sprintf(buf, "the dungeon %ld turn%s ago", moves, plur(moves));
*/
    Sprintf(buf, "%ldターン前に迷宮に入った", moves);
    /* same phrasing at end of game:  "entered" is unconditional */
    enlght_line(You_, "", buf, "");

#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        /* describes what's shown on status line, which is an approximation;
           only show it here if player has the 'showscore' option enabled */
#if 0 /*JP*/
        Sprintf(buf, "%ld%s", botl_score(),
                !final ? "" : " before end-of-game adjustments");
        enl_msg("Your score ", "is ", "was ", buf, "");
#else
        Sprintf(buf, "%s%ld", botl_score(),
                !final ? "" : "ゲーム終了時の調整前は");
        enl_msg("あなたのスコアは", "である", "であった", buf, "");
#endif
    }
#endif

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
    extern const char *const attrname[]; /* attrib.c */
    boolean hide_innate_value = FALSE, interesting_alimit;
    int acurrent, abase, apeak, alimit;
    const char *paren_pfx;
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
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER && uarmg->cursed)
            hide_innate_value = TRUE;
        break;
    case A_DEX:
        break;
    case A_CON:
        if (uwep && uwep->oartifact == ART_OGRESMASHER && uwep->cursed)
            hide_innate_value = TRUE;
        break;
    case A_INT:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_WIS:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_CHA:
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
    Sprintf(subjbuf, "Your %s ", attrname[attrindx]);
*/
    Sprintf(subjbuf, "あなたの%sは", attrname[attrindx]);

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
    if (Upolyd) {
#if 0 /*JP*/
        Strcpy(buf, "transformed");
        if (ugenocided())
            Sprintf(eos(buf), " and %s %s inside",
                    final ? "felt" : "feel", udeadinside());
        you_are(buf, "");
#else /*JP:TODO:変化+虐殺パターン*/
        you_are_ing("変化して", "");
#endif
    }
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
        you_are(buf, "");
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
#if 0 /*JP*/
            Sprintf(buf, "stuck to %s %s", s_suffix(steedname),
                    simpleonames(saddle));
            you_are(buf, "");
#else
            Sprintf(buf, "%sの%sにつかまって", steedname,
                    simpleonames(saddle));
            you_are_ing(buf, "");
#endif
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
#if 0 /*JP*/
    static NEARDATA const char if_surroundings_permitted[] =
        " if surroundings permitted";
#endif
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

/*JP
    Sprintf(buf, "%s", piousness(TRUE, "aligned"));
*/
    Sprintf(buf, "%s", piousness(TRUE, "信仰心"));
    if (u.ualign.record >= 0)
        you_are(buf, "");
    else
        you_have(buf, "");

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
    if (Warn_of_mon && context.warntype.speciesidx >= LOW_PM) {
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
            you_are("状況が許せば浮遊する状態", "");
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
            enl_msg(You_, "できる", "できた",
                    "飛ぶことが",
                    Levitation
                       ? "浮遊していなければ"
                       : (save_BFly == FROMOUTSIDE)
                          ? "状況が許せば"
                          /* both surroundings and [latent] levitation */
                          : "事情が許せば");
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
        you_are(buf, "");
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
/*JP
        you_are("harmed by silver", "");
*/
        enl_msg("あなたは銀に弱", "い", "かった", "", "");
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

#ifdef DEBUG
    /* named fruit debugging (doesn't really belong here...); to enable,
       include 'fruit' in DEBUGFILES list (even though it isn't a file...) */
    if (wizard && explicitdebug("fruit")) {
        struct fruit *f;

        reorder_fruit(TRUE); /* sort by fruit index, from low to high;
                              * this modifies the ffruit chain, so could
                              * possibly mask or even introduce a problem,
                              * but it does useful sanity checking */
        for (f = ffruit; f; f = f->nextf) {
/*JP
            Sprintf(buf, "Fruit #%d ", f->fid);
*/
            Sprintf(buf, "fruit $%d は", f->fid);
/*JP
            enl_msg(buf, "is ", "was ", f->fname, "");
*/
            enl_msg(buf, "だ", "だった", f->fname, "");
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
#endif

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
#if 0 /*JP*//*後ろに回す*//* not used */
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

        you_are(buf, "");
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
int
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
        /* but beverages are okay */
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
#if 0 /*JP:T*/
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
#if 0 /*JP:T*/
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
#if 0 /*JP:T*/
        Sprintf(buf, "polymorphed %ld item%s", u.uconduct.polypiles,
                plur(u.uconduct.polypiles));
        you_have_X(buf);
#else
        Sprintf(buf, "%ld個の物を変化させ", u.uconduct.polypiles);
        you_have_X(buf);
#endif
    }

    if (!u.uconduct.polyselfs) {
/*JP
        you_have_never("changed form");
*/
        you_have_never("あなたは変化し");
    } else if (wizard) {
#if 0 /*JP:T*/
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
#else
        Sprintf(buf, "%ld回願い事をし", u.uconduct.wishes);
#endif
        if (u.uconduct.wisharti) {
            /* if wisharti == wishes
             *  1 wish (for an artifact)
             *  2 wishes (both for artifacts)
             *  N wishes (all for artifacts)
             * else (N is at least 2 in order to get here; M < N)
             *  N wishes (1 for an artifact)
             *  N wishes (M for artifacts)
             */
            if (u.uconduct.wisharti == u.uconduct.wishes)
                Sprintf(eos(buf), " (%s",
                        (u.uconduct.wisharti > 2L) ? "all "
                          : (u.uconduct.wisharti == 2L) ? "both " : "");
            else
                Sprintf(eos(buf), " (%ld ", u.uconduct.wisharti);

            Sprintf(eos(buf), "for %s)",
                    (u.uconduct.wisharti == 1L) ? "an artifact"
                                                : "artifacts");
        }
        you_have_X(buf);

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

/* Macros for meta and ctrl modifiers:
 *   M and C return the meta/ctrl code for the given character;
 *     e.g., (C('c') is ctrl-c
 */
#ifndef M
#ifndef NHSTDC
#define M(c) (0x80 | (c))
#else
#define M(c) ((c) - 128)
#endif /* NHSTDC */
#endif

#ifndef C
#define C(c) (0x1f & (c))
#endif

/* ordered by command name */
struct ext_func_tab extcmdlist[] = {
    { '#', "#", "perform an extended command",
            doextcmd, IFBURIED | GENERALCMD },
#if 0 /*JP*/
    { M('?'), "?", "list all extended commands",
#else
    { M('?'), "?", "この拡張コマンド一覧を表示する",
#endif
            doextlist, IFBURIED | AUTOCOMPLETE | GENERALCMD },
#if 0 /*JP*/
    { M('a'), "adjust", "adjust inventory letters",
#else
    { M('a'), "adjust", "持ち物一覧の調整",
#endif
            doorganize, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { M('A'), "annotate", "name current level",
#else
    { M('A'), "annotate", "現在の階に名前をつける",
#endif
            donamelevel, IFBURIED | AUTOCOMPLETE },
    { 'a', "apply", "apply (use) a tool (pick-axe, key, lamp...)",
            doapply },
    { C('x'), "attributes", "show your attributes",
            doattributes, IFBURIED },
    { '@', "autopickup", "toggle the pickup option on/off",
            dotogglepickup, IFBURIED },
    { 'C', "call", "call (name) something", docallcmd, IFBURIED },
    { 'Z', "cast", "zap (cast) a spell", docast, IFBURIED },
#if 0 /*JP*/
    { M('c'), "chat", "talk to someone", dotalk, IFBURIED | AUTOCOMPLETE },
#else
    { M('c'), "chat", "誰かと話す", dotalk, IFBURIED | AUTOCOMPLETE },
#endif
    { 'c', "close", "close a door", doclose },
#if 0 /*JP*/
    { M('C'), "conduct", "list voluntary challenges you have maintained",
#else
    { M('C'), "conduct", "どういう行動をとったか見る",
#endif
            doconduct, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { M('d'), "dip", "dip an object into something", dodip, AUTOCOMPLETE },
#else
    { M('d'), "dip", "何かに物を浸す", dodip, AUTOCOMPLETE },
#endif
    { '>', "down", "go down a staircase", dodown },
    { 'd', "drop", "drop an item", dodrop },
    { 'D', "droptype", "drop specific item types", doddrop },
    { 'e', "eat", "eat something", doeat },
    { 'E', "engrave", "engrave writing on the floor", doengrave },
#if 0 /*JP*/
    { M('e'), "enhance", "advance or check weapon and spell skills",
#else
    { M('e'), "enhance", "武器熟練度を高める",
#endif
            enhance_weapon_skill, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { '\0', "exploremode", "enter explore (discovery) mode",
#else
    { '\0', "exploremode", "探検(発見)モードに入る",
#endif
            enter_explore_mode, IFBURIED },
    { 'f', "fire", "fire ammunition from quiver", dofire },
#if 0 /*JP*/
    { M('f'), "force", "force a lock", doforce, AUTOCOMPLETE },
#else
    { M('f'), "force", "鍵をこじあける", doforce, AUTOCOMPLETE },
#endif
    { ';', "glance", "show what type of thing a map symbol corresponds to",
            doquickwhatis, IFBURIED | GENERALCMD },
    { '?', "help", "give a help message", dohelp, IFBURIED | GENERALCMD },
    { '\0', "herecmdmenu", "show menu of commands you can do here",
            doherecmdmenu, IFBURIED },
    { 'V', "history", "show long version and game history",
            dohistory, IFBURIED | GENERALCMD },
    { 'i', "inventory", "show your inventory", ddoinv, IFBURIED },
    { 'I', "inventtype", "inventory specific item types",
            dotypeinv, IFBURIED },
#if 0 /*JP*/
    { M('i'), "invoke", "invoke an object's special powers",
#else
    { M('i'), "invoke", "物の特別な力を使う",
#endif
            doinvoke, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { M('j'), "jump", "jump to another location", dojump, AUTOCOMPLETE },
#else
    { M('j'), "jump", "他の位置に飛びうつる", dojump, AUTOCOMPLETE },
#endif
    { C('d'), "kick", "kick something", dokick },
    { '\\', "known", "show what object types have been discovered",
            dodiscovered, IFBURIED | GENERALCMD },
    { '`', "knownclass", "show discovered types for one class of objects",
            doclassdisco, IFBURIED | GENERALCMD },
#if 0 /*JP*/
    { '\0', "levelchange", "change experience level",
#else
    { '\0', "levelchange", "経験レベルを変える",
#endif
            wiz_level_change, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { '\0', "lightsources", "show mobile light sources",
#else
    { '\0', "lightsources", "移動光源を見る",
#endif
            wiz_light_sources, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { ':', "look", "look at what is here", dolook, IFBURIED },
#if 0 /*JP*/
    { M('l'), "loot", "loot a box on the floor", doloot, AUTOCOMPLETE },
#else
    { M('l'), "loot", "床の上の箱を開ける", doloot, AUTOCOMPLETE },
#endif
#ifdef DEBUG_MIGRATING_MONS
#if 0 /*JP*/
    { '\0', "migratemons", "migrate N random monsters",
#else
    { '\0', "migratemons", "ランダムな怪物を何体か移住させる",
#endif
            wiz_migrate_mons, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#endif
#if 0 /*JP*/
    { '\0', "monpolycontrol", "control monster polymorphs",
#else
    { '\0', "monpolycontrol", "怪物への変化を制御する",
#endif
            wiz_mon_polycontrol, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { M('m'), "monster", "use monster's special ability",
#else
    { M('m'), "monster", "怪物の特別能力を使う",
#endif
            domonability, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { 'N', "name", "name a monster or an object",
#else
    { 'N', "name", "アイテムや物に名前をつける",
#endif
            docallcmd, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { M('o'), "offer", "offer a sacrifice to the gods",
#else
    { M('o'), "offer", "神に供物を捧げる",
#endif
            dosacrifice, AUTOCOMPLETE },
    { 'o', "open", "open a door", doopen },
    { 'O', "options", "show option settings, possibly change them",
            doset, IFBURIED | GENERALCMD },
#if 0 /*JP*/
    { C('o'), "overview", "show a summary of the explored dungeon",
#else
    { C('o'), "overview", "探索した迷宮の概要を表示する",
#endif
            dooverview, IFBURIED | AUTOCOMPLETE },
#if 0 /*JP*/
    { '\0', "panic", "test panic routine (fatal to game)",
#else
    { '\0', "panic", "パニックルーチンをテストする(致命的)",
#endif
            wiz_panic, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { 'p', "pay", "pay your shopping bill", dopay },
    { ',', "pickup", "pick up things at the current location", dopickup },
#if 0 /*JP*/
    { '\0', "polyself", "polymorph self",
#else
    { '\0', "polyself", "変化する",
#endif
            wiz_polyself, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#ifdef PORT_DEBUG
#if 0 /*JP*/
    { '\0', "portdebug", "wizard port debug command",
#else
    { '\0', "portdebug", "ウィザードポートデバッグコマンド",
#endif
            wiz_port_debug, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#endif
#if 0 /*JP*/
    { M('p'), "pray", "pray to the gods for help",
#else
    { M('p'), "pray", "神に祈る",
#endif
            dopray, IFBURIED | AUTOCOMPLETE },
    { C('p'), "prevmsg", "view recent game messages",
            doprev_message, IFBURIED | GENERALCMD },
    { 'P', "puton", "put on an accessory (ring, amulet, etc)", doputon },
    { 'q', "quaff", "quaff (drink) something", dodrink },
#if 0 /*JP*/
    { M('q'), "quit", "exit without saving current game",
#else
    { M('q'), "quit", "セーブしないで終了",
#endif
            done2, IFBURIED | AUTOCOMPLETE | GENERALCMD },
    { 'Q', "quiver", "select ammunition for quiver", dowieldquiver },
    { 'r', "read", "read a scroll or spellbook", doread },
    { C('r'), "redraw", "redraw screen", doredraw, IFBURIED | GENERALCMD },
    { 'R', "remove", "remove an accessory (ring, amulet, etc)", doremring },
#if 0 /*JP*/
    { M('R'), "ride", "mount or dismount a saddled steed",
#else
    { M('R'), "ride", "怪物に乗る(または降りる)",
#endif
            doride, AUTOCOMPLETE },
#if 0 /*JP*/
    { M('r'), "rub", "rub a lamp or a stone", dorub, AUTOCOMPLETE },
#else
    { M('r'), "rub", "ランプをこする", dorub, AUTOCOMPLETE },
#endif
    { 'S', "save", "save the game and exit", dosave, IFBURIED | GENERALCMD },
#if 0 /*JP*/
    { 's', "search", "search for traps and secret doors",
            dosearch, IFBURIED, "searching" },
#else
    { 's', "search", "罠や隠し扉を探す",
            dosearch, IFBURIED, "探す" },
#endif
    { '*', "seeall", "show all equipment in use", doprinuse, IFBURIED },
    { AMULET_SYM, "seeamulet", "show the amulet currently worn",
            dopramulet, IFBURIED },
    { ARMOR_SYM, "seearmor", "show the armor currently worn",
            doprarm, IFBURIED },
    { GOLD_SYM, "seegold", "count your gold", doprgold, IFBURIED },
#if 0 /*JP*/
    { '\0', "seenv", "show seen vectors",
#else
    { '\0', "seenv", "視線ベクトルを見る",
#endif
            wiz_show_seenv, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { RING_SYM, "seerings", "show the ring(s) currently worn",
            doprring, IFBURIED },
    { SPBOOK_SYM, "seespells", "list and reorder known spells",
            dovspell, IFBURIED },
    { TOOL_SYM, "seetools", "show the tools currently in use",
            doprtool, IFBURIED },
    { '^', "seetrap", "show the type of adjacent trap", doidtrap, IFBURIED },
    { WEAPON_SYM, "seeweapon", "show the weapon currently wielded",
            doprwep, IFBURIED },
#ifdef SHELL
    { '!', "shell", "do a shell escape", dosh, IFBURIED | GENERALCMD },
#endif /* SHELL */
#if 0 /*JP*/
    { M('s'), "sit", "sit down", dosit, AUTOCOMPLETE },
    { '\0', "stats", "show memory statistics",
#else
    { M('s'), "sit", "座る", dosit, AUTOCOMPLETE },
    { '\0', "stats", "メモリ状態を見る",
#endif
            wiz_show_stats, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#ifdef SUSPEND
    { C('z'), "suspend", "suspend the game",
            dosuspend_core, IFBURIED | GENERALCMD },
#endif /* SUSPEND */
    { 'x', "swap", "swap wielded and secondary weapons", doswapweapon },
    { 'T', "takeoff", "take off one piece of armor", dotakeoff },
    { 'A', "takeoffall", "remove all armor", doddoremarm },
    { C('t'), "teleport", "teleport around the level", dotele, IFBURIED },
#if 0 /*JP*/
    { '\0', "terrain", "show map without obstructions",
#else
    { '\0', "terrain", "邪魔されずに地図を見る",
#endif
            doterrain, IFBURIED | AUTOCOMPLETE },
    { '\0', "therecmdmenu",
            "menu of commands you can do from here to adjacent spot",
            dotherecmdmenu },
    { 't', "throw", "throw something", dothrow },
#if 0 /*JP*/
    { '\0', "timeout", "look at timeout queue and hero's timed intrinsics",
#else
    { '\0', "timeout", "時間切れキューとプレイヤーの時間経過を見る",
#endif
            wiz_timeout_queue, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { M('T'), "tip", "empty a container", dotip, AUTOCOMPLETE },
#else
    { M('T'), "tip", "入れ物を空にする", dotip, AUTOCOMPLETE },
#endif
    { '_', "travel", "travel to a specific location on the map", dotravel },
#if 0 /*JP*/
    { M('t'), "turn", "turn undead away", doturn, IFBURIED | AUTOCOMPLETE },
#else
    { M('t'), "turn", "アンデットを土に返す", doturn, IFBURIED | AUTOCOMPLETE },
#endif
#if 0 /*JP*/
    { 'X', "twoweapon", "toggle two-weapon combat",
#else
    { 'X', "twoweapon", "両手持ちの切り替え",
#endif
            dotwoweapon, AUTOCOMPLETE },
#if 0 /*JP*/
    { M('u'), "untrap", "untrap something", dountrap, AUTOCOMPLETE },
#else
    { M('u'), "untrap", "罠をはずす", dountrap, AUTOCOMPLETE },
#endif
    { '<', "up", "go up a staircase", doup },
#if 0 /*JP*/
    { '\0', "vanquished", "list vanquished monsters",
#else
    { '\0', "vanquished", "倒した怪物の一覧を見る",
#endif
            dovanquished, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { M('v'), "version",
#if 0 /*JP*/
            "list compile time options for this version of NetHack",
#else
            "コンパイル時のオプションを表示する",
#endif
            doextversion, IFBURIED | AUTOCOMPLETE | GENERALCMD },
    { 'v', "versionshort", "show version", doversion, IFBURIED | GENERALCMD },
#if 0 /*JP*/
    { '\0', "vision", "show vision array",
#else
    { '\0', "vision", "視界配列を見る",
#endif
            wiz_show_vision, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { '.', "wait", "rest one move while doing nothing",
            donull, IFBURIED, "waiting" },
#else
    { '.', "wait", "一歩分何もしない",
            donull, IFBURIED, "休憩する" },
#endif
    { 'W', "wear", "wear a piece of armor", dowear },
    { '&', "whatdoes", "tell what a command does", dowhatdoes, IFBURIED },
    { '/', "whatis", "show what type of thing a symbol corresponds to",
            dowhatis, IFBURIED | GENERALCMD },
    { 'w', "wield", "wield (put in use) a weapon", dowield },
#if 0 /*JP*/
    { M('w'), "wipe", "wipe off your face", dowipe, AUTOCOMPLETE },
#else
    { M('w'), "wipe", "顔を拭う", dowipe, AUTOCOMPLETE },
#endif
#ifdef DEBUG
#if 0 /*JP*/
    { '\0', "wizdebug_bury", "wizard debug: bury objs under and around you",
#else
    { '\0', "wizdebug_bury", "ウィザードでバッグ: 物をあなたの周りに埋める",
#endif
            wiz_debug_cmd_bury, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { '\0', "wizdebug_traveldisplay", "wizard debug: toggle travel display",
#else
    { '\0', "wizdebug_traveldisplay", "ウィザードデバッグ: 移動表示を切り替える",
#endif
          wiz_debug_cmd_traveldisplay, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#endif
    { C('e'), "wizdetect", "reveal hidden things within a small radius",
            wiz_detect, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { C('g'), "wizgenesis", "create a monster",
            wiz_genesis, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { C('i'), "wizidentify", "identify all items in inventory",
            wiz_identify, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { '\0', "wizintrinsic", "set an intrinsic",
            wiz_intrinsic, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { C('v'), "wizlevelport", "teleport to another level",
            wiz_level_tele, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { '\0', "wizmakemap", "recreate the current level",
            wiz_makemap, IFBURIED | WIZMODECMD },
    { C('f'), "wizmap", "map the level",
            wiz_map, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { '\0', "wizrumorcheck", "verify rumor boundaries",
#else
    { '\0', "wizrumorcheck", "噂の境界を検証する",
#endif
            wiz_rumor_check, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { '\0', "wizsmell", "smell monster",
#else
    { '\0', "wizsmell", "怪物の匂いを嗅ぐ",
#endif
            wiz_smell, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { '\0', "wizwhere", "show locations of special levels",
            wiz_where, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { C('w'), "wizwish", "wish for something",
            wiz_wish, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
#if 0 /*JP*/
    { '\0', "wmode", "show wall modes",
#else
    { '\0', "wmode", "壁モードを見る",
#endif
            wiz_show_wmodes, IFBURIED | AUTOCOMPLETE | WIZMODECMD },
    { 'z', "zap", "zap a wand", dozap },
    { '\0', (char *) 0, (char *) 0, donull, 0, (char *) 0 } /* sentinel */
};

const char *
key2extcmddesc(key)
uchar key;
{
    if (Cmd.commands[key] && Cmd.commands[key]->ef_txt)
        return Cmd.commands[key]->ef_desc;
    return (char *) 0;
}

boolean
bind_key(key, command)
uchar key;
const char *command;
{
    struct ext_func_tab *extcmd;

    /* special case: "nothing" is reserved for unbinding */
    if (!strcmp(command, "nothing")) {
        Cmd.commands[key] = (struct ext_func_tab *) 0;
        return TRUE;
    }

    for (extcmd = extcmdlist; extcmd->ef_txt; extcmd++) {
        if (strcmp(command, extcmd->ef_txt))
            continue;
        Cmd.commands[key] = extcmd;
        return TRUE;
    }

    return FALSE;
}

/* initialize all keyboard commands */
void
commands_init()
{
    struct ext_func_tab *extcmd;

    for (extcmd = extcmdlist; extcmd->ef_txt; extcmd++)
        if (extcmd->key)
            Cmd.commands[extcmd->key] = extcmd;

    (void) bind_key(C('l'), "redraw"); /* if number_pad is set */
    /*       'b', 'B' : go sw */
    /*       'F' : fight (one time) */
    /*       'g', 'G' : multiple go */
    /*       'h', 'H' : go west */
    (void) bind_key('h',    "help"); /* if number_pad is set */
    (void) bind_key('j',    "jump"); /* if number_pad is on */
    /*       'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' move commands */
    (void) bind_key('k',    "kick"); /* if number_pad is on */
    (void) bind_key('l',    "loot"); /* if number_pad is on */
    (void) bind_key(C('n'), "annotate"); /* if number_pad is on */
    (void) bind_key(M('n'), "name");
    (void) bind_key(M('N'), "name");
    (void) bind_key('u',    "untrap"); /* if number_pad is on */

    /* alt keys: */
    (void) bind_key(M('O'), "overview");
    (void) bind_key(M('2'), "twoweapon");

    /* wait_on_space */
    (void) bind_key(' ',    "wait");
}

int
dokeylist_putcmds(datawin, docount, cmdflags, exflags, keys_used)
winid datawin;
boolean docount;
int cmdflags, exflags;
boolean *keys_used; /* boolean keys_used[256] */
{
    int i;
    char buf[BUFSZ];
    char buf2[QBUFSZ];
    int count = 0;

    for (i = 0; i < 256; i++) {
        const struct ext_func_tab *extcmd;
        uchar key = (uchar) i;

        if (keys_used[i])
            continue;
        if (key == ' ' && !flags.rest_on_space)
            continue;
        if ((extcmd = Cmd.commands[i]) != (struct ext_func_tab *) 0) {
            if ((cmdflags && !(extcmd->flags & cmdflags))
                || (exflags && (extcmd->flags & exflags)))
                continue;
            if (docount) {
                count++;
                continue;
            }
            Sprintf(buf, "%-8s %-12s %s", key2txt(key, buf2),
                    extcmd->ef_txt,
                    extcmd->ef_desc);
            putstr(datawin, 0, buf);
            keys_used[i] = TRUE;
        }
    }
    return count;
}

/* list all keys and their bindings, like dat/hh but dynamic */
void
dokeylist(VOID_ARGS)
{
    char buf[BUFSZ], buf2[BUFSZ];
    uchar key;
    boolean keys_used[256] = {0};
    winid datawin;
    int i;
    static const char
        run_desc[] = "Prefix: run until something very interesting is seen",
        forcefight_desc[] =
                     "Prefix: force fight even if you don't see a monster";
    static const struct {
        int nhkf;
        const char *desc;
        boolean numpad;
    } misc_keys[] = {
        { NHKF_ESC, "escape from the current query/action", FALSE },
        { NHKF_RUSH,
          "Prefix: rush until something interesting is seen", FALSE },
        { NHKF_RUN, run_desc, FALSE },
        { NHKF_RUN2, run_desc, TRUE },
        { NHKF_FIGHT, forcefight_desc, FALSE },
        { NHKF_FIGHT2, forcefight_desc, TRUE } ,
        { NHKF_NOPICKUP,
          "Prefix: move without picking up objects/fighting", FALSE },
        { NHKF_RUN_NOPICKUP,
          "Prefix: run without picking up objects/fighting", FALSE },
        { NHKF_DOINV, "view inventory", TRUE },
        { NHKF_REQMENU, "Prefix: request a menu", FALSE },
#ifdef REDO
        { NHKF_DOAGAIN , "re-do: perform the previous command again", FALSE },
#endif
        { 0, (const char *) 0, FALSE }
    };

    datawin = create_nhwindow(NHW_TEXT);
    putstr(datawin, 0, "");
    putstr(datawin, 0, "            Full Current Key Bindings List");

    /* directional keys */
    putstr(datawin, 0, "");
    putstr(datawin, 0, "Directional keys:");
    show_direction_keys(datawin, '.', FALSE); /* '.'==self in direction grid */

    keys_used[(uchar) Cmd.move_NW] = keys_used[(uchar) Cmd.move_N]
        = keys_used[(uchar) Cmd.move_NE] = keys_used[(uchar) Cmd.move_W]
        = keys_used[(uchar) Cmd.move_E] = keys_used[(uchar) Cmd.move_SW]
        = keys_used[(uchar) Cmd.move_S] = keys_used[(uchar) Cmd.move_SE]
        = TRUE;

    if (!iflags.num_pad) {
        keys_used[(uchar) highc(Cmd.move_NW)]
            = keys_used[(uchar) highc(Cmd.move_N)]
            = keys_used[(uchar) highc(Cmd.move_NE)]
            = keys_used[(uchar) highc(Cmd.move_W)]
            = keys_used[(uchar) highc(Cmd.move_E)]
            = keys_used[(uchar) highc(Cmd.move_SW)]
            = keys_used[(uchar) highc(Cmd.move_S)]
            = keys_used[(uchar) highc(Cmd.move_SE)] = TRUE;
        keys_used[(uchar) C(Cmd.move_NW)]
            = keys_used[(uchar) C(Cmd.move_N)]
            = keys_used[(uchar) C(Cmd.move_NE)]
            = keys_used[(uchar) C(Cmd.move_W)]
            = keys_used[(uchar) C(Cmd.move_E)]
            = keys_used[(uchar) C(Cmd.move_SW)]
            = keys_used[(uchar) C(Cmd.move_S)]
            = keys_used[(uchar) C(Cmd.move_SE)] = TRUE;
        putstr(datawin, 0, "");
        putstr(datawin, 0,
          "Shift-<direction> will move in specified direction until you hit");
        putstr(datawin, 0, "        a wall or run into something.");
        putstr(datawin, 0,
          "Ctrl-<direction> will run in specified direction until something");
        putstr(datawin, 0, "        very interesting is seen.");
    }

    putstr(datawin, 0, "");
    putstr(datawin, 0, "Miscellaneous keys:");
    for (i = 0; misc_keys[i].desc; i++) {
        key = Cmd.spkeys[misc_keys[i].nhkf];
        if (key && ((misc_keys[i].numpad && iflags.num_pad)
                    || !misc_keys[i].numpad)) {
            keys_used[(uchar) key] = TRUE;
            Sprintf(buf, "%-8s %s", key2txt(key, buf2), misc_keys[i].desc);
            putstr(datawin, 0, buf);
        }
    }
#ifndef NO_SIGNAL
    putstr(datawin, 0, "^c       break out of NetHack (SIGINT)");
    keys_used[(uchar) C('c')] = TRUE;
#endif

    putstr(datawin, 0, "");
    show_menu_controls(datawin, TRUE);

    if (dokeylist_putcmds(datawin, TRUE, GENERALCMD, WIZMODECMD, keys_used)) {
        putstr(datawin, 0, "");
        putstr(datawin, 0, "General commands:");
        (void) dokeylist_putcmds(datawin, FALSE, GENERALCMD, WIZMODECMD,
                                 keys_used);
    }

    if (dokeylist_putcmds(datawin, TRUE, 0, WIZMODECMD, keys_used)) {
        putstr(datawin, 0, "");
        putstr(datawin, 0, "Game commands:");
        (void) dokeylist_putcmds(datawin, FALSE, 0, WIZMODECMD, keys_used);
    }

    if (wizard
        && dokeylist_putcmds(datawin, TRUE, WIZMODECMD, 0, keys_used)) {
        putstr(datawin, 0, "");
        putstr(datawin, 0, "Wizard-mode commands:");
        (void) dokeylist_putcmds(datawin, FALSE, WIZMODECMD, 0, keys_used);
    }

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);
}

char
cmd_from_func(fn)
int NDECL((*fn));
{
    int i;

    for (i = 0; i < 256; ++i)
        if (Cmd.commands[i] && Cmd.commands[i]->ef_funct == fn)
            return (char) i;
    return '\0';
}

/*
 * wizard mode sanity_check code
 */

static const char template[] = "%-27s  %4ld  %6ld";
static const char stats_hdr[] = "                             count  bytes";
static const char stats_sep[] = "---------------------------  ----- -------";

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
obj_chain(win, src, chain, force, total_count, total_size)
winid win;
const char *src;
struct obj *chain;
boolean force;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count = 0L, size = 0L;

    count_obj(chain, &count, &size, TRUE, FALSE);

    if (count || size || force) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
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

    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
}

STATIC_OVL void
contained_stats(win, src, total_count, total_size)
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

    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
}

STATIC_OVL int
size_monst(mtmp, incl_wsegs)
struct monst *mtmp;
boolean incl_wsegs;
{
    int sz = (int) sizeof (struct monst);

    if (mtmp->wormno && incl_wsegs)
        sz += size_wseg(mtmp);

    if (mtmp->mextra) {
        sz += (int) sizeof (struct mextra);
        if (MNAME(mtmp))
            sz += (int) strlen(MNAME(mtmp)) + 1;
        if (EGD(mtmp))
            sz += (int) sizeof (struct egd);
        if (EPRI(mtmp))
            sz += (int) sizeof (struct epri);
        if (ESHK(mtmp))
            sz += (int) sizeof (struct eshk);
        if (EMIN(mtmp))
            sz += (int) sizeof (struct emin);
        if (EDOG(mtmp))
            sz += (int) sizeof (struct edog);
        /* mextra->mcorpsenm doesn't point to more memory */
    }
    return sz;
}

STATIC_OVL void
mon_chain(win, src, chain, force, total_count, total_size)
winid win;
const char *src;
struct monst *chain;
boolean force;
long *total_count;
long *total_size;
{
    char buf[BUFSZ];
    long count, size;
    struct monst *mon;
    /* mon->wormno means something different for migrating_mons and mydogs */
    boolean incl_wsegs = !strcmpi(src, "fmon");

    count = size = 0L;
    for (mon = chain; mon; mon = mon->nmon) {
        count++;
        size += size_monst(mon, incl_wsegs);
    }
    if (count || size || force) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, src, count, size);
        putstr(win, 0, buf);
    }
}

STATIC_OVL void
misc_stats(win, total_count, total_size)
winid win;
long *total_count;
long *total_size;
{
    char buf[BUFSZ], hdrbuf[QBUFSZ];
    long count, size;
    int idx;
    struct trap *tt;
    struct damage *sd; /* shop damage */
    struct cemetery *bi; /* bones info */

    /* traps and engravings are output unconditionally;
     * others only if nonzero
     */
    count = size = 0L;
    for (tt = ftrap; tt; tt = tt->ntrap) {
        ++count;
        size += (long) sizeof *tt;
    }
    *total_count += count;
    *total_size += size;
    Sprintf(hdrbuf, "traps, size %ld", (long) sizeof (struct trap));
    Sprintf(buf, template, hdrbuf, count, size);
    putstr(win, 0, buf);

    count = size = 0L;
    engr_stats("engravings, size %ld+text", hdrbuf, &count, &size);
    *total_count += count;
    *total_size += size;
    Sprintf(buf, template, hdrbuf, count, size);
    putstr(win, 0, buf);

    count = size = 0L;
    light_stats("light sources, size %ld", hdrbuf, &count, &size);
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    timer_stats("timers, size %ld", hdrbuf, &count, &size);
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    for (sd = level.damagelist; sd; sd = sd->next) {
        ++count;
        size += (long) sizeof *sd;
    }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(hdrbuf, "shop damage, size %ld",
                (long) sizeof (struct damage));
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    region_stats("regions, size %ld+%ld*rect+N", hdrbuf, &count, &size);
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    for (bi = level.bonesinfo; bi; bi = bi->next) {
        ++count;
        size += (long) sizeof *bi;
    }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Sprintf(hdrbuf, "bones history, size %ld",
                (long) sizeof (struct cemetery));
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }

    count = size = 0L;
    for (idx = 0; idx < NUM_OBJECTS; ++idx)
        if (objects[idx].oc_uname) {
            ++count;
            size += (long) (strlen(objects[idx].oc_uname) + 1);
        }
    if (count || size) {
        *total_count += count;
        *total_size += size;
        Strcpy(hdrbuf, "object type names, text");
        Sprintf(buf, template, hdrbuf, count, size);
        putstr(win, 0, buf);
    }
}

/*
 * Display memory usage of all monsters and objects on the level.
 */
static int
wiz_show_stats()
{
    char buf[BUFSZ];
    winid win;
    long total_obj_size, total_obj_count,
         total_mon_size, total_mon_count,
         total_ovr_size, total_ovr_count,
         total_misc_size, total_misc_count;

    win = create_nhwindow(NHW_TEXT);
    putstr(win, 0, "Current memory statistics:");

    total_obj_count = total_obj_size = 0L;
    putstr(win, 0, stats_hdr);
    Sprintf(buf, "  Objects, base size %ld", (long) sizeof (struct obj));
    putstr(win, 0, buf);
    obj_chain(win, "invent", invent, TRUE, &total_obj_count, &total_obj_size);
    obj_chain(win, "fobj", fobj, TRUE, &total_obj_count, &total_obj_size);
    obj_chain(win, "buried", level.buriedobjlist, FALSE,
              &total_obj_count, &total_obj_size);
    obj_chain(win, "migrating obj", migrating_objs, FALSE,
              &total_obj_count, &total_obj_size);
    obj_chain(win, "billobjs", billobjs, FALSE,
              &total_obj_count, &total_obj_size);
    mon_invent_chain(win, "minvent", fmon, &total_obj_count, &total_obj_size);
    mon_invent_chain(win, "migrating minvent", migrating_mons,
                     &total_obj_count, &total_obj_size);
    contained_stats(win, "contained", &total_obj_count, &total_obj_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Obj total", total_obj_count, total_obj_size);
    putstr(win, 0, buf);

    total_mon_count = total_mon_size = 0L;
    putstr(win, 0, "");
    Sprintf(buf, "  Monsters, base size %ld", (long) sizeof (struct monst));
    putstr(win, 0, buf);
    mon_chain(win, "fmon", fmon, TRUE, &total_mon_count, &total_mon_size);
    mon_chain(win, "migrating", migrating_mons, FALSE,
              &total_mon_count, &total_mon_size);
    /* 'mydogs' is only valid during level change or end of game disclosure,
       but conceivably we've been called from within debugger at such time */
    if (mydogs) /* monsters accompanying hero */
        mon_chain(win, "mydogs", mydogs, FALSE,
                  &total_mon_count, &total_mon_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Mon total", total_mon_count, total_mon_size);
    putstr(win, 0, buf);

    total_ovr_count = total_ovr_size = 0L;
    putstr(win, 0, "");
    putstr(win, 0, "  Overview");
    overview_stats(win, template, &total_ovr_count, &total_ovr_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Over total", total_ovr_count, total_ovr_size);
    putstr(win, 0, buf);

    total_misc_count = total_misc_size = 0L;
    putstr(win, 0, "");
    putstr(win, 0, "  Miscellaneous");
    misc_stats(win, &total_misc_count, &total_misc_size);
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Misc total", total_misc_count, total_misc_size);
    putstr(win, 0, buf);

    putstr(win, 0, "");
    putstr(win, 0, stats_sep);
    Sprintf(buf, template, "  Grand total",
            (total_obj_count + total_mon_count
             + total_ovr_count + total_misc_count),
            (total_obj_size + total_mon_size
             + total_ovr_size + total_misc_size));
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
    char inbuf[BUFSZ] = DUMMY;
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

struct {
    int nhkf;
    char key;
    const char *name;
} const spkeys_binds[] = {
    { NHKF_ESC,              '\033', (char *) 0 }, /* no binding */
    { NHKF_DOAGAIN,          DOAGAIN, "repeat" },
    { NHKF_REQMENU,          'm', "reqmenu" },
    { NHKF_RUN,              'G', "run" },
    { NHKF_RUN2,             '5', "run.numpad" },
    { NHKF_RUSH,             'g', "rush" },
    { NHKF_FIGHT,            'F', "fight" },
    { NHKF_FIGHT2,           '-', "fight.numpad" },
    { NHKF_NOPICKUP,         'm', "nopickup" },
    { NHKF_RUN_NOPICKUP,     'M', "run.nopickup" },
    { NHKF_DOINV,            '0', "doinv" },
    { NHKF_TRAVEL,           CMD_TRAVEL, (char *) 0 }, /* no binding */
    { NHKF_CLICKLOOK,        CMD_CLICKLOOK, (char *) 0 }, /* no binding */
    { NHKF_REDRAW,           C('r'), "redraw" },
    { NHKF_REDRAW2,          C('l'), "redraw.numpad" },
    { NHKF_GETDIR_SELF,      '.', "getdir.self" },
    { NHKF_GETDIR_SELF2,     's', "getdir.self2" },
    { NHKF_GETDIR_HELP,      '?', "getdir.help" },
    { NHKF_COUNT,            'n', "count" },
    { NHKF_GETPOS_SELF,      '@', "getpos.self" },
    { NHKF_GETPOS_PICK,      '.', "getpos.pick" },
    { NHKF_GETPOS_PICK_Q,    ',', "getpos.pick.quick" },
    { NHKF_GETPOS_PICK_O,    ';', "getpos.pick.once" },
    { NHKF_GETPOS_PICK_V,    ':', "getpos.pick.verbose" },
    { NHKF_GETPOS_SHOWVALID, '$', "getpos.valid" },
    { NHKF_GETPOS_AUTODESC,  '#', "getpos.autodescribe" },
    { NHKF_GETPOS_MON_NEXT,  'm', "getpos.mon.next" },
    { NHKF_GETPOS_MON_PREV,  'M', "getpos.mon.prev" },
    { NHKF_GETPOS_OBJ_NEXT,  'o', "getpos.obj.next" },
    { NHKF_GETPOS_OBJ_PREV,  'O', "getpos.obj.prev" },
    { NHKF_GETPOS_DOOR_NEXT, 'd', "getpos.door.next" },
    { NHKF_GETPOS_DOOR_PREV, 'D', "getpos.door.prev" },
    { NHKF_GETPOS_UNEX_NEXT, 'x', "getpos.unexplored.next" },
    { NHKF_GETPOS_UNEX_PREV, 'X', "getpos.unexplored.prev" },
    { NHKF_GETPOS_VALID_NEXT, 'z', "getpos.valid.next" },
    { NHKF_GETPOS_VALID_PREV, 'Z', "getpos.valid.prev" },
    { NHKF_GETPOS_INTERESTING_NEXT, 'a', "getpos.all.next" },
    { NHKF_GETPOS_INTERESTING_PREV, 'A', "getpos.all.prev" },
    { NHKF_GETPOS_HELP,      '?', "getpos.help" },
    { NHKF_GETPOS_LIMITVIEW, '"', "getpos.filter" },
    { NHKF_GETPOS_MOVESKIP,  '*', "getpos.moveskip" },
    { NHKF_GETPOS_MENU,      '!', "getpos.menu" }
};

boolean
bind_specialkey(key, command)
uchar key;
const char *command;
{
    int i;
    for (i = 0; i < SIZE(spkeys_binds); i++) {
        if (!spkeys_binds[i].name || strcmp(command, spkeys_binds[i].name))
            continue;
        Cmd.spkeys[spkeys_binds[i].nhkf] = key;
        return TRUE;
    }
    return FALSE;
}

/* returns a one-byte character from the text (it may massacre the txt
 * buffer) */
char
txt2key(txt)
char *txt;
{
    txt = trimspaces(txt);
    if (!*txt)
        return '\0';

    /* simple character */
    if (!txt[1])
        return txt[0];

    /* a few special entries */
    if (!strcmp(txt, "<enter>"))
        return '\n';
    if (!strcmp(txt, "<space>"))
        return ' ';
    if (!strcmp(txt, "<esc>"))
        return '\033';

    /* control and meta keys */
    switch (*txt) {
    case 'm': /* can be mx, Mx, m-x, M-x */
    case 'M':
        txt++;
        if (*txt == '-' && txt[1])
            txt++;
        if (txt[1])
            return '\0';
        return M(*txt);
    case 'c': /* can be cx, Cx, ^x, c-x, C-x, ^-x */
    case 'C':
    case '^':
        txt++;
        if (*txt == '-' && txt[1])
            txt++;
        if (txt[1])
            return '\0';
        return C(*txt);
    }

    /* ascii codes: must be three-digit decimal */
    if (*txt >= '0' && *txt <= '9') {
        uchar key = 0;
        int i;

        for (i = 0; i < 3; i++) {
            if (txt[i] < '0' || txt[i] > '9')
                return '\0';
            key = 10 * key + txt[i] - '0';
        }
        return key;
    }

    return '\0';
}

/* returns the text for a one-byte encoding;
 * must be shorter than a tab for proper formatting */
char *
key2txt(c, txt)
uchar c;
char *txt; /* sufficiently long buffer */
{
    /* should probably switch to "SPC", "ESC", "RET"
       since nethack's documentation uses ESC for <escape> */
    if (c == ' ')
        Sprintf(txt, "<space>");
    else if (c == '\033')
        Sprintf(txt, "<esc>");
    else if (c == '\n')
        Sprintf(txt, "<enter>");
    else if (c == '\177')
        Sprintf(txt, "<del>"); /* "<delete>" won't fit */
    else
        Strcpy(txt, visctrl((char) c));
    return txt;
}


void
parseautocomplete(autocomplete, condition)
char *autocomplete;
boolean condition;
{
    struct ext_func_tab *efp;
    register char *autoc;

    /* break off first autocomplete from the rest; parse the rest */
    if ((autoc = index(autocomplete, ',')) != 0
        || (autoc = index(autocomplete, ':')) != 0) {
        *autoc++ = '\0';
        parseautocomplete(autoc, condition);
    }

    /* strip leading and trailing white space */
    autocomplete = trimspaces(autocomplete);

    if (!*autocomplete)
        return;

    /* take off negation */
    if (*autocomplete == '!') {
        /* unlike most options, a leading "no" might actually be a part of
         * the extended command.  Thus you have to use ! */
        autocomplete++;
        autocomplete = trimspaces(autocomplete);
        condition = !condition;
    }

    /* find and modify the extended command */
    for (efp = extcmdlist; efp->ef_txt; efp++) {
        if (!strcmp(autocomplete, efp->ef_txt)) {
            if (condition)
                efp->flags |= AUTOCOMPLETE;
            else
                efp->flags &= ~AUTOCOMPLETE;
            return;
        }
    }

    /* not a real extended command */
    raw_printf("Bad autocomplete: invalid extended command '%s'.",
               autocomplete);
    wait_synch();
}

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
    static struct ext_func_tab *back_dir_cmd[8];
    const struct ext_func_tab *cmdtmp;
    boolean flagtemp;
    int c, i, updated = 0;
    static boolean backed_dir_cmd = FALSE;

    if (initial) {
        updated = 1;
        Cmd.num_pad = FALSE;
        Cmd.pcHack_compat = Cmd.phone_layout = Cmd.swap_yz = FALSE;
        for (i = 0; i < SIZE(spkeys_binds); i++)
            Cmd.spkeys[spkeys_binds[i].nhkf] = spkeys_binds[i].key;
        commands_init();
    } else {

        if (backed_dir_cmd) {
            for (i = 0; i < 8; i++) {
                Cmd.commands[(uchar) Cmd.dirchars[i]] = back_dir_cmd[i];
            }
        }

        /* basic num_pad */
        flagtemp = iflags.num_pad;
        if (flagtemp != Cmd.num_pad) {
            Cmd.num_pad = flagtemp;
            ++updated;
        }
        /* swap_yz mode (only applicable for !num_pad); intended for
           QWERTZ keyboard used in Central Europe, particularly Germany */
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
                c = '1' + i;             /* 1,2,3 <-> 7,8,9 */
                cmdtmp = Cmd.commands[c];              /* tmp = [1] */
                Cmd.commands[c] = Cmd.commands[c + 6]; /* [1] = [7] */
                Cmd.commands[c + 6] = cmdtmp;          /* [7] = tmp */
                c = (M('1') & 0xff) + i; /* M-1,M-2,M-3 <-> M-7,M-8,M-9 */
                cmdtmp = Cmd.commands[c];              /* tmp = [M-1] */
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

    if (!initial) {
        for (i = 0; i < 8; i++) {
            back_dir_cmd[i] =
                (struct ext_func_tab *) Cmd.commands[(uchar) Cmd.dirchars[i]];
            Cmd.commands[(uchar) Cmd.dirchars[i]] = (struct ext_func_tab *) 0;
        }
        backed_dir_cmd = TRUE;
        for (i = 0; i < 8; i++)
            (void) bind_key(Cmd.dirchars[i], "nothing");
    }
}

/* non-movement commands which accept 'm' prefix to request menu operation */
STATIC_OVL boolean
accept_menu_prefix(cmd_func)
int NDECL((*cmd_func));
{
    if (cmd_func == dopickup || cmd_func == dotip
        /* eat, #offer, and apply tinning-kit all use floorfood() to pick
           an item on floor or in invent; 'm' skips picking from floor
           (ie, inventory only) rather than request use of menu operation */
        || cmd_func == doeat || cmd_func == dosacrifice || cmd_func == doapply
        /* 'm' for removing saddle from adjacent monster without checking
           for containers at <u.ux,u.uy> */
        || cmd_func == doloot
        /* travel: pop up a menu of interesting targets in view */
        || cmd_func == dotravel
        /* wizard mode ^V */
        || cmd_func == wiz_level_tele
        /* 'm' prefix allowed for some extended commands */
        || cmd_func == doextcmd || cmd_func == doextlist)
        return TRUE;
    return FALSE;
}

int
ch2spkeys(c, start, end)
char c;
int start,end;
{
    int i;

    for (i = start; i <= end; i++)
        if (Cmd.spkeys[i] == c)
            return i;
    return NHKF_ESC;
}

void
rhack(cmd)
register char *cmd;
{
    int spkey;
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
    if (*cmd == Cmd.spkeys[NHKF_ESC]) {
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
    spkey = ch2spkeys(*cmd, NHKF_RUN, NHKF_CLICKLOOK);

    switch (spkey) {
    case NHKF_RUSH:
        if (movecmd(cmd[1])) {
            context.run = 2;
            do_rush = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case NHKF_RUN2:
        if (!Cmd.num_pad)
            break;
        /*FALLTHRU*/
    case NHKF_RUN:
        if (movecmd(lowc(cmd[1]))) {
            context.run = 3;
            do_rush = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case NHKF_FIGHT2:
        if (!Cmd.num_pad)
            break;
        /*FALLTHRU*/
    /* Effects of movement commands and invisible monsters:
     * m: always move onto space (even if 'I' remembered)
     * F: always attack space (even if 'I' not remembered)
     * normal movement: attack if 'I', move otherwise.
     */
    case NHKF_FIGHT:
        if (movecmd(cmd[1])) {
            context.forcefight = 1;
            do_walk = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case NHKF_NOPICKUP:
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
    case NHKF_RUN_NOPICKUP:
        if (movecmd(lowc(cmd[1]))) {
            context.run = 1;
            context.nopick = 1;
            do_rush = TRUE;
        } else
            prefix_seen = TRUE;
        break;
    case NHKF_DOINV:
        if (!Cmd.num_pad)
            break;
        (void) ddoinv(); /* a convenience borrowed from the PC */
        context.move = FALSE;
        multi = 0;
        return;
    case NHKF_CLICKLOOK:
        if (iflags.clicklook) {
            context.move = FALSE;
            do_look(2, &clicklook_cc);
        }
        return;
    case NHKF_TRAVEL:
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
    if (prefix_seen && cmd[0] == Cmd.spkeys[NHKF_REQMENU]) {
        /* (for func_tab cast, see below) */
        const struct ext_func_tab *ft = Cmd.commands[cmd[1] & 0xff];
        int NDECL((*func)) = ft ? ((struct ext_func_tab *) ft)->ef_funct : 0;

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
/*JP
        You_cant("get there from here...");
*/
        You_cant("ここからそこへは行けません．．．");
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
    } else if (prefix_seen && cmd[1] == Cmd.spkeys[NHKF_ESC]) {
        /* <prefix><escape> */
        /* don't report "unknown command" for change of heart... */
        bad_command = FALSE;
    } else if (*cmd == ' ' && !flags.rest_on_space) {
        bad_command = TRUE; /* skip cmdlist[] loop */

    /* handle all other commands */
    } else {
        register const struct ext_func_tab *tlist;
        int res, NDECL((*func));

        /* current - use *cmd to directly index cmdlist array */
        if ((tlist = Cmd.commands[*cmd & 0xff]) != 0) {
            if (!wizard && (tlist->flags & WIZMODECMD)) {
/*JP
                You_cant("do that!");
*/
                pline("それはできません！");
                res = 0;
            } else if (u.uburied && !(tlist->flags & IFBURIED)) {
/*JP
                You_cant("do that while you are buried!");
*/
                You("埋まっている時にそんなことはできない！");
                res = 0;
            } else {
                /* we discard 'const' because some compilers seem to have
                   trouble with the pointer passed to set_occupation() */
                func = ((struct ext_func_tab *) tlist)->ef_funct;
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
        char expcmd[20]; /* we expect 'cmd' to point to 1 or 2 chars */
        char c, c1 = cmd[1];

        expcmd[0] = '\0';
        while ((c = *cmd++) != '\0')
            Strcat(expcmd, visctrl(c)); /* add 1..4 chars plus terminator */

/*JP
        if (!prefix_seen || !help_dir(c1, spkey, "Invalid direction key!"))
*/
        if (!prefix_seen || !help_dir(c1, spkey, "無効な方向指定です！"))
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
    return (boolean) (c == Cmd.spkeys[NHKF_REDRAW]
                      || (Cmd.num_pad && c == Cmd.spkeys[NHKF_REDRAW2]));
}

boolean
prefix_cmd(c)
char c;
{
    return (c == Cmd.spkeys[NHKF_RUSH]
            || c == Cmd.spkeys[NHKF_RUN]
            || c == Cmd.spkeys[NHKF_NOPICKUP]
            || c == Cmd.spkeys[NHKF_RUN_NOPICKUP]
            || c == Cmd.spkeys[NHKF_FIGHT]
            || (Cmd.num_pad && (c == Cmd.spkeys[NHKF_RUN2]
                                || c == Cmd.spkeys[NHKF_FIGHT2])));
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

    if (dirsym == Cmd.spkeys[NHKF_GETDIR_SELF]
        || dirsym == Cmd.spkeys[NHKF_GETDIR_SELF2]) {
        u.dx = u.dy = u.dz = 0;
    } else if (!(is_mov = movecmd(dirsym)) && !u.dz) {
        boolean did_help = FALSE, help_requested;

        if (!index(quitchars, dirsym)) {
            help_requested = (dirsym == Cmd.spkeys[NHKF_GETDIR_HELP]);
            if (help_requested || iflags.cmdassist) {
                did_help = help_dir((s && *s == '^') ? dirsym : '\0',
                                    NHKF_ESC,
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

STATIC_OVL void
show_direction_keys(win, centerchar, nodiag)
winid win; /* should specify a window which is using a fixed-width font... */
char centerchar; /* '.' or '@' or ' ' */
boolean nodiag;
{
    char buf[BUFSZ];

    if (!centerchar)
        centerchar = ' ';

    if (nodiag) {
        Sprintf(buf, "             %c   ", Cmd.move_N);
        putstr(win, 0, buf);
        putstr(win, 0, "             |   ");
        Sprintf(buf, "          %c- %c -%c",
                Cmd.move_W, centerchar, Cmd.move_E);
        putstr(win, 0, buf);
        putstr(win, 0, "             |   ");
        Sprintf(buf, "             %c   ", Cmd.move_S);
        putstr(win, 0, buf);
    } else {
        Sprintf(buf, "          %c  %c  %c",
                Cmd.move_NW, Cmd.move_N, Cmd.move_NE);
        putstr(win, 0, buf);
        putstr(win, 0, "           \\ | / ");
        Sprintf(buf, "          %c- %c -%c",
                Cmd.move_W, centerchar, Cmd.move_E);
        putstr(win, 0, buf);
        putstr(win, 0, "           / | \\ ");
        Sprintf(buf, "          %c  %c  %c",
                Cmd.move_SW, Cmd.move_S, Cmd.move_SE);
        putstr(win, 0, buf);
    };
}

/* explain choices if player has asked for getdir() help or has given
   an invalid direction after a prefix key ('F', 'g', 'm', &c), which
   might be bogus but could be up, down, or self when not applicable */
STATIC_OVL boolean
help_dir(sym, spkey, msg)
char sym;
int spkey; /* NHKF_ code for prefix key, if one was used, or for ESC */
const char *msg;
{
    static const char wiz_only_list[] = "EFGIVW";
    char ctrl;
    winid win;
    char buf[BUFSZ], buf2[BUFSZ], *explain;
    const char *dothat, *how;
    boolean prefixhandling, viawindow;

    /* NHKF_ESC indicates that player asked for help at getdir prompt */
    viawindow = (spkey == NHKF_ESC || iflags.cmdassist);
    prefixhandling = (spkey != NHKF_ESC);
    /*
     * Handling for prefix keys that don't want special directions.
     * Delivered via pline if 'cmdassist' is off, or instead of the
     * general message if it's on.
     */
    dothat = "do that";
    how = " at"; /* for "<action> at yourself"; not used for up/down */
    switch (spkey) {
    case NHKF_NOPICKUP:
        dothat = "move";
        break;
    case NHKF_RUSH:
        dothat = "rush";
        break;
    case NHKF_RUN2:
        if (!Cmd.num_pad)
            break;
        /*FALLTHRU*/
    case NHKF_RUN:
    case NHKF_RUN_NOPICKUP:
        dothat = "run";
        break;
    case NHKF_FIGHT2:
        if (!Cmd.num_pad)
            break;
        /*FALLTHRU*/
    case NHKF_FIGHT:
        dothat = "fight";
        how = ""; /* avoid "fight at yourself" */
        break;
    default:
        prefixhandling = FALSE;
        break;
    }

    buf[0] = '\0';
    /* for movement prefix followed by '.' or (numpad && 's') to mean 'self';
       note: '-' for hands (inventory form of 'self') is not handled here */
    if (prefixhandling
        && (sym == Cmd.spkeys[NHKF_GETDIR_SELF]
            || (Cmd.num_pad && sym == Cmd.spkeys[NHKF_GETDIR_SELF2]))) {
        Sprintf(buf, "You can't %s%s yourself.", dothat, how);
    /* for movement prefix followed by up or down */
    } else if (prefixhandling && (sym == '<' || sym == '>')) {
        Sprintf(buf, "You can't %s %s.", dothat,
                /* was "upwards" and "downwards", but they're considered
                   to be variants of canonical "upward" and "downward" */
                (sym == '<') ? "upward" : "downward");
    }

    /* if '!cmdassist', display via pline() and we're done (note: asking
       for help at getdir() prompt forces cmdassist for this operation) */
    if (!viawindow) {
        if (prefixhandling) {
            if (!*buf)
                Sprintf(buf, "Invalid direction for '%s' prefix.",
                        visctrl(Cmd.spkeys[spkey]));
            pline("%s", buf);
            return TRUE;
        }
        /* when 'cmdassist' is off and caller doesn't insist, do nothing */
        return FALSE;
    }

    win = create_nhwindow(NHW_TEXT);
    if (!win)
        return FALSE;

    if (*buf) {
        /* show bad-prefix message instead of general invalid-direction one */
        putstr(win, 0, buf);
        putstr(win, 0, "");
    } else if (msg) {
        Sprintf(buf, "cmdassist: %s", msg);
        putstr(win, 0, buf);
        putstr(win, 0, "");
    }

    if (!prefixhandling && (letter(sym) || sym == '[')) {
        /* '[': old 'cmdhelp' showed ESC as ^[ */
        sym = highc(sym); /* @A-Z[ (note: letter() accepts '@') */
        ctrl = (sym - 'A') + 1; /* 0-27 (note: 28-31 aren't applicable) */
        if ((explain = dowhatdoes_core(ctrl, buf2)) != 0
            && (!index(wiz_only_list, sym) || wizard)) {
            Sprintf(buf, "Are you trying to use ^%c%s?", sym,
                    index(wiz_only_list, sym) ? ""
                        : " as specified in the Guidebook");
            putstr(win, 0, buf);
            putstr(win, 0, "");
            putstr(win, 0, explain);
            putstr(win, 0, "");
            putstr(win, 0,
                  "To use that command, hold down the <Ctrl> key as a shift");
            Sprintf(buf, "and press the <%c> key.", sym);
            putstr(win, 0, buf);
            putstr(win, 0, "");
        }
    }

    Sprintf(buf, "Valid direction keys%s%s%s are:",
            prefixhandling ? " to " : "", prefixhandling ? dothat : "",
            NODIAG(u.umonnum) ? " in your current form" : "");
    putstr(win, 0, buf);
    show_direction_keys(win, !prefixhandling ? '.' : ' ', NODIAG(u.umonnum));

    if (!prefixhandling || spkey == NHKF_NOPICKUP) {
        /* NOPICKUP: unlike the other prefix keys, 'm' allows up/down for
           stair traversal; we won't get here when "m<" or "m>" has been
           given but we include up and down for 'm'+invalid_direction;
           self is excluded as a viable direction for every prefix */
        putstr(win, 0, "");
        putstr(win, 0, "          <  up");
        putstr(win, 0, "          >  down");
        if (!prefixhandling) {
            int selfi = Cmd.num_pad ? NHKF_GETDIR_SELF2 : NHKF_GETDIR_SELF;

            Sprintf(buf,   "       %4s  direct at yourself",
                    visctrl(Cmd.spkeys[selfi]));
            putstr(win, 0, buf);
        }
    }

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

/* #herecmdmenu command */
STATIC_PTR int
doherecmdmenu(VOID_ARGS)
{
    char ch = here_cmd_menu(TRUE);

    return ch ? 1 : 0;
}

/* #therecmdmenu command, a way to test there_cmd_menu without mouse */
STATIC_PTR int
dotherecmdmenu(VOID_ARGS)
{
    char ch;

    if (!getdir((const char *) 0) || !isok(u.ux + u.dx, u.uy + u.dy))
        return 0;

    if (u.dx || u.dy)
        ch = there_cmd_menu(TRUE, u.ux + u.dx, u.uy + u.dy);
    else
        ch = here_cmd_menu(TRUE);

    return ch ? 1 : 0;
}

STATIC_OVL void
add_herecmd_menuitem(win, func, text)
winid win;
int NDECL((*func));
const char *text;
{
    char ch;
    anything any;

    if ((ch = cmd_from_func(func)) != '\0') {
        any = zeroany;
        any.a_nfunc = func;
        add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, text, MENU_UNSELECTED);
    }
}

STATIC_OVL char
there_cmd_menu(doit, x, y)
boolean doit;
int x, y;
{
    winid win;
    char ch;
    char buf[BUFSZ];
    schar typ = levl[x][y].typ;
    int npick, K = 0;
    menu_item *picks = (menu_item *) 0;
    struct trap *ttmp;
    struct monst *mtmp;

    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    if (IS_DOOR(typ)) {
        boolean key_or_pick, card;
        int dm = levl[x][y].doormask;

        if ((dm & (D_CLOSED | D_LOCKED))) {
            add_herecmd_menuitem(win, doopen, "Open the door"), ++K;
            /* unfortunately there's no lknown flag for doors to
               remember the locked/unlocked state */
            key_or_pick = (carrying(SKELETON_KEY) || carrying(LOCK_PICK));
            card = (carrying(CREDIT_CARD) != 0);
            if (key_or_pick || card) {
                Sprintf(buf, "%sunlock the door",
                        key_or_pick ? "lock or " : "");
                add_herecmd_menuitem(win, doapply, upstart(buf)), ++K;
            }
            /* unfortunately there's no tknown flag for doors (or chests)
               to remember whether a trap had been found */
            add_herecmd_menuitem(win, dountrap,
                                 "Search the door for a trap"), ++K;
            /* [what about #force?] */
            add_herecmd_menuitem(win, dokick, "Kick the door"), ++K;
        } else if ((dm & D_ISOPEN)) {
            add_herecmd_menuitem(win, doclose, "Close the door"), ++K;
        }
    }

    if (typ <= SCORR)
        add_herecmd_menuitem(win, dosearch, "Search for secret doors"), ++K;

    if ((ttmp = t_at(x, y)) != 0 && ttmp->tseen) {
        add_herecmd_menuitem(win, doidtrap, "Examine trap"), ++K;
        if (ttmp->ttyp != VIBRATING_SQUARE)
            add_herecmd_menuitem(win, dountrap, "Attempt to disarm trap"), ++K;
    }

    mtmp = m_at(x, y);
    if (mtmp && !canspotmon(mtmp))
        mtmp = 0;
    if (mtmp && which_armor(mtmp, W_SADDLE)) {
        char *mnam = x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                              SUPPRESS_SADDLE, FALSE);

        if (!u.usteed) {
            Sprintf(buf, "Ride %s", mnam);
            add_herecmd_menuitem(win, doride, buf), ++K;
        }
        Sprintf(buf, "Remove saddle from %s", mnam);
        add_herecmd_menuitem(win, doloot, buf), ++K;
    }
    if (mtmp && can_saddle(mtmp) && !which_armor(mtmp, W_SADDLE)
        && carrying(SADDLE)) {
        Sprintf(buf, "Put saddle on %s", mon_nam(mtmp)), ++K;
        add_herecmd_menuitem(win, doapply, buf);
    }
#if 0
    if (mtmp || glyph_is_invisible(glyph_at(x, y))) {
        /* "Attack %s", mtmp ? mon_nam(mtmp) : "unseen creature" */
    } else {
        /* "Move %s", direction */
    }
#endif

    if (K) {
        end_menu(win, "What do you want to do?");
        npick = select_menu(win, PICK_ONE, &picks);
    } else {
        pline("No applicable actions.");
        npick = 0;
    }
    destroy_nhwindow(win);
    ch = '\0';
    if (npick > 0) {
        int NDECL((*func)) = picks->item.a_nfunc;
        free((genericptr_t) picks);

        if (doit) {
            int ret = (*func)();

            ch = (char) ret;
        } else {
            ch = cmd_from_func(func);
        }
    }
    return ch;
}

STATIC_OVL char
here_cmd_menu(doit)
boolean doit;
{
    winid win;
    char ch;
    char buf[BUFSZ];
    schar typ = levl[u.ux][u.uy].typ;
    int npick;
    menu_item *picks = (menu_item *) 0;

    win = create_nhwindow(NHW_MENU);
    start_menu(win);

    if (IS_FOUNTAIN(typ) || IS_SINK(typ)) {
        Sprintf(buf, "Drink from the %s",
                defsyms[IS_FOUNTAIN(typ) ? S_fountain : S_sink].explanation);
        add_herecmd_menuitem(win, dodrink, buf);
    }
    if (IS_FOUNTAIN(typ))
        add_herecmd_menuitem(win, dodip,
                             "Dip something into the fountain");
    if (IS_THRONE(typ))
        add_herecmd_menuitem(win, dosit,
                             "Sit on the throne");

    if ((u.ux == xupstair && u.uy == yupstair)
        || (u.ux == sstairs.sx && u.uy == sstairs.sy && sstairs.up)
        || (u.ux == xupladder && u.uy == yupladder)) {
        Sprintf(buf, "Go up the %s",
                (u.ux == xupladder && u.uy == yupladder)
                ? "ladder" : "stairs");
        add_herecmd_menuitem(win, doup, buf);
    }
    if ((u.ux == xdnstair && u.uy == ydnstair)
        || (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)
        || (u.ux == xdnladder && u.uy == ydnladder)) {
        Sprintf(buf, "Go down the %s",
                (u.ux == xupladder && u.uy == yupladder)
                ? "ladder" : "stairs");
        add_herecmd_menuitem(win, dodown, buf);
    }
    if (u.usteed) { /* another movement choice */
        Sprintf(buf, "Dismount %s",
                x_monnam(u.usteed, ARTICLE_THE, (char *) 0,
                         SUPPRESS_SADDLE, FALSE));
        add_herecmd_menuitem(win, doride, buf);
    }

#if 0
    if (Upolyd) { /* before objects */
        Sprintf(buf, "Use %s special ability",
                s_suffix(mons[u.umonnum].mname));
        add_herecmd_menuitem(win, domonability, buf);
    }
#endif

    if (OBJ_AT(u.ux, u.uy)) {
        struct obj *otmp = level.objects[u.ux][u.uy];

        Sprintf(buf, "Pick up %s", otmp->nexthere ? "items" : doname(otmp));
        add_herecmd_menuitem(win, dopickup, buf);

        if (Is_container(otmp)) {
            Sprintf(buf, "Loot %s", doname(otmp));
            add_herecmd_menuitem(win, doloot, buf);
        }
        if (otmp->oclass == FOOD_CLASS) {
            Sprintf(buf, "Eat %s", doname(otmp));
            add_herecmd_menuitem(win, doeat, buf);
        }
    }

    if (invent)
        add_herecmd_menuitem(win, dodrop, "Drop items");

    add_herecmd_menuitem(win, donull, "Rest one turn");
    add_herecmd_menuitem(win, dosearch, "Search around you");
    add_herecmd_menuitem(win, dolook, "Look at what is here");

    end_menu(win, "What do you want to do?");
    npick = select_menu(win, PICK_ONE, &picks);
    destroy_nhwindow(win);
    ch = '\0';
    if (npick > 0) {
        int NDECL((*func)) = picks->item.a_nfunc;
        free((genericptr_t) picks);

        if (doit) {
            int ret = (*func)();

            ch = (char) ret;
        } else {
            ch = cmd_from_func(func);
        }
    }
    return ch;
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
        cmd[0] = Cmd.spkeys[NHKF_CLICKLOOK];
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
            cmd[0] = Cmd.spkeys[NHKF_TRAVEL];
            return cmd;
        }

        if (x == 0 && y == 0) {
            if (iflags.herecmd_menu) {
                cmd[0] = here_cmd_menu(FALSE);
                return cmd;
            }

            /* here */
            if (IS_FOUNTAIN(levl[u.ux][u.uy].typ)
                || IS_SINK(levl[u.ux][u.uy].typ)) {
                cmd[0] = cmd_from_func(mod == CLICK_1 ? dodrink : dodip);
                return cmd;
            } else if (IS_THRONE(levl[u.ux][u.uy].typ)) {
                cmd[0] = cmd_from_func(dosit);
                return cmd;
            } else if ((u.ux == xupstair && u.uy == yupstair)
                       || (u.ux == sstairs.sx && u.uy == sstairs.sy
                           && sstairs.up)
                       || (u.ux == xupladder && u.uy == yupladder)) {
                cmd[0] = cmd_from_func(doup);
                return cmd;
            } else if ((u.ux == xdnstair && u.uy == ydnstair)
                       || (u.ux == sstairs.sx && u.uy == sstairs.sy
                           && !sstairs.up)
                       || (u.ux == xdnladder && u.uy == ydnladder)) {
                cmd[0] = cmd_from_func(dodown);
                return cmd;
            } else if (OBJ_AT(u.ux, u.uy)) {
                cmd[0] = cmd_from_func(Is_container(level.objects[u.ux][u.uy])
                                       ? doloot : dopickup);
                return cmd;
            } else {
                cmd[0] = cmd_from_func(donull); /* just rest */
                return cmd;
            }
        }

        /* directional commands */

        dir = xytod(x, y);

        if (!m_at(u.ux + x, u.uy + y)
            && !test_move(u.ux, u.uy, x, y, TEST_MOVE)) {
            cmd[1] = Cmd.dirchars[dir];
            cmd[2] = '\0';
            if (iflags.herecmd_menu) {
                cmd[0] = there_cmd_menu(FALSE, u.ux + x, u.uy + y);
                if (cmd[0] == '\0')
                    cmd[1] = '\0';
                return cmd;
            }

            if (IS_DOOR(levl[u.ux + x][u.uy + y].typ)) {
                /* slight assistance to the player: choose kick/open for them
                 */
                if (levl[u.ux + x][u.uy + y].doormask & D_LOCKED) {
                    cmd[0] = cmd_from_func(dokick);
                    return cmd;
                }
                if (levl[u.ux + x][u.uy + y].doormask & D_CLOSED) {
                    cmd[0] = cmd_from_func(doopen);
                    return cmd;
                }
            }
            if (levl[u.ux + x][u.uy + y].typ <= SCORR) {
                cmd[0] = cmd_from_func(dosearch);
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

        if (x == 0 && y == 0) {
            /* map click on player to "rest" command */
            cmd[0] = cmd_from_func(donull);
            return cmd;
        }
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

char
get_count(allowchars, inkey, maxcount, count, historical)
char *allowchars;
char inkey;
long maxcount;
long *count;
boolean historical; /* whether to include in message history: True => yes */
{
    char qbuf[QBUFSZ];
    int key;
    long cnt = 0L;
    boolean backspaced = FALSE;
    /* this should be done in port code so that we have erase_char
       and kill_char available; we can at least fake erase_char */
#define STANDBY_erase_char '\177'

    for (;;) {
        if (inkey) {
            key = inkey;
            inkey = '\0';
        } else
            key = readchar();

        if (digit(key)) {
            cnt = 10L * cnt + (long) (key - '0');
            if (cnt < 0)
                cnt = 0;
            else if (maxcount > 0 && cnt > maxcount)
                cnt = maxcount;
        } else if (cnt && (key == '\b' || key == STANDBY_erase_char)) {
            cnt = cnt / 10;
            backspaced = TRUE;
        } else if (key == Cmd.spkeys[NHKF_ESC]) {
            break;
        } else if (!allowchars || index(allowchars, key)) {
            *count = cnt;
            break;
        }

        if (cnt > 9 || backspaced) {
            clear_nhwindow(WIN_MESSAGE);
            if (backspaced && !cnt) {
/*JP
                Sprintf(qbuf, "Count: ");
*/
                Sprintf(qbuf, "数: ");
            } else {
/*JP
                Sprintf(qbuf, "Count: %ld", cnt);
*/
                Sprintf(qbuf, "数: %ld", cnt);
                backspaced = FALSE;
            }
            /* bypassing pline() keeps intermediate prompt out of
               DUMPLOG message history */
            putstr(WIN_MESSAGE, 0, qbuf);
            mark_synch();
        }
    }

    if (historical) {
/*JP
        Sprintf(qbuf, "Count: %ld ", *count);
*/
        Sprintf(qbuf, "数: %ld ", *count);
        (void) key2txt((uchar) key, eos(qbuf));
        putmsghistory(qbuf, FALSE);
    }

    return key;
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

    iflags.in_parse = TRUE;
    multi = 0;
    context.move = 1;
    flush_screen(1); /* Flush screen buffer. Put the cursor on the hero. */

#ifdef ALTMETA
    alt_esc = iflags.altmeta; /* readchar() hack */
#endif
    if (!Cmd.num_pad || (foo = readchar()) == Cmd.spkeys[NHKF_COUNT]) {
        long tmpmulti = multi;

        foo = get_count((char *) 0, '\0', LARGEST_INT, &tmpmulti, FALSE);
        last_multi = multi = tmpmulti;
    }
#ifdef ALTMETA
    alt_esc = FALSE; /* readchar() reset */
#endif

    if (foo == Cmd.spkeys[NHKF_ESC]) { /* esc cancels count (TH) */
        clear_nhwindow(WIN_MESSAGE);
        multi = last_multi = 0;
    } else if (foo == Cmd.spkeys[NHKF_DOAGAIN] || in_doagain) {
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
            foo = Cmd.spkeys[NHKF_RUSH];
            break;
        case M('5'):
            foo = Cmd.spkeys[NHKF_RUN];
            break;
        case M('0'):
            foo = Cmd.spkeys[NHKF_DOINV];
            break;
        default:
            break; /* as is */
        }
    }

    in_line[0] = foo;
    in_line[1] = '\0';
    if (prefix_cmd(foo)) {
        foo = readchar();
        savech((char) foo);
        in_line[1] = foo;
        in_line[2] = 0;
    }
    clear_nhwindow(WIN_MESSAGE);
    if (prezero)
        in_line[0] = Cmd.spkeys[NHKF_ESC];

    iflags.in_parse = FALSE;
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
    nh_terminate(EXIT_SUCCESS);
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
#endif /*ALTMETA*/
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
    iflags.getloc_travelmode = TRUE;
    if (iflags.menu_requested) {
        int gf = iflags.getloc_filter;
        iflags.getloc_filter = GFILTER_VIEW;
        if (!getpos_menu(&cc, GLOC_INTERESTING)) {
            iflags.getloc_filter = gf;
            iflags.getloc_travelmode = FALSE;
            return 0;
        }
        iflags.getloc_filter = gf;
    } else {
/*JP
    pline("Where do you want to travel to?");
*/
    pline("どこに移動する？");
/*JP
    if (getpos(&cc, TRUE, "the desired destination") < 0) {
*/
    if (getpos(&cc, TRUE, "移動先") < 0) {
            /* user pressed ESC */
            iflags.getloc_travelmode = FALSE;
            return 0;
        }
    }
    iflags.getloc_travelmode = FALSE;
    iflags.travelcc.x = u.tx = cc.x;
    iflags.travelcc.y = u.ty = cc.y;
    cmd[0] = Cmd.spkeys[NHKF_TRAVEL];
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
        { (char *) 0, (void NDECL((*))) 0 } /* array terminator */
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
    char res, qbuf[QBUFSZ];
#ifdef DUMPLOG
    extern unsigned saved_pline_index; /* pline.c */
    unsigned idx = saved_pline_index;
    /* buffer to hold query+space+formatted_single_char_response */
    char dumplog_buf[QBUFSZ + 1 + 15]; /* [QBUFSZ+1+7] should suffice */
#endif

    iflags.last_msg = PLNMSG_UNKNOWN; /* most recent pline is clobbered */

    /* maximum acceptable length is QBUFSZ-1 */
    if (strlen(query) >= QBUFSZ) {
        /* caller shouldn't have passed anything this long */
        paniclog("Query truncated: ", query);
        (void) strncpy(qbuf, query, QBUFSZ - 1 - 3);
        Strcpy(&qbuf[QBUFSZ - 1 - 3], "...");
        query = qbuf;
    }
    res = (*windowprocs.win_yn_function)(query, resp, def);
#ifdef DUMPLOG
    if (idx == saved_pline_index) {
        /* when idx is still the same as saved_pline_index, the interface
           didn't put the prompt into saved_plines[]; we put a simplified
           version in there now (without response choices or default) */
        Sprintf(dumplog_buf, "%s ", query);
        (void) key2txt((uchar) res, eos(dumplog_buf));
        dumplogmsg(dumplog_buf);
    }
#endif
    return res;
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
        char qbuf[QBUFSZ], ans[BUFSZ] = DUMMY;
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
/*JP
        Norep("Suspend command not available.");
*/
        Norep("中断コマンドは利用できません．");
    return 0;
}

/*cmd.c*/
