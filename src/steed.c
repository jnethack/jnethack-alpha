/* NetHack 5.0	steed.c	$NHDT-Date: 1720128167 2024/07/04 21:22:47 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.121 $ */
/* Copyright (c) Kevin Hugo, 1998-1999. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-                */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Monsters that might be ridden */
static NEARDATA const char steeds[] = { S_QUADRUPED, S_UNICORN, S_ANGEL,
                                        S_CENTAUR,   S_DRAGON,  S_JABBERWOCK,
                                        '\0' };

staticfn boolean landing_spot(coord *, int, int);
staticfn void maybewakesteed(struct monst *);

/* caller has decided that hero can't reach something while mounted */
void
rider_cant_reach(void)
{
/*JP
    You("aren't skilled enough to reach from %s.", y_monnam(u.usteed));
*/
    You("まだ十分に技量を積んでいないので，%sから届かない．", y_monnam(u.usteed));
}

/*** Putting the saddle on ***/

/* Can this monster wear a saddle? */
boolean
can_saddle(struct monst *mtmp)
{
    struct permonst *ptr = mtmp->data;

    return (strchr(steeds, ptr->mlet) && (ptr->msize >= MZ_MEDIUM)
            && (!humanoid(ptr) || ptr->mlet == S_CENTAUR) && !amorphous(ptr)
            && !noncorporeal(ptr) && !is_whirly(ptr) && !unsolid(ptr));
}

int
use_saddle(struct obj *otmp)
{
    struct monst *mtmp;
    struct permonst *ptr;
    int chance;

    if (!u_handsy())
        return ECMD_OK;

    /* Select an animal */
    if (u.uswallow || Underwater || !getdir((char *) 0)) {
        pline1(Never_mind);
        return ECMD_CANCEL;
    }
    if (!u.dx && !u.dy) {
/*JP
        pline("Saddle yourself?  Very funny...");
*/
        pline("自分自身に鞍？おもしろい．．．");
        return ECMD_OK;
    }
    if (!isok(u.ux + u.dx, u.uy + u.dy)
        || !(mtmp = m_at(u.ux + u.dx, u.uy + u.dy)) || !canspotmon(mtmp)) {
/*JP
        pline("I see nobody there.");
*/
        pline("そこには誰もいないように見える．");
        return ECMD_TIME;
    }

    /* Is this a valid monster? */
    if ((mtmp->misc_worn_check & W_SADDLE) != 0L
        || which_armor(mtmp, W_SADDLE)) {
/*JP
        pline("%s doesn't need another one.", Monnam(mtmp));
*/
        pline("%sはもう鞍が取りつけられている．", Monnam(mtmp));
        return ECMD_TIME;
    }
    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
        char kbuf[BUFSZ];

/*JP
        You("touch %s.", mon_nam(mtmp));
*/
        You("%sに触れた．", mon_nam(mtmp));
        if (!(poly_when_stoned(gy.youmonst.data) && polymon(PM_STONE_GOLEM))) {
#if 0 /*JP:T*/
            Sprintf(kbuf, "attempting to saddle %s",
                    an(pmname(mtmp->data, Mgender(mtmp))));
#else
            Sprintf(kbuf, "%sに鞍を取りつけようとして",
                    pmname(mtmp->data, Mgender(mtmp)));
#endif
            instapetrify(kbuf);
        }
    }
    if (ptr == &mons[PM_AMOROUS_DEMON]) {
/*JP
        pline("Shame on you!");
*/
        pline("恥を知れ！");
        exercise(A_WIS, FALSE);
        return ECMD_TIME;
    }
    if (mtmp->isminion || mtmp->isshk || mtmp->ispriest || mtmp->isgd
        || mtmp->iswiz) {
/*JP
        pline("I think %s would mind.", mon_nam(mtmp));
*/
        pline("%sは嫌がっているようだ．", mon_nam(mtmp));
        return ECMD_TIME;
    }
    if (!can_saddle(mtmp)) {
/*JP
        You_cant("saddle such a creature.");
*/
        You("その生き物に鞍はとりつけられない．");
        return ECMD_TIME;
    }

    /* Calculate your chance */
    chance = ACURR(A_DEX) + ACURR(A_CHA) / 2 + 2 * mtmp->mtame;
    chance += u.ulevel * (mtmp->mtame ? 20 : 5);
    if (!mtmp->mtame)
        chance -= 10 * mtmp->m_lev;
    if (Role_if(PM_KNIGHT))
        chance += 20;
    switch (P_SKILL(P_RIDING)) {
    case P_ISRESTRICTED:
    case P_UNSKILLED:
    default:
        chance -= 20;
        break;
    case P_BASIC:
        break;
    case P_SKILLED:
        chance += 15;
        break;
    case P_EXPERT:
        chance += 30;
        break;
    }
    if (Confusion || Fumbling || Glib)
        chance -= 20;
#if 0 /*JP:T*/
    else if (uarmg && objdescr_is(uarmg, "riding gloves"))
#else
    else if (uarmg && objdescr_is(uarmg, "乗馬用の手袋"))
#endif
        /* Bonus for wearing "riding" (but not fumbling) gloves */
        chance += 10;
