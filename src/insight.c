/* NetHack 5.0	insight.c	$NHDT-Date: 1777004419 2026/04/23 20:20:19 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.134 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Enlightenment and Conduct+Achievements and Vanquished+Extinct+Geno'd
 * and stethoscope/probing feedback.
 *
 * Most code used to reside in cmd.c, presumably because ^X was originally
 * a wizard mode command and the majority of those are in that file.
 * Some came from end.c where it is used during end of game disclosure.
 * And some came from priest.c that had once been in pline.c.
 */

#include "hack.h"

staticfn void enlght_out(const char *);
staticfn void enlght_line(const char *, const char *, const char *,
                          const char *);
staticfn char *enlght_combatinc(const char *, int, int, char *);
staticfn void enlght_halfdmg(int, int);
staticfn boolean walking_on_water(void);
staticfn boolean cause_known(int);
staticfn char *attrval(int, int, char *);
staticfn char *fmt_elapsed_time(char *, int);
staticfn char *N_times(long, char *) NONNULL NONNULLARG2;
staticfn void background_enlightenment(int, int);
staticfn void basics_enlightenment(int, int);
staticfn void characteristics_enlightenment(int, int);
staticfn void one_characteristic(int, int, int);
staticfn void status_enlightenment(int, int);
staticfn void weapon_insight(int);
staticfn void attributes_enlightenment(int, int);
staticfn void show_achievements(int);
staticfn int QSORTCALLBACK vanqsort_cmp(const genericptr, const genericptr);
staticfn int num_extinct(void);
staticfn int num_gone(int, int *);
staticfn char *size_str(int);
staticfn void item_resistance_message(int, const char *, int);

extern const char *const hu_stat[];  /* hunger status from eat.c */
extern const char *const enc_stat[]; /* encumbrance status from botl.c */

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

/* for livelogging: */
struct ll_achieve_msg {
    long llflag;
    const char *msg;
};
/* ordered per 'enum achievements' in you.h */
/* take care to keep them in sync! */
static struct ll_achieve_msg achieve_msg [] = {
    { 0, "" }, /* actual achievements are numbered from 1 */
    { LL_ACHIEVE, "acquired the Bell of Opening" },
    { LL_ACHIEVE, "entered Gehennom" },
    { LL_ACHIEVE, "acquired the Candelabrum of Invocation" },
    { LL_ACHIEVE, "acquired the Book of the Dead" },
    { LL_ACHIEVE, "performed the invocation" },
    { LL_ACHIEVE, "acquired The Amulet of Yendor" },
    { LL_ACHIEVE, "entered the Elemental Planes" },
    { LL_ACHIEVE, "entered the Astral Plane" },
    { LL_ACHIEVE, "ascended" },
    /* if the type of item isn't discovered yet, disclosing the event
       via #chronicle would be a spoiler (particularly for gray stone);
       the ID'd name for the type of item will be appended to the next
       two messages, for display via livelog and/or dumplog */
    { LL_ACHIEVE | LL_SPOILER, "acquired the Mines' End" }, /* " luckstone" */
    { LL_ACHIEVE | LL_SPOILER, "acquired the Sokoban" }, /* " <item>" */
    { LL_ACHIEVE | LL_UMONST, "killed Medusa" },
     /* these two are not logged */
    { 0, "hero was always blond, no, blind" },
    { 0, "hero never wore armor" },
     /* */
    { LL_MINORAC | LL_DUMP, "entered the Gnomish Mines" },
    { LL_ACHIEVE, "reached Mine Town" }, /* probably minor, but dnh logs it */
    { LL_MINORAC, "entered a shop" },
    { LL_MINORAC, "entered a temple" },
    { LL_ACHIEVE, "consulted the Oracle" }, /* minor, but rare enough */
    { LL_MINORAC | LL_DUMP, "read a Discworld novel" }, /* even more so */
    { LL_ACHIEVE, "entered Sokoban" }, /* keep as major for turn comparison
                                        * with completed sokoban */
    { LL_ACHIEVE, "entered the Bigroom" },
    /* The following 8 are for advancing through the ranks
       and messages differ by role so are created on the fly;
       rank 0 (Xp 1 and 2) isn't an achievement */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 3 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 6 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 10 */
    { LL_ACHIEVE, "" }, /* Xp 14, so able to attempt the quest */
    { LL_ACHIEVE, "" }, /* Xp 18 */
    { LL_ACHIEVE, "" }, /* Xp 22 */
    { LL_ACHIEVE, "" }, /* Xp 26 */
    { LL_ACHIEVE, "" }, /* Xp 30 */
    { LL_MINORAC, "learned castle drawbridge's tune" }, /* achievement #31 */
    { 0, "" } /* keep this one at the end */
};

/* macros to simplify output of enlightenment messages; also used by
   conduct and achievements */
#if 0 /*JP*/
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line((prefix), final ? (past) : (present), (suffix), (ps))
#else
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line((prefix), (ps), (suffix), final ? (past) : (present))
#endif
#define you_are(attr, ps) enl_msg(You_, are, were, (attr), (ps))
#define you_have(attr, ps) enl_msg(You_, have, had, (attr), (ps))
#define you_can(attr, ps) enl_msg(You_, can, could, (attr), (ps))
#if 0 /*JP*/
#define you_have_been(goodthing) \
    enl_msg(You_, have_been, were, (goodthing), "")
#else
#define you_have_been(goodthing) \
    enl_msg(You_, are, were, (goodthing), "")
#endif
#if 0 /*JP*/
#define you_have_never(badthing) \
    enl_msg(You_, have_never, never, (badthing), "")
#else
#define you_have_never(badthing) \
    enl_msg((badthing), "ていない", "なかった", "", "")
#endif
#if 0 /*JP*/
#define you_have_X(something) \
    enl_msg(You_, have, (const char *) "", (something), "")
#else
#define you_have_X(something) \
    enl_msg((something), "ている", "た", "", "")
#endif
#if 1 /*JP*/
#define you_are_ing(goodthing, ps) enl_msg(You_, iru, ita, goodthing, ps)
#endif

staticfn void
enlght_out(const char *buf)
{
    if (ge.en_via_menu) {
        add_menu_str(ge.en_win, buf);
    } else
        putstr(ge.en_win, 0, buf);
}

staticfn void
enlght_line(
    const char *start,
    const char *middle,
    const char *end,
    const char *ps)
{
#ifndef NO_ENLGHT_CONTRACTIONS
    static const struct contrctn {
        const char *twowords, *contrctn;
    } contra[] = {
        { " are not ", " aren't " },
        { " were not ", " weren't " },
        { " have not ", " haven't " },
        { " had not ", " hadn't " },
        { " can not ", " can't " },
        { " could not ", " couldn't " },
    };
    int i;
#endif
    char buf[BUFSZ];

/*JP
    Sprintf(buf, " %s%s%s%s.", start, middle, end, ps);
*/
    Sprintf(buf, "%s%s%s%s．", start, middle, end, ps);
#ifndef NO_ENLGHT_CONTRACTIONS
    if (strstri(buf, " not ")) { /* TODO: switch to libc strstr() */
        for (i = 0; i < SIZE(contra); ++i)
            (void) strsubst(buf, contra[i].twowords, contra[i].contrctn);
    }
#endif
    enlght_out(buf);
}

/* format increased chance to hit or damage or defense (Protection) */
staticfn char *
enlght_combatinc(
    const char *inctyp, /* "to hit" or "damage" or "defense" */
    int incamt,         /* amount of increment (negative if decrement) */
    int final,          /* ENL_{GAMEINPROGRESS,GAMEOVERALIVE,GAMEOVERDEAD} */
    char *outbuf)
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
#if 0 /*JP:T*/
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
    bonus = (incamt >= 0) ? "ボーナス" : "ペナルティ";
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
staticfn void
enlght_halfdmg(int category, int final)
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
#if 0 /*JP:T*/
    Sprintf(buf, " %s %s damage", (final || wizard) ? "half" : "reduced",
            category_name);
    enl_msg(You_, "take", "took", buf, from_what(category));
#else
    Sprintf(buf, " %sダメージを%s",
            category_name,
            (final || wizard) ? "半減" : "減少");
    enl_msg(You_, "している", "していた", buf, from_what(category));
#endif
}

/* is hero actively using water walking capability on water (or lava)? */
staticfn boolean
walking_on_water(void)
{
    if (u.uinwater || Levitation || Flying)
        return FALSE;
    return (boolean) (Wwalking && is_pool_or_lava(u.ux, u.uy));
}

/* describe u.utraptype; used by status_enlightenment() and self_lookat() */
char *
trap_predicament(char *outbuf, int final, boolean wizxtra)
{
    struct trap *t;

    /* caller has verified u.utrap */
    *outbuf = '\0';
    switch (u.utraptype) {
    case TT_BURIEDBALL:
/*JP
        Strcpy(outbuf, "tethered to something buried");
*/
        Strcpy(outbuf, "何か埋まっているものにつながれて");
        break;
    case TT_LAVA:
/*JP
        Sprintf(outbuf, "sinking into %s", final ? "lava" : hliquid("lava"));
*/
        Sprintf(outbuf, "%sに沈んで", final ? "溶岩" : hliquid("溶岩"));
        break;
    case TT_INFLOOR:
/*JP
        Sprintf(outbuf, "stuck in %s", the(surface(u.ux, u.uy)));
*/
        Sprintf(outbuf, "%sに埋まって", surface(u.ux, u.uy));
        break;
    default: /* TT_BEARTRAP, TT_PIT, or TT_WEB */
#if 0 /*JP*/
        Strcpy(outbuf, "trapped");
        if ((t = t_at(u.ux, u.uy)) != 0) /* should never be null */
            Sprintf(eos(outbuf), " in %s", an(trapname(t->ttyp, FALSE)));
#else
        outbuf[0] = '\0';
        if ((t = t_at(u.ux, u.uy)) != 0) /* should never be null */
            Sprintf(outbuf, "%sに", trapname(t->ttyp, FALSE));
        Strcat(outbuf, "ひっかかって");
#endif
        break;
    }
    if (wizxtra) { /* give extra information for wizard mode enlightenment */
        /* curly braces: u.utrap is an escape attempt counter rather than a
           turn timer so use different ornamentation than usual parentheses */
        Sprintf(eos(outbuf), " {%u}", u.utrap);
    }
    return outbuf;
}

/* check whether hero is wearing something that player definitely knows
   confers the target property; item must have been seen and its type
   discovered but it doesn't necessarily have to be fully identified */
staticfn boolean
cause_known(
    int propindx) /* index of a property which can be conveyed by worn item */
{
    struct obj *o;
    long mask = W_ARMOR | W_AMUL | W_RING | W_TOOL;

    /* simpler than from_what()/what_gives(); we don't attempt to
       handle artifacts and we deliberately ignore wielded items */
    for (o = gi.invent; o; o = o->nobj) {
        if (!(o->owornmask & mask))
            continue;
        if ((int) objects[o->otyp].oc_oprop == propindx
            && objects[o->otyp].oc_name_known && o->dknown)
            return TRUE;
    }
    return FALSE;
}

/* format a characteristic value, accommodating Strength's strangeness */
staticfn char *
attrval(
    int attrindx,
    int attrvalue,
    char resultbuf[]) /* should be at least [7] to hold "18/100\0" */
{
    if (attrindx != A_STR || attrvalue <= 18)
        Sprintf(resultbuf, "%d", attrvalue);
    else if (attrvalue > STR18(100)) /* 19 to 25 */
        Sprintf(resultbuf, "%d", attrvalue - 100);
    else /* simplify "18/\**" to be "18/100" */
        Sprintf(resultbuf, "18/%02d", attrvalue - 18);
    return resultbuf;
}

/* format urealtime.realtime as
      " D days, H hours, M minutes and S seconds"
   with any fields having a value of 0 omitted:
      0-00:00:20 => " 20 seconds"
      0-00:15:05 => " 15 minutes and 5 seconds"
      0-00:16:00 => " 16 minutes"
      0-01:15:10 => " 1 hour, 15 minutes and 10 seconds"
      0-02:00:01 => " 2 hours and 1 second"
      3-00:25:40 => " 3 days, 25 minutes and 40 seconds"
   (note: for a list of more than two entries, nethack usually includes the
   [style-wise] optional comma before "and" but in this instance it does not)
 */
staticfn char *
fmt_elapsed_time(char *outbuf, int final)
{
    int fieldcnt;
    long edays, ehours, eminutes, eseconds;
    /* for a game that's over, reallydone() has updated urealtime.realtime
       to its final value before calling us during end of game disclosure;
       for a game that's still in progress, it holds the amount of elapsed
       game time from previous sessions up through most recent save/restore
       (or up through latest level change when 'checkpoint' is On);
       '.start_timing' has a non-zero value even if '.realtime' is 0 */
    long etim = urealtime.realtime;

    if (!final)
        etim += timet_delta(getnow(), urealtime.start_timing);
    /* we could use localtime() to convert the value into a 'struct tm'
       to get date and time fields but this is simple and straightforward */
    eseconds = etim % 60L, etim /= 60L;
    eminutes = etim % 60L, etim /= 60L;
    ehours = etim % 24L;
    edays = etim / 24L;
    fieldcnt = !!edays + !!ehours + !!eminutes + !!eseconds;

#if 0 /*JP:T*/
    Strcpy(outbuf, fieldcnt ? "" : " none"); /* 'none' should never happen */
#else
    Strcpy(outbuf, fieldcnt ? "" : "なし"); /* 'none' should never happen */
#endif
    if (edays) {
#if 0 /*JP:T*/
        Sprintf(eos(outbuf), " %ld day%s", edays, plur(edays));
#else
        Sprintf(eos(outbuf), "%ld日", edays);
#endif
#if 0 /*JP:T*/
        if (fieldcnt > 1) /* hours and/or minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? " and" : ",");
#endif
        --fieldcnt; /* edays has been processed */
    }
    if (ehours) {
#if 0 /*JP:T*/
        Sprintf(eos(outbuf), " %ld hour%s", ehours, plur(ehours));
#else
        Sprintf(eos(outbuf), "%ld時間", ehours);
#endif
#if 0 /*JP:T*/
        if (fieldcnt > 1) /* minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? " and" : ",");
#endif
        --fieldcnt; /* ehours has been processed */
    }
    if (eminutes) {
#if 0 /*JP:T*/
        Sprintf(eos(outbuf), " %ld minute%s", eminutes, plur(eminutes));
#else
        Sprintf(eos(outbuf), "%ld分", eminutes);
#endif
#if 0 /*JP:T*/
        if (fieldcnt > 1) /* seconds to follow */
            Strcat(outbuf, " and");
        /* eminutes has been processed but no need to decrement fieldcnt */
#endif
    }
    if (eseconds)
#if 0 /*JP:T*/
        Sprintf(eos(outbuf), " %ld second%s", eseconds, plur(eseconds));
#else
        Sprintf(eos(outbuf), "%ld秒", eseconds);
#endif
    return outbuf;
}

/* "once" vs "twice" vs "17 times", used in several places */
staticfn char *
N_times(long n, char *outbuf)
{
#if 0 /*JP:T*/
    switch (n) {
    case 0:
    default:
        Sprintf(outbuf, "%ld times", n);
        break;
    case 1:
        Strcpy(outbuf, "once");
        break;
    case 2:
        Strcpy(outbuf, "twice");
        break;
    case 3:
        Strcpy(outbuf, "thrice");
        break;
    }
#else
    Sprintf(outbuf, "%ld回", n);
#endif
    return outbuf;
}

