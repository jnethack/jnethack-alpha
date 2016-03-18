/* NetHack 3.6	fountain.c	$NHDT-Date: 1444937416 2015/10/15 19:30:16 $  $NHDT-Branch: master $:$NHDT-Revision: 1.55 $ */
/*	Copyright Scott R. Turner, srt@ucla, 10/27/86 */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2016            */
/* JNetHack may be freely redistributed.  See license for details. */

/* Code for drinking from fountains. */

#include "hack.h"

STATIC_DCL void NDECL(dowatersnakes);
STATIC_DCL void NDECL(dowaterdemon);
STATIC_DCL void NDECL(dowaternymph);
STATIC_PTR void FDECL(gush, (int, int, genericptr_t));
STATIC_DCL void NDECL(dofindgem);

/* used when trying to dip in or drink from fountain or sink or pool while
   levitating above it, or when trying to move downwards in that state */
void
floating_above(what)
const char *what;
{
/*JP
    const char *umsg = "are floating high above the %s.";
*/
    const char *umsg = "%sの遥か上方に浮いている．";

    if (u.utrap && (u.utraptype == TT_INFLOOR || u.utraptype == TT_LAVA)) {
        /* when stuck in floor (not possible at fountain or sink location,
           so must be attempting to move down), override the usual message */
/*JP
        umsg = "are trapped in the %s.";
*/
        umsg = "%sにつかまっている．";
        what = surface(u.ux, u.uy); /* probably redundant */
    }
    You(umsg, what);
}

/* Fountain of snakes! */
STATIC_OVL void
dowatersnakes()
{
    register int num = rn1(5, 2);
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_MOCCASIN].mvflags & G_GONE)) {
        if (!Blind)
#if 0 /*JP*/
            pline("An endless stream of %s pours forth!",
                  Hallucination ? makeplural(rndmonnam(NULL)) : "snakes");
#else
            pline("%sがどどっと流れ出てきた！",
                  Hallucination ? rndmonnam(NULL) : "蛇");
#endif
        else
/*JP
            You_hear("%s hissing!", something);
*/
            You_hear("シーッという音を聞いた！");
        while (num-- > 0)
            if ((mtmp = makemon(&mons[PM_WATER_MOCCASIN], u.ux, u.uy,
                                NO_MM_FLAGS)) != 0
                && t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp);
    } else
/*JP
        pline_The("fountain bubbles furiously for a moment, then calms.");
*/
        pline("泉は突然激しく泡だち，やがて静かになった．");
}

/* Water demon */
STATIC_OVL void
dowaterdemon()
{
    struct monst *mtmp;

    if (!(mvitals[PM_WATER_DEMON].mvflags & G_GONE)) {
        if ((mtmp = makemon(&mons[PM_WATER_DEMON], u.ux, u.uy,
                            NO_MM_FLAGS)) != 0) {
            if (!Blind)
/*JP
                You("unleash %s!", a_monnam(mtmp));
*/
                You("%sを解き放した！", a_monnam(mtmp));
            else
/*JP
                You_feel("the presence of evil.");
*/
                You_feel("邪悪な存在を感じた！");

            /* Give those on low levels a (slightly) better chance of survival
             */
            if (rnd(100) > (80 + level_difficulty())) {
#if 0 /*JP*/
                pline("Grateful for %s release, %s grants you a wish!",
                      mhis(mtmp), mhe(mtmp));
#else
                pline("%sは解放をとても感謝し，のぞみをかなえてくれるようだ！",
                      mhe(mtmp));
#endif
                /* give a wish and discard the monster (mtmp set to null) */
                mongrantswish(&mtmp);
            } else if (t_at(mtmp->mx, mtmp->my))
                (void) mintrap(mtmp);
        }
    } else
/*JP
        pline_The("fountain bubbles furiously for a moment, then calms.");
*/
        pline("泉は突然激しく泡だち，やがて静かになった．");
}