#if 0 /*JP:T*/
    else if (uarmf && objdescr_is(uarmf, "riding boots"))
#else
    else if (uarmf && objdescr_is(uarmf, "乗馬用の靴"))
#endif
        /* ... or for "riding boots" */
        chance += 10;
    if (otmp->cursed)
        chance -= 50;

    /* [intended] steed becomes alert if possible */
    maybewakesteed(mtmp);

    /* Make the attempt */
    if (rn2(100) < chance) {
/*JP
        You("put the saddle on %s.", mon_nam(mtmp));
*/
        You("鞍を%sに取りつけた．", mon_nam(mtmp));
        if (otmp->owornmask)
            remove_worn_item(otmp, FALSE);
        freeinv(otmp);
        /* !can_saddle(mtmp) already eliminated above */
        put_saddle_on_mon(otmp, mtmp);
    } else
/*JP
        pline("%s resists!", Monnam(mtmp));
*/
        pline("%sは拒否した！", Monnam(mtmp));
    return ECMD_TIME;
}

void
put_saddle_on_mon(struct obj *saddle, struct monst *mtmp)
{
    if (!can_saddle(mtmp) || which_armor(mtmp, W_SADDLE)) {
        if (saddle)
            impossible("put_saddle_on_mon: saddle obj could get orphaned");
        return;
    }
    if (!saddle) {
        if ((saddle = mksobj(SADDLE, TRUE, FALSE)) != 0) {
            fully_identify_obj(saddle);
            /* mpickobj can later override identification if out-of-view */
        } else {
            return;
        }
    }
    if (mpickobj(mtmp, saddle))
        panic("merged saddle?");
    mtmp->misc_worn_check |= W_SADDLE;
    saddle->owornmask = W_SADDLE;
    saddle->leashmon = mtmp->m_id;
    update_mon_extrinsics(mtmp, saddle, TRUE, FALSE);
}

/*** Riding the monster ***/

/* Can we ride this monster?  Caller should also check can_saddle() */
boolean
can_ride(struct monst *mtmp)
{
    return (mtmp->mtame && humanoid(gy.youmonst.data)
            && !verysmall(gy.youmonst.data) && !bigmonst(gy.youmonst.data)
            && (!Underwater || is_swimmer(mtmp->data)));
}

/* the #ride command */
int
doride(void)
{
    boolean forcemount = FALSE;

    if (u.usteed) {
        dismount_steed(DISMOUNT_BYCHOICE);
    } else if (getdir((char *) 0) && isok(u.ux + u.dx, u.uy + u.dy)) {
#if 0 /*JP:T*/
        if (wizard && y_n("Force the mount to succeed?") == 'y')
#else
        if (wizard && y_n("無理矢理成功させる？") == 'y')
#endif
            forcemount = TRUE;
        return (mount_steed(m_at(u.ux + u.dx, u.uy + u.dy), forcemount)
                ? ECMD_TIME : ECMD_OK);
    } else {
        return ECMD_CANCEL;
    }
    return ECMD_TIME;
}