void
enlightenment(
    int mode,  /* BASICENLIGHTENMENT | MAGICENLIGHTENMENT (| both) */
    int final) /* ENL_GAMEINPROGRESS:0, ENL_GAMEOVERALIVE, ENL_GAMEOVERDEAD */
{
    char buf[BUFSZ], tmpbuf[BUFSZ];

    ge.en_win = create_nhwindow(NHW_MENU);
    ge.en_via_menu = !final;
    if (ge.en_via_menu)
        start_menu(ge.en_win, MENU_BEHAVE_STANDARD);

    Strcpy(tmpbuf, svp.plname);
    *tmpbuf = highc(*tmpbuf); /* same adjustment as bottom line */
    /* as in background_enlightenment, when poly'd we need to use the saved
       gender in u.mfemale rather than the current you-as-monster gender */
#if 0 /*JP:T*/
    Snprintf(buf, sizeof(buf), "%s the %s's attributes:", tmpbuf,
             ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
                ? gu.urole.name.f
                : gu.urole.name.m);
#else
    Snprintf(buf, sizeof(buf), "%sの%sの属性:",
             ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
                ? gu.urole.name.f
                : gu.urole.name.m,
                tmpbuf);
#endif

    /* title */
    enlght_out(buf); /* "Conan the Archeologist's attributes:" */
    /* background and characteristics; ^X or end-of-game disclosure */
    if (mode & BASICENLIGHTENMENT) {
        /* role, race, alignment, deities, dungeon level, time, experience */
        background_enlightenment(mode, final);
        /* hit points, energy points, armor class, gold */
        basics_enlightenment(mode, final);
        /* strength, dexterity, &c */
        characteristics_enlightenment(mode, final);
    }
    /* expanded status line information, including things which aren't
       included there due to space considerations;
       shown for both basic and magic enlightenment */
    status_enlightenment(mode, final);
    /* remaining attributes; shown for potion,&c or wizard mode and
       explore mode ^X or end of game disclosure */
    if (mode & MAGICENLIGHTENMENT) {
        /* intrinsics and other traditional enlightenment feedback */
        attributes_enlightenment(mode, final);
    }

    enlght_out(""); /* separator */
/*JP
    enlght_out("Miscellaneous:");
*/
    enlght_out("その他:");
    /* reminder to player and/or information for dumplog */
    if ((mode & BASICENLIGHTENMENT) != 0 && (wizard || discover || final)) {
        if (wizard || discover) {
/*JP
            Sprintf(buf, "running in %s mode", wizard ? "debug" : "explore");
*/
            Sprintf(buf, "%sモードに", wizard ? "デバッグ" : "探索");
/*JP
            you_are(buf, "");
*/
            you_are_ing(buf, "");
        }

        if (!flags.bones) {
            /* mention not saving bones iff hero just died */
#if 0 /*JP:T*/
            Sprintf(buf, "disabled loading%s of bones levels",
                    (final == ENL_GAMEOVERDEAD) ? " and storing" : "");
            you_have_X(buf);
#else
            Sprintf(buf, "骨レベルの読み込み%sは無効",
                    (final == ENL_GAMEOVERDEAD) ? "と保管" : "");
            you_are(buf, "");
#endif
        } else if (!u.uroleplay.numbones) {
#if 0 /*JP:T*/
            enl_msg(You_, "haven't encountered", "didn't encounter",
                    " any bones levels", "");
#else
            enl_msg(You_, "遭遇していない", "遭遇しなかった",
                    "骨レベルに", "");
#endif
        } else {
#if 0 /*JP:T*/
            Sprintf(buf, "encountered %ld bones level%s",
                    u.uroleplay.numbones, plur(u.uroleplay.numbones));
            you_have_X(buf);
#else
            Sprintf(buf, "%ld回骨レベルに遭遇し",
                    u.uroleplay.numbones);
            you_have_X(buf);
#endif
        }
    }
    (void) fmt_elapsed_time(buf, final);
#if 0 /*JP:T*/
    enl_msg("Total elapsed playing time ", "is", "was", buf, "");
#else
    enl_msg("合計経過時間は", "である", "であった", buf, "");
#endif

    if (!ge.en_via_menu) {
        display_nhwindow(ge.en_win, TRUE);
    } else {
        menu_item *selected = 0;

        end_menu(ge.en_win, (char *) 0);
        if (select_menu(ge.en_win, PICK_NONE, &selected) > 0)
            free((genericptr_t) selected);
        ge.en_via_menu = FALSE;
    }
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*ARGSUSED*/
/* display role, race, alignment and such to en_win */
staticfn void
background_enlightenment(int unused_mode UNUSED, int final)
{
    const char *role_titl, *rank_titl;
    int innategend, difgend, difalgn;
    char buf[BUFSZ], tmpbuf[BUFSZ];

    /* note that if poly'd, we need to use u.mfemale instead of flags.female
       to access hero's saved gender-as-human/elf/&c rather than current */
    innategend = (Upolyd ? u.mfemale : flags.female) ? 1 : 0;
    role_titl = (innategend && gu.urole.name.f) ? gu.urole.name.f
                                                : gu.urole.name.m;
    rank_titl = rank_of(u.ulevel, Role_switch, innategend);

    enlght_out(""); /* separator after title */
/*JP
    enlght_out("Background:");
*/
    enlght_out("背景情報:");

    /* if polymorphed, report current shape before underlying role;
       will be repeated as first status: "you are transformed" and also
       among various attributes: "you are in beast form" (after being
       told about lycanthropy) or "you are polymorphed into <a foo>"
       (with countdown timer appended for wizard mode); we really want
       the player to know he's not a samurai at the moment... */
    if (Upolyd) {
        char anbuf[20]; /* includes trailing space; [4] suffices */
        struct permonst *uasmon = gy.youmonst.data;
        boolean altphrasing = vampshifted(&gy.youmonst);

        tmpbuf[0] = '\0';
        /* here we always use current gender, not saved role gender */
        if (!is_male(uasmon) && !is_female(uasmon) && !is_neuter(uasmon))
/*JP
            Sprintf(tmpbuf, "%s ", genders[flags.female ? 1 : 0].adj);
*/
            Sprintf(tmpbuf, "%sの", genders[flags.female ? 1 : 0].adj);
        if (altphrasing)
#if 0 /*JP:T*/
            Sprintf(eos(tmpbuf), "%s in ",
                    pmname(&mons[gy.youmonst.cham],
                           flags.female ? FEMALE : MALE));
#else
            Sprintf(eos(tmpbuf), "%sの姿の",
                    pmname(&mons[gy.youmonst.cham],
                           flags.female ? FEMALE : MALE));
#endif
#if 0 /*JP:T*/
        Snprintf(buf, sizeof(buf), "%s%s%s%s form",
                 !final ? "currently " : "",
                 altphrasing ? just_an(anbuf, tmpbuf) : "in ",
                 tmpbuf, pmname(uasmon, flags.female ? FEMALE : MALE));
#else
        Snprintf(buf, sizeof(buf), "%s%s%s%sの姿",
                 !final ? "今のところ" : "",
                 altphrasing ? just_an(anbuf, tmpbuf) : "で",
                 tmpbuf, pmname(uasmon, flags.female ? FEMALE : MALE));
#endif
        you_are(buf, "");
    }

    /* report role; omit gender if it's redundant (eg, "female priestess") */
    tmpbuf[0] = '\0';
    if (!gu.urole.name.f
        && ((gu.urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            || innategend != flags.initgend))
/*JP
        Sprintf(tmpbuf, "%s ", genders[innategend].adj);
*/
        Sprintf(tmpbuf, "%sの", genders[innategend].adj);
    buf[0] = '\0';
    if (Upolyd)
#if 0 /*JP:T*/
        Strcpy(buf, "actually "); /* "You are actually a ..." */
#else
        Strcpy(buf, "実際には"); /* "あなたは実際には..." */
#endif
    if (!strcmpi(rank_titl, role_titl)) {
        /* omit role when rank title matches it */
#if 0 /*JP:T*/
        Sprintf(eos(buf), "%s, level %d %s%s", an(rank_titl), u.ulevel,
                tmpbuf, gu.urace.noun);
#else
        Sprintf(eos(buf), "レベル%dの%s%s%s", u.ulevel,
                tmpbuf, gu.urace.adj, rank_titl);
#endif
    } else {
#if 0 /*JP:T*/
        Sprintf(eos(buf), "%s, a level %d %s%s %s", an(rank_titl), u.ulevel,
                tmpbuf, gu.urace.adj, role_titl);
#else
        Sprintf(eos(buf), "レベル%dの%s%s%sの%s", u.ulevel,
                tmpbuf, gu.urace.adj, role_titl, rank_titl);
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
                  : (!u.uconduct.gnostic && svm.moves > 1000L)
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
                  : (!u.uconduct.gnostic && svm.moves > 1000L) ? "名義上"
                     /* lastly, normal case */
                     : "",
            u_gname(), !final ? iru : ita);
#endif
    enlght_out(buf);
    /* show the rest of this game's pantheon (finishes previous sentence)
       [appending "also Moloch" at the end would allow for straightforward
       trailing "and" on all three aligned entries but looks too verbose] */
#if 0 /*JP*/
    Sprintf(buf, " who %s opposed by", !final ? "is" : "was");
#else
    Strcpy(buf, "あなたは");
#endif
    if (u.ualign.type != A_LAWFUL)
#if 0 /*JP:T*/
        Sprintf(eos(buf), " %s (%s) and", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
#else
        Sprintf(eos(buf), "%s(%s)および", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
#endif
    if (u.ualign.type != A_NEUTRAL)
#if 0 /*JP:T*/
        Sprintf(eos(buf), " %s (%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? " and" : "");
#else
        Sprintf(eos(buf), "%s(%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? "および" : "");
#endif
    if (u.ualign.type != A_CHAOTIC)
#if 0 /*JP:T*/
        Sprintf(eos(buf), " %s (%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
#else
        Sprintf(eos(buf), "%s(%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
#endif
#if 0 /*JP*/
    Strcat(buf, "."); /* terminate sentence */
#else
    Sprintf(eos(buf), "と対立して%s．", !final ? iru : ita);
#endif
    enlght_out(buf);

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
#if 0 /*JP*/
        you_are(buf, "");
#else
        enl_msg(buf, "である", "であった", "", "");
#endif
        difalgn &= ~1; /* suppress helm from "started out <foo>" message */
    }
    if (difgend || difalgn) { /* sex change or perm align change or both */
#if 0 /*JP:T*/
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
        enlght_out(buf);
    }

    /* "You are left-handed." won't work well if polymorphed into something
       without hands; use "You are normally left-handed." in that situation */
#if 0 /*JP:T*/
    Sprintf(buf, "%s%s-handed",
            !strcmp(body_part(HANDED), "handed") ? "" : "normally ",
            URIGHTY ? "right" : "left");
#else
    Sprintf(buf, "%s%s利き",
            !strcmp(body_part(HANDED), "手にする") ? "" : "通常",
            URIGHTY ? "右" : "左");
#endif
    you_are(buf, "");

    /* As of 3.6.2: dungeon level, so that ^X really has all status info as
       claimed by the comment below; this reveals more information than
       the basic status display, but that's one of the purposes of ^X;
       similar information is revealed by #overview; the "You died in
       <location>" given by really_done() is more rudimentary than this */
    *buf = *tmpbuf = '\0';
    if (In_endgame(&u.uz)) {
        int egdepth = observable_depth(&u.uz);

        (void) endgamelevelname(tmpbuf, egdepth);
#if 0 /*JP*/
        Snprintf(buf, sizeof(buf), "in the endgame, on the %s%s",
                 !strncmp(tmpbuf, "Plane", 5) ? "Elemental " : "", tmpbuf);
#else
        Snprintf(buf, sizeof(buf), "最終試練の%s", tmpbuf);
#endif
    } else if (Is_knox(&u.uz)) {
        /* this gives away the fact that the knox branch is only 1 level */
/*JP
        Sprintf(buf, "on the %s level", svd.dungeons[u.uz.dnum].dname);
*/
        Sprintf(buf, "%s", svd.dungeons[u.uz.dnum].dname);
        /* TODO? maybe phrase it differently when actually inside the fort,
           if we're able to determine that (not trivial) */
    } else {
        char dgnbuf[QBUFSZ];

        Strcpy(dgnbuf, svd.dungeons[u.uz.dnum].dname);
#if 0 /*JP*/
        if (!strncmpi(dgnbuf, "The ", 4))
            *dgnbuf = lowc(*dgnbuf);
#endif
#if 0 /*JP*/
        Sprintf(tmpbuf, "level %d",
                In_quest(&u.uz) ? dunlev(&u.uz) : depth(&u.uz));
#else
        if (In_quest(&u.uz)) {
            Sprintf(tmpbuf, "第%d階層", dunlev(&u.uz));
        } else {
            Sprintf(tmpbuf, "地下%d階", depth(&u.uz));
        }
#endif
        /* TODO? maybe extend this bit to include various other automatic
           annotations from the dungeon overview code */
        if (Is_rogue_level(&u.uz))
/*JP
            Strcat(tmpbuf, ", a primitive area");
*/
            Strcat(tmpbuf, ", 単純な世界");
        else if (Is_bigroom(&u.uz) && !Blind)
/*JP
            Strcat(tmpbuf, ", a very big room");
*/
            Strcat(tmpbuf, ", とても大きな部屋");
#if 0 /*JP:T*/
        Snprintf(buf, sizeof(buf), "in %s, on %s", dgnbuf, tmpbuf);
#else
        Snprintf(buf, sizeof(buf), "%sの%s", dgnbuf, tmpbuf);
#endif
    }
    you_are(buf, "");

    /* this is shown even if the 'time' option is off */
    if (svm.moves == 1L) {
#if 0 /*JP:T*/
        you_have("just started your adventure", "");
#else
        enlght_line(You_, "", "冒険を開始したところだ", "");
#endif
    } else {
        /* 'turns' grates on the nerves in this context... */
#if 0 /*JP:T*/
        Sprintf(buf, "the dungeon %ld turn%s ago",
                svm.moves, plur(svm.moves));
#else
        Sprintf(buf, "%ldターン前に迷宮に入った",
                svm.moves);
#endif
        /* same phrasing for current and final: "entered" is unconditional */
#if 0 /*JP:T*/
        enlght_line(You_, "entered ", buf, "");
#else
        enlght_line(You_, "", buf, "");
#endif
    }

    /* for gameover, these have been obtained in really_done() so that they
       won't vary if user leaves a disclosure prompt or --More-- unanswered
       long enough for the dynamic value to change between then and now */
    if (final ? iflags.at_midnight : midnight()) {
#if 0 /*JP:T*/
        enl_msg("It ", "is ", "was ", "the midnight hour", "");
#else
        enl_msg("時間帯は深夜", "だ", "だった", "", "");
#endif
    } else if (final ? iflags.at_night : night()) {
#if 0 /*JP:T*/
        enl_msg("It ", "is ", "was ", "nighttime", "");
#else
        enl_msg("時間帯は夜", "だ", "だった", "", "");
#endif
    }
    /* other environmental factors */
    if (flags.moonphase == FULL_MOON || flags.moonphase == NEW_MOON) {
        /* [This had "tonight" but has been changed to "in effect".
           There is a similar issue to Friday the 13th--it's the value
           at the start of the current session but that session might
           have dragged on for an arbitrary amount of time.  We want to
           report the values that currently affect play--or affected
           play when game ended--rather than actual outside situation.] */
#if 0 /*JP:T*/
        Sprintf(buf, "a %s moon in effect%s",
                (flags.moonphase == FULL_MOON) ? "full"
                : (flags.moonphase == NEW_MOON) ? "new"
                  /* showing these would probably just lead to confusion
                     since they have no effect on game play... */
                  : (flags.moonphase < FULL_MOON) ? "first quarter"
                    : "last quarter",
                /* we don't have access to 'how' here--aside from survived
                   vs died--so settle for general platitude */
                final ? " when your adventure ended" : "");
        enl_msg("There ", "is ", "was ", buf, "");
#else
        Sprintf(buf, "%s%s月",
                /* we don't have access to 'how' here--aside from survived
                   vs died--so settle for general platitude */
                final ? "冒険を終えたとき，" : "",
                (flags.moonphase == FULL_MOON) ? "満"
                : (flags.moonphase == NEW_MOON) ? "新"
                  /* showing these would probably just lead to confusion
                     since they have no effect on game play... */
                  : (flags.moonphase < FULL_MOON) ? "上弦の"
                    : "下弦の");
        enl_msg("", "だ", "だった", buf, "");
#endif
    }
    if (flags.friday13) {
        /* let player know that friday13 penalty is/was in effect;
           we don't say "it is/was Friday the 13th" because that was at
           the start of the session and it might be past midnight (or
           days later if the game has been paused without save/restore),
           so phrase this similar to the start up message */
#if 0 /*JP:T*/
        Sprintf(buf, " Bad things %s on Friday the 13th.",
                !final ? "can happen"
                : (final == ENL_GAMEOVERALIVE) ? "could have happened"
                  /* there's no may to tell whether -1 Luck made a
                     difference but hero has died... */
                  : "happened");
#else
        Sprintf(buf, "１３日の金曜日にはよくないことが%s．",
                !final ? "ある"
                : (final == ENL_GAMEOVERALIVE) ? "あったかもしれない"
                  /* there's no may to tell whether -1 Luck made a
                     difference but hero has died... */
                  : "あった");
#endif
        enlght_out(buf);
    }

    if (!Upolyd) {
        int ulvl = (int) u.ulevel;
        /* [flags.showexp currently does not matter; should it?] */

        /* experience level is already shown above */
#if 0 /*JP*/
        Sprintf(buf, "%-1ld experience point%s", u.uexp, plur(u.uexp));
#else
        Sprintf(buf, "経験値%-1ldポイント", u.uexp);
#endif
        /* TODO?
         *  Remove wizard-mode restriction since patient players can
         *  determine the numbers needed without resorting to spoilers
         *  (even before this started being disclosed for 'final';
         *  just enable 'showexp' and look at normal status lines
         *  after drinking gain level potions or eating wraith corpses
         *  or being level-drained by vampires).
         */
        if (ulvl < 30 && (final || wizard)) {
            long nxtlvl = newuexp(ulvl), delta = nxtlvl - u.uexp;

#if 0 /*JP*/
            Sprintf(eos(buf), ", %ld %s%sneeded %s level %d",
                    delta, (u.uexp > 0) ? "more " : "",
                    /* present tense=="needed", past tense=="were needed" */
                    !final ? "" : (delta == 1L) ? "was " : "were ",
                    /* "for": grammatically iffy but less likely to wrap */
                    (ulvl < 18) ? "to attain" : "for", (ulvl + 1));
#else
            Sprintf(eos(buf), "(レベル%dまで%ldポイント)",
                    (ulvl + 1), delta);
#endif
        }
        you_have(buf, "");
    }
#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        /* describes what's shown on status line, which is an approximation;
           only show it here if player has the 'showscore' option enabled */
#if 0 /*JP*/
        Sprintf(buf, "%ld%s", botl_score(),
                !final ? "" : " before end-of-game adjustments");
        enl_msg("Your score ", "is ", "was ", buf, "");
#else
        Sprintf(buf, "%s%ld",
                !final ? "" : "ゲーム終了時の調整前は",
                botl_score());
        enl_msg("あなたのスコアは", "である", "であった", buf, "");
#endif
    }
#endif
}

/* hit points, energy points, armor class -- essential information which
   doesn't fit very well in other categories */
/*ARGSUSED*/
staticfn void
basics_enlightenment(int mode UNUSED, int final)
{
#if 0 /*JP*//*unused*/
    static char Power[] = "energy points (spell power)";
#endif
    char buf[BUFSZ];
    int pw = u.uen, hp = (Upolyd ? u.mh : u.uhp),
        pwmax = u.uenmax, hpmax = (Upolyd ? u.mhmax : u.uhpmax);

    enlght_out(""); /* separator after background */
/*JP
    enlght_out("Basics:");
*/
    enlght_out("基本:");

    if (hp < 0)
        hp = 0;
    /* "1 out of 1" rather than "all" if max is only 1; should never happen */
#if 0 /*JP*/
    if (hp == hpmax && hpmax > 1)
        Sprintf(buf, "all %d hit points", hpmax);
    else
        Sprintf(buf, "%d out of %d hit point%s", hp, hpmax, plur(hpmax));
#else
    Sprintf(buf, "%dヒットポイント(最大:%d)", hp, hpmax);
#endif
    you_have(buf, "");

    /* low max energy is feasible, so handle couple of extra special cases */
#if 0 /*JP*/
    if (pwmax == 0 || (pw == pwmax && pwmax == 2)) /* both: not "all 2" */
        Sprintf(buf, "%s %s", !pwmax ? "no" : "both", Power);
    else if (pw == pwmax && pwmax > 2)
        Sprintf(buf, "all %d %s", pwmax, Power);
    else
        Sprintf(buf, "%d out of %d %s", pw, pwmax, Power);
#else
    Sprintf(buf, "%d魔力ポイント(最大:%d)", pw, pwmax);
#endif
    you_have(buf, "");

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
        you_have(buf, "");
    }

    find_ac(); /* enforces AC_MAX cap */
#if 0 /*JP:T*/
    Sprintf(buf, "%d", u.uac);
    if (abs(u.uac) == AC_MAX)
        Sprintf(eos(buf), ", the %s possible",
                (u.uac < 0) ? "best" : "worst");
#else
    Sprintf(buf, "%s%d",
                (abs(u.uac) == AC_MAX) ? "最高値である" : "", u.uac);
#endif
/*JP
    enl_msg("Your armor class ", "is ", "was ", buf, "");
*/
    enl_msg("あなたの防御値は", "である", "であった", buf, "");

    /* gold; similar to doprgold (#showgold) but without shop billing info;
       includes container contents, unlike status line but like doprgold */
    {
        long umoney = money_cnt(gi.invent), hmoney = hidden_gold(final);

        if (!umoney) {
#if 0 /*JP:T*/
            Sprintf(buf, " Your wallet %s empty", !final ? "is" : "was");
#else
            Sprintf(buf, "あなたの財布は空%s", !final ? "である" : "だった");
#endif
        } else {
#if 0 /*JP:T*/
            Sprintf(buf, " Your wallet contain%s %ld %s", !final ? "s" : "ed",
                    umoney, currency(umoney));
#else
            Sprintf(buf, "あなたの財布には%ld%s入ってい%s",
                    umoney, currency(umoney), !final ? "る" : "た");
#endif
        }
        /* terminate the wallet line if appropriate, otherwise add an
           introduction to subsequent continuation; output now either way */
/*JP
        Strcat(buf, !hmoney ? "." : !umoney ? ", but" : ", and");
*/
        Strcat(buf, !hmoney ? "．" : !umoney ? "が，" : "．そして");
        enlght_out(buf);

        /* put contained gold on its own line to avoid excessive width; it's
           phrased as a continuation of the wallet line so not capitalized */
        if (hmoney) {
#if 0 /*JP:T*/
            Sprintf(buf, "%ld %s stashed away in your pack",
                    hmoney, umoney ? "more" : currency(hmoney));
            enl_msg("you ", "have ", "had ", buf, "");
#else
            Sprintf(buf, "ナップザックに%s%ld%s分けて入って",
                    umoney ? "さらに" : "", hmoney, currency(hmoney));
            you_are_ing(buf, "");
#endif
        }
    }

    if (flags.pickup) {
        char ocl[MAXOCLASSES + 1];

#if 0 /*JP*//*後に回す*/
        Strcpy(buf, "on");
#endif
        if (costly_spot(u.ux, u.uy)) {
            /* being in a shop inhibits autopickup, even 'pickup_thrown' */
#if 0 /*JP*/
            Strcat(buf, ", but temporarily disabled while inside the shop");
#else
            Strcpy(buf, "オン，しかし店の中では一時的にオフ");
#endif
        } else {
            oc_to_str(flags.pickup_types, ocl);
#if 0 /*JP*/
            Sprintf(eos(buf), " for %s%s%s", *ocl ? "'" : "",
                    *ocl ? ocl : "all types", *ocl ? "'" : "");
#else
            Sprintf(buf, "%s%s%s",
                *ocl ? "'" : "", *ocl ? ocl : "全ての種類", *ocl ? "'" : "");
#endif
            if (flags.pickup_thrown && *ocl)
#if 0 /*JP:T*/
                Strcat(buf, " plus thrown"); /* show when not 'all types' */
#else
                Strcat(buf, "に加えて投げるもの"); /* show when not 'all types' */
#endif
            if (ga.apelist)
/*JP
                Strcat(buf, ", with exceptions");
*/
                Strcat(buf, "(例外あり)");
#if 1 /*JP*/
            Strcat(buf, "に対してオン");
#endif
        }
    } else
/*JP
        Strcpy(buf, "off");
*/
        Strcpy(buf, "オフ");
/*JP
    enl_msg("Autopickup ", "is ", "was ", buf, "");
*/
    enl_msg("自動拾い設定は", "である", "であった", buf, "");
}

/* characteristics: expanded version of bottom line strength, dexterity, &c */
staticfn void
characteristics_enlightenment(int mode, int final)
{
    char buf[BUFSZ];

    enlght_out("");
#if 0 /*JP:T*/
    Sprintf(buf, "%sCharacteristics:", !final ? "" : "Final ");
#else
    Sprintf(buf, "%s特性：", !final ? "" : "最終");
#endif
    enlght_out(buf);

    /* bottom line order */
    one_characteristic(mode, final, A_STR); /* strength */
    one_characteristic(mode, final, A_DEX); /* dexterity */
    one_characteristic(mode, final, A_CON); /* constitution */
    one_characteristic(mode, final, A_INT); /* intelligence */
    one_characteristic(mode, final, A_WIS); /* wisdom */
    one_characteristic(mode, final, A_CHA); /* charisma */
}

/* display one attribute value for characteristics_enlightenment() */
staticfn void
one_characteristic(int mode, int final, int attrindx)
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
        if (u_wield_art(ART_OGRESMASHER) && uwep->cursed)
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
#if 0 /*JP:T*/
            Sprintf(eos(valubuf), "%sbase:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
#else
            Sprintf(eos(valubuf), "%s基本:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
#endif
            paren_pfx = ", ";
        }
        if (abase != apeak) {
#if 0 /*JP:T*/
            Sprintf(eos(valubuf), "%speak:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
#else
            Sprintf(eos(valubuf), "%s最大:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
#endif
            paren_pfx = ", ";
        }
        if (interesting_alimit) {
#if 0 /*JP:T*/
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
staticfn void
status_enlightenment(int mode, int final)
{
    boolean magic = (mode & MAGICENLIGHTENMENT) ? TRUE : FALSE;
    int cap;
    char buf[BUFSZ], youtoo[BUFSZ], heldmon[BUFSZ];
#if 0 /*JP:T*/
    boolean Riding = (u.usteed
                      /* if hero dies while dismounting, u.usteed will still
                         be set; we want to ignore steed in that situation */
                      && !(final == ENL_GAMEOVERDEAD
                           && !strcmp(svk.killer.name, "riding accident")));
#else /*JP:steed.cと合わせる必要あり*/
    boolean Riding = (u.usteed
                      /* if hero dies while dismounting, u.usteed will still
                         be set; we want to ignore steed in that situation */
                      && !(final == ENL_GAMEOVERDEAD
                           && !strcmp(svk.killer.name, "騎乗事故で")));
#endif
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
    enlght_out(""); /* separator after title or characteristics */
/*JP
    enlght_out(final ? "Final Status:" : "Status:");
*/
    enlght_out(final ? "最終状態:" : "状態:");

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
#if 0 /*JP:T*/
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
        enl_msg(You_, "いる", "いた", Swimming ? "泳いで" : "水中に", from_what(SWIMMING));
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
    if (Upolyd && (u.uundetected || U_AP_TYPE != M_AP_NOTHING))
        youhiding(TRUE, final);

    /* internal troubles, mostly in the order that prayer ranks them */
    if (Stoned) {
        if (final && (Stoned & I_SPECIAL))
/*JP
            enlght_out(" You turned into stone.");
*/
            enlght_out(" あなたは石になった．");
        else
/*JP
            you_are("turning to stone", "");
*/
            enl_msg("あなたは", "なりつつある", "なった", "石に", "");
    }
    if (Slimed) {
        if (final && (Slimed & I_SPECIAL))
/*JP
            enlght_out(" You turned into slime.");
*/
            enlght_out(" あなたはスライムになった．");
        else
/*JP
            you_are("turning into slime", "");
*/
            enl_msg("あなたは", "なりつつある", "なった", "スライムに", "");
    }
    if (Strangled) {
        if (u.uburied) {
/*JP
            you_are("buried", "");
*/
            you_are_ing("埋まって", "");
        } else {
            if (final && (Strangled & I_SPECIAL)) {
/*JP
                enlght_out(" You died from strangulation.");
*/
                enlght_out(" あなたは窒息死した．");
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
    }
    if (Sick) {
        /* the two types of sickness are lumped together; hero can be
           afflicted by both but there is only one timeout; botl status
           puts TermIll before FoodPois and death due to timeout reports
           terminal illness if both are in effect, so do the same here */
        if (final && (Sick & I_SPECIAL)) {
#if 0 /*JP:T*/
            Sprintf(buf, " %sdied from %s.", You_, /* has trailing space */
                    (u.usick_type & SICK_NONVOMITABLE)
                    ? "terminal illness" : "food poisoning");
#else
            Sprintf(buf, " %s%sで死んだ．", You_, /* has trailing space */
                    (u.usick_type & SICK_NONVOMITABLE)
                    ? "病気" : "食中毒");
#endif
            enlght_out(buf);
        } else {
            /* unlike death due to sickness, report the two cases separately
               because it is possible to cure one without curing the other */
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
    }
    if (Vomiting)
/*JP
        you_are("nauseated", "");
*/
        enl_msg(You_, "ある", "あった", "吐き気が", "");
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
        /* check the reasons in same order as from_what() */
#if 0 /*JP:T*/
        Sprintf(buf, "%s blind",
                (HBlinded & FROMOUTSIDE) != 0L ? "permanently"
                : (HBlinded & FROMFORM) ? "innately"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "deliberately"
                    /* timed, possibly combined with blindfold */
                    : "temporarily");
#else
        Sprintf(buf, "%s盲目",
                (HBlinded & FROMOUTSIDE) != 0L ? "恒久的に"
                : (HBlinded & FROMFORM) ? "生まれながらに"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "故意に"
                    /* timed, possibly combined with blindfold */
                    : "一時的に");
#endif
        if (wizard && (HBlinded == BlindedTimeout && !Blindfolded))
            Sprintf(eos(buf), " (%ld)", BlindedTimeout);
        /* !haseyes: avoid "you are innately blind innately" */
        you_are(buf, !haseyes(gy.youmonst.data) ? "" : from_what(BLINDED));
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
#if 0 /*JP:T*/
        you_are(buf, "");
#else
        you_are_ing(buf, "");
#endif
    }
    if (u.utrap) {
        char predicament[BUFSZ];
        boolean anchored = (u.utraptype == TT_BURIEDBALL);

        (void) trap_predicament(predicament, final, wizard);
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
    heldmon[0] = '\0'; /* lint suppression */
    if (u.ustuck) { /* includes u.uswallow */
        Strcpy(heldmon, a_monnam(u.ustuck));
#if 0 /*JP:T*/
        if (!strcmp(heldmon, "it")
            && (!has_mgivenname(u.ustuck)
                || strcmp(MGIVENNAME(u.ustuck), "it") != 0))
            Strcpy(heldmon, "an unseen creature");
#else
        if (!strcmp(heldmon, "何者か")
            && (!has_mgivenname(u.ustuck)
                || strcmp(MGIVENNAME(u.ustuck), "何者か") != 0))
            Strcpy(heldmon, "未確認の怪物");
#endif
    }
    if (u.uswallow) {
        assert(u.ustuck != NULL); /* implied by u.uswallow */
#if 0 /*JP:T*/
        Snprintf(buf, sizeof buf, "%s by %s",
                digests(u.ustuck->data) ? "swallowed" : "engulfed",
                heldmon);
#else
        Snprintf(buf, sizeof buf, "%sに%s込まれた状態",
                heldmon,
                digests(u.ustuck->data) ? "飲み" : "巻き");
#endif
        if (dmgtype(u.ustuck->data, AD_DGST)) {
            /* if final, death via digestion can be deduced by u.uswallow
               still being True and u.uswldtim having been decremented to 0 */
            if (final && !u.uswldtim)
/*JP
                Strcat(buf, " and got totally digested");
*/
                Strcat(buf, "，完全に消化された状態");
            else
#if 0 /*JP:T*/
                Sprintf(eos(buf), " and %s being digested",
                        final ? "were" : "are");
#else
                Strcat(buf, "，消化されている途中");
#endif
        }
        if (wizard)
            Sprintf(eos(buf), " (%u)", u.uswldtim);
        you_are(buf, "");
    } else if (u.ustuck) {
        boolean ustick = (Upolyd && sticks(gy.youmonst.data));
        int dx = u.ustuck->mx - u.ux, dy = u.ustuck->my - u.uy;

#if 0 /*JP*/
        Snprintf(buf, sizeof buf, "%s %s (%s)",
                 ustick ? "holding" : "held by",
                 heldmon, dxdy_to_dist_descr(dx, dy, TRUE));
        you_are(buf, "");
#else
        Snprintf(buf, sizeof buf, "%s(%s)%s",
                 heldmon,
                 dxdy_to_dist_descr(dx, dy, TRUE),
                 ustick ? "を捕まえて" : "に捕まって");
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
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long whichleg = (EWounded_legs & BOTH_SIDES);
#if 0 /*JP:T*/
        const char *bp = u.usteed ? mbodypart(u.usteed, LEG) : body_part(LEG),
            *article = "a ", /* precedes "wounded", so never "an " */
            *leftright = "";

        if (whichleg == BOTH_SIDES)
            bp = makeplural(bp), article = "";
        else
            leftright = (whichleg == LEFT_SIDE) ? "left " : "right ";
        Sprintf(buf, "%swounded %s%s", article, leftright, bp);
#else
        const char *bp = u.usteed ? mbodypart(u.usteed, LEG) : body_part(LEG),
            *leftright;

        if (whichleg == BOTH_SIDES)
            leftright = "両";
        else
            leftright = (whichleg == LEFT_SIDE) ? "左" : "右";
        Sprintf(buf, "%s%sを怪我して", leftright, bp);
#endif

        /* when mounted, Wounded_legs applies to steed rather than to
           hero; we only report steed's wounded legs in wizard mode */
        if (u.usteed) { /* not `Riding' here */
            if (wizard && steedname) {
#if 0 /*JP*/
                char steednambuf[BUFSZ];

                Strcpy(steednambuf, steedname);
                *steednambuf = highc(*steednambuf);
                enl_msg(steednambuf, " has ", " had ", buf, "");
#else
                enl_msg(steedname, iru, ita, "は肢を怪我して", "");
#endif
            }
        } else {
            you_are_ing(buf, "");
        }
    }
    if (Glib) {
#if 0 /*JP:T*/
        Sprintf(buf, "slippery %s", fingers_or_gloves(TRUE));
        if (wizard)
            Sprintf(eos(buf), " (%ld)", (Glib & TIMEOUT));
        you_have(buf, "");
#else
        Sprintf(buf, "%sがぬるぬるして", fingers_or_gloves(TRUE));
        if (wizard)
            Sprintf(eos(buf), " (%ld)", (Glib & TIMEOUT));
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
#if 0 /*JP:T*/
            enl_msg(You_, "hunger", "hungered", " rapidly",
                    from_what(HUNGER));
#else
            enl_msg("あなたはすぐに腹が減る状態", "である", "だった", "", "");
#endif
    }
    Strcpy(buf, hu_stat[u.uhs]); /* hunger status; omitted if "normal" */
    mungspaces(buf);             /* strip trailing spaces */
    /* status line doesn't show hunger when state is "not hungry", we do;
       needed for wizard mode's reveal of u.uhunger but add it for everyone */
    if (!*buf)
/*JP
        Strcpy(buf, "not hungry");
*/
        Strcpy(buf, "空腹でない");
    if (*buf) { /* (since "not hungry" was added, this will always be True) */
#if 0 /*JP*/
        *buf = lowc(*buf); /* override capitalization */
        if (!strcmp(buf, "weak"))
            Strcat(buf, " from severe hunger");
        else if (!strncmp(buf, "faint", 5)) /* fainting, fainted */
            Strcat(buf, " due to starvation");
#else
        Strcat(buf, "状態");
#endif
        if (wizard)
            Sprintf(eos(buf), " <%d>", u.uhunger);
        you_are(buf, "");
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
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        Sprintf(eos(buf), "; movement %s %s%s", !final ? "is" : "was", adj,
                (cap < OVERLOADED) ? " slowed" : "");
#else
        Sprintf(buf, "荷物によって%s状態", enc_stat[cap]);
#endif
        you_are(buf, "");
    } else {
        /* last resort entry, guarantees Status section is non-empty
           (no longer needed for that purpose since weapon status added;
           still useful though) */
/*JP
        Strcpy(buf, "unencumbered");
*/
        Strcpy(buf, "荷物は邪魔にならない状態");
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        you_are(buf, "");
    }
    /* current weapon(s) and corresponding skill level(s) */
    weapon_insight(final);
    /* unlike ring of increase accuracy's effect, the monk's suit penalty
       is too blatant to be restricted to magical enlightenment */
    if (iflags.tux_penalty && !Upolyd) {
/*JP
        (void) enlght_combatinc("to hit", -gu.urole.spelarmr, final, buf);
*/
        (void) enlght_combatinc("命中率", -gu.urole.spelarmr, final, buf);
        /* if from_what() ever gets extended from wizard mode to normal
           play, it could be adapted to handle this */
        Sprintf(eos(buf), " due to your %s", suit_simple_name(uarm));
        you_have(buf, "");
    }
    /* report 'nudity' */
    if (!uarm && !uarmu && !uarmc && !uarms && !uarmg && !uarmf && !uarmh) {
        if (u.uroleplay.nudist)
#if 0 /*JP:T*/
            enl_msg(You_, "do", "did", " not wear any armor", "");
#else
            enl_msg(You_, "い", "かった", "何の鎧も装備しな", "");
#endif
        else
#if 0 /*JP:T*/
            you_are("not wearing any armor", "");
#else
            enl_msg(You_, "い", "かった", "何の鎧も装備していな", "");
#endif
    }
}

/* extracted from status_enlightenment() to reduce clutter there */
staticfn void
weapon_insight(int final)
{
    char buf[BUFSZ];
    int wtype;

    /* report being weaponless; distinguish whether gloves are worn
       [perhaps mention silver ring(s) when not wearing gloves?] */
    if (!uwep) {
#if 0 /*JP*/
        you_are(empty_handed(), "");
#else
        enl_msg(You_, "い", "かった", empty_handed(), "");
#endif

    /* two-weaponing implies hands and
       a weapon or wep-tool (not other odd stuff) in each hand */
    } else if (u.twoweap) {
/*JP
        you_are("wielding two weapons at once", "");
*/
        you_are("二刀流", "");

    /* report most weapons by their skill class (so a katana will be
       described as a long sword, for instance; mattock, hook, and aklys
       are exceptions), or wielded non-weapon item by its object class */
    } else {
        const char *what = weapon_descr(uwep);

        /* [what about other silver items?] */
        if (uwep->otyp == SHIELD_OF_REFLECTION)
            what = shield_simple_name(uwep); /* silver|smooth shield */
        else if (is_wet_towel(uwep))
            what = /* (uwep->spe < 3) ? "moist towel" : */ "wet towel";

#if 0 /*JP*/
        if (!strcmpi(what, "armor") || !strcmpi(what, "food")
            || !strcmpi(what, "venom"))
            Sprintf(buf, "wielding some %s", what);
        else
            /* [maybe include known blessed?] */
            Sprintf(buf, "wielding %s",
                    (uwep->quan == 1L) ? an(what) : makeplural(what));
        you_are(buf, "");
#else
        Sprintf(buf, "%sを装備して", what);
        enl_msg(You_, "いる", "いた", buf, "");
#endif
    }

    /*
     * Skill with current weapon.  Might help players who've never
     * noticed #enhance or decided that it was pointless.
     */
    if ((wtype = weapon_type(uwep)) != P_NONE && (!uwep || !is_ammo(uwep))) {
        char sklvlbuf[20];
        int sklvl = P_SKILL(wtype);
        boolean hav = (sklvl != P_UNSKILLED && sklvl != P_SKILLED);

        if (sklvl == P_ISRESTRICTED)
/*JP
            Strcpy(sklvlbuf, "no");
*/
            Strcpy(sklvlbuf, "制限");
        else
            (void) lcase(skill_level_name(wtype, sklvlbuf));
        /* "you have no/basic/expert/master/grand-master skill with <skill>"
           or "you are unskilled/skilled in <skill>" */
#if 0 /*JP:T*/
        Sprintf(buf, "%s %s %s", sklvlbuf,
                hav ? "skill with" : "in", skill_name(wtype));
#else
        Sprintf(buf, "%sの%sスキル", skill_name(wtype), sklvlbuf);
#endif

        if (!u.twoweap) {
            if (can_advance(wtype, FALSE))
#if 0 /*JP:T*/
                Sprintf(eos(buf), " and %s that",
                        !final ? "can enhance" : "could have enhanced");
#else
            Sprintf(eos(buf), "(高めることができ%s)",
                    !final ? "る" : "た");
#endif
            if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

        } else { /* two-weapon */
#if 0 /*JP*/
            static const char also_[] = "also ";
#endif
            char pfx[QBUFSZ], sfx[QBUFSZ],
                sknambuf2[20], sklvlbuf2[20], twobuf[20];
            const char *also = "", *also2 = "", *also3 = (char *) 0,
                       *verb_present, *verb_past;
            int wtype2 = weapon_type(uswapwep),
                sklvl2 = P_SKILL(wtype2),
                twoskl = P_SKILL(P_TWO_WEAPON_COMBAT);
            boolean a1, a2, ab,
                    hav2 = (sklvl2 != P_UNSKILLED && sklvl2 != P_SKILLED);

            /* normally hero must have access to two-weapon skill in
               order to initiate u.twoweap, but not if polymorphed into
               a form which has multiple weapon attacks, so we need to
               avoid getting bitten by unexpected skill value */
            if (twoskl == P_ISRESTRICTED) {
                twoskl = P_UNSKILLED;
                /* restricted is the same as unskilled as far as bonus
                   or penalty goes, and it isn't ordinarily seen so
                   skill_level_name() returns "Unknown" for it */
#if 0 /*JP*/
                Strcpy(twobuf, "restricted");
#else
                Strcpy(twobuf, "制限");
#endif
            } else {
                (void) lcase(skill_level_name(P_TWO_WEAPON_COMBAT, twobuf));
            }

            /* keep buf[] from above in case skill levels match */
            pfx[0] = sfx[0] = '\0';
            if (twoskl < sklvl) {
                /* twoskil won't be restricted so sklvl is at least basic */
#if 0 /*JP*/
                Sprintf(pfx, "Your skill in %s ", skill_name(wtype));
                Sprintf(sfx, " limited by being %s with two weapons", twobuf);
                also = also_;
#else
                Sprintf(pfx, "あなたの");
                Sprintf(sfx, "%sスキルは二刀流の%sによって",
                        skill_name(wtype), twobuf);
#endif
            } else if (twoskl > sklvl) {
                /* sklvl might be restricted */
#if 0 /*JP*/
                Strcpy(pfx, "Your two weapon skill ");
                Strcpy(sfx, " limited by ");
                if (sklvl > P_ISRESTRICTED)
                    Sprintf(eos(sfx), "being %s", sklvlbuf);
                else
                    Sprintf(eos(sfx), "having no skill");
                Sprintf(eos(sfx), " with %s", skill_name(wtype));
                also2 = also_;
#else
                Strcpy(pfx, "あなたの");
                Sprintf(sfx, "二刀流のスキルは%sの%sによって",
                        skill_name(wtype),
                        (sklvl > P_ISRESTRICTED) ? sklvlbuf : "制限");
#endif
            } else {
#if 0 /*JP*/
                Strcat(buf, " and two weapons");
                also3 = also_;
#else
                Sprintf(eos(buf), "と二刀流の%sスキル", sklvlbuf);
                also3 = "";
#endif
            }
            if (*pfx)
#if 0 /*JP*/
                enl_msg(pfx, "is", "was", sfx, "");
#else
                enl_msg(pfx, "制限されている", "制限されていた", sfx, "");
#endif
            else if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

            /* skip comparison between secondary and two-weapons if it is
               identical to the comparison between primary and twoweap */
            if (wtype2 != wtype) {
                Strcpy(sknambuf2, skill_name(wtype2));
                (void) lcase(skill_level_name(wtype2, sklvlbuf2));
#if 0 /*JP*/
                verb_present = "is", verb_past = "was";
#else
                verb_present = "制限されている", verb_past = "制限されていた";
#endif
                pfx[0] = sfx[0] = buf[0] = '\0';
                if (twoskl < sklvl2) {
                    /* twoskil is at least unskilled, sklvl2 at least basic */
#if 0 /*JP*/
                    Sprintf(pfx, "Your skill in %s ", sknambuf2);
                    Sprintf(sfx, " %slimited by being %s with two weapons",
                            also, twobuf);
#else
                    Sprintf(pfx, "あなたの");
                    Sprintf(sfx, "%sスキルは二刀流の%sによって",
                            sknambuf2, twobuf);
#endif
                } else if (twoskl > sklvl2) {
                    /* sklvl2 might be restricted */
#if 0 /*JP*/
                    Strcpy(pfx, "Your two weapon skill ");
                    Sprintf(sfx, " %slimited by ", also2);
                    if (sklvl2 > P_ISRESTRICTED)
                        Sprintf(eos(sfx), "being %s", sklvlbuf2);
                    else
                        Strcat(eos(sfx), "having no skill");
                    Sprintf(eos(sfx), " with %s", sknambuf2);
#else
                    Strcpy(pfx, "あなたの");
                    Sprintf(sfx, "二刀流のスキルは%sの%sによって",
                            sknambuf2,
                            (sklvl2 > P_ISRESTRICTED) ? sklvlbuf2 : "制限");
#endif
                } else {
                    /* equal; two-weapon is at least unskilled, so sklvl2 is
                       too; "you [also] have basic/expert/master/grand-master
                       skill with <skill>" or "you [also] are unskilled/
                       skilled in <skill> */
#if 0 /*JP*/
                    Sprintf(buf, "%s %s %s", sklvlbuf2,
                            hav2 ? "skill with" : "in", sknambuf2);
                    Strcat(buf, " and two weapons");
#else
                    Sprintf(buf, "%sと二刀流の%sスキル",
                            sknambuf2, sklvlbuf2);
#endif
                    if (also3) {
#if 0 /*JP*/
                        Strcpy(pfx, "You also ");
                        Snprintf(sfx, sizeof(sfx), " %s", buf), buf[0] = '\0';
                        verb_present = hav2 ? "have" : "are";
                        verb_past = hav2 ? "had" : "were";
#else
                        Strcpy(pfx, "あなたも");
                        verb_present = hav2 ? "をもっている" : "である";
                        verb_past = hav2 ? "をもっていた" : "であった";
                        Snprintf(sfx, sizeof(sfx), "%s", buf);
                        buf[0] = '\0';
#endif
                    }
                }
                if (*pfx)
                    enl_msg(pfx, verb_present, verb_past, sfx, "");
                else if (hav2)
                    you_have(buf, "");
                else
                    you_are(buf, "");
            } /* wtype2 != wtype */

            /* if training and available skill credits already allow
               #enhance for any of primary, secondary, or two-weapon,
               tell the player; avoid attempting figure out whether
               spending skill credits enhancing one might make either
               or both of the others become ineligible for enhancement */
            a1 = can_advance(wtype, FALSE);
            a2 = (wtype2 != wtype) ? can_advance(wtype2, FALSE) : FALSE;
            ab = can_advance(P_TWO_WEAPON_COMBAT, FALSE);
            if (a1 || a2 || ab) {
                static const char also_wik_[] = " and also with ";

                /* for just one, the conditionals yield
                   1) "skill with <that one>"; for more than one:
                   2) "skills with <primary> and also with <secondary>" or
                   3) "skills with <primary> and also with two-weapons" or
                   4) "skills with <secondary> and also with two-weapons" or
                   5) "skills with <primary>, <secondary>, and two-weapons"
                   (no 'also's or extra 'with's for case 5); when primary
                   and secondary use the same skill, only cases 1 and 3 are
                   possible because 'a2' gets forced to False above */
#if 0 /*JP*/
                Sprintf(sfx, " skill%s with %s%s%s%s%s",
                        ((int) a1 + (int) a2 + (int) ab > 1) ? "s" : "",
                        a1 ? skill_name(wtype) : "",
                        ((a1 && a2 && ab) ? ", "
                         : (a1 && (a2 || ab)) ? also_wik_ : ""),
                        a2 ? skill_name(wtype2) : "",
                        ((a1 && a2 && ab) ? ", and "
                         : (a2 && ab) ? also_wik_ : ""),
                        ab ? "two weapons" : "");
#else
                Sprintf(sfx, "%s%s%s%s%sのスキルを高めることが",
                        a1 ? skill_name(wtype) : "",
                        (a1 && a2) ? "と" : "",
                        a2 ? skill_name(wtype2) : "",
                        ((a1 || a2) && ab) ? "と" : "",
                        ab ? "二刀流" : "");
#endif
#if 0 /*JP:T*/
                enl_msg(You_, "can enhance", "could have enhanced", sfx, "");
#else
                enl_msg(You_, "できる", "できた", sfx, "");
#endif
            }
        } /* two-weapon */
    } /* skill applies */
}

staticfn void
item_resistance_message(
    int adtyp,
    const char *prot_message,
    int final)
{
    int protection = u_adtyp_resistance_obj(adtyp);

    if (protection) {
        boolean somewhat = protection < 99;

#if 0 /*JP:T*/
        enl_msg("Your items ",
                somewhat ? "are somewhat" : "are",
                somewhat ? "were somewhat" : "were",
                prot_message, item_what(adtyp));
#else
        enl_msg(somewhat ? "あなたの持ち物はある程度" : "あなたの持ち物は",
                "されている",
                "されていた",
                prot_message, item_what(adtyp));
#endif
    }
}

/* attributes: intrinsics and the like, other non-obvious capabilities */
staticfn void
attributes_enlightenment(
    int unused_mode UNUSED,
    int final)
{
#if 0 /*JP*/
    static NEARDATA const char
        if_surroundings_permitted[] = " if surroundings permitted";
#endif
    int ltmp, armpro, warnspecies;
    char buf[BUFSZ];

    /*\
     *  Attributes
    \*/
    enlght_out("");
/*JP
    enlght_out(final ? "Final Attributes:" : "Attributes:");
*/
    enlght_out(final ? "最終属性:" : "属性:");

    if (u.uevent.uhand_of_elbereth) {
#if 0 /*JP:T*/
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
#if 0 /*JP:T*/
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
/*JP
    item_resistance_message(AD_FIRE, " protected from fire", final);
*/
    item_resistance_message(AD_FIRE, "火から保護", final);
    if (Cold_resistance)
/*JP
        you_are("cold resistant", from_what(COLD_RES));
*/
        you_have("寒さへの耐性", from_what(COLD_RES));
/*JP
    item_resistance_message(AD_COLD, " protected from cold", final);
*/
    item_resistance_message(AD_COLD, "寒さから保護", final);
    if (Sleep_resistance)
/*JP
        you_are("sleep resistant", from_what(SLEEP_RES));
*/
        you_have("眠りへの耐性", from_what(SLEEP_RES));
    if (Disint_resistance)
/*JP
        you_are("disintegration resistant", from_what(DISINT_RES));
*/
        you_have("粉砕への耐性", from_what(DISINT_RES));
/*JP
    item_resistance_message(AD_DISN, " protected from disintegration", final);
*/
    item_resistance_message(AD_DISN, "粉砕から保護", final);
    if (Shock_resistance)
/*JP
        you_are("shock resistant", from_what(SHOCK_RES));
*/
        you_have("電撃への耐性", from_what(SHOCK_RES));
#if 0 /*JP:T*/
    item_resistance_message(AD_ELEC, " protected from electric shocks",
                            final);
#else
    item_resistance_message(AD_ELEC, "電撃から保護",
                            final);
#endif
    if (Poison_resistance)
/*JP
        you_are("poison resistant", from_what(POISON_RES));
*/
        you_have("毒への耐性", from_what(POISON_RES));
    if (Acid_resistance) {
#if 0 /*JP:T*/
        Sprintf(buf, "%.20s%.30s",
                temp_resist(ACID_RES) ? "temporarily " : "",
                "acid resistant");
        you_are(buf, from_what(ACID_RES));
#else
        Sprintf(buf, "%s%s",
                temp_resist(ACID_RES) ? "一時的な" : "",
                "酸への耐性");
        you_have(buf, from_what(ACID_RES));
#endif
    }
/*JP
    item_resistance_message(AD_ACID, " protected from acid", final);
*/
    item_resistance_message(AD_ACID, "酸から保護", final);
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
    if (Stone_resistance) {
#if 0 /*JP:T*/
        Sprintf(buf, "%.20s%.30s",
                temp_resist(STONE_RES) ? "temporarily " : "",
                "petrification resistant");
        you_are(buf, from_what(STONE_RES));
#else
        Sprintf(buf, "%s%s",
                temp_resist(STONE_RES) ? "一時的な" : "",
                "石化への耐性");
        you_have(buf, from_what(STONE_RES));
#endif
    }
    if (Halluc_resistance)
#if 0 /*JP:T*/
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
    if ((HBlinded || EBlinded) && BBlinded) /* blind w/ blindness blocked */
#if 0 /*JP:T*/
        you_can("see", from_what(-BLINDED)); /* Eyes of the Overworld */
#else /*「超世界の目によって見ることができる」*/
        you_can("見ることが", from_what(-BLINDED)); /* Eyes of the Overworld */
#endif
    if (Blnd_resist && !Blind) /* skip if no eyes or blindfolded */
#if 0 /*JP:T*/
        you_are("not subject to light-induced blindness",
                from_what(BLND_RES));
#else
        you_have("光による盲目への耐性", from_what(BLND_RES));
#endif
    if (See_invisible) {
        if (!Blind)
/*JP
            enl_msg(You_, "see", "saw", " invisible", from_what(SEE_INVIS));
*/
            enl_msg("あなたは透明なものを見られ", "る", "た", "", from_what(SEE_INVIS));
        else if (!PermaBlind)
#if 0 /*JP:T*/
            enl_msg(You_, "will see", "would have seen",
                    " invisible when not blind", "");
#else
            enl_msg(You_, "る", "た",
                    "盲目でないときには透明なものを見られ", "");
#endif
        else
#if 0 /*JP:T*/
            enl_msg(You_, "would see", "would have seen",
                    " invisible if not blind", "");
#else
            enl_msg(You_, "る", "た",
                    "盲目でなければ透明なものを見られ", "");
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
    if (Warn_of_mon && svc.context.warntype.obj) {
#if 0 /*JP:T*/
        Sprintf(buf, "aware of the presence of %s",
                (svc.context.warntype.obj & M2_ORC) ? "orcs"
                : (svc.context.warntype.obj & M2_ELF) ? "elves"
                  : (svc.context.warntype.obj & M2_DEMON) ? "demons"
                    : something);
#else
        Sprintf(buf, "%sの存在を感じる能力",
                (svc.context.warntype.obj & M2_ORC) ? "オーク"
                : (svc.context.warntype.obj & M2_ELF) ? "エルフ"
                  : (svc.context.warntype.obj & M2_DEMON) ? "悪魔"
                    : something);
#endif
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Warn_of_mon && svc.context.warntype.polyd) {
#if 0 /*JP:T*/
        Sprintf(buf, "aware of the presence of %s",
                ((svc.context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF)) ? "humans and elves"
                    : (svc.context.warntype.polyd & M2_HUMAN) ? "humans"
                      : (svc.context.warntype.polyd & M2_ELF) ? "elves"
                        : (svc.context.warntype.polyd & M2_ORC) ? "orcs"
                          : (svc.context.warntype.polyd & M2_DEMON) ? "demons"
                            : "certain monsters");
        you_are(buf, "");
#else
        Sprintf(buf, "%sの存在を感じる能力",
                ((svc.context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF)) ? "人間とエルフ"
                    : (svc.context.warntype.polyd & M2_HUMAN) ? "人間"
                      : (svc.context.warntype.polyd & M2_ELF) ? "エルフ"
                        : (svc.context.warntype.polyd & M2_ORC) ? "オーク"
                          : (svc.context.warntype.polyd & M2_DEMON) ? "悪魔"
                            : "ある種の怪物");
        you_have(buf, "");
#endif
    }
    warnspecies =  svc.context.warntype.speciesidx;
    if (Warn_of_mon && ismnum(warnspecies)) {
#if 0 /*JP*/
        Sprintf(buf, "aware of the presence of %s",
                makeplural(mons[warnspecies].pmnames[NEUTRAL]));
        you_are(buf, from_what(WARN_OF_MON));
#else
        Sprintf(buf, "%sの存在を感じる能力",
                mons[warnspecies].pmnames[NEUTRAL]);
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
    if (Clairvoyant) {
/*JP
        you_are("clairvoyant", from_what(CLAIRVOYANT));
*/
        you_have("千里眼能力", from_what(CLAIRVOYANT));
    } else if ((HClairvoyant || EClairvoyant) && BClairvoyant) {
        Strcpy(buf, from_what(-CLAIRVOYANT));
#if 0 /*JP*/
        (void) strsubst(buf, " because of ", " if not for ");
        enl_msg(You_, "could be", "could have been", " clairvoyant", buf);
#else
        /*JP:「…によって」*/
        (void) strsubst(buf, "によって", "がなければ");
        you_have("千里眼能力", buf);
#endif
    }
    if (Infravision)
/*JP
        you_have("infravision", from_what(INFRAVISION));
*/
        you_have("赤外線が見える視覚", from_what(INFRAVISION));
    if (Detect_monsters) {
#if 0 /*JP:T*/
        Strcpy(buf, "sensing the presence of monsters");
        if (wizard) {
            long detectmon_timeout = (HDetect_monsters & TIMEOUT);

            if (detectmon_timeout)
                Sprintf(eos(buf), " (%ld)", detectmon_timeout);
        }
        you_are(buf, "");
#else
        Strcpy(buf, "怪物を探す能力");
        if (wizard) {
            long detectmon_timeout = (HDetect_monsters & TIMEOUT);

            if (detectmon_timeout)
                Sprintf(eos(buf), " (%ld)", detectmon_timeout);
        }
        you_have(buf, "");
#endif
    }
    if (u.umconf) { /* 'u.umconf' is a counter rather than a timeout */
#if 0 /*JP:T*/
        Strcpy(buf, " monsters when hitting them");
        if (wizard && !final) {
            if (u.umconf == 1)
                Strcat(buf, " (next hit only)");
            else /* u.umconf > 1 */
                Sprintf(eos(buf), " (next %u hits)", u.umconf);
        }
        enl_msg(You_, "will confuse", "would have confused", buf, "");
#else
        Strcpy(buf, "怪物を攻撃したとき混乱させる能力");
        if (wizard && !final) {
            if (u.umconf == 1)
                Strcat(buf, "(次の攻撃のみ)");
            else /* u.umconf > 1 */
                Sprintf(eos(buf), "(次の%u回の攻撃)", u.umconf);
        }
        you_have(buf, "");
#endif
    }

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
        Sprintf(buf, "魅力%s",
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
    if (Stealth) {
/*JP
        you_are("stealthy", from_what(STEALTH));
*/
        you_have("人目を盗む能力", from_what(STEALTH));
    } else if (BStealth && (HStealth || EStealth)) {
#if 0 /*JP:T*/
        Sprintf(buf, " stealthy%s",
                (BStealth == FROMOUTSIDE) ? " if not mounted" : "");
        enl_msg(You_, "would be", "would have been", buf, "");
#else
        Sprintf(buf, "%s人目を盗む能力",
                (BStealth == FROMOUTSIDE) ? "騎乗していなければ" : "");
        you_have(buf, "");
#endif
    }
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
        if (Levitation) {
            /* either trapped in the floor or inside solid rock
               (or both if chained to buried iron ball and have
               moved one step into solid rock somehow) */
#if 0 /*JP*/
            boolean trapped = (save_BLev & I_SPECIAL) != 0L,
                    terrain = (save_BLev & FROMOUTSIDE) != 0L;

            Sprintf(buf, "%s%s%s",
                    trapped ? " if not trapped" : "",
                    (trapped && terrain) ? " and" : "",
                    terrain ? if_surroundings_permitted : "");
            enl_msg(You_, "would levitate", "would have levitated", buf, "");
#else
            you_are("状況が許せば浮遊する状態", "");
#endif
        }
        BLevitation = save_BLev;
    }
    /* actively flying handled earlier as a status condition */
    if (BFlying) { /* flight is blocked */
        long save_BFly = BFlying;

        BFlying = 0L;
        if (Flying) {
#if 0 /*JP:T*/
            enl_msg(You_, "would fly", "would have flown",
                    /* wording quibble: for past tense, "hadn't been"
                       would sound better than "weren't" (and
                       "had permitted" better than "permitted"), but
                       "weren't" and "permitted" are adequate so the
                       extra complexity to handle that isn't worth it */
                    Levitation
                       ? " if you weren't levitating"
                       : (save_BFly == I_SPECIAL)
                          /* this is an oversimplification; being trapped
                             might also be blocking levitation so flight
                             would still be blocked after escaping trap */
                          ? " if you weren't trapped"
                          : (save_BFly == FROMOUTSIDE)
                             ? if_surroundings_permitted
                             /* two or more of levitation, surroundings,
                                and being trapped in the floor */
                             : " if circumstances permitted",
                    "");
#else
            enl_msg(You_, "飛ぶことができる", "飛ぶことができた",
                    /* wording quibble: for past tense, "hadn't been"
                       would sound better than "weren't" (and
                       "had permitted" better than "permitted"), but
                       "weren't" and "permitted" are adequate so the
                       extra complexity to handle that isn't worth it */
                    Levitation
                       ? "浮遊していなければ"
                       : (save_BFly == I_SPECIAL)
                          /* this is an oversimplification; being trapped
                             might also be blocking levitation so flight
                             would still be blocked after escaping trap */
                          ? "捕まっていなければ"
                          : (save_BFly == FROMOUTSIDE)
                             ? "状況が許せば"
                             /* two or more of levitation, surroundings,
                                and being trapped in the floor */
                             : "事情が許せば",
                    "");
#endif
        }
        BFlying = save_BFly;
    }
    /* including this might bring attention to the fact that ceiling
       clinging has inconsistencies... */
    if (is_clinger(gy.youmonst.data)) {
        boolean has_lid = has_ceiling(&u.uz);

        if (has_lid && !u.uinwater) {
/*JP
            you_can("cling to the ceiling", "");
*/
            you_can("天井にぶら下がることが", "");
        } else {
#if 0 /*JP:T*/
            Sprintf(buf, " to the ceiling if %s%s%s",
                    !has_lid ? "there was one" : "",
                    (!has_lid && u.uinwater) ? " and " : "",
                    u.uinwater ? (Underwater ? "you weren't underwater"
                                  : "you weren't in the water") : "");
            /* past tense is applicable for death while Unchanging */
            enl_msg(You_, "could cling", "could have clung", buf, "");
#else
            Sprintf(buf, "%s%s%s天井にぶら下がることが",
                    !has_lid ? "天井があれば" : "",
                    (!has_lid && u.uinwater) ? "，かつ" : "",
                    u.uinwater ? (Underwater ? "水底にいなければ"
                                  : "水の中にいなければ") : "");
            /* past tense is applicable for death while Unchanging */
            you_can(buf, "");
#endif
        }
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
    if (u.uhitinc) {
/*JP
        (void) enlght_combatinc("to hit", u.uhitinc, final, buf);
*/
        (void) enlght_combatinc("命中率", u.uhitinc, final, buf);
#if 0 /*JP:T*/
        if (iflags.tux_penalty && !Upolyd)
            Sprintf(eos(buf), " %s your suit's penalty",
                    (u.uhitinc < 0) ? "increasing"
                    : (u.uhitinc < 4 * gu.urole.spelarmr / 5)
                      ? "partly offsetting"
                      : (u.uhitinc < gu.urole.spelarmr) ? "nearly offsetting"
                        : "overcoming");
        you_have(buf, "");
#else
        if (iflags.tux_penalty && !Upolyd)
            Sprintf(eos(buf), "は鎧によるペナルティを%sて",
                    (u.uhitinc < 0) ? "増やし"
                    : (u.uhitinc < 4 * gu.urole.spelarmr / 5)
                      ? "部分的に相殺し"
                      : (u.uhitinc < gu.urole.spelarmr) ? "ほぼ相殺し"
                        : "上回っ");
        enl_msg(buf, iru, ita, "", "");
#endif
    }
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
        if (uamul && uamul->otyp == AMULET_OF_GUARDING)
            prot += 2;
        if (HProtection & INTRINSIC)
            prot += u.ublessed;
        prot += u.uspellprot;
        if (prot)
/*JP
            you_have(enlght_combatinc("defense", prot, final, buf), "");
*/
            you_have(enlght_combatinc("防御", prot, final, buf), "");
    }
    if ((armpro = magic_negation(&gy.youmonst)) > 0) {
        /* magic cancellation factor, conferred by worn armor */
        static const char *const mc_types[] = {
#if 0 /*JP:T*/
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
    if (Half_gas_damage)
/*JP
        enl_msg(You_, "take", "took", " reduced poison gas damage", "");
*/
        you_can("毒ガスのダメージを減らすことが", "");
    if (spellid(0) > NO_SPELL) { /* skip if no spells are known yet */
        /* greatly simplified edition of percent_success(spell.c)--may need
           to be suppressed if oversimplification leads to player confusion */
        char cast_adj[QBUFSZ];
        boolean suit = uarm && is_metallic(uarm),
                robe = uarmc && uarmc->otyp == ROBE;

        *cast_adj = '\0';
#if 0 /*JP:T*/
        if (suit) /* omit "wearing" to shorten the text */
            Sprintf(cast_adj, " impaired by metallic armor%s",
                    robe ? ", mitigated by your robe" : "");
        else if (robe)
            Strcpy(cast_adj, " enhanced by wearing a robe");

        if (*cast_adj)
            enl_msg("Your spell casting ", "is", "was", cast_adj, "");
#else
        if (suit) /* omit "wearing" to shorten the text */
            Sprintf(cast_adj, "金属製鎧で弱体化され%sて",
                    robe ? "，ローブによって緩和され" : "");
        else if (robe)
            Strcpy(cast_adj, "ローブを着ることで強化されて");

        if (*cast_adj)
            enl_msg("あなたの呪文詠唱は", iru, ita, cast_adj, "");
#endif
    }
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
        else if (ismnum(u.ulycn))
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
        you_are_ing("定期的に変化して", from_what(POLYMORPH));
    }
    if (Polymorph_control)
/*JP
        you_have("polymorph control", from_what(POLYMORPH_CONTROL));
*/
        you_have("変化の制御能力", from_what(POLYMORPH_CONTROL));
    if (Upolyd && u.umonnum != u.ulycn
        /* if we've died from turning into slime, we're polymorphed
           right now but don't want to list it as a temporary attribute
           [we need a more reliable way to detect this situation] */
        && !(final == ENL_GAMEOVERDEAD
             && u.umonnum == PM_GREEN_SLIME && !Unchanging)) {
        /* foreign shape (except were-form which is handled below) */
        if (!vampshifted(&gy.youmonst))
#if 0 /*JP:T*/
            Sprintf(buf, "polymorphed into %s",
                    an(pmname(gy.youmonst.data,
                              flags.female ? FEMALE : MALE)));
#else
            Sprintf(buf, "%sに変化して",
                    pmname(gy.youmonst.data,
                              flags.female ? FEMALE : MALE));
#endif
        else
#if 0 /*JP:T*/
            Sprintf(buf, "polymorphed into %s in %s form",
                    an(pmname(&mons[gy.youmonst.cham],
                              flags.female ? FEMALE : MALE)),
                    pmname(gy.youmonst.data, flags.female ? FEMALE : MALE));
#else
            Sprintf(buf, "%sの形の%sに変化して",
                    pmname(gy.youmonst.data, flags.female ? FEMALE : MALE),
                    pmname(&mons[gy.youmonst.cham],
                              flags.female ? FEMALE : MALE));
#endif
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.mtimedone);
#if 0 /*JP:T*/
        you_are(buf, "");
#else
        you_are_ing(buf, "");
#endif
    }
    if (lays_eggs(gy.youmonst.data) && flags.female) /* Upolyd */
/*JP
        you_can("lay eggs", "");
*/
        you_can("卵を産むことが", "");
    if (ismnum(u.ulycn)) {
#if 0 /*JP*/
        /* "you are a werecreature [in beast form]" */
        Strcpy(buf, an(pmname(&mons[u.ulycn],
               flags.female ? FEMALE : MALE)));
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
        Strcat(buf, pmname(&mons[u.ulycn],
               flags.female ? FEMALE : MALE));
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
#if 0 /*JP:T*/
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
        ltmp = stone_luck(FALSE);
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
                              * this modifies the gf.ffruit chain, so could
                              * possibly mask or even introduce a problem,
                              * but it does useful sanity checking */
        for (f = gf.ffruit; f; f = f->nextf) {
/*JP
            Sprintf(buf, "Fruit #%d ", f->fid);
*/
            Sprintf(buf, "fruit #%d は", f->fid);
/*JP
            enl_msg(buf, "is ", "was ", f->fname, "");
*/
            enl_msg(buf, "だ", "だった", f->fname, "");
        }
#if 0 /*JP:T*/
        enl_msg("The current fruit ", "is ", "was ", svp.pl_fruit, "");
#else
        enl_msg("現在の fruit は", "だ", "だった", svp.pl_fruit, "");
#endif
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
            if (!u.umortality)
/*JP
                p = !final ? (char *) 0 : "survived";
*/
                p = !final ? (char *)0 : "生き延びた";
            else
                (void) N_times((long) u.umortality, buf);
        } else { /* game ended in character's death */
/*JP
            p = "are dead";
*/
            p = "死んでいる";
            switch (u.umortality) {
            case 0:
                impossible("dead without dying?");
                FALLTHROUGH;
                /* FALLTHRU */
            case 1:
                break; /* just "are dead" */
            default:
#if 0 /*JP:T*/
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

/* ^X command */
int
doattributes(void)
{
    int mode = BASICENLIGHTENMENT;

    /* show more--as if final disclosure--for wizard and explore modes */
    if (wizard || discover)
        mode |= MAGICENLIGHTENMENT;

    enlightenment(mode, ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

void
youhiding(boolean via_enlghtmt, /* enlightenment line vs topl message */
          int msgflag)          /* for variant message phrasing */
{
    char *bp, buf[BUFSZ];

#if 0 /*JP:T*/
    Strcpy(buf, "hiding");
#else
    buf[0] = '\0';
#endif
    if (U_AP_TYPE != M_AP_NOTHING) {
        /* mimic; hero is only able to mimic a strange object or gold
           or hallucinatory alternative to gold, so we skip the details
           for the hypothetical furniture and monster cases */
#if 0 /*JP*//*後ろに回す*//* not used */
        bp = eos(strcpy(buf, "mimicking"));
#endif
        if (U_AP_TYPE == M_AP_OBJECT) {
/*JP
            Sprintf(bp, " %s", an(simple_typename(gy.youmonst.mappearance)));
*/
            Strcpy(buf, simple_typename(gy.youmonst.mappearance));
        } else if (U_AP_TYPE == M_AP_FURNITURE) {
/*JP
            Strcpy(bp, " something");
*/
            Strcpy(buf, "何か");
        } else if (U_AP_TYPE == M_AP_MONSTER) {
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
#if 0 /*JP:T*/
        bp = eos(buf); /* points past "hiding" */
#else
        bp = buf; /* overwrite initial "hiding" text */
        *bp = '\0';
#endif
        if (gy.youmonst.data->mlet == S_EEL) {
            if (is_pool(u.ux, u.uy))
/*JP
                Sprintf(bp, " in the %s", waterbody_name(u.ux, u.uy));
*/
                Sprintf(bp, "%sの中に", waterbody_name(u.ux, u.uy));
        } else if (hides_under(gy.youmonst.data)) {
            struct obj *o = svl.level.objects[u.ux][u.uy];

            if (o)
/*JP
                Sprintf(bp, " underneath %s", ansimpleoname(o));
*/
                Sprintf(bp, "%sの下に", ansimpleoname(o));
        } else if (is_clinger(gy.youmonst.data) || Flying) {
            /* Flying: 'lurker above' hides on ceiling but doesn't cling */
/*JP
            Sprintf(bp, " on the %s", ceiling(u.ux, u.uy));
*/
            Sprintf(bp, "%sに", ceiling(u.ux, u.uy));
        } else {
            /* on floor; is_hider() but otherwise not special: 'trapper' */
            if (u.utrap && u.utraptype == TT_PIT) {
                struct trap *t = t_at(u.ux, u.uy);

#if 0 /*JP:T*/
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
        Strcat(bp, "隠れ");
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

/* #conduct command [KMH]; shares enlightenment's tense handling */
int
doconduct(void)
{
    show_conduct(ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

/* display conducts; for doconduct(), also disclose() and dump_everything() */
void
show_conduct(int final)
{
    char buf[BUFSZ], bufN[40];
    int ngenocided;

    /* Create the conduct window */
    ge.en_win = create_nhwindow(NHW_MENU);
/*JP
    putstr(ge.en_win, 0, "Voluntary challenges:");
*/
    putstr(ge.en_win, 0, "自発的挑戦:");

    /* rerolling; "You <this or that>" is about the character, rerolling
       is about the player so phrase it differently;
       also, always use past tense since the chance to do something with it
       is gone by time player can issue #conduct command or see disclosure */
    if (!u.uroleplay.reroll)
/*JP
        Strcpy(buf, " Character rerolling was not enabled.");
*/
        Strcpy(buf, " キャラクター再作成は無効．");
    else if (!u.uroleplay.numrerolls)
/*JP
        Strcpy(buf, " Your character was not rerolled.");
*/
        Strcpy(buf, " キャラクター再作成は行われていません.");
    else
#if 0 /*JP:T*/
        Sprintf(buf, " Your character was rerolled %s.",
                N_times(u.uroleplay.numrerolls, bufN));
#else
        Sprintf(buf, " キャラクター再作成は%s行われました．",
                N_times(u.uroleplay.numrerolls, bufN));
#endif
    enlght_out(buf);

    if (u.uroleplay.blind)
/*JP
        you_have_been("blind from birth");
*/
        you_have_been("生まれながらに盲目");
    if (u.uroleplay.deaf)
/*JP
        you_have_been("deaf from birth");
*/
        you_have_been("生まれながらに耳が聞こえない");
    /* note: we don't report "you are without possessions" unless the
       game started with the pauper option set */
    if (u.uroleplay.pauper)
#if 0 /*JP:T*/
        enl_msg(You_, gi.invent ? "started" : "are", "started out",
                " without possessions", "");
#else
        enl_msg(You_, gi.invent ? "で開始した" : "だ", "で開始した",
                "所持品なしで", "");
#endif
    /* nudist is far more than a subset of possessionless, and a much
       more impressive accomplishment, but showing "started out without
       possessions" before "faithfully nudist" looks more logical */
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
#if 0 /*JP:T*/
        Sprintf(buf, "hit with a wielded weapon %ld time%s",
                u.uconduct.weaphit, plur(u.uconduct.weaphit));
#else
        Sprintf(buf, "あなたは%ld回装備した武器を使用し", u.uconduct.weaphit);
#endif
        you_have_X(buf);
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

    if (!u.uconduct.pets)
/*JP
        you_have_never("had a pet");
*/
        enl_msg("ペットを飼ったことがな", "い", "かった", "", "");

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
#if 0 /*JP:T*/
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
#if 0 /*JP*/
            if (u.uconduct.wisharti == u.uconduct.wishes)
                Sprintf(eos(buf), " (%s",
                        (u.uconduct.wisharti > 2L) ? "all "
                          : (u.uconduct.wisharti == 2L) ? "both " : "");
            else
                Sprintf(eos(buf), " (%ld ", u.uconduct.wisharti);

            Sprintf(eos(buf), "for %s)",
                    (u.uconduct.wisharti == 1L) ? "an artifact"
                                                : "artifacts");
#else
            Sprintf(eos(buf), " (聖器は%ld回)", u.uconduct.wisharti);
#endif
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

    /* only report Sokoban conduct if the Sokoban branch has been entered */
    if (sokoban_in_play()) {
#if 0 /*JP:T*/
        const char *presentverb = "have violated", *pastverb = "violated";

        if (!u.uconduct.sokocheat) {
            presentverb = "have not violated";
            pastverb = "did not violate";
            Strcpy(buf, " any of the special Sokoban rules");
        } else {
            Strcpy(buf, " the special Sokoban rules ");
            Strcat(buf, N_times(u.uconduct.sokocheat, bufN));
        }
#else
        const char *presentverb = "反した", *pastverb = "反していた";

        if (!u.uconduct.sokocheat) {
            presentverb = "反していない";
            pastverb = "反していなかった";
            Strcpy(buf, "倉庫番の特殊ルールに");
        } else {
            Sprintf(buf, "倉庫番の特殊ルールに%s",
                N_times(u.uconduct.sokocheat, bufN));
        }
#endif
        enl_msg(You_, presentverb, pastverb, buf, "");
    }

    show_achievements(final);

    /* Pop up the window and wait for a key */
    display_nhwindow(ge.en_win, TRUE);
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*
 *      Achievements (see 'enum achievements' in you.h).
 */

staticfn void
show_achievements(
    int final) /* 'final' is used "behind the curtain" by enl_foo() macros */
{
    int i, achidx, absidx, acnt;
    char title[QBUFSZ], buf[QBUFSZ];
    winid awin = WIN_ERR;

    /* unfortunately we can't show the achievements (at least not all of
       them) while the game is in progress because it would give away the
       ID of luckstone (at Mine's End) and of real Amulet of Yendor */
    if (!final && !wizard)
        return;

    /* first, figure whether any achievements have been accomplished
       so that we don't show the header for them if the resulting list
       below it would be empty */
    if ((acnt = count_achievements()) == 0)
        return;

    if (ge.en_win != WIN_ERR) {
        awin = ge.en_win; /* end of game disclosure window */
        putstr(awin, 0, "");
    } else {
        awin = create_nhwindow(NHW_MENU);
    }
#if 0 /*JP:T*/
    Sprintf(title, "Achievement%s:", plur(acnt));
    putstr(awin, 0, title);
#else
    putstr(awin, 0, "実績：");
#endif

    /* display achievements in the order in which they were recorded;
       lone exception is to defer the Amulet if we just ascended;
       it warrants alternate wording when given away during ascension,
       but the Amulet achievement is always attained before entering
       endgame and the alternate wording looks strange if shown before
       "reached endgame" and "reached Astral" */
    if (remove_achievement(ACH_UWIN)) { /* UWIN == Ascended! */
        /* for ascension, force it to be last and Amulet next to last
           by taking them out and then adding them back */
        if (remove_achievement(ACH_AMUL)) /* should always be True here */
            record_achievement(ACH_AMUL);
        record_achievement(ACH_UWIN);
    }
    for (i = 0; i < acnt; ++i) {
        achidx = u.uachieved[i];
        absidx = abs(achidx);

        switch (absidx) {
        case ACH_BLND:
#if 0 /*JP:T*/
            enl_msg(You_, "are exploring", "explored",
                    " without being able to see", "");
#else
            you_have_X("物を見ることなく探索し");
#endif
            break;
        case ACH_NUDE:
#if 0 /*JP:T*/
            enl_msg(You_, "have gone", "went", " without any armor", "");
#else
            you_have_X("鎧なしで出発し");
#endif
            break;
        case ACH_MINE:
#if 0 /*JP:T*/
            you_have_X("entered the Gnomish Mines");
#else
            enlght_out("あなたはノームの鉱山に入った．");
#endif
            break;
        case ACH_TOWN:
#if 0 /*JP:T*/
            you_have_X("entered Minetown");
#else
            enlght_out("あなたはノームの街に入った．");
#endif
            break;
        case ACH_SHOP:
#if 0 /*JP:T*/
            you_have_X("entered a shop");
#else
            enlght_out("あなたは店に入った．");
#endif
            break;
        case ACH_TMPL:
#if 0 /*JP:T*/
            you_have_X("entered a temple");
#else
            enlght_out("あなたは寺院に入った．");
#endif
            break;
        case ACH_ORCL:
#if 0 /*JP:T*/
            you_have_X("consulted the Oracle of Delphi");
#else
            enlght_out("あなたはデルファイに相談した．");
#endif
            break;
        case ACH_NOVL:
#if 0 /*JP:T*/
            you_have_X("read from a Discworld novel");
#else
            enlght_out("あなたはディスクワールドの小説を読んだ．");
#endif
            break;
        case ACH_SOKO:
#if 0 /*JP:T*/
            you_have_X("entered Sokoban");
#else
            enlght_out("あなたは倉庫番に入った．");
#endif
            break;
        case ACH_SOKO_PRIZE: /* hard to reach guaranteed bag or amulet */
#if 0 /*JP:T*/
            you_have_X("completed Sokoban");
#else
            enlght_out("あなたは倉庫番をクリアした．");
#endif
            break;
        case ACH_MINE_PRIZE: /* hidden guaranteed luckstone */
#if 0 /*JP:T*/
            you_have_X("completed the Gnomish Mines");
#else
            enlght_out("あなたはノームの鉱山をクリアした．");
#endif
            break;
        case ACH_BGRM:
#if 0 /*JP:T*/
            you_have_X("entered the Big Room");
#else
            enlght_out("あなたは大きな部屋に入った．");
#endif
            break;
        case ACH_MEDU:
#if 0 /*JP:T*/
            you_have_X("defeated Medusa");
#else
            enlght_out("あなたはメデューサを倒した．");
#endif
            break;
        case ACH_TUNE:
#if 0 /*JP:T*/
            you_have_X(
                "learned the tune to open and close the Castle's drawbridge");
#else
            enlght_out("あなたは城の跳ね橋を上げ下げするための音程を学んだ．");
#endif
            break;
        case ACH_BELL:
            /* alternate phrasing for present vs past and also for
               possessing the item vs once held it */
#if 0 /*JP:T*/
            enl_msg(You_,
                    u.uhave.bell ? "have" : "have handled",
                    u.uhave.bell ? "had" : "handled",
                    " the Bell of Opening", "");
#else
            enlght_out("あなたは開放のベルを扱った．");
#endif
            break;
        case ACH_HELL:
#if 0 /*JP:T*/
            enl_msg(You_, "have ", "", "entered Gehennom", "");
#else
            enlght_out("あなたはゲヘナに入った．");
#endif
            break;
        case ACH_CNDL:
#if 0 /*JP:T*/
            enl_msg(You_,
                    u.uhave.menorah ? "have" : "have handled",
                    u.uhave.menorah ? "had" : "handled",
                    " the Candelabrum of Invocation", "");
#else
            enlght_out("あなたは祈りの燭台を扱った．");
#endif
            break;
        case ACH_BOOK:
#if 0 /*JP:T*/
            enl_msg(You_,
                    u.uhave.book ? "have" : "have handled",
                    u.uhave.book ? "had" : "handled",
                    " the Book of the Dead", "");
#else
            enlght_out("あなたは死者の書を扱った．");
#endif
            break;
        case ACH_INVK:
#if 0 /*JP:T*/
            you_have_X("gained access to Moloch's Sanctum");
#else
            enlght_out("あなたはモーロックの聖域に入った．");
#endif
            break;
        case ACH_AMUL:
            /* alternate wording for ascended (always past tense) since
               hero had it until #offer forced it to be relinquished */
#if 0 /*JP:T*/
            enl_msg(You_,
                    u.uhave.amulet ? "have" : "have obtained",
                    u.uevent.ascended ? "delivered"
                     : u.uhave.amulet ? "had" : "had obtained",
                    " the Amulet of Yendor", "");
#else
            enlght_out("あなたはイェンダーの魔除けを得た．");
#endif
            break;

        /* reaching Astral makes feedback about reaching the Planes
           be redundant and ascending makes both be redundant, but
           we display all that apply */
        case ACH_ENDG:
#if 0 /*JP:T*/
            you_have_X("reached the Elemental Planes");
#else
            enlght_out("あなたは精霊界に到達した．");
#endif
            break;
        case ACH_ASTR:
#if 0 /*JP:T*/
            you_have_X("reached the Astral Plane");
#else
            enlght_out("あなたは天上界に到達した．");
#endif
            break;
        case ACH_UWIN:
            /* the ultimate achievement... */
#if 0 /*JP:T*/
            enlght_out(" You ascended!");
#else
            enlght_out("あなたは昇天した！");
#endif
            break;

        /* rank 0 is the starting condition, not an achievement; 8 is Xp 30 */
        case ACH_RNK1: case ACH_RNK2: case ACH_RNK3: case ACH_RNK4:
        case ACH_RNK5: case ACH_RNK6: case ACH_RNK7: case ACH_RNK8:
#if 0 /*JP:T*/
            Sprintf(buf, "attained the rank of %s",
                    rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                            Role_switch, (achidx < 0) ? TRUE : FALSE));
            you_have_X(buf);
#else
            Sprintf(buf, "あなたは%sの称号を得た．",
                    rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                            Role_switch, (achidx < 0) ? TRUE : FALSE));
            enlght_out(buf);
#endif
            break;

        default:
#if 0 /*JP:T*/
            Sprintf(buf, " [Unexpected achievement #%d.]", achidx);
#else
            Sprintf(buf, " [不明な実績 #%d]", achidx);
#endif
            enlght_out(buf);
            break;
        } /* switch */
    } /* for */

    if (awin != ge.en_win) {
        display_nhwindow(awin, TRUE);
        destroy_nhwindow(awin);
    }
}

/* record an achievement (add at end of list unless already present) */
void
record_achievement(schar achidx)
{
    int i, absidx;
    int repeat_achievement = 0;

    absidx = abs(achidx);
    /* valid achievements range from 1 to N_ACH-1; however, ranks can be
       stored as the complement (ie, negative) to track gender */
    if ((achidx < 1 && (absidx < ACH_RNK1 || absidx > ACH_RNK8))
        || achidx >= N_ACH) {
        impossible("Achievement #%d is out of range.", achidx);
        return;
    }

    /* the list has an extra slot so there is always at least one 0 at
       its end (more than one unless all N_ACH-1 possible achievements
       have been recorded); find first empty slot or achievement #achidx;
       an attempt to duplicate an achievement can happen if any of Bell,
       Candelabrum, Book, or Amulet is dropped then picked up again */
    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == absidx) {
            repeat_achievement = 1;
            break;
        }

    /*
     * We do the sound for an achievement, even if it has already been
     * achieved before. Some players might have set up level-based
     * theme music or something. We do let the sound interface know
     * that it's not the original achievement though.
     */
    SoundAchievement(achidx, 0, repeat_achievement);

    if (repeat_achievement)
        return; /* already recorded, don't duplicate it */
    u.uachieved[i] = achidx;

    /* avoid livelog for achievements recorded during final disclosure:
       nudist and blind-from-birth; also ascension which is suppressed
       by this gets logged separately in really_done() */
    if (program_state.gameover)
        return;

    if (absidx >= ACH_RNK1 && absidx <= ACH_RNK8) {
        livelog_printf(achieve_msg[absidx].llflag,
                       "attained the rank of %s (level %d)",
                       rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                               Role_switch, (achidx < 0) ? TRUE : FALSE),
                       u.ulevel);
    } else if (achidx == ACH_SOKO_PRIZE
               || achidx == ACH_MINE_PRIZE) {
        /* need to supply extra information for these two */
        short otyp = ((achidx == ACH_SOKO_PRIZE)
                      ? svc.context.achieveo.soko_prize_otyp
                      : svc.context.achieveo.mines_prize_otyp);

        /* note: OBJ_NAME() works here because both "bag of holding" and
           "amulet of reflection" are fully named in their objects[] entry
           but that's not true in the general case */
        livelog_printf(achieve_msg[achidx].llflag, "%s %s",
                       achieve_msg[achidx].msg, OBJ_NAME(objects[otyp]));
    } else {
        livelog_printf(achieve_msg[absidx].llflag, "%s",
                       achieve_msg[absidx].msg);
    }
}

/* discard a recorded achievement; return True if removed, False otherwise */
boolean
remove_achievement(schar achidx)
{
    int i;

    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == abs(achidx))
            break; /* stop when found */
    if (!u.uachieved[i]) /* not found */
        return FALSE;
    /* list is 0 terminated so any beyond the removed one move up a slot */
    do {
        u.uachieved[i] = u.uachieved[i + 1];
    } while (u.uachieved[++i]);
    return TRUE;
}

/* used to decide whether there are any achievements to display */
int
count_achievements(void)
{
    int i, acnt = 0;

    for (i = 0; u.uachieved[i]; ++i)
        ++acnt;
    return acnt;
}

/* convert a rank index to an achievement number; encode it when female
   in order to subsequently report gender-specific ranks accurately */
schar
achieve_rank(int rank) /* 1..8 */
{
    schar achidx = (schar) ((rank - 1) + ACH_RNK1);

    if (flags.female)
        achidx = -achidx;
    return achidx;
}

/* return True if sokoban branch has been entered, False otherwise */
boolean
sokoban_in_play(void)
{
    int achidx;

    /* TODO? move this to dungeon.c and test furthest level reached of the
       sokoban branch instead of relying on the entered-sokoban achievement */

    for (achidx = 0; u.uachieved[achidx]; ++achidx)
        if (u.uachieved[achidx] == ACH_SOKO)
            return TRUE;
    return FALSE;
}

/* #chronicle command */
int
do_gamelog(void)
{
#ifdef CHRONICLE
    if (gg.gamelog) {
        show_gamelog(ENL_GAMEINPROGRESS);
    } else {
        pline("No chronicled events.");
    }
#else
    pline("Chronicle was turned off during compile-time.");
#endif /* !CHRONICLE */
    return ECMD_OK;
}

/* 'major' events for dumplog; inclusion or exclusion here may need tuning */
#define LL_majors (0L \
                   | LL_WISH            \
                   | LL_ACHIEVE         \
                   | LL_UMONST          \
                   | LL_DIVINEGIFT      \
                   | LL_LIFESAVE        \
                   | LL_ARTIFACT        \
                   | LL_GENOCIDE        \
                   | LL_DUMP) /* explicitly for dumplog */
#define majorevent(llmsg) (((llmsg)->flags & LL_majors) != 0)
#define spoilerevent(llmsg) (((llmsg)->flags & LL_SPOILER) != 0)

/* #chronicle details */
void
show_gamelog(int final)
{
#ifdef CHRONICLE
    struct gamelog_line *llmsg;
    winid win;
    char buf[BUFSZ];
    int eventcnt = 0;

    win = create_nhwindow(NHW_TEXT);
    Sprintf(buf, "%s events:", final ? "Major" : "Logged");
    putstr(win, 0, buf);
    for (llmsg = gg.gamelog; llmsg; llmsg = llmsg->next) {
        if (final && !majorevent(llmsg))
            continue;
        if (!final && !wizard && spoilerevent(llmsg))
            continue;
        if (!eventcnt++)
            putstr(win, 0, " Turn");
        Snprintf(buf, sizeof buf, "%5ld: %s", llmsg->turn, llmsg->text);
        putstr(win, 0, buf);
    }
    /* since start of game is logged as a major event, 'eventcnt' should
       never end up as 0; for 'final', end of game is a major event too */
    if (!eventcnt)
        putstr(win, 0, " none");

    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
#else
    nhUse(final);
#endif /* !CHRONICLE */
    return;
}

/*
 *      Vanquished monsters.
 */

/* the two uppercase choices are implemented but suppressed from menu.
   also used in options.c */
const char *const vanqorders[NUM_VANQ_ORDER_MODES][3] = {
    { "t", "traditional: by monster level",
           "traditional: by monster level, by internal monster index" },
    { "d", "by monster difficulty rating",
           "by monster difficulty rating, by internal monster index" },
    { "a", "alphabetically, unique monsters separate",
           "alphabetically, first unique monsters, then others" },
    { "A", "alphabetically, unique monsters intermixed",
           "alphabetically, unique monsters and others intermixed" },
    { "C", "by monster class, high to low level in class",
           "by monster class, high to low level within class" },
    { "c", "by monster class, low to high level in class",
           "by monster class, low to high level within class" },
    { "n", "by count, high to low",
           "by count, high to low, by internal index within tied count" },
    { "z", "by count, low to high",
           "by count, low to high, by internal index within tied count" },
};

staticfn int QSORTCALLBACK
vanqsort_cmp(
    const genericptr vptr1,
    const genericptr vptr2)
{
    int indx1 = *(short *) vptr1, indx2 = *(short *) vptr2,
        mlev1, mlev2, mstr1, mstr2, uniq1, uniq2, died1, died2, res;
    const char *name1, *name2, *punct;
    schar mcls1, mcls2;

    switch (flags.vanq_sortmode) {
    default:
    case VANQ_MLVL_MNDX:
        /* sort by monster level */
        mlev1 = mons[indx1].mlevel;
        mlev2 = mons[indx2].mlevel;
        res = mlev2 - mlev1; /* mlevel high to low */
        break;
    case VANQ_MSTR_MNDX:
        /* sort by monster toughness */
        mstr1 = mons[indx1].difficulty;
        mstr2 = mons[indx2].difficulty;
        res = mstr2 - mstr1; /* monstr high to low */
        break;
    case VANQ_ALPHA_SEP:
        uniq1 = ((mons[indx1].geno & G_UNIQ) && indx1 != PM_HIGH_CLERIC);
        uniq2 = ((mons[indx2].geno & G_UNIQ) && indx2 != PM_HIGH_CLERIC);
        if (uniq1 ^ uniq2) { /* one or other uniq, but not both */
            res = uniq2 - uniq1;
            break;
        } /* else both unique or neither unique */
        FALLTHROUGH;
        /*FALLTHRU*/
    case VANQ_ALPHA_MIX:
        name1 = mons[indx1].pmnames[NEUTRAL];
        name2 = mons[indx2].pmnames[NEUTRAL];
        res = strcmpi(name1, name2); /* caseblind alpha, low to high */
        break;
    case VANQ_MCLS_HTOL:
    case VANQ_MCLS_LTOH:
        /* mons[].mlet is a small integer, 1..N, of type plain char;
           if 'char' happens to be unsigned, (mlet1 - mlet2) would yield
           an inappropriate result when mlet2 is greater than mlet1,
           so force our copies (mcls1, mcls2) to be signed */
        mcls1 = (schar) mons[indx1].mlet;
        mcls2 = (schar) mons[indx2].mlet;
        /* S_ANT through S_ZRUTY correspond to lowercase monster classes,
           S_ANGEL through S_ZOMBIE correspond to uppercase, and various
           punctuation characters are used for classes beyond those */
        if (mcls1 > S_ZOMBIE && mcls2 > S_ZOMBIE) {
            /* force a specific order to the punctuation classes that's
               different from the internal order;
               internal order is ok if neither or just one is punctuation
               since letters have lower values so come out before punct */
            static const char punctclasses[] = {
                S_LIZARD, S_EEL, S_GOLEM, S_GHOST, S_DEMON, S_HUMAN, '\0'
            };

            if ((punct = strchr(punctclasses, mcls1)) != 0)
                mcls1 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
            if ((punct = strchr(punctclasses, mcls2)) != 0)
                mcls2 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
        }
        res = mcls1 - mcls2; /* class */
        if (res == 0) {
            /* Riders are in the same class as major demons, yielding res==0
               above when both mcls1 and mcls2 are either Riders or demons or
               one of each; force Riders to be sorted before demons */
            res = is_rider(&mons[indx2]) - is_rider(&mons[indx1]);
            /* res -1 => #1 is a Rider, #2 isn't;
                    0 => both Riders or neither;
                   +1 => #2 is a Rider, #1 isn't */
            if (res)
                break;
            mlev1 = mons[indx1].mlevel;
            mlev2 = mons[indx2].mlevel;
            res = mlev1 - mlev2; /* mlevel low to high */
            if (flags.vanq_sortmode == VANQ_MCLS_HTOL)
                res = -res; /* mlevel high to low */
        }
        break;
    case VANQ_COUNT_H_L:
    case VANQ_COUNT_L_H:
        died1 = svm.mvitals[indx1].died;
        died2 = svm.mvitals[indx2].died;
        res = died2 - died1; /* dead count high to low */
        if (flags.vanq_sortmode == VANQ_COUNT_L_H)
            res = -res; /* dead count low to high */
        break;
    }
    /* tiebreaker: internal mons[] index */
    if (res == 0)
        res = indx1 - indx2; /* mndx low to high */
    return res;
}

/* returns -1 if cancelled via ESC */
int
set_vanq_order(boolean for_vanq)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char buf[BUFSZ];
    const char *desc;
    int i, n, choice,
        clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */
    for (i = 0; i < SIZE(vanqorders); i++) {
        if (i == VANQ_ALPHA_MIX || i == VANQ_MCLS_HTOL) /* skip these */
            continue;
        /* suppress some orderings if this menu if for 'm #genocided' */
        if (!for_vanq && (i == VANQ_COUNT_H_L || i == VANQ_COUNT_L_H))
            continue;
        desc = vanqorders[i][2];
        /* unique monsters can't be genocided so "alpha, unique separate"
           and "alpha, unique intermixed" are confusing descriptions when
           this menu is for #genocided rather than for #vanquished */
        if (!for_vanq && i == VANQ_ALPHA_SEP)
/*JP
            desc = "alphabetically";
*/
            desc = "名前順";
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, *vanqorders[i][0], 0,
                 ATR_NONE, clr, desc,
                 (i == flags.vanq_sortmode) ? MENU_ITEMFLAGS_SELECTED
                                            : MENU_ITEMFLAGS_NONE);
    }
#if 0 /*JP:T*/
    Sprintf(buf, "Sort order for %s",
            for_vanq ? "vanquished monster counts (also genocided types)"
                     : "genocided monster types (also vanquished counts)");
#else
    Sprintf(buf, "%sのソート順",
            for_vanq ? "倒した敵の数(および虐殺した種類)"
                     : "虐殺した怪物の種類(および倒した数)");
#endif
    end_menu(tmpwin, buf);

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == flags.vanq_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        flags.vanq_sortmode = choice;
    }
    return (n < 0) ? -1 : flags.vanq_sortmode;
}

/* #vanquished command */
int
dovanquished(void)
{
    list_vanquished(iflags.menu_requested ? 'A' : 'y', FALSE);
    iflags.menu_requested = FALSE;
    return ECMD_OK;
}

/* high priests aren't unique but are flagged as such to simplify something */
#define UniqCritterIndx(mndx) \
    ((mons[mndx].geno & G_UNIQ) != 0 && mndx != PM_HIGH_CLERIC)

#define done_stopprint program_state.stopprint

/* used for #vanquished and end of game disclosure and end of game dumplog */
void
list_vanquished(char defquery, boolean ask)
{
    int i;
    int pfx, nkilled;
    unsigned ntypes, ni;
    long total_killed = 0L;
    winid klwin;
    short mindx[NUMMONS];
    char c, buf[BUFSZ], buftoo[BUFSZ];
    /* 'A' is only supplied by 'm #vanquished'; 'd' is only supplied by
       dump_everything() when writing dumplog, so won't happen if built
       without '#define DUMPLOG' but there's no need for conditionals here */
    boolean force_sort = (defquery == 'A'),
            dumping = (defquery == 'd');

    /* normally we don't ask about sort order for the vanquished list unless
       it contains at least two entries; however, if player has used explicit
       'm #vanquished', choose order no matter what it contains so far */
    if (force_sort) { /* iflags.menu_requested via dovanquished() */
        /* choose value for vanq_sortmode via menu; ESC cancels choosing
           sort order but continues with vanquishd monsters display */
        (void) set_vanq_order(TRUE);
    }
    if (dumping || force_sort) {
        /* switch from 'A' or 'd' to 'y'; 'ask' is already False for the
           cases that might supply 'A' or 'd' */
        defquery = 'y';
        ask = FALSE; /* redundant */
    }

    /* get totals first */
    ntypes = 0;
    for (i = LOW_PM; i < NUMMONS; i++) {
        if ((nkilled = (int) svm.mvitals[i].died) == 0)
            continue;
        mindx[ntypes++] = i;
        total_killed += (long) nkilled;
    }

    /* vanquished creatures list;
     * includes all dead monsters, not just those killed by the player
     */
    if (ntypes != 0) {
        char mlet, prev_mlet = 0; /* used as small integer, not character */
        boolean class_header, uniq_header, Rider,
                was_uniq = FALSE, special_hdr = FALSE;

        if (ask) {
            char allow_yn[10];

            if (ntypes > 1) {
                Strcpy(allow_yn, ynaqchars);
            } else {
                Strcpy(allow_yn, ynqchars); /* don't include 'a', but */
                Strcat(allow_yn, "\033a");  /* allow user to answer 'a' */
                if (defquery == 'a') /* potential default from 'disclose' */
                    defquery = 'y';
            }
#if 0 /*JP:T*/
            c = yn_function("Do you want an account of creatures vanquished?",
                            allow_yn, defquery, TRUE);
#else
            c = yn_function("倒した敵の一覧を見ますか？",
                            allow_yn, defquery, TRUE);
#endif
        } else {
            c = defquery;
        }

        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            if (c == 'a' && ntypes > 1) { /* ask user to choose sort order */
                /* choose value for vanq_sortmode via menu; ESC cancels list
                   of vanquished monsters but does not set 'done_stopprint' */
                if (set_vanq_order(TRUE) < 0)
                    return;
            }
            uniq_header = (flags.vanq_sortmode == VANQ_ALPHA_SEP);
            class_header = ((flags.vanq_sortmode == VANQ_MCLS_LTOH
                             || flags.vanq_sortmode == VANQ_MCLS_HTOL)
                            && ntypes > 1);

            klwin = create_nhwindow(NHW_MENU);
/*JP
            putstr(klwin, 0, "Vanquished creatures:");
*/
            putstr(klwin, 0, "倒した敵：");
            if (!dumping)
                putstr(klwin, 0, "");

            qsort((genericptr_t) mindx, ntypes, sizeof *mindx, vanqsort_cmp);
            for (ni = 0; ni < ntypes; ni++) {
                i = mindx[ni];
                nkilled = svm.mvitals[i].died;
                Rider = is_rider(&mons[i]);
                mlet = mons[i].mlet;
                if (class_header
                    && (mlet != prev_mlet || (special_hdr && !Rider))) {
                    if (!Rider) {
                        Strcpy(buf, def_monsyms[(int) mlet].explain);
                        special_hdr = FALSE;
                    } else {
/*JP
                        Strcpy(buf, "Rider");
*/
                        Strcpy(buf, "乗り手");
                        special_hdr = TRUE;
                    }
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                if (UniqCritterIndx(i)) {
#if 0 /*JP*/
                    Sprintf(buf, "%s%s",
                            !type_is_pname(&mons[i]) ? "the " : "",
                            mons[i].pmnames[NEUTRAL]);
#else
                    Sprintf(buf, "%s", mons[i].pmnames[NEUTRAL]);
#endif
                    if (nkilled > 1)
                        Sprintf(eos(buf), " (%s)",
                                N_times((long) nkilled, buftoo));
                    was_uniq = TRUE;
                } else {
                    if (uniq_header && was_uniq) {
                        putstr(klwin, 0, "");
                        was_uniq = FALSE;
                    }
                    /* trolls or undead might have come back,
                       but we don't keep track of that */
                    if (nkilled == 1)
                        Strcpy(buf, an(mons[i].pmnames[NEUTRAL]));
                    else
#if 0 /*JP:T*/
                        Sprintf(buf, "%3d %s", nkilled,
                                makeplural(mons[i].pmnames[NEUTRAL]));
#else
                        Sprintf(buf, "%d体の%s", nkilled,
                                mons[i].pmnames[NEUTRAL]);
#endif
                }
                /* number of leading spaces to match 3 digit prefix */
#if 0 /*JP*/
                pfx = !strncmpi(buf, "the ", 4) ? 0
                      : !strncmpi(buf, "an ", 3) ? 1
                        : !strncmpi(buf, "a ", 2) ? 2
                          : !digit(buf[2]) ? 4 : 0;
#else
                pfx = !digit(buf[2]) ? 4 : 0;
#endif
                if (class_header)
                    ++pfx;
                Snprintf(buftoo, sizeof buftoo, "%*s%s", pfx, "", buf);
                putstr(klwin, 0, buftoo);
            }
            /*
             * if (Hallucination)
             *     putstr(klwin, 0, "and a partridge in a pear tree");
             */
            if (ntypes > 1) {
                if (!dumping)
                    putstr(klwin, 0, "");
/*JP
                Sprintf(buf, "%ld creatures vanquished.", total_killed);
*/
                Sprintf(buf, "%ld匹の敵を倒した．", total_killed);
                putstr(klwin, 0, buf);
            }
            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /*
     * For end-of-game disclosure, we're only called when some monsters
     * were vanquished and won't reach these 'else-if's.
     *
     * If no monsters have been vanquished, we're either called for game
     * still in progress, so use present tense via pline(), or for dumplog
     * which needs putstr() and past tense.
     */
    } else if (!program_state.gameover) {
        /* #vanquished rather than final disclosure, so pline() is ok */
#if 0 /*JP:T*/
        pline("No creatures have been vanquished.");
#else
        pline("倒した敵はいなかった．");
#endif
#ifdef DUMPLOG
    } else if (dumping) {
        putstr(0, 0, "No creatures were vanquished."); /* not pline() */
#endif
    }
}

/* number of monster species which have been genocided */
int
num_genocides(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (svm.mvitals[i].mvflags & G_GENOD) {
            ++n;
            if (UniqCritterIndx(i))
                impossible("unique creature '%d: %s' genocided?",
                           i, mons[i].pmnames[NEUTRAL]);
        }
    }
    return n;
}

/* return a count of the number of extinct species */
staticfn int
num_extinct(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (UniqCritterIndx(i))
            continue;
        if ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT)
            ++n;
    }
    return n;
}

/* collect both genocides and extinctions, skipping uniques */
staticfn int
num_gone(int mvflags, int *mindx)
{
    uchar mflg = (uchar) mvflags;
    int i, n = 0;

    (void) memset((genericptr_t) mindx, 0, NUMMONS * sizeof *mindx);

    for (i = LOW_PM; i < NUMMONS; ++i) {
        /* uniques can't be genocided but can become extinct;
           however, they're never reported as extinct, so skip them */
        if (UniqCritterIndx(i))
            continue;

        if ((svm.mvitals[i].mvflags & mflg) != 0)
            mindx[n++] = i;
    }
    return n;
}

/* show genocided and extinct monster types for final disclosure/dumplog
   or for the #genocided command */
void
list_genocided(char defquery, boolean ask)
{
    int i, mndx;
    int ngenocided, nextinct, ngone, mvflags, mindx[NUMMONS];
    char c;
    winid klwin;
    char buf[BUFSZ];
    boolean genoing, /* prompting for genocide or class genocide */
            dumping; /* for DUMPLOG; doesn't need to be conditional */
    boolean both = (program_state.gameover || wizard || discover);

    dumping = (defquery == 'd');
    genoing = (defquery == 'g');
    if (dumping || genoing)
        defquery = 'y';
    if (genoing)
        both = FALSE; /* genocides only, not extinctions */

    /* this goes through the whole monster list up to three times but will
       happen rarely and is simpler than a more general single pass check;
       extinctions are only revealed during end of game disclosure or when
       running in wizard or explore mode */
    ngenocided = num_genocides();
    nextinct = both ? num_extinct() : 0;
    mvflags = G_GENOD | (both ? G_EXTINCT : 0);
    ngone = num_gone(mvflags, mindx);

    /* genocided or extinct species list */
    if (ngone > 0) {
#if 0 /*JP:T*/
        Sprintf(buf, "Do you want a list of %sspecies%s%s?",
                (nextinct && !ngenocided) ? "extinct " : "",
                (ngenocided) ? " genocided" : "",
                (nextinct && ngenocided) ? " and extinct" : "");
#else
        Sprintf(buf, "%s%s%sした種の一覧を見ますか？",
                (nextinct && !ngenocided) ? "絶滅" : "",
                (ngenocided) ? "虐殺" : "",
                (nextinct && ngenocided) ? "および絶滅" : "");
#endif
        c = ask ? yn_function(buf, (ngone > 1) ? "ynaq" : "ynq\033a",
                              defquery, TRUE)
                : defquery;
        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            int save_sortmode;
            char mlet, prev_mlet = 0;
            boolean class_header = FALSE;

            if (ngone > 1) {
                if (c == 'a') { /* ask player to choose sort order */
                    /* #genocided shares #vanquished's sort order */
                    if (set_vanq_order(FALSE) < 0)
                        return;
                }
                /* sort orderings count-high-to-low or count-low-to-high
                   don't make sense for genocides; if the preferred order
                   to set to either of those, use alphabetical instead;
                   note: the tie breaker for by-class is level-high-to-low
                   or level-low-to-high rather than count so is ok as-is */
                save_sortmode = flags.vanq_sortmode;
                if (flags.vanq_sortmode == VANQ_COUNT_H_L
                    || flags.vanq_sortmode == VANQ_COUNT_L_H)
                    flags.vanq_sortmode = VANQ_ALPHA_MIX;
                qsort((genericptr_t) mindx, ngone,
                      sizeof *mindx, vanqsort_cmp);
                class_header = (flags.vanq_sortmode == VANQ_MCLS_LTOH
                                || flags.vanq_sortmode == VANQ_MCLS_HTOL);
                flags.vanq_sortmode = save_sortmode;
            }

            klwin = create_nhwindow(NHW_MENU);
#if 0 /*JP:T*/
            Sprintf(buf, "%s%s species:",
                    (ngenocided) ? "Genocided" : "Extinct",
                    (nextinct && ngenocided) ? " or extinct" : "");
#else
            Sprintf(buf, "%s%sした種:",
                    (ngenocided) ? "虐殺" : "絶滅",
                    (nextinct && ngenocided) ? "または絶滅" : "");
#endif
            putstr(klwin, 0, buf);
            if (!dumping)
                putstr(klwin, 0, "");

            for (i = 0; i < ngone; ++i) {
                mndx = mindx[i];
                mlet = mons[mndx].mlet;
                if (class_header && mlet != prev_mlet) {
                    Strcpy(buf, def_monsyms[(int) mlet].explain);
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                Sprintf(buf, " %s", makeplural(mons[mndx].pmnames[NEUTRAL]));
                /*
                 * "Extinct" is unfortunate terminology.  A species
                 * is marked extinct when its birth limit is reached,
                 * but there might be members of the species still
                 * alive, contradicting the meaning of the word.
                 *
                 * We only append "(extinct)" if the G_GENOD bit is
                 * clear.  During normal play, 'mndx' won't be in the
                 * collected list unless that bit is set.
                 */
                if ((svm.mvitals[mndx].mvflags & G_GONE) == G_EXTINCT)
/*JP
                    Strcat(buf, " (extinct)");
*/
                    Strcat(buf, "(絶滅)");
                putstr(klwin, 0, buf);
            }
            if (!dumping)
                putstr(klwin, 0, "");
            if (ngenocided > 0) {
/*JP
                Sprintf(buf, "%d species genocided.", ngenocided);
*/
                Sprintf(buf, "%d種類の種を虐殺した．", ngenocided);
                putstr(klwin, 0, buf);
            }
            if (nextinct > 0) {
/*JP
                Sprintf(buf, "%d species extinct.", nextinct);
*/
                Sprintf(buf, "%d種類の種を絶滅させた．", nextinct);
                putstr(klwin, 0, buf);
            }

            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /* See the comment for similar code near the end of list_vanquished(). */
    } else if (!program_state.gameover) {
        /* #genocided rather than final disclosure, so pline() is ok and
           extinction has been ignored */
/*JP
        pline("No creatures have been genocided%s.", genoing ? " yet" : "");
*/
        pline("虐殺したり絶滅させたりした種は%sいなかった．", genoing ? "まだ" : "");
#ifdef DUMPLOG
    } else if (dumping) { /* 'gameover' is True if we make it here */
        putstr(0, 0, "No species were genocided or became extinct.");
#endif
    }
}

/* M-g - #genocided command */
int
dogenocided(void)
{
    list_genocided(iflags.menu_requested ? 'a' : 'y', FALSE);
    return ECMD_OK;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* #wizborn extended command */
int
doborn(void)
{
    static const char fmt[] = "%4i %4i %c %-30s";
    int i;
    winid datawin = create_nhwindow(NHW_TEXT);
    char buf[BUFSZ];
    int nborn = 0, ndied = 0;

    putstr(datawin, 0, "died born");
    for (i = LOW_PM; i < NUMMONS; i++)
        if (svm.mvitals[i].born || svm.mvitals[i].died
            || (svm.mvitals[i].mvflags & G_GONE) != 0) {
            Sprintf(buf, fmt,
                    svm.mvitals[i].died, svm.mvitals[i].born,
                    ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT) ? 'E'
                    : ((svm.mvitals[i].mvflags & G_GONE) == G_GENOD) ? 'G'
                      : ((svm.mvitals[i].mvflags & G_GONE) != 0) ? 'X'
                        : ' ',
                    mons[i].pmnames[NEUTRAL]);
            putstr(datawin, 0, buf);
            nborn += svm.mvitals[i].born;
            ndied += svm.mvitals[i].died;
        }

    putstr(datawin, 0, "");
    Sprintf(buf, fmt, ndied, nborn, ' ', "");

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);

    return ECMD_OK;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/*
 * align_str(), piousness(), mstatusline() and ustatusline() once resided
 * in pline.c, then got moved to priest.c just to be out of there.  They
 * fit better here.
 */

const char *
align_str(aligntyp alignment)
{
    switch ((int) alignment) {
    case A_CHAOTIC:
/*JP
        return "chaotic";
*/
        return "混沌";
    case A_NEUTRAL:
/*JP
        return "neutral";
*/
        return "中立";
    case A_LAWFUL:
/*JP
        return "lawful";
*/
        return "秩序";
    case A_NONE:
/*JP
        return "unaligned";
*/
        return "無心";
    }
/*JP
    return "unknown";
*/
    return "不明";
}

staticfn char *
size_str(int msize)
{
    static char outbuf[40];

    switch (msize) {
    case MZ_TINY:
        Strcpy(outbuf, "tiny");
        break;
    case MZ_SMALL:
        Strcpy(outbuf, "small");
        break;
    case MZ_MEDIUM:
        Strcpy(outbuf, "medium");
        break;
    case MZ_LARGE:
        Strcpy(outbuf, "large");
        break;
    case MZ_HUGE:
        Strcpy(outbuf, "huge");
        break;
    case MZ_GIGANTIC:
        Strcpy(outbuf, "gigantic");
        break;
    default:
        Sprintf(outbuf, "unknown size (%d)", msize);
        break;
    }
    return outbuf;
}

/* used for self-probing */
char *
piousness(boolean showneg, const char *suffix)
{
    static char buf[32]; /* bigger than "insufficiently neutral" */
    const char *pio;

    /* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20)
/*JP
        pio = "piously";
*/
        pio = "敬虔な";
    else if (u.ualign.record > 13)
/*JP
        pio = "devoutly";
*/
        pio = "信心深い";
    else if (u.ualign.record > 8)
/*JP
        pio = "fervently";
*/
        pio = "熱心な";
    else if (u.ualign.record > 3)
/*JP
        pio = "stridently";
*/
        pio = "大げさな";
    else if (u.ualign.record == 3)
        pio = "";
    else if (u.ualign.record > 0)
/*JP
        pio = "haltingly";
*/
        pio = "不完全な";
    else if (u.ualign.record == 0)
/*JP
        pio = "nominally";
*/
        pio = "形だけの";
    else if (!showneg)
/*JP
        pio = "insufficiently";
*/
        pio = "不十分な";
    else if (u.ualign.record >= -3)
/*JP
        pio = "strayed";
*/
        pio = "迷いを持った";
    else if (u.ualign.record >= -8)
/*JP
        pio = "sinned";
*/
        pio = "罪を負った";
    else
/*JP
        pio = "transgressed";
*/
        pio = "逸脱した";

    Sprintf(buf, "%s", pio);
    if (suffix && (!showneg || u.ualign.record >= 0)) {
#if 0 /*JP*/
        if (u.ualign.record != 3)
            Strcat(buf, " ");
#endif
        Strcat(buf, suffix);
    }
    return buf;
}

/* stethoscope or probing applied to monster -- one-line feedback */
void
mstatusline(struct monst *mtmp)
{
    aligntyp alignment = mon_aligntyp(mtmp);
    char info[BUFSZ], monnambuf[BUFSZ];

    info[0] = 0;
    if (mtmp->mtame) {
/*JP
        Strcat(info, ", tame");
*/
        Strcat(info, ", 飼いならされている");
        if (wizard) {
            Sprintf(eos(info), " (%d", mtmp->mtame);
            if (!mtmp->isminion)
                Sprintf(eos(info), "; hungry %ld; apport %d",
                        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
            Strcat(info, ")");
        }
    } else if (mtmp->mpeaceful)
/*JP
        Strcat(info, ", peaceful");
*/
        Strcat(info, ", 友好的");

    if (mtmp->data == &mons[PM_LONG_WORM]) {
        int segndx, nsegs = count_wsegs(mtmp);

        /* the worm code internals don't consider the head to be one of
           the worm's segments, but we count it as such when presenting
           worm feedback to the player */
        if (!nsegs) {
/*JP
            Strcat(info, ", single segment");
*/
            Strcat(info, ", 一つの節");
        } else {
            ++nsegs; /* include head in the segment count */
            segndx = wseg_at(mtmp, gb.bhitpos.x, gb.bhitpos.y);
#if 0 /*JP:T*/
            Sprintf(eos(info), ", %d%s of %d segments",
                    segndx, ordin(segndx), nsegs);
#else
            Sprintf(eos(info), ", %d節のうち%d番目",
                    nsegs, segndx);
#endif
        }
    }
    if (ismnum(mtmp->cham) && mtmp->data != &mons[mtmp->cham])
        /* don't reveal the innate form (chameleon, vampire, &c),
           just expose the fact that this current form isn't it */
/*JP
        Strcat(info, ", shapechanger");
*/
        Strcat(info, ", 変化");
    /* pets eating mimic corpses mimic while eating, so this comes first */
    if (mtmp->meating)
/*JP
        Strcat(info, ", eating");
*/
        Strcat(info, ", 食事中");
    /* a stethoscope exposes mimic before getting here so this
       won't be relevant for it, but wand of probing doesn't */
    if (mtmp->mundetected || mtmp->m_ap_type
        || visible_region_at(gb.bhitpos.x, gb.bhitpos.y))
        mhidden_description(mtmp,
                       MHID_PREFIX | MHID_ARTICLE | MHID_ALTMON | MHID_REGION,
                            eos(info));
    if (mtmp->mcan)
/*JP
        Strcat(info, ", cancelled");
*/
        Strcat(info, ", 無力");
    if (mtmp->mconf)
/*JP
        Strcat(info, ", confused");
*/
        Strcat(info, ", 混乱状態");
    if (mtmp->mblinded || !mtmp->mcansee)
/*JP
        Strcat(info, ", blind");
*/
        Strcat(info, ", 盲目");
    if (mtmp->mstun)
/*JP
        Strcat(info, ", stunned");
*/
        Strcat(info, ", くらくら状態");
    if (mtmp->msleeping)
/*JP
        Strcat(info, ", asleep");
*/
        Strcat(info, ", 睡眠状態");
#if 0 /* unfortunately mfrozen covers temporary sleep and being busy
       * (donning armor, for instance) as well as paralysis */
    else if (mtmp->mfrozen)
        Strcat(info, ", paralyzed");
#else
    else if (mtmp->mfrozen || !mtmp->mcanmove)
/*JP
        Strcat(info, ", can't move");
*/
        Strcat(info, ", 動けない");
#endif
    /* [arbitrary reason why it isn't moving] */
    else if ((mtmp->mstrategy & STRAT_WAITMASK) != 0)
/*JP
        Strcat(info, ", meditating");
*/
        Strcat(info, ", 冥想中");
    if (mtmp->mflee)
/*JP
        Strcat(info, ", scared");
*/
        Strcat(info, ", 怯えている");
    if (mtmp->mtrapped)
/*JP
        Strcat(info, ", trapped");
*/
        Strcat(info, ", 罠にかかっている");
    if (mtmp->mspeed)
#if 0 /*JP:T*/
        Strcat(info, (mtmp->mspeed == MFAST) ? ", fast"
                      : (mtmp->mspeed == MSLOW) ? ", slow"
                         : ", [? speed]");
#else
        Strcat(info, (mtmp->mspeed == MFAST) ? ", 素早い"
                      : (mtmp->mspeed == MSLOW) ? ", 遅い"
                         : ", [速度不明]");
#endif
    if (mtmp->minvis)
/*JP
        Strcat(info, ", invisible");
*/
        Strcat(info, ", 不可視");
    if (mtmp == u.ustuck) {
        struct permonst *pm = u.ustuck->data;

        /* being swallowed/engulfed takes priority over sticks(youmonst);
           this used to have that backwards and checked sticks() first */
#if 0 /*JP:T*/
        Strcat(info, u.uswallow ? (digests(pm)
                                   ? ", digesting you"
                                   /* note: the "swallowing you" case won't
                                      happen because all animal engulfers
                                      either digest their victims (purple
                                      worm) or enfold them (trappers and
                                      lurkers above) */
                                   : (is_animal(pm) && !enfolds(pm))
                                     ? ", swallowing you"
                                     : ", engulfing you")
                     /* !u.uswallow; if both youmonst and ustuck are holders,
                        youmonst wins */
                     : (!sticks(gy.youmonst.data) ? ", holding you"
                                                 : ", held by you"));
#else
        Strcat(info, u.uswallow ? (digests(pm)  ? ", 消化している"
                                   : (is_animal(pm) && !enfolds(pm))
                                     ? ", 飲み込んでいる"
                                     : ", 巻き込んでいる")
                     /* !u.uswallow; if both youmonst and ustuck are holders,
                        youmonst wins */
                     : (!sticks(gy.youmonst.data) ? ", 掴まえている"
                                                 : ", あなたが掴まえている"));
#endif
    }
    if (mtmp == u.usteed) {
/*JP
        Strcat(info, ", carrying you");
*/
        Strcat(info, ", あなたを乗せている");
        if (Wounded_legs) {
            /* EWounded_legs is used to track left/right/both rather than
               some form of extrinsic impairment; HWounded_legs is used for
               timeout; both apply to steed instead of hero when mounted */
            long legs = (EWounded_legs & BOTH_SIDES);
            const char *what = mbodypart(mtmp, LEG);

            if (legs == BOTH_SIDES)
                what = makeplural(what);
/*JP
            Sprintf(eos(info), ", injured %s", what);
*/
            Sprintf(eos(info), ", %sにけがをしている", what);
        }
    }
    if (mtmp->mleashed)
/*JP
        Strcat(info, ", leashed");
*/
        Strcat(info, ", つながれている");

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_YOUR, (char *) 0,
                               (SUPPRESS_IT | SUPPRESS_INVISIBLE), FALSE));

#if 0 /*JP:T*/
    pline("Status of %s (%s, %s):  Level %d  HP %d(%d)  AC %d%s.",
          monnambuf, align_str(alignment), size_str(mtmp->data->msize),
          mtmp->m_lev, mtmp->mhp, mtmp->mhpmax, find_mac(mtmp), info);
#else
    pline("%sの状態 (%s, %s)： Level %d  HP %d(%d)  AC %d%s",
          monnambuf, align_str(alignment), size_str(mtmp->data->msize),
          mtmp->m_lev, mtmp->mhp, mtmp->mhpmax, find_mac(mtmp), info);
#endif
}

/* stethoscope or probing applied to hero -- one-line feedback */
void
ustatusline(void)
{
    NhRegion *reg;
    char info[BUFSZ];
    size_t ln;

    info[0] = '\0';
    if (Sick) {
#if 0 /*JP*/
        Strcat(info, ", dying from");
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, " food poisoning");
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, " and");
            Strcat(info, " illness");
        }
#else
        Strcat(info, ", ");
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, "食中毒");
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, "と");
            Strcat(info, "病気");
        }
        Strcat(info, "で死につつある");
#endif
    }
    if (Stoned)
/*JP
        Strcat(info, ", solidifying");
*/
        Strcat(info, ", 石化しつつある");
    if (Slimed)
/*JP
        Strcat(info, ", becoming slimy");
*/
        Strcat(info, ", スライムになりつつある");
    if (Strangled)
/*JP
        Strcat(info, ", being strangled");
*/
        Strcat(info, ", 首を絞められている");
    if (Vomiting)
#if 0 /*JP:T*/
        Strcat(info, ", nauseated"); /* !"nauseous" */
#else
        Strcat(info, ", 吐き気がする");
#endif
    if (Confusion)
/*JP
        Strcat(info, ", confused");
*/
        Strcat(info, ", 混乱状態");
    if (Blind) {
#if 0 /*JP*/
        Strcat(info, ", blind");
        if (u.ucreamed) {
            if ((long) u.ucreamed < BlindedTimeout || Blindfolded
                || !haseyes(gy.youmonst.data))
                Strcat(info, ", cover");
            Strcat(info, "ed by sticky goop");
        } /* note: "goop" == "glop"; variation is intentional */
#else
        Strcat(info, ", ");
        if (u.ucreamed) {
            Strcat(info, "ねばねばべとつくもので");
            if ((long)u.ucreamed < BlindedTimeout || Blindfolded
                || !haseyes(gy.youmonst.data))
              Strcat(info, "覆われて");
        }
        Strcat(info, "盲目状態");
#endif
    }
    if (Stunned)
/*JP
        Strcat(info, ", stunned");
*/
        Strcat(info, ", くらくら状態");
    if (Wounded_legs && !u.usteed) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long legs = (EWounded_legs & BOTH_SIDES);
        const char *what = body_part(LEG);

        if (legs == BOTH_SIDES)
            what = makeplural(what);
        /* when it's just one leg, ^X reports which, left or right;
           ustatusline() doesn't, in order to keep the output a bit shorter */
/*JP
        Sprintf(eos(info), ", injured %s", what);
*/
        Sprintf(eos(info), ", %sにけがをしている", what);
    }
    if (Glib)
/*JP
        Sprintf(eos(info), ", slippery %s", fingers_or_gloves(TRUE));
*/
        Sprintf(eos(info), ", %sがぬるぬる", fingers_or_gloves(TRUE));
    if (u.utrap)
/*JP
        Strcat(info, ", trapped");
*/
        Strcat(info, ", 罠にかかっている");
    if (Fast)
/*JP
        Strcat(info, Very_fast ? ", very fast" : ", fast");
*/
        Strcat(info, Very_fast ? ", とても素早い" : ", 素早い");
    if (u.uundetected)
/*JP
        Strcat(info, ", concealed");
*/
        Strcat(info, ", 隠れている");
    else if (U_AP_TYPE != M_AP_NOTHING)
/*JP
        Strcat(info, ", disguised");
*/
        Strcat(info, ", ふりをしている");
    if (Invis)
/*JP
        Strcat(info, ", invisible");
*/
        Strcat(info, ", 不可視");
    if (u.ustuck) {
#if 0 /*JP*/
        if (u.uswallow)
            Strcat(info, digests(u.ustuck->data) ? ", being digested by "
                                                 : ", engulfed by ");
        else if (!sticks(gy.youmonst.data))
            Strcat(info, ", held by ");
        else
            Strcat(info, ", holding ");
        /* FIXME? a_monnam() uses x_monnam() which has a special case that
           forces "the" instead of "a" when formatting u.ustuck while hero
           is swallowed; we don't really want that here but it isn't worth
           fiddling with just for self-probing while engulfed */
        Strcat(info, a_monnam(u.ustuck));
#else
        Strcat(info, ", ");
        Strcat(info, mon_nam(u.ustuck));
        if (u.uswallow)
            Strcat(info, digests(u.ustuck->data) ? "に飲み込まれている"
                                                 : "巻き込まれている");
        else if (sticks(gy.youmonst.data))
            Strcat(info, "を掴まえている");
        else
            Strcat(info, "に掴まえられている");
#endif
    }
    if (!u.uswallow
        && (reg = visible_region_at(u.ux, u.uy)) != 0
        && (ln = strlen(info)) < sizeof info)
#if 0 /*JP:T*/
        Snprintf(eos(info), sizeof info - ln, ", in a cloud of %s",
                 reg_damg(reg) ? "poison gas" : "vapor");
#else
        Snprintf(eos(info), sizeof info - ln, ", %sの雲の中にいる",
                 reg_damg(reg) ? "毒ガス" : "蒸気");
#endif

#if 0 /*JP:T*/
    pline("Status of %s (%s):  Level %d  HP %d(%d)  AC %d%s.", svp.plname,
          piousness(FALSE, align_str(u.ualign.type)),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel, Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax, u.uac, info);
#else
    pline("%sの状態 (%s)： Level %d  HP %d(%d)  AC %d%s", svp.plname,
          piousness(FALSE, align_str(u.ualign.type)),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel, Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax, u.uac, info);
#endif
}

/* for 'onefile' processing where end of this file isn't necessarily the
   end of the source code seen by the compiler */
#undef enl_msg
#undef you_are
#undef you_have
#undef you_can
#undef you_have_been
#undef you_have_never
#undef you_have_X
#undef LL_majors
#undef majorevent
#undef spoilerevent
#undef UniqCritterIndx
#undef done_stopprint

/*insight.c*/