/* Water Nymph */
STATIC_OVL void
dowaternymph()
{
    register struct monst *mtmp;

    if (!(mvitals[PM_WATER_NYMPH].mvflags & G_GONE)
        && (mtmp = makemon(&mons[PM_WATER_NYMPH], u.ux, u.uy,
                           NO_MM_FLAGS)) != 0) {
        if (!Blind)
/*JP
            You("attract %s!", a_monnam(mtmp));
*/
            pline("%sが現われた！", a_monnam(mtmp));
        else
/*JP
            You_hear("a seductive voice.");
*/
            You_hear("魅惑的な声を聞いた．");
        mtmp->msleeping = 0;
        if (t_at(mtmp->mx, mtmp->my))
            (void) mintrap(mtmp);
    } else if (!Blind)
/*JP
        pline("A large bubble rises to the surface and pops.");
*/
        pline("大きな泡が沸き出てはじけた．");
    else
/*JP
        You_hear("a loud pop.");
*/
        You_hear("大きなものがはじける音を聞いた．");
}

/* Gushing forth along LOS from (u.ux, u.uy) */
void
dogushforth(drinking)
int drinking;
{
    int madepool = 0;

    do_clear_area(u.ux, u.uy, 7, gush, (genericptr_t) &madepool);
    if (!madepool) {
        if (drinking)
/*JP
            Your("thirst is quenched.");
*/
            Your("渇きは癒された．");
        else
/*JP
            pline("Water sprays all over you.");
*/
            pline("水しぶきがあなたにかかった．");
    }
}

STATIC_PTR void
gush(x, y, poolcnt)
int x, y;
genericptr_t poolcnt;
{
    register struct monst *mtmp;
    register struct trap *ttmp;

    if (((x + y) % 2) || (x == u.ux && y == u.uy)
        || (rn2(1 + distmin(u.ux, u.uy, x, y))) || (levl[x][y].typ != ROOM)
        || (sobj_at(BOULDER, x, y)) || nexttodoor(x, y))
        return;

    if ((ttmp = t_at(x, y)) != 0 && !delfloortrap(ttmp))
        return;

    if (!((*(int *) poolcnt)++))
/*JP
        pline("Water gushes forth from the overflowing fountain!");
*/
        pline("泉から水がどどっと溢れ出た！");

    /* Put a pool at x, y */
    levl[x][y].typ = POOL;
    /* No kelp! */
    del_engr_at(x, y);
    water_damage_chain(level.objects[x][y], TRUE);

    if ((mtmp = m_at(x, y)) != 0)
        (void) minliquid(mtmp);
    else
        newsym(x, y);
}

/* Find a gem in the sparkling waters. */
STATIC_OVL void
dofindgem()
{
    if (!Blind)
/*JP
        You("spot a gem in the sparkling waters!");
*/
        pline("きらめく水の中に宝石を見つけた！");
    else
/*JP
        You_feel("a gem here!");
*/
        You_feel("宝石があるようだ！");
    (void) mksobj_at(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE - 1), u.ux, u.uy,
                     FALSE, FALSE);
    SET_FOUNTAIN_LOOTED(u.ux, u.uy);
    newsym(u.ux, u.uy);
    exercise(A_WIS, TRUE); /* a discovery! */
}

void
dryup(x, y, isyou)
xchar x, y;
boolean isyou;
{
    if (IS_FOUNTAIN(levl[x][y].typ)
        && (!rn2(3) || FOUNTAIN_IS_WARNED(x, y))) {
        if (isyou && in_town(x, y) && !FOUNTAIN_IS_WARNED(x, y)) {
            struct monst *mtmp;

            SET_FOUNTAIN_WARNED(x, y);
            /* Warn about future fountain use. */
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp))
                    continue;
                if (is_watch(mtmp->data) && couldsee(mtmp->mx, mtmp->my)
                    && mtmp->mpeaceful) {
/*JP
                    pline("%s yells:", Amonnam(mtmp));
*/
                    pline("%sは叫んだ：", Amonnam(mtmp));
/*JP
                    verbalize("Hey, stop using that fountain!");
*/
                    verbalize("おい，泉を汚すな！");
                    break;
                }
            }
            /* You can see or hear this effect */
            if (!mtmp)
/*JP
                pline_The("flow reduces to a trickle.");
*/
                pline("流れはちょろちょろになった．");
            return;
        }
        if (isyou && wizard) {
/*JP
            if (yn("Dry up fountain?") == 'n')
*/
            if (yn("泉を干上がらせますか？") == 'n')
                return;
        }
        /* replace the fountain with ordinary floor */
        levl[x][y].typ = ROOM;
        levl[x][y].looted = 0;
        levl[x][y].blessedftn = 0;
        if (cansee(x, y))