/* Start riding, with the given monster */
boolean
mount_steed(
    struct monst *mtmp, /* The animal */
    boolean force)      /* Quietly force this animal */
{
    struct obj *otmp;
    char buf[BUFSZ];
    struct permonst *ptr;

    /* Sanity checks */
    if (u.usteed) {
/*JP
        You("are already riding %s.", mon_nam(u.usteed));
*/
        You("もう%sに乗っている．", mon_nam(u.usteed));
        return (FALSE);
    }

    /* Is the player in the right form? */
    if (Hallucination && !force) {
#if 0 /*JP:T*/
        pline("Maybe you should find a designated driver.");
#else
        pline("おそらくあなたは指定ドライバーを探すべきだろう．");
#endif
        return (FALSE);
    }
    /* While riding, Wounded_legs refers to the steed's
     * legs, not the hero's legs.
     * That opens up a potential abuse where the player
     * can mount a steed, then dismount immediately to
     * heal leg damage, because leg damage is always
     * healed upon dismount (Wounded_legs context switch).
     * By preventing a hero with Wounded_legs from
     * mounting a steed, the potential for abuse is
     * reduced.  However, dismounting still immediately
     * heals the steed's wounded legs.  [In 3.4.3 and
     * earlier, that unintentionally made the hero's
     * temporary 1 point Dex loss become permanent.]
     */
    if (Wounded_legs) {
        char qbuf[QBUFSZ];

/*JP
        legs_in_no_shape("riding", FALSE);
*/
        legs_in_no_shape("騎乗ができる", FALSE);
        Sprintf(qbuf, "Heal your leg%s?",
                ((HWounded_legs & BOTH_SIDES) == BOTH_SIDES) ? "s" : "");
        if (force && wizard && y_n(qbuf) == 'y')
            heal_legs(0);
        else
            return (FALSE);
    }

    if (Upolyd && (!humanoid(gy.youmonst.data)
                   || verysmall(gy.youmonst.data)
                   || bigmonst(gy.youmonst.data)
                   || slithy(gy.youmonst.data))) {
/*JP
        You("won't fit on a saddle.");
*/
        You("鞍に合わない．");
        return (FALSE);
    }
    if (!force && (near_capacity() > SLT_ENCUMBER)) {
/*JP
        You_cant("do that while carrying so much stuff.");
*/
        You("沢山物を持ちすぎており出来ない．");
        return (FALSE);
    }

    /* Can the player reach and see the monster? */
    if (!mtmp || (!force && ((Blind && !Blind_telepat) || mtmp->mundetected
                             || M_AP_TYPE(mtmp) == M_AP_FURNITURE
                             || M_AP_TYPE(mtmp) == M_AP_OBJECT))) {
/*JP
        pline("I see nobody there.");
*/
        pline("そこには何も見えない．");
        return (FALSE);
    }
    if (mtmp->data == &mons[PM_LONG_WORM]
        && (u.ux + u.dx != mtmp->mx || u.uy + u.dy != mtmp->my)) {
        /* As of 3.6.2:  test_move(below) is used to check for trying to mount
           diagonally into or out of a doorway or through a tight squeeze;
           attempting to mount a tail segment when hero was not adjacent
           to worm's head could trigger an impossible() in worm_cross()
           called from test_move(), so handle not-on-head before that */
/*JP
        You("couldn't ride %s, let alone its tail.", a_monnam(mtmp));
*/
        You("%sには乗れない，もちろん尻尾にも乗れない．", a_monnam(mtmp));
        return FALSE;
    }
    if (u.uswallow || u.ustuck || u.utrap || Punished
        || !test_move(u.ux, u.uy, mtmp->mx - u.ux, mtmp->my - u.uy,
                      TEST_MOVE)) {
        if (Punished || !(u.uswallow || u.ustuck || u.utrap))
/*JP
            You("are unable to swing your %s over.", body_part(LEG));
*/
            You("鞍をまたぐことができない．");
        else
/*JP
            You("are stuck here for now.");
*/
            You("はまっているので出来ない．");
        return (FALSE);
    }

    /* Is this a valid monster? */
    otmp = which_armor(mtmp, W_SADDLE);
    if (!otmp) {
/*JP
        pline("%s is not saddled.", Monnam(mtmp));
*/
        pline("%sには鞍が取りつけられていない．", Monnam(mtmp));
        return (FALSE);
    }

    ptr = mtmp->data;
    if (touch_petrifies(ptr) && !Stone_resistance) {
        char kbuf[BUFSZ];

/*JP
        You("touch %s.", mon_nam(mtmp));
*/
        You("%sに触れた．", mon_nam(mtmp));
#if 0 /*JP:T*/
        Sprintf(kbuf, "attempting to ride %s",
                an(pmname(mtmp->data, Mgender(mtmp))));
#else
        Sprintf(kbuf, "%sに乗ろうとして",
                pmname(mtmp->data, Mgender(mtmp)));
#endif
        instapetrify(kbuf);
    }
    if (!mtmp->mtame || mtmp->isminion) {
/*JP
        pline("I think %s would mind.", mon_nam(mtmp));
*/
        pline("%sは嫌がっているようだ．", mon_nam(mtmp));
        return (FALSE);
    }
    if (mtmp->mtrapped) {
        struct trap *t = t_at(mtmp->mx, mtmp->my);

#if 0 /*JP:T*/
        You_cant("mount %s while %s's trapped in %s.", mon_nam(mtmp),
                 mhe(mtmp), an(trapname(t->ttyp, FALSE)));
#else
        You_cant("%sに捕まっている%sには乗れない．",
                 trapname(t->ttyp, FALSE), mon_nam(mtmp));
#endif
        return (FALSE);
    }

    if (!force && !Role_if(PM_KNIGHT) && !(--mtmp->mtame)) {
        /* no longer tame */
        newsym(mtmp->mx, mtmp->my);
#if 0 /*JP:T*/
        pline("%s resists%s!", Monnam(mtmp),
              mtmp->mleashed ? " and its leash comes off" : "");
#else
        pline("%sは拒否%s！", Monnam(mtmp),
              mtmp->mleashed ? "して，紐をはずした" : "した");
#endif
        if (mtmp->mleashed)
            m_unleash(mtmp, FALSE);
        return (FALSE);
    }
    if (!force && Underwater && !is_swimmer(ptr)) {
#if 0 /*JP*/
        You_cant("ride that creature while under %s.",
                 hliquid("water"));
#else /*とりあえず水だけ*/
        You("水中で乗ることはできない．");
#endif
        return (FALSE);
    }
    if (!can_saddle(mtmp) || !can_ride(mtmp)) {
/*JP
        You_cant("ride such a creature.");
*/
        You("その生き物に乗ることはできない．");
        return FALSE;
    }

    /* Is the player impaired? */
    if (!force && !is_floater(ptr) && !is_flyer(ptr) && Levitation
        && !Lev_at_will) {
/*JP
        You("cannot reach %s.", mon_nam(mtmp));
*/
        You("%sに届かない．", mon_nam(mtmp));
        return (FALSE);
    }
    if (!force && uarm && is_metallic(uarm) && greatest_erosion(uarm)) {
#if 0 /*JP:T*/
        Your("%s armor is too stiff to be able to mount %s.",
             uarm->oeroded ? "rusty" : "corroded", mon_nam(mtmp));
#else
        Your("%s鎧はギシギシいっており%sに乗れない．",
             uarm->oeroded ? "錆びた" : "腐食した", mon_nam(mtmp));
#endif
        return (FALSE);
    }
    if (!force
        && (Confusion || Fumbling || Glib || Wounded_legs || otmp->cursed
            || otmp->greased
            || (u.ulevel + mtmp->mtame < rnd(MAXULEV / 2 + 5)))) {
        if (Levitation) {
/*JP
            pline("%s slips away from you.", Monnam(mtmp));
*/
            pline("%sはあなたからはなれていった．", Monnam(mtmp));
            return FALSE;
        }
/*JP
        You("slip while trying to get on %s.", mon_nam(mtmp));
*/
        You("%sに乗ろうとしてすべった．", mon_nam(mtmp));

/*JP
        Sprintf(buf, "slipped while mounting %s",
*/
        Sprintf(buf, "%sに乗ろうとしてすべり落ちて",
                /* "a saddled mumak" or "a saddled pony called Dobbin" */
                x_monnam(mtmp, ARTICLE_A, (char *) 0,
                         SUPPRESS_IT | SUPPRESS_INVISIBLE
                             | SUPPRESS_HALLUCINATION,
                         TRUE));
#if 0 /*JP*/
        losehp(Maybe_Half_Phys(rn1(5, 10)), buf, NO_KILLER_PREFIX);
#else
        losehp(Maybe_Half_Phys(rn1(5, 10)), buf, KILLED_BY);
#endif
        return (FALSE);
    }

    /* Success */
    maybewakesteed(mtmp);
    if (!force) {
        if (Levitation && !is_floater(ptr) && !is_flyer(ptr))
            /* Must have Lev_at_will at this point */
/*JP
            pline("%s magically floats up!", Monnam(mtmp));
*/
            pline("%sは魔法の力で浮いた！", Monnam(mtmp));
/*JP
        You("mount %s.", mon_nam(mtmp));
*/
        You("%sに乗った．", mon_nam(mtmp));
        if (Flying)
/*JP
            You("and %s take flight together.", mon_nam(mtmp));
*/
            You("%sと一緒に空を飛んだ．", mon_nam(mtmp));
    }
    /* setuwep handles polearms differently when you're mounted */
    if (uwep && is_pole(uwep))
        gu.unweapon = FALSE;
    u.usteed = mtmp;
    {
        boolean was_stealthy = Stealth != 0;

        steed_vs_stealth();
        if (was_stealthy && !Stealth)
            You("aren't stealthy anymore.");
    }
    remove_monster(mtmp->mx, mtmp->my);
    teleds(mtmp->mx, mtmp->my, TELEDS_ALLOW_DRAG);
    disp.botl = TRUE;
    return TRUE;
}

