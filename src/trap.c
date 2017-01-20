/* NetHack 3.6	trap.c	$NHDT-Date: 1448492213 2015/11/25 22:56:53 $  $NHDT-Branch: master $:$NHDT-Revision: 1.249 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2016            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern const char *const destroy_strings[][3]; /* from zap.c */

STATIC_DCL void FDECL(dofiretrap, (struct obj *));
STATIC_DCL void NDECL(domagictrap);
STATIC_DCL boolean FDECL(emergency_disrobe, (boolean *));
STATIC_DCL int FDECL(untrap_prob, (struct trap *));
STATIC_DCL void FDECL(move_into_trap, (struct trap *));
STATIC_DCL int FDECL(try_disarm, (struct trap *, BOOLEAN_P));
STATIC_DCL void FDECL(reward_untrap, (struct trap *, struct monst *));
STATIC_DCL int FDECL(disarm_holdingtrap, (struct trap *));
STATIC_DCL int FDECL(disarm_landmine, (struct trap *));
STATIC_DCL int FDECL(disarm_squeaky_board, (struct trap *));
STATIC_DCL int FDECL(disarm_shooting_trap, (struct trap *, int));
STATIC_DCL int FDECL(try_lift, (struct monst *, struct trap *, int,
                                BOOLEAN_P));
STATIC_DCL int FDECL(help_monster_out, (struct monst *, struct trap *));
STATIC_DCL boolean FDECL(thitm, (int, struct monst *, struct obj *, int,
                                 BOOLEAN_P));
STATIC_DCL void FDECL(launch_drop_spot, (struct obj *, XCHAR_P, XCHAR_P));
STATIC_DCL int FDECL(mkroll_launch, (struct trap *, XCHAR_P, XCHAR_P,
                                     SHORT_P, long));
STATIC_DCL boolean FDECL(isclearpath, (coord *, int, SCHAR_P, SCHAR_P));
STATIC_DCL char *FDECL(trapnote, (struct trap *, BOOLEAN_P));
#if 0
STATIC_DCL void FDECL(join_adjacent_pits, (struct trap *));
#endif
STATIC_DCL void FDECL(clear_conjoined_pits, (struct trap *));
STATIC_DCL int FDECL(steedintrap, (struct trap *, struct obj *));
STATIC_DCL boolean FDECL(keep_saddle_with_steedcorpse, (unsigned,
                                                        struct obj *,
                                                        struct obj *));
STATIC_DCL void NDECL(maybe_finish_sokoban);

/* mintrap() should take a flags argument, but for time being we use this */
STATIC_VAR int force_mintrap = 0;

#if 0 /*JP*/
STATIC_VAR const char *const a_your[2] = { "a", "your" };
STATIC_VAR const char *const A_Your[2] = { "A", "Your" };
STATIC_VAR const char tower_of_flame[] = "tower of flame";
STATIC_VAR const char *const A_gush_of_water_hits = "A gush of water hits";
#endif
#if 0 /*JP*/
STATIC_VAR const char *const blindgas[6] = { "humid",   "odorless",
                                             "pungent", "chilling",
                                             "acrid",   "biting" };
#else
STATIC_VAR const char * const blindgas[6] = {
    "むしむしする", "無臭の",
    "刺激臭のする", "冷たい",
    "ツンとしたにおいの", "ひりひりする"
    };
#endif
#if 1 /*JP*/
const char *set_you[2] = { "", "あなたの仕掛けた" };
STATIC_VAR const char *dig_you[2] = { "", "あなたが掘った" };
STATIC_VAR const char *web_you[2] = { "", "あなたが張った" };
#endif

/* called when you're hit by fire (dofiretrap,buzz,zapyourself,explode);
   returns TRUE if hit on torso */
boolean
burnarmor(victim)
struct monst *victim;
{
    struct obj *item;
    char buf[BUFSZ];
    int mat_idx, oldspe;
    boolean hitting_u;

    if (!victim)
        return 0;
    hitting_u = (victim == &youmonst);

    /* burning damage may dry wet towel */
    item = hitting_u ? carrying(TOWEL) : m_carrying(victim, TOWEL);
    while (item) {
        if (is_wet_towel(item)) {
            oldspe = item->spe;
            dry_a_towel(item, rn2(oldspe + 1), TRUE);
            if (item->spe != oldspe)
                break; /* stop once one towel has been affected */
        }
        item = item->nobj;
    }

#define burn_dmg(obj, descr) erode_obj(obj, descr, ERODE_BURN, EF_GREASE)
    while (1) {
        switch (rn2(5)) {
        case 0:
            item = hitting_u ? uarmh : which_armor(victim, W_ARMH);
            if (item) {
                mat_idx = objects[item->otyp].oc_material;
#if 0 /*JP*/
                Sprintf(buf, "%s %s", materialnm[mat_idx],
                        helm_simple_name(item));
#else
                Sprintf(buf, "%sの%s", materialnm[mat_idx],
                        helm_simple_name(item));
#endif
            }
/*JP
            if (!burn_dmg(item, item ? buf : "helmet"))
*/
            if (!burn_dmg(item, item ? buf : "兜"))
                continue;
            break;
        case 1:
            item = hitting_u ? uarmc : which_armor(victim, W_ARMC);
            if (item) {
                (void) burn_dmg(item, cloak_simple_name(item));
                return TRUE;
            }
            item = hitting_u ? uarm : which_armor(victim, W_ARM);
            if (item) {
                (void) burn_dmg(item, xname(item));
                return TRUE;
            }
            item = hitting_u ? uarmu : which_armor(victim, W_ARMU);
            if (item)
/*JP
                (void) burn_dmg(item, "shirt");
*/
                (void) burn_dmg(item, "シャツ");
            return TRUE;
        case 2:
            item = hitting_u ? uarms : which_armor(victim, W_ARMS);
/*JP
            if (!burn_dmg(item, "wooden shield"))
*/
            if (!burn_dmg(item, "木の盾"))
                continue;
            break;
        case 3:
            item = hitting_u ? uarmg : which_armor(victim, W_ARMG);
/*JP
            if (!burn_dmg(item, "gloves"))
*/
            if (!burn_dmg(item, "小手"))
                continue;
            break;
        case 4:
            item = hitting_u ? uarmf : which_armor(victim, W_ARMF);
/*JP
            if (!burn_dmg(item, "boots"))
*/
            if (!burn_dmg(item, "靴"))
                continue;
            break;
        }
        break; /* Out of while loop */
    }
#undef burn_dmg

    return FALSE;
}

/* Generic erode-item function.
 * "ostr", if non-null, is an alternate string to print instead of the
 *   object's name.
 * "type" is an ERODE_* value for the erosion type
 * "flags" is an or-ed list of EF_* flags
 *
 * Returns an erosion return value (ER_*)
 */
int
erode_obj(otmp, ostr, type, ef_flags)
register struct obj *otmp;
const char *ostr;
int type;
int ef_flags;
{
#if 0 /*JP*/
    static NEARDATA const char *const action[] = { "smoulder", "rust", "rot",
                                                   "corrode" };
#else
    static NEARDATA const char * const action[] = {
        "くすぶった", "錆びた", "腐った", "腐食した" };
#endif
#if 0 /*JP*/
    static NEARDATA const char *const msg[] = { "burnt", "rusted", "rotten",
                                                "corroded" };
#else
    static NEARDATA const char * const msg[] =  {
        "焦げた", "錆びた", "腐った", "腐食した" };
#endif
    boolean vulnerable = FALSE;
    boolean is_primary = TRUE;
    boolean check_grease = ef_flags & EF_GREASE;
    boolean print = ef_flags & EF_VERBOSE;
    int erosion;
    struct monst *victim;
    boolean vismon;
    boolean visobj;
    int cost_type;

    if (!otmp)
        return ER_NOTHING;

    victim = carried(otmp) ? &youmonst : mcarried(otmp) ? otmp->ocarry : NULL;
    vismon = victim && (victim != &youmonst) && canseemon(victim);
    /* Is bhitpos correct here? Ugh. */
    visobj = !victim && cansee(bhitpos.x, bhitpos.y);

    switch (type) {
    case ERODE_BURN:
        vulnerable = is_flammable(otmp);
        check_grease = FALSE;
        cost_type = COST_BURN;
        break;
    case ERODE_RUST:
        vulnerable = is_rustprone(otmp);
        cost_type = COST_RUST;
        break;
    case ERODE_ROT:
        vulnerable = is_rottable(otmp);
        check_grease = FALSE;
        is_primary = FALSE;
        cost_type = COST_ROT;
        break;
    case ERODE_CORRODE:
        vulnerable = is_corrodeable(otmp);
        is_primary = FALSE;
        cost_type = COST_CORRODE;
        break;
    default:
        impossible("Invalid erosion type in erode_obj");
        return ER_NOTHING;
    }
    erosion = is_primary ? otmp->oeroded : otmp->oeroded2;

    if (!ostr)
        ostr = cxname(otmp);

    if (check_grease && otmp->greased) {
        grease_protect(otmp, ostr, victim);
        return ER_GREASED;
    } else if (!vulnerable || (otmp->oerodeproof && otmp->rknown)) {
        if (print && flags.verbose) {
            if (victim == &youmonst)
/*JP
                Your("%s %s not affected.", ostr, vtense(ostr, "are"));
*/
                Your("%sは影響を受けなかった．", ostr);
            else if (vismon)
#if 0 /*JP*/
                pline("%s %s %s not affected.", s_suffix(Monnam(victim)),
                      ostr, vtense(ostr, "are"));
#else
                pline("%sの%sは影響を受けなかった．", Monnam(victim),
                      ostr);
#endif
        }
        return ER_NOTHING;
    } else if (otmp->oerodeproof || (otmp->blessed && !rnl(4))) {
        if (flags.verbose && (print || otmp->oerodeproof)) {
            if (victim == &youmonst)
#if 0 /*JP*/
                pline("Somehow, your %s %s not affected.", ostr,
                      vtense(ostr, "are"));
#else
                pline("なぜか，%sは影響を受けなかった．",ostr);
#endif
            else if (vismon)
#if 0 /*JP*/
                pline("Somehow, %s %s %s not affected.",
                      s_suffix(mon_nam(victim)), ostr, vtense(ostr, "are"));
#else
                pline("なぜか，%sの%sは影響を受けなかった．",
                      mon_nam(victim), ostr);
#endif
            else if (visobj)
#if 0 /*JP*/
                pline("Somehow, the %s %s not affected.", ostr,
                      vtense(ostr, "are"));
#else
                pline("なぜか，%sは影響を受けなかった．", ostr);
#endif
        }
        /* We assume here that if the object is protected because it
         * is blessed, it still shows some minor signs of wear, and
         * the hero can distinguish this from an object that is
         * actually proof against damage. */
        if (otmp->oerodeproof) {
            otmp->rknown = TRUE;
            if (victim == &youmonst)
                update_inventory();
        }

        return ER_NOTHING;
    } else if (erosion < MAX_ERODE) {
#if 0 /*JP*/
        const char *adverb = (erosion + 1 == MAX_ERODE)
                                 ? " completely"
                                 : erosion ? " further" : "";
#else
        const char *adverb = (erosion + 1 == MAX_ERODE)
                                 ? "完全に"
                                 : erosion ? "さらに" : "";
#endif

        if (victim == &youmonst)
/*JP
            Your("%s %s%s!", ostr, vtense(ostr, action[type]), adverb);
*/
            pline("%sは%s%s！", ostr, adverb, action[type]);
        else if (vismon)
#if 0 /*JP*/
            pline("%s %s %s%s!", s_suffix(Monnam(victim)), ostr,
                  vtense(ostr, action[type]), adverb);
#else
            pline("%sの%sは%s%s！", Monnam(victim), ostr,
                  adverb, action[type]);
#endif
        else if (visobj)
/*JP
            pline("The %s %s%s!", ostr, vtense(ostr, action[type]), adverb);
*/
            pline("%sは%s%s！", ostr, adverb, action[type]);

        if (ef_flags & EF_PAY)
            costly_alteration(otmp, cost_type);

        if (is_primary)
            otmp->oeroded++;
        else
            otmp->oeroded2++;

        if (victim == &youmonst)
            update_inventory();

        return ER_DAMAGED;
    } else if (ef_flags & EF_DESTROY) {
        if (victim == &youmonst)
/*JP
            Your("%s %s away!", ostr, vtense(ostr, action[type]));
*/
            Your("%sは完全に%s！", ostr, action[type]);
        else if (vismon)
#if 0 /*JP*/
            pline("%s %s %s away!", s_suffix(Monnam(victim)), ostr,
                  vtense(ostr, action[type]));
#else
            pline("%sの%sは完全に%s！", Monnam(victim), ostr,
                  action[type]);
#endif
        else if (visobj)
/*JP
            pline("The %s %s away!", ostr, vtense(ostr, action[type]));
*/
            pline("%sは完全に%s！", ostr, action[type]);

        if (ef_flags & EF_PAY)
            costly_alteration(otmp, cost_type);

        setnotworn(otmp);
        delobj(otmp);
        return ER_DESTROYED;
    } else {
        if (flags.verbose && print) {
            if (victim == &youmonst)
#if 0 /*JP*/
                Your("%s %s completely %s.", ostr,
                     vtense(ostr, Blind ? "feel" : "look"), msg[type]);
#else
                Your("%sは完全に%s%s．", ostr,
                     msg[type], Blind ? "ようだ" : "");
#endif
            else if (vismon)
#if 0 /*JP*/
                pline("%s %s %s completely %s.", s_suffix(Monnam(victim)),
                      ostr, vtense(ostr, "look"), msg[type]);
#else
                pline("%sの%sは完全に%s．", Monnam(victim),
                      ostr, msg[type]);
#endif
            else if (visobj)
#if 0 /*JP*/
                pline("The %s %s completely %s.", ostr, vtense(ostr, "look"),
                      msg[type]);
#else
                pline("%sは完全に%s．", ostr, msg[type]);
#endif
        }
        return ER_NOTHING;
    }
}

/* Protect an item from erosion with grease. Returns TRUE if the grease
 * wears off.
 */
boolean
grease_protect(otmp, ostr, victim)
register struct obj *otmp;
const char *ostr;
struct monst *victim;
{
/*JP
    static const char txt[] = "protected by the layer of grease!";
*/
    static const char txt[] = "油の塗りこみによって守られている！";
    boolean vismon = victim && (victim != &youmonst) && canseemon(victim);

    if (ostr) {
        if (victim == &youmonst)
/*JP
            Your("%s %s %s", ostr, vtense(ostr, "are"), txt);
*/
            Your("%sは%s", ostr, txt);
        else if (vismon)
#if 0 /*JP*/
            pline("%s's %s %s %s", Monnam(victim),
                  ostr, vtense(ostr, "are"), txt);
#else
            pline("%sの%sは%s", Monnam(victim), ostr, txt);
#endif
    } else if (victim == &youmonst || vismon) {
/*JP
        pline("%s %s", Yobjnam2(otmp, "are"), txt);
*/
        Your("%sは%s",xname(otmp), txt);
    }
    if (!rn2(2)) {
        otmp->greased = 0;
        if (carried(otmp)) {
/*JP
            pline_The("grease dissolves.");
*/
            pline("油ははげてしまった．");
            update_inventory();
        }
        return TRUE;
    }
    return FALSE;
}

struct trap *
maketrap(x, y, typ)
register int x, y, typ;
{
    static union vlaunchinfo zero_vl;
    register struct trap *ttmp;
    register struct rm *lev;
    boolean oldplace;

    if ((ttmp = t_at(x, y)) != 0) {
        if (ttmp->ttyp == MAGIC_PORTAL || ttmp->ttyp == VIBRATING_SQUARE)
            return (struct trap *) 0;
        oldplace = TRUE;
        if (u.utrap && x == u.ux && y == u.uy
            && ((u.utraptype == TT_BEARTRAP && typ != BEAR_TRAP)
                || (u.utraptype == TT_WEB && typ != WEB)
                || (u.utraptype == TT_PIT && typ != PIT
                    && typ != SPIKED_PIT)))
            u.utrap = 0;
        /* old <tx,ty> remain valid */
    } else if (IS_FURNITURE(levl[x][y].typ)) {
        /* no trap on top of furniture (caller usually screens the
           location to inhibit this, but wizard mode wishing doesn't) */
        return (struct trap *) 0;
    } else {
        oldplace = FALSE;
        ttmp = newtrap();
        ttmp->ntrap = 0;
        ttmp->tx = x;
        ttmp->ty = y;
    }
    /* [re-]initialize all fields except ntrap (handled below) and <tx,ty> */
    ttmp->vl = zero_vl;
    ttmp->launch.x = ttmp->launch.y = -1; /* force error if used before set */
    ttmp->dst.dnum = ttmp->dst.dlevel = -1;
    ttmp->madeby_u = 0;
    ttmp->once = 0;
    ttmp->tseen = (typ == HOLE); /* hide non-holes */
    ttmp->ttyp = typ;

    switch (typ) {
    case SQKY_BOARD: {
        int tavail[12], tpick[12], tcnt = 0, k;
        struct trap *t;

        for (k = 0; k < 12; ++k)
            tavail[k] = tpick[k] = 0;
        for (t = ftrap; t; t = t->ntrap)
            if (t->ttyp == SQKY_BOARD && t != ttmp)
                tavail[t->tnote] = 1;
        /* now populate tpick[] with the available indices */
        for (k = 0; k < 12; ++k)
            if (tavail[k] == 0)
                tpick[tcnt++] = k;
        /* choose an unused note; if all are in use, pick a random one */
        ttmp->tnote = (short) ((tcnt > 0) ? tpick[rn2(tcnt)] : rn2(12));
        break;
    }
    case STATUE_TRAP: { /* create a "living" statue */
        struct monst *mtmp;
        struct obj *otmp, *statue;
        struct permonst *mptr;
        int trycount = 10;

        do { /* avoid ultimately hostile co-aligned unicorn */
            mptr = &mons[rndmonnum()];
        } while (--trycount > 0 && is_unicorn(mptr)
                 && sgn(u.ualign.type) == sgn(mptr->maligntyp));
        statue = mkcorpstat(STATUE, (struct monst *) 0, mptr, x, y,
                            CORPSTAT_NONE);
        mtmp = makemon(&mons[statue->corpsenm], 0, 0, MM_NOCOUNTBIRTH);
        if (!mtmp)
            break; /* should never happen */
        while (mtmp->minvent) {
            otmp = mtmp->minvent;
            otmp->owornmask = 0;
            obj_extract_self(otmp);
            (void) add_to_container(statue, otmp);
        }
        statue->owt = weight(statue);
        mongone(mtmp);
        break;
    }
    case ROLLING_BOULDER_TRAP: /* boulder will roll towards trigger */
        (void) mkroll_launch(ttmp, x, y, BOULDER, 1L);
        break;
    case PIT:
    case SPIKED_PIT:
        ttmp->conjoined = 0;
        /*FALLTHRU*/
    case HOLE:
    case TRAPDOOR:
        lev = &levl[x][y];
        if (*in_rooms(x, y, SHOPBASE)
            && (typ == HOLE || typ == TRAPDOOR
                || IS_DOOR(lev->typ) || IS_WALL(lev->typ)))
            add_damage(x, y, /* schedule repair */
                       ((IS_DOOR(lev->typ) || IS_WALL(lev->typ))
                        && !context.mon_moving)
                           ? 200L
                           : 0L);
        lev->doormask = 0;     /* subsumes altarmask, icedpool... */
        if (IS_ROOM(lev->typ)) /* && !IS_AIR(lev->typ) */
            lev->typ = ROOM;
        /*
         * some cases which can happen when digging
         * down while phazing thru solid areas
         */
        else if (lev->typ == STONE || lev->typ == SCORR)
            lev->typ = CORR;
        else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
            lev->typ = level.flags.is_maze_lev
                           ? ROOM
                           : level.flags.is_cavernous_lev ? CORR : DOOR;

        unearth_objs(x, y);
        break;
    }

    if (!oldplace) {
        ttmp->ntrap = ftrap;
        ftrap = ttmp;
    } else {
        /* oldplace;
           it shouldn't be possible to override a sokoban pit or hole
           with some other trap, but we'll check just to be safe */
        if (Sokoban)
            maybe_finish_sokoban();
    }
    return ttmp;
}

void
fall_through(td)
boolean td; /* td == TRUE : trap door or hole */
{
    d_level dtmp;
    char msgbuf[BUFSZ];
    const char *dont_fall = 0;
    int newlevel, bottom;

    /* we'll fall even while levitating in Sokoban; otherwise, if we
       won't fall and won't be told that we aren't falling, give up now */
    if (Blind && Levitation && !Sokoban)
        return;

    bottom = dunlevs_in_dungeon(&u.uz);
    /* when in the upper half of the quest, don't fall past the
       middle "quest locate" level if hero hasn't been there yet */
    if (In_quest(&u.uz)) {
        int qlocate_depth = qlocate_level.dlevel;

        /* deepest reached < qlocate implies current < qlocate */
        if (dunlev_reached(&u.uz) < qlocate_depth)
            bottom = qlocate_depth; /* early cut-off */
    }
    newlevel = dunlev(&u.uz); /* current level */
    do {
        newlevel++;
    } while (!rn2(4) && newlevel < bottom);

    if (td) {
        struct trap *t = t_at(u.ux, u.uy);

        feeltrap(t);
        if (!Sokoban) {
            if (t->ttyp == TRAPDOOR)
/*JP
                pline("A trap door opens up under you!");
*/
                pline("落し扉があなたの足元に開いた！");
            else
/*JP
                pline("There's a gaping hole under you!");
*/
                pline("あなたの足下にぽっかりと穴が開いている！");
        }
    } else
/*JP
        pline_The("%s opens up under you!", surface(u.ux, u.uy));
*/
        pline("足元の%sに穴が開いた！", surface(u.ux,u.uy));

    if (Sokoban && Can_fall_thru(&u.uz))
        ; /* KMH -- You can't escape the Sokoban level traps */
    else if (Levitation || u.ustuck
             || (!Can_fall_thru(&u.uz) && !levl[u.ux][u.uy].candig) || Flying
             || is_clinger(youmonst.data)
             || (Inhell && !u.uevent.invoked && newlevel == bottom)) {
/*JP
        dont_fall = "don't fall in.";
*/
        dont_fall = "しかしあなたは落ちなかった．";
    } else if (youmonst.data->msize >= MZ_HUGE) {
/*JP
        dont_fall = "don't fit through.";
*/
        dont_fall = "通り抜けるにはサイズが合わない．";
    } else if (!next_to_u()) {
/*JP
        dont_fall = "are jerked back by your pet!";
*/
        dont_fall = "あなたはペットによって引っぱりもどされた！";
    }
    if (dont_fall) {
#if 0 /*JP*//*「あなたは」で始まらないものもある*/
        You1(dont_fall);
#else
        pline1(dont_fall);
#endif
        /* hero didn't fall through, but any objects here might */
        impact_drop((struct obj *) 0, u.ux, u.uy, 0);
        if (!td) {
            display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
            pline_The("opening under you closes up.");
*/
            pline_The("足下に開いていたものは閉じた．");
        }
        return;
    }

    if (*u.ushops)
        shopdig(1);
    if (Is_stronghold(&u.uz)) {
        find_hell(&dtmp);
    } else {
        int dist = newlevel - dunlev(&u.uz);
        dtmp.dnum = u.uz.dnum;
        dtmp.dlevel = newlevel;
        if (dist > 1)
#if 0 /*JP*/
            You("fall down a %s%sshaft!", dist > 3 ? "very " : "",
                dist > 2 ? "deep " : "");
#else
            You("%s%s穴の中を落ちていった！", dist > 3 ? "とても" : "",
                dist > 2 ? "深い" : "");
#endif
    }
    if (!td)
/*JP
        Sprintf(msgbuf, "The hole in the %s above you closes up.",
*/
        Sprintf(msgbuf, "%sに開いた穴は閉じた．",
                ceiling(u.ux, u.uy));

    schedule_goto(&dtmp, FALSE, TRUE, 0, (char *) 0,
                  !td ? msgbuf : (char *) 0);
}

/*
 * Animate the given statue.  May have been via shatter attempt, trap,
 * or stone to flesh spell.  Return a monster if successfully animated.
 * If the monster is animated, the object is deleted.  If fail_reason
 * is non-null, then fill in the reason for failure (or success).
 *
 * The cause of animation is:
 *
 *      ANIMATE_NORMAL  - hero "finds" the monster
 *      ANIMATE_SHATTER - hero tries to destroy the statue
 *      ANIMATE_SPELL   - stone to flesh spell hits the statue
 *
 * Perhaps x, y is not needed if we can use get_obj_location() to find
 * the statue's location... ???
 *
 * Sequencing matters:
 *      create monster; if it fails, give up with statue intact;
 *      give "statue comes to life" message;
 *      if statue belongs to shop, have shk give "you owe" message;
 *      transfer statue contents to monster (after stolen_value());
 *      delete statue.
 *      [This ordering means that if the statue ends up wearing a cloak of
 *       invisibility or a mummy wrapping, the visibility checks might be
 *       wrong, but to avoid that we'd have to clone the statue contents
 *       first in order to give them to the monster before checking their
 *       shop status--it's not worth the hassle.]
 */