/*JP
            pline_The("fountain dries up!");
*/
            pline("泉は干上がった！");
        /* The location is seen if the hero/monster is invisible
           or felt if the hero is blind. */
        newsym(x, y);
        level.flags.nfountains--;
        if (isyou && in_town(x, y))
            (void) angry_guards(FALSE);
    }
}

void
drinkfountain()
{
    /* What happens when you drink from a fountain? */
    register boolean mgkftn = (levl[u.ux][u.uy].blessedftn == 1);
    register int fate = rnd(30);

    if (Levitation) {
/*JP
        floating_above("fountain");
*/
        floating_above("泉");
        return;
    }

    if (mgkftn && u.uluck >= 0 && fate >= 10) {
        int i, ii, littleluck = (u.uluck < 4);

/*JP
        pline("Wow!  This makes you feel great!");
*/
        pline("ワォ！とても気持ちよくなった！");
        /* blessed restore ability */
        for (ii = 0; ii < A_MAX; ii++)
            if (ABASE(ii) < AMAX(ii)) {
                ABASE(ii) = AMAX(ii);
                context.botl = 1;
            }
        /* gain ability, blessed if "natural" luck is high */
        i = rn2(A_MAX); /* start at a random attribute */
        for (ii = 0; ii < A_MAX; ii++) {
            if (adjattrib(i, 1, littleluck ? -1 : 0) && littleluck)
                break;
            if (++i >= A_MAX)
                i = 0;
        }
        display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
        pline("A wisp of vapor escapes the fountain...");
*/
        pline("煙のかたまりが泉から逃げた．．．");
        exercise(A_WIS, TRUE);
        levl[u.ux][u.uy].blessedftn = 0;
        return;
    }

    if (fate < 10) {
/*JP
        pline_The("cool draught refreshes you.");
*/
        pline("冷たい一杯でさっぱりした．");
        u.uhunger += rnd(10); /* don't choke on water */
        newuhs(FALSE);
        if (mgkftn)
            return;
    } else {
        switch (fate) {
        case 19: /* Self-knowledge */
/*JP
            You_feel("self-knowledgeable...");
*/
            You("自分自身が判るような気がした．．．");
            display_nhwindow(WIN_MESSAGE, FALSE);
            enlightenment(MAGICENLIGHTENMENT, ENL_GAMEINPROGRESS);
            exercise(A_WIS, TRUE);
/*JP
            pline_The("feeling subsides.");
*/
            pline("その感じはなくなった．");
            break;
        case 20: /* Foul water */
/*JP
            pline_The("water is foul!  You gag and vomit.");
*/
            pline("水はひどく不快な味がした！あなたは吐き戻した．");
            morehungry(rn1(20, 11));
            vomit();
            break;
        case 21: /* Poisonous */
/*JP
            pline_The("water is contaminated!");
*/
            pline("水は汚染されている！");
            if (Poison_resistance) {
/*JP
                pline("Perhaps it is runoff from the nearby %s farm.",
*/
                pline("たぶん，これは近くの%sの農場から流れている．",
                      fruitname(FALSE));
/*JP
                losehp(rnd(4), "unrefrigerated sip of juice", KILLED_BY_AN);
*/
                losehp(rnd(4),"腐った果汁のしたたりで", KILLED_BY_AN);
                break;
            }
            losestr(rn1(4, 3));
/*JP
            losehp(rnd(10), "contaminated water", KILLED_BY);
*/
            losehp(rnd(10),"汚染された水で", KILLED_BY);
            exercise(A_CON, FALSE);
            break;
        case 22: /* Fountain of snakes! */
            dowatersnakes();
            break;
        case 23: /* Water demon */
            dowaterdemon();
            break;
        case 24: /* Curse an item */ {
            register struct obj *obj;

/*JP
            pline("This water's no good!");
*/
            pline("この水はとてもまずい！");
            morehungry(rn1(20, 11));
            exercise(A_CON, FALSE);
            for (obj = invent; obj; obj = obj->nobj)
                if (!rn2(5))
                    curse(obj);
            break;
        }
        case 25: /* See invisible */
            if (Blind) {
                if (Invisible) {
/*JP
                    You("feel transparent.");
*/
                    You("透明になった気がする．");
                } else {
/*JP
                    You("feel very self-conscious.");
*/
                    You("自意識過剰に感じた．");
/*JP
                    pline("Then it passes.");
*/
                    pline("その感じは消えた．");
                }
            } else {
/*JP
                You_see("an image of someone stalking you.");
*/
                You("何かが自分の後をつけている映像を見た．");
/*JP
                pline("But it disappears.");
*/
                pline("しかし，それは消えてしまった．");
            }
            HSee_invisible |= FROMOUTSIDE;
            newsym(u.ux, u.uy);
            exercise(A_WIS, TRUE);
            break;
        case 26: /* See Monsters */
            (void) monster_detect((struct obj *) 0, 0);
            exercise(A_WIS, TRUE);
            break;
        case 27: /* Find a gem in the sparkling waters. */
            if (!FOUNTAIN_IS_LOOTED(u.ux, u.uy)) {
                dofindgem();
                break;
            }
        case 28: /* Water Nymph */
            dowaternymph();
            break;
        case 29: /* Scare */
        {
            register struct monst *mtmp;

/*JP
            pline("This water gives you bad breath!");
*/
            pline("水を飲んだら息が臭くなった！");
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (DEADMONSTER(mtmp))
                    continue;
                monflee(mtmp, 0, FALSE, FALSE);
            }
            break;
        }
        case 30: /* Gushing forth in this room */
            dogushforth(TRUE);
            break;
        default:
/*JP
            pline("This tepid water is tasteless.");
*/
            pline("このなまぬるい水は味がない．");
            break;
        }
    }
    dryup(u.ux, u.uy, TRUE);
}