/* You and your steed have moved */
void
exercise_steed(void)
{
    if (!u.usteed)
        return;

    /* It takes many turns of riding to exercise skill */
    if (++u.urideturns >= 100) {
        u.urideturns = 0;
        use_skill(P_RIDING, 1);
    }
    return;
}

/* The player kicks or whips the steed */
void
kick_steed(void)
{
    char He[BUFSZ]; /* monverbself() appends to the "He"/"She"/"It" value */
    if (!u.usteed)
        return;

    /* [ALI] Various effects of kicking sleeping/paralyzed steeds */
    if (helpless(u.usteed)) {
#if 0 /*JP:T*/
        /* We assume a message has just been output of the form
         * "You kick <steed>."
         */
        Strcpy(He, mhe(u.usteed));
        *He = highc(*He);
#else /*乗騎に「彼」などは不自然なので直接名前にする*/
        Strcpy(He, mon_nam(u.usteed));
#endif
        if ((u.usteed->mcanmove || u.usteed->mfrozen) && !rn2(2)) {
            if (u.usteed->mcanmove)
                u.usteed->msleeping = 0;
            else if (u.usteed->mfrozen > 2)
                u.usteed->mfrozen -= 2;
            else {
                u.usteed->mfrozen = 0;
                u.usteed->mcanmove = 1;
            }
            if (helpless(u.usteed))
#if 0 /*JP:T*/
                pline("%s stirs.", He);
#else
                pline("%sは身じろぎした．", He);
#endif
            else
                /* if hallucinating, might yield "He rouses herself" or
                   "She rouses himself" */
#if 0 /*JP:T*/
                pline("%s!", monverbself(u.usteed, He, "rouse", (char *) 0));
#else
                pline("%sは奮起した！", He);
#endif
        } else
#if 0 /*JP:T*/
            pline("%s does not respond.", He);
#else
            pline("%sは反応しない．", He);
#endif
        return;
    }

    /* Make the steed less tame and check if it resists */
    if (u.usteed->mtame)
        u.usteed->mtame--;
    if (!u.usteed->mtame && u.usteed->mleashed)
        m_unleash(u.usteed, TRUE);
    if (!u.usteed->mtame
        || (u.ulevel + u.usteed->mtame < rnd(MAXULEV / 2 + 5))) {
        newsym(u.usteed->mx, u.usteed->my);
        dismount_steed(DISMOUNT_THROWN);
        return;
    }

/*JP
    pline("%s gallops!", Monnam(u.usteed));
*/
    pline("%sは速足になった！", Monnam(u.usteed));
    u.ugallop += rn1(20, 30);
    return;
}