struct monst *
animate_statue(statue, x, y, cause, fail_reason)
struct obj *statue;
xchar x, y;
int cause;
int *fail_reason;
{
    int mnum = statue->corpsenm;
    struct permonst *mptr = &mons[mnum];
    struct monst *mon = 0, *shkp;
    struct obj *item;
    coord cc;
    boolean historic = (Role_if(PM_ARCHEOLOGIST)
                        && (statue->spe & STATUE_HISTORIC) != 0),
            golem_xform = FALSE, use_saved_traits;
    const char *comes_to_life;
    char statuename[BUFSZ], tmpbuf[BUFSZ];
    static const char historic_statue_is_gone[] =
/*JP
        "that the historic statue is now gone";
*/
        "歴史的な彫像がなくなってしまったこと";
    if (cant_revive(&mnum, TRUE, statue)) {
        /* mnum has changed; we won't be animating this statue as itself */
        if (mnum != PM_DOPPELGANGER)
            mptr = &mons[mnum];
        use_saved_traits = FALSE;
    } else if (is_golem(mptr) && cause == ANIMATE_SPELL) {
        /* statue of any golem hit by stone-to-flesh becomes flesh golem */
        golem_xform = (mptr != &mons[PM_FLESH_GOLEM]);
        mnum = PM_FLESH_GOLEM;
        mptr = &mons[PM_FLESH_GOLEM];
        use_saved_traits = (has_omonst(statue) && !golem_xform);
    } else {
        use_saved_traits = has_omonst(statue);
    }

    if (use_saved_traits) {
        /* restore a petrified monster */
        cc.x = x, cc.y = y;
        mon = montraits(statue, &cc);
        if (mon && mon->mtame && !mon->isminion)
            wary_dog(mon, TRUE);
    } else {
        /* statues of unique monsters from bones or wishing end
           up here (cant_revive() sets mnum to be doppelganger;
           mptr reflects the original form for use by newcham()) */
        if ((mnum == PM_DOPPELGANGER && mptr != &mons[PM_DOPPELGANGER])
            /* block quest guards from other roles */
            || (mptr->msound == MS_GUARDIAN
                && quest_info(MS_GUARDIAN) != mnum)) {
            mon = makemon(&mons[PM_DOPPELGANGER], x, y,
                          NO_MINVENT | MM_NOCOUNTBIRTH | MM_ADJACENTOK);
            /* if hero has protection from shape changers, cham field will
               be NON_PM; otherwise, set form to match the statue */
            if (mon && mon->cham >= LOW_PM)
                (void) newcham(mon, mptr, FALSE, FALSE);
        } else
            mon = makemon(mptr, x, y, (cause == ANIMATE_SPELL)
                                          ? (NO_MINVENT | MM_ADJACENTOK)
                                          : NO_MINVENT);
    }

    if (!mon) {
        if (fail_reason)
            *fail_reason = unique_corpstat(&mons[statue->corpsenm])
                               ? AS_MON_IS_UNIQUE
                               : AS_NO_MON;
        return (struct monst *) 0;
    }

    /* a non-montraits() statue might specify gender */
    if (statue->spe & STATUE_MALE)
        mon->female = FALSE;
    else if (statue->spe & STATUE_FEMALE)
        mon->female = TRUE;
    /* if statue has been named, give same name to the monster */
    if (has_oname(statue))
        mon = christen_monst(mon, ONAME(statue));
    /* mimic statue becomes seen mimic; other hiders won't be hidden */
    if (mon->m_ap_type)
        seemimic(mon);
    else
        mon->mundetected = FALSE;
    mon->msleeping = 0;
    if (cause == ANIMATE_NORMAL || cause == ANIMATE_SHATTER) {
        /* trap always releases hostile monster */
        mon->mtame = 0; /* (might be petrified pet tossed onto trap) */
        mon->mpeaceful = 0;
        set_malign(mon);
    }

#if 0 /*JP*/
    comes_to_life = !canspotmon(mon)
                        ? "disappears"
                        : golem_xform
                              ? "turns into flesh"
                              : (nonliving(mon->data) || is_vampshifter(mon))
                                    ? "moves"
                                    : "comes to life";
#else
    comes_to_life = !canspotmon(mon)
                        ? "消え"
                        : golem_xform
                              ? "肉体に戻っ"
                              : (nonliving(mon->data) || is_vampshifter(mon))
                                    ? "動い"
                                    : "生命を帯び";
#endif
    if ((x == u.ux && y == u.uy) || cause == ANIMATE_SPELL) {
        /* "the|your|Manlobbi's statue [of a wombat]" */
        shkp = shop_keeper(*in_rooms(mon->mx, mon->my, SHOPBASE));
#if 0 /*JP*/
        Sprintf(statuename, "%s%s", shk_your(tmpbuf, statue),
                (cause == ANIMATE_SPELL
                 /* avoid "of a shopkeeper" if it's Manlobbi himself
                    (if carried, it can't be unpaid--hence won't be
                    described as "Manlobbi's statue"--because there
                    wasn't any living shk when statue was picked up) */
                 && (mon != shkp || carried(statue)))
                   ? xname(statue)
                   : "statue");
#else
        Sprintf(statuename, "%s%s", shk_your(tmpbuf, statue),
                (cause == ANIMATE_SPELL
                 && (mon != shkp || carried(statue)))
                   ? xname(statue)
                   : "彫像");
#endif
/*JP
        pline("%s %s!", upstart(statuename), comes_to_life);
*/
        pline("%sは%sた！", upstart(statuename), comes_to_life);
    } else if (Hallucination) { /* They don't know it's a statue */
/*JP
        pline_The("%s suddenly seems more animated.", rndmonnam((char *) 0));
*/
        pline_The("%sは突然より活動的になった．", rndmonnam((char *) 0));
    } else if (cause == ANIMATE_SHATTER) {
        if (cansee(x, y))
            Sprintf(statuename, "%s%s", shk_your(tmpbuf, statue),
                    xname(statue));
        else
/*JP
            Strcpy(statuename, "a statue");
*/
            Strcpy(statuename, "彫像");
#if 0 /*JP*/
        pline("Instead of shattering, %s suddenly %s!", statuename,
              comes_to_life);
#else
        pline("%sは砕けずに%sた！", statuename,
              comes_to_life);
#endif
    } else { /* cause == ANIMATE_NORMAL */
/*JP
        You("find %s posing as a statue.",
*/
        You("%sが彫像のふりをしているのを見つけた．",
            canspotmon(mon) ? a_monnam(mon) : something);
        if (!canspotmon(mon) && Blind)
            map_invisible(x, y);
        stop_occupation();
    }

    /* if this isn't caused by a monster using a wand of striking,
       there might be consequences for the hero */
    if (!context.mon_moving) {
        /* if statue is owned by a shop, hero will have to pay for it;
           stolen_value gives a message (about debt or use of credit)
           which refers to "it" so needs to follow a message describing
           the object ("the statue comes to life" one above) */
        if (cause != ANIMATE_NORMAL && costly_spot(x, y)
            && (shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) != 0
            /* avoid charging for Manlobbi's statue of Manlobbi
               if stone-to-flesh is used on petrified shopkeep */
            && mon != shkp)
            (void) stolen_value(statue, x, y, (boolean) shkp->mpeaceful,
                                FALSE);

        if (historic) {
/*JP
            You_feel("guilty %s.", historic_statue_is_gone);
*/
            You_feel("%sに罪を感じた．", historic_statue_is_gone);
            adjalign(-1);
        }
    } else {
        if (historic && cansee(x, y))
/*JP
            You_feel("regret %s.", historic_statue_is_gone);
*/
            You_feel("%sを後悔した．", historic_statue_is_gone);
        /* no alignment penalty */
    }

    /* transfer any statue contents to monster's inventory */
    while ((item = statue->cobj) != 0) {
        obj_extract_self(item);
        (void) mpickobj(mon, item);
    }
    m_dowear(mon, TRUE);
    /* in case statue is wielded and hero zaps stone-to-flesh at self */
    if (statue->owornmask)
        remove_worn_item(statue, TRUE);
    /* statue no longer exists */
    delobj(statue);

    /* avoid hiding under nothing */
    if (x == u.ux && y == u.uy && Upolyd && hides_under(youmonst.data)
        && !OBJ_AT(x, y))
        u.uundetected = 0;

    if (fail_reason)
        *fail_reason = AS_OK;
    return mon;
}

/*
 * You've either stepped onto a statue trap's location or you've triggered a
 * statue trap by searching next to it or by trying to break it with a wand
 * or pick-axe.
 */
struct monst *
activate_statue_trap(trap, x, y, shatter)
struct trap *trap;
xchar x, y;
boolean shatter;
{
    struct monst *mtmp = (struct monst *) 0;
    struct obj *otmp = sobj_at(STATUE, x, y);
    int fail_reason;

    /*
     * Try to animate the first valid statue.  Stop the loop when we
     * actually create something or the failure cause is not because
     * the mon was unique.
     */
    deltrap(trap);
    while (otmp) {
        mtmp = animate_statue(otmp, x, y,
                              shatter ? ANIMATE_SHATTER : ANIMATE_NORMAL,
                              &fail_reason);
        if (mtmp || fail_reason != AS_MON_IS_UNIQUE)
            break;

        otmp = nxtobj(otmp, STATUE, TRUE);
    }

    feel_newsym(x, y);
    return mtmp;
}

STATIC_OVL boolean
keep_saddle_with_steedcorpse(steed_mid, objchn, saddle)
unsigned steed_mid;
struct obj *objchn, *saddle;
{
    if (!saddle)
        return FALSE;
    while (objchn) {
        if (objchn->otyp == CORPSE && has_omonst(objchn)) {
            struct monst *mtmp = OMONST(objchn);

            if (mtmp->m_id == steed_mid) {
                /* move saddle */
                xchar x, y;
                if (get_obj_location(objchn, &x, &y, 0)) {
                    obj_extract_self(saddle);
                    place_object(saddle, x, y);
                    stackobj(saddle);
                }
                return TRUE;
            }
        }
        if (Has_contents(objchn)
            && keep_saddle_with_steedcorpse(steed_mid, objchn->cobj, saddle))
            return TRUE;
        objchn = objchn->nobj;
    }
    return FALSE;
}

void
dotrap(trap, trflags)
register struct trap *trap;
unsigned trflags;
{
    register int ttype = trap->ttyp;
    register struct obj *otmp;
    boolean already_seen = trap->tseen,
            forcetrap = (trflags & FORCETRAP) != 0,
            webmsgok = (trflags & NOWEBMSG) == 0,
            forcebungle = (trflags & FORCEBUNGLE) != 0,
            plunged = (trflags & TOOKPLUNGE) != 0,
            adj_pit = conjoined_pits(trap, t_at(u.ux0, u.uy0), TRUE);
    int oldumort;
    int steed_article = ARTICLE_THE;

    nomul(0);

    /* KMH -- You can't escape the Sokoban level traps */
    if (Sokoban && (ttype == PIT || ttype == SPIKED_PIT
                    || ttype == HOLE || ttype == TRAPDOOR)) {
        /* The "air currents" message is still appropriate -- even when
         * the hero isn't flying or levitating -- because it conveys the
         * reason why the player cannot escape the trap with a dexterity
         * check, clinging to the ceiling, etc.
         */
#if 0 /*JP*/
        pline("Air currents pull you down into %s %s!",
              a_your[trap->madeby_u],
              defsyms[trap_to_defsym(ttype)].explanation);
#else
        pline("空気の流れがあなたを%sに引き戻した！",
              defsyms[trap_to_defsym(ttype)].explanation);
#endif
        /* then proceed to normal trap effect */
    } else if (already_seen && !forcetrap) {
        if ((Levitation || (Flying && !plunged))
            && (ttype == PIT || ttype == SPIKED_PIT || ttype == HOLE
                || ttype == BEAR_TRAP)) {
#if 0 /*JP*/
            You("%s over %s %s.", Levitation ? "float" : "fly",
                a_your[trap->madeby_u],
                defsyms[trap_to_defsym(ttype)].explanation);
#else
            You("%s%sの上%s．",
                set_you[trap->madeby_u],
                defsyms[trap_to_defsym(ttype)].explanation,
                Levitation ? "を見下ろした" : "を飛んでいる");
#endif
            return;
        }
        if (!Fumbling && ttype != MAGIC_PORTAL && ttype != VIBRATING_SQUARE
            && ttype != ANTI_MAGIC && !forcebungle && !plunged && !adj_pit
            && (!rn2(5) || ((ttype == PIT || ttype == SPIKED_PIT)
                            && is_clinger(youmonst.data)))) {
#if 0 /*JP:T*/
            You("escape %s %s.", (ttype == ARROW_TRAP && !trap->madeby_u)
                                     ? "an"
                                     : a_your[trap->madeby_u],
                defsyms[trap_to_defsym(ttype)].explanation);
#else
            You("%s%sをするりと避けた．",
                set_you[trap->madeby_u],
                defsyms[trap_to_defsym(ttype)].explanation);
#endif
            return;
        }
    }

    if (u.usteed) {
        u.usteed->mtrapseen |= (1 << (ttype - 1));
        /* suppress article in various steed messages when using its
           name (which won't occur when hallucinating) */
        if (has_mname(u.usteed) && !Hallucination)
            steed_article = ARTICLE_NONE;
    }

    switch (ttype) {
    case ARROW_TRAP:
        if (trap->once && trap->tseen && !rn2(15)) {
/*JP
            You_hear("a loud click!");
*/
            You_hear("ガチャッという大きな音を聞いた！");
            deltrap(trap);
            newsym(u.ux, u.uy);
            break;
        }
        trap->once = 1;
        seetrap(trap);
/*JP
        pline("An arrow shoots out at you!");
*/
        pline("矢が飛んできた！");
        otmp = mksobj(ARROW, TRUE, FALSE);
        otmp->quan = 1L;
        otmp->owt = weight(otmp);
        otmp->opoisoned = 0;
        if (u.usteed && !rn2(2) && steedintrap(trap, otmp)) { /* nothing */
            ;
/*JP
        } else if (thitu(8, dmgval(otmp, &youmonst), otmp, "arrow")) {
*/
        } else if (thitu(8, dmgval(otmp, &youmonst), otmp, "矢")) {
            obfree(otmp, (struct obj *) 0);
        } else {
            place_object(otmp, u.ux, u.uy);
            if (!Blind)
                otmp->dknown = 1;
            stackobj(otmp);
            newsym(u.ux, u.uy);
        }
        break;

    case DART_TRAP:
        if (trap->once && trap->tseen && !rn2(15)) {
/*JP
            You_hear("a soft click.");
*/
            You_hear("カチッという音を聞いた．");
            deltrap(trap);
            newsym(u.ux, u.uy);
            break;
        }
        trap->once = 1;
        seetrap(trap);
/*JP
        pline("A little dart shoots out at you!");
*/
        pline("小さな投げ矢が飛んできた！");
        otmp = mksobj(DART, TRUE, FALSE);
        otmp->quan = 1L;
        otmp->owt = weight(otmp);
        if (!rn2(6))
            otmp->opoisoned = 1;
        oldumort = u.umortality;
        if (u.usteed && !rn2(2) && steedintrap(trap, otmp)) { /* nothing */
            ;
/*JP
        } else if (thitu(7, dmgval(otmp, &youmonst), otmp, "little dart")) {
*/
        } else if (thitu(7, dmgval(otmp, &youmonst), otmp, "投げ矢")) {
            if (otmp->opoisoned)
#if 0 /*JP*/
                poisoned("dart", A_CON, "little dart",
                         /* if damage triggered life-saving,
                            poison is limited to attrib loss */
                         (u.umortality > oldumort) ? 0 : 10, TRUE);
#else
                poisoned("投げ矢", A_CON, "小さな投げ矢",
                         /* if damage triggered life-saving,
                            poison is limited to attrib loss */
                         (u.umortality > oldumort) ? 0 : 10, TRUE);
#endif
            obfree(otmp, (struct obj *) 0);
        } else {
            place_object(otmp, u.ux, u.uy);
            if (!Blind)
                otmp->dknown = 1;
            stackobj(otmp);
            newsym(u.ux, u.uy);
        }
        break;

    case ROCKTRAP:
        if (trap->once && trap->tseen && !rn2(15)) {
/*JP
            pline("A trap door in %s opens, but nothing falls out!",
*/
            pline("落し扉が%sに開いたが，何も落ちてこなかった！",
                  the(ceiling(u.ux, u.uy)));
            deltrap(trap);
            newsym(u.ux, u.uy);
        } else {
            int dmg = d(2, 6); /* should be std ROCK dmg? */

            trap->once = 1;
            feeltrap(trap);
            otmp = mksobj_at(ROCK, u.ux, u.uy, TRUE, FALSE);
            otmp->quan = 1L;
            otmp->owt = weight(otmp);

#if 0 /*JP*/
            pline("A trap door in %s opens and %s falls on your %s!",
                  the(ceiling(u.ux, u.uy)), an(xname(otmp)), body_part(HEAD));
#else
            pline("落し扉が%sに開き，%sがあなたの%sに落ちてきた！",
                  ceiling(u.ux,u.uy), xname(otmp), body_part(HEAD));
#endif

            if (uarmh) {
                if (is_metallic(uarmh)) {
/*JP
                    pline("Fortunately, you are wearing a hard helmet.");
*/
                    pline("幸運にも，あなたは固い兜を身につけていた．");
                    dmg = 2;
                } else if (flags.verbose) {
/*JP
                    pline("%s does not protect you.", Yname2(uarmh));
*/
                    pline("%sでは防げない．", xname(uarmh));
                }
            }

            if (!Blind)
                otmp->dknown = 1;
            stackobj(otmp);
            newsym(u.ux, u.uy); /* map the rock */

/*JP
            losehp(Maybe_Half_Phys(dmg), "falling rock", KILLED_BY_AN);
*/
            losehp(Maybe_Half_Phys(dmg), "落岩で", KILLED_BY_AN);
            exercise(A_STR, FALSE);
        }
        break;

    case SQKY_BOARD: /* stepped on a squeaky board */
        if ((Levitation || Flying) && !forcetrap) {
            if (!Blind) {
                seetrap(trap);
                if (Hallucination)
/*JP
                    You("notice a crease in the linoleum.");
*/
                    You("床の仕上材のしわに気がついた．");
                else
/*JP
                    You("notice a loose board below you.");
*/
                    You("足元の緩んだ板に気がついた．");
            }
        } else {
            seetrap(trap);
#if 0 /*JP*/
            pline("A board beneath you %s%s%s.",
                  Deaf ? "vibrates" : "squeaks ",
                  Deaf ? "" : trapnote(trap, 0), Deaf ? "" : " loudly");
#else
            if (Deaf) {
                pline("足元の板がゆれた．");
            } else {
                pline("足元の板が大きく%sの音にきしんだ．", trapnote(trap, 0));
            }
#endif
            wake_nearby();
        }
        break;

    case BEAR_TRAP: {
        int dmg = d(2, 4);

        if ((Levitation || Flying) && !forcetrap)
            break;
        feeltrap(trap);
        if (amorphous(youmonst.data) || is_whirly(youmonst.data)
            || unsolid(youmonst.data)) {
#if 0 /*JP*/
            pline("%s bear trap closes harmlessly through you.",
                  A_Your[trap->madeby_u]);
#else
            pline("%s熊の罠は噛みついたが，するっと通り抜けた．",
                  set_you[trap->madeby_u]);
#endif
            break;
        }
        if (!u.usteed && youmonst.data->msize <= MZ_SMALL) {
#if 0 /*JP*/
            pline("%s bear trap closes harmlessly over you.",
                  A_Your[trap->madeby_u]);
#else
            pline("%s熊の罠は噛みついたが，遥か上方だった．",
                  set_you[trap->madeby_u]);
#endif
            break;
        }
        u.utrap = rn1(4, 4);
        u.utraptype = TT_BEARTRAP;
        if (u.usteed) {
#if 0 /*JP*/
            pline("%s bear trap closes on %s %s!", A_Your[trap->madeby_u],
                  s_suffix(mon_nam(u.usteed)), mbodypart(u.usteed, FOOT));
#else
            pline("%s熊の罠は%sの%sに噛みついた！", set_you[trap->madeby_u],
                  mon_nam(u.usteed), mbodypart(u.usteed, FOOT));
#endif
            if (thitm(0, u.usteed, (struct obj *) 0, dmg, FALSE))
                u.utrap = 0; /* steed died, hero not trapped */
        } else {
#if 0 /*JP*/
            pline("%s bear trap closes on your %s!", A_Your[trap->madeby_u],
                  body_part(FOOT));
#else
            pline("%s熊の罠があなたの%sに噛みついた！", set_you[trap->madeby_u],
                  body_part(FOOT));
#endif
            set_wounded_legs(rn2(2) ? RIGHT_SIDE : LEFT_SIDE, rn1(10, 10));
            if (u.umonnum == PM_OWLBEAR || u.umonnum == PM_BUGBEAR)
/*JP
                You("howl in anger!");
*/
                You("怒りの咆哮をあげた！");
/*JP
            losehp(Maybe_Half_Phys(dmg), "bear trap", KILLED_BY_AN);
*/
            losehp(Maybe_Half_Phys(dmg), "熊の罠で", KILLED_BY_AN);
        }
        exercise(A_DEX, FALSE);
        break;
    }

    case SLP_GAS_TRAP:
        seetrap(trap);
        if (Sleep_resistance || breathless(youmonst.data)) {
/*JP
            You("are enveloped in a cloud of gas!");
*/
            You("ガス雲につつまれた！");
        } else {
/*JP
            pline("A cloud of gas puts you to sleep!");
*/
            pline("あなたはガス雲で眠ってしまった！");
            fall_asleep(-rnd(25), TRUE);
        }
        (void) steedintrap(trap, (struct obj *) 0);
        break;

    case RUST_TRAP:
        seetrap(trap);

        /* Unlike monsters, traps cannot aim their rust attacks at
         * you, so instead of looping through and taking either the
         * first rustable one or the body, we take whatever we get,
         * even if it is not rustable.
         */
        switch (rn2(5)) {
        case 0:
/*JP
            pline("%s you on the %s!", A_gush_of_water_hits, body_part(HEAD));
*/
            pline("水が噴出してあなたの%sに命中した！", body_part(HEAD));
            (void) water_damage(uarmh, helm_simple_name(uarmh), TRUE);
            break;
        case 1:
/*JP
            pline("%s your left %s!", A_gush_of_water_hits, body_part(ARM));
*/
            pline("水が噴出してあなたの左%sに命中した！", body_part(ARM));
/*JP
            if (water_damage(uarms, "shield", TRUE) != ER_NOTHING)
*/
            if (water_damage(uarms, "盾", TRUE) != ER_NOTHING)
                break;
            if (u.twoweap || (uwep && bimanual(uwep)))
                (void) water_damage(u.twoweap ? uswapwep : uwep, 0, TRUE);
        glovecheck:
/*JP
            (void) water_damage(uarmg, "gauntlets", TRUE);
*/
            (void) water_damage(uarmg, "小手", TRUE);
            /* Not "metal gauntlets" since it gets called
             * even if it's leather for the message
             */
            break;
        case 2:
/*JP
            pline("%s your right %s!", A_gush_of_water_hits, body_part(ARM));
*/
            pline("水が噴出してあなたの右%sに命中した！", body_part(ARM));
            (void) water_damage(uwep, 0, TRUE);
            goto glovecheck;
        default:
/*JP
            pline("%s you!", A_gush_of_water_hits);
*/
            pline("水が噴出してあなたに命中した！");
            for (otmp = invent; otmp; otmp = otmp->nobj)
                if (otmp->lamplit && otmp != uwep
                    && (otmp != uswapwep || !u.twoweap))
                    (void) snuff_lit(otmp);
            if (uarmc)
                (void) water_damage(uarmc, cloak_simple_name(uarmc), TRUE);
            else if (uarm)
/*JP
                (void) water_damage(uarm, "armor", TRUE);
*/
                (void) water_damage(uarm, "鎧", TRUE);
            else if (uarmu)
/*JP
                (void) water_damage(uarmu, "shirt", TRUE);
*/
                (void) water_damage(uarmu, "シャツ", TRUE);
        }
        update_inventory();

        if (u.umonnum == PM_IRON_GOLEM) {
            int dam = u.mhmax;

/*JP
            pline("%s you!", A_gush_of_water_hits);
*/
            pline("水が噴出してあなたに命中した！");
/*JP
            You("are covered with rust!");
*/
            You("錆に覆われた！");
/*JP
            losehp(Maybe_Half_Phys(dam), "rusting away", KILLED_BY);
*/
            losehp(Maybe_Half_Phys(dam), "完全に錆びて", KILLED_BY);
        } else if (u.umonnum == PM_GREMLIN && rn2(3)) {
/*JP
            pline("%s you!", A_gush_of_water_hits);
*/
            pline("水が噴出してあなたに命中した！");
            (void) split_mon(&youmonst, (struct monst *) 0);
        }

        break;

    case FIRE_TRAP:
        seetrap(trap);
        dofiretrap((struct obj *) 0);
        break;

    case PIT:
    case SPIKED_PIT:
        /* KMH -- You can't escape the Sokoban level traps */
        if (!Sokoban && (Levitation || (Flying && !plunged)))
            break;
        feeltrap(trap);
        if (!Sokoban && is_clinger(youmonst.data) && !plunged) {
            if (trap->tseen) {
#if 0 /*JP*/
                You_see("%s %spit below you.", a_your[trap->madeby_u],
                        ttype == SPIKED_PIT ? "spiked " : "");
#else
                pline("足元に%s%s落し穴を発見した．", dig_you[trap->madeby_u],
                      ttype == SPIKED_PIT ? "トゲだらけの" : "");
#endif
            } else {
#if 0 /*JP*/
                pline("%s pit %sopens up under you!", A_Your[trap->madeby_u],
                      ttype == SPIKED_PIT ? "full of spikes " : "");
#else
                pline("%s%s落し穴が足元に開いた！", dig_you[trap->madeby_u],
                      ttype == SPIKED_PIT ? "トゲだらけの" : "");
#endif
/*JP
                You("don't fall in!");
*/
                pline("しかし，あなたは落ちなかった！");
            }
            break;
        }
        if (!Sokoban) {
            char verbbuf[BUFSZ];

            if (u.usteed) {
                if ((trflags & RECURSIVETRAP) != 0)
/*JP
                    Sprintf(verbbuf, "and %s fall",
*/
                    Sprintf(verbbuf, "と%s",
                            x_monnam(u.usteed, steed_article, (char *) 0,
                                     SUPPRESS_SADDLE, FALSE));
                else
#if 0 /*JP*/
                    Sprintf(verbbuf, "lead %s",
                            x_monnam(u.usteed, steed_article, "poor",
                                     SUPPRESS_SADDLE, FALSE));
#else
                    Sprintf(verbbuf,"と%s",
                            x_monnam(u.usteed, steed_article, "かわいそうな",
                                     SUPPRESS_SADDLE, FALSE));
#endif
            } else if (adj_pit) {
/*JP
                You("move into an adjacent pit.");
*/
                You("隣の落し穴に移動した．");
            } else {
#if 0 /*JP*/
                Strcpy(verbbuf,
                       !plunged ? "fall" : (Flying ? "dive" : "plunge"));
#else
                Strcpy(verbbuf,
                       !plunged ? "落ちた" : (Flying ? "飛び込んだ" : "突入した"));
#endif
/*JP
                You("%s into %s pit!", verbbuf, a_your[trap->madeby_u]);
*/
                You("%s落し穴に%s!", set_you[trap->madeby_u], verbbuf);
            }
        }
        /* wumpus reference */
        if (Role_if(PM_RANGER) && !trap->madeby_u && !trap->once
            && In_quest(&u.uz) && Is_qlocate(&u.uz)) {
/*JP
            pline("Fortunately it has a bottom after all...");
*/
            pline("幸い，結局は底があった．．．");
            trap->once = 1;
        } else if (u.umonnum == PM_PIT_VIPER || u.umonnum == PM_PIT_FIEND) {
/*JP
            pline("How pitiful.  Isn't that the pits?");
*/
            pline("この落し穴はいい仕事をしている．");
        }
        if (ttype == SPIKED_PIT) {
/*JP
            const char *predicament = "on a set of sharp iron spikes";
*/
            const char *predicament = "鋭い鉄のトゲトゲの上に";

            if (u.usteed) {
#if 0 /*JP*/
                pline("%s %s %s!",
                      upstart(x_monnam(u.usteed, steed_article, "poor",
                                       SUPPRESS_SADDLE, FALSE)),
                      adj_pit ? "steps" : "lands", predicament);
#else
                pline("%sは%s%s！",
                      upstart(x_monnam(u.usteed, steed_article, "かわいそうな",
                                       SUPPRESS_SADDLE, FALSE)),
                      predicament, adj_pit ? "落ちた" : "降りた");
#endif
            } else
#if 0 /*JP*/
                You("%s %s!", adj_pit ? "step" : "land", predicament);
#else
                You("%s%s！", predicament, adj_pit ? "落ちた" : "降りた");
#endif
        }
        u.utrap = rn1(6, 2);
        u.utraptype = TT_PIT;
        if (!steedintrap(trap, (struct obj *) 0)) {
            if (ttype == SPIKED_PIT) {
                oldumort = u.umortality;
#if 0 /*JP*/
                losehp(Maybe_Half_Phys(rnd(adj_pit ? 6 : 10)),
                       plunged
                           ? "deliberately plunged into a pit of iron spikes"
                           : adj_pit ? "stepped into a pit of iron spikes"
                                     : "fell into a pit of iron spikes",
                       NO_KILLER_PREFIX);
#else
                losehp(Maybe_Half_Phys(rnd(adj_pit ? 6 : 10)),
                       plunged
                           ? "わざわざトゲだらけの落し穴に突入して"
                           : adj_pit ? "トゲだらけの落し穴に踏み込んで"
                                     : "トゲだらけの落し穴に落ちて",
                       KILLED_BY);
#endif
                if (!rn2(6))
#if 0 /*JP*/
                    poisoned("spikes", A_STR,
                             adj_pit ? "stepping on poison spikes"
                                     : "fall onto poison spikes",
                             /* if damage triggered life-saving,
                                poison is limited to attrib loss */
                             (u.umortality > oldumort) ? 0 : 8, FALSE);
#else
                    poisoned("トゲ", A_STR,
                             adj_pit ? "毒の塗られたトゲを踏んで"
                                     : "毒の塗られたトゲの上に落ちて",
                             /* if damage triggered life-saving,
                                poison is limited to attrib loss */
                             (u.umortality > oldumort) ? 0 : 8, FALSE);
#endif
            } else {
                /* plunging flyers take spike damage but not pit damage */
                if (!adj_pit
                    && !(plunged && (Flying || is_clinger(youmonst.data))))
#if 0 /*JP*/
                    losehp(Maybe_Half_Phys(rnd(6)),
                           plunged ? "deliberately plunged into a pit"
                                   : "fell into a pit",
                           NO_KILLER_PREFIX);
#else
                    losehp(Maybe_Half_Phys(rnd(6)),
                           plunged ? "わざわざ落し穴に突入して"
                                   : "落し穴に落ちて",
                           KILLED_BY);
#endif
            }
            if (Punished && !carried(uball)) {
                unplacebc();
                ballfall();
                placebc();
            }
            if (!adj_pit)
/*JP
                selftouch("Falling, you");
*/
                selftouch("落下中，あなたは");
            vision_full_recalc = 1; /* vision limits change */
            exercise(A_STR, FALSE);
            exercise(A_DEX, FALSE);
        }
        break;

    case HOLE:
    case TRAPDOOR:
        if (!Can_fall_thru(&u.uz)) {
            seetrap(trap); /* normally done in fall_through */
            impossible("dotrap: %ss cannot exist on this level.",
                       defsyms[trap_to_defsym(ttype)].explanation);
            break; /* don't activate it after all */
        }
        fall_through(TRUE);
        break;

    case TELEP_TRAP:
        seetrap(trap);
        tele_trap(trap);
        break;

    case LEVEL_TELEP:
        seetrap(trap);
        level_tele_trap(trap);
        break;

    case WEB: /* Our luckless player has stumbled into a web. */
        feeltrap(trap);
        if (amorphous(youmonst.data) || is_whirly(youmonst.data)
            || unsolid(youmonst.data)) {
            if (acidic(youmonst.data) || u.umonnum == PM_GELATINOUS_CUBE
                || u.umonnum == PM_FIRE_ELEMENTAL) {
                if (webmsgok)
#if 0 /*JP*/
                    You("%s %s spider web!",
                        (u.umonnum == PM_FIRE_ELEMENTAL) ? "burn"
                                                         : "dissolve",
                        a_your[trap->madeby_u]);
#else
                    You("%sくもの巣を%s！",
                        web_you[trap->madeby_u],
                        (u.umonnum == PM_FIRE_ELEMENTAL) ? "焼いた"
                                                         : "こなごなにした");
#endif
                deltrap(trap);
                newsym(u.ux, u.uy);
                break;
            }
            if (webmsgok)
/*JP
                You("flow through %s spider web.", a_your[trap->madeby_u]);
*/
                You("%sくもの巣をするりと通り抜けた．", web_you[trap->madeby_u]);
            break;
        }
        if (webmaker(youmonst.data)) {
            if (webmsgok)
#if 0 /*JP*/
                pline(trap->madeby_u ? "You take a walk on your web."
                                     : "There is a spider web here.");
#else
                pline(trap->madeby_u ? "自分で張ったくもの巣の上を歩いた．"
                                     : "ここにはくもの巣がある．");
#endif
            break;
        }
        if (webmsgok) {
            char verbbuf[BUFSZ];

#if 0 /*JP*/
            if (forcetrap) {
                Strcpy(verbbuf, "are caught by");
            } else if (u.usteed) {
                Sprintf(verbbuf, "lead %s into",
                        x_monnam(u.usteed, steed_article, "poor",
                                 SUPPRESS_SADDLE, FALSE));
            } else {
                Sprintf(verbbuf, "%s into",
                        Levitation ? (const char *) "float"
                                   : locomotion(youmonst.data, "stumble"));
            }
            You("%s %s spider web!", verbbuf, a_your[trap->madeby_u]);
#else
            if (forcetrap) {
                Strcpy(verbbuf, "あなたは");
            } else if (u.usteed) {
                Sprintf(verbbuf, "あなたと%s",
                        x_monnam(u.usteed, steed_article, "かわいそうな",
                                 SUPPRESS_SADDLE, FALSE));
            } else {
                Sprintf(verbbuf, "あなたは%s",
                        Levitation ? (const char *) "浮きながら"
                        : jconj(locomotion(youmonst.data, "つまずく"), "て"));
            }
            pline("%s%sくもの巣にひっかかった！", verbbuf, web_you[trap->madeby_u]);
#endif
        }
        u.utraptype = TT_WEB;

        /* Time stuck in the web depends on your/steed strength. */
        {
            register int str = ACURR(A_STR);

            /* If mounted, the steed gets trapped.  Use mintrap
             * to do all the work.  If mtrapped is set as a result,
             * unset it and set utrap instead.  In the case of a
             * strongmonst and mintrap said it's trapped, use a
             * short but non-zero trap time.  Otherwise, monsters
             * have no specific strength, so use player strength.
             * This gets skipped for webmsgok, which implies that
             * the steed isn't a factor.
             */
            if (u.usteed && webmsgok) {
                /* mtmp location might not be up to date */
                u.usteed->mx = u.ux;
                u.usteed->my = u.uy;

                /* mintrap currently does not return 2(died) for webs */
                if (mintrap(u.usteed)) {
                    u.usteed->mtrapped = 0;
                    if (strongmonst(u.usteed->data))
                        str = 17;
                } else {
                    break;
                }

                webmsgok = FALSE; /* mintrap printed the messages */
            }
            if (str <= 3)
                u.utrap = rn1(6, 6);
            else if (str < 6)
                u.utrap = rn1(6, 4);
            else if (str < 9)
                u.utrap = rn1(4, 4);
            else if (str < 12)
                u.utrap = rn1(4, 2);
            else if (str < 15)
                u.utrap = rn1(2, 2);
            else if (str < 18)
                u.utrap = rnd(2);
            else if (str < 69)
                u.utrap = 1;
            else {
                u.utrap = 0;
                if (webmsgok)
/*JP
                    You("tear through %s web!", a_your[trap->madeby_u]);
*/
                    You("%sくもの巣を切り裂いた！", web_you[trap->madeby_u]);
                deltrap(trap);
                newsym(u.ux, u.uy); /* get rid of trap symbol */
            }
        }
        break;

    case STATUE_TRAP:
        (void) activate_statue_trap(trap, u.ux, u.uy, FALSE);
        break;

    case MAGIC_TRAP: /* A magic trap. */
        seetrap(trap);
        if (!rn2(30)) {
            deltrap(trap);
            newsym(u.ux, u.uy); /* update position */
/*JP
            You("are caught in a magical explosion!");
*/
            You("魔法の爆発を浴びた！");
/*JP
            losehp(rnd(10), "magical explosion", KILLED_BY_AN);
*/
            losehp(rnd(10), "魔法の爆発を浴びて", KILLED_BY_AN);
/*JP
            Your("body absorbs some of the magical energy!");
*/
            Your("体は魔法のエネルギーを少し吸いとった！");
            u.uen = (u.uenmax += 2);
            break;
        } else {
            domagictrap();
        }
        (void) steedintrap(trap, (struct obj *) 0);
        break;

    case ANTI_MAGIC:
        seetrap(trap);
        /* hero without magic resistance loses spell energy,
           hero with magic resistance takes damage instead;
           possibly non-intuitive but useful for play balance */
        if (!Antimagic) {
            drain_en(rnd(u.ulevel) + 1);
        } else {
            int dmgval2 = rnd(4), hp = Upolyd ? u.mh : u.uhp;

            /* Half_XXX_damage has opposite its usual effect (approx)
               but isn't cumulative if hero has more than one */
            if (Half_physical_damage || Half_spell_damage)
                dmgval2 += rnd(4);
            /* give Magicbane wielder dose of own medicine */
            if (uwep && uwep->oartifact == ART_MAGICBANE)
                dmgval2 += rnd(4);
            /* having an artifact--other than own quest one--which
               confers magic resistance simply by being carried
               also increases the effect */
            for (otmp = invent; otmp; otmp = otmp->nobj)
                if (otmp->oartifact && !is_quest_artifact(otmp)
                    && defends_when_carried(AD_MAGM, otmp))
                    break;
            if (otmp)
                dmgval2 += rnd(4);
            if (Passes_walls)
                dmgval2 = (dmgval2 + 3) / 4;

#if 0 /*JP*/
            You_feel((dmgval2 >= hp) ? "unbearably torpid!"
                                     : (dmgval2 >= hp / 4) ? "very lethargic."
                                                           : "sluggish.");
#else
            You_feel((dmgval2 >= hp) ? "耐えられないほど無気力になった！"
                                     : (dmgval2 >= hp / 4) ? "とても気だるくなった．"
                                                           : "ものぐさな気分になった．");
#endif
            /* opposite of magical explosion */
/*JP
            losehp(dmgval2, "anti-magic implosion", KILLED_BY_AN);
*/
            losehp(dmgval2, "反魔法の内部破裂で", KILLED_BY_AN);
        }
        break;

    case POLY_TRAP: {
        char verbbuf[BUFSZ];

        seetrap(trap);
        if (u.usteed)
/*JP
            Sprintf(verbbuf, "lead %s",
*/
            Sprintf(verbbuf, "%sとともに飛び込んだ",
                    x_monnam(u.usteed, steed_article, (char *) 0,
                             SUPPRESS_SADDLE, FALSE));
        else
#if 0 /*JP*/
            Sprintf(verbbuf, "%s", Levitation
                                       ? (const char *) "float"
                                       : locomotion(youmonst.data, "step"));
#else
            Sprintf(verbbuf,"%s",
                    jpast(Levitation ? (const char *)"浮きながら飛びこむ"
                          : locomotion(youmonst.data, "踏み込む")));
#endif
/*JP
        You("%s onto a polymorph trap!", verbbuf);
*/
        You("変化の罠に%s！", verbbuf);
        if (Antimagic || Unchanging) {
            shieldeff(u.ux, u.uy);
/*JP
            You_feel("momentarily different.");
*/
            You("一瞬違った感じがした．");
            /* Trap did nothing; don't remove it --KAA */
        } else {
            (void) steedintrap(trap, (struct obj *) 0);
            deltrap(trap);      /* delete trap before polymorph */
            newsym(u.ux, u.uy); /* get rid of trap symbol */
/*JP
            You_feel("a change coming over you.");
*/
            You("変化が訪れたような気がした．");
            polyself(0);
        }
        break;
    }
    case LANDMINE: {
        unsigned steed_mid = 0;
        struct obj *saddle = 0;

        if ((Levitation || Flying) && !forcetrap) {
            if (!already_seen && rn2(3))
                break;
            feeltrap(trap);
#if 0 /*JP*/
            pline("%s %s in a pile of soil below you.",
                  already_seen ? "There is" : "You discover",
                  trap->madeby_u ? "the trigger of your mine" : "a trigger");
#else
            if(already_seen)
                pline("ここには%s地雷の起爆スイッチがある．",
                      set_you[trap->madeby_u]);
            else
                You("足下の土の山に%s地雷の起爆スイッチをみつけた．",
                    set_you[trap->madeby_u]);
#endif
            if (already_seen && rn2(3))
                break;
#if 0 /*JP*/
            pline("KAABLAMM!!!  %s %s%s off!",
                  forcebungle ? "Your inept attempt sets"
                              : "The air currents set",
                  already_seen ? a_your[trap->madeby_u] : "",
                  already_seen ? " land mine" : "it");
#else
            pline("ちゅどーん！！%s%s%s起爆スイッチが発動した！",
                  forcebungle ? "不器用なせいで"
                              : "空気の流れで",
                  already_seen ? set_you[trap->madeby_u] : "",
                  already_seen ? "地雷の" : "");
#endif
        } else {
            /* prevent landmine from killing steed, throwing you to
             * the ground, and you being affected again by the same
             * mine because it hasn't been deleted yet
             */
            static boolean recursive_mine = FALSE;

            if (recursive_mine)
                break;
            feeltrap(trap);
#if 0 /*JP*/
            pline("KAABLAMM!!!  You triggered %s land mine!",
                  a_your[trap->madeby_u]);
#else
            pline("ちゅどーん！！%s地雷の起爆スイッチを踏んだ！",
                  set_you[trap->madeby_u]);
#endif
            if (u.usteed)
                steed_mid = u.usteed->m_id;
            recursive_mine = TRUE;
            (void) steedintrap(trap, (struct obj *) 0);
            recursive_mine = FALSE;
            saddle = sobj_at(SADDLE, u.ux, u.uy);
            set_wounded_legs(LEFT_SIDE, rn1(35, 41));
            set_wounded_legs(RIGHT_SIDE, rn1(35, 41));
            exercise(A_DEX, FALSE);
        }
        blow_up_landmine(trap);
        if (steed_mid && saddle && !u.usteed)
            (void) keep_saddle_with_steedcorpse(steed_mid, fobj, saddle);
        newsym(u.ux, u.uy); /* update trap symbol */
/*JP
        losehp(Maybe_Half_Phys(rnd(16)), "land mine", KILLED_BY_AN);
*/
        losehp(Maybe_Half_Phys(rnd(16)), "地雷を踏んで", KILLED_BY_AN);
        /* fall recursively into the pit... */
        if ((trap = t_at(u.ux, u.uy)) != 0)
            dotrap(trap, RECURSIVETRAP);
        fill_pit(u.ux, u.uy);
        break;
    }

    case ROLLING_BOULDER_TRAP: {
        int style = ROLL | (trap->tseen ? LAUNCH_KNOWN : 0);

        feeltrap(trap);
/*JP
        pline("Click! You trigger a rolling boulder trap!");
*/
        pline("カチッ！あなたは巨岩の罠のスイッチを踏んだ！");
        if (!launch_obj(BOULDER, trap->launch.x, trap->launch.y,
                        trap->launch2.x, trap->launch2.y, style)) {
            deltrap(trap);
            newsym(u.ux, u.uy); /* get rid of trap symbol */
/*JP
            pline("Fortunately for you, no boulder was released.");
*/
            pline("運のよいことに岩は転がってこなかった．");
        }
        break;
    }

    case MAGIC_PORTAL:
        feeltrap(trap);
        domagicportal(trap);
        break;

    case VIBRATING_SQUARE:
        feeltrap(trap);
        /* messages handled elsewhere; the trap symbol is merely to mark the
         * square for future reference */
        break;

    default:
        feeltrap(trap);
        impossible("You hit a trap of type %u", trap->ttyp);
    }
}