void
dipfountain(obj)
register struct obj *obj;
{
    if (Levitation) {
/*JP
        floating_above("fountain");
*/
        floating_above("泉");
        return;
    }

    /* Don't grant Excalibur when there's more than one object.  */
    /* (quantity could be > 1 if merged daggers got polymorphed) */
    if (obj->otyp == LONG_SWORD && obj->quan == 1L && u.ulevel >= 5 && !rn2(6)
        && !obj->oartifact
        && !exist_artifact(LONG_SWORD, artiname(ART_EXCALIBUR))) {
        if (u.ualign.type != A_LAWFUL) {
            /* Ha!  Trying to cheat her. */
            pline(
/*JP
             "A freezing mist rises from the water and envelopes the sword.");
*/
             "氷の霧が水から立ち昇り，剣をつつんだ．");
/*JP
            pline_The("fountain disappears!");
*/
            pline("泉は消えてしまった！");
            curse(obj);
            if (obj->spe > -6 && !rn2(3))
                obj->spe--;
            obj->oerodeproof = FALSE;
            exercise(A_WIS, FALSE);
        } else {
            /* The lady of the lake acts! - Eric Backus */
            /* Be *REAL* nice */
            pline(
/*JP
              "From the murky depths, a hand reaches up to bless the sword.");
*/
              "にごった深みから，剣を祝福せんと手が伸びてきた．");
/*JP
            pline("As the hand retreats, the fountain disappears!");
*/
            pline("手が退くと，泉は消えてしまった！");
            obj = oname(obj, artiname(ART_EXCALIBUR));
            discover_artifact(ART_EXCALIBUR);
            bless(obj);
            obj->oeroded = obj->oeroded2 = 0;
            obj->oerodeproof = TRUE;
            exercise(A_WIS, TRUE);
        }
        update_inventory();
        levl[u.ux][u.uy].typ = ROOM;
        levl[u.ux][u.uy].looted = 0;
        newsym(u.ux, u.uy);
        level.flags.nfountains--;
        if (in_town(u.ux, u.uy))
            (void) angry_guards(FALSE);
        return;
    } else {
        int er = water_damage(obj, NULL, TRUE);

        if (obj->otyp == POT_ACID
            && er != ER_DESTROYED) { /* Acid and water don't mix */
            useup(obj);
            return;
        } else if (er != ER_NOTHING && !rn2(2)) { /* no further effect */
            return;
        }
    }

    switch (rnd(30)) {
    case 16: /* Curse the item */
        curse(obj);
        break;
    case 17:
    case 18:
    case 19:
    case 20: /* Uncurse the item */
        if (obj->cursed) {
            if (!Blind)
/*JP
                pline_The("water glows for a moment.");
*/
                pline("水は輝きだした．");
            uncurse(obj);
        } else {
/*JP
            pline("A feeling of loss comes over you.");
*/
            pline("奇妙な脱力感があなたをおそった．");
        }
        break;
    case 21: /* Water Demon */
        dowaterdemon();
        break;
    case 22: /* Water Nymph */
        dowaternymph();
        break;
    case 23: /* an Endless Stream of Snakes */
        dowatersnakes();
        break;
    case 24: /* Find a gem */
        if (!FOUNTAIN_IS_LOOTED(u.ux, u.uy)) {
            dofindgem();
            break;
        }
    case 25: /* Water gushes forth */
        dogushforth(FALSE);
        break;
    case 26: /* Strange feeling */
/*JP
        pline("A strange tingling runs up your %s.", body_part(ARM));
*/
        pline("奇妙なしびれがあなたの%sに走った．", body_part(ARM));
        break;
    case 27: /* Strange feeling */
/*JP
        You_feel("a sudden chill.");
*/
        You("突然寒けを感じた．");
        break;
    case 28: /* Strange feeling */
/*JP
        pline("An urge to take a bath overwhelms you.");
*/
        pline("風呂に入りたいという欲望にかられた．");
        {
            long money = money_cnt(invent);
            struct obj *otmp;
            if (money > 10) {
                /* Amount to lose.  Might get rounded up as fountains don't
                 * pay change... */
                money = somegold(money) / 10;
                for (otmp = invent; otmp && money > 0; otmp = otmp->nobj)
                    if (otmp->oclass == COIN_CLASS) {
                        int denomination = objects[otmp->otyp].oc_cost;
                        long coin_loss =
                            (money + denomination - 1) / denomination;
                        coin_loss = min(coin_loss, otmp->quan);
                        otmp->quan -= coin_loss;
                        money -= coin_loss * denomination;
                        if (!otmp->quan)
                            delobj(otmp);
                    }
/*JP
                You("lost some of your money in the fountain!");
*/
                You("金貨を数枚，泉に落としてしまった！");
                CLEAR_FOUNTAIN_LOOTED(u.ux, u.uy);
                exercise(A_WIS, FALSE);
            }
        }
        break;
    case 29: /* You see coins */
        /* We make fountains have more coins the closer you are to the
         * surface.  After all, there will have been more people going
         * by.  Just like a shopping mall!  Chris Woodbury  */

        if (FOUNTAIN_IS_LOOTED(u.ux, u.uy))
            break;
        SET_FOUNTAIN_LOOTED(u.ux, u.uy);
        (void) mkgold((long) (rnd((dunlevs_in_dungeon(&u.uz) - dunlev(&u.uz)
                                   + 1) * 2) + 5),
                      u.ux, u.uy);
        if (!Blind)
/*JP
            pline("Far below you, you see coins glistening in the water.");
*/
            You("遥か下の水中に金貨の輝きをみつけた．");
        exercise(A_WIS, TRUE);
        newsym(u.ux, u.uy);
        break;
    }
    update_inventory();
    dryup(u.ux, u.uy, TRUE);
}

