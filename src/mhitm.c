/* NetHack 5.0	mhitm.c	$NHDT-Date: 1732979463 2024/11/30 07:11:03 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.253 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-                */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

static const char brief_feeling[] =
/*JP
    "have a %s feeling for a moment, then it passes.";
*/
    "%s気持におそわれたが，すぐに過ぎさった．";

staticfn void noises(struct monst *, struct attack *);
staticfn void pre_mm_attack(struct monst *, struct monst *);
staticfn void missmm(struct monst *, struct monst *, struct attack *);
staticfn int hitmm(struct monst *, struct monst *, struct attack *,
                 struct obj *, int);
staticfn int gazemm(struct monst *, struct monst *, struct attack *);
staticfn int gulpmm(struct monst *, struct monst *, struct attack *);
staticfn int explmm(struct monst *, struct monst *, struct attack *);
staticfn int mdamagem(struct monst *, struct monst *, struct attack *,
                    struct obj *, int);
staticfn void mswingsm(struct monst *, struct monst *, struct obj *);
staticfn int passivemm(struct monst *, struct monst *, boolean, int,
                     struct obj *);

staticfn void
noises(struct monst *magr, struct attack *mattk)
{
    boolean farq = (mdistu(magr) > 15);

    if (!Deaf && (farq != gf.far_noise || svm.moves - gn.noisetime > 10)) {
        gf.far_noise = farq;
        gn.noisetime = svm.moves;
#if 0 /*JP:T*/
        You_hear("%s%s.",
                 (mattk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
                 farq ? " in the distance" : "");
#else
        You_hear("%s%sを聞いた．",
                 farq ? "遠くで" : "",
                 (mattk->aatyp == AT_EXPL) ? "爆発音" : "何かが戦う音"
                 );
#endif
    }
}

staticfn void
pre_mm_attack(struct monst *magr, struct monst *mdef)
{
    boolean showit = FALSE;

    /* unhiding or unmimicking happens even if hero can't see it
       because the formerly concealed monster is now in action */
    if (M_AP_TYPE(mdef)) {
        seemimic(mdef);
        showit |= gv.vis;
    } else if (mdef->mundetected) {
        mdef->mundetected = 0;
        showit |= gv.vis;
    }
    if (M_AP_TYPE(magr)) {
        seemimic(magr);
        showit |= gv.vis;
    } else if (magr->mundetected) {
        magr->mundetected = 0;
        showit |= gv.vis;
    }

    if (gv.vis) {
        if (!canspotmon(magr))
            map_invisible(magr->mx, magr->my);
        else if (showit)
            newsym(magr->mx, magr->my);
        if (!canspotmon(mdef))
            map_invisible(mdef->mx, mdef->my);
        else if (showit)
            newsym(mdef->mx, mdef->my);
    }
}

/* feedback for when a monster-vs-monster attack misses */
staticfn void
missmm(
    struct monst *magr, /* attacker */
    struct monst *mdef, /* defender */
    struct attack *mattk) /* attack and damage types */
{
    pre_mm_attack(magr, mdef);

    if (gv.vis) {
#if 0 /*JP*/
        pline("%s %s %s.", Monnam(magr),
              (magr->mcan || !could_seduce(magr, mdef, mattk)) ? "misses"
                  : "pretends to be friendly to",
              mon_nam_too(mdef, magr));
#else
        pline((could_seduce(magr,mdef,mattk) && !magr->mcan)
                  ? "%sは%sに友好的なふりをした．"
                  : "%sの%sへの攻撃は外れた．",
                   Monnam(magr), mon_nam_too(mdef, magr));
#endif
    } else {
        noises(magr, mattk);
    }
}

/*
 *  fightm()  -- fight some other monster
 *
 *  Returns:
 *      0 - Monster did nothing.
 *      1 - If the monster made an attack.  The monster might have died.
 *
 *  There is an exception to the above.  If mtmp has the hero swallowed,
 *  then we report that the monster did nothing so it will continue to
 *  digest the hero.
 */
 /* have monsters fight each other */
int
fightm(struct monst *mtmp)
{
    struct monst *mon, *nmon;
    int result, has_u_swallowed;
    /* perhaps the monster will resist Conflict */
    if (resist_conflict(mtmp))
        return 0;

    if (u.ustuck == mtmp) {
        /* perhaps we're holding it... */
        if (itsstuck(mtmp))
            return 0;
    }
    has_u_swallowed = engulfing_u(mtmp);

    for (mon = fmon; mon; mon = nmon) {
        nmon = mon->nmon;
        if (nmon == mtmp)
            nmon = mtmp->nmon;
        /* Be careful to ignore monsters that are already dead, since we
         * might be calling this before we've cleaned them up.  This can
         * happen if the monster attacked a cockatrice bare-handedly, for
         * instance.
         */
        if (mon != mtmp && !DEADMONSTER(mon)) {
            if (monnear(mtmp, mon->mx, mon->my)) {
                if (!u.uswallow && (mtmp == u.ustuck)) {
                    if (!rn2(4)) {
                        set_ustuck((struct monst *) 0);
/*JP
                        pline("%s releases you!", Monnam(mtmp));
*/
                        pline("%sはあなたを解放した！", Monnam(mtmp));
                    } else
                        break;
                }

                /* mtmp can be killed */
                gb.bhitpos.x = mon->mx, gb.bhitpos.y = mon->my;
                gn.notonhead = FALSE;
                result = mattackm(mtmp, mon);

                if (result & M_ATTK_AGR_DIED)
                    return 1; /* mtmp died */
                /*
                 * If mtmp has the hero swallowed, lie and say there
                 * was no attack (this allows mtmp to digest the hero).
                 */
                if (has_u_swallowed)
                    return 0;

                /* allow attacked monsters a chance to hit back, primarily
                   to allow monsters that resist conflict to respond */
                if ((result & (M_ATTK_HIT | M_ATTK_DEF_DIED)) == M_ATTK_HIT
                    && rn2(4) && mon->movement > rn2(NORMAL_SPEED)) {
                    if (mon->movement > NORMAL_SPEED)
                        mon->movement -= NORMAL_SPEED;
                    else
                        mon->movement = 0;
                    gb.bhitpos.x = mtmp->mx, gb.bhitpos.y = mtmp->my;
                    gn.notonhead = FALSE;
                    (void) mattackm(mon, mtmp); /* return attack */
                }

                return (result & M_ATTK_HIT) ? 1 : 0;
            }
        }
    }
    return 0;
}

/*
 * mdisplacem() -- attacker moves defender out of the way;
 *                 returns same results as mattackm().
 */
int
mdisplacem(
    struct monst *magr,
    struct monst *mdef,
    boolean quietly)
{
    struct permonst *pa, *pd;
    int tx, ty, fx, fy;

    /* sanity checks; could matter if we unexpectedly get a long worm */
    if (!magr || !mdef || magr == mdef)
        return M_ATTK_MISS;
    pa = magr->data, pd = mdef->data;
    tx = mdef->mx, ty = mdef->my; /* destination */
    fx = magr->mx, fy = magr->my; /* current location */
    if (m_at(fx, fy) != magr || m_at(tx, ty) != mdef)
        return M_ATTK_MISS;

    /* The 1 in 7 failure below matches the chance in do_attack()
     * for pet displacement.
     */
    if (!rn2(7))
        return M_ATTK_MISS;

    /* Grid bugs cannot displace at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
        && magr->my != mdef->my)
        return M_ATTK_MISS;

    /* undetected monster becomes un-hidden if it is displaced */
    if (mdef->mundetected)
        mdef->mundetected = 0;
    if (M_AP_TYPE(mdef) && M_AP_TYPE(mdef) != M_AP_MONSTER)
        seemimic(mdef);
    /* wake up the displaced defender */
    mdef->msleeping = 0;
    mdef->mstrategy &= ~STRAT_WAITMASK;
    finish_meating(mdef);

    /*
     * Set up the visibility of action.
     * You can observe monster displacement if you can see both of
     * the monsters involved.
     */
    gv.vis = (canspotmon(magr) && canspotmon(mdef));

    if (touch_petrifies(pd) && !resists_ston(magr)) {
        if (!which_armor(magr, W_ARMG)) {
            if (poly_when_stoned(pa)) {
                mon_to_stone(magr);
                return M_ATTK_HIT; /* no damage during the polymorph */
            }
            if (!quietly && canspotmon(magr)) {
                if (gv.vis) {
                    pline("%s tries to move %s out of %s way.", Monnam(magr),
                          mon_nam(mdef), is_rider(pa) ? "the" : mhis(magr));
                }
/*JP
                pline_mon(magr, "%s turns to stone!", Monnam(magr));
*/
                pline_mon(magr, "%sは石になった！", Monnam(magr));
            }
            monstone(magr);
            if (!DEADMONSTER(magr))
                return M_ATTK_HIT; /* lifesaved */
            else if (magr->mtame && !gv.vis)
/*JP
                You(brief_feeling, "peculiarly sad");
*/
                You(brief_feeling, "もの悲しい");
            return M_ATTK_AGR_DIED;
        }
    }

    remove_monster(fx, fy); /* pick up from orig position */
    if (mdef->wormno)
        remove_worm(mdef);
    else
        remove_monster(tx, ty);
    place_monster(magr, tx, ty); /* put down at target spot */
    place_monster(mdef, fx, fy);
    if (mdef->wormno) /* now put down tail */
        place_worm_tail_randomly(mdef, fx, fy);
    /* either creature might move into or out of a poison gas cloud */
    update_monster_region(magr);
    update_monster_region(mdef);

    if (gv.vis && !quietly)
#if 0 /*JP:T*/
        pline("%s moves %s out of %s way!", Monnam(magr), mon_nam(mdef),
              is_rider(pa) ? "the" : mhis(magr));
#else
        pline("%sは%sを押しのけた！", Monnam(magr), mon_nam(mdef));
#endif
    newsym(fx, fy);  /* see it       */
    newsym(tx, ty);  /*   all happen */
    flush_screen(0); /* make sure it shows up */

    return M_ATTK_HIT;
}

/*
 * mattackm() -- a monster attacks another monster.
 *
 *          --------- aggressor died
 *         /  ------- defender died
 *        /  /  ----- defender was hit
 *       /  /  /
 *      x  x  x
 *
 *      0x8     M_ATTK_AGR_DONE
 *      0x4     M_ATTK_AGR_DIED
 *      0x2     M_ATTK_DEF_DIED
 *      0x1     M_ATTK_HIT
 *      0x0     M_ATTK_MISS
 *
 * Each successive attack has a lower probability of hitting.  Some rely on
 * success of previous attacks.  ** this doesn't seem to be implemented -dl **
 *
 * Attacker has targeted <bhitpos.x,bhitpos.y> rather than
 * <mdef->mx,mdef->my>; matters for long worms.
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(
    struct monst *magr,
    struct monst *mdef)
{
    int i,          /* loop counter */
        tmp,        /* armor class difference */
        strike = 0, /* hit this attack */
        attk,       /* attack attempted this time */
        struck = 0, /* hit at least once */
        res[NATTK], /* results of all attacks */
        dieroll = 0;
    struct attack *mattk, alt_attk;
    struct obj *mwep;
    struct permonst *pa, *pd;

    if (!magr || !mdef)
        return M_ATTK_MISS; /* mike@genat */
    if (helpless(magr))
        return M_ATTK_MISS;
    pa = magr->data;
    pd = mdef->data;

    /* Grid bugs cannot attack at an angle. */
    if (pa == &mons[PM_GRID_BUG] && magr->mx != mdef->mx
        && magr->my != mdef->my)
        return M_ATTK_MISS;

    /* Calculate the armour class differential. */
    tmp = find_mac(mdef) + magr->m_lev;
    if (mdef->mconf || helpless(mdef)) {
        tmp += 4;
        mdef->msleeping = 0;
    }

    /* mundetected monsters become un-hidden if they are attacked */
    if (mdef->mundetected) {
        mdef->mundetected = 0;
        newsym(mdef->mx, mdef->my);
        if (canseemon(mdef) && !sensemon(mdef)) {
            if (Unaware) {
#if 0 /*JP*/
                boolean justone = (mdef->data->geno & G_UNIQ) != 0L;
                const char *montype;

                montype = noname_monnam(mdef, justone ? ARTICLE_THE
                                                      : ARTICLE_NONE);
                if (!justone)
                    montype = makeplural(montype);
                You("dream of %s.", montype);
#else
                You("%sの夢を見た．", a_monnam(mdef));
#endif
            } else {
                if (iflags.last_msg == PLNMSG_HIDE_UNDER
                    && mdef->m_id == gl.last_hider)
                    pline_mon(mdef, "%s emerges from hiding.", Monnam(mdef));
                else if (mdef->m_id == gl.last_hider)
                    You("notice %s.", mon_nam(mdef));
                else
/*JP
                    pline("Suddenly, you notice %s.", a_monnam(mdef));
*/
                    pline("突然，あなたは%sに気がついた．", a_monnam(mdef));
            }
        }
    }

    /* Elves hate orcs. */
    if (is_elf(pa) && is_orc(pd))
        tmp++;

    /* Set up the visibility of action */
    gv.vis = ((cansee(magr->mx, magr->my) && canspotmon(magr))
              || (cansee(mdef->mx, mdef->my) && canspotmon(mdef)));

    /* Set flag indicating monster has moved this turn.  Necessary since a
     * monster might get an attack out of sequence (i.e. before its move) in
     * some cases, in which case this still counts as its move for the round
     * and it shouldn't move again.
     */
    magr->mlstmv = svm.moves;

    /* controls whether a mind flayer uses all of its tentacle-for-DRIN
       attacks; when fighting a headless monster, stop after the first
       one because repeating the same failing hit (or even an ordinary
       tentacle miss) is very verbose and makes the flayer look stupid */
    gs.skipdrin = FALSE;

    /* Now perform all attacks for the monster. */
    for (i = 0; i < NATTK; i++) {
        res[i] = M_ATTK_MISS;

        /* target might no longer be there */
        if (i > 0 && (m_at(gb.bhitpos.x, gb.bhitpos.y) != mdef
                      || DEADMONSTER(magr) || DEADMONSTER(mdef)))
            continue;

        mattk = getmattk(magr, mdef, i, res, &alt_attk);
        /* reduce verbosity for mind flayer attacking creature without a
           head (or worm's tail); this is similar to monster with multiple
           attacks after a wildmiss against displaced or invisible hero */
        if (gs.skipdrin && mattk->aatyp == AT_TENT && mattk->adtyp == AD_DRIN)
            continue;
        mwep = (struct obj *) 0;
        attk = 1;

        switch (mattk->aatyp) {
        case AT_WEAP: /* "hand to hand" attacks */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1) {
                /* D: Do a ranged attack here! */
                strike = (thrwmm(magr, mdef) == M_ATTK_MISS) ? 0 : 1;
                if (strike)
                    /* don't really know if we hit or not; pretend we did */
                    res[i] |= M_ATTK_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = M_ATTK_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
                break;
            }
            if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
                magr->weapon_check = NEED_HTH_WEAPON;
                if (mon_wield_item(magr) != 0)
                    return M_ATTK_MISS;
            }
            possibly_unwield(magr, FALSE);
            if ((mwep = MON_WEP(magr)) != 0) {
                if (gv.vis)
                    mswingsm(magr, mdef, mwep);
                tmp += hitval(mwep, mdef);
            }
            FALLTHROUGH;
            /*FALLTHRU*/
        case AT_CLAW:
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            if (mattk->aatyp == AT_KICK && mtrapped_in_pit(magr))
                    continue;
            /* Nymph that teleported away on first attack? */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1)
                /* Continue because the monster may have a ranged attack. */
                continue;
            /* Monsters won't attack cockatrices physically if they
             * have a weapon instead.  This instinct doesn't work for
             * players, or under conflict or confusion.
             */
            if (!magr->mconf && !Conflict && mwep && mattk->aatyp != AT_WEAP
                && touch_petrifies(mdef->data)) {
                strike = 0;
                break;
            }
            dieroll = rnd(20 + i);
            strike = (tmp > dieroll);
            /* KMH -- don't accumulate to-hit bonuses */
            if (mwep)
                tmp -= hitval(mwep, mdef);
            if (strike) {
                /* for eel AT_TUCH+AD_WRAP attack: can't grab an unsolid
                   target; the unsolid test is redundant since failed_grab
                   checks it too, but is cheap and avoids calling failed_grab
                   for ordinary targets */
                if (unsolid(mdef->data) && failed_grab(magr, mdef, mattk)) {
                    strike = 0;
                    break;
                }
                res[i] = hitmm(magr, mdef, mattk, mwep, dieroll);
                if ((mdef->data == &mons[PM_BLACK_PUDDING]
                     || mdef->data == &mons[PM_BROWN_PUDDING])
                    && (mwep && (objects[mwep->otyp].oc_material == IRON
                                 || objects[mwep->otyp].oc_material == METAL))
                    && mdef->mhp > 1 && !mdef->mcan) {
                    struct monst *mclone;

                    if ((mclone = clone_mon(mdef, 0, 0)) != 0) {
                        if (gv.vis && canspotmon(mdef))
#if 0 /*JP:T*/
                            pline("%s divides as %s hits it!",
                                  Monnam(mdef), mon_nam(magr));
#else
                            pline("%sの攻撃で%sが分裂した！",
                                  mon_nam(magr), Monnam(mdef));
#endif
                        (void) mintrap(mclone, NO_TRAP_FLAGS);
                        if (DEADMONSTER(magr))
                            res[i] |= M_ATTK_AGR_DIED;
                    }
                }
            } else
                missmm(magr, mdef, mattk);
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            strike = (i >= 2 && res[i - 1] == M_ATTK_HIT
                      && res[i - 2] == M_ATTK_HIT);
            if (strike) {
                /* note: monsters with hug attacks don't wear cloaks or gloves
                   so this doesn't need a special case for hugging a shade
                   while covered by blessed armor (which does damage but does
                   not achieve a successful hold); likewise, rope golems can't
                   wield weapons so ability to choke isn't affected by such */
                if (failed_grab(magr, mdef, mattk))
                    strike = 0;
                else
                    res[i] = hitmm(magr, mdef, mattk, (struct obj *) 0, 0);
            }
            break;

        case AT_GAZE:
            strike = 0;
            res[i] = gazemm(magr, mdef, mattk);
            break;

        case AT_EXPL:
            /* D: Prevent explosions from a distance */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1)
                continue;

            res[i] = explmm(magr, mdef, mattk);
            if (res[i] == M_ATTK_MISS) { /* cancelled--no attack */
                strike = 0;
                attk = 0;
            } else
                strike = 1; /* automatic hit */
            break;

        case AT_ENGL:
            if (mdef->data == &mons[PM_SHADE]) { /* no silver teeth... */
                if (gv.vis)
#if 0 /*JP:T*/
                    pline("%s attempt to engulf %s is futile.",
                          s_suffix(Monnam(magr)), mon_nam(mdef));
#else
                    pline("%sは%sを飲み込もうとしたがむだだった．",
                          Monnam(magr), mon_nam(mdef));
#endif
                strike = 0;
                break;
            }
            if (u.usteed && mdef == u.usteed) {
                strike = 0;
                break;
            }
            /* D: Prevent engulf from a distance */
            if (distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1)
                continue;
            /* Engulfing attacks are directed at the hero if possible. -dlc */
            if (engulfing_u(magr)) {
                strike = 0;
            } else if ((strike = (tmp > rnd(20 + i))) != 0) {
                if (failed_grab(magr, mdef, mattk))
                    strike = 0; /* purple worm can't swallow unsolid mons */
                else
                    res[i] = gulpmm(magr, mdef, mattk);
            } else {
                missmm(magr, mdef, mattk);
            }
            break;

        case AT_BREA:
        case AT_SPIT:
            /*
             * Ranged attacks aren't allowed at point blank range.
             *
             * That impacts pet use of ranged attacks.  It's rather arbitrary
             * but various parts of the code assume it to be the case, not to
             * mention a part of player tactics when fighting dragons.
             */
            if (!monnear(magr, mdef->mx, mdef->my)) {
                int mmtmp = ((mattk->aatyp == AT_BREA)
                             ? breamm(magr, mattk, mdef)
                             : spitmm(magr, mattk, mdef));

                strike = (mmtmp == M_ATTK_MISS) ? 0 : 1;
                /* We don't really know if we hit or not; pretend we did. */
                if (strike)
                    res[i] |= M_ATTK_HIT;
                if (DEADMONSTER(mdef))
                    res[i] = M_ATTK_DEF_DIED;
                if (DEADMONSTER(magr))
                    res[i] |= M_ATTK_AGR_DIED;
            } else {
                strike = 0;
                attk = 0;
            }
            break;

        default: /* no attack */
            strike = 0;
            attk = 0;
            break;
        }

        if (attk && !(res[i] & M_ATTK_AGR_DIED)
            && distmin(magr->mx, magr->my, mdef->mx, mdef->my) <= 1)
            res[i] = passivemm(magr, mdef, strike,
                               (res[i] & M_ATTK_DEF_DIED), mwep);

        if (res[i] & M_ATTK_DEF_DIED)
            return res[i];
        if (res[i] & M_ATTK_AGR_DIED)
            return res[i];
        /* return if aggressor can no longer attack */
        if ((res[i] & M_ATTK_AGR_DONE) || helpless(magr))
            return res[i];
        /* eg. defender was knocked into a level teleport trap */
        if (mon_offmap(mdef))
            return res[i];
        if (res[i] & M_ATTK_HIT)
            struck = 1; /* at least one hit */
    } /* for (;i < NATTK;) loop */

    return (struck ? M_ATTK_HIT : M_ATTK_MISS);
}