/*
 * Try to find a dismount point adjacent to the steed's location.
 * If all else fails, try enexto().  Use enexto() as a last resort because
 * enexto() chooses its point randomly, possibly even outside the
 * room's walls, which is not what we want.
 * Adapted from mail daemon code.
 */
staticfn boolean
landing_spot(
    coord *spot, /* landing position (we fill it in) */
    int reason,
    int forceit)
{
    coord cc, try[8]; /* 8: the 8 spots adjacent to the hero's spot */
    int i, j, best_j, clockwise_j, counterclk_j,
        n, viable, distance, min_distance = -1;
    coordxy x, y;
    boolean found, impaird, kn_trap, boulder;
    struct trap *t;

    (void) memset((genericptr_t) try, 0, sizeof try);
    n = 0;
    j = xytodir(u.dx, u.dy);
    if (reason == DISMOUNT_KNOCKED && j != DIR_ERR) {
        /* we'll check preferred location first; if viable it'll be picked */
        best_j = j;
        try[0].x = u.dx, try[0].y = u.dy;
        /* the two next best locations are checked second and third */
        i = rn2(2);
        clockwise_j = DIR_RIGHT(j); /* (j + 1) % 8 */
        dirtocoord(&cc, clockwise_j);
        try[1 + i].x = cc.x, try[1 + i].y = cc.y; /* [1] or [2] */
        counterclk_j = DIR_LEFT(j); /* (j + 8 - 1) % 8 */
        dirtocoord(&cc, counterclk_j);
        try[2 - i].x = cc.x, try[2 - i].y = cc.y; /* [2] or [1] */
        n = 3;
        debugpline3("knock from saddle: best %s, next %s or %s",
                    directionname(best_j),
                    directionname(clockwise_j), directionname(counterclk_j));
    } else {
        best_j = clockwise_j = counterclk_j = -1;
    }
    for (j = 0; j < N_DIRS; ++j) {
        /* fortunately NODIAG() handling isn't needed for DISMOUNT_KNOCKED
           because hero can only ride when humanoid */
        if (j == best_j || j == clockwise_j || j == counterclk_j)
            continue;
        /* j==0 is W, j==1 NW, j==2 N, j==3 NE, ..., around to j==7 SW;
           so odd j values are diagonal directions here */
        if (reason == DISMOUNT_POLY && NODIAG(u.umonnum) && (j % 1) != 0)
            continue;
        dirtocoord(&cc, j);
        try[n++] = cc;
    }

    /*
     * Up to three passes;
     * i==0: voluntary dismount without impairment avoids known traps and
     *       boulders;
     * i==1: voluntary dismount with impairment or knocked out of saddle
     *       avoids boulders but allows known traps;
     * i==2: other, allow traps and boulders.
     *
     * Fallback to i==1 if nothing appropriate was found for i==0 and
     * to i==2 as last resort.
     */
    impaird = (Stunned || Confusion || Fumbling);
    viable = 0;
    found = FALSE;
    for (i = (reason == DISMOUNT_BYCHOICE && !impaird) ? 0
             : ((reason == DISMOUNT_BYCHOICE && impaird)
                || reason == DISMOUNT_KNOCKED) ? 1
               : 2;
         i <= 2 && !found; ++i) {
        for (j = 0; j < n; ++j) {
            x = u.ux + try[j].x;
            y = u.uy + try[j].y;
            if (!isok(x, y) || u_at(x, y)) /* [note: u_at() can't happen] */
                continue;

            if (accessible(x, y) && !MON_AT(x, y)
                && test_move(u.ux, u.uy, x - u.ux, y - u.uy, TEST_MOVE)) {
                ++viable;
                distance = distu(x, y);
                if (min_distance < 0 /* no viable candidate yet */
                    /* or better than pending candidate (note: orthogonal
                       spots are distance 1 and diagonal ones distance 2;
                       treating one as better than the other is arbitrary
                       and not wanted for DISMOUNT_KNOCKED) */
                    || ((best_j == -1) ? (distance < min_distance) : (j < 3))
                    /* or equally good, maybe substitute this one */
                    || (distance == min_distance && !rn2(viable))) {
                    /* traps avoided on pass 0; boulders avoided on 0 and 1 */
                    kn_trap = i == 0 && ((t = t_at(x, y)) != 0 && t->tseen
                                         && t->ttyp != VIBRATING_SQUARE);
                    boulder = i <= 1 && (sobj_at(BOULDER, x, y)
                                         && !throws_rocks(gy.youmonst.data));
                    if (!kn_trap && !boulder) {
                        spot->x = x;
                        spot->y = y;
                        min_distance = distance;
                        found = TRUE;
                        if (best_j != -1 && j < 3)
                            /* since best_j is first candidate (j==0), j==1
                               and j==2 can only get here when best_j was
                               not viable; 50:50 chance for clockwise_j to
                               come before counterclk_j so each has same
                               chance to be next after best_j */
                            break;
                    }
                }
            }
        }
    }

    /* If we didn't find a good spot and forceit is on, try enexto(). */
    if (forceit && !found)
        found = enexto(spot, u.ux, u.uy, gy.youmonst.data);

    return found;
}