STATIC_OVL char *
trapnote(trap, noprefix)
struct trap *trap;
boolean noprefix;
{
    static char tnbuf[12];
    const char *tn,
#if 0 /*JP*/
        *tnnames[12] = { "C note",  "D flat", "D note",  "E flat",
                         "E note",  "F note", "F sharp", "G note",
                         "G sharp", "A note", "B flat",  "B note" };
#else
        *tnnames[12] = { "ド",  "レ♭", "レ",  "ミ♭",
                         "ミ",  "ファ", "ファ♯", "ソ",
                         "ソ♯", "ラ", "シ♭",  "シ" };
#endif

    tnbuf[0] = '\0';
    tn = tnnames[trap->tnote];
#if 0 /*JP*/
    if (!noprefix)
        Sprintf(tnbuf, "%s ",
                (*tn == 'A' || *tn == 'E' || *tn == 'F') ? "an" : "a");
    Sprintf(eos(tnbuf), "%s", tn);
#else
    Sprintf(tnbuf, "%sの音", tn);
#endif
    return tnbuf;
}

STATIC_OVL int
steedintrap(trap, otmp)
struct trap *trap;
struct obj *otmp;
{
    struct monst *steed = u.usteed;
    int tt;
    boolean trapkilled, steedhit;

    if (!steed || !trap)
        return 0;
    tt = trap->ttyp;
    steed->mx = u.ux;
    steed->my = u.uy;
    trapkilled = steedhit = FALSE;

    switch (tt) {
    case ARROW_TRAP:
        if (!otmp) {
            impossible("steed hit by non-existant arrow?");
            return 0;
        }
        trapkilled = thitm(8, steed, otmp, 0, FALSE);
        steedhit = TRUE;
        break;
    case DART_TRAP:
        if (!otmp) {
            impossible("steed hit by non-existant dart?");
            return 0;
        }
        trapkilled = thitm(7, steed, otmp, 0, FALSE);
        steedhit = TRUE;
        break;
    case SLP_GAS_TRAP:
        if (!resists_sleep(steed) && !breathless(steed->data)
            && !steed->msleeping && steed->mcanmove) {
            if (sleep_monst(steed, rnd(25), -1))
                /* no in_sight check here; you can feel it even if blind */
/*JP
                pline("%s suddenly falls asleep!", Monnam(steed));
*/
                pline("%sはとつぜん眠ってしまった！", Monnam(steed));
        }
        steedhit = TRUE;
        break;
    case LANDMINE:
        trapkilled = thitm(0, steed, (struct obj *) 0, rnd(16), FALSE);
        steedhit = TRUE;
        break;
    case PIT:
    case SPIKED_PIT:
        trapkilled = (steed->mhp <= 0
                      || thitm(0, steed, (struct obj *) 0,
                               rnd((tt == PIT) ? 6 : 10), FALSE));
        steedhit = TRUE;
        break;
    case POLY_TRAP:
        if (!resists_magm(steed) && !resist(steed, WAND_CLASS, 0, NOTELL)) {
            (void) newcham(steed, (struct permonst *) 0, FALSE, FALSE);
            if (!can_saddle(steed) || !can_ride(steed))
                dismount_steed(DISMOUNT_POLY);
            else
/*JP
                You("have to adjust yourself in the saddle on %s.",
*/
                You("%sの鞍の上で座りなおした．",
                    x_monnam(steed, ARTICLE_A, (char *) 0, SUPPRESS_SADDLE,
                             FALSE));
        }
        steedhit = TRUE;
        break;
    default:
        break;
    }

    if (trapkilled) {
        dismount_steed(DISMOUNT_POLY);
        return 2;
    }
    return steedhit ? 1 : 0;
}

/* some actions common to both player and monsters for triggered landmine */
void
blow_up_landmine(trap)
struct trap *trap;
{
    int x = trap->tx, y = trap->ty, dbx, dby;
    struct rm *lev = &levl[x][y];

    (void) scatter(x, y, 4,
                   MAY_DESTROY | MAY_HIT | MAY_FRACTURE | VIS_EFFECTS,
                   (struct obj *) 0);
    del_engr_at(x, y);
    wake_nearto(x, y, 400);
    if (IS_DOOR(lev->typ))
        lev->doormask = D_BROKEN;
    /* destroy drawbridge if present */
    if (lev->typ == DRAWBRIDGE_DOWN || is_drawbridge_wall(x, y) >= 0) {
        dbx = x, dby = y;
        /* if under the portcullis, the bridge is adjacent */
        if (find_drawbridge(&dbx, &dby))
            destroy_drawbridge(dbx, dby);
        trap = t_at(x, y); /* expected to be null after destruction */
    }
    /* convert landmine into pit */
    if (trap) {
        if (Is_waterlevel(&u.uz) || Is_airlevel(&u.uz)) {
            /* no pits here */
            deltrap(trap);
        } else {
            trap->ttyp = PIT;       /* explosion creates a pit */
            trap->madeby_u = FALSE; /* resulting pit isn't yours */
            seetrap(trap);          /* and it isn't concealed */
        }
    }
}

/*
 * The following are used to track launched objects to
 * prevent them from vanishing if you are killed. They
 * will reappear at the launchplace in bones files.
 */
static struct {
    struct obj *obj;
    xchar x, y;
} launchplace;

static void
launch_drop_spot(obj, x, y)
struct obj *obj;
xchar x, y;
{
    if (!obj) {
        launchplace.obj = (struct obj *) 0;
        launchplace.x = 0;
        launchplace.y = 0;
    } else {
        launchplace.obj = obj;
        launchplace.x = x;
        launchplace.y = y;
    }
}

boolean
launch_in_progress()
{
    if (launchplace.obj)
        return TRUE;
    return FALSE;
}

void
force_launch_placement()
{
    if (launchplace.obj) {
        launchplace.obj->otrapped = 0;
        place_object(launchplace.obj, launchplace.x, launchplace.y);
    }
}

/*
 * Move obj from (x1,y1) to (x2,y2)
 *
 * Return 0 if no object was launched.
 *        1 if an object was launched and placed somewhere.
 *        2 if an object was launched, but used up.
 */
int
launch_obj(otyp, x1, y1, x2, y2, style)
short otyp;
register int x1, y1, x2, y2;
int style;
{
    register struct monst *mtmp;
    register struct obj *otmp, *otmp2;
    register int dx, dy;
    struct obj *singleobj;
    boolean used_up = FALSE;
    boolean otherside = FALSE;
    int dist;
    int tmp;
    int delaycnt = 0;

    otmp = sobj_at(otyp, x1, y1);
    /* Try the other side too, for rolling boulder traps */
    if (!otmp && otyp == BOULDER) {
        otherside = TRUE;
        otmp = sobj_at(otyp, x2, y2);
    }
    if (!otmp)
        return 0;
    if (otherside) { /* swap 'em */
        int tx, ty;

        tx = x1;
        ty = y1;
        x1 = x2;
        y1 = y2;
        x2 = tx;
        y2 = ty;
    }

    if (otmp->quan == 1L) {
        obj_extract_self(otmp);
        singleobj = otmp;
        otmp = (struct obj *) 0;
    } else {
        singleobj = splitobj(otmp, 1L);
        obj_extract_self(singleobj);
    }
    newsym(x1, y1);
    /* in case you're using a pick-axe to chop the boulder that's being
       launched (perhaps a monster triggered it), destroy context so that
       next dig attempt never thinks you're resuming previous effort */
    if ((otyp == BOULDER || otyp == STATUE)
        && singleobj->ox == context.digging.pos.x
        && singleobj->oy == context.digging.pos.y)
        (void) memset((genericptr_t) &context.digging, 0,
                      sizeof(struct dig_info));

    dist = distmin(x1, y1, x2, y2);
    bhitpos.x = x1;
    bhitpos.y = y1;
    dx = sgn(x2 - x1);
    dy = sgn(y2 - y1);
    switch (style) {
    case ROLL | LAUNCH_UNSEEN:
        if (otyp == BOULDER) {
#if 0 /*JP*/
            You_hear(Hallucination ? "someone bowling."
                                   : "rumbling in the distance.");
#else
            You_hear(Hallucination ? "誰かがボーリングをしている音を聞いた．"
                                   : "遠くのゴロゴロという音を聞いた．");
#endif
        }
        style &= ~LAUNCH_UNSEEN;
        goto roll;
    case ROLL | LAUNCH_KNOWN:
        /* use otrapped as a flag to ohitmon */
        singleobj->otrapped = 1;
        style &= ~LAUNCH_KNOWN;
    /* fall through */
    roll:
    case ROLL:
        delaycnt = 2;
    /* fall through */
    default:
        if (!delaycnt)
            delaycnt = 1;
        if (!cansee(bhitpos.x, bhitpos.y))
            curs_on_u();
        tmp_at(DISP_FLASH, obj_to_glyph(singleobj));
        tmp_at(bhitpos.x, bhitpos.y);
    }
    /* Mark a spot to place object in bones files to prevent
     * loss of object. Use the starting spot to ensure that
     * a rolling boulder will still launch, which it wouldn't
     * do if left midstream. Unfortunately we can't use the
     * target resting spot, because there are some things/situations
     * that would prevent it from ever getting there (bars), and we
     * can't tell that yet.
     */
    launch_drop_spot(singleobj, bhitpos.x, bhitpos.y);

    /* Set the object in motion */
    while (dist-- > 0 && !used_up) {
        struct trap *t;
        tmp_at(bhitpos.x, bhitpos.y);
        tmp = delaycnt;

        /* dstage@u.washington.edu -- Delay only if hero sees it */
        if (cansee(bhitpos.x, bhitpos.y))
            while (tmp-- > 0)
                delay_output();

        bhitpos.x += dx;
        bhitpos.y += dy;
        t = t_at(bhitpos.x, bhitpos.y);

        if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
            if (otyp == BOULDER && throws_rocks(mtmp->data)) {
                if (rn2(3)) {
/*JP
                    pline("%s snatches the boulder.", Monnam(mtmp));
*/
                    pline("%sは岩をつかみ取った．", Monnam(mtmp));
                    singleobj->otrapped = 0;
                    (void) mpickobj(mtmp, singleobj);
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                    break;
                }
            }
            if (ohitmon(mtmp, singleobj, (style == ROLL) ? -1 : dist,
                        FALSE)) {
                used_up = TRUE;
                launch_drop_spot((struct obj *) 0, 0, 0);
                break;
            }
        } else if (bhitpos.x == u.ux && bhitpos.y == u.uy) {
            if (multi)
                nomul(0);
            if (thitu(9 + singleobj->spe, dmgval(singleobj, &youmonst),
                      singleobj, (char *) 0))
                stop_occupation();
        }
        if (style == ROLL) {
            if (down_gate(bhitpos.x, bhitpos.y) != -1) {
                if (ship_object(singleobj, bhitpos.x, bhitpos.y, FALSE)) {
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                    break;
                }
            }
            if (t && otyp == BOULDER) {
                switch (t->ttyp) {
                case LANDMINE:
                    if (rn2(10) > 2) {
#if 0 /*JP*/
                        pline(
                            "KAABLAMM!!!%s",
                            cansee(bhitpos.x, bhitpos.y)
                                ? " The rolling boulder triggers a land mine."
                                : "");
#else
                        pline(
                            "ちゅどーん！！%s",
                            cansee(bhitpos.x, bhitpos.y)
                                ? "転がってきた岩が地雷の起爆スイッチを踏んだ．"
                                : "");
#endif
                        deltrap(t);
                        del_engr_at(bhitpos.x, bhitpos.y);
                        place_object(singleobj, bhitpos.x, bhitpos.y);
                        singleobj->otrapped = 0;
                        fracture_rock(singleobj);
                        (void) scatter(bhitpos.x, bhitpos.y, 4,
                                       MAY_DESTROY | MAY_HIT | MAY_FRACTURE
                                           | VIS_EFFECTS,
                                       (struct obj *) 0);
                        if (cansee(bhitpos.x, bhitpos.y))
                            newsym(bhitpos.x, bhitpos.y);
                        used_up = TRUE;
                        launch_drop_spot((struct obj *) 0, 0, 0);
                    }
                    break;
                case LEVEL_TELEP:
                case TELEP_TRAP:
                    if (cansee(bhitpos.x, bhitpos.y))
/*JP
                        pline("Suddenly the rolling boulder disappears!");
*/
                        pline("転がってきた岩がとつぜん消えた！");
                    else
/*JP
                        You_hear("a rumbling stop abruptly.");
*/
                        pline("ゴロゴロという音が突然止まった．");
                    singleobj->otrapped = 0;
                    if (t->ttyp == TELEP_TRAP)
                        (void) rloco(singleobj);
                    else {
                        int newlev = random_teleport_level();
                        d_level dest;

                        if (newlev == depth(&u.uz) || In_endgame(&u.uz))
                            continue;
                        add_to_migration(singleobj);
                        get_level(&dest, newlev);
                        singleobj->ox = dest.dnum;
                        singleobj->oy = dest.dlevel;
                        singleobj->owornmask = (long) MIGR_RANDOM;
                    }
                    seetrap(t);
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                    break;
                case PIT:
                case SPIKED_PIT:
                case HOLE:
                case TRAPDOOR:
                    /* the boulder won't be used up if there is a
                       monster in the trap; stop rolling anyway */
                    x2 = bhitpos.x, y2 = bhitpos.y; /* stops here */
/*JP
                    if (flooreffects(singleobj, x2, y2, "fall")) {
*/
                    if (flooreffects(singleobj, x2, y2, "落ちる")) {
                        used_up = TRUE;
                        launch_drop_spot((struct obj *) 0, 0, 0);
                    }
                    dist = -1; /* stop rolling immediately */
                    break;
                }
                if (used_up || dist == -1)
                    break;
            }
/*JP
            if (flooreffects(singleobj, bhitpos.x, bhitpos.y, "fall")) {
*/
            if (flooreffects(singleobj, bhitpos.x, bhitpos.y, "落ちる")) {
                used_up = TRUE;
                launch_drop_spot((struct obj *) 0, 0, 0);
                break;
            }
            if (otyp == BOULDER
                && (otmp2 = sobj_at(BOULDER, bhitpos.x, bhitpos.y)) != 0) {
/*JP
                const char *bmsg = " as one boulder sets another in motion";
*/
                const char *bmsg = "ひとつの岩が他の岩を動かしたかのような，";

                if (!isok(bhitpos.x + dx, bhitpos.y + dy) || !dist
                    || IS_ROCK(levl[bhitpos.x + dx][bhitpos.y + dy].typ))
/*JP
                    bmsg = " as one boulder hits another";
*/
                    bmsg = "ひとつの岩が他の岩に当たったような";

/*JP
                You_hear("a loud crash%s!",
*/
                You_hear("%s大きなゴンという音を聞いた！",
                         cansee(bhitpos.x, bhitpos.y) ? bmsg : "");
                obj_extract_self(otmp2);
                /* pass off the otrapped flag to the next boulder */
                otmp2->otrapped = singleobj->otrapped;
                singleobj->otrapped = 0;
                place_object(singleobj, bhitpos.x, bhitpos.y);
                singleobj = otmp2;
                otmp2 = (struct obj *) 0;
                wake_nearto(bhitpos.x, bhitpos.y, 10 * 10);
            }
        }
        if (otyp == BOULDER && closed_door(bhitpos.x, bhitpos.y)) {
            if (cansee(bhitpos.x, bhitpos.y))
/*JP
                pline_The("boulder crashes through a door.");
*/
                pline("岩は扉を破壊した．");
            levl[bhitpos.x][bhitpos.y].doormask = D_BROKEN;
            if (dist)
                unblock_point(bhitpos.x, bhitpos.y);
        }

        /* if about to hit iron bars, do so now */
        if (dist > 0 && isok(bhitpos.x + dx, bhitpos.y + dy)
            && levl[bhitpos.x + dx][bhitpos.y + dy].typ == IRONBARS) {
            x2 = bhitpos.x, y2 = bhitpos.y; /* object stops here */
            if (hits_bars(&singleobj, x2, y2, !rn2(20), 0)) {
                if (!singleobj) {
                    used_up = TRUE;
                    launch_drop_spot((struct obj *) 0, 0, 0);
                }
                break;
            }
        }
    }
    tmp_at(DISP_END, 0);
    launch_drop_spot((struct obj *) 0, 0, 0);
    if (!used_up) {
        singleobj->otrapped = 0;
        place_object(singleobj, x2, y2);
        newsym(x2, y2);
        return 1;
    } else
        return 2;
}