/* can't hold an unsolid target (ghosts, lights, vortices, most elementals)
   or a long worm tail */
boolean
failed_grab(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk)
{
    if ((unsolid(mdef->data) || gn.notonhead)
        /* hug attack: most holders (owlbear, python, pit fiend, &c);
           wrap damage: eel grabbing, trapper/lurker-above engulfing;
           stick-to damage: mimic, lichen;
           digestion damage: purple worm swallowing */
        && (mattk->aatyp == AT_HUGS || mattk->adtyp == AD_WRAP
            || mattk->adtyp == AD_STCK  || mattk->adtyp == AD_DGST)) {
        if ((gv.vis && canspotmon(mdef)) /* mon-vs-mon */
            || magr == &gy.youmonst || mdef == &gy.youmonst) {
            char magrnam[BUFSZ], mdefnam[BUFSZ];
            boolean tailmiss = gn.notonhead;
            const char *verb = (mattk->adtyp == AD_DGST) ? "gulp"
                               : (mattk->adtyp == AD_STCK) ? "adhere"
                                 : "grab";

            /* beware of "Foo's grab passes through Bar's ghost";
               mon_nam(x_monnam) calls s_suffix() for named ghosts and
               s_suffix() uses a single static buffer; make copies of both
               names to overcome that [note: comment predates 'tailmiss'] */
            Strcpy(magrnam, (magr == &gy.youmonst) ? "Your"
                                                   : s_suffix(Monnam(magr)));
            if (!tailmiss) {
                Strcpy(mdefnam, (mdef == &gy.youmonst) ? "you"
                                                       : mon_nam(mdef));
            } else {
                /* hero poly'd into long worm can't grow tail
                   so no 'youmonst' handling is needed here */
                Sprintf(mdefnam, "%s tail", s_suffix(some_mon_nam(mdef)));
            }
            /* unsolid grab misses are actually somewhat iffy--how come
               ordinary attacks don't also pass right through? */
            pline("%.99s %s attempt %s %.99s!", magrnam, verb,
                  !tailmiss ? "passes right through" : "fails to hold",
                  mdefnam);
        }
        return TRUE;
    }
    return FALSE;
}

