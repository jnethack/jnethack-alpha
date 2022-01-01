/* NetHack 3.6	music.c	$NHDT-Date: 1573063606 2019/11/06 18:06:46 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.60 $ */
/*      Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2022            */
/* JNetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the different functions designed to manipulate the
 * musical instruments and their various effects.
 *
 * The list of instruments / effects is :
 *
 * (wooden) flute       may calm snakes if player has enough dexterity
 * magic flute          may put monsters to sleep:  area of effect depends
 *                      on player level.
 * (tooled) horn        Will awaken monsters:  area of effect depends on
 *                      player level.  May also scare monsters.
 * fire horn            Acts like a wand of fire.
 * frost horn           Acts like a wand of cold.
 * bugle                Will awaken soldiers (if any):  area of effect depends
 *                      on player level.
 * (wooden) harp        May calm nymph if player has enough dexterity.
 * magic harp           Charm monsters:  area of effect depends on player
 *                      level.
 * (leather) drum       Will awaken monsters like the horn.
 * drum of earthquake   Will initiate an earthquake whose intensity depends
 *                      on player level.  That is, it creates random pits
 *                      called here chasms.
 */

#include "hack.h"

STATIC_DCL void FDECL(awaken_monsters, (int));
STATIC_DCL void FDECL(put_monsters_to_sleep, (int));
STATIC_DCL void FDECL(charm_snakes, (int));
STATIC_DCL void FDECL(calm_nymphs, (int));
STATIC_DCL void FDECL(charm_monsters, (int));
STATIC_DCL void FDECL(do_earthquake, (int));
STATIC_DCL int FDECL(do_improvisation, (struct obj *));

#ifdef UNIX386MUSIC
STATIC_DCL int NDECL(atconsole);
STATIC_DCL void FDECL(speaker, (struct obj *, char *));
#endif
#ifdef VPIX_MUSIC
extern int sco_flag_console; /* will need changing if not _M_UNIX */
STATIC_DCL void NDECL(playinit);
STATIC_DCL void FDECL(playstring, (char *, size_t));
STATIC_DCL void FDECL(speaker, (struct obj *, char *));
#endif
#ifdef PCMUSIC
void FDECL(pc_speaker, (struct obj *, char *));
#endif
#ifdef AMIGA
void FDECL(amii_speaker, (struct obj *, char *, int));
#endif

/*
 * Wake every monster in range...
 */

STATIC_OVL void
awaken_monsters(distance)
int distance;
{
    register struct monst *mtmp;
    register int distm;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if ((distm = distu(mtmp->mx, mtmp->my)) < distance) {
            mtmp->msleeping = 0;
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
            /* may scare some monsters -- waiting monsters excluded */
            if (!unique_corpstat(mtmp->data)
                && (mtmp->mstrategy & STRAT_WAITMASK) != 0)
                mtmp->mstrategy &= ~STRAT_WAITMASK;
            else if (distm < distance / 3
                     && !resist(mtmp, TOOL_CLASS, 0, NOTELL)
                     /* some monsters are immune */
                     && onscary(0, 0, mtmp))
                monflee(mtmp, 0, FALSE, TRUE);
        }
    }
}

/*
 * Make monsters fall asleep.  Note that they may resist the spell.
 */

STATIC_OVL void
put_monsters_to_sleep(distance)
int distance;
{
    register struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (distu(mtmp->mx, mtmp->my) < distance
            && sleep_monst(mtmp, d(10, 10), TOOL_CLASS)) {
            mtmp->msleeping = 1; /* 10d10 turns + wake_nearby to rouse */
            slept_monst(mtmp);
        }
    }
}

/*
 * Charm snakes in range.  Note that the snakes are NOT tamed.
 */

STATIC_OVL void
charm_snakes(distance)
int distance;
{
    register struct monst *mtmp;
    int could_see_mon, was_peaceful;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->data->mlet == S_SNAKE && mtmp->mcanmove
            && distu(mtmp->mx, mtmp->my) < distance) {
            was_peaceful = mtmp->mpeaceful;
            mtmp->mpeaceful = 1;
            mtmp->mavenge = 0;
            mtmp->mstrategy &= ~STRAT_WAITMASK;
            could_see_mon = canseemon(mtmp);
            mtmp->mundetected = 0;
            newsym(mtmp->mx, mtmp->my);
            if (canseemon(mtmp)) {
                if (!could_see_mon)
/*JP
                    You("notice %s, swaying with the music.", a_monnam(mtmp));
*/
                    You("%sが音楽に合わせて揺れているのに気付いた．", a_monnam(mtmp));
                else
#if 0 /*JP:T*/
                    pline("%s freezes, then sways with the music%s.",
                          Monnam(mtmp),
                          was_peaceful ? "" : ", and now seems quieter");
#else
                    pline("%sは立ちすくみ，音楽に合わせて揺れ%sた．",
                          Monnam(mtmp),
                          was_peaceful ? "" : "，おとなしくなっ");
#endif
            }
        }
    }
}

/*
 * Calm nymphs in range.
 */