void
seetrap(trap)
struct trap *trap;
{
    if (!trap->tseen) {
        trap->tseen = 1;
        newsym(trap->tx, trap->ty);
    }
}

/* like seetrap() but overrides vision */
void
feeltrap(trap)
struct trap *trap;
{
    trap->tseen = 1;
    map_trap(trap, 1);
    /* in case it's beneath something, redisplay the something */
    newsym(trap->tx, trap->ty);
}

STATIC_OVL int
mkroll_launch(ttmp, x, y, otyp, ocount)
struct trap *ttmp;
xchar x, y;
short otyp;
long ocount;
{
    struct obj *otmp;
    register int tmp;
    schar dx, dy;
    int distance;
    coord cc;
    coord bcc;
    int trycount = 0;
    boolean success = FALSE;
    int mindist = 4;

    if (ttmp->ttyp == ROLLING_BOULDER_TRAP)
        mindist = 2;
    distance = rn1(5, 4); /* 4..8 away */
    tmp = rn2(8);         /* randomly pick a direction to try first */
    while (distance >= mindist) {
        dx = xdir[tmp];
        dy = ydir[tmp];
        cc.x = x;
        cc.y = y;
        /* Prevent boulder from being placed on water */
        if (ttmp->ttyp == ROLLING_BOULDER_TRAP
            && is_pool_or_lava(x + distance * dx, y + distance * dy))
            success = FALSE;
        else
            success = isclearpath(&cc, distance, dx, dy);
        if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
            boolean success_otherway;

            bcc.x = x;
            bcc.y = y;
            success_otherway = isclearpath(&bcc, distance, -(dx), -(dy));
            if (!success_otherway)
                success = FALSE;
        }
        if (success)
            break;
        if (++tmp > 7)
            tmp = 0;
        if ((++trycount % 8) == 0)
            --distance;
    }
    if (!success) {
        /* create the trap without any ammo, launch pt at trap location */
        cc.x = bcc.x = x;
        cc.y = bcc.y = y;
    } else {
        otmp = mksobj(otyp, TRUE, FALSE);
        otmp->quan = ocount;
        otmp->owt = weight(otmp);
        place_object(otmp, cc.x, cc.y);
        stackobj(otmp);
    }
    ttmp->launch.x = cc.x;
    ttmp->launch.y = cc.y;
    if (ttmp->ttyp == ROLLING_BOULDER_TRAP) {
        ttmp->launch2.x = bcc.x;
        ttmp->launch2.y = bcc.y;
    } else
        ttmp->launch_otyp = otyp;
    newsym(ttmp->launch.x, ttmp->launch.y);
    return 1;
}

STATIC_OVL boolean
isclearpath(cc, distance, dx, dy)
coord *cc;
int distance;
schar dx, dy;
{
    uchar typ;
    xchar x, y;

    x = cc->x;
    y = cc->y;
    while (distance-- > 0) {
        x += dx;
        y += dy;
        typ = levl[x][y].typ;
        if (!isok(x, y) || !ZAP_POS(typ) || closed_door(x, y))
            return FALSE;
    }
    cc->x = x;
    cc->y = y;
    return TRUE;
}

int
mintrap(mtmp)
register struct monst *mtmp;
{
    register struct trap *trap = t_at(mtmp->mx, mtmp->my);
    boolean trapkilled = FALSE;
    struct permonst *mptr = mtmp->data;
    struct obj *otmp;

    if (!trap) {
        mtmp->mtrapped = 0;      /* perhaps teleported? */
    } else if (mtmp->mtrapped) { /* is currently in the trap */
        if (!trap->tseen && cansee(mtmp->mx, mtmp->my) && canseemon(mtmp)
            && (trap->ttyp == SPIKED_PIT || trap->ttyp == BEAR_TRAP
                || trap->ttyp == HOLE || trap->ttyp == PIT
                || trap->ttyp == WEB)) {
            /* If you come upon an obviously trapped monster, then
             * you must be able to see the trap it's in too.
             */
            seetrap(trap);
        }

        if (!rn2(40)) {
            if (sobj_at(BOULDER, mtmp->mx, mtmp->my)
                && (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT)) {
                if (!rn2(2)) {
                    mtmp->mtrapped = 0;
                    if (canseemon(mtmp))
/*JP
                        pline("%s pulls free...", Monnam(mtmp));
*/
                        pline("%sは助け上げられた．", Monnam(mtmp));
                    fill_pit(mtmp->mx, mtmp->my);
                }
            } else {
                mtmp->mtrapped = 0;
            }
        } else if (metallivorous(mptr)) {
            if (trap->ttyp == BEAR_TRAP) {
                if (canseemon(mtmp))
/*JP
                    pline("%s eats a bear trap!", Monnam(mtmp));
*/
                    pline("%sは熊の罠を食べた！", Monnam(mtmp));
                deltrap(trap);
                mtmp->meating = 5;
                mtmp->mtrapped = 0;
            } else if (trap->ttyp == SPIKED_PIT) {
                if (canseemon(mtmp))
/*JP
                    pline("%s munches on some spikes!", Monnam(mtmp));
*/
                    pline("%sはトゲトゲを食べた！", Monnam(mtmp));
                trap->ttyp = PIT;
                mtmp->meating = 5;
            }
        }
    } else {
        register int tt = trap->ttyp;
        boolean in_sight, tear_web, see_it,
            inescapable = force_mintrap || ((tt == HOLE || tt == PIT)
                                            && Sokoban && !trap->madeby_u);
        const char *fallverb;

        /* true when called from dotrap, inescapable is not an option */
        if (mtmp == u.usteed)
            inescapable = TRUE;
        if (!inescapable && ((mtmp->mtrapseen & (1 << (tt - 1))) != 0
                             || (tt == HOLE && !mindless(mptr)))) {
            /* it has been in such a trap - perhaps it escapes */
            if (rn2(4))
                return 0;
        } else {
            mtmp->mtrapseen |= (1 << (tt - 1));
        }
        /* Monster is aggravated by being trapped by you.
           Recognizing who made the trap isn't completely
           unreasonable; everybody has their own style. */
        if (trap->madeby_u && rnl(5))
            setmangry(mtmp);

        in_sight = canseemon(mtmp);
        see_it = cansee(mtmp->mx, mtmp->my);
        /* assume hero can tell what's going on for the steed */
        if (mtmp == u.usteed)
            in_sight = TRUE;
        switch (tt) {
        case ARROW_TRAP:
            if (trap->once && trap->tseen && !rn2(15)) {
                if (in_sight && see_it)
#if 0 /*JP:T*/
                    pline("%s triggers a trap but nothing happens.",
                          Monnam(mtmp));
#else
                    pline("%sがわなを起動させたが何も起こらなかった．",
                          Monnam(mtmp));
#endif
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
                break;
            }
            trap->once = 1;
            otmp = mksobj(ARROW, TRUE, FALSE);
            otmp->quan = 1L;
            otmp->owt = weight(otmp);
            otmp->opoisoned = 0;
            if (in_sight)
                seetrap(trap);
            if (thitm(8, mtmp, otmp, 0, FALSE))
                trapkilled = TRUE;
            break;
        case DART_TRAP:
            if (trap->once && trap->tseen && !rn2(15)) {
                if (in_sight && see_it)
#if 0 /*JP:T*/
                    pline("%s triggers a trap but nothing happens.",
                          Monnam(mtmp));
#else
                    pline("%sがわなを起動させたが何も起こらなかった．",
                          Monnam(mtmp));
#endif
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
                break;
            }
            trap->once = 1;
            otmp = mksobj(DART, TRUE, FALSE);
            otmp->quan = 1L;
            otmp->owt = weight(otmp);
            if (!rn2(6))
                otmp->opoisoned = 1;
            if (in_sight)
                seetrap(trap);
            if (thitm(7, mtmp, otmp, 0, FALSE))
                trapkilled = TRUE;
            break;
        case ROCKTRAP:
            if (trap->once && trap->tseen && !rn2(15)) {
                if (in_sight && see_it)
                    pline(
/*JP
                        "A trap door above %s opens, but nothing falls out!",
*/
                        "%sの上の扉が開いたが，何も落ちてこなかった！",
                        mon_nam(mtmp));
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
                break;
            }
            trap->once = 1;
            otmp = mksobj(ROCK, TRUE, FALSE);
            otmp->quan = 1L;
            otmp->owt = weight(otmp);
            if (in_sight)
                seetrap(trap);
            if (thitm(0, mtmp, otmp, d(2, 6), FALSE))
                trapkilled = TRUE;
            break;
        case SQKY_BOARD:
            if (is_flyer(mptr))
                break;
            /* stepped on a squeaky board */
            if (in_sight) {
                if (!Deaf) {
#if 0 /*JP*/
                    pline("A board beneath %s squeaks %s loudly.",
                          mon_nam(mtmp), trapnote(trap, 0));
#else
                    pline("%sの足元の板が大きく%sにきしんだ．",
                          mon_nam(mtmp), trapnote(trap, 0));
#endif
                    seetrap(trap);
                } else {
/*JP
                    pline("%s stops momentarily and appears to cringe.",
*/
                    pline("%sは一瞬止まって身をすくめたようだ．",
                          Monnam(mtmp));
                }
            } else
/*JP
                You_hear("a distant %s squeak.", trapnote(trap, 1));
*/
                You_hear("遠くで%sできしむ音を聞いた．", trapnote(trap, 1));
            /* wake up nearby monsters */
            wake_nearto(mtmp->mx, mtmp->my, 40);
            break;
        case BEAR_TRAP:
            if (mptr->msize > MZ_SMALL && !amorphous(mptr) && !is_flyer(mptr)
                && !is_whirly(mptr) && !unsolid(mptr)) {
                mtmp->mtrapped = 1;
                if (in_sight) {
#if 0 /*JP*/
                    pline("%s is caught in %s bear trap!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
#else
                    pline("%sは%s熊の罠につかまった！", Monnam(mtmp),
                          set_you[trap->madeby_u]);
#endif
                    seetrap(trap);
                } else {
                    if (mptr == &mons[PM_OWLBEAR]
                        || mptr == &mons[PM_BUGBEAR])
/*JP
                        You_hear("the roaring of an angry bear!");
*/
                        You_hear("怒りの咆哮を聞いた！");
                }
            } else if (force_mintrap) {
                if (in_sight) {
#if 0 /*JP*/
                    pline("%s evades %s bear trap!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
#else
                    pline("%sは%s熊の罠をたくみに避けた！", Monnam(mtmp),
                          set_you[trap->madeby_u]);
#endif
                    seetrap(trap);
                }
            }
            if (mtmp->mtrapped)
                trapkilled = thitm(0, mtmp, (struct obj *) 0, d(2, 4), FALSE);
            break;
        case SLP_GAS_TRAP:
            if (!resists_sleep(mtmp) && !breathless(mptr) && !mtmp->msleeping
                && mtmp->mcanmove) {
                if (sleep_monst(mtmp, rnd(25), -1) && in_sight) {
/*JP
                    pline("%s suddenly falls asleep!", Monnam(mtmp));
*/
                    pline("%sは突然眠りに落ちた！", Monnam(mtmp));
                    seetrap(trap);
                }
            }
            break;
        case RUST_TRAP: {
            struct obj *target;

            if (in_sight)
                seetrap(trap);
            switch (rn2(5)) {
            case 0:
                if (in_sight)
#if 0 /*JP*/
                    pline("%s %s on the %s!", A_gush_of_water_hits,
                          mon_nam(mtmp), mbodypart(mtmp, HEAD));
#else
                    pline("水が噴出して，%sの%sに命中した！", 
                          mon_nam(mtmp), mbodypart(mtmp, HEAD));
#endif
                target = which_armor(mtmp, W_ARMH);
                (void) water_damage(target, helm_simple_name(target), TRUE);
                break;
            case 1:
                if (in_sight)
#if 0 /*JP*/
                    pline("%s %s's left %s!", A_gush_of_water_hits,
                          mon_nam(mtmp), mbodypart(mtmp, ARM));
#else
                    pline("水が噴出して，%sの左%sに命中した！",
                          mon_nam(mtmp), mbodypart(mtmp, ARM));
#endif
                target = which_armor(mtmp, W_ARMS);
/*JP
                if (water_damage(target, "shield", TRUE) != ER_NOTHING)
*/
                if (water_damage(target, "盾", TRUE) != ER_NOTHING)
                    break;
                target = MON_WEP(mtmp);
                if (target && bimanual(target))
                    (void) water_damage(target, 0, TRUE);
            glovecheck:
                target = which_armor(mtmp, W_ARMG);
/*JP
                (void) water_damage(target, "gauntlets", TRUE);
*/
                (void) water_damage(target, "小手", TRUE);
                break;
            case 2:
                if (in_sight)
#if 0 /*JP*/
                    pline("%s %s's right %s!", A_gush_of_water_hits,
                          mon_nam(mtmp), mbodypart(mtmp, ARM));
#else
                    pline("水が噴出して，%sの右%sに命中した！",
                          mon_nam(mtmp), mbodypart(mtmp, ARM));
#endif
                (void) water_damage(MON_WEP(mtmp), 0, TRUE);
                goto glovecheck;
            default:
                if (in_sight)
/*JP
                    pline("%s %s!", A_gush_of_water_hits, mon_nam(mtmp));
*/
                    pline("水が噴出して，%sに命中した！", mon_nam(mtmp));
                for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                    if (otmp->lamplit
                        && (otmp->owornmask & (W_WEP | W_SWAPWEP)) == 0)
                        (void) snuff_lit(otmp);
                if ((target = which_armor(mtmp, W_ARMC)) != 0)
                    (void) water_damage(target, cloak_simple_name(target),
                                        TRUE);
                else if ((target = which_armor(mtmp, W_ARM)) != 0)
/*JP
                    (void) water_damage(target, "armor", TRUE);
*/
                    (void) water_damage(target, "鎧", TRUE);
                else if ((target = which_armor(mtmp, W_ARMU)) != 0)
/*JP
                    (void) water_damage(target, "shirt", TRUE);
*/
                    (void) water_damage(target, "シャツ", TRUE);
            }

            if (mptr == &mons[PM_IRON_GOLEM]) {
                if (in_sight)
/*JP
                    pline("%s falls to pieces!", Monnam(mtmp));
*/
                    pline("%sはくだけちった！", Monnam(mtmp));
                else if (mtmp->mtame)
/*JP
                    pline("May %s rust in peace.", mon_nam(mtmp));
*/
                    pline("%sよ，安らかに錆びん事を．", mon_nam(mtmp));
                mondied(mtmp);
                if (mtmp->mhp <= 0)
                    trapkilled = TRUE;
            } else if (mptr == &mons[PM_GREMLIN] && rn2(3)) {
                (void) split_mon(mtmp, (struct monst *) 0);
            }
            break;
        } /* RUST_TRAP */
        case FIRE_TRAP:
        mfiretrap:
            if (in_sight)
#if 0 /*JP*/
                pline("A %s erupts from the %s under %s!", tower_of_flame,
                      surface(mtmp->mx, mtmp->my), mon_nam(mtmp));
#else
                pline("火柱が%sの足元の%sから立ちのぼった！",
                      mon_nam(mtmp), surface(mtmp->mx,mtmp->my));
#endif
            else if (see_it) /* evidently `mtmp' is invisible */
#if 0 /*JP*/
                You_see("a %s erupt from the %s!", tower_of_flame,
                        surface(mtmp->mx, mtmp->my));
#else
                You("火柱が%sから生じるのを見た！",
                    surface(mtmp->mx,mtmp->my));
#endif

            if (resists_fire(mtmp)) {
                if (in_sight) {
                    shieldeff(mtmp->mx, mtmp->my);
/*JP
                    pline("%s is uninjured.", Monnam(mtmp));
*/
                    pline("が，%sは傷つかない．", Monnam(mtmp));
                }
            } else {
                int num = d(2, 4), alt;
                boolean immolate = FALSE;

                /* paper burns very fast, assume straw is tightly
                 * packed and burns a bit slower */
                switch (monsndx(mptr)) {
                case PM_PAPER_GOLEM:
                    immolate = TRUE;
                    alt = mtmp->mhpmax;
                    break;
                case PM_STRAW_GOLEM:
                    alt = mtmp->mhpmax / 2;
                    break;
                case PM_WOOD_GOLEM:
                    alt = mtmp->mhpmax / 4;
                    break;
                case PM_LEATHER_GOLEM:
                    alt = mtmp->mhpmax / 8;
                    break;
                default:
                    alt = 0;
                    break;
                }
                if (alt > num)
                    num = alt;

                if (thitm(0, mtmp, (struct obj *) 0, num, immolate))
                    trapkilled = TRUE;
                else
                    /* we know mhp is at least `num' below mhpmax,
                       so no (mhp > mhpmax) check is needed here */
                    mtmp->mhpmax -= rn2(num + 1);
            }
            if (burnarmor(mtmp) || rn2(3)) {
                (void) destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
                (void) destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
                (void) destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
            }
            if (burn_floor_objects(mtmp->mx, mtmp->my, see_it, FALSE)
                && !see_it && distu(mtmp->mx, mtmp->my) <= 3 * 3)
/*JP
                You("smell smoke.");
*/
                pline("煙の匂いがした．");
            if (is_ice(mtmp->mx, mtmp->my))
                melt_ice(mtmp->mx, mtmp->my, (char *) 0);
            if (see_it)
                seetrap(trap);
            break;
        case PIT:
        case SPIKED_PIT:
/*JP
            fallverb = "falls";
*/
            fallverb = "落ちた";
            if (is_flyer(mptr) || is_floater(mptr)
                || (mtmp->wormno && count_wsegs(mtmp) > 5)
                || is_clinger(mptr)) {
                if (force_mintrap && !Sokoban) {
                    /* openfallingtrap; not inescapable here */
                    if (in_sight) {
                        seetrap(trap);
/*JP
                        pline("%s doesn't fall into the pit.", Monnam(mtmp));
*/
                        pline("%sは落し穴に落ちなかった．", Monnam(mtmp));
                    }
                    break; /* inescapable = FALSE; */
                }
                if (!inescapable)
                    break;               /* avoids trap */
#if 0 /*JP*/
                fallverb = "is dragged"; /* sokoban pit */
#else
                fallverb = "ずり落ちた"; /* sokoban pit */
#endif
            }
            if (!passes_walls(mptr))
                mtmp->mtrapped = 1;
            if (in_sight) {
#if 0 /*JP*/
                pline("%s %s into %s pit!", Monnam(mtmp), fallverb,
                      a_your[trap->madeby_u]);
#else
                pline("%sは%s落し穴に%s！", Monnam(mtmp),
                      set_you[trap->madeby_u], fallverb);
#endif
                if (mptr == &mons[PM_PIT_VIPER]
                    || mptr == &mons[PM_PIT_FIEND])
/*JP
                    pline("How pitiful.  Isn't that the pits?");
*/
                    pline("この落し穴はいい仕事をしている．");
                seetrap(trap);
            }
/*JP
            mselftouch(mtmp, "Falling, ", FALSE);
*/
            mselftouch(mtmp, "落下中，", FALSE);
            if (mtmp->mhp <= 0 || thitm(0, mtmp, (struct obj *) 0,
                                        rnd((tt == PIT) ? 6 : 10), FALSE))
                trapkilled = TRUE;
            break;
        case HOLE:
        case TRAPDOOR:
            if (!Can_fall_thru(&u.uz)) {
                impossible("mintrap: %ss cannot exist on this level.",
                           defsyms[trap_to_defsym(tt)].explanation);
                break; /* don't activate it after all */
            }
            if (is_flyer(mptr) || is_floater(mptr) || mptr == &mons[PM_WUMPUS]
                || (mtmp->wormno && count_wsegs(mtmp) > 5)
                || mptr->msize >= MZ_HUGE) {
                if (force_mintrap && !Sokoban) {
                    /* openfallingtrap; not inescapable here */
                    if (in_sight) {
                        seetrap(trap);
                        if (tt == TRAPDOOR)
                            pline(
/*JP
                            "A trap door opens, but %s doesn't fall through.",
*/
                            "落し扉が開いたが，%sは落ちなかった．",
                                  mon_nam(mtmp));
                        else /* (tt == HOLE) */
/*JP
                            pline("%s doesn't fall through the hole.",
*/
                            pline("%sは穴に落ちなかった．",
                                  Monnam(mtmp));
                    }
                    break; /* inescapable = FALSE; */
                }
                if (inescapable) { /* sokoban hole */
                    if (in_sight) {
/*JP
                        pline("%s seems to be yanked down!", Monnam(mtmp));
*/
                        pline("%sは引き落されたようだ！", Monnam(mtmp));
                        /* suppress message in mlevel_tele_trap() */
                        in_sight = FALSE;
                        seetrap(trap);
                    }
                } else
                    break;
            }
            /*FALLTHRU*/
        case LEVEL_TELEP:
        case MAGIC_PORTAL: {
            int mlev_res;

            mlev_res = mlevel_tele_trap(mtmp, trap, inescapable, in_sight);
            if (mlev_res)
                return mlev_res;
            break;
        }
        case TELEP_TRAP:
            mtele_trap(mtmp, trap, in_sight);
            break;
        case WEB:
            /* Monster in a web. */
            if (webmaker(mptr))
                break;
            if (amorphous(mptr) || is_whirly(mptr) || unsolid(mptr)) {
                if (acidic(mptr) || mptr == &mons[PM_GELATINOUS_CUBE]
                    || mptr == &mons[PM_FIRE_ELEMENTAL]) {
                    if (in_sight)
#if 0 /*JP*/
                        pline("%s %s %s spider web!", Monnam(mtmp),
                              (mptr == &mons[PM_FIRE_ELEMENTAL])
                                  ? "burns"
                                  : "dissolves",
                              a_your[trap->madeby_u]);
#else
                        pline("%s%sくもの巣を%s！",
                              Monnam(mtmp),
                              web_you[trap->madeby_u],
                              (mptr == &mons[PM_FIRE_ELEMENTAL]) ?
                              "焼いた" : "こなごなにした");
#endif
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                    break;
                }
                if (in_sight) {
#if 0 /*JP*/
                    pline("%s flows through %s spider web.", Monnam(mtmp),
                          a_your[trap->madeby_u]);
#else
                    pline("%sは%sくもの巣をするりと通り抜けた．", Monnam(mtmp),
                          web_you[trap->madeby_u]);
#endif
                    seetrap(trap);
                }
                break;
            }
            tear_web = FALSE;
            switch (monsndx(mptr)) {
            case PM_OWLBEAR: /* Eric Backus */
            case PM_BUGBEAR:
                if (!in_sight) {
/*JP
                    You_hear("the roaring of a confused bear!");
*/
                    You_hear("混乱の咆哮を聞いた！");
                    mtmp->mtrapped = 1;
                    break;
                }
            /* fall though */
            default:
                if (mptr->mlet == S_GIANT
                    /* exclude baby dragons and relatively short worms */
                    || (mptr->mlet == S_DRAGON && extra_nasty(mptr))
                    || (mtmp->wormno && count_wsegs(mtmp) > 5)) {
                    tear_web = TRUE;
                } else if (in_sight) {
#if 0 /*JP*/
                    pline("%s is caught in %s spider web.", Monnam(mtmp),
                          a_your[trap->madeby_u]);
#else
                    pline("%sは%sくもの巣につかまった．", Monnam(mtmp),
                          web_you[trap->madeby_u]);
#endif
                    seetrap(trap);
                }
                mtmp->mtrapped = tear_web ? 0 : 1;
                break;
            /* this list is fairly arbitrary; it deliberately
               excludes wumpus & giant/ettin zombies/mummies */
            case PM_TITANOTHERE:
            case PM_BALUCHITHERIUM:
            case PM_PURPLE_WORM:
            case PM_JABBERWOCK:
            case PM_IRON_GOLEM:
            case PM_BALROG:
            case PM_KRAKEN:
            case PM_MASTODON:
            case PM_ORION:
            case PM_NORN:
            case PM_CYCLOPS:
            case PM_LORD_SURTUR:
                tear_web = TRUE;
                break;
            }
            if (tear_web) {
                if (in_sight)
#if 0 /*JP*/
                    pline("%s tears through %s spider web!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
#else
                    pline("%sは%sくもの巣を引き裂いた！", Monnam(mtmp),
                          web_you[trap->madeby_u]);
#endif
                deltrap(trap);
                newsym(mtmp->mx, mtmp->my);
            } else if (force_mintrap && !mtmp->mtrapped) {
                if (in_sight) {
#if 0 /*JP*/
                    pline("%s avoids %s spider web!", Monnam(mtmp),
                          a_your[trap->madeby_u]);
#else
                    pline("%sは%sくもの巣を避けた！", Monnam(mtmp),
                          web_you[trap->madeby_u]);
#endif
                    seetrap(trap);
                }
            }
            break;
        case STATUE_TRAP:
            break;
        case MAGIC_TRAP:
            /* A magic trap.  Monsters usually immune. */
            if (!rn2(21))
                goto mfiretrap;
            break;
        case ANTI_MAGIC:
            /* similar to hero's case, more or less */
            if (!resists_magm(mtmp)) { /* lose spell energy */
                if (!mtmp->mcan && (attacktype(mptr, AT_MAGC)
                                    || attacktype(mptr, AT_BREA))) {
                    mtmp->mspec_used += d(2, 2);
                    if (in_sight) {
                        seetrap(trap);
/*JP
                        pline("%s seems lethargic.", Monnam(mtmp));
*/
                        pline("%s一瞬無気力感を感じた．", Monnam(mtmp));
                    }
                }
            } else { /* take some damage */
                int dmgval2 = rnd(4);

                if ((otmp = MON_WEP(mtmp)) != 0
                    && otmp->oartifact == ART_MAGICBANE)
                    dmgval2 += rnd(4);
                for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
                    if (otmp->oartifact
                        && defends_when_carried(AD_MAGM, otmp))
                        break;
                if (otmp)
                    dmgval2 += rnd(4);
                if (passes_walls(mptr))
                    dmgval2 = (dmgval2 + 3) / 4;

                if (in_sight)
                    seetrap(trap);
                if ((mtmp->mhp -= dmgval2) <= 0)
                    monkilled(mtmp,
                              in_sight
/*JP
                                  ? "compression from an anti-magic field"
*/
                                  ? "反魔法空間の圧縮"
                                  : (const char *) 0,
                              -AD_MAGM);
                if (mtmp->mhp <= 0)
                    trapkilled = TRUE;
                if (see_it)
                    newsym(trap->tx, trap->ty);
            }
            break;
        case LANDMINE:
            if (rn2(3))
                break; /* monsters usually don't set it off */
            if (is_flyer(mptr)) {
                boolean already_seen = trap->tseen;

                if (in_sight && !already_seen) {
/*JP
                    pline("A trigger appears in a pile of soil below %s.",
*/
                    pline("%sの足元の土の山に起爆スイッチが現われた．",
                          mon_nam(mtmp));
                    seetrap(trap);
                }
                if (rn2(3))
                    break;
                if (in_sight) {
                    newsym(mtmp->mx, mtmp->my);
#if 0 /*JP*/
                    pline_The("air currents set %s off!",
                              already_seen ? "a land mine" : "it");
#else
                    pline("空気の流れでスイッチが入った！");
#endif
                }
            } else if (in_sight) {
                newsym(mtmp->mx, mtmp->my);
#if 0 /*JP*/
                pline("KAABLAMM!!!  %s triggers %s land mine!", Monnam(mtmp),
                      a_your[trap->madeby_u]);
#else
                pline("ちゅどーん！！%sは%s地雷の起爆スイッチを踏んだ！", Monnam(mtmp),
                      set_you[trap->madeby_u]);
#endif
            }
            if (!in_sight)
/*JP
                pline("Kaablamm!  You hear an explosion in the distance!");
*/
                pline("ちゅどーん！あなたは遠方の爆発音を聞いた！");
            blow_up_landmine(trap);
            /* explosion might have destroyed a drawbridge; don't
               dish out more damage if monster is already dead */
            if (mtmp->mhp <= 0
                || thitm(0, mtmp, (struct obj *) 0, rnd(16), FALSE))
                trapkilled = TRUE;
            else {
                /* monsters recursively fall into new pit */
                if (mintrap(mtmp) == 2)
                    trapkilled = TRUE;
            }
            /* a boulder may fill the new pit, crushing monster */
            fill_pit(trap->tx, trap->ty);
            if (mtmp->mhp <= 0)
                trapkilled = TRUE;
            if (unconscious()) {
                multi = -1;
/*JP
                nomovemsg = "The explosion awakens you!";
*/
                nomovemsg = "爆発であなたは起きた！";
            }
            break;
        case POLY_TRAP:
            if (resists_magm(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
            } else if (!resist(mtmp, WAND_CLASS, 0, NOTELL)) {
                if (newcham(mtmp, (struct permonst *) 0, FALSE, FALSE))
                    /* we're done with mptr but keep it up to date */
                    mptr = mtmp->data;
                if (in_sight)
                    seetrap(trap);
            }
            break;
        case ROLLING_BOULDER_TRAP:
            if (!is_flyer(mptr)) {
                int style = ROLL | (in_sight ? 0 : LAUNCH_UNSEEN);

                newsym(mtmp->mx, mtmp->my);
                if (in_sight)
#if 0 /*JP*/
                    pline("Click! %s triggers %s.", Monnam(mtmp),
                          trap->tseen ? "a rolling boulder trap" : something);
#else
                    pline("カチッ！%sは%sのスイッチを踏んだ！", Monnam(mtmp),
                          trap->tseen ? "落岩の罠" : "何か");
#endif
                if (launch_obj(BOULDER, trap->launch.x, trap->launch.y,
                               trap->launch2.x, trap->launch2.y, style)) {
                    if (in_sight)
                        trap->tseen = TRUE;
                    if (mtmp->mhp <= 0)
                        trapkilled = TRUE;
                } else {
                    deltrap(trap);
                    newsym(mtmp->mx, mtmp->my);
                }
            }
            break;
        case VIBRATING_SQUARE:
            if (see_it && !Blind) {
                if (in_sight)
#if 0 /*JP*/
                    pline("You see a strange vibration beneath %s %s.",
                          s_suffix(mon_nam(mtmp)),
                          makeplural(mbodypart(mtmp, FOOT)));
#else
                    pline("%sの%sが不思議に振動しているのを見た．",
                          mon_nam(mtmp), mbodypart(mtmp, FOOT));
#endif
                else
/*JP
                    pline("You see the ground vibrate in the distance.");
*/
                    pline("遠くで地面が振動しているのを見た．");
                seetrap(trap);
            }
            break;
        default:
            impossible("Some monster encountered a strange trap of type %d.",
                       tt);
        }
    }
    if (trapkilled)
        return 2;
    return mtmp->mtrapped;
}

/* Combine cockatrice checks into single functions to avoid repeating code. */
void
instapetrify(str)
const char *str;
{
    if (Stone_resistance)
        return;
    if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
        return;
/*JP
    You("turn to stone...");
*/
    You("石になった．．．");
    killer.format = KILLED_BY;
    if (str != killer.name)
        Strcpy(killer.name, str ? str : "");
    done(STONING);
}

void
minstapetrify(mon, byplayer)
struct monst *mon;
boolean byplayer;
{
    if (resists_ston(mon))
        return;
    if (poly_when_stoned(mon->data)) {
        mon_to_stone(mon);
        return;
    }

    /* give a "<mon> is slowing down" message and also remove
       intrinsic speed (comparable to similar effect on the hero) */
    mon_adjust_speed(mon, -3, (struct obj *) 0);

    if (cansee(mon->mx, mon->my))
/*JP
        pline("%s turns to stone.", Monnam(mon));
*/
        pline("%sは石になった．", Monnam(mon));
    if (byplayer) {
        stoned = TRUE;
        xkilled(mon, 0);
    } else
        monstone(mon);
}

void
selftouch(arg)
const char *arg;
{
    char kbuf[BUFSZ];

    if (uwep && uwep->otyp == CORPSE && touch_petrifies(&mons[uwep->corpsenm])
        && !Stone_resistance) {
/*JP
        pline("%s touch the %s corpse.", arg, mons[uwep->corpsenm].mname);
*/
        pline("%s%sの死体に触った．", arg, mons[uwep->corpsenm].mname);
/*JP
        Sprintf(kbuf, "%s corpse", an(mons[uwep->corpsenm].mname));
*/
        Sprintf(kbuf, "%sの死体で", mons[uwep->corpsenm].mname);
        instapetrify(kbuf);
        /* life-saved; unwield the corpse if we can't handle it */
        if (!uarmg && !Stone_resistance)
            uwepgone();
    }
    /* Or your secondary weapon, if wielded [hypothetical; we don't
       allow two-weapon combat when either weapon is a corpse] */
    if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE
        && touch_petrifies(&mons[uswapwep->corpsenm]) && !Stone_resistance) {
/*JP
        pline("%s touch the %s corpse.", arg, mons[uswapwep->corpsenm].mname);
*/
        pline("%s%sの死体に触った．", arg, mons[uswapwep->corpsenm].mname);
/*JP
        Sprintf(kbuf, "%s corpse", an(mons[uswapwep->corpsenm].mname));
*/
        Sprintf(kbuf, "%sの死体で", mons[uswapwep->corpsenm].mname);
        instapetrify(kbuf);
        /* life-saved; unwield the corpse */
        if (!uarmg && !Stone_resistance)
            uswapwepgone();
    }
}

void
mselftouch(mon, arg, byplayer)
struct monst *mon;
const char *arg;
boolean byplayer;
{
    struct obj *mwep = MON_WEP(mon);

    if (mwep && mwep->otyp == CORPSE && touch_petrifies(&mons[mwep->corpsenm])
        && !resists_ston(mon)) {
        if (cansee(mon->mx, mon->my)) {
#if 0 /*JP*/
            pline("%s%s touches %s.", arg ? arg : "",
                  arg ? mon_nam(mon) : Monnam(mon),
                  corpse_xname(mwep, (const char *) 0, CXN_PFX_THE));
#else
            pline("%s%sは%sの死体に触った．", arg ? arg : "",
                  arg ? mon_nam(mon) : Monnam(mon),
                  corpse_xname(mwep, (const char *) 0, CXN_PFX_THE));
#endif
        }
        minstapetrify(mon, byplayer);
        /* if life-saved, might not be able to continue wielding */
        if (mon->mhp > 0 && !which_armor(mon, W_ARMG) && !resists_ston(mon))
            mwepgone(mon);
    }
}

/* start levitating */
void
float_up()
{
    if (u.utrap) {
        if (u.utraptype == TT_PIT) {
            u.utrap = 0;
/*JP
            You("float up, out of the pit!");
*/
            You("浮き上がって，落し穴から出た！");
            vision_full_recalc = 1; /* vision limits change */
            fill_pit(u.ux, u.uy);
        } else if (u.utraptype == TT_INFLOOR) {
/*JP
            Your("body pulls upward, but your %s are still stuck.",
*/
            Your("体は引き上げられた．しかし%sはまだはまっている．",
                 makeplural(body_part(LEG)));
        } else {
/*JP
            You("float up, only your %s is still stuck.", body_part(LEG));
*/
            You("浮き出た．%sだけがはまっている．", body_part(LEG));
        }
#if 0
    } else if (Is_waterlevel(&u.uz)) {
/*JP
        pline("It feels as though you've lost some weight.");
*/
        You("まるで体重が減ったように感じた．");
#endif
    } else if (u.uinwater) {
        spoteffects(TRUE);
    } else if (u.uswallow) {
#if 0 /*JP*/
        You(is_animal(u.ustuck->data) ? "float away from the %s."
                                      : "spiral up into %s.",
            is_animal(u.ustuck->data) ? surface(u.ux, u.uy)
                                      : mon_nam(u.ustuck));
#else
        You(is_animal(u.ustuck->data) ? "%sの中で浮いた．"
                                      : "%sの中でぐるぐる回転した．",
            is_animal(u.ustuck->data) ? surface(u.ux, u.uy)
                                      : mon_nam(u.ustuck));
#endif
    } else if (Hallucination) {
/*JP
        pline("Up, up, and awaaaay!  You're walking on air!");
*/
        pline("上れ，上れ，上れぇぇぇぇ！あなたは空中を歩いている！");
    } else if (Is_airlevel(&u.uz)) {
/*JP
        You("gain control over your movements.");
*/
        You("うまく動けるようになった．");
    } else {
/*JP
        You("start to float in the air!");
*/
        You("空中に浮きはじめた！");
    }
    if (u.usteed && !is_floater(u.usteed->data)
        && !is_flyer(u.usteed->data)) {
        if (Lev_at_will) {
/*JP
            pline("%s magically floats up!", Monnam(u.usteed));
*/
            pline("%sは魔法の力で浮いた！", Monnam(u.usteed));
        } else {
/*JP
            You("cannot stay on %s.", mon_nam(u.usteed));
*/
            You("%sの上に乗っていられない．", mon_nam(u.usteed));
            dismount_steed(DISMOUNT_GENERIC);
        }
    }
    if (Flying)
/*JP
        You("are no longer able to control your flight.");
*/
        You("空中でうまく動けなくなった．");
    BFlying |= I_SPECIAL;
    return;
}

void
fill_pit(x, y)
int x, y;
{
    struct obj *otmp;
    struct trap *t;

    if ((t = t_at(x, y)) && ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT))
        && (otmp = sobj_at(BOULDER, x, y))) {
        obj_extract_self(otmp);
/*JP
        (void) flooreffects(otmp, x, y, "settle");
*/
        (void) flooreffects(otmp, x, y, "はまる");
    }
}

/* stop levitating */
int
float_down(hmask, emask)
long hmask, emask; /* might cancel timeout */
{
    register struct trap *trap = (struct trap *) 0;
    d_level current_dungeon_level;
    boolean no_msg = FALSE;

    HLevitation &= ~hmask;
    ELevitation &= ~emask;
    if (Levitation)
        return 0; /* maybe another ring/potion/boots */
    if (BLevitation) {
        /* Levitation is blocked, so hero is not actually floating
           hence shouldn't have float_down effects and feedback */
        float_vs_flight(); /* before nomul() rather than after */
        return 0;
    }
    nomul(0); /* stop running or resting */
    if (BFlying) {
        /* controlled flight no longer overridden by levitation */
        BFlying &= ~I_SPECIAL;
        if (Flying) {
/*JP
            You("have stopped levitating and are now flying.");
*/
            You("空中浮遊を止めて空を飛びはじめた．");
            return 1;
        }
    }
    if (u.uswallow) {
#if 0 /*JP*/
        You("float down, but you are still %s.",
            is_animal(u.ustuck->data) ? "swallowed" : "engulfed");
#else
        You("着地したが，まだ飲み込まれたままだ．");
#endif
        return 1;
    }

    if (Punished && !carried(uball)
        && (is_pool(uball->ox, uball->oy)
            || ((trap = t_at(uball->ox, uball->oy))
                && ((trap->ttyp == PIT) || (trap->ttyp == SPIKED_PIT)
                    || (trap->ttyp == TRAPDOOR) || (trap->ttyp == HOLE))))) {
        u.ux0 = u.ux;
        u.uy0 = u.uy;
        u.ux = uball->ox;
        u.uy = uball->oy;
        movobj(uchain, uball->ox, uball->oy);
        newsym(u.ux0, u.uy0);
        vision_full_recalc = 1; /* in case the hero moved. */
    }
    /* check for falling into pool - added by GAN 10/20/86 */
    if (!Flying) {
        if (!u.uswallow && u.ustuck) {
            if (sticks(youmonst.data))
#if 0 /*JP:T*/
                You("aren't able to maintain your hold on %s.",
                    mon_nam(u.ustuck));
#else
                You("%sをつかまえ続けていられなくなった．",
                    mon_nam(u.ustuck));
#endif
            else
#if 0 /*JP:T*/
                pline("Startled, %s can no longer hold you!",
                      mon_nam(u.ustuck));
#else
                pline("%sは驚いてあなたを放してしまった！",
                      mon_nam(u.ustuck));
#endif
            u.ustuck = 0;
        }
        /* kludge alert:
         * drown() and lava_effects() print various messages almost
         * every time they're called which conflict with the "fall
         * into" message below.  Thus, we want to avoid printing
         * confusing, duplicate or out-of-order messages.
         * Use knowledge of the two routines as a hack -- this
         * should really be handled differently -dlc
         */
        if (is_pool(u.ux, u.uy) && !Wwalking && !Swimming && !u.uinwater)
            no_msg = drown();

        if (is_lava(u.ux, u.uy)) {
            (void) lava_effects();
            no_msg = TRUE;
        }
    }
    if (!trap) {
        trap = t_at(u.ux, u.uy);
        if (Is_airlevel(&u.uz)) {
/*JP
            You("begin to tumble in place.");
*/
            You("その場でひっくり返りはじめた．");
        } else if (Is_waterlevel(&u.uz) && !no_msg) {
/*JP
            You_feel("heavier.");
*/
            You("重くなったような気がした．");
        /* u.uinwater msgs already in spoteffects()/drown() */
        } else if (!u.uinwater && !no_msg) {
            if (!(emask & W_SADDLE)) {
                if (Sokoban && trap) {
                    /* Justification elsewhere for Sokoban traps is based
                     * on air currents.  This is consistent with that.
                     * The unexpected additional force of the air currents
                     * once levitation ceases knocks you off your feet.
                     */
                    if (Hallucination)
/*JP
                        pline("Bummer!  You've crashed.");
*/
                        pline("やめてぇ！あなたは叩きつけられた．");
                    else
/*JP
                        You("fall over.");
*/
                        You("つまづいた．");
/*JP
                    losehp(rnd(2), "dangerous winds", KILLED_BY);
*/
                    losehp(rnd(2), "危険な風で", KILLED_BY);
                    if (u.usteed)
                        dismount_steed(DISMOUNT_FELL);
/*JP
                    selftouch("As you fall, you");
*/
                    selftouch("落ちながら，あなたは");
                } else if (u.usteed && (is_floater(u.usteed->data)
                                        || is_flyer(u.usteed->data))) {
/*JP
                    You("settle more firmly in the saddle.");
*/
                    You("よりしっかりと鞍に納まった．");
                } else if (Hallucination) {
#if 0 /*JP*/
                    pline("Bummer!  You've %s.",
                          is_pool(u.ux, u.uy)
                             ? "splashed down"
                             : "hit the ground");
#else
                    pline("やめてぇ！あなたは%sに叩きつけられた．",
                          is_pool(u.ux, u.uy)
                             ? "水面"
                             : "地面");
#endif
                } else {
/*JP
                    You("float gently to the %s.", surface(u.ux, u.uy));
*/
                    You("静かに%sまで辿りついた．", surface(u.ux, u.uy));
                }
            }
        }
    }

    /* can't rely on u.uz0 for detecting trap door-induced level change;
       it gets changed to reflect the new level before we can check it */
    assign_level(&current_dungeon_level, &u.uz);
    if (trap) {
        switch (trap->ttyp) {
        case STATUE_TRAP:
            break;
        case HOLE:
        case TRAPDOOR:
            if (!Can_fall_thru(&u.uz) || u.ustuck)
                break;
            /*FALLTHRU*/
        default:
            if (!u.utrap) /* not already in the trap */
                dotrap(trap, 0);
        }
    }
    if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) && !u.uswallow
        /* falling through trap door calls goto_level,
           and goto_level does its own pickup() call */
        && on_level(&u.uz, &current_dungeon_level))
        (void) pickup(1);
    return 1;
}