/* Returns the result of mdamagem(). */
staticfn int
hitmm(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk,
    struct obj *mwep,
    int dieroll)
{
    int compat;
    boolean weaponhit = (mattk->aatyp == AT_WEAP
                         || (mattk->aatyp == AT_CLAW && mwep)),
            silverhit = (weaponhit && mwep
                         && objects[mwep->otyp].oc_material == SILVER);

    pre_mm_attack(magr, mdef);

    compat = !magr->mcan ? could_seduce(magr, mdef, mattk) : 0;
    if (!compat && shade_miss(magr, mdef, mwep, FALSE, gv.vis))
        return M_ATTK_MISS; /* bypass mdamagem() */

    if (gv.vis) {
        char buf[BUFSZ], magr_name[BUFSZ];

        Strcpy(magr_name, Monnam(magr));
        if (compat) {
#if 0 /*JP:T*/
            Snprintf(buf, sizeof buf, "%s %s", magr_name,
                    mdef->mcansee ? "smiles at" : "talks to");
            pline("%s %s %s.", buf, mon_nam(mdef),
                  (compat == 2) ? "engagingly" : "seductively");
#else
            /*magr_name自体に'%'が含まれていると壊れるのでエスケープ*/
            (void) strNsubst(magr_name, "%", "%%", 0);
            Snprintf(buf, sizeof buf, "%sは%%sに%%s%s．", magr_name,
                    mdef->mcansee ? "微笑みかけた" : "話しかけた");
            pline(buf, mon_nam(mdef),
                  (compat == 2) ? "魅力的に" : "誘惑的に");
#endif
        } else {
            buf[0] = '\0';
            switch (mattk->aatyp) {
            case AT_BITE:
#if 0 /*JP*/
                Snprintf(buf, sizeof buf, "%s bites", magr_name);
#else
                Snprintf(buf, sizeof buf, "は%sに噛みついた", mon_nam_too(mdef, magr));
#endif
                break;
            case AT_STNG:
#if 0 /*JP*/
                Snprintf(buf, sizeof buf, "%s stings", magr_name);
#else
                Snprintf(buf, sizeof buf, "は%sを突きさした", mon_nam_too(mdef, magr));
#endif
                break;
            case AT_BUTT:
#if 0 /*JP*/
                Snprintf(buf, sizeof buf, "%s butts", magr_name);
#else
                Snprintf(buf, sizeof buf, "は%sに頭突きをくらわした", mon_nam_too(mdef, magr));
#endif
                break;
            case AT_TUCH:
#if 0 /*JP*/
                Snprintf(buf, sizeof buf, "%s touches", magr_name);
#else
                Snprintf(buf, sizeof buf, "は%sに触れた", mon_nam_too(mdef, magr));
#endif
                break;
            case AT_TENT:
#if 0 /*JP*/
                Snprintf(buf, sizeof buf, "%s tentacles suck",
                         s_suffix(magr_name));
#else
                Snprintf(buf, sizeof buf, "の触手が%sの体液を吸いとった",
                         mon_nam_too(mdef, magr));
#endif
                break;
            case AT_HUGS:
                if (magr != u.ustuck) {
#if 0 /*JP*/
                    Snprintf(buf, sizeof buf, "%s squeezes", magr_name);
#else
                    Snprintf(buf, sizeof buf, "は%sを絞めた", mon_nam_too(mdef, magr));
#endif
                    break;
                }
                FALLTHROUGH;
                /*FALLTHRU*/
            default:
                if (!weaponhit || !mwep || !mwep->oartifact)
#if 0 /*JP*/
                    Snprintf(buf, sizeof buf, "%s hits", magr_name);
#else
                    Snprintf(buf, sizeof buf, "の%sへの攻撃は命中した", mon_nam_too(mdef, magr));
#endif
                break;
            }
            if (*buf)
#if 0 /*JP*/
                pline("%s %s.", buf, mon_nam_too(mdef, magr));
#else
                pline("%s%s．", magr_name, buf);
#endif

            if (mon_hates_silver(mdef) && silverhit) {
                char *mdef_name = mon_nam_too(mdef, magr);

                /* note: mon_nam_too returns a modifiable buffer; so
                   does s_suffix, but it returns a single static buffer
                   and we might be calling it twice for this message */
                Strcpy(magr_name, s_suffix(magr_name));
                if (!noncorporeal(mdef->data) && !amorphous(mdef->data)) {
                    if (mdef != magr) {
                        mdef_name = s_suffix(mdef_name);
                    } else {
                        (void) strsubst(mdef_name, "himself", "his own");
                        (void) strsubst(mdef_name, "herself", "her own");
                        (void) strsubst(mdef_name, "itself", "its own");
                    }
/*JP
                    Strcat(mdef_name, " flesh");
*/
                    Strcat(mdef_name, "の肉");
                }

#if 0 /*JP:T*/
                pline("%s %s sears %s!", magr_name, /* s_suffix(magr_name), */
                      simpleonames(mwep), mdef_name);
#else
                pline("%s%sが%sを焼いた！", magr_name, /* s_suffix(magr_name), */
                      simpleonames(mwep), mdef_name);
#endif
            }
        }
    } else
        noises(magr, mattk);

    return mdamagem(magr, mdef, mattk, mwep, dieroll);
}