STATIC_OVL void
calm_nymphs(distance)
int distance;
{
    register struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (mtmp->data->mlet == S_NYMPH && mtmp->mcanmove
            && distu(mtmp->mx, mtmp->my) < distance) {
            mtmp->msleeping = 0;
            mtmp->mpeaceful = 1;
            mtmp->mavenge = 0;
            mtmp->mstrategy &= ~STRAT_WAITMASK;
            if (canseemon(mtmp))
                pline(
/*JP
                    "%s listens cheerfully to the music, then seems quieter.",
*/
                    "%sは音楽に聞きいり，おとなしくなった．",
                      Monnam(mtmp));
        }
    }
}

/* Awake soldiers anywhere the level (and any nearby monster). */
void
awaken_soldiers(bugler)
struct monst *bugler; /* monster that played instrument */
{
    register struct monst *mtmp;
    int distance, distm;

    /* distance of affected non-soldier monsters to bugler */
    distance = ((bugler == &youmonst) ? u.ulevel : bugler->data->mlevel) * 30;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (is_mercenary(mtmp->data) && mtmp->data != &mons[PM_GUARD]) {
            mtmp->mpeaceful = mtmp->msleeping = mtmp->mfrozen = 0;
            mtmp->mcanmove = 1;
            mtmp->mstrategy &= ~STRAT_WAITMASK;
            if (canseemon(mtmp))
/*JP
                pline("%s is now ready for battle!", Monnam(mtmp));
*/
                pline("%sは戦いの準備が整った！", Monnam(mtmp));
            else if (!Deaf)
#if 0 /*JP*/
                Norep("%s the rattle of battle gear being readied.",
                      "You hear");  /* Deaf-aware */
#else
                /*JP:TODO:Deaf対応*/
                Norep("あなたは戦いの準備が整ったことを示す音を聞いた．");
#endif
        } else if ((distm = ((bugler == &youmonst)
                                 ? distu(mtmp->mx, mtmp->my)
                                 : dist2(bugler->mx, bugler->my, mtmp->mx,
                                         mtmp->my))) < distance) {
            mtmp->msleeping = 0;
            mtmp->mcanmove = 1;
            mtmp->mfrozen = 0;
            /* may scare some monsters -- waiting monsters excluded */
            if (!unique_corpstat(mtmp->data)
                && (mtmp->mstrategy & STRAT_WAITMASK) != 0)
                mtmp->mstrategy &= ~STRAT_WAITMASK;
            else if (distm < distance / 3
                     && !resist(mtmp, TOOL_CLASS, 0, NOTELL))
                monflee(mtmp, 0, FALSE, TRUE);
        }
    }
}

/* Charm monsters in range.  Note that they may resist the spell.
 * If swallowed, range is reduced to 0.
 */
STATIC_OVL void
charm_monsters(distance)
int distance;
{
    struct monst *mtmp, *mtmp2;

    if (u.uswallow) {
        if (!resist(u.ustuck, TOOL_CLASS, 0, NOTELL))
            (void) tamedog(u.ustuck, (struct obj *) 0);
    } else {
        for (mtmp = fmon; mtmp; mtmp = mtmp2) {
            mtmp2 = mtmp->nmon;
            if (DEADMONSTER(mtmp))
                continue;

            if (distu(mtmp->mx, mtmp->my) <= distance) {
                if (!resist(mtmp, TOOL_CLASS, 0, NOTELL))
                    (void) tamedog(mtmp, (struct obj *) 0);
            }
        }
    }
}

/* Generate earthquake :-) of desired force.
 * That is:  create random chasms (pits).
 */