/* Stop riding the current steed */
void
dismount_steed(
    int reason) /* Player was thrown off etc. */
{
    struct monst *mtmp;
    struct obj *otmp;
    const char *verb;
    coord cc, steedcc;
    unsigned save_utrap = u.utrap;
    boolean ulev, ufly,
            repair_leg_damage = (Wounded_legs != 0L),
            have_spot = landing_spot(&cc, reason, 0);

    mtmp = u.usteed; /* make a copy of steed pointer */
    /* Sanity check */
    if (!mtmp) /* Just return silently */
        return;
    u.usteed = 0; /* affects Fly test; could hypothetically affect Lev;
                   * also affects u_locomotion() */
    ufly = Flying ? TRUE : FALSE;
    ulev = Levitation ? TRUE : FALSE;
#if 0 /*JP*//*後で設定*/
    verb = u_locomotion("fall"); /* only used for _FELL and _KNOCKED */
#endif
    u.usteed = mtmp;

    /* Check the reason for dismounting */
    otmp = which_armor(mtmp, W_SADDLE);
    switch (reason) {
    case DISMOUNT_THROWN:
#if 0 /*JP*/
        verb = "are thrown";
        FALLTHROUGH;
        /*FALLTHRU*/
#endif
    case DISMOUNT_KNOCKED:
    case DISMOUNT_FELL:
#if 0 /*JP:T*/
        You("%s off of %s!", verb, mon_nam(mtmp));
#else
        if (reason == DISMOUNT_THROWN) {
            verb = (!ulev && !ufly) ? "振り落とされた" : "振りはらわれた";
        } else {
            verb = (!ulev && !ufly) ? "落ちた" : "降りた";
        }
        You("%sから%s！", mon_nam(mtmp), verb);
#endif
        if (!have_spot)
            have_spot = landing_spot(&cc, reason, 1);
        if (!ulev && !ufly) {
#if 0 /*JP:T*/
            losehp(Maybe_Half_Phys(rn1(10, 10)), "riding accident",
                   KILLED_BY_AN);
#else
            losehp(Maybe_Half_Phys(rn1(10, 10)), "騎乗事故で",
                   KILLED_BY_AN);
#endif
            set_wounded_legs(BOTH_SIDES, (int) HWounded_legs + rn1(5, 5));
            repair_leg_damage = FALSE;
        }
        break;
    case DISMOUNT_POLY:
/*JP
        You("can no longer ride %s.", mon_nam(u.usteed));
*/
        You("%sに乗ってられない．", mon_nam(u.usteed));
        if (!have_spot)
            have_spot = landing_spot(&cc, reason, 1);
        break;
    case DISMOUNT_ENGULFED:
        /* caller displays message */
        break;
    case DISMOUNT_BONES:
        /* hero has just died... */
        break;
    case DISMOUNT_GENERIC:
        /* no messages, just make it so */
        break;
    case DISMOUNT_BYCHOICE:
    default:
        if (otmp && otmp->cursed) {
#if 0 /*JP:T*/
            You("can't.  The saddle %s cursed.",
                otmp->bknown ? "is" : "seems to be");
#else
            You("降りられない．鞍は呪われている%s．",
                otmp->bknown ? "" : "ようだ");
#endif
            otmp->bknown = 1; /* ok to skip set_bknown() here */
            return;
        }
        if (!have_spot) {
/*JP
            You("can't.  There isn't anywhere for you to stand.");
*/
            pline("あなたの立つ場所がないので降りられない．");
            return;
        }
        if (!has_mgivenname(mtmp)) {
#if 0 /*JP:T*/
            pline("You've been through the dungeon on %s with no name.",
                  an(pmname(mtmp->data, Mgender(mtmp))));
#else
            pline("あなたは名前のない%sと共に迷宮内にいる．",
                  pmname(mtmp->data, Mgender(mtmp)));
#endif
            if (Hallucination)
/*JP
                pline("It felt good to get out of the rain.");
*/
                pline("雨が降らないというのはいい気分だった．");
        } else
/*JP
            You("dismount %s.", mon_nam(mtmp));
*/
            You("%sから降りた．", mon_nam(mtmp));
    }
    /* While riding, Wounded_legs refers to the steed's legs;
       after dismounting, it reverts to the hero's legs. */
    if (repair_leg_damage)
        heal_legs(1);

    /* Release the steed */
    u.usteed = (struct monst *) NULL;
    u.ugallop = 0L;
    {
        boolean was_stealthy = Stealth != 0;

        steed_vs_stealth();
        if (Stealth && !was_stealthy)
            You("seem less noisy now.");
    }

    if (u.utraptype == TT_BEARTRAP
        || u.utraptype == TT_PIT
        || u.utraptype == TT_WEB) {
        mtmp->mtrapped = 1;
    }

    /*
     * rloc(), rloc_to(), and monkilled()->mondead()->m_detach() all
     * expect mtmp to be on the map or else have mtmp->mx be 0, but
     * setting the latter to 0 here would interfere with dropping
     * the saddle.  Prior to 3.6.2, being off the map didn't matter.
     *
     * place_monster() expects mtmp to be alive and not be u.usteed.
     *
     * Unfortunately, <u.ux,u.uy> (former steed's implicit location)
     * might now be occupied by an engulfer, so we can't just put mtmp
     * at that spot.  An engulfer's previous spot will be unoccupied
     * but we don't know where that was and even if we did, it might
     * be hostile terrain.
     */
    steedcc.x = u.ux, steedcc.y = u.uy;
    if (m_at(u.ux, u.uy)) {
        /* hero's spot has a monster in it; hero must have been plucked
           from saddle as engulfer moved into his spot--other dismounts
           shouldn't run into this situation; find nearest viable spot */
        if (!enexto(&steedcc, u.ux, u.uy, mtmp->data)
            /* no spot? must have been engulfed by a lurker-above over
               water or lava; try requesting a location for a flyer */
            && !enexto(&steedcc, u.ux, u.uy, &mons[PM_BAT]))
            /* still no spot; last resort is any spot within bounds */
            (void) enexto(&steedcc, u.ux, u.uy, &mons[PM_GHOST]);
    }

    if (!DEADMONSTER(mtmp)) {
        gi.in_steed_dismounting++;
        place_monster(mtmp, steedcc.x, steedcc.y);
        gi.in_steed_dismounting--;

        /* if for bones, there's no reason to place the hero;
           we want to make room for potential ghost, so move steed */
        if (reason == DISMOUNT_BONES) {
            /* move the steed to an adjacent square */
            if (enexto(&cc, u.ux, u.uy, mtmp->data))
                rloc_to(mtmp, cc.x, cc.y);
            else /* evidently no room nearby; move steed elsewhere */
                (void) rloc(mtmp, RLOC_ERR | RLOC_NOMSG);
            return;
        }

        /* Set hero's and/or steed's positions.  Usually try moving the
           hero first.  Note: for DISMOUNT_ENGULFED, caller hasn't set
           u.uswallow yet but has set u.ustuck. */
        if (!u.uswallow && !u.ustuck && have_spot) {
            struct permonst *mdat = mtmp->data;

            /* The steed may drop into water/lava */
            if (grounded(mdat)) {
                if (is_pool(u.ux, u.uy)) {
                    if (!Underwater)
/*JP
                        pline("%s falls into the %s!", Monnam(mtmp),
*/
                        pline("%sは%sに落ちた！", Monnam(mtmp),
                              surface(u.ux, u.uy));
                    if (!cant_drown(mdat)) {
                        killed(mtmp);
                        adjalign(-1);
                    }
                } else if (is_lava(u.ux, u.uy)) {
#if 0 /*JP:T*/
                    pline("%s is pulled into the %s!", Monnam(mtmp),
                          hliquid("lava"));
#else
                    pline("%sは%sの中にひっぱられた！", Monnam(mtmp),
                          hliquid("溶岩"));
#endif
                    if (!likes_lava(mdat)) {
                        killed(mtmp);
                        adjalign(-1);
                    }
                }
            }
            /* Steed dismounting consists of two steps: being moved to another
             * square, and descending to the floor.  We have functions to do
             * each of these activities, but they're normally called
             * individually and include an attempt to look at or pick up the
             * objects on the floor:
             * teleds() --> spoteffects() --> pickup()
             * float_down() --> pickup()
             * We use this kludge to make sure there is only one such attempt.
             *
             * Clearly this is not the best way to do it.  A full fix would
             * involve having these functions not call pickup() at all,
             * instead calling them first and calling pickup() afterwards.
             * But it would take a lot of work to keep this change from
             * having any unforeseen side effects (for instance, you would
             * no longer be able to walk onto a square with a hole, and
             * autopickup before falling into the hole).
             */
            /* [ALI] No need to move the player if the steed died. */
            if (!DEADMONSTER(mtmp)) {
                /* Keep steed here, move the player to cc;
                 * teleds() clears u.utrap
                 */
                gi.in_steed_dismounting = TRUE;
                teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG);
                if (sobj_at(BOULDER, cc.x, cc.y))
                    sokoban_guilt();
                gi.in_steed_dismounting = FALSE;

                /* Put your steed in your trap */
                if (save_utrap)
                    (void) mintrap(mtmp, NO_TRAP_FLAGS);
            }

        /* Couldn't move hero... try moving the steed. */
        } else if (enexto(&cc, u.ux, u.uy, mtmp->data)) {
            /* Keep player here, move the steed to cc */
            rloc_to(mtmp, cc.x, cc.y);
            /* Player stays put */

        /* Otherwise, steed goes bye-bye. */
        } else {
#if 1       /* original there's-no-room handling */
            if (reason == DISMOUNT_BYCHOICE) {
                /* [un]#ride: hero gets credit/blame for killing steed */
                killed(mtmp);
                adjalign(-1);
            } else {
                /* other dismount: kill former steed with no penalty;
                   damage type is just "neither AD_DGST nor -AD_RBRE" */
                monkilled(mtmp, "", -AD_PHYS);
            }
#else
            /* Can't use this [yet?] because it violates monmove()'s
             * assumption that a moving monster (engulfer) can't cause
             * another monster (steed) to be removed from the fmon list.
             * That other monster (steed) might be cached as the next one
             * to move.
             */
            /* migrate back to this level if hero leaves and returns
               or to next level if it is happening in the endgame */
            mdrop_special_objs(mtmp);
            deal_with_overcrowding(mtmp);
#endif
        }
    } /* !DEADMONST(mtmp) */

    /* usually return the hero to the surface */
    if (reason != DISMOUNT_ENGULFED && reason != DISMOUNT_BONES) {
        gi.in_steed_dismounting = TRUE;
        (void) float_down(0L, W_SADDLE);
        gi.in_steed_dismounting = FALSE;
        disp.botl = TRUE;
        encumber_msg();
        gv.vision_full_recalc = 1;
    } else
        disp.botl = TRUE;
    /* polearms behave differently when not mounted */
    if (uwep && is_pole(uwep))
        gu.unweapon = TRUE;
    return;
}