/* Returns the same values as mdamagem(). */
staticfn int
gazemm(struct monst *magr, struct monst *mdef, struct attack *mattk)
{
    char buf[BUFSZ];
    /* an Archon's gaze affects target even if Archon itself is blinded */
    boolean archon = (magr->data == &mons[PM_ARCHON]
                      && mattk->adtyp == AD_BLND),
            altmesg = (archon && !magr->mcansee);

    /* bring target out of hiding even if hero doesn't see it happen (this
       is already done in pre_mm_attack() and shouldn't be needed here) */
    if (mdef->data->mlet == S_MIMIC && M_AP_TYPE(mdef) != M_AP_NOTHING)
        seemimic(mdef);
    mdef->mundetected = 0;

    if (gv.vis) {
#if 0 /*JP:T*/
        Sprintf(buf, "%s gazes %s",
                altmesg ? Adjmonnam(magr, "blinded") : Monnam(magr),
                altmesg ? "toward" : "at");
        pline("%s %s...", buf,
              canspotmon(mdef) ? mon_nam(mdef) : "something");
#else
        char namebuf[BUFSZ];
        /*この文字列自体に'%'が含まれていると壊れるのでエスケープ*/
        Strcpy(namebuf, altmesg ? Adjmonnam(magr, "目の見えない") : Monnam(magr));
        (void) strNsubst(namebuf, "%", "%%", 0);
        Sprintf(buf, "%sは%%s%sをにらみつけた．．．",
                namebuf,
                altmesg ? "の方" : "");
        pline(buf, canspotmon(mdef) ? mon_nam(mdef) : "何か");
#endif
    }

    if (magr->mcan || !mdef->mcansee
        || (archon ? resists_blnd(mdef) : !magr->mcansee)
        || (magr->minvis && !perceives(mdef->data)) || mdef->msleeping) {
        if (gv.vis && canspotmon(mdef))
/*JP
            pline("but nothing happens.");
*/
            pline("しかし何もおこらなかった．");
        return M_ATTK_MISS;
    }
    /* call mon_reflects 2x, first test, then, if visible, print message */
    if (magr->data == &mons[PM_MEDUSA] && mon_reflects(mdef, (char *) 0)) {
        if (canseemon(mdef))
/*JP
            (void) mon_reflects(mdef, "The gaze is reflected away by %s %s.");
*/
            (void) mon_reflects(mdef, "にらみは%sの%sで反射した．");
        if (mdef->mcansee) {
            if (mon_reflects(magr, (char *) 0)) {
                if (canseemon(magr))
#if 0 /*JP:T*/
                    (void) mon_reflects(magr,
                                      "The gaze is reflected away by %s %s.");
#else
                    (void) mon_reflects(magr,
                                      "にらみは%sの%sで反射した．");
#endif
                return M_ATTK_MISS;
            }
            if (mdef->minvis && !perceives(magr->data)) {
                if (canseemon(magr)) {
#if 0 /*JP:T*/
                    pline(
                      "%s doesn't seem to notice that %s gaze was reflected.",
                          Monnam(magr), mhis(magr));
#else
                    pline("にらみが反射していることに%sは気付いていないようだ．",
                          Monnam(magr));
#endif
                }
                return M_ATTK_MISS;
            }
            if (canseemon(magr))
/*JP
                pline_mon(magr, "%s is turned to stone!", Monnam(magr));
*/
                pline_mon(magr, "%sは石になった！", Monnam(magr));
            monstone(magr);
            if (!DEADMONSTER(magr))
                return M_ATTK_MISS;
            return M_ATTK_AGR_DIED;
        }
    } else if (archon) {
        mhitm_ad_blnd(magr, mattk, mdef, (struct mhitm_data *) 0);
        /* an Archon's blinding radiance also stuns;
           this is different from the way the hero gets stunned because
           a stunned monster recovers randomly instead of via countdown;
           both cases make an effort to prevent the target from being
           continuously stunned due to repeated gaze attacks */
        if (rn2(2))
            mdef->mstun = 1;
    }

    return mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
}