STATIC_OVL void
do_earthquake(force)
int force;
{
    register int x, y;
    struct monst *mtmp;
    struct obj *otmp;
    struct trap *chasm, *trap_at_u = t_at(u.ux, u.uy);
    int start_x, start_y, end_x, end_y;
    schar filltype;
    unsigned tu_pit = 0;

    if (trap_at_u)
        tu_pit = is_pit(trap_at_u->ttyp);
    start_x = u.ux - (force * 2);
    start_y = u.uy - (force * 2);
    end_x = u.ux + (force * 2);
    end_y = u.uy + (force * 2);
    start_x = max(start_x, 1);
    start_y = max(start_y, 0);
    end_x = min(end_x, COLNO - 1);
    end_y = min(end_y, ROWNO - 1);
    for (x = start_x; x <= end_x; x++)
        for (y = start_y; y <= end_y; y++) {
            if ((mtmp = m_at(x, y)) != 0) {
                wakeup(mtmp, TRUE); /* peaceful monster will become hostile */
                if (mtmp->mundetected && is_hider(mtmp->data)) {
                    mtmp->mundetected = 0;
                    if (cansee(x, y))
/*JP
                        pline("%s is shaken loose from the ceiling!",
*/
                        pline("%sは揺すられ，天井から落ちてきた！",
                              Amonnam(mtmp));
                    else
/*JP
                        You_hear("a thumping sound.");
*/
                        You_hear("ドンドンという音を聞いた．");
                    if (x == u.ux && y == u.uy)
/*JP
                        You("easily dodge the falling %s.", mon_nam(mtmp));
*/
                        You("落ちてきた%sを簡単にかわした．", mon_nam(mtmp));
                    newsym(x, y);
                }
            }
            if (!rn2(14 - force))
                switch (levl[x][y].typ) {
                case FOUNTAIN: /* Make the fountain disappear */
                    if (cansee(x, y))
/*JP
                        pline_The("fountain falls into a chasm.");
*/
                        pline("泉は地割れに落ちた．");
                    goto do_pit;
                case SINK:
                    if (cansee(x, y))
/*JP
                        pline_The("kitchen sink falls into a chasm.");
*/
                        pline("流し台は地割れに落ちた．");
                    goto do_pit;
                case ALTAR:
                    if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz))
                        break;

                    if (cansee(x, y))
/*JP
                        pline_The("altar falls into a chasm.");
*/
                        pline("祭壇は地割れに落ちた．");
                    goto do_pit;
                case GRAVE:
                    if (cansee(x, y))
/*JP
                        pline_The("headstone topples into a chasm.");
*/
                        pline("墓石は崩れさった．");
                    goto do_pit;
                case THRONE:
                    if (cansee(x, y))
/*JP
                        pline_The("throne falls into a chasm.");
*/
                        pline("玉座は地割れに落ちた．");
                    /*FALLTHRU*/
                case ROOM:
                case CORR: /* Try to make a pit */
 do_pit:
                    chasm = maketrap(x, y, PIT);
                    if (!chasm)
                        break; /* no pit if portal at that location */
                    chasm->tseen = 1;

                    /* TODO:
                     * This ought to be split into a separate routine to
                     * reduce indentation and the consequent line-wraps.
                     */

                    levl[x][y].doormask = 0;
                    /*
                     * Let liquid flow into the newly created chasm.
                     * Adjust corresponding code in apply.c for
                     * exploding wand of digging if you alter this sequence.
                     */
                    filltype = fillholetyp(x, y, FALSE);
                    if (filltype != ROOM) {
                        levl[x][y].typ = filltype; /* flags set via doormask */
                        liquid_flow(x, y, filltype, chasm, (char *) 0);
                    }

                    mtmp = m_at(x, y);

                    if ((otmp = sobj_at(BOULDER, x, y)) != 0) {
                        if (cansee(x, y))
#if 0 /*JP:T*/
                            pline("KADOOM!  The boulder falls into a chasm%s!",
                                  (x == u.ux && y == u.uy) ? " below you"
                                                           : "");
#else
                            pline("ドドーン！岩は%s地割れに落ちた！",
                                  (x == u.ux && y == u.uy) ? "あなたの下の"
                                                           : "");
#endif
                        if (mtmp)
                            mtmp->mtrapped = 0;
                        obj_extract_self(otmp);
                        (void) flooreffects(otmp, x, y, "");
                        break;
                    }

                    /* We have to check whether monsters or player
                       falls in a chasm... */
                    if (mtmp) {
                        if (!is_flyer(mtmp->data)
                            && !is_clinger(mtmp->data)) {
                            boolean m_already_trapped = mtmp->mtrapped;

                            mtmp->mtrapped = 1;
                            if (!m_already_trapped) { /* suppress messages */
                                if (cansee(x, y))
/*JP
                                    pline("%s falls into a chasm!",
*/
                                    pline("%sは地割れに落ちた！",
                                          Monnam(mtmp));
                                else if (humanoid(mtmp->data))
/*JP
                                    You_hear("a scream!");
*/
                                    You_hear("叫び声を聞いた！");
                            }
                            /* Falling is okay for falling down
                                within a pit from jostling too */
/*JP
                            mselftouch(mtmp, "Falling, ", TRUE);
*/
                            mselftouch(mtmp, "落下中，", TRUE);
                            if (!DEADMONSTER(mtmp)) {
                                mtmp->mhp -= rnd(m_already_trapped ? 4 : 6);
                                if (DEADMONSTER(mtmp)) {
                                    if (!cansee(x, y)) {
/*JP
                                        pline("It is destroyed!");
*/
                                        pline("何者かは死んだ！");
                                    } else {
#if 0 /*JP*/
                                        You("destroy %s!",
                                            mtmp->mtame
                                              ? x_monnam(mtmp, ARTICLE_THE,
                                                         "poor",
                                                         has_mname(mtmp)
                                                           ? SUPPRESS_SADDLE
                                                           : 0,
                                                         FALSE)
                                              : mon_nam(mtmp));
#else
                                        pline("%sは死んだ！",
                                            mtmp->mtame
                                              ? x_monnam(mtmp, ARTICLE_THE,
                                                         "かわいそうな",
                                                         has_mname(mtmp)
                                                           ? SUPPRESS_SADDLE
                                                           : 0,
                                                         FALSE)
                                              : mon_nam(mtmp));
#endif
                                    }
                                    xkilled(mtmp, XKILL_NOMSG);
                                }
                            }
                        }
                    } else if (x == u.ux && y == u.uy) {
                        if (u.utrap && u.utraptype == TT_BURIEDBALL) {
                            /* Note:  the chain should break if a pit gets
                               created at the buried ball's location, which
                               is not necessarily here.  But if we don't do
                               things this way, entering the new pit below
                               will override current trap anyway, but too
                               late to get Lev and Fly handling. */
/*JP
                            Your("chain breaks!");
*/
                            Your("鎖は壊れた！");
                            reset_utrap(TRUE);
                        }
                        if (Levitation || Flying
                            || is_clinger(youmonst.data)) {
                            if (!tu_pit) { /* no pit here previously */
/*JP
                                pline("A chasm opens up under you!");
*/
                                pline("地割れが足元に開いた！");
/*JP
                                You("don't fall in!");
*/
                                You("落ちなかった！");
                            }
                        } else if (!tu_pit || !u.utrap
                                   || (u.utrap && u.utraptype != TT_PIT)) {
                            /* no pit here previously, or you were
                               not in it even if there was */
/*JP
                            You("fall into a chasm!");
*/
                            You("地割れに落ちた！");
                            set_utrap(rn1(6, 2), TT_PIT);
#if 0 /*JP:T*/
                            losehp(Maybe_Half_Phys(rnd(6)),
                                   "fell into a chasm", NO_KILLER_PREFIX);
#else
                            losehp(Maybe_Half_Phys(rnd(6)),
                                   "地割れに落ちて", KILLED_BY);
#endif
/*JP
                            selftouch("Falling, you");
*/
                            selftouch("落ちながら，あなたは");
                        } else if (u.utrap && u.utraptype == TT_PIT) {
                            boolean keepfooting =
                                ((Fumbling && !rn2(5))
                                 || (!rnl(Role_if(PM_ARCHEOLOGIST) ? 3 : 9))
                                 || ((ACURR(A_DEX) > 7) && rn2(5)));

/*JP
                            You("are jostled around violently!");
*/
                            You("乱暴に押しのけられた！");
                            set_utrap(rn1(6, 2), TT_PIT);
#if 0 /*JP:T*/
                            losehp(Maybe_Half_Phys(rnd(keepfooting ? 2 : 4)),
                                   "hurt in a chasm", NO_KILLER_PREFIX);
#else
                            losehp(Maybe_Half_Phys(rnd(keepfooting ? 2 : 4)),
                                   "地割れで傷ついて", KILLED_BY);
#endif
                            if (keepfooting)
                                exercise(A_DEX, TRUE);
                            else
#if 0 /*JP:T*/
                                selftouch(
                                    (Upolyd && (slithy(youmonst.data)
                                                || nolimbs(youmonst.data)))
                                        ? "Shaken, you"
                                        : "Falling down, you");
#else
                                selftouch(
                                    (Upolyd && (slithy(youmonst.data)
                                                || nolimbs(youmonst.data)))
                                        ? "揺さぶられて，あなたは"
                                        : "落ちながら，あなたは");
#endif
                        }
                    } else
                        newsym(x, y);
                    break;
                case DOOR: /* Make the door collapse */
                    if (levl[x][y].doormask == D_NODOOR)
                        goto do_pit;
                    if (cansee(x, y))
/*JP
                        pline_The("door collapses.");
*/
                        pline_The("扉はこなごなになった．");
                    if (*in_rooms(x, y, SHOPBASE))
                        add_damage(x, y, 0L);
                    levl[x][y].doormask = D_NODOOR;
                    unblock_point(x, y);
                    newsym(x, y);
                    break;
                }
        }
}