/* shared code for climbing out of a pit */
void
climb_pit()
{
    if (!u.utrap || u.utraptype != TT_PIT)
        return;

    if (Passes_walls) {
        /* marked as trapped so they can pick things up */
/*JP
        You("ascend from the pit.");
*/
        You("落し穴を上っていった．");
        u.utrap = 0;
        fill_pit(u.ux, u.uy);
        vision_full_recalc = 1; /* vision limits change */
    } else if (!rn2(2) && sobj_at(BOULDER, u.ux, u.uy)) {
        Your("%s gets stuck in a crevice.", body_part(LEG));
        display_nhwindow(WIN_MESSAGE, FALSE);
        clear_nhwindow(WIN_MESSAGE);
/*JP
        You("free your %s.", body_part(LEG));
*/
        Your("%sは自由になった．", body_part(LEG));
    } else if ((Flying || is_clinger(youmonst.data)) && !Sokoban) {
        /* eg fell in pit, then poly'd to a flying monster;
           or used '>' to deliberately enter it */
/*JP
        You("%s from the pit.", Flying ? "fly" : "climb");
*/
        You("落し穴から%sいった．", Flying ? "上って" : "登って");
        u.utrap = 0;
        fill_pit(u.ux, u.uy);
        vision_full_recalc = 1; /* vision limits change */
    } else if (!(--u.utrap)) {
#if 0 /*JP*/
        You("%s to the edge of the pit.",
            (Sokoban && Levitation)
                ? "struggle against the air currents and float"
                : u.usteed ? "ride" : "crawl");
#else
        You("%s落し穴の端にたどり着いた．",
            (Sokoban && Levitation)
                ? "空気の流れの中でもがきながら" : "");
#endif
        fill_pit(u.ux, u.uy);
        vision_full_recalc = 1; /* vision limits change */
    } else if (u.dz || flags.verbose) {
        if (u.usteed)
/*JP
            Norep("%s is still in a pit.", upstart(y_monnam(u.usteed)));
*/
            Norep("%sはまだ落し穴の中にいる．", y_monnam(u.usteed));
        else
#if 0 /*JP*/
            Norep((Hallucination && !rn2(5))
                      ? "You've fallen, and you can't get up."
                      : "You are still in a pit.");
#else
            Norep((Hallucination && !rn2(5))
                      ? "あなたは落ち，上がれない．"
                      : "あなたはまだ落し穴の中にいる．");
#endif
    }
}

STATIC_OVL void
dofiretrap(box)
struct obj *box; /* null for floor trap */
{
    boolean see_it = !Blind;
    int num, alt;

    /* Bug: for box case, the equivalent of burn_floor_objects() ought
     * to be done upon its contents.
     */

    if ((box && !carried(box)) ? is_pool(box->ox, box->oy) : Underwater) {
/*JP
        pline("A cascade of steamy bubbles erupts from %s!",
*/
        pline("蒸気の泡が%sからしゅーっと発生した！",
              the(box ? xname(box) : surface(u.ux, u.uy)));
        if (Fire_resistance)
/*JP
            You("are uninjured.");
*/
            You("傷つかない．");
        else
/*JP
            losehp(rnd(3), "boiling water", KILLED_BY);
*/
            losehp(rnd(3), "沸騰した水で", KILLED_BY);
        return;
    }
#if 0 /*JP*/
    pline("A %s %s from %s!", tower_of_flame, box ? "bursts" : "erupts",
          the(box ? xname(box) : surface(u.ux, u.uy)));
#else
    pline("火柱が%sから%s！", box ? xname(box) : surface(u.ux,u.uy),
          box ? "吹き出した" : "立ちのぼった");
#endif
    if (Fire_resistance) {
        shieldeff(u.ux, u.uy);
        num = rn2(2);
    } else if (Upolyd) {
        num = d(2, 4);
        switch (u.umonnum) {
        case PM_PAPER_GOLEM:
            alt = u.mhmax;
            break;
        case PM_STRAW_GOLEM:
            alt = u.mhmax / 2;
            break;
        case PM_WOOD_GOLEM:
            alt = u.mhmax / 4;
            break;
        case PM_LEATHER_GOLEM:
            alt = u.mhmax / 8;
            break;
        default:
            alt = 0;
            break;
        }
        if (alt > num)
            num = alt;
        if (u.mhmax > mons[u.umonnum].mlevel)
            u.mhmax -= rn2(min(u.mhmax, num + 1)), context.botl = 1;
    } else {
        num = d(2, 4);
        if (u.uhpmax > u.ulevel)
            u.uhpmax -= rn2(min(u.uhpmax, num + 1)), context.botl = 1;
    }
    if (!num)
/*JP
        You("are uninjured.");
*/
        You("傷つかない．");
    else
#if 0 /*JP*/
        losehp(num, tower_of_flame, KILLED_BY_AN); /* fire damage */
#else
        losehp(num, "火柱で", KILLED_BY_AN); /* fire damage */
#endif
    burn_away_slime();

    if (burnarmor(&youmonst) || rn2(3)) {
        destroy_item(SCROLL_CLASS, AD_FIRE);
        destroy_item(SPBOOK_CLASS, AD_FIRE);
        destroy_item(POTION_CLASS, AD_FIRE);
    }
    if (!box && burn_floor_objects(u.ux, u.uy, see_it, TRUE) && !see_it)
/*JP
        You("smell paper burning.");
*/
        You("紙のこげる匂いがした．");
    if (is_ice(u.ux, u.uy))
        melt_ice(u.ux, u.uy, (char *) 0);
}