/* return True if magr is allowed to swallow mdef, False otherwise */
boolean
engulf_target(struct monst *magr, struct monst *mdef)
{
    struct rm *lev;
    int ax, ay, dx, dy;
    boolean uatk = (magr == &gy.youmonst),
            udef = (mdef == &gy.youmonst);

    /* can't swallow something that's too big */
    if (mdef->data->msize >= MZ_HUGE
        || (magr->data->msize < mdef->data->msize && !is_whirly(magr->data)))
        return FALSE;

    /* can't (move to) swallow if trapped. TODO: could do some? */
    if (mdef->mtrapped || magr->mtrapped)
        return FALSE;

    /* if attacker is phasing in solid rock and defender can't move there,
       or vice versa, don't allow engulf to succeed; otherwise expelling
       might not be able to place attacker and defender both back on map;
       when defender is the hero, a sanity_check complaint about placing
       the hero on top of a monster can occur */
    dx = (mdef == &gy.youmonst) ? u.ux : mdef->mx;
    dy = (mdef == &gy.youmonst) ? u.uy : mdef->my;
    lev = &levl[dx][dy];
    if (!(udef ? Passes_walls : passes_walls(mdef->data))
          && (IS_OBSTRUCTED(lev->typ) || closed_door(dx, dy) || IS_TREE(lev->typ)
              /* not passes_bars(); engulfer isn't squeezing through */
              || (lev->typ == IRONBARS && !is_whirly(magr->data))))
        return FALSE;
    ax = (magr == &gy.youmonst) ? u.ux : magr->mx;
    ay = (magr == &gy.youmonst) ? u.uy : magr->my;
    lev = &levl[ax][ay];
    if (!(uatk ? Passes_walls : passes_walls(magr->data))
        && (IS_OBSTRUCTED(lev->typ) || closed_door(ax, ay) || IS_TREE(lev->typ)
            || (lev->typ == IRONBARS && !is_whirly(mdef->data))))
        return FALSE;

    return TRUE;
}

/* Returns the same values as mattackm(). */
staticfn int
gulpmm(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk)
{
    coordxy ax, ay, dx, dy;
    int status;
    struct obj *obj;

    if (!engulf_target(magr, mdef))
        return M_ATTK_MISS;

    if (gv.vis) {
#if 0 /*JP:T*/
        pline("%s %s %s.", Monnam(magr),
              digests(magr->data) ? "swallows"
              : enfolds(magr->data) ? "encloses"
                : "engulfs",
              mon_nam(mdef));
#else /*訳し分けるまではしない*/
        pline("%sは%sをぐっと飲みこんだ．", Monnam(magr),
              mon_nam(mdef));
#endif
    }
    if (!flaming(magr->data)) {
        for (obj = mdef->minvent; obj; obj = obj->nobj)
            (void) snuff_lit(obj);
    }

    if (is_vampshifter(mdef)
        && newcham(mdef, &mons[mdef->cham], NO_NC_FLAGS)) {
        if (gv.vis) {
            /* 'it' -- previous form is no longer available and
               using that would be excessively verbose */
#if 0 /*JP:T*/
            pline("%s expels %s.", Monnam(magr),
                  canspotmon(mdef) ? "it" : something);
#else
            pline("%sは%sを吐き出した．", Monnam(magr),
                  canspotmon(mdef) ? "それ" : something);
#endif
            if (canspotmon(mdef)) {
/*JP
                pline("It turns into %s.",
*/
                pline("それは%sになった．",
                      x_monnam(mdef, ARTICLE_A, (char *) 0,
                               (SUPPRESS_NAME | SUPPRESS_IT
                                | SUPPRESS_INVISIBLE), FALSE));
            }
        }
        return M_ATTK_HIT; /* bypass mdamagem() */
    }

    /*
     *  All of this manipulation is needed to keep the display correct.
     *  There is a flush at the next pline().
     */
    ax = magr->mx;
    ay = magr->my;
    dx = mdef->mx;
    dy = mdef->my;
    /*
     *  Leave the defender in the monster chain at its current position,
     *  but don't leave it on the screen.  Move the aggressor to the
     *  defender's position.
     */
    remove_monster(dx, dy);
    remove_monster(ax, ay);
    place_monster(magr, dx, dy);
    newsym(ax, ay); /* erase old position */
    newsym(dx, dy); /* update new position */

    gm.mswallower = magr; /* corpse_chance() wants this */
    status = mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
    gm.mswallower = (struct monst *) 0; /* reset */

    if ((status & (M_ATTK_AGR_DIED | M_ATTK_DEF_DIED))
        == (M_ATTK_AGR_DIED | M_ATTK_DEF_DIED)) {
        ;                              /* both died -- do nothing  */
    } else if (status & M_ATTK_DEF_DIED) { /* defender died */
        /*
         *  Note: mdamagem() -> monkilled() -> mondead() -> m_detach()
         *  -> relmon() used to call remove_monster() for the dead
         *  monster even when it wasn't the one on the map, so we
         *  needed to put magr back after mdef was killed and removed
         *  from their shared spot.  But now [5.0] relmon() calls
         *  mon_leaving_level() and that checks whether the monster at
         *  dying monster's coordinates is that dying monster and only
         *  removes it when they match.  So magr is still at mdef's
         *  former spot these days.
         *
         *  We still potentially do one fixup:  if the gulp targeted
         *  an inhospitable location, magr will return to its previous
         *  spot instead of staying.
         */
        if (!goodpos(dx, dy, magr, MM_IGNOREWATER)) {
            if (m_at(dx, dy) == magr) {
                remove_monster(dx, dy);
                newsym(dx, dy);
            }
            dx = ax, dy = ay; /* magr's spot at start of the attack */
        }
        if (m_at(dx, dy) != magr) {
            place_monster(magr, dx, dy);
            newsym(dx, dy);
        }
        /* aggressor moves to <dx,dy> and might encounter trouble there */
        if (minliquid(magr)
            || (t_at(dx, dy)
                && mintrap(magr, NO_TRAP_FLAGS) == Trap_Killed_Mon))
            status |= M_ATTK_AGR_DIED;
    } else if (status & M_ATTK_AGR_DIED) { /* aggressor died */
        place_monster(mdef, dx, dy);
        newsym(dx, dy);
    } else {                           /* both alive, put them back */
        if (cansee(dx, dy)) {
#if 0 /*JP:T*/
            pline("%s is %s!", Monnam(mdef),
                  digests(magr->data) ? "regurgitated"
                    : enfolds(magr->data) ? "released"
                      : "expelled");
#else
            pline("%sは%s！", Monnam(mdef),
                  digests(magr->data) ? "吐き戻された"
                    : enfolds(magr->data) ? "解放された"
                      : "吐き出された");
#endif
        }

        remove_monster(dx,dy);
        place_monster(magr, ax, ay);
        place_monster(mdef, dx, dy);
        newsym(ax, ay);
        newsym(dx, dy);
    }

    return status;
}