const char *
generic_lvl_desc()
{
    if (Is_astralevel(&u.uz))
/*JP
        return "astral plane";
*/
        return "天上界";
    else if (In_endgame(&u.uz))
/*JP
        return "plane";
*/
        return "精霊界";
    else if (Is_sanctum(&u.uz))
/*JP
        return "sanctum";
*/
        return "聖域";
    else if (In_sokoban(&u.uz))
/*JP
        return "puzzle";
*/
        return "倉庫";
    else if (In_V_tower(&u.uz))
/*JP
        return "tower";
*/
        return "塔";
    else
/*JP
        return "dungeon";
*/
        return "迷宮";
}

const char *beats[] = {
    "stepper", "one drop", "slow two", "triple stroke roll",
    "double shuffle", "half-time shuffle", "second line", "train"
};

/*
 * The player is trying to extract something from his/her instrument.
 */
STATIC_OVL int
do_improvisation(instr)
struct obj *instr;
{
    int damage, mode, do_spec = !(Stunned || Confusion);
    struct obj itmp;
    boolean mundane = FALSE;

    itmp = *instr;
    itmp.oextra = (struct oextra *) 0; /* ok on this copy as instr maintains
                                          the ptr to free at some point if
                                          there is one */

    /* if won't yield special effect, make sound of mundane counterpart */
    if (!do_spec || instr->spe <= 0)
        while (objects[itmp.otyp].oc_magic) {
            itmp.otyp -= 1;
            mundane = TRUE;
        }

#ifdef MAC
    mac_speaker(&itmp, "C");
#endif
#ifdef AMIGA
    amii_speaker(&itmp, "Cw", AMII_OKAY_VOLUME);
#endif
#ifdef VPIX_MUSIC
    if (sco_flag_console)
        speaker(&itmp, "C");
#endif
#ifdef PCMUSIC
    pc_speaker(&itmp, "C");
#endif

#define PLAY_NORMAL   0x00
#define PLAY_STUNNED  0x01
#define PLAY_CONFUSED 0x02
#define PLAY_HALLU    0x04
    mode = PLAY_NORMAL;
    if (Stunned)
        mode |= PLAY_STUNNED;
    if (Confusion)
        mode |= PLAY_CONFUSED;
    if (Hallucination)
        mode |= PLAY_HALLU;