/* when attempting to saddle or mount a sleeping steed, try to wake it up
   (for the saddling case, it won't be u.usteed yet) */
staticfn void
maybewakesteed(struct monst *steed)
{
    int frozen = (int) steed->mfrozen;
    boolean wasimmobile = helpless(steed);

    steed->msleeping = 0;
    if (frozen) {
        frozen = (frozen + 1) / 2; /* half */
        /* might break out of timed sleep or paralysis */
        if (!rn2(frozen)) {
            steed->mfrozen = 0;
            steed->mcanmove = 1;
        } else {
            /* didn't awake, but remaining duration is halved */
            steed->mfrozen = frozen;
        }
    }
    if (wasimmobile && !helpless(steed))
/*JP
        pline("%s wakes up.", Monnam(steed));
*/
        pline("%sは起きた．", Monnam(steed));
    /* regardless of waking, terminate any meal in progress */
    finish_meating(steed);
}

/* steed has taken on a new shape */
void
poly_steed(
    struct monst *steed,
    struct permonst *oldshape)
{
    if (!can_saddle(steed) || !can_ride(steed)) {
        /* can't get here; newcham() -> mon_break_armor() -> m_lose_armor()
           removes saddle and/or forces hero to dismount, if applicable,
           before newcham() calls us */
        dismount_steed(DISMOUNT_FELL);
    } else {
        char buf[BUFSZ];

        Strcpy(buf, x_monnam(steed, ARTICLE_YOUR, (char *) 0,
                             SUPPRESS_SADDLE, FALSE));
        if (oldshape != steed->data)
            (void) strsubst(buf, "your ", "your new ");
/*JP
        You("adjust yourself in the saddle on %s.", buf);
*/
        You("%sの鞍の上で座りなおした．", buf);

        /* riding blocks stealth unless hero+steed fly */
        steed_vs_stealth();
    }
}