STATIC_OVL void
domagictrap()
{
    register int fate = rnd(20);

    /* What happened to the poor sucker? */

    if (fate < 10) {
        /* Most of the time, it creates some monsters. */
        register int cnt = rnd(4);

        if (!resists_blnd(&youmonst)) {
/*JP
            You("are momentarily blinded by a flash of light!");
*/
            You("まばゆい光で一瞬目がくらんだ！");
            make_blinded((long) rn1(5, 10), FALSE);
            if (!Blind)
                Your1(vision_clears);
        } else if (!Blind) {
/*JP
            You_see("a flash of light!");
*/
            You("まばゆい光を浴びた！");
        } else
/*JP
            You_hear("a deafening roar!");
*/
            You_hear("耳をつんざくような咆哮を聞いた！");
        incr_itimeout(&HDeaf, rn1(20, 30));
        while (cnt--)
            (void) makemon((struct permonst *) 0, u.ux, u.uy, NO_MM_FLAGS);
    } else
        switch (fate) {
        case 10:
        case 11:
            /* sometimes nothing happens */
            break;
        case 12: /* a flash of fire */
            dofiretrap((struct obj *) 0);
            break;

        /* odd feelings */
        case 13:
/*JP
            pline("A shiver runs up and down your %s!", body_part(SPINE));
*/
            pline("震えがあなたの%sを走った！", body_part(SPINE));
            break;
        case 14:
#if 0 /*JP*/
            You_hear(Hallucination ? "the moon howling at you."
                                   : "distant howling.");
#else
            You_hear(Hallucination ? "月が吠えているのを聞いた．"
                                   : "遠方の遠吠を聞いた．");
#endif
            break;
        case 15:
            if (on_level(&u.uz, &qstart_level))
#if 0 /*JP*/
                You_feel(
                    "%slike the prodigal son.",
                    (flags.female || (Upolyd && is_neuter(youmonst.data)))
                        ? "oddly "
                        : "");
#else /*JP 聖書のルカ伝より */
                You("%s放蕩息子のようになったような気がした．",
                    (flags.female || (Upolyd && is_neuter(youmonst.data)))
                    ? "異常に"
                    : "");
#endif
            else
#if 0 /*JP*/
                You("suddenly yearn for %s.",
                    Hallucination
                        ? "Cleveland"
                        : (In_quest(&u.uz) || at_dgn_entrance("The Quest"))
                              ? "your nearby homeland"
                              : "your distant homeland");
#else
                You("突然%sが恋しくなった．",
                    Hallucination ? "青森"
                        : (In_quest(&u.uz) || at_dgn_entrance("クエスト"))
                            ? "すぐそこにある故郷"
                            : "はるかかなたの故郷");
#endif
            break;
        case 16:
/*JP
            Your("pack shakes violently!");
*/
            Your("袋は激しく揺れた！");
            break;
        case 17:
/*JP
            You(Hallucination ? "smell hamburgers." : "smell charred flesh.");
*/
            You(Hallucination ? "ハンバーガーの匂いがした．" : "黒焦げの肉の匂いがした．");
            break;
        case 18:
/*JP
            You_feel("tired.");
*/
            You("疲れを感じた．");
            break;

        /* very occasionally something nice happens. */
        case 19: { /* tame nearby monsters */
            int i, j;
            struct monst *mtmp;

            (void) adjattrib(A_CHA, 1, FALSE);
            for (i = -1; i <= 1; i++)
                for (j = -1; j <= 1; j++) {
                    if (!isok(u.ux + i, u.uy + j))
                        continue;
                    mtmp = m_at(u.ux + i, u.uy + j);
                    if (mtmp)
                        (void) tamedog(mtmp, (struct obj *) 0);
                }
            break;
        }
        case 20: { /* uncurse stuff */
            struct obj pseudo;
            long save_conf = HConfusion;

            pseudo = zeroobj; /* neither cursed nor blessed,
                                 and zero out oextra */
            pseudo.otyp = SCR_REMOVE_CURSE;
            HConfusion = 0L;
            (void) seffects(&pseudo);
            HConfusion = save_conf;
            break;
        }
        default:
            break;
        }
}

/* Set an item on fire.
 *   "force" means not to roll a luck-based protection check for the
 *     item.
 *   "x" and "y" are the coordinates to dump the contents of a
 *     container, if it burns up.
 *
 * Return whether the object was destroyed.
 */
boolean
fire_damage(obj, force, x, y)
struct obj *obj;
boolean force;
xchar x, y;
{
    int chance;
    struct obj *otmp, *ncobj;
    int in_sight = !Blind && couldsee(x, y); /* Don't care if it's lit */
    int dindx;

    /* object might light in a controlled manner */
    if (catch_lit(obj))
        return FALSE;

    if (Is_container(obj)) {
        switch (obj->otyp) {
        case ICE_BOX:
            return FALSE; /* Immune */
        case CHEST:
            chance = 40;
            break;
        case LARGE_BOX:
            chance = 30;
            break;
        default:
            chance = 20;
            break;
        }
        if ((!force && (Luck + 5) > rn2(chance))
            || (is_flammable(obj) && obj->oerodeproof))
            return FALSE;
        /* Container is burnt up - dump contents out */
        if (in_sight)
/*JP
            pline("%s catches fire and burns.", Yname2(obj));
*/
            pline("%sは火がついて燃えた．", Yname2(obj));
        if (Has_contents(obj)) {
            if (in_sight)
/*JP
                pline("Its contents fall out.");
*/
                pline("中身が出てきた．");
            for (otmp = obj->cobj; otmp; otmp = ncobj) {
                ncobj = otmp->nobj;
                obj_extract_self(otmp);
                if (!flooreffects(otmp, x, y, ""))
                    place_object(otmp, x, y);
            }
        }
        setnotworn(obj);
        delobj(obj);
        return TRUE;
    } else if (!force && (Luck + 5) > rn2(20)) {
        /*  chance per item of sustaining damage:
          *     max luck (Luck==13):    10%
          *     avg luck (Luck==0):     75%
          *     awful luck (Luck<-4):  100%
          */
        return FALSE;
    } else if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS) {
        if (obj->otyp == SCR_FIRE || obj->otyp == SPE_FIREBALL)
            return FALSE;
        if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
            if (in_sight)
/*JP
                pline("Smoke rises from %s.", the(xname(obj)));
*/
                pline("%sから煙があがった．", the(xname(obj)));
            return FALSE;
        }
        dindx = (obj->oclass == SCROLL_CLASS) ? 3 : 4;
        if (in_sight)
#if 0 /*JP*/
            pline("%s %s.", Yname2(obj),
                  destroy_strings[dindx][(obj->quan > 1L)]);
#else
            pline("%sは%s．", Yname2(obj),
                  destroy_strings[dindx][(obj->quan > 1L)]);
#endif
        setnotworn(obj);
        delobj(obj);
        return TRUE;
    } else if (obj->oclass == POTION_CLASS) {
        dindx = (obj->otyp != POT_OIL) ? 1 : 2;
        if (in_sight)
#if 0 /*JP*/
            pline("%s %s.", Yname2(obj),
                  destroy_strings[dindx][(obj->quan > 1L)]);
#else
            pline("%sは%s．", Yname2(obj),
                  destroy_strings[dindx][(obj->quan > 1L)]);
#endif
        setnotworn(obj);
        delobj(obj);
        return TRUE;
    } else if (erode_obj(obj, (char *) 0, ERODE_BURN, EF_DESTROY)
               == ER_DESTROYED) {
        return TRUE;
    }
    return FALSE;
}

/*
 * Apply fire_damage() to an entire chain.
 *
 * Return number of objects destroyed. --ALI
 */
int
fire_damage_chain(chain, force, here, x, y)
struct obj *chain;
boolean force, here;
xchar x, y;
{
    struct obj *obj, *nobj;
    int num = 0;
    for (obj = chain; obj; obj = nobj) {
        nobj = here ? obj->nexthere : obj->nobj;
        if (fire_damage(obj, force, x, y))
            ++num;
    }

    if (num && (Blind && !couldsee(x, y)))
/*JP
        You("smell smoke.");
*/
        You("煙のにおいがした．");
    return num;
}

void
acid_damage(obj)
struct obj *obj;
{
    /* Scrolls but not spellbooks can be erased by acid. */
    struct monst *victim;
    boolean vismon;

    if (!obj)
        return;

    victim = carried(obj) ? &youmonst : mcarried(obj) ? obj->ocarry : NULL;
    vismon = victim && (victim != &youmonst) && canseemon(victim);

    if (obj->greased) {
        grease_protect(obj, (char *) 0, victim);
    } else if (obj->oclass == SCROLL_CLASS && obj->otyp != SCR_BLANK_PAPER) {
        if (obj->otyp != SCR_BLANK_PAPER
#ifdef MAIL
            && obj->otyp != SCR_MAIL
#endif
            ) {
            if (!Blind) {
                if (victim == &youmonst)
/*JP
                    pline("Your %s.", aobjnam(obj, "fade"));
*/
                    pline("%sの文字は薄れた．", xname(obj));
                else if (vismon)
#if 0 /*JP*/
                    pline("%s %s.", s_suffix(Monnam(victim)),
                          aobjnam(obj, "fade"));
#else
                    pline("%sの%sの文字は薄れた．", Monnam(victim),
                          xname(obj));
#endif
            }
        }
        obj->otyp = SCR_BLANK_PAPER;
        obj->spe = 0;
        obj->dknown = 0;
    } else
        erode_obj(obj, (char *) 0, ERODE_CORRODE, EF_GREASE | EF_VERBOSE);
}

/* context for water_damage(), managed by water_damage_chain();
   when more than one stack of potions of acid explode while processing
   a chain of objects, use alternate phrasing after the first message */
static struct h2o_ctx {
    int dkn_boom, unk_boom; /* track dknown, !dknown separately */
    boolean ctx_valid;
} acid_ctx = { 0, 0, FALSE };

/* Get an object wet and damage it appropriately.
 *   "ostr", if present, is used instead of the object name in some
 *     messages.
 *   "force" means not to roll luck to protect some objects.
 * Returns an erosion return value (ER_*)
 */
int
water_damage(obj, ostr, force)
struct obj *obj;
const char *ostr;
boolean force;
{
    if (!obj)
        return ER_NOTHING;

    if (snuff_lit(obj))
        return ER_DAMAGED;

    if (!ostr)
        ostr = cxname(obj);

    if (obj->otyp == CAN_OF_GREASE && obj->spe > 0) {
        return ER_NOTHING;
    } else if (obj->otyp == TOWEL && obj->spe < 7) {
        wet_a_towel(obj, rnd(7), TRUE);
        return ER_NOTHING;
    } else if (obj->greased) {
        if (!rn2(2))
            obj->greased = 0;
        if (carried(obj))
            update_inventory();
        return ER_GREASED;
    } else if (Is_container(obj) && !Is_box(obj)
               && (obj->otyp != OILSKIN_SACK || (obj->cursed && !rn2(3)))) {
        if (carried(obj))
/*JP
            pline("Water gets into your %s!", ostr);
*/
            pline("水が%sの中に入った！", ostr);

        water_damage_chain(obj->cobj, FALSE);
        return ER_NOTHING;
    } else if (!force && (Luck + 5) > rn2(20)) {
        /*  chance per item of sustaining damage:
            *   max luck:               10%
            *   avg luck (Luck==0):     75%
            *   awful luck (Luck<-4):  100%
            */
        return ER_NOTHING;
    } else if (obj->oclass == SCROLL_CLASS) {
#ifdef MAIL
        if (obj->otyp == SCR_MAIL)
            return 0;
#endif
        if (carried(obj))
/*JP
            pline("Your %s %s.", ostr, vtense(ostr, "fade"));
*/
            pline("%sの文字は薄れた．", ostr);

        obj->otyp = SCR_BLANK_PAPER;
        obj->dknown = 0;
        obj->spe = 0;
        if (carried(obj))
            update_inventory();
        return ER_DAMAGED;
    } else if (obj->oclass == SPBOOK_CLASS) {
        if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
/*JP
            pline("Steam rises from %s.", the(xname(obj)));
*/
            pline("蒸気が%sから立ちのぼった．", xname(obj));
            return 0;
        }

        if (carried(obj))
/*JP
            pline("Your %s %s.", ostr, vtense(ostr, "fade"));
*/
            pline("%sの文字は薄れた．", ostr);

        if (obj->otyp == SPE_NOVEL) {
            obj->novelidx = 0;
            free_oname(obj);
        }

        obj->otyp = SPE_BLANK_PAPER;
        obj->dknown = 0;
        if (carried(obj))
            update_inventory();
        return ER_DAMAGED;
    } else if (obj->oclass == POTION_CLASS) {
        if (obj->otyp == POT_ACID) {
            char *bufp;
            boolean one = (obj->quan == 1L), update = carried(obj),
                    exploded = FALSE;

            if (Blind && !carried(obj))
                obj->dknown = 0;
            if (acid_ctx.ctx_valid)
                exploded = ((obj->dknown ? acid_ctx.dkn_boom
                                         : acid_ctx.unk_boom) > 0);
            /* First message is
             * "a [potion|<color> potion|potion of acid] explodes"
             * depending on obj->dknown (potion has been seen) and
             * objects[POT_ACID].oc_name_known (fully discovered),
             * or "some {plural version} explode" when relevant.
             * Second and subsequent messages for same chain and
             * matching dknown status are
             * "another [potion|<color> &c] explodes" or plural
             * variant.
             */
            bufp = simpleonames(obj);
#if 0 /*JP*/
            pline("%s %s %s!", /* "A potion explodes!" */
                  !exploded ? (one ? "A" : "Some")
                            : (one ? "Another" : "More"),
                  bufp, vtense(bufp, "explode"));
#else
            pline("%sは爆発した！", bufp);
#endif
            if (acid_ctx.ctx_valid) {
                if (obj->dknown)
                    acid_ctx.dkn_boom++;
                else
                    acid_ctx.unk_boom++;
            }
            setnotworn(obj);
            delobj(obj);
            if (update)
                update_inventory();
            return ER_DESTROYED;
        } else if (obj->odiluted) {
            if (carried(obj))
/*JP
                pline("Your %s %s further.", ostr, vtense(ostr, "dilute"));
*/
                pline("%sはさらに薄まった．", ostr);

            obj->otyp = POT_WATER;
            obj->dknown = 0;
            obj->blessed = obj->cursed = 0;
            obj->odiluted = 0;
            if (carried(obj))
                update_inventory();
            return ER_DAMAGED;
        } else if (obj->otyp != POT_WATER) {
            if (carried(obj))
/*JP
                pline("Your %s %s.", ostr, vtense(ostr, "dilute"));
*/
                pline("%sは薄まった．", ostr);

            obj->odiluted++;
            if (carried(obj))
                update_inventory();
            return ER_DAMAGED;
        }
    } else {
        return erode_obj(obj, ostr, ERODE_RUST, EF_NONE);
    }
    return ER_NOTHING;
}

void
water_damage_chain(obj, here)
struct obj *obj;
boolean here;
{
    struct obj *otmp;

    /* initialize acid context: so far, neither seen (dknown) potions of
       acid nor unseen have exploded during this water damage sequence */
    acid_ctx.dkn_boom = acid_ctx.unk_boom = 0;
    acid_ctx.ctx_valid = TRUE;

    for (; obj; obj = otmp) {
        otmp = here ? obj->nexthere : obj->nobj;
        water_damage(obj, (char *) 0, FALSE);
    }

    /* reset acid context */
    acid_ctx.dkn_boom = acid_ctx.unk_boom = 0;
    acid_ctx.ctx_valid = FALSE;
}

/*
 * This function is potentially expensive - rolling
 * inventory list multiple times.  Luckily it's seldom needed.
 * Returns TRUE if disrobing made player unencumbered enough to
 * crawl out of the current predicament.
 */
STATIC_OVL boolean
emergency_disrobe(lostsome)
boolean *lostsome;
{
    int invc = inv_cnt(TRUE);

    while (near_capacity() > (Punished ? UNENCUMBERED : SLT_ENCUMBER)) {
        register struct obj *obj, *otmp = (struct obj *) 0;
        register int i;

        /* Pick a random object */
        if (invc > 0) {
            i = rn2(invc);
            for (obj = invent; obj; obj = obj->nobj) {
                /*
                 * Undroppables are: body armor, boots, gloves,
                 * amulets, and rings because of the time and effort
                 * in removing them + loadstone and other cursed stuff
                 * for obvious reasons.
                 */
                if (!((obj->otyp == LOADSTONE && obj->cursed) || obj == uamul
                      || obj == uleft || obj == uright || obj == ublindf
                      || obj == uarm || obj == uarmc || obj == uarmg
                      || obj == uarmf || obj == uarmu
                      || (obj->cursed && (obj == uarmh || obj == uarms))
                      || welded(obj)))
                    otmp = obj;
                /* reached the mark and found some stuff to drop? */
                if (--i < 0 && otmp)
                    break;

                /* else continue */
            }
        }
        if (!otmp)
            return FALSE; /* nothing to drop! */
        if (otmp->owornmask)
            remove_worn_item(otmp, FALSE);
        *lostsome = TRUE;
        dropx(otmp);
        invc--;
    }
    return TRUE;
}


/*  return TRUE iff player relocated */
boolean
drown()
{
    const char *pool_of_water;
    boolean inpool_ok = FALSE, crawl_ok;
    int i, x, y;

    /* happily wading in the same contiguous pool */
    if (u.uinwater && is_pool(u.ux - u.dx, u.uy - u.dy)
        && (Swimming || Amphibious)) {
        /* water effects on objects every now and then */
        if (!rn2(5))
            inpool_ok = TRUE;
        else
            return FALSE;
    }

    if (!u.uinwater) {
#if 0 /*JP*/
        You("%s into the water%c", Is_waterlevel(&u.uz) ? "plunge" : "fall",
            Amphibious || Swimming ? '.' : '!');
#else
        You("水の中に%s%s", Is_waterlevel(&u.uz) ? "飛びこんだ" : "落ちた",
            Amphibious || Swimming ? "．" : "！");
#endif
        if (!Swimming && !Is_waterlevel(&u.uz))
/*JP
            You("sink like %s.", Hallucination ? "the Titanic" : "a rock");
*/
            You("%sのように沈んだ．", Hallucination ? "タイタニック号" : "岩");
    }

    water_damage_chain(invent, FALSE);

    if (u.umonnum == PM_GREMLIN && rn2(3))
        (void) split_mon(&youmonst, (struct monst *) 0);
    else if (u.umonnum == PM_IRON_GOLEM) {
/*JP
        You("rust!");
*/
        You("錆びた！");
        i = Maybe_Half_Phys(d(2, 6));
        if (u.mhmax > i)
            u.mhmax -= i;
/*JP
        losehp(i, "rusting away", KILLED_BY);
*/
        losehp(i, "完全に錆びて", KILLED_BY);
    }
    if (inpool_ok)
        return FALSE;

    if ((i = number_leashed()) > 0) {
#if 0 /*JP*/
        pline_The("leash%s slip%s loose.", (i > 1) ? "es" : "",
                  (i > 1) ? "" : "s");
#else
        pline("紐がゆるんだ．");
#endif
        unleash_all();
    }

    if (Amphibious || Swimming) {
        if (Amphibious) {
            if (flags.verbose)
/*JP
                pline("But you aren't drowning.");
*/
                pline("しかし，あなたは溺れなかった．");
            if (!Is_waterlevel(&u.uz)) {
                if (Hallucination)
/*JP
                    Your("keel hits the bottom.");
*/
                    You("底にニードロップを決めた．");
                else
/*JP
                    You("touch bottom.");
*/
                    You("底についた．");
            }
        }
        if (Punished) {
            unplacebc();
            placebc();
        }
        vision_recalc(2); /* unsee old position */
        u.uinwater = 1;
        under_water(1);
        vision_full_recalc = 1;
        return FALSE;
    }
    if ((Teleportation || can_teleport(youmonst.data)) && !Unaware
        && (Teleport_control || rn2(3) < Luck + 2)) {
#if 0 /*JP*/
        You("attempt a teleport spell."); /* utcsri!carroll */
#else
        You("瞬間移動の呪文を唱えてみた．");
#endif
        if (!level.flags.noteleport) {
            (void) dotele();
            if (!is_pool(u.ux, u.uy))
                return TRUE;
        } else
/*JP
            pline_The("attempted teleport spell fails.");
*/
            pline("瞬間移動の呪文は失敗した．");
    }
    if (u.usteed) {
        dismount_steed(DISMOUNT_GENERIC);
        if (!is_pool(u.ux, u.uy))
            return TRUE;
    }
    crawl_ok = FALSE;
    x = y = 0; /* lint suppression */
    /* if sleeping, wake up now so that we don't crawl out of water
       while still asleep; we can't do that the same way that waking
       due to combat is handled; note unmul() clears u.usleep */
    if (u.usleep)
/*JP
        unmul("Suddenly you wake up!");
*/
        unmul("突然あなたは目が覚めた！");
    /* being doused will revive from fainting */
    if (is_fainted())
        reset_faint();
    /* can't crawl if unable to move (crawl_ok flag stays false) */
    if (multi < 0 || (Upolyd && !youmonst.data->mmove))
        goto crawl;
    /* look around for a place to crawl to */
    for (i = 0; i < 100; i++) {
        x = rn1(3, u.ux - 1);
        y = rn1(3, u.uy - 1);
        if (crawl_destination(x, y)) {
            crawl_ok = TRUE;
            goto crawl;
        }
    }
    /* one more scan */
    for (x = u.ux - 1; x <= u.ux + 1; x++)
        for (y = u.uy - 1; y <= u.uy + 1; y++)
            if (crawl_destination(x, y)) {
                crawl_ok = TRUE;
                goto crawl;
            }
crawl:
    if (crawl_ok) {
        boolean lost = FALSE;
        /* time to do some strip-tease... */
        boolean succ = Is_waterlevel(&u.uz) ? TRUE : emergency_disrobe(&lost);

/*JP
        You("try to crawl out of the water.");
*/
        You("水からはいあがろうとした．");
        if (lost)
/*JP
            You("dump some of your gear to lose weight...");
*/
            You("体を軽くするためいくつか物を投げすてた．．．");
        if (succ) {
/*JP
            pline("Pheew!  That was close.");
*/
            pline("ハァハァ！あぶなかった．");
            teleds(x, y, TRUE);
            return TRUE;
        }
        /* still too much weight */
/*JP
        pline("But in vain.");
*/
        pline("が，無駄だった．");
    }
    u.uinwater = 1;
/*JP
    You("drown.");
*/
    You("溺れた．");
    for (i = 0; i < 5; i++) { /* arbitrary number of loops */
        /* killer format and name are reconstructed every iteration
           because lifesaving resets them */
        pool_of_water = waterbody_name(u.ux, u.uy);
        killer.format = KILLED_BY_AN;
#if 0 /*JP*/
        /* avoid "drowned in [a] water" */
        if (!strcmp(pool_of_water, "water"))
            pool_of_water = "deep water", killer.format = KILLED_BY;
#endif
        Strcpy(killer.name, pool_of_water);
        done(DROWNING);
        /* oops, we're still alive.  better get out of the water. */
        if (safe_teleds(TRUE))
            break; /* successful life-save */
        /* nowhere safe to land; repeat drowning loop... */
/*JP
        pline("You're still drowning.");
*/
        You("まだ溺れている．");
    }
    if (u.uinwater) {
        u.uinwater = 0;
#if 0 /*JP*/
        You("find yourself back %s.",
            Is_waterlevel(&u.uz) ? "in an air bubble" : "on land");
#else
        You("いつのまにか%sにいるのに気がついた．",
            Is_waterlevel(&u.uz) ? "空気の泡の中" : "地面");
#endif
    }
    return TRUE;
}

void
drain_en(n)
int n;
{
    if (!u.uenmax) {
        /* energy is completely gone */
/*JP
        You_feel("momentarily lethargic.");
*/
        You("一瞬無気力感を感じた．");
    } else {
        /* throttle further loss a bit when there's not much left to lose */
        if (n > u.uenmax || n > u.ulevel)
            n = rnd(n);

/*JP
        You_feel("your magical energy drain away%c", (n > u.uen) ? '!' : '.');
*/
        You("魔法のエネルギーが吸いとられたような気がした%s", (n > u.uen) ? "！" : "．");
        u.uen -= n;
        if (u.uen < 0) {
            u.uenmax -= rnd(-u.uen);
            if (u.uenmax < 0)
                u.uenmax = 0;
            u.uen = 0;
        }
        context.botl = 1;
    }
}

/* disarm a trap */
int
dountrap()
{
    if (near_capacity() >= HVY_ENCUMBER) {
/*JP
        pline("You're too strained to do that.");
*/
        pline("罠を解除しようにも物を持ちすぎている．");
        return 0;
    }
    if ((nohands(youmonst.data) && !webmaker(youmonst.data))
        || !youmonst.data->mmove) {
/*JP
        pline("And just how do you expect to do that?");
*/
        pline("いったい何を期待しているんだい？");
        return 0;
    } else if (u.ustuck && sticks(youmonst.data)) {
/*JP
        pline("You'll have to let go of %s first.", mon_nam(u.ustuck));
*/
        pline("%sを手離さないことにはできない．", mon_nam(u.ustuck));
        return 0;
    }
    if (u.ustuck || (welded(uwep) && bimanual(uwep))) {
/*JP
        Your("%s seem to be too busy for that.", makeplural(body_part(HAND)));
*/
        Your("そんなことをする余裕なんてない．");
        return 0;
    }
    return untrap(FALSE);
}

/* Probability of disabling a trap.  Helge Hafting */
STATIC_OVL int
untrap_prob(ttmp)
struct trap *ttmp;
{
    int chance = 3;

    /* Only spiders know how to deal with webs reliably */
    if (ttmp->ttyp == WEB && !webmaker(youmonst.data))
        chance = 30;
    if (Confusion || Hallucination)
        chance++;
    if (Blind)
        chance++;
    if (Stunned)
        chance += 2;
    if (Fumbling)
        chance *= 2;
    /* Your own traps are better known than others. */
    if (ttmp && ttmp->madeby_u)
        chance--;
    if (Role_if(PM_ROGUE)) {
        if (rn2(2 * MAXULEV) < u.ulevel)
            chance--;
        if (u.uhave.questart && chance > 1)
            chance--;
    } else if (Role_if(PM_RANGER) && chance > 1)
        chance--;
    return rn2(chance);
}

/* Replace trap with object(s).  Helge Hafting */
void
cnv_trap_obj(otyp, cnt, ttmp, bury_it)
int otyp;
int cnt;
struct trap *ttmp;
boolean bury_it;
{
    struct obj *otmp = mksobj(otyp, TRUE, FALSE);

    otmp->quan = cnt;
    otmp->owt = weight(otmp);
    /* Only dart traps are capable of being poisonous */
    if (otyp != DART)
        otmp->opoisoned = 0;
    place_object(otmp, ttmp->tx, ttmp->ty);
    if (bury_it) {
        /* magical digging first disarms this trap, then will unearth it */
        (void) bury_an_obj(otmp, (boolean *) 0);
    } else {
        /* Sell your own traps only... */
        if (ttmp->madeby_u)
            sellobj(otmp, ttmp->tx, ttmp->ty);
        stackobj(otmp);
    }
    newsym(ttmp->tx, ttmp->ty);
    if (u.utrap && ttmp->tx == u.ux && ttmp->ty == u.uy)
        u.utrap = 0;
    deltrap(ttmp);
}

/* while attempting to disarm an adjacent trap, we've fallen into it */
STATIC_OVL void
move_into_trap(ttmp)
struct trap *ttmp;
{
    int bc = 0;
    xchar x = ttmp->tx, y = ttmp->ty, bx, by, cx, cy;
    boolean unused;

    bx = by = cx = cy = 0; /* lint suppression */
    /* we know there's no monster in the way, and we're not trapped */
    if (!Punished
        || drag_ball(x, y, &bc, &bx, &by, &cx, &cy, &unused, TRUE)) {
        u.ux0 = u.ux, u.uy0 = u.uy;
        u.ux = x, u.uy = y;
        u.umoved = TRUE;
        newsym(u.ux0, u.uy0);
        vision_recalc(1);
        check_leash(u.ux0, u.uy0);
        if (Punished)
            move_bc(0, bc, bx, by, cx, cy);
        /* marking the trap unseen forces dotrap() to treat it like a new
           discovery and prevents pickup() -> look_here() -> check_here()
           from giving a redundant "there is a <trap> here" message when
           there are objects covering this trap */
        ttmp->tseen = 0; /* hack for check_here() */
        /* trigger the trap */
        spoteffects(TRUE); /* pickup() + dotrap() */
        exercise(A_WIS, FALSE);
    }
}

/* 0: doesn't even try
 * 1: tries and fails
 * 2: succeeds
 */