    if (!rn2(2)) {
        /*
         * TEMPORARY?  for multiple impairments, don't always
         * give the generic "it's far from music" message.
         */
        /* remove if STUNNED+CONFUSED ever gets its own message below */
        if (mode == (PLAY_STUNNED | PLAY_CONFUSED))
            mode = !rn2(2) ? PLAY_STUNNED : PLAY_CONFUSED;
        /* likewise for stunned and/or confused combined with hallucination */
        if (mode & PLAY_HALLU)
            mode = PLAY_HALLU;
    }

    /* 3.6.3: most of these gave "You produce <blah>" and then many of
       the instrument-specific messages below which immediately follow
       also gave "You produce <something>."  That looked strange so we
       now use a different verb here */
    switch (mode) {
    case PLAY_NORMAL:
/*JP
        You("start playing %s.", yname(instr));
*/
        You("%sを奏ではじめた．", yname(instr));
        break;
    case PLAY_STUNNED:
        if (!Deaf)
/*JP
            You("radiate an obnoxious droning sound.");
*/
            You("不愉快で単調な音を発した．");
        else
/*JP
            You_feel("a monotonous vibration.");
*/
            You_feel("単調な振動を感じた．");
        break;
    case PLAY_CONFUSED:
        if (!Deaf)
/*JP
            You("generate a raucous noise.");
*/
            You("耳障りな音を出した．");
        else
/*JP
            You_feel("a jarring vibration.");
*/
            You_feel("耳障りな振動を感じた．");
        break;
    case PLAY_HALLU:
/*JP
        You("disseminate a kaleidoscopic display of floating butterflies.");
*/
        You("空に浮かぶ蝶の万華鏡的な表現を創出した．");
        break;
    /* TODO? give some or all of these combinations their own feedback;
       hallucination ones should reference senses other than hearing... */
    case PLAY_STUNNED | PLAY_CONFUSED:
    case PLAY_STUNNED | PLAY_HALLU:
    case PLAY_CONFUSED | PLAY_HALLU:
    case PLAY_STUNNED | PLAY_CONFUSED | PLAY_HALLU:
    default:
/*JP
        pline("What you perform is quite far from music...");
*/
        pline("あなたが奏でたものは音楽とはとても呼べない．．．");
        break;
    }
#undef PLAY_NORMAL
#undef PLAY_STUNNED
#undef PLAY_CONFUSED
#undef PLAY_HALLU

