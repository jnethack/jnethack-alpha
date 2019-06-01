/* NetHack 3.6	were.c	$NHDT-Date: 1505214877 2017/09/12 11:14:37 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.21 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2019            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#if 1 /*JP*/
STATIC_DCL char *FDECL(beastname, (const char *));

/*JP 「ジャッカル人間」から「ジャッカル」を取り出す */
STATIC_OVL char *
beastname(name)
const char *name;
{
    static char werebuf[BUFSZ];
    strcpy(werebuf, name);
    werebuf[strlen(werebuf) - 4] = '\0';
    return werebuf;
}
#endif

void
were_change(mon)
register struct monst *mon;
{
    if (!is_were(mon->data))
        return;

    if (is_human(mon->data)) {
        if (!Protection_from_shape_changers
            && !rn2(night() ? (flags.moonphase == FULL_MOON ? 3 : 30)
                            : (flags.moonphase == FULL_MOON ? 10 : 50))) {
            new_were(mon); /* change into animal form */
            if (!Deaf && !canseemon(mon)) {
                const char *howler;

                switch (monsndx(mon->data)) {
                case PM_WEREWOLF:
/*JP
                    howler = "wolf";
*/
                    howler = "狼";
                    break;
                case PM_WEREJACKAL:
/*JP
                    howler = "jackal";
*/
                    howler = "ジャッカル";
                    break;
                default:
                    howler = (char *) 0;
                    break;
                }
                if (howler)
/*JP
                    You_hear("a %s howling at the moon.", howler);
*/
                    You_hear("月夜に%sが吠える声を聞いた．", howler);
            }
        }
    } else if (!rn2(30) || Protection_from_shape_changers) {
        new_were(mon); /* change back into human form */
    }
    /* update innate intrinsics (mainly Drain_resistance) */
    set_uasmon(); /* new_were() doesn't do this */
}

int
counter_were(pm)
int pm;
{
    switch (pm) {
    case PM_WEREWOLF:
        return PM_HUMAN_WEREWOLF;
    case PM_HUMAN_WEREWOLF:
        return PM_WEREWOLF;
    case PM_WEREJACKAL:
        return PM_HUMAN_WEREJACKAL;
    case PM_HUMAN_WEREJACKAL:
        return PM_WEREJACKAL;
    case PM_WERERAT:
        return PM_HUMAN_WERERAT;
    case PM_HUMAN_WERERAT:
        return PM_WERERAT;
    default:
        return NON_PM;
    }
}

/* convert monsters similar to werecritters into appropriate werebeast */
int
were_beastie(pm)
int pm;
{
    switch (pm) {
    case PM_WERERAT:
    case PM_SEWER_RAT:
    case PM_GIANT_RAT:
    case PM_RABID_RAT:
        return PM_WERERAT;
    case PM_WEREJACKAL:
    case PM_JACKAL:
    case PM_FOX:
    case PM_COYOTE:
        return PM_WEREJACKAL;
    case PM_WEREWOLF:
    case PM_WOLF:
    case PM_WARG:
    case PM_WINTER_WOLF:
        return PM_WEREWOLF;
    default:
        break;
    }
    return NON_PM;
}

void
new_were(mon)
register struct monst *mon;
{
    register int pm;

    pm = counter_were(monsndx(mon->data));
    if (pm < LOW_PM) {
        impossible("unknown lycanthrope %s.", mon->data->mname);
        return;
    }

    if (canseemon(mon) && !Hallucination)
#if 0 /*JP*/
        pline("%s changes into a %s.", Monnam(mon),
              is_human(&mons[pm]) ? "human" : mons[pm].mname + 4);
#else
        pline("%sは%sの姿になった．", Monnam(mon),
              is_human(&mons[pm]) ? "人間" : beastname(mons[pm].mname));
#endif

    set_mon_data(mon, &mons[pm], 0);
    if (mon->msleeping || !mon->mcanmove) {
        /* transformation wakens and/or revitalizes */
        mon->msleeping = 0;
        mon->mfrozen = 0; /* not asleep or paralyzed */
        mon->mcanmove = 1;
    }
    /* regenerate by 1/4 of the lost hit points */
    mon->mhp += (mon->mhpmax - mon->mhp) / 4;
    newsym(mon->mx, mon->my);
    mon_break_armor(mon, FALSE);
    possibly_unwield(mon, FALSE);
}