void
breaksink(x, y)
int x, y;
{
    if (cansee(x, y) || (x == u.ux && y == u.uy))
/*JP
        pline_The("pipes break!  Water spurts out!");
*/
        pline("配管が壊れ水が噴出した！");
    level.flags.nsinks--;
    levl[x][y].doormask = 0;
    levl[x][y].typ = FOUNTAIN;
    level.flags.nfountains++;
    newsym(x, y);
}

void
drinksink()
{
    struct obj *otmp;
    struct monst *mtmp;

    if (Levitation) {
/*JP
        floating_above("sink");
*/
        floating_above("流し台");
        return;
    }
    switch (rn2(20)) {
    case 0:
/*JP
        You("take a sip of very cold water.");
*/
        You("とても冷たい水を一口飲んだ．");
        break;
    case 1:
/*JP
        You("take a sip of very warm water.");
*/
        You("とてもあたたかい水を一口飲んだ．");
        break;
    case 2:
/*JP
        You("take a sip of scalding hot water.");
*/
        You("とても熱い水を一口飲んだ．");
        if (Fire_resistance)
/*JP
            pline("It seems quite tasty.");
*/
            pline("とてもおいしい水だ．");
        else
/*JP
            losehp(rnd(6), "sipping boiling water", KILLED_BY);
*/
            losehp(rnd(6), "沸騰した水を飲んで", KILLED_BY);
        /* boiling water burns considered fire damage */
        break;
    case 3:
        if (mvitals[PM_SEWER_RAT].mvflags & G_GONE)
/*JP
            pline_The("sink seems quite dirty.");
*/
            pline("流し台はとても汚ならしい．");
        else {
            mtmp = makemon(&mons[PM_SEWER_RAT], u.ux, u.uy, NO_MM_FLAGS);
            if (mtmp)
#if 0 /*JP*/
                pline("Eek!  There's %s in the sink!",
                      (Blind || !canspotmon(mtmp)) ? "something squirmy"
                                                   : a_monnam(mtmp));
#else
                pline("げ！流し台に%sがいる！",
                      (Blind || !canspotmon(mtmp)) ? "身もだえするようなもの"
                                                   : a_monnam(mtmp));
#endif
        }
        break;
    case 4:
        do {
            otmp = mkobj(POTION_CLASS, FALSE);
            if (otmp->otyp == POT_WATER) {
                obfree(otmp, (struct obj *) 0);
                otmp = (struct obj *) 0;
            }
        } while (!otmp);
        otmp->cursed = otmp->blessed = 0;
#if 0 /*JP*/
        pline("Some %s liquid flows from the faucet.",
              Blind ? "odd" : hcolor(OBJ_DESCR(objects[otmp->otyp])));
#else
        pline("蛇口から%s液体が流れた．",
              Blind ? "奇妙な" :
              hcolor(OBJ_DESCR(objects[otmp->otyp])));
#endif
        otmp->dknown = !(Blind || Hallucination);
        otmp->quan++;       /* Avoid panic upon useup() */
        otmp->fromsink = 1; /* kludge for docall() */
        (void) dopotion(otmp);
        obfree(otmp, (struct obj *) 0);
        break;
    case 5:
        if (!(levl[u.ux][u.uy].looted & S_LRING)) {
/*JP
            You("find a ring in the sink!");
*/
            You("流し台に指輪をみつけた！");
            (void) mkobj_at(RING_CLASS, u.ux, u.uy, TRUE);
            levl[u.ux][u.uy].looted |= S_LRING;
            exercise(A_WIS, TRUE);
            newsym(u.ux, u.uy);
        } else
/*JP
            pline("Some dirty water backs up in the drain.");
*/
            pline("汚水が排水口から逆流してきた．");
        break;
    case 6:
        breaksink(u.ux, u.uy);
        break;
    case 7:
/*JP
        pline_The("water moves as though of its own will!");
*/
        pline("水が意思を持っているかのように動いた！");
        if ((mvitals[PM_WATER_ELEMENTAL].mvflags & G_GONE)
            || !makemon(&mons[PM_WATER_ELEMENTAL], u.ux, u.uy, NO_MM_FLAGS))
/*JP
            pline("But it quiets down.");
*/
            pline("しかし，静かになった．");
        break;
    case 8:
/*JP
        pline("Yuk, this water tastes awful.");
*/
        pline("オェ，とてもひどい味がする．");
        more_experienced(1, 0);
        newexplevel();
        break;
    case 9:
/*JP
        pline("Gaggg... this tastes like sewage!  You vomit.");
*/
        pline("ゲェー．下水のような味がする！あなたは吐き戻した．");
        morehungry(rn1(30 - ACURR(A_CON), 11));
        vomit();
        break;
    case 10:
/*JP
        pline("This water contains toxic wastes!");
*/
        pline("この水は有毒な排水を含んでいる！");
        if (!Unchanging) {
/*JP
            You("undergo a freakish metamorphosis!");
*/
            You("奇形な変化をしはじめた！");
            polyself(0);
        }
        break;
    /* more odd messages --JJB */
    case 11:
/*JP
        You_hear("clanking from the pipes...");
*/
        You_hear("配管のカチンという音を聞いた．．．");
        break;
    case 12:
/*JP
        You_hear("snatches of song from among the sewers...");
*/
        You_hear("下水の中からとぎれとぎれの歌を聞いた．．．");
        break;
    case 19:
        if (Hallucination) {
/*JP
            pline("From the murky drain, a hand reaches up... --oops--");
*/
            pline("暗い排水口から，手が伸びてきた．．--おっと--");
            break;
        }
    default:
#if 0 /*JP*/
        You("take a sip of %s water.",
            rn2(3) ? (rn2(2) ? "cold" : "warm") : "hot");
#else
        You("%s水を一口飲んだ．",
            rn2(3) ? (rn2(2) ? "冷たい" : "あたたかい") : "熱い");
#endif
    }
}

/*fountain.c*/