    switch (itmp.otyp) { /* note: itmp.otyp might differ from instr->otyp */
    case MAGIC_FLUTE: /* Make monster fall asleep */
        consume_obj_charge(instr, TRUE);

#if 0 /*JP*/
        You("%sproduce %s music.", !Deaf ? "" : "seem to ",
            Hallucination ? "piped" : "soft");
#else
        You("%sを奏でた%s．",
            Hallucination ? "ＢＧＭ" : "軟らかい曲",
            !Deaf ? "" : "ようだ");
#endif
        put_monsters_to_sleep(u.ulevel * 5);
        exercise(A_DEX, TRUE);
        break;
    case WOODEN_FLUTE: /* May charm snakes */
        do_spec &= (rn2(ACURR(A_DEX)) + u.ulevel > 25);
        if (!Deaf)
/*JP
            pline("%s.", Tobjnam(instr, do_spec ? "trill" : "toot"));
*/
            pline("%sを%sた．", xname(instr), do_spec ? "奏で" : "吹い");
        else
/*JP
            You_feel("%s %s.", yname(instr), do_spec ? "trill" : "toot");
*/
            You_feel("%sを%sた感じがした．", yname(instr), do_spec ? "奏で" : "吹い");
        if (do_spec)
            charm_snakes(u.ulevel * 3);
        exercise(A_DEX, TRUE);
        break;
    case FIRE_HORN:  /* Idem wand of fire */
    case FROST_HORN: /* Idem wand of cold */
        consume_obj_charge(instr, TRUE);

        if (!getdir((char *) 0)) {
/*JP
            pline("%s.", Tobjnam(instr, "vibrate"));
*/
            pline("%sは震えた．", xname(instr));
            break;
        } else if (!u.dx && !u.dy && !u.dz) {
            if ((damage = zapyourself(instr, TRUE)) != 0) {
                char buf[BUFSZ];

/*JP
                Sprintf(buf, "using a magical horn on %sself", uhim());
*/
                Strcpy(buf, "自分自身の魔法のホルンの力を浴びて");
                losehp(damage, buf, KILLED_BY); /* fire or frost damage */
            }
        } else {
            buzz((instr->otyp == FROST_HORN) ? AD_COLD - 1 : AD_FIRE - 1,
                 rn1(6, 6), u.ux, u.uy, u.dx, u.dy);
        }
        makeknown(instr->otyp);
        break;
    case TOOLED_HORN: /* Awaken or scare monsters */
        if (!Deaf)
/*JP
            You("produce a frightful, grave sound.");
*/
            You("身震いするような死者の音楽を奏でた．");
        else
/*JP
            You("blow into the horn.");
*/
            You("ホルンを吹いた．");
        awaken_monsters(u.ulevel * 30);
        exercise(A_WIS, FALSE);
        break;
    case BUGLE: /* Awaken & attract soldiers */
        if (!Deaf)
/*JP
            You("extract a loud noise from %s.", yname(instr));
*/
            You("%sから大きな耳障りな音を出した．", yname(instr));
        else
/*JP
            You("blow into the bugle.");
*/
            You("ラッパを吹いた．");
        awaken_soldiers(&youmonst);
        exercise(A_WIS, FALSE);
        break;
    case MAGIC_HARP: /* Charm monsters */
        consume_obj_charge(instr, TRUE);

        if (!Deaf)
/*JP
            pline("%s very attractive music.", Tobjnam(instr, "produce"));
*/
            pline("%sはとても魅力的な音楽を奏でた．", xname(instr));
        else
/*JP
            You_feel("very soothing vibrations.");
*/
            You_feel("とても落ち着いた雰囲気を感じた．");
        charm_monsters((u.ulevel - 1) / 3 + 1);
        exercise(A_DEX, TRUE);
        break;
    case WOODEN_HARP: /* May calm Nymph */
        do_spec &= (rn2(ACURR(A_DEX)) + u.ulevel > 25);
        if (!Deaf)
#if 0 /*JP:T*/
            pline("%s %s.", Yname2(instr),
                  do_spec ? "produces a lilting melody" : "twangs");
#else
        You("%s．", 
            do_spec ? "軽快な音楽を奏でた" : "ポローンという音を出した");
#endif
        else
/*JP
            You_feel("soothing vibrations.");
*/
            You_feel("落ち着いた雰囲気を感じた．");
        if (do_spec)
            calm_nymphs(u.ulevel * 3);
        exercise(A_DEX, TRUE);
        break;
    case DRUM_OF_EARTHQUAKE: /* create several pits */
        /* a drum of earthquake does not cause deafness
           while still magically functional, nor afterwards
           when it invokes the LEATHER_DRUM case instead and
           mundane is flagged */
        consume_obj_charge(instr, TRUE);

/*JP
        You("produce a heavy, thunderous rolling!");
*/
        You("重厚な雷のような音を奏でた！");
/*JP
        pline_The("entire %s is shaking around you!", generic_lvl_desc());
*/
        pline("あなたの回りの%sが揺れた！", generic_lvl_desc());
        do_earthquake((u.ulevel - 1) / 3 + 1);
        /* shake up monsters in a much larger radius... */
        awaken_monsters(ROWNO * COLNO);
        makeknown(DRUM_OF_EARTHQUAKE);
        break;
    case LEATHER_DRUM: /* Awaken monsters */
        if (!mundane) {
            if (!Deaf) {
/*JP
                You("beat a deafening row!");
*/
                You("耳が聞こえなくなるくらい叩いた！");
                incr_itimeout(&HDeaf, rn1(20, 30));
            } else {
/*JP
                You("pound on the drum.");
*/
                You("太鼓を激しく叩いた．");
            }
            exercise(A_WIS, FALSE);
        } else
#if 0 /*JP*/
            You("%s %s.",
                rn2(2) ? "butcher" : rn2(2) ? "manage" : "pull off",
                an(beats[rn2(SIZE(beats))]));
#else
            /*少しシンプルに*/
            You("%sを叩いた．",
                beats[rn2(SIZE(beats))]);
#endif
        awaken_monsters(u.ulevel * (mundane ? 5 : 40));
        context.botl = TRUE;
        break;
    default:
        impossible("What a weird instrument (%d)!", instr->otyp);
        return 0;
    }
    return 2; /* That takes time */
}

/*
 * So you want music...
 */
int
do_play_instrument(instr)
struct obj *instr;
{
    char buf[BUFSZ] = DUMMY, c = 'y';
    char *s;
    int x, y;
    boolean ok;

