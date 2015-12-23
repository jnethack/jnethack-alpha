/* NetHack 3.6	were.c	$NHDT-Date: 1432512763 2015/05/25 00:12:43 $  $NHDT-Branch: master $:$NHDT-Revision: 1.18 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

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
}

int
counter_were(pm)
int pm;
{
    switch (pm) {
    case PM_WEREWOLF:
        return (PM_HUMAN_WEREWOLF);
    case PM_HUMAN_WEREWOLF:
        return (PM_WEREWOLF);
    case PM_WEREJACKAL:
        return (PM_HUMAN_WEREJACKAL);
    case PM_HUMAN_WEREJACKAL:
        return (PM_WEREJACKAL);
    case PM_WERERAT:
        return (PM_HUMAN_WERERAT);
    case PM_HUMAN_WERERAT:
        return (PM_WERERAT);
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
              is_human(&mons[pm]) ? "人間" : mons[pm].mname + 4);
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

int were_summon(ptr, yours, visible,
                genbuf) /* were-creature (even you) summons a horde */
register struct permonst *ptr;
register boolean yours;
int *visible; /* number of visible helpers created */
char *genbuf;
{
    register int i, typ, pm = monsndx(ptr);
    register struct monst *mtmp;
    int total = 0;

    *visible = 0;
    if (Protection_from_shape_changers && !yours)
        return 0;
    for (i = rnd(5); i > 0; i--) {
        switch (pm) {
        case PM_WERERAT:
        case PM_HUMAN_WERERAT:
            typ =
                rn2(3) ? PM_SEWER_RAT : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT;
            if (genbuf)
/*JP
                Strcpy(genbuf, "rat");
*/
                Strcpy(genbuf, "ネズミ");
            break;
        case PM_WEREJACKAL:
        case PM_HUMAN_WEREJACKAL:
            typ = PM_JACKAL;
            if (genbuf)
/*JP
                Strcpy(genbuf, "jackal");
*/
                Strcpy(genbuf, "ジャッカル");
            break;
        case PM_WEREWOLF:
        case PM_HUMAN_WEREWOLF:
            typ = rn2(5) ? PM_WOLF : PM_WINTER_WOLF;
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

    if (Unchanging || (u.umonnum == u.ulycn))
        return;
    if (controllable_poly) {
#if 0 /*JP*/
        /* `+4' => skip "were" prefix to get name of beast */
        Sprintf(qbuf, "Do you want to change into %s?",
                an(mons[u.ulycn].mname + 4));
#else /*JP: 日本語ではそこまでしない */
        Sprintf(qbuf,"%sに変化しますか？",
                mons[u.ulycn].mname + 4);
#endif
        if (yn(qbuf) == 'n')
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
        u.ulycn = NON_PM; /* cure lycanthropy */
    }
    if (!Unchanging && is_were(youmonst.data)
/*JP
        && (!controllable_poly || yn("Remain in beast form?") == 'n'))
*/
        && (!controllable_poly || yn("獣の姿のままでいる？") == 'n'))
        rehumanize();
}

/*were.c*/