/* were-creature (even you) summons a horde */
int
were_summon(ptr, yours, visible, genbuf)
struct permonst *ptr;
boolean yours;
int *visible; /* number of visible helpers created */
char *genbuf;
{
    int i, typ, pm = monsndx(ptr);
    struct monst *mtmp;
    int total = 0;

    *visible = 0;
    if (Protection_from_shape_changers && !yours)
        return 0;
    for (i = rnd(5); i > 0; i--) {
        switch (pm) {
        case PM_WERERAT:
        case PM_HUMAN_WERERAT:
            typ = rn2(3) ? PM_SEWER_RAT
                         : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT;
            if (genbuf)
/*JP
                Strcpy(genbuf, "rat");
*/
                Strcpy(genbuf, "ネズミ");
            break;
        case PM_WEREJACKAL:
        case PM_HUMAN_WEREJACKAL:
            typ = rn2(7) ? PM_JACKAL : rn2(3) ? PM_COYOTE : PM_FOX;
            if (genbuf)
/*JP
                Strcpy(genbuf, "jackal");
*/
                Strcpy(genbuf, "ジャッカル");
            break;
        case PM_WEREWOLF:
        case PM_HUMAN_WEREWOLF:
            typ = rn2(5) ? PM_WOLF : rn2(2) ? PM_WARG : PM_WINTER_WOLF;
            if (genbuf)
/*JP
                Strcpy(genbuf, "wolf");
*/
                Strcpy(genbuf, "狼");
            break;
        default:
            continue;
        }
        mtmp = makemon(&mons[typ], u.ux, u.uy, NO_MM_FLAGS);
        if (mtmp) {
            total++;
            if (canseemon(mtmp))
                *visible += 1;
        }
        if (yours && mtmp)
            (void) tamedog(mtmp, (struct obj *) 0);
    }
    return total;
}

void
you_were()
{
    char qbuf[QBUFSZ];
    boolean controllable_poly = Polymorph_control && !(Stunned || Unaware);

    if (Unchanging || u.umonnum == u.ulycn)
        return;
    if (controllable_poly) {
#if 0 /*JP*/
        /* `+4' => skip "were" prefix to get name of beast */
        Sprintf(qbuf, "Do you want to change into %s?",
                an(mons[u.ulycn].mname + 4));
#else /* 日本語では専用関数を使う */
        Sprintf(qbuf, "%sに変化しますか？",
                beastname(mons[u.ulycn].mname));
#endif
        if (!paranoid_query(ParanoidWerechange, qbuf))
            return;
    }
    (void) polymon(u.ulycn);
}

void
you_unwere(purify)
boolean purify;
{
    boolean controllable_poly = Polymorph_control && !(Stunned || Unaware);

    if (purify) {
/*JP
        You_feel("purified.");
*/
        You("浄められたような気がした．");
        set_ulycn(NON_PM); /* cure lycanthropy */
    }
    if (!Unchanging && is_were(youmonst.data)
        && (!controllable_poly
/*JP
            || !paranoid_query(ParanoidWerechange, "Remain in beast form?")))
*/
            || !paranoid_query(ParanoidWerechange, "獣の姿のままでいる？")))
        rehumanize();
    else if (is_were(youmonst.data) && !u.mtimedone)
        u.mtimedone = rn1(200, 200); /* 40% of initial were change */
}

/* lycanthropy is being caught or cured, but no shape change is involved */
void
set_ulycn(which)
int which;
{
    u.ulycn = which;
    /* add or remove lycanthrope's innate intrinsics (Drain_resistance) */
    set_uasmon();
}

/*were.c*/