    if (Underwater) {
/*JP
        You_cant("play music underwater!");
*/
        You("水の中では音楽を奏でられない！");
        return 0;
    } else if ((instr->otyp == WOODEN_FLUTE || instr->otyp == MAGIC_FLUTE
                || instr->otyp == TOOLED_HORN || instr->otyp == FROST_HORN
                || instr->otyp == FIRE_HORN || instr->otyp == BUGLE)
               && !can_blow(&youmonst)) {
/*JP
        You("are incapable of playing %s.", the(distant_name(instr, xname)));
*/
        You("%sを演奏する能力がない．", the(distant_name(instr, xname)));
        return 0;
    }
    if (instr->otyp != LEATHER_DRUM && instr->otyp != DRUM_OF_EARTHQUAKE
        && !(Stunned || Confusion || Hallucination)) {
/*JP
        c = ynq("Improvise?");
*/
        c = ynq("即興で演奏する？");
        if (c == 'q')
            goto nevermind;
    }

    if (c == 'n') {
        if (u.uevent.uheard_tune == 2)
/*JP
            c = ynq("Play the passtune?");
*/
            c = ynq("コードを演奏する？");
        if (c == 'q') {
            goto nevermind;
        } else if (c == 'y') {
            Strcpy(buf, tune);
        } else {
/*JP
            getlin("What tune are you playing? [5 notes, A-G]", buf);
*/
            getlin("どのような調べを演奏しますか？[A-G から5音をいれてね]", buf);
            (void) mungspaces(buf);
            if (*buf == '\033')
                goto nevermind;

            /* convert to uppercase and change any "H" to the expected "B" */
            for (s = buf; *s; s++) {
#ifndef AMIGA
                *s = highc(*s);
#else
                /* The AMIGA supports two octaves of notes */
                if (*s == 'h')
                    *s = 'b';
#endif
                if (*s == 'H')
                    *s = 'B';
            }
        }

#if 0 /*JP:T*/
        You(!Deaf ? "extract a strange sound from %s!"
                  : "can feel %s emitting vibrations.", the(xname(instr)));
#else
        You(!Deaf ? "%sから奇妙な音を出した！"
                  : "%sが振動したのを感じた．", the(xname(instr)));
#endif

#ifdef UNIX386MUSIC
        /* if user is at the console, play through the console speaker */
        if (atconsole())
            speaker(instr, buf);
#endif
#ifdef VPIX_MUSIC
        if (sco_flag_console)
            speaker(instr, buf);
#endif
#ifdef MAC
        mac_speaker(instr, buf);
#endif
#ifdef PCMUSIC
        pc_speaker(instr, buf);
#endif
#ifdef AMIGA
        {
            char nbuf[20];
            int i;

            for (i = 0; buf[i] && i < 5; ++i) {
                nbuf[i * 2] = buf[i];
                nbuf[(i * 2) + 1] = 'h';
            }
            nbuf[i * 2] = 0;
            amii_speaker(instr, nbuf, AMII_OKAY_VOLUME);
        }
#endif
        /* Check if there was the Stronghold drawbridge near
         * and if the tune conforms to what we're waiting for.
         */
        if (Is_stronghold(&u.uz)) {
            exercise(A_WIS, TRUE); /* just for trying */
            if (!strcmp(buf, tune)) {
                /* Search for the drawbridge */
                for (y = u.uy - 1; y <= u.uy + 1; y++)
                    for (x = u.ux - 1; x <= u.ux + 1; x++)
                        if (isok(x, y))
                            if (find_drawbridge(&x, &y)) {
                                /* tune now fully known */
                                u.uevent.uheard_tune = 2;
                                if (levl[x][y].typ == DRAWBRIDGE_DOWN)
                                    close_drawbridge(x, y);
                                else
                                    open_drawbridge(x, y);
                                return 1;
                            }
            } else if (!Deaf) {
                if (u.uevent.uheard_tune < 1)
                    u.uevent.uheard_tune = 1;
                /* Okay, it wasn't the right tune, but perhaps
                 * we can give the player some hints like in the
                 * Mastermind game */
                ok = FALSE;
                for (y = u.uy - 1; y <= u.uy + 1 && !ok; y++)
                    for (x = u.ux - 1; x <= u.ux + 1 && !ok; x++)
                        if (isok(x, y))
                            if (IS_DRAWBRIDGE(levl[x][y].typ)
                                || is_drawbridge_wall(x, y) >= 0)
                                ok = TRUE;
                if (ok) { /* There is a drawbridge near */
                    int tumblers, gears;
                    boolean matched[5];

                    tumblers = gears = 0;
                    for (x = 0; x < 5; x++)
                        matched[x] = FALSE;

                    for (x = 0; x < (int) strlen(buf); x++)
                        if (x < 5) {
                            if (buf[x] == tune[x]) {
                                gears++;
                                matched[x] = TRUE;
                            } else {
                                for (y = 0; y < 5; y++)
                                    if (!matched[y] && buf[x] == tune[y]
                                        && buf[y] != tune[y]) {
                                        tumblers++;
                                        matched[y] = TRUE;
                                        break;
                                    }
                            }
                        }
                    if (tumblers) {
                        if (gears)
#if 0 /*JP:T*/
                            You_hear("%d tumbler%s click and %d gear%s turn.",
                                     tumblers, plur(tumblers), gears,
                                     plur(gears));
#else
                            You_hear("%d個の金具がカチっとなり，%d個の歯車がまわる音を聞いた．",
                                tumblers, gears);
#endif
                        else
#if 0 /*JP:T*/
                            You_hear("%d tumbler%s click.", tumblers,
                                     plur(tumblers));
#else
                            You_hear("%d個の金具がカチっとなる音を聞いた．",
                                     tumblers);
#endif
                    } else if (gears) {
/*JP
                        You_hear("%d gear%s turn.", gears, plur(gears));
*/
                        You_hear("%d個の歯車が回る音を聞いた．", gears);
                        /* could only get `gears == 5' by playing five
                           correct notes followed by excess; otherwise,
                           tune would have matched above */
                        if (gears == 5)
                            u.uevent.uheard_tune = 2;
                    }
                }
            }
        }
        return 1;
    } else
        return do_improvisation(instr);