STATIC_OVL int
try_disarm(ttmp, force_failure)
struct trap *ttmp;
boolean force_failure;
{
    struct monst *mtmp = m_at(ttmp->tx, ttmp->ty);
    int ttype = ttmp->ttyp;
    boolean under_u = (!u.dx && !u.dy);
    boolean holdingtrap = (ttype == BEAR_TRAP || ttype == WEB);

    /* Test for monster first, monsters are displayed instead of trap. */
    if (mtmp && (!mtmp->mtrapped || !holdingtrap)) {
/*JP
        pline("%s is in the way.", Monnam(mtmp));
*/
        pline("そこには%sがいる．", Monnam(mtmp));
        return 0;
    }
    /* We might be forced to move onto the trap's location. */
    if (sobj_at(BOULDER, ttmp->tx, ttmp->ty) && !Passes_walls && !under_u) {
/*JP
        There("is a boulder in your way.");
*/
        pline("そこには岩がある．");
        return 0;
    }
    /* duplicate tight-space checks from test_move */
    if (u.dx && u.dy && bad_rock(youmonst.data, u.ux, ttmp->ty)
        && bad_rock(youmonst.data, ttmp->tx, u.uy)) {
        if ((invent && (inv_weight() + weight_cap() > 600))
            || bigmonst(youmonst.data)) {
            /* don't allow untrap if they can't get thru to it */
#if 0 /*JP*/
            You("are unable to reach the %s!",
                defsyms[trap_to_defsym(ttype)].explanation);
#else
            You("%sに届かない！",
                defsyms[trap_to_defsym(ttype)].explanation);
#endif
            return 0;
        }
    }
    /* untrappable traps are located on the ground. */
    if (!can_reach_floor(TRUE)) {
        if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
            rider_cant_reach();
        else
#if 0 /*JP*/
            You("are unable to reach the %s!",
                defsyms[trap_to_defsym(ttype)].explanation);
#else
            You("%sに届かない！",
                defsyms[trap_to_defsym(ttype)].explanation);
#endif
        return 0;
    }

    /* Will our hero succeed? */
    if (force_failure || untrap_prob(ttmp)) {
        if (rnl(5)) {
/*JP
            pline("Whoops...");
*/
            pline("うわっ．．．");
            if (mtmp) { /* must be a trap that holds monsters */
                if (ttype == BEAR_TRAP) {
                    if (mtmp->mtame)
                        abuse_dog(mtmp);
                    if ((mtmp->mhp -= rnd(4)) <= 0)
                        killed(mtmp);
                } else if (ttype == WEB) {
                    if (!webmaker(youmonst.data)) {
                        struct trap *ttmp2 = maketrap(u.ux, u.uy, WEB);

                        if (ttmp2) {
                            pline_The(
/*JP
                                "webbing sticks to you. You're caught too!");
*/
                                "くもの巣があなたにからんできた．ますます捕まってしまった！");
                            dotrap(ttmp2, NOWEBMSG);
                            if (u.usteed && u.utrap) {
                                /* you, not steed, are trapped */
                                dismount_steed(DISMOUNT_FELL);
                            }
                        }
                    } else
/*JP
                        pline("%s remains entangled.", Monnam(mtmp));
*/
                        pline("%sはからまったままだ．", Monnam(mtmp));
                }
            } else if (under_u) {
                dotrap(ttmp, 0);
            } else {
                move_into_trap(ttmp);
            }
        } else {
#if 0 /*JP*/
            pline("%s %s is difficult to %s.",
                  ttmp->madeby_u ? "Your" : under_u ? "This" : "That",
                  defsyms[trap_to_defsym(ttype)].explanation,
                  (ttype == WEB) ? "remove" : "disarm");
#else
            pline("%s%sを解除するのは困難だ．",
                  ttmp->madeby_u ? "あなたの仕掛けた" : under_u ? "この" : "その",
                  defsyms[trap_to_defsym(ttype)].explanation);
#endif
        }
        return 1;
    }
    return 2;
}

STATIC_OVL void
reward_untrap(ttmp, mtmp)
struct trap *ttmp;
struct monst *mtmp;
{
    if (!ttmp->madeby_u) {
        if (rnl(10) < 8 && !mtmp->mpeaceful && !mtmp->msleeping
            && !mtmp->mfrozen && !mindless(mtmp->data)
            && mtmp->data->mlet != S_HUMAN) {
            mtmp->mpeaceful = 1;
            set_malign(mtmp); /* reset alignment */
/*JP
            pline("%s is grateful.", Monnam(mtmp));
*/
            pline("%sは喜んでいる．", Monnam(mtmp));
        }
        /* Helping someone out of a trap is a nice thing to do,
         * A lawful may be rewarded, but not too often.  */
        if (!rn2(3) && !rnl(8) && u.ualign.type == A_LAWFUL) {
            adjalign(1);
/*JP
            You_feel("that you did the right thing.");
*/
            You("正しいことをしたような気がした．");
        }
    }
}

STATIC_OVL int
disarm_holdingtrap(ttmp) /* Helge Hafting */
struct trap *ttmp;
{
    struct monst *mtmp;
    int fails = try_disarm(ttmp, FALSE);

    if (fails < 2)
        return fails;

    /* ok, disarm it. */

    /* untrap the monster, if any.
       There's no need for a cockatrice test, only the trap is touched */
    if ((mtmp = m_at(ttmp->tx, ttmp->ty)) != 0) {
        mtmp->mtrapped = 0;
#if 0 /*JP*/
        You("remove %s %s from %s.", the_your[ttmp->madeby_u],
            (ttmp->ttyp == BEAR_TRAP) ? "bear trap" : "webbing",
            mon_nam(mtmp));
#else
        You("%s%sを%sからはずした．", set_you[ttmp->madeby_u],
            (ttmp->ttyp == BEAR_TRAP) ? "熊の罠" : "くもの巣",
            mon_nam(mtmp));
#endif
        reward_untrap(ttmp, mtmp);
    } else {
        if (ttmp->ttyp == BEAR_TRAP) {
/*JP
            You("disarm %s bear trap.", the_your[ttmp->madeby_u]);
*/
            You("%s熊の罠を解除した．", set_you[ttmp->madeby_u]);
            cnv_trap_obj(BEARTRAP, 1, ttmp, FALSE);
        } else /* if (ttmp->ttyp == WEB) */ {
/*JP
            You("succeed in removing %s web.", the_your[ttmp->madeby_u]);
*/
            You("%sくもの巣を取り除いた．", set_you[ttmp->madeby_u]);
            deltrap(ttmp);
        }
    }
    newsym(u.ux + u.dx, u.uy + u.dy);
    return 1;
}

STATIC_OVL int
disarm_landmine(ttmp) /* Helge Hafting */
struct trap *ttmp;
{
    int fails = try_disarm(ttmp, FALSE);

    if (fails < 2)
        return fails;
/*JP
    You("disarm %s land mine.", the_your[ttmp->madeby_u]);
*/
    You("%s地雷を解除した．", set_you[ttmp->madeby_u]);
    cnv_trap_obj(LAND_MINE, 1, ttmp, FALSE);
    return 1;
}

/* getobj will filter down to cans of grease and known potions of oil */
static NEARDATA const char oil[] = { ALL_CLASSES, TOOL_CLASS, POTION_CLASS,
                                     0 };

/* it may not make much sense to use grease on floor boards, but so what? */
STATIC_OVL int
disarm_squeaky_board(ttmp)
struct trap *ttmp;
{
    struct obj *obj;
    boolean bad_tool;
    int fails;

    obj = getobj(oil, "untrap with");
    if (!obj)
        return 0;

    bad_tool = (obj->cursed
                || ((obj->otyp != POT_OIL || obj->lamplit)
                    && (obj->otyp != CAN_OF_GREASE || !obj->spe)));
    fails = try_disarm(ttmp, bad_tool);
    if (fails < 2)
        return fails;

    /* successfully used oil or grease to fix squeaky board */
    if (obj->otyp == CAN_OF_GREASE) {
        consume_obj_charge(obj, TRUE);
    } else {
        useup(obj); /* oil */
        makeknown(POT_OIL);
    }
#if 0 /*JP*/
    You("repair the squeaky board."); /* no madeby_u */
#else
    You("きしむ板を修理した．"); /* no madeby_u */
#endif
    deltrap(ttmp);
    newsym(u.ux + u.dx, u.uy + u.dy);
    more_experienced(1, 5);
    newexplevel();
    return 1;
}

/* removes traps that shoot arrows, darts, etc. */
STATIC_OVL int
disarm_shooting_trap(ttmp, otyp)
struct trap *ttmp;
int otyp;
{
    int fails = try_disarm(ttmp, FALSE);

    if (fails < 2)
        return fails;
/*JP
    You("disarm %s trap.", the_your[ttmp->madeby_u]);
*/
    pline("%s罠を解除した．", set_you[ttmp->madeby_u]);
    cnv_trap_obj(otyp, 50 - rnl(50), ttmp, FALSE);
    return 1;
}

/* Is the weight too heavy?
 * Formula as in near_capacity() & check_capacity() */
STATIC_OVL int
try_lift(mtmp, ttmp, wt, stuff)
struct monst *mtmp;
struct trap *ttmp;
int wt;
boolean stuff;
{
    int wc = weight_cap();

    if (((wt * 2) / wc) >= HVY_ENCUMBER) {
#if 0 /*JP*/
        pline("%s is %s for you to lift.", Monnam(mtmp),
              stuff ? "carrying too much" : "too heavy");
#else
        pline("%sは%s持ちあげることができない．", Monnam(mtmp),
              stuff ? "物を持ちすぎており" : "重すぎて");
#endif
        if (!ttmp->madeby_u && !mtmp->mpeaceful && mtmp->mcanmove
            && !mindless(mtmp->data) && mtmp->data->mlet != S_HUMAN
            && rnl(10) < 3) {
            mtmp->mpeaceful = 1;
            set_malign(mtmp); /* reset alignment */
/*JP
            pline("%s thinks it was nice of you to try.", Monnam(mtmp));
*/
            pline("%sはあなたの努力に感謝しているようだ．", Monnam(mtmp));
        }
        return 0;
    }
    return 1;
}

/* Help trapped monster (out of a (spiked) pit) */
STATIC_OVL int
help_monster_out(mtmp, ttmp)
struct monst *mtmp;
struct trap *ttmp;
{
    int wt;
    struct obj *otmp;
    boolean uprob;

    /*
     * This works when levitating too -- consistent with the ability
     * to hit monsters while levitating.
     *
     * Should perhaps check that our hero has arms/hands at the
     * moment.  Helping can also be done by engulfing...
     *
     * Test the monster first - monsters are displayed before traps.
     */
    if (!mtmp->mtrapped) {
/*JP
        pline("%s isn't trapped.", Monnam(mtmp));
*/
        pline("%sは罠にかかっていない．", Monnam(mtmp));
        return 0;
    }
    /* Do you have the necessary capacity to lift anything? */
    if (check_capacity((char *) 0))
        return 1;

    /* Will our hero succeed? */
    if ((uprob = untrap_prob(ttmp)) && !mtmp->msleeping && mtmp->mcanmove) {
#if 0 /*JP*/
        You("try to reach out your %s, but %s backs away skeptically.",
            makeplural(body_part(ARM)), mon_nam(mtmp));
#else
        You("%sを差し延べようとしたが%sは警戒している．",
            body_part(ARM), mon_nam(mtmp));
#endif
        return 1;
    }

    /* is it a cockatrice?... */
    if (touch_petrifies(mtmp->data) && !uarmg && !Stone_resistance) {
#if 0 /*JP*/
        You("grab the trapped %s using your bare %s.", mtmp->data->mname,
            makeplural(body_part(HAND)));
#else
        You("罠にかかっている%sを素%sで掴んだ．", mtmp->data->mname,
            body_part(HAND));
#endif

        if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM)) {
            display_nhwindow(WIN_MESSAGE, FALSE);
        } else {
            char kbuf[BUFSZ];

#if 0 /*JP*/
            Sprintf(kbuf, "trying to help %s out of a pit",
                    an(mtmp->data->mname));
#else
            Sprintf(kbuf, "罠にかかっている%sを助けようとして",
                    a_monnam(mtmp));
#endif
            instapetrify(kbuf);
            return 1;
        }
    }
    /* need to do cockatrice check first if sleeping or paralyzed */
    if (uprob) {
/*JP
        You("try to grab %s, but cannot get a firm grasp.", mon_nam(mtmp));
*/
        You("%sをつかもうとしたが，しっかりと握れなかった．", mon_nam(mtmp));
        if (mtmp->msleeping) {
            mtmp->msleeping = 0;
/*JP
            pline("%s awakens.", Monnam(mtmp));
*/
            pline("%sは目を覚ました．", Monnam(mtmp));
        }
        return 1;
    }

#if 0 /*JP*/
    You("reach out your %s and grab %s.", makeplural(body_part(ARM)),
        mon_nam(mtmp));
#else
    You("%sを伸ばして%sをつかんだ．", body_part(ARM),
        mon_nam(mtmp));
#endif

    if (mtmp->msleeping) {
        mtmp->msleeping = 0;
/*JP
        pline("%s awakens.", Monnam(mtmp));
*/
        pline("%sは目を覚ました．", Monnam(mtmp));
    } else if (mtmp->mfrozen && !rn2(mtmp->mfrozen)) {
        /* After such manhandling, perhaps the effect wears off */
        mtmp->mcanmove = 1;
        mtmp->mfrozen = 0;
/*JP
        pline("%s stirs.", Monnam(mtmp));
*/
        pline("%sは動き始めた．", Monnam(mtmp));
    }

    /* is the monster too heavy? */
    wt = inv_weight() + mtmp->data->cwt;
    if (!try_lift(mtmp, ttmp, wt, FALSE))
        return 1;

    /* is the monster with inventory too heavy? */
    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
        wt += otmp->owt;
    if (!try_lift(mtmp, ttmp, wt, TRUE))
        return 1;

/*JP
    You("pull %s out of the pit.", mon_nam(mtmp));
*/
    You("%sを落し穴からひっぱった．", mon_nam(mtmp));
    mtmp->mtrapped = 0;
    fill_pit(mtmp->mx, mtmp->my);
    reward_untrap(ttmp, mtmp);
    return 1;
}

int
untrap(force)
boolean force;
{
    register struct obj *otmp;
    register int x, y;
    int ch;
    struct trap *ttmp;
    struct monst *mtmp;
    const char *trapdescr;
    boolean here, useplural, confused = (Confusion || Hallucination),
                             trap_skipped = FALSE, deal_with_floor_trap;
    int boxcnt = 0;
    char the_trap[BUFSZ], qbuf[QBUFSZ];

    if (!getdir((char *) 0))
        return 0;
    x = u.ux + u.dx;
    y = u.uy + u.dy;
    if (!isok(x, y)) {
/*JP
        pline_The("perils lurking there are beyond your grasp.");
*/
        pline_The("そこにある危険はあなたの手に負えない．");
        return 0;
    }
    ttmp = t_at(x, y);
    if (ttmp && !ttmp->tseen)
        ttmp = 0;
    trapdescr = ttmp ? defsyms[trap_to_defsym(ttmp->ttyp)].explanation : 0;
    here = (x == u.ux && y == u.uy); /* !u.dx && !u.dy */

    if (here) /* are there are one or more containers here? */
        for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
            if (Is_box(otmp)) {
                if (++boxcnt > 1)
                    break;
            }

    deal_with_floor_trap = can_reach_floor(FALSE);
    if (!deal_with_floor_trap) {
        *the_trap = '\0';
        if (ttmp)
            Strcat(the_trap, an(trapdescr));
        if (ttmp && boxcnt)
/*JP
            Strcat(the_trap, " and ");
*/
            Strcat(the_trap, "と");
        if (boxcnt)
/*JP
            Strcat(the_trap, (boxcnt == 1) ? "a container" : "containers");
*/
            Strcat(the_trap, "容器");
        useplural = ((ttmp && boxcnt > 0) || boxcnt > 1);
        /* note: boxcnt and useplural will always be 0 for !here case */
        if (ttmp || boxcnt)
#if 0 /*JP*/
            There("%s %s %s but you can't reach %s%s.",
                  useplural ? "are" : "is", the_trap, here ? "here" : "there",
                  useplural ? "them" : "it",
                  u.usteed ? " while mounted" : "");
#else
            pline("%sがあるが，%s届かない．",
                  the_trap,
                  u.usteed ? "乗っていると" : "");
#endif
        trap_skipped = (ttmp != 0);
    } else { /* deal_with_floor_trap */

        if (ttmp) {
            Strcpy(the_trap, the(trapdescr));
            if (boxcnt) {
                if (ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT) {
#if 0 /*JP*/
                    You_cant("do much about %s%s.", the_trap,
                             u.utrap ? " that you're stuck in"
                                     : " while standing on the edge of it");
#else
                    pline("%s%sに対してはたいしたことはできない．",
                          u.utrap ? "自分がはまっている"
                                  : "すぐそばの",
                          the_trap);
#endif
                    trap_skipped = TRUE;
                    deal_with_floor_trap = FALSE;
                } else {
#if 0 /*JP*/
                    Sprintf(
                        qbuf, "There %s and %s here. %s %s?",
                        (boxcnt == 1) ? "is a container" : "are containers",
                        an(trapdescr),
                        (ttmp->ttyp == WEB) ? "Remove" : "Disarm", the_trap);
#else
                    Sprintf(
                        qbuf, "箱と%sがある．%s？",
                        trapdescr,
                        (ttmp->ttyp == WEB) ? "取り除く" : "解除する");
#endif
                    switch (ynq(qbuf)) {
                    case 'q':
                        return 0;
                    case 'n':
                        trap_skipped = TRUE;
                        deal_with_floor_trap = FALSE;
                        break;
                    }
                }
            }
            if (deal_with_floor_trap) {
                if (u.utrap) {
#if 0 /*JP*/
                    You("cannot deal with %s while trapped%s!", the_trap,
                        (x == u.ux && y == u.uy) ? " in it" : "");
#else
                    pline("罠にかかっている間は罠を解除できない！");
#endif
                    return 1;
                }
                if ((mtmp = m_at(x, y)) != 0
                    && (mtmp->m_ap_type == M_AP_FURNITURE
                        || mtmp->m_ap_type == M_AP_OBJECT)) {
                    stumble_onto_mimic(mtmp);
                    return 1;
                }
                switch (ttmp->ttyp) {
                case BEAR_TRAP:
                case WEB:
                    return disarm_holdingtrap(ttmp);
                case LANDMINE:
                    return disarm_landmine(ttmp);
                case SQKY_BOARD:
                    return disarm_squeaky_board(ttmp);
                case DART_TRAP:
                    return disarm_shooting_trap(ttmp, DART);
                case ARROW_TRAP:
                    return disarm_shooting_trap(ttmp, ARROW);
                case PIT:
                case SPIKED_PIT:
                    if (here) {
/*JP
                        You("are already on the edge of the pit.");
*/
                        You("もう落し穴の端にいる．");
                        return 0;
                    }
                    if (!mtmp) {
/*JP
                        pline("Try filling the pit instead.");
*/
                        pline("なんとか埋めることを考えてみたら？");
                        return 0;
                    }
                    return help_monster_out(mtmp, ttmp);
                default:
/*JP
                    You("cannot disable %s trap.", !here ? "that" : "this");
*/
                    pline("%s罠は解除できない．", !here ? "その" : "この");
                    return 0;
                }
            }
        } /* end if */

        if (boxcnt) {
            for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
                if (Is_box(otmp)) {
#if 0 /*JP*/
                    (void) safe_qbuf(qbuf, "There is ",
                                     " here.  Check it for traps?", otmp,
                                     doname, ansimpleoname, "a box");
#else
                    (void) safe_qbuf(qbuf, "",
                                     "がある．罠を調べますか？", otmp,
                                     doname, ansimpleoname, "箱");
#endif
                    switch (ynq(qbuf)) {
                    case 'q':
                        return 0;
                    case 'n':
                        continue;
                    }

                    if ((otmp->otrapped
                         && (force || (!confused
                                       && rn2(MAXULEV + 1 - u.ulevel) < 10)))
                        || (!force && confused && !rn2(3))) {
/*JP
                        You("find a trap on %s!", the(xname(otmp)));
*/
                        pline("%sに罠を発見した！", the(xname(otmp)));
                        if (!confused)
                            exercise(A_WIS, TRUE);

/*JP
                        switch (ynq("Disarm it?")) {
*/
                        switch (ynq("解除しますか？")) {
                        case 'q':
                            return 1;
                        case 'n':
                            trap_skipped = TRUE;
                            continue;
                        }

                        if (otmp->otrapped) {
                            exercise(A_DEX, TRUE);
                            ch = ACURR(A_DEX) + u.ulevel;
                            if (Role_if(PM_ROGUE))
                                ch *= 2;
                            if (!force && (confused || Fumbling
                                           || rnd(75 + level_difficulty() / 2)
                                                  > ch)) {
                                (void) chest_trap(otmp, FINGER, TRUE);
                            } else {
/*JP
                                You("disarm it!");
*/
                                You("解除した！");
                                otmp->otrapped = 0;
                            }
                        } else
/*JP
                            pline("That %s was not trapped.", xname(otmp));
*/
                            pline("その%sに罠はない．", xname(otmp));
                        return 1;
                    } else {
/*JP
                        You("find no traps on %s.", the(xname(otmp)));
*/
                        pline("罠を発見できなかった．");
                        return 1;
                    }
                }

#if 0 /*JP*/
            You(trap_skipped ? "find no other traps here."
                             : "know of no traps here.");
#else
            You(trap_skipped ? "他の罠を見つけられなかった．"
                             : "ここに罠がないことを知っている．");
#endif
            return 0;
        }

        if (stumble_on_door_mimic(x, y))
            return 1;

    } /* deal_with_floor_trap */
    /* doors can be manipulated even while levitating/unskilled riding */

    if (!IS_DOOR(levl[x][y].typ)) {
        if (!trap_skipped)
/*JP
            You("know of no traps there.");
*/
            You("そこに罠がないことを知っている．");
        return 0;
    }

    switch (levl[x][y].doormask) {
    case D_NODOOR:
/*JP
        You("%s no door there.", Blind ? "feel" : "see");
*/
        pline("そこには扉がない%s．", Blind ? "ようだ" : "ように見える");
        return 0;
    case D_ISOPEN:
/*JP
        pline("This door is safely open.");
*/
        pline("その扉は安全に開いている．");
        return 0;
    case D_BROKEN:
/*JP
        pline("This door is broken.");
*/
        pline("その扉は壊れている．");
        return 0;
    }

    if ((levl[x][y].doormask & D_TRAPPED
         && (force || (!confused && rn2(MAXULEV - u.ulevel + 11) < 10)))
        || (!force && confused && !rn2(3))) {
/*JP
        You("find a trap on the door!");
*/
        pline("扉に罠を発見した！");
        exercise(A_WIS, TRUE);
/*JP
        if (ynq("Disarm it?") != 'y')
*/
        if (ynq("解除しますか？") != 'y')
            return 1;
        if (levl[x][y].doormask & D_TRAPPED) {
            ch = 15 + (Role_if(PM_ROGUE) ? u.ulevel * 3 : u.ulevel);
            exercise(A_DEX, TRUE);
            if (!force && (confused || Fumbling
                           || rnd(75 + level_difficulty() / 2) > ch)) {
/*JP
                You("set it off!");
*/
                You("スイッチを入れてしまった！");
/*JP
                b_trapped("door", FINGER);
*/
                b_trapped("扉", FINGER);
                levl[x][y].doormask = D_NODOOR;
                unblock_point(x, y);
                newsym(x, y);
                /* (probably ought to charge for this damage...) */
                if (*in_rooms(x, y, SHOPBASE))
                    add_damage(x, y, 0L);
            } else {
/*JP
                You("disarm it!");
*/
                You("解除した！");
                levl[x][y].doormask &= ~D_TRAPPED;
            }
        } else
/*JP
            pline("This door was not trapped.");
*/
            pline("扉に罠はなかった．");
        return 1;
    } else {
/*JP
        You("find no traps on the door.");
*/
        pline("扉に罠を発見できなかった．");
        return 1;
    }
}

/* for magic unlocking; returns true if targetted monster (which might
   be hero) gets untrapped; the trap remains intact */
boolean
openholdingtrap(mon, noticed)
struct monst *mon;
boolean *noticed; /* set to true iff hero notices the effect; */
{                 /* otherwise left with its previous value intact */
    struct trap *t;
    char buf[BUFSZ];
    const char *trapdescr, *which;
    boolean ishero = (mon == &youmonst);

    if (mon == u.usteed)
        ishero = TRUE;
    t = t_at(ishero ? u.ux : mon->mx, ishero ? u.uy : mon->my);
    /* if no trap here or it's not a holding trap, we're done */
    if (!t || (t->ttyp != BEAR_TRAP && t->ttyp != WEB))
        return FALSE;

    trapdescr = defsyms[trap_to_defsym(t->ttyp)].explanation;
#if 0 /*JP*/
    which = t->tseen ? the_your[t->madeby_u]
                     : index(vowels, *trapdescr) ? "an" : "a";
#else
    which = t->tseen ? set_you[t->madeby_u] : "";
#endif

    if (ishero) {
        if (!u.utrap)
            return FALSE;
        u.utrap = 0; /* released regardless of type */
        *noticed = TRUE;
        /* give message only if trap was the expected type */
        if (u.utraptype == TT_BEARTRAP || u.utraptype == TT_WEB) {
            if (u.usteed)
/*JP
                Sprintf(buf, "%s is", noit_Monnam(u.usteed));
*/
                Strcpy(buf, noit_Monnam(u.usteed));
            else
/*JP
                Strcpy(buf, "You are");
*/
                Strcpy(buf, "あなた");
/*JP
            pline("%s released from %s %s.", buf, which, trapdescr);
*/
            pline("%sは%s%sから解放された．", buf, which, trapdescr);
        }
    } else {
        if (!mon->mtrapped)
            return FALSE;
        mon->mtrapped = 0;
        if (canspotmon(mon)) {
            *noticed = TRUE;
#if 0 /*JP*/
            pline("%s is released from %s %s.", Monnam(mon), which,
                  trapdescr);
#else
            pline("%sは%s%sから解放された．", Monnam(mon), which,
                  trapdescr);
#endif
        } else if (cansee(t->tx, t->ty) && t->tseen) {
            *noticed = TRUE;
            if (t->ttyp == WEB)
#if 0 /*JP*/
                pline("%s is released from %s %s.", Something, which,
                      trapdescr);
#else
                pline("何者かは%s%sから解放された．", which,
                      trapdescr);
#endif
            else /* BEAR_TRAP */
/*JP
                pline("%s %s opens.", upstart(strcpy(buf, which)), trapdescr);
*/
                pline("%s%sは開いた．", which, trapdescr);
        }
        /* might pacify monster if adjacent */
        if (rn2(2) && distu(mon->mx, mon->my) <= 2)
            reward_untrap(t, mon);
    }
    return TRUE;
}

/* for magic locking; returns true if targetted monster (which might
   be hero) gets hit by a trap (might avoid actually becoming trapped) */
boolean
closeholdingtrap(mon, noticed)
struct monst *mon;
boolean *noticed; /* set to true iff hero notices the effect; */
{                 /* otherwise left with its previous value intact */
    struct trap *t;
    unsigned dotrapflags;
    boolean ishero = (mon == &youmonst), result;

    if (mon == u.usteed)
        ishero = TRUE;
    t = t_at(ishero ? u.ux : mon->mx, ishero ? u.uy : mon->my);
    /* if no trap here or it's not a holding trap, we're done */
    if (!t || (t->ttyp != BEAR_TRAP && t->ttyp != WEB))
        return FALSE;

    if (ishero) {
        if (u.utrap)
            return FALSE; /* already trapped */
        *noticed = TRUE;
        dotrapflags = FORCETRAP;
        /* dotrap calls mintrap when mounted hero encounters a web */
        if (u.usteed)
            dotrapflags |= NOWEBMSG;
        ++force_mintrap;
        dotrap(t, dotrapflags);
        --force_mintrap;
        result = (u.utrap != 0);
    } else {
        if (mon->mtrapped)
            return FALSE; /* already trapped */
        /* you notice it if you see the trap close/tremble/whatever
           or if you sense the monster who becomes trapped */
        *noticed = cansee(t->tx, t->ty) || canspotmon(mon);
        ++force_mintrap;
        result = (mintrap(mon) != 0);
        --force_mintrap;
    }
    return result;
}