/* decide whether hero's steed is able to move;
   doesn't check for holding traps--those affect the hero directly */
boolean
stucksteed(boolean checkfeeding)
{
    struct monst *steed = u.usteed;

    if (steed) {
        /* check whether steed can move */
        if (helpless(steed)) {
/*JP
            pline("%s won't move!", YMonnam(steed));
*/
            pline("%sは動けない！", YMonnam(steed));
            return TRUE;
        }
        /* optionally check whether steed is in the midst of a meal */
        if (checkfeeding && steed->meating) {
/*JP
            pline("%s is still eating.", YMonnam(steed));
*/
            pline("%sはまだ食べている．", YMonnam(steed));
            return TRUE;
        }
    }
    return FALSE;
}

void
place_monster(struct monst *mon, coordxy x, coordxy y)
{
    struct monst *othermon;
    const char *monnm, *othnm;
    char buf[QBUFSZ];

    buf[0] = '\0';
    /* normal map bounds are <1..COLNO-1,0..ROWNO-1> but sometimes
       vault guards (either living or dead) are parked at <0,0> */
    if (!isok(x, y) && (x != 0 || y != 0 || !mon->isgd)) {
        describe_level(buf, 0);
        impossible("trying to place %s at <%d,%d> mstate:%lx on %s",
                   minimal_monnam(mon, TRUE), x, y, mon->mstate, buf);
        x = y = 0;
    }
    if ((mon == u.usteed && !gi.in_steed_dismounting)
        /* special case is for convoluted vault guard handling */
        || (DEADMONSTER(mon) && !(mon->isgd && x == 0 && y == 0))) {
        describe_level(buf, 0);
        impossible("placing %s onto map, mstate:%lx, on %s?",
                   (mon == u.usteed) ? "steed" : "defunct monster",
                   mon->mstate, buf);
        return;
    }
    if ((othermon = svl.level.monsters[x][y]) != 0) {
        describe_level(buf, 0);
        monnm = minimal_monnam(mon, FALSE);
        othnm = (mon != othermon) ? minimal_monnam(othermon, TRUE) : "itself";
        impossible("placing %s over %s at <%d,%d>, mstates:%lx %lx on %s?",
                   monnm, othnm, x, y, othermon->mstate, mon->mstate, buf);
    }
    mon->mx = x, mon->my = y;
    svl.level.monsters[x][y] = mon;
    mon->mstate = MON_FLOOR;
}

/*steed.c*/