 nevermind:
    pline1(Never_mind);
    return 0;
}

#ifdef UNIX386MUSIC
/*
 * Play audible music on the machine's speaker if appropriate.
 */

STATIC_OVL int
atconsole()
{
    /*
     * Kluge alert: This code assumes that your [34]86 has no X terminals
     * attached and that the console tty type is AT386 (this is always true
     * under AT&T UNIX for these boxen). The theory here is that your remote
     * ttys will have terminal type `ansi' or something else other than
     * `AT386' or `xterm'. We'd like to do better than this, but testing
     * to see if we're running on the console physical terminal is quite
     * difficult given the presence of virtual consoles and other modern
     * UNIX impedimenta...
     */
    char *termtype = nh_getenv("TERM");

/*JP
    return (!strcmp(termtype, "AT386") || !strcmp(termtype, "xterm"));
*/
    return (!strcmp(termtype, "AT386") || !strcmp(termtype, "xterm") || !strcmp(termtype, "kterm"));
}

STATIC_OVL void
speaker(instr, buf)
struct obj *instr;
char *buf;
{
    /*
     * For this to work, you need to have installed the PD speaker-control
     * driver for PC-compatible UNIX boxes that I (esr@snark.thyrsus.com)
     * posted to comp.sources.unix in Feb 1990.  A copy should be included
     * with your nethack distribution.
     */
    int fd;

    if ((fd = open("/dev/speaker", 1)) != -1) {
        /* send a prefix to modify instrumental `timbre' */
        switch (instr->otyp) {
        case WOODEN_FLUTE:
        case MAGIC_FLUTE:
            (void) write(fd, ">ol", 1); /* up one octave & lock */
            break;
        case TOOLED_HORN:
        case FROST_HORN:
        case FIRE_HORN:
            (void) write(fd, "<<ol", 2); /* drop two octaves & lock */
            break;
        case BUGLE:
            (void) write(fd, "ol", 2); /* octave lock */
            break;
        case WOODEN_HARP:
        case MAGIC_HARP:
            (void) write(fd, "l8mlol", 4); /* fast, legato, octave lock */
            break;
        }
        (void) write(fd, buf, strlen(buf));
        (void) nhclose(fd);
    }
}
#endif /* UNIX386MUSIC */

#ifdef VPIX_MUSIC

#if 0
#include <sys/types.h>
#include <sys/console.h>
#include <sys/vtkd.h>
#else
#define KIOC ('K' << 8)
#define KDMKTONE (KIOC | 8)
#endif

#define noDEBUG

/* emit tone of frequency hz for given number of ticks */
STATIC_OVL void
tone(hz, ticks)
unsigned int hz, ticks;
{
    ioctl(0, KDMKTONE, hz | ((ticks * 10) << 16));
#ifdef DEBUG
    printf("TONE: %6d %6d\n", hz, ticks * 10);
#endif
    nap(ticks * 10);
}

/* rest for given number of ticks */
STATIC_OVL void
rest(ticks)
int ticks;
{
    nap(ticks * 10);
#ifdef DEBUG
    printf("REST:        %6d\n", ticks * 10);
#endif
}

#include "interp.c" /* from snd86unx.shr */

STATIC_OVL void
speaker(instr, buf)
struct obj *instr;
char *buf;
{
    /* emit a prefix to modify instrumental `timbre' */
    playinit();
    switch (instr->otyp) {
    case WOODEN_FLUTE:
    case MAGIC_FLUTE:
        playstring(">ol", 1); /* up one octave & lock */
        break;
    case TOOLED_HORN:
    case FROST_HORN:
    case FIRE_HORN:
        playstring("<<ol", 2); /* drop two octaves & lock */
        break;
    case BUGLE:
        playstring("ol", 2); /* octave lock */
        break;
    case WOODEN_HARP:
    case MAGIC_HARP:
        playstring("l8mlol", 4); /* fast, legato, octave lock */
        break;
    }
    playstring(buf, strlen(buf));
}

#ifdef VPIX_DEBUG
main(argc, argv)
int argc;
char *argv[];
{
    if (argc == 2) {
        playinit();
        playstring(argv[1], strlen(argv[1]));
    }
}
#endif
#endif /* VPIX_MUSIC */

/*music.c*/