/* for magic unlocking; returns true if targetted monster (which might
   be hero) gets hit by a trap (target might avoid its effect) */
boolean
openfallingtrap(mon, trapdoor_only, noticed)
struct monst *mon;
boolean trapdoor_only;
boolean *noticed; /* set to true iff hero notices the effect; */
{                 /* otherwise left with its previous value intact */
    struct trap *t;
    boolean ishero = (mon == &youmonst), result;

    if (mon == u.usteed)
        ishero = TRUE;
    t = t_at(ishero ? u.ux : mon->mx, ishero ? u.uy : mon->my);
    /* if no trap here or it's not a falling trap, we're done
       (note: falling rock traps have a trapdoor in the ceiling) */
    if (!t || ((t->ttyp != TRAPDOOR && t->ttyp != ROCKTRAP)
               && (trapdoor_only || (t->ttyp != HOLE && t->ttyp != PIT
                                     && t->ttyp != SPIKED_PIT))))
        return FALSE;

    if (ishero) {
        if (u.utrap)
            return FALSE; /* already trapped */
        *noticed = TRUE;
        dotrap(t, FORCETRAP);
        result = (u.utrap != 0);
    } else {
        if (mon->mtrapped)
            return FALSE; /* already trapped */
        /* you notice it if you see the trap close/tremble/whatever
           or if you sense the monster who becomes trapped */
        *noticed = cansee(t->tx, t->ty) || canspotmon(mon);
        /* monster will be angered; mintrap doesn't handle that */
        wakeup(mon);
        ++force_mintrap;
        result = (mintrap(mon) != 0);
        --force_mintrap;
        /* mon might now be on the migrating monsters list */
    }
    return TRUE;
}

/* only called when the player is doing something to the chest directly */
boolean
chest_trap(obj, bodypart, disarm)
register struct obj *obj;
register int bodypart;
boolean disarm;
{
    register struct obj *otmp = obj, *otmp2;
    char buf[80];
    const char *msg;
    coord cc;

    if (get_obj_location(obj, &cc.x, &cc.y, 0)) /* might be carried */
        obj->ox = cc.x, obj->oy = cc.y;

    otmp->otrapped = 0; /* trap is one-shot; clear flag first in case
                           chest kills you and ends up in bones file */
/*JP
    You(disarm ? "set it off!" : "trigger a trap!");
*/
    You(disarm ? "スイッチを入れてしまった！" : "罠にひっかかった！");
    display_nhwindow(WIN_MESSAGE, FALSE);
    if (Luck > -13 && rn2(13 + Luck) > 7) { /* saved by luck */
        /* trap went off, but good luck prevents damage */
        switch (rn2(13)) {
        case 12:
        case 11:
/*JP
            msg = "explosive charge is a dud";
*/
            msg = "爆発は不発だった";
            break;
        case 10:
        case 9:
/*JP
            msg = "electric charge is grounded";
*/
            msg = "電撃が放出されたがアースされていた";
            break;
        case 8:
        case 7:
/*JP
            msg = "flame fizzles out";
*/
            msg = "炎はシューっと消えた";
            break;
        case 6:
        case 5:
        case 4:
/*JP
            msg = "poisoned needle misses";
*/
            msg = "毒針は刺さらなかった";
            break;
        case 3:
        case 2:
        case 1:
        case 0:
/*JP
            msg = "gas cloud blows away";
*/
            msg = "ガス雲は吹き飛んだ";
            break;
        default:
            impossible("chest disarm bug");
            msg = (char *) 0;
            break;
        }
        if (msg)
/*JP
            pline("But luckily the %s!", msg);
*/
            pline("しかし運のよいことに%s！", msg);
    } else {
        switch (rn2(20) ? ((Luck >= 13) ? 0 : rn2(13 - Luck)) : rn2(26)) {
        case 25:
        case 24:
        case 23:
        case 22:
        case 21: {
            struct monst *shkp = 0;
            long loss = 0L;
            boolean costly, insider;
            register xchar ox = obj->ox, oy = obj->oy;

            /* the obj location need not be that of player */
            costly = (costly_spot(ox, oy)
                      && (shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE)))
                             != (struct monst *) 0);
            insider = (*u.ushops && inside_shop(u.ux, u.uy)
                       && *in_rooms(ox, oy, SHOPBASE) == *u.ushops);

/*JP
            pline("%s!", Tobjnam(obj, "explode"));
*/
            pline("%sは爆発した！", xname(obj));
/*JP
            Sprintf(buf, "exploding %s", xname(obj));
*/
            Sprintf(buf, "%sの爆発で", xname(obj));

            if (costly)
                loss += stolen_value(obj, ox, oy, (boolean) shkp->mpeaceful,
                                     TRUE);
            delete_contents(obj);
            /* we're about to delete all things at this location,
             * which could include the ball & chain.
             * If we attempt to call unpunish() in the
             * for-loop below we can end up with otmp2
             * being invalid once the chain is gone.
             * Deal with ball & chain right now instead.
             */
            if (Punished && !carried(uball)
                && ((uchain->ox == u.ux && uchain->oy == u.uy)
                    || (uball->ox == u.ux && uball->oy == u.uy)))
                unpunish();

            for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
                otmp2 = otmp->nexthere;
                if (costly)
                    loss += stolen_value(otmp, otmp->ox, otmp->oy,
                                         (boolean) shkp->mpeaceful, TRUE);
                delobj(otmp);
            }
            wake_nearby();
            losehp(Maybe_Half_Phys(d(6, 6)), buf, KILLED_BY_AN);
            exercise(A_STR, FALSE);
            if (costly && loss) {
                if (insider)
#if 0 /*JP*/
                    You("owe %ld %s for objects destroyed.", loss,
                        currency(loss));
#else
                    You("器物破損で%ld%sの借りをつくった．", loss,
                        currency(loss));
#endif
                else {
#if 0 /*JP*/
                    You("caused %ld %s worth of damage!", loss,
                        currency(loss));
#else
                    You("%ld%s分の損害を引きおこした！", loss,
                        currency(loss));
#endif
                    make_angry_shk(shkp, ox, oy);
                }
            }
            return TRUE;
        } /* case 21 */
        case 20:
        case 19:
        case 18:
        case 17:
/*JP
            pline("A cloud of noxious gas billows from %s.", the(xname(obj)));
*/
            pline("有毒ガスが%sから渦まいた．", the(xname(obj)));
/*JP
            poisoned("gas cloud", A_STR, "cloud of poison gas", 15, FALSE);
*/
            poisoned("ガス雲", A_STR, "ガス雲", 15, FALSE);
            exercise(A_CON, FALSE);
            break;
        case 16:
        case 15:
        case 14:
        case 13:
/*JP
            You_feel("a needle prick your %s.", body_part(bodypart));
*/
            You("%sチクッという痛みを感じた．", body_part(bodypart));
/*JP
            poisoned("needle", A_CON, "poisoned needle", 10, FALSE);
*/
            poisoned("針", A_CON, "毒針", 10, FALSE);
            exercise(A_CON, FALSE);
            break;
        case 12:
        case 11:
        case 10:
        case 9:
            dofiretrap(obj);
            break;
        case 8:
        case 7:
        case 6: {
            int dmg;

/*JP
            You("are jolted by a surge of electricity!");
*/
            You("電気ショックをくらった！");
            if (Shock_resistance) {
                shieldeff(u.ux, u.uy);
/*JP
                You("don't seem to be affected.");
*/
                pline("しかしあなたは影響を受けない．");
                dmg = 0;
            } else
                dmg = d(4, 4);
            destroy_item(RING_CLASS, AD_ELEC);
            destroy_item(WAND_CLASS, AD_ELEC);
            if (dmg)
/*JP
                losehp(dmg, "electric shock", KILLED_BY_AN);
*/
                losehp(dmg, "電気ショックで", KILLED_BY_AN);
            break;
        } /* case 6 */
        case 5:
        case 4:
        case 3:
            if (!Free_action) {
/*JP
                pline("Suddenly you are frozen in place!");
*/
                pline("突然その場で動けなくなった！");
                nomul(-d(5, 6));
/*JP
                multi_reason = "frozen by a trap";
*/
                multi_reason = "罠で硬直している時に";
                exercise(A_DEX, FALSE);
                nomovemsg = You_can_move_again;
            } else
/*JP
                You("momentarily stiffen.");
*/
                You("一瞬硬直した．");
            break;
        case 2:
        case 1:
        case 0:
#if 0 /*JP*/
            pline("A cloud of %s gas billows from %s.",
                  Blind ? blindgas[rn2(SIZE(blindgas))] : rndcolor(),
                  the(xname(obj)));
#else
            pline("%sガス雲が%sの底で渦まいた．",
                  Blind ? blindgas[rn2(SIZE(blindgas))] :
                  jconj_adj(rndcolor()), xname(obj));
#endif
            if (!Stunned) {
                if (Hallucination)
/*JP
                    pline("What a groovy feeling!");
*/
                    pline("なんて素敵なんだ！");
                else
#if 0 /*JP*/
                    You("%s%s...", stagger(youmonst.data, "stagger"),
                        Halluc_resistance ? ""
                                          : Blind ? " and get dizzy"
                                                  : " and your vision blurs");
#else
                    You("くらくらし%sた...",
                        Halluc_resistance ? ""
                                          : Blind ? "，めまいがし"
                                                  : "，景色がぼやけてき");
#endif
            }
            make_stunned((HStun & TIMEOUT) + (long) rn1(7, 16), FALSE);
            (void) make_hallucinated(
                (HHallucination & TIMEOUT) + (long) rn1(5, 16), FALSE, 0L);
            break;
        default:
            impossible("bad chest trap");
            break;
        }
        bot(); /* to get immediate botl re-display */
    }

    return FALSE;
}

struct trap *
t_at(x, y)
register int x, y;
{
    register struct trap *trap = ftrap;

    while (trap) {
        if (trap->tx == x && trap->ty == y)
            return trap;
        trap = trap->ntrap;
    }
    return (struct trap *) 0;
}

void
deltrap(trap)
register struct trap *trap;
{
    register struct trap *ttmp;

    clear_conjoined_pits(trap);
    if (trap == ftrap) {
        ftrap = ftrap->ntrap;
    } else {
        for (ttmp = ftrap; ttmp; ttmp = ttmp->ntrap)
            if (ttmp->ntrap == trap)
                break;
        if (!ttmp)
            panic("deltrap: no preceding trap!");
        ttmp->ntrap = trap->ntrap;
    }
    if (Sokoban && (trap->ttyp == PIT || trap->ttyp == HOLE))
        maybe_finish_sokoban();
    dealloc_trap(trap);
}

boolean
conjoined_pits(trap2, trap1, u_entering_trap2)
struct trap *trap2, *trap1;
boolean u_entering_trap2;
{
    int dx, dy, diridx, adjidx;

    if (!trap1 || !trap2)
        return FALSE;
    if (!isok(trap2->tx, trap2->ty) || !isok(trap1->tx, trap1->ty)
        || !(trap2->ttyp == PIT || trap2->ttyp == SPIKED_PIT)
        || !(trap1->ttyp == PIT || trap1->ttyp == SPIKED_PIT)
        || (u_entering_trap2 && !(u.utrap && u.utraptype == TT_PIT)))
        return FALSE;
    dx = sgn(trap2->tx - trap1->tx);
    dy = sgn(trap2->ty - trap1->ty);
    for (diridx = 0; diridx < 8; diridx++)
        if (xdir[diridx] == dx && ydir[diridx] == dy)
            break;
    /* diridx is valid if < 8 */
    if (diridx < 8) {
        adjidx = (diridx + 4) % 8;
        if ((trap1->conjoined & (1 << diridx))
            && (trap2->conjoined & (1 << adjidx)))
            return TRUE;
    }
    return FALSE;
}

void
clear_conjoined_pits(trap)
struct trap *trap;
{
    int diridx, adjidx, x, y;
    struct trap *t;

    if (trap && (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT)) {
        for (diridx = 0; diridx < 8; ++diridx) {
            if (trap->conjoined & (1 << diridx)) {
                x = trap->tx + xdir[diridx];
                y = trap->ty + ydir[diridx];
                if (isok(x, y)
                    && (t = t_at(x, y)) != 0
                    && (t->ttyp == PIT || t->ttyp == SPIKED_PIT)) {
                    adjidx = (diridx + 4) % 8;
                    t->conjoined &= ~(1 << adjidx);
                }
                trap->conjoined &= ~(1 << diridx);
            }
        }
    }
}

#if 0
/*
 * Mark all neighboring pits as conjoined pits.
 * (currently not called from anywhere)
 */
STATIC_OVL void
join_adjacent_pits(trap)
struct trap *trap;
{
    struct trap *t;
    int diridx, x, y;

    if (!trap)
        return;
    for (diridx = 0; diridx < 8; ++diridx) {
        x = trap->tx + xdir[diridx];
        y = trap->ty + ydir[diridx];
        if (isok(x, y)) {
            if ((t = t_at(x, y)) != 0
                && (t->ttyp == PIT || t->ttyp == SPIKED_PIT)) {
                trap->conjoined |= (1 << diridx);
                join_adjacent_pits(t);
            } else
                trap->conjoined &= ~(1 << diridx);
        }
    }
}
#endif /*0*/

/*
 * Returns TRUE if you escaped a pit and are standing on the precipice.
 */
boolean
uteetering_at_seen_pit(trap)
struct trap *trap;
{
    if (trap && trap->tseen && (!u.utrap || u.utraptype != TT_PIT)
        && (trap->ttyp == PIT || trap->ttyp == SPIKED_PIT))
        return TRUE;
    else
        return FALSE;
}

/* Destroy a trap that emanates from the floor. */
boolean
delfloortrap(ttmp)
register struct trap *ttmp;
{
    /* some of these are arbitrary -dlc */
    if (ttmp && ((ttmp->ttyp == SQKY_BOARD) || (ttmp->ttyp == BEAR_TRAP)
                 || (ttmp->ttyp == LANDMINE) || (ttmp->ttyp == FIRE_TRAP)
                 || (ttmp->ttyp == PIT) || (ttmp->ttyp == SPIKED_PIT)
                 || (ttmp->ttyp == HOLE) || (ttmp->ttyp == TRAPDOOR)
                 || (ttmp->ttyp == TELEP_TRAP) || (ttmp->ttyp == LEVEL_TELEP)
                 || (ttmp->ttyp == WEB) || (ttmp->ttyp == MAGIC_TRAP)
                 || (ttmp->ttyp == ANTI_MAGIC))) {
        register struct monst *mtmp;

        if (ttmp->tx == u.ux && ttmp->ty == u.uy) {
            u.utrap = 0;
            u.utraptype = 0;
        } else if ((mtmp = m_at(ttmp->tx, ttmp->ty)) != 0) {
            mtmp->mtrapped = 0;
        }
        deltrap(ttmp);
        return TRUE;
    }
    return FALSE;
}

/* used for doors (also tins).  can be used for anything else that opens. */
void
b_trapped(item, bodypart)
const char *item;
int bodypart;
{
    int lvl = level_difficulty(),
        dmg = rnd(5 + (lvl < 5 ? lvl : 2 + lvl / 2));

/*JP
    pline("KABOOM!!  %s was booby-trapped!", The(item));
*/
    pline("ちゅどーん！！%sにブービートラップが仕掛けられていた！", item);
    wake_nearby();
/*JP
    losehp(Maybe_Half_Phys(dmg), "explosion", KILLED_BY_AN);
*/
    losehp(Maybe_Half_Phys(dmg), "ブービートラップの爆発で", KILLED_BY_AN);
    exercise(A_STR, FALSE);
    if (bodypart)
        exercise(A_CON, FALSE);
    make_stunned((HStun & TIMEOUT) + (long) dmg, TRUE);
}

/* Monster is hit by trap. */
/* Note: doesn't work if both obj and d_override are null */
STATIC_OVL boolean
thitm(tlev, mon, obj, d_override, nocorpse)
int tlev;
struct monst *mon;
struct obj *obj;
int d_override;
boolean nocorpse;
{
    int strike;
    boolean trapkilled = FALSE;

    if (d_override)
        strike = 1;
    else if (obj)
        strike = (find_mac(mon) + tlev + obj->spe <= rnd(20));
    else
        strike = (find_mac(mon) + tlev <= rnd(20));

    /* Actually more accurate than thitu, which doesn't take
     * obj->spe into account.
     */
    if (!strike) {
        if (obj && cansee(mon->mx, mon->my))
/*JP
            pline("%s is almost hit by %s!", Monnam(mon), doname(obj));
*/
            pline("もう少しで%sが%sに命中するところだった！", doname(obj), Monnam(mon));
    } else {
        int dam = 1;

        if (obj && cansee(mon->mx, mon->my))
/*JP
            pline("%s is hit by %s!", Monnam(mon), doname(obj));
*/
            pline("%sが%sに命中した！", doname(obj), Monnam(mon));
        if (d_override)
            dam = d_override;
        else if (obj) {
            dam = dmgval(obj, mon);
            if (dam < 1)
                dam = 1;
        }
        if ((mon->mhp -= dam) <= 0) {
            int xx = mon->mx;
            int yy = mon->my;

            monkilled(mon, "", nocorpse ? -AD_RBRE : AD_PHYS);
            if (mon->mhp <= 0) {
                newsym(xx, yy);
                trapkilled = TRUE;
            }
        }
    }
    if (obj && (!strike || d_override)) {
        place_object(obj, mon->mx, mon->my);
        stackobj(obj);
    } else if (obj)
        dealloc_obj(obj);

    return trapkilled;
}

boolean
unconscious()
{
    if (multi >= 0)
        return FALSE;

#if 0 /*JP*/
    return (boolean) (u.usleep
                      || (nomovemsg
                          && (!strncmp(nomovemsg, "You awake", 9)
                              || !strncmp(nomovemsg, "You regain con", 14)
                              || !strncmp(nomovemsg, "You are consci", 14))));
#else
/*JP 3.6.0 での出現位置/回数
 "You awake"      : potion.c(1)
  "You regain con": eat.c(1)
  "You are consci": eat.c(1)
*/
    return (boolean) (u.usleep
                      || (nomovemsg
                          && (!strncmp(nomovemsg, "目がさめたが頭痛", 18)
                              || !strncmp(nomovemsg, "あなたは正気づいた", 18)
                              || !strncmp(nomovemsg, "あなたはまた正気づ", 18))));
#endif
}

/*JP
static const char lava_killer[] = "molten lava";
*/
static const char lava_killer[] = "どろどろの溶岩で";

boolean
lava_effects()
{
    register struct obj *obj, *obj2;
    int dmg = d(6, 6); /* only applicable for water walking */
    boolean usurvive, boil_away;

    burn_away_slime();
    if (likes_lava(youmonst.data))
        return FALSE;

    usurvive = Fire_resistance || (Wwalking && dmg < u.uhp);
    /*
     * A timely interrupt might manage to salvage your life
     * but not your gear.  For scrolls and potions this
     * will destroy whole stacks, where fire resistant hero
     * survivor only loses partial stacks via destroy_item().
     *
     * Flag items to be destroyed before any messages so
     * that player causing hangup at --More-- won't get an
     * emergency save file created before item destruction.
     */
    if (!usurvive)
        for (obj = invent; obj; obj = obj->nobj)
            if ((is_organic(obj) || obj->oclass == POTION_CLASS)
                && !obj->oerodeproof
                && objects[obj->otyp].oc_oprop != FIRE_RES
                && obj->otyp != SCR_FIRE && obj->otyp != SPE_FIREBALL
                && !obj_resists(obj, 0, 0)) /* for invocation items */
                obj->in_use = TRUE;

    /* Check whether we should burn away boots *first* so we know whether to
     * make the player sink into the lava. Assumption: water walking only
     * comes from boots.
     */
    if (Wwalking && uarmf && is_organic(uarmf) && !uarmf->oerodeproof) {
        obj = uarmf;
/*JP
        pline("%s into flame!", Yobjnam2(obj, "burst"));
*/
        Your("%sは燃えた！", xname(obj));
        iflags.in_lava_effects++; /* (see above) */
        (void) Boots_off();
        useup(obj);
        iflags.in_lava_effects--;
    }

    if (!Fire_resistance) {
        if (Wwalking) {
/*JP
            pline_The("lava here burns you!");
*/
            pline("溶岩があなたを焼きつくした！");
            if (usurvive) {
                losehp(dmg, lava_killer, KILLED_BY); /* lava damage */
                goto burn_stuff;
            }
        } else
/*JP
            You("fall into the lava!");
*/
            You("溶岩に落ちた！");

        usurvive = Lifesaved || discover;
        if (wizard)
            usurvive = TRUE;

        /* prevent remove_worn_item() -> Boots_off(WATER_WALKING_BOOTS) ->
           spoteffects() -> lava_effects() recursion which would
           successfully delete (via useupall) the no-longer-worn boots;
           once recursive call returned, we would try to delete them again
           here in the outer call (and access stale memory, probably panic) */
        iflags.in_lava_effects++;

        for (obj = invent; obj; obj = obj2) {
            obj2 = obj->nobj;
            /* above, we set in_use for objects which are to be destroyed */
            if (obj->otyp == SPE_BOOK_OF_THE_DEAD && !Blind) {
                if (usurvive)
#if 0 /*JP*/
                    pline("%s glows a strange %s, but remains intact.",
                          The(xname(obj)), hcolor("dark red"));
#else
                    pline("%sは不思議に%s輝いたが，無傷のようだ．",
                          xname(obj), jconj_adj(hcolor("暗赤色の")));
#endif
            } else if (obj->in_use) {
                if (obj->owornmask) {
                    if (usurvive)
/*JP
                        pline("%s into flame!", Yobjnam2(obj, "burst"));
*/
                        Your("%sは燃えた！", xname(obj));
                    remove_worn_item(obj, TRUE);
                }
                useupall(obj);
            }
        }

        iflags.in_lava_effects--;

        /* s/he died... */
        boil_away = (u.umonnum == PM_WATER_ELEMENTAL
                     || u.umonnum == PM_STEAM_VORTEX
                     || u.umonnum == PM_FOG_CLOUD);
        for (;;) {
            u.uhp = -1;
            /* killer format and name are reconstructed every iteration
               because lifesaving resets them */
            killer.format = KILLED_BY;
            Strcpy(killer.name, lava_killer);
/*JP
            You("%s...", boil_away ? "boil away" : "burn to a crisp");
*/
            You("%s．．．", boil_away ? "沸騰した" : "燃えてパリパリになった");
            done(BURNING);
            if (safe_teleds(TRUE))
                break; /* successful life-save */
            /* nowhere safe to land; repeat burning loop */
/*JP
            pline("You're still burning.");
*/
            You("まだ燃えている．");
        }
/*JP
        You("find yourself back on solid %s.", surface(u.ux, u.uy));
*/
        You("いつのまにか固い%sに戻っていた．", surface(u.ux, u.uy));
        return TRUE;
    } else if (!Wwalking && (!u.utrap || u.utraptype != TT_LAVA)) {
        boil_away = !Fire_resistance;
        /* if not fire resistant, sink_into_lava() will quickly be fatal;
           hero needs to escape immediately */
        u.utrap = rn1(4, 4) + ((boil_away ? 2 : rn1(4, 12)) << 8);
        u.utraptype = TT_LAVA;
#if 0 /*JP*/
        You("sink into the lava%s!", !boil_away
                                         ? ", but it only burns slightly"
                                         : " and are about to be immolated");
#else
        You("溶岩に沈ん%sだ！", !boil_away
                                         ? "だが，ちょっと焦げただけ"
                                         : "で焼き殺されそう");
#endif
        if (u.uhp > 1)
            losehp(!boil_away ? 1 : (u.uhp / 2), lava_killer,
                   KILLED_BY); /* lava damage */
    }

burn_stuff:
    destroy_item(SCROLL_CLASS, AD_FIRE);
    destroy_item(SPBOOK_CLASS, AD_FIRE);
    destroy_item(POTION_CLASS, AD_FIRE);
    return FALSE;
}

/* called each turn when trapped in lava */
void
sink_into_lava()
{
/*JP
    static const char sink_deeper[] = "You sink deeper into the lava.";
*/
    static const char sink_deeper[] = "あなたはより深く溶岩に沈んだ．";

    if (!u.utrap || u.utraptype != TT_LAVA) {
        ; /* do nothing; this shouldn't happen */
    } else if (!is_lava(u.ux, u.uy)) {
        u.utrap = 0; /* this shouldn't happen either */
    } else if (!u.uinvulnerable) {
        /* ordinarily we'd have to be fire resistant to survive long
           enough to become stuck in lava, but it can happen without
           resistance if water walking boots allow survival and then
           get burned up; u.utrap time will be quite short in that case */
        if (!Fire_resistance)
            u.uhp = (u.uhp + 2) / 3;

        u.utrap -= (1 << 8);
        if (u.utrap < (1 << 8)) {
            killer.format = KILLED_BY;
/*JP
            Strcpy(killer.name, "molten lava");
*/
            Strcpy(killer.name, "どろどろの溶岩で");
/*JP
            You("sink below the surface and die.");
*/
            You("完全に溶岩の中に沈んで、死にました．．．");
            burn_away_slime(); /* add insult to injury? */
            done(DISSOLVED);
        } else if (!u.umoved) {
            /* can't fully turn into slime while in lava, but might not
               have it be burned away until you've come awfully close */
            if (Slimed && rnd(10 - 1) >= (int) (Slimed & TIMEOUT)) {
                pline(sink_deeper);
                burn_away_slime();
            } else {
                Norep(sink_deeper);
            }
            u.utrap += rnd(4);
        }
    }
}

/* called when something has been done (breaking a boulder, for instance)
   which entails a luck penalty if performed on a sokoban level */
void
sokoban_guilt()
{
    if (Sokoban) {
        change_luck(-1);
        /* TODO: issue some feedback so that player can learn that whatever
           he/she just did is a naughty thing to do in sokoban and should
           probably be avoided in future....
           Caveat: doing this might introduce message sequencing issues,
           depending upon feedback during the various actions which trigger
           Sokoban luck penalties. */
    }
}

/* called when a trap has been deleted or had its ttyp replaced */
STATIC_OVL void
maybe_finish_sokoban()
{
    struct trap *t;

    if (Sokoban && !in_mklev) {
        /* scan all remaining traps, ignoring any created by the hero;
           if this level has no more pits or holes, the current sokoban
           puzzle has been solved */
        for (t = ftrap; t; t = t->ntrap) {
            if (t->madeby_u)
                continue;
            if (t->ttyp == PIT || t->ttyp == HOLE)
                break;
        }
        if (!t) {
            /* we've passed the last trap without finding a pit or hole;
               clear the sokoban_rules flag so that luck penalties for
               things like breaking boulders or jumping will no longer
               be given, and restrictions on diagonal moves are lifted */
            Sokoban = 0; /* clear level.flags.sokoban_rules */
            /* TODO: give some feedback about solving the sokoban puzzle
               (perhaps say "congratulations" in Japanese?) */
        }
    }
}

/*trap.c*/