staticfn int
explmm(struct monst *magr, struct monst *mdef, struct attack *mattk)
{
    int result;

    if (magr->mcan)
        return M_ATTK_MISS;

    if (cansee(magr->mx, magr->my))
/*JP
        pline_mon(magr, "%s explodes!", Monnam(magr));
*/
        pline_mon(magr, "%sは爆発した！", Monnam(magr));
    else
        noises(magr, mattk);

    /* monster explosion types which actually create an explosion */
    if (mattk->adtyp == AD_FIRE || mattk->adtyp == AD_COLD
        || mattk->adtyp == AD_ELEC) {
        mon_explodes(magr, mattk);
        /* unconditionally set AGR_DIED here; lifesaving is accounted below */
        result = M_ATTK_AGR_DIED | (DEADMONSTER(mdef) ? M_ATTK_DEF_DIED : 0);
    } else {
        result = mdamagem(magr, mdef, mattk, (struct obj *) 0, 0);
    }

    /* Kill off aggressor if it didn't die. */
    if (!(result & M_ATTK_AGR_DIED)) {
        boolean was_leashed = (magr->mleashed != 0);

        mondead(magr);
        if (!DEADMONSTER(magr))
            return result; /* life saved */
        result |= M_ATTK_AGR_DIED;

        /* mondead() -> m_detach() -> m_unleash() always suppresses
           the m_unleash() slack message, so deliver it here instead */
        if (was_leashed)
/*JP
            Your("leash falls slack.");
*/
            Your("紐がたるんで落ちた．");
    }
    if (magr->mtame) /* give this one even if it was visible */
/*JP
        You(brief_feeling, "melancholy");
*/
        You(brief_feeling, "憂うつな");

    return result;
}

/*
 *  See comment at top of mattackm(), for return values.
 */
staticfn int
mdamagem(
    struct monst *magr,
    struct monst *mdef,
    struct attack *mattk,
    struct obj *mwep,
    int dieroll)
{
    struct permonst *pa = magr->data, *pd = mdef->data;
    struct mhitm_data mhm;
    mhm.damage = d((int) mattk->damn, (int) mattk->damd);
    mhm.hitflags = M_ATTK_MISS;
    mhm.permdmg = 0;
    mhm.specialdmg = 0;
    mhm.dieroll = dieroll;
    mhm.done = FALSE;

    if ((touch_petrifies(pd) /* or flesh_petrifies() */
         || (mattk->adtyp == AD_DGST && pd == &mons[PM_MEDUSA]))
        && !resists_ston(magr)) {
        long protector = attk_protection((int) mattk->aatyp),
             wornitems = magr->misc_worn_check;

        /* wielded weapon gives same protection as gloves here */
        if (mwep)
            wornitems |= W_ARMG;

        if (protector == 0L
            || (protector != ~0L && (wornitems & protector) != protector)) {
            if (poly_when_stoned(pa)) {
                mon_to_stone(magr);
                return M_ATTK_HIT; /* no damage during the polymorph */
            }
            if (gv.vis && canspotmon(magr))
                pline_mon(magr, "%s turns to stone!", Monnam(magr));
            monstone(magr);
            if (!DEADMONSTER(magr))
                return M_ATTK_HIT; /* lifesaved */
            else if (magr->mtame && !gv.vis)
/*JP
                You(brief_feeling, "peculiarly sad");
*/
                You(brief_feeling, "もの悲しい");
            return M_ATTK_AGR_DIED;
        }
    }

    mhitm_adtyping(magr, mattk, mdef, &mhm);

    if (mhitm_knockback(magr, mdef, mattk, &mhm.hitflags,
                        (MON_WEP(magr) != 0))
        && ((mhm.hitflags & (M_ATTK_DEF_DIED | M_ATTK_HIT)) != 0
            || mon_offmap(mdef)))
        return mhm.hitflags;

    if (mhm.done)
        return mhm.hitflags;

    if (!mhm.damage)
        return mhm.hitflags;

    mdef->mhp -= mhm.damage;
    if (mdef->mhp < 1) {
        if (m_at(mdef->mx, mdef->my) == magr) { /* see gulpmm() */
            remove_monster(mdef->mx, mdef->my);
            mdef->mhp = 1; /* otherwise place_monster will complain */
            place_monster(mdef, mdef->mx, mdef->my);
            mdef->mhp = 0;
        }
        if (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_CLAW)
            gm.mkcorpstat_norevive = troll_baned(mdef, mwep) ? TRUE : FALSE;
        gz.zombify = (!mwep && zombie_maker(magr)
                     && (mattk->aatyp == AT_TUCH
                         || mattk->aatyp == AT_CLAW
                         || mattk->aatyp == AT_BITE)
                     && zombie_form(mdef->data) != NON_PM);
        monkilled(mdef, "", (int) mattk->adtyp);
        gz.zombify = FALSE; /* reset */
        gm.mkcorpstat_norevive = FALSE;
        if (!DEADMONSTER(mdef))
            return mhm.hitflags; /* mdef lifesaved */
        else if (mhm.hitflags == M_ATTK_AGR_DIED)
            return (M_ATTK_DEF_DIED | M_ATTK_AGR_DIED);

        if (mattk->adtyp == AD_DGST) {
            /* various checks similar to dog_eat and meatobj.
             * after monkilled() to provide better message ordering */
            if (ismnum(mdef->cham)) {
                (void) newcham(magr, (struct permonst *) 0, NC_SHOW_MSG);
            } else if (pd == &mons[PM_GREEN_SLIME] && !slimeproof(pa)) {
                (void) newcham(magr, &mons[PM_GREEN_SLIME], NC_SHOW_MSG);
            } else if (pd == &mons[PM_WRAITH]) {
                (void) grow_up(magr, (struct monst *) 0);
                /* don't grow up twice */
                return (M_ATTK_DEF_DIED
                        | (!DEADMONSTER(magr) ? 0 : M_ATTK_AGR_DIED));
            } else if (pd == &mons[PM_NURSE]) {
                healmon(magr, magr->mhpmax, 0);
            }
            mon_givit(magr, pd);
        }
        /* caveat: above digestion handling doesn't keep `pa' up to date */

        return (M_ATTK_DEF_DIED
                | (grow_up(magr, mdef) ? 0 : M_ATTK_AGR_DIED));
    }
    return (mhm.hitflags == M_ATTK_AGR_DIED) ? M_ATTK_AGR_DIED : M_ATTK_HIT;
}

int
mon_poly(struct monst *magr, struct monst *mdef, int dmg)
{
    static const char freaky[] = " undergoes a freakish metamorphosis";
    struct permonst *oldform = mdef->data;

    if (mdef == &gy.youmonst) {
        if (Antimagic) {
            shieldeff(u.ux, u.uy);
        } else if (Unchanging) {
            ; /* just take a little damage */
        } else {
            /* system shock might take place in polyself() */
            if (u.ulycn == NON_PM) {
                You("are subjected to a freakish metamorphosis.");
                polyself(POLY_NOFLAGS);
            } else if (u.umonnum != u.ulycn) {
                You_feel("an unnatural urge coming on.");
                you_were();
            } else {
                You_feel("a natural urge coming on.");
                you_unwere(FALSE);
            }
            dmg = 0;
        }
    } else {
        char Before[BUFSZ];

        Strcpy(Before, Monnam(mdef));
        if (resists_magm(mdef)) {
            /* Magic resistance */
            if (gv.vis)
                shieldeff_mon(mdef);
        } else if (resist(mdef, WAND_CLASS, 0, TELL)) {
            /* general resistance to magic... */
            ;
        } else if (!rn2(25) && mdef->cham == NON_PM
                   && (mdef->mcan
                       || pm_to_cham(monsndx(mdef->data)) != NON_PM)) {
            /* system shock; this variation takes away half of mon's HP
               rather than kill outright */
            if (gv.vis)
                pline("%s shudders!", Before);

            dmg += (mdef->mhpmax + 1) / 2;
            mdef->mhp -= dmg;
            dmg = 0;
            if (DEADMONSTER(mdef)) {
                if (magr == &gy.youmonst)
                    xkilled(mdef, XKILL_GIVEMSG | XKILL_NOCORPSE);
                else
                    monkilled(mdef, "", AD_RBRE);
            }
        } else if (newcham(mdef, (struct permonst *) 0, NO_NC_FLAGS)) {
            if (gv.vis) { /* either seen or adjacent */
                boolean was_seen = !!strcmpi("It", Before),
                        verbosely = flags.verbose || !was_seen;

                if (canspotmon(mdef))
                    pline("%s%s%s turns into %s.", Before,
                          verbosely ? freaky : "", verbosely ? " and" : "",
                          x_monnam(mdef, ARTICLE_A, (char *) 0,
                                   (SUPPRESS_NAME | SUPPRESS_IT
                                    | SUPPRESS_INVISIBLE), FALSE));
                else if (was_seen || magr == &gy.youmonst)
                    pline("%s%s%s.", Before, freaky,
                          !was_seen ? "" : " and disappears");
            }
            dmg = 0;
            if (can_teleport(magr->data)) {
                if (magr == &gy.youmonst)
                    tele();
                else if (!tele_restrict(magr))
                    (void) rloc(magr, RLOC_MSG);
            }
        } else {
            if (gv.vis && flags.verbose)
                pline1(nothing_happens);
        }
    }
    /* when a transformation has happened, can't attack again for poly
       effect during next turn or two; not enforced for poly'd hero */
    if (mdef->data != oldform && magr != &gy.youmonst)
        magr->mspec_used += rnd(2);

    return dmg;
}

void
paralyze_monst(struct monst *mon, int amt)
{
    if (amt > 127)
        amt = 127;

    mon->mcanmove = 0;
    mon->mfrozen = amt;
    mon->meating = 0; /* terminate any meal-in-progress */
    mon->mstrategy &= ~STRAT_WAITFORU;
}

/* `mon' is hit by a sleep attack; return 1 if it's affected, 0 otherwise */
int
sleep_monst(struct monst *mon, int amt, int how)
{
    /* reveal mimic unless already asleep or paralyzed (won't be 'busy') */
    if (how >= 0 && !mon->msleeping && !mon->mfrozen
        && mon->data->mlet == S_MIMIC && (M_AP_TYPE(mon) == M_AP_FURNITURE
                                          || M_AP_TYPE(mon) == M_AP_OBJECT))
        seemimic(mon);

    if (resists_sleep(mon) || defended(mon, AD_SLEE)
        || (how >= 0 && resist(mon, (char) how, 0, NOTELL))) {
        shieldeff(mon->mx, mon->my);
    } else if (mon->mcanmove) {
        finish_meating(mon); /* terminate any meal-in-progress */
        amt += (int) mon->mfrozen;
        if (amt > 0) { /* sleep for N turns */
            mon->mcanmove = 0;
            mon->mfrozen = min(amt, 127);
        } else { /* sleep until awakened */
            mon->msleeping = 1;
        }
        return 1;
    }
    return 0;
}

/* sleeping grabber releases, engulfer doesn't; don't use for paralysis! */
void
slept_monst(struct monst *mon)
{
    if (helpless(mon) && mon == u.ustuck
        && !sticks(gy.youmonst.data) && !u.uswallow) {
/*JP
        pline_mon(mon, "%s grip relaxes.", s_suffix(Monnam(mon)));
*/
        pline_mon(mon, "%sの握る力が弱くなった．", Monnam(mon));
        unstuck(mon);
    }
}

void
rustm(struct monst *mdef, struct obj *obj)
{
    int dmgtyp = ERODE_NONE, chance = 1;

    if (!mdef || !obj)
        return; /* just in case */
    /* AD_ACID and AD_ENCH are handled in passivemm() and passiveum() */
    if (dmgtype(mdef->data, AD_CORR)) {
        dmgtyp = ERODE_CORRODE;
    } else if (dmgtype(mdef->data, AD_RUST)) {
        dmgtyp = ERODE_RUST;
    } else if (dmgtype(mdef->data, AD_FIRE)
               /* steam vortex: fire resist applies, fire damage doesn't */
               && mdef->data != &mons[PM_STEAM_VORTEX]) {
        dmgtyp = ERODE_BURN;
        chance = 6;
    }

    if (dmgtyp != ERODE_NONE && !rn2(chance))
        (void) erode_obj(obj, (char *) 0, dmgtyp, EF_GREASE | EF_VERBOSE);
}

staticfn void
mswingsm(
    struct monst *magr, /* attacker */
    struct monst *mdef, /* defender */
    struct obj *otemp)  /* attacker's weapon */
{
    if (flags.verbose && !Blind && mon_visible(magr)) {
        boolean bash = (is_pole(otemp) && !is_art(otemp, ART_SNICKERSNEE)
                        && (dist2(magr->mx, magr->my, mdef->mx, mdef->my)
                            <= 2));

#if 0 /*JP*/
        pline("%s %s %s%s %s at %s.", Monnam(magr), mswings_verb(otemp, bash),
              (otemp->quan > 1L) ? "one of " : "", mhis(magr), xname(otemp),
              mon_nam(mdef));
#else
        pline((objects[otemp->otyp].oc_dir & PIERCE) ?
              "%sは%sで%sを突いた．" :
              "%sは%sを振りまわし%sを攻撃した．", Monnam(magr),
              xname(otemp), mon_nam(mdef));
#endif
    }
}

/*
 * Passive responses by defenders.  Does not replicate responses already
 * handled above.  Returns same values as mattackm.
 */
staticfn int
passivemm(
    struct monst *magr,
    struct monst *mdef,
    boolean mhitb,
    int mdead,
    struct obj *mwep)
{
    struct permonst *mddat = mdef->data;
    struct permonst *madat = magr->data;
    char buf[BUFSZ];
    int i, tmp;
    int mhit = mhitb ? M_ATTK_HIT : M_ATTK_MISS;

    for (i = 0;; i++) {
        if (i >= NATTK)
            return (mdead | mhit); /* no passive attacks */
        if (mddat->mattk[i].aatyp == AT_NONE)
            break;
    }
    if (mddat->mattk[i].damn)
        tmp = d((int) mddat->mattk[i].damn, (int) mddat->mattk[i].damd);
    else if (mddat->mattk[i].damd)
        tmp = d((int) mddat->mlevel + 1, (int) mddat->mattk[i].damd);
    else
        tmp = 0;

    /* These affect the enemy even if defender killed */
    switch (mddat->mattk[i].adtyp) {
    case AD_ACID:
        if (mhitb && !rn2(2)) {
            Strcpy(buf, Monnam(magr));
            if (canseemon(magr))
#if 0 /*JP:T*/
                pline("%s is splashed by %s %s!", buf,
                      s_suffix(mon_nam(mdef)), hliquid("acid"));
#else
                pline("%sは%sの%sを浴びた！", buf,
                      mon_nam(mdef), hliquid("酸"));
#endif
            if (resists_acid(magr)) {
                if (canseemon(magr))
/*JP
                    pline("%s is not affected.", Monnam(magr));
*/
                    pline("%sは影響を受けない．", Monnam(magr));
                tmp = 0;
            }
        } else
            tmp = 0;
        if (!rn2(30))
            erode_armor(magr, ERODE_CORRODE);
        if (!rn2(6))
            acid_damage(MON_WEP(magr));
        goto assess_dmg;
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mhitb && !mdef->mcan && mwep) {
            (void) drain_item(mwep, FALSE);
            /* No message */
        }
        break;
    default:
        break;
    }
    if (mdead || mdef->mcan)
        return (mdead | mhit);

    /* These affect the enemy only if defender is still alive */
    if (rn2(3))
        switch (mddat->mattk[i].adtyp) {
        case AD_PLYS: /* Floating eye */
            if (tmp > 127)
                tmp = 127;
            if (mddat == &mons[PM_FLOATING_EYE]) {
                if (!rn2(4))
                    tmp = 127;
                if (magr->mcansee && haseyes(madat) && mdef->mcansee
                    && (perceives(madat) || !mdef->minvis)) {
                    /* construct format string; guard against '%' in Monnam */
#if 0 /*JP*/
                    Strcpy(buf, s_suffix(Monnam(mdef)));
#else
                    Strcpy(buf, Monnam(mdef));
#endif
                    (void) strNsubst(buf, "%", "%%", 0);
/*JP
                    Strcat(buf, " gaze is reflected by %s %s.");
*/
                    Strcat(buf, "のにらみは%sの%sによって反射した．");
                    if (mon_reflects(magr,
                                     canseemon(magr) ? buf : (char *) 0))
                        return (mdead | mhit);
                    Strcpy(buf, Monnam(magr));
                    if (canseemon(magr))
#if 0 /*JP:T*/
                        pline("%s is frozen by %s gaze!", buf,
                              s_suffix(mon_nam(mdef)));
#else
                        pline("%sは%sのにらみで動けなくなった！", buf,
                              mon_nam(mdef));
#endif
                    paralyze_monst(magr, tmp);
                    return (mdead | mhit);
                }
            } else { /* gelatinous cube */
                Strcpy(buf, Monnam(magr));
                if (canseemon(magr))
/*JP
                    pline("%s is frozen by %s.", buf, mon_nam(mdef));
*/
                    pline("%sは%sによって動けなくなった．", buf, mon_nam(mdef));
                paralyze_monst(magr, tmp);
                return (mdead | mhit);
            }
            return 1;
        case AD_COLD:
            if (resists_cold(magr)) {
                if (canseemon(magr)) {
/*JP
                    pline_mon(magr, "%s is mildly chilly.", Monnam(magr));
*/
                    pline_mon(magr, "%sはほんのり冷えた．", Monnam(magr));
                    golemeffects(magr, AD_COLD, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
/*JP
                pline_mon(magr, "%s is suddenly very cold!", Monnam(magr));
*/
                pline_mon(magr, "%sは突然凍りづけになった！", Monnam(magr));
            healmon(mdef, tmp/2, tmp/2);
            if (mdef->mhpmax > ((int) (mdef->m_lev + 1) * 8))
                (void) split_mon(mdef, magr);
            break;
        case AD_STUN:
            if (!magr->mstun) {
                magr->mstun = 1;
                if (canseemon(magr))
#if 0 /*JP:T*/
                    pline_mon(magr, "%s %s...", Monnam(magr),
                          makeplural(stagger(magr->data, "stagger")));
#else
                    pline_mon(magr, "%sは%s．．．", Monnam(magr),
                          jpast(stagger(magr->data, "よろめく")));
#endif
            }
            tmp = 0;
            break;
        case AD_FIRE:
            if (resists_fire(magr)) {
                if (canseemon(magr)) {
/*JP
                    pline_mon(magr, "%s is mildly warmed.", Monnam(magr));
*/
                    pline_mon(magr, "%sはほんのり暖かくなった．", Monnam(magr));
                    golemeffects(magr, AD_FIRE, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
/*JP
                pline_mon(magr, "%s is suddenly very hot!", Monnam(magr));
*/
                pline_mon(magr, "%sは突然とても熱くなった！", Monnam(magr));
            break;
        case AD_ELEC:
            if (resists_elec(magr)) {
                if (canseemon(magr)) {
/*JP
                    pline_mon(magr, "%s is mildly tingled.", Monnam(magr));
*/
                    pline_mon(magr, "%sはピリピリしている．", Monnam(magr));
                    golemeffects(magr, AD_ELEC, tmp);
                }
                tmp = 0;
                break;
            }
            if (canseemon(magr))
#if 0 /*JP:T*/
                pline_mon(magr, "%s is jolted with electricity!",
                          Monnam(magr));
#else
                pline_mon(magr, "%sは電気ショックをうけた！", Monnam(magr));
#endif
            break;
        default:
            tmp = 0;
            break;
        }
    else
        tmp = 0;

 assess_dmg:
    if ((magr->mhp -= tmp) <= 0) {
        monkilled(magr, "", (int) mddat->mattk[i].adtyp);
        return (mdead | mhit | M_ATTK_AGR_DIED);
    }
    return (mdead | mhit);
}

/* hero or monster has successfully hit target mon with drain energy attack */
void
xdrainenergym(struct monst *mon, boolean givemsg)
{
    if (mon->mspec_used < 20 /* limit draining */
        && (attacktype(mon->data, AT_MAGC)
            || attacktype(mon->data, AT_BREA))) {
        mon->mspec_used += d(2, 2);
        if (givemsg)
/*JP
            pline_mon(mon, "%s seems lethargic.", Monnam(mon));
*/
            pline_mon(mon, "%sは無気力になったようだ．", Monnam(mon));
    }
}

/* "aggressive defense"; what type of armor prevents specified attack
   from touching its target? */
long
attk_protection(int aatyp)
{
    long w_mask = 0L;

    switch (aatyp) {
    case AT_NONE:
    case AT_SPIT:
    case AT_EXPL:
    case AT_BOOM:
    case AT_GAZE:
    case AT_BREA:
    case AT_MAGC:
        w_mask = ~0L; /* special case; no defense needed */
        break;
    case AT_CLAW:
    case AT_TUCH:
    case AT_WEAP:
        w_mask = W_ARMG; /* caller needs to check for weapon */
        break;
    case AT_KICK:
        w_mask = W_ARMF;
        break;
    case AT_BUTT:
        w_mask = W_ARMH;
        break;
    case AT_HUGS:
        w_mask = (W_ARMC | W_ARMG); /* attacker needs both to be protected */
        break;
    case AT_BITE:
    case AT_STNG:
    case AT_ENGL:
    case AT_TENT:
    default:
        w_mask = 0L; /* no defense available */
        break;
    }
    return w_mask;
}

/*mhitm.c*/
