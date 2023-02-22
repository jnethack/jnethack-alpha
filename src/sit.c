/* NetHack 3.6	sit.c	$NHDT-Date: 1559670609 2019/06/04 17:50:09 $  $NHDT-Branch: NetHack-3.6 $:$NHDT-Revision: 1.61 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2023            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"


/* take away the hero's money */
void
take_gold()
{
    struct obj *otmp, *nobj;
    int lost_money = 0;

    for (otmp = invent; otmp; otmp = nobj) {
        nobj = otmp->nobj;
        if (otmp->oclass == COIN_CLASS) {
            lost_money = 1;
            remove_worn_item(otmp, FALSE);
            delobj(otmp);
        }
    }
    if (!lost_money) {
/*JP
        You_feel("a strange sensation.");
*/
        You("奇妙な感覚を覚えた．");
    } else {
/*JP
        You("notice you have no money!");
*/
        You("お金を持ってないことに気がついた！");
        context.botl = 1;
    }
}

/* #sit command */
int
dosit()
{
/*JP
    static const char sit_message[] = "sit on the %s.";
*/
    static const char sit_message[] = "%sに座った．";
    register struct trap *trap = t_at(u.ux, u.uy);
    register int typ = levl[u.ux][u.uy].typ;

    if (u.usteed) {
/*JP
        You("are already sitting on %s.", mon_nam(u.usteed));
*/
        You("もう%sに座っている．", mon_nam(u.usteed));
        return 0;
    }
    if (u.uundetected && is_hider(youmonst.data) && u.umonnum != PM_TRAPPER)
        u.uundetected = 0; /* no longer on the ceiling */

    if (!can_reach_floor(FALSE)) {
        if (u.uswallow)
/*JP
            There("are no seats in here!");
*/
            pline("ここには椅子がない！");
        else if (Levitation)
/*JP
            You("tumble in place.");
*/
            You("その場で宙返りした．");
        else
/*JP
            You("are sitting on air.");
*/
            You("空中に座った．");
        return 0;
    } else if (u.ustuck && !sticks(youmonst.data)) {
        /* holding monster is next to hero rather than beneath, but
           hero is in no condition to actually sit at has/her own spot */
        if (humanoid(u.ustuck->data))
/*JP
            pline("%s won't offer %s lap.", Monnam(u.ustuck), mhis(u.ustuck));
*/
            pline("%sはひざを出さなかった．", Monnam(u.ustuck));
        else
/*JP
            pline("%s has no lap.", Monnam(u.ustuck));
*/
            pline("%sにはひざがない．", Monnam(u.ustuck));
        return 0;
    } else if (is_pool(u.ux, u.uy) && !Underwater) { /* water walking */
        goto in_water;
    }

    if (OBJ_AT(u.ux, u.uy)
        /* ensure we're not standing on the precipice */
        && !(uteetering_at_seen_pit(trap) || uescaped_shaft(trap))) {
        register struct obj *obj;

        obj = level.objects[u.ux][u.uy];
        if (youmonst.data->mlet == S_DRAGON && obj->oclass == COIN_CLASS) {
#if 0 /*JP:T*/
            You("coil up around your %shoard.",
                (obj->quan + money_cnt(invent) < u.ulevel * 1000) ? "meager "
                                                                  : "");
#else
            You("%sお宝のまわりでとぐろを巻いた．",
                (obj->quan + money_cnt(invent) < u.ulevel * 1000) ? "わずかな"
                                                                  : "");
#endif
        } else {
/*JP
            You("sit on %s.", the(xname(obj)));
*/
            You("%sに座った．", the(xname(obj)));
            if (!(Is_box(obj) || objects[obj->otyp].oc_material == CLOTH))
/*JP
                pline("It's not very comfortable...");
*/
                pline("あまり座りごこちがよくない．．．");
        }
    } else if (trap != 0 || (u.utrap && (u.utraptype >= TT_LAVA))) {
        if (u.utrap) {
            exercise(A_WIS, FALSE); /* you're getting stuck longer */
            if (u.utraptype == TT_BEARTRAP) {
/*JP
                You_cant("sit down with your %s in the bear trap.",
*/
                pline("%sが熊の罠にはさまっているので座れない．",
                         body_part(FOOT));
                u.utrap++;
            } else if (u.utraptype == TT_PIT) {
                if (trap && trap->ttyp == SPIKED_PIT) {
/*JP
                    You("sit down on a spike.  Ouch!");
*/
                    You("トゲの上に座った．いてっ！");
                    losehp(Half_physical_damage ? rn2(2) : 1,
/*JP
                           "sitting on an iron spike", KILLED_BY);
*/
                           "鉄のトゲの上に座って", KILLED_BY);
                    exercise(A_STR, FALSE);
                } else
/*JP
                    You("sit down in the pit.");
*/
                    You("落し穴の中で座った．");
                u.utrap += rn2(5);
            } else if (u.utraptype == TT_WEB) {
/*JP
                You("sit in the spider web and get entangled further!");
*/
                You("くもの巣の中で座ったら，ますます絡まった！");
                u.utrap += rn1(10, 5);
            } else if (u.utraptype == TT_LAVA) {
                /* Must have fire resistance or they'd be dead already */
/*JP
                You("sit in the %s!", hliquid("lava"));
*/
                You("%sの中に座った！", hliquid("溶岩"));
                if (Slimed)
                    burn_away_slime();
                u.utrap += rnd(4);
/*JP
                losehp(d(2, 10), "sitting in lava",
*/
                losehp(d(2, 10), "溶岩の中に座って",
                       KILLED_BY); /* lava damage */
            } else if (u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_BURIEDBALL) {
/*JP
                You_cant("maneuver to sit!");
*/
                You("座るような動作ができない！");
                u.utrap++;
            }
        } else {
/*JP
            You("sit down.");
*/
            You("座った．");
            dotrap(trap, VIASITTING);
        }
    } else if ((Underwater || Is_waterlevel(&u.uz))
                && !eggs_in_water(youmonst.data)) {
        if (Is_waterlevel(&u.uz))
/*JP
            There("are no cushions floating nearby.");
*/
            pline("近くに浮いているクッションはない．");
        else
/*JP
            You("sit down on the muddy bottom.");
*/
            You("どろどろした底に座った．");
    } else if (is_pool(u.ux, u.uy) && !eggs_in_water(youmonst.data)) {
    in_water:
/*JP
        You("sit in the %s.", hliquid("water"));
*/
        You("%sの中で座った．", hliquid("水"));
        if (!rn2(10) && uarm)
/*JP
            (void) water_damage(uarm, "armor", TRUE);
*/
            (void) water_damage(uarm, "鎧", TRUE);
        if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
/*JP
            (void) water_damage(uarm, "armor", TRUE);
*/
            (void) water_damage(uarm, "鎧", TRUE);
    } else if (IS_SINK(typ)) {
        You(sit_message, defsyms[S_sink].explanation);
/*JP
        Your("%s gets wet.", humanoid(youmonst.data) ? "rump" : "underside");
*/
        Your("%sは濡れた．", humanoid(youmonst.data) ? "尻" : "下部");
    } else if (IS_ALTAR(typ)) {
        You(sit_message, defsyms[S_altar].explanation);
        altar_wrath(u.ux, u.uy);
    } else if (IS_GRAVE(typ)) {
        You(sit_message, defsyms[S_grave].explanation);
    } else if (typ == STAIRS) {
/*JP
        You(sit_message, "stairs");
*/
        You(sit_message, "階段");
    } else if (typ == LADDER) {
/*JP
        You(sit_message, "ladder");
*/
        You(sit_message, "はしご");
    } else if (is_lava(u.ux, u.uy)) {
        /* must be WWalking */
/*JP
        You(sit_message, hliquid("lava"));
*/
        You(sit_message, hliquid("溶岩"));
        burn_away_slime();
        if (likes_lava(youmonst.data)) {
/*JP
            pline_The("%s feels warm.", hliquid("lava"));
*/
            pline_The("%sは暖かい．", hliquid("溶岩"));
            return 1;
        }
/*JP
        pline_The("%s burns you!", hliquid("lava"));
*/
        pline_The("%sで燃えた！", hliquid("溶岩"));
        losehp(d((Fire_resistance ? 2 : 10), 10), /* lava damage */
/*JP
               "sitting on lava", KILLED_BY);
*/
               "溶岩に座って", KILLED_BY);
    } else if (is_ice(u.ux, u.uy)) {
        You(sit_message, defsyms[S_ice].explanation);
        if (!Cold_resistance)
/*JP
            pline_The("ice feels cold.");
*/
            pline("氷は冷たく感じた．");
    } else if (typ == DRAWBRIDGE_DOWN) {
/*JP
        You(sit_message, "drawbridge");
*/
        You(sit_message, "跳ね橋");
    } else if (IS_THRONE(typ)) {
        You(sit_message, defsyms[S_throne].explanation);
        if (rnd(6) > 4) {
            switch (rnd(13)) {
            case 1:
                (void) adjattrib(rn2(A_MAX), -rn1(4, 3), FALSE);
/*JP
                losehp(rnd(10), "cursed throne", KILLED_BY_AN);
*/
                losehp(rnd(10), "呪われた玉座で", KILLED_BY_AN);
                break;
            case 2:
                (void) adjattrib(rn2(A_MAX), 1, FALSE);
                break;
            case 3:
#if 0 /*JP:T*/
                pline("A%s electric shock shoots through your body!",
                      (Shock_resistance) ? "n" : " massive");
#else
                pline("%s電気があなたの体を走り抜けた！",
                      (Shock_resistance) ? "" : "激しい");
#endif
/*JP
                losehp(Shock_resistance ? rnd(6) : rnd(30), "electric chair",
*/
                losehp(Shock_resistance ? rnd(6) : rnd(30), "電気椅子で",
                       KILLED_BY_AN);
                exercise(A_CON, FALSE);
                break;
            case 4:
/*JP
                You_feel("much, much better!");
*/
                You_feel("とても，とても元気になったような気がした！");
                if (Upolyd) {
                    if (u.mh >= (u.mhmax - 5))
                        u.mhmax += 4;
                    u.mh = u.mhmax;
                }
                if (u.uhp >= (u.uhpmax - 5))
                    u.uhpmax += 4;
                u.uhp = u.uhpmax;
                u.ucreamed = 0;
                make_blinded(0L, TRUE);
                make_sick(0L, (char *) 0, FALSE, SICK_ALL);
                heal_legs(0);
                context.botl = 1;
                break;
            case 5:
                take_gold();
                break;
            case 6:
                if (u.uluck + rn2(5) < 0) {
/*JP
                    You_feel("your luck is changing.");
*/
                    pline("運が向いてきた気がする．");
                    change_luck(1);
                } else
                    makewish();
                break;
            case 7:
              {
                int cnt = rnd(10);

                /* Magical voice not affected by deafness */
/*JP
                pline("A voice echoes:");
*/
                pline("声が響いた:");
#if 0 /*JP:T*/
                verbalize("Thy audience hath been summoned, %s!",
                          flags.female ? "Dame" : "Sire");
#else
                verbalize("%sよ！汝の聴衆召喚されし．",
                          flags.female ? "女" : "男");
#endif
                while (cnt--)
                    (void) makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
                break;
              }
            case 8:
                /* Magical voice not affected by deafness */
/*JP
                pline("A voice echoes:");
*/
                pline("声が響いた:");
#if 0 /*JP:T*/
                verbalize("By thine Imperious order, %s...",
                          flags.female ? "Dame" : "Sire");
#else
                verbalize("%sよ！汝の傲慢聞きいれようぞ．",
                          flags.female ? "女" : "男");
#endif
                do_genocide(5); /* REALLY|ONTHRONE, see do_genocide() */
                break;
            case 9:
                /* Magical voice not affected by deafness */
/*JP
                pline("A voice echoes:");
*/
                pline("声が響いた:");
                verbalize(
/*JP
                 "A curse upon thee for sitting upon this most holy throne!");
*/
                 "聖なる玉座に座りし汝に呪いあれ！");
                if (Luck > 0) {
                    make_blinded(Blinded + rn1(100, 250), TRUE);
                    change_luck((Luck > 1) ? -rnd(2) : -1);
                } else
                    rndcurse();
                break;
            case 10:
                if (Luck < 0 || (HSee_invisible & INTRINSIC)) {
                    if (level.flags.nommap) {
/*JP
                        pline("A terrible drone fills your head!");
*/
                        pline("恐しいブンブンという音が頭に響いた！");
                        make_confused((HConfusion & TIMEOUT) + (long) rnd(30),
                                      FALSE);
                    } else {
/*JP
                        pline("An image forms in your mind.");
*/
                        pline("あるイメージが頭に浮んだ．");
                        do_mapping();
                    }
                } else {
/*JP
                    Your("vision becomes clear.");
*/
                    Your("視界は冴え渡った．");
                    HSee_invisible |= FROMOUTSIDE;
                    newsym(u.ux, u.uy);
                }
                break;
            case 11:
                if (Luck < 0) {
/*JP
                    You_feel("threatened.");
*/
                    You("脅迫されているような気がした．");
                    aggravate();
                } else {
/*JP
                    You_feel("a wrenching sensation.");
*/
                    You("ねじられたような感覚を感じた．");
                    tele(); /* teleport him */
                }
                break;
            case 12:
/*JP
                You("are granted an insight!");
*/
                You("洞察力を得た！");
                if (invent) {
                    /* rn2(5) agrees w/seffects() */
                    identify_pack(rn2(5), FALSE);
                }
                break;
            case 13:
/*JP
                Your("mind turns into a pretzel!");
*/
                Your("心はクネクネになった！");
                make_confused((HConfusion & TIMEOUT) + (long) rn1(7, 16),
                              FALSE);
                break;
            default:
                impossible("throne effect");
                break;
            }
        } else {
            if (is_prince(youmonst.data))
/*JP
                You_feel("very comfortable here.");
*/
                You("ここはとても落ち着く．");
            else
/*JP
                You_feel("somehow out of place...");
*/
                You("何か場違いの気がした．．．");
        }

        if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ)) {
            /* may have teleported */
            levl[u.ux][u.uy].typ = ROOM, levl[u.ux][u.uy].flags = 0;
/*JP
            pline_The("throne vanishes in a puff of logic.");
*/
            pline("玉座はふっと消えた．");
            newsym(u.ux, u.uy);
        }
    } else if (lays_eggs(youmonst.data)) {
        struct obj *uegg;

        if (!flags.female) {
#if 0 /*JP:T*/
            pline("%s can't lay eggs!",
                  Hallucination
                      ? "You may think you are a platypus, but a male still"
                      : "Males");
#else
            pline("%s雄は卵を産めない！",
                  Hallucination
                      ? "あなたは自分がカモノハシだと思っているかもしれないが，やっぱり"
                      : "");
#endif
            return 0;
        } else if (u.uhunger < (int) objects[EGG].oc_nutrition) {
/*JP
            You("don't have enough energy to lay an egg.");
*/
            You("卵を産むだけのエネルギーがない．");
            return 0;
        } else if (eggs_in_water(youmonst.data)) {
            if (!(Underwater || Is_waterlevel(&u.uz))) {
/*JP
                pline("A splash tetra you are not.");
*/
                pline("あなたはコペラ・アーノルディではなかった．");
                return 0;
            }
            if (Upolyd &&
                (youmonst.data == &mons[PM_GIANT_EEL]
                 || youmonst.data == &mons[PM_ELECTRIC_EEL])) {
/*JP
                You("yearn for the Sargasso Sea.");
*/
                You("サルガッソー海にあこがれた．");
                return 0;
            }
        }
        uegg = mksobj(EGG, FALSE, FALSE);
        uegg->spe = 1;
        uegg->quan = 1L;
        uegg->owt = weight(uegg);
        /* this sets hatch timers if appropriate */
        set_corpsenm(uegg, egg_type_from_parent(u.umonnum, FALSE));
        uegg->known = uegg->dknown = 1;
#if 0 /*JP:T*/
        You("%s an egg.", eggs_in_water(youmonst.data) ? "spawn" : "lay");
#else
        You("卵を産んだ．");
#endif
        dropy(uegg);
        stackobj(uegg);
        morehungry((int) objects[EGG].oc_nutrition);
    } else {
/*JP
        pline("Having fun sitting on the %s?", surface(u.ux, u.uy));
*/
        pline("%sに座って楽しいかい？", surface(u.ux,u.uy));
    }
    return 1;
}

/* curse a few inventory items at random! */
void
rndcurse()
{
    int nobj = 0;
    int cnt, onum;
    struct obj *otmp;
/*JP
    static const char mal_aura[] = "feel a malignant aura surround %s.";
*/
    static const char mal_aura[] = "邪悪なオーラを%sの回りに感じた．";

    if (uwep && (uwep->oartifact == ART_MAGICBANE) && rn2(20)) {
/*JP
        You(mal_aura, "the magic-absorbing blade");
*/
        You(mal_aura, "魔力を吸いとる刀");
        return;
    }

    if (Antimagic) {
        shieldeff(u.ux, u.uy);
/*JP
        You(mal_aura, "you");
*/
        You(mal_aura, "あなた");
    }

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        /* gold isn't subject to being cursed or blessed */
        if (otmp->oclass == COIN_CLASS)
            continue;
        nobj++;
    }
    if (nobj) {
        for (cnt = rnd(6 / ((!!Antimagic) + (!!Half_spell_damage) + 1));
             cnt > 0; cnt--) {
            onum = rnd(nobj);
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                /* as above */
                if (otmp->oclass == COIN_CLASS)
                    continue;
                if (--onum == 0)
                    break; /* found the target */
            }
            /* the !otmp case should never happen; picking an already
               cursed item happens--avoid "resists" message in that case */
            if (!otmp || otmp->cursed)
                continue; /* next target */

            if (otmp->oartifact && spec_ability(otmp, SPFX_INTEL)
                && rn2(10) < 8) {
/*JP
                pline("%s!", Tobjnam(otmp, "resist"));
*/
                pline("%sは影響を受けない！", xname(otmp));
                continue;
            }

            if (otmp->blessed)
                unbless(otmp);
            else
                curse(otmp);
        }
        update_inventory();
    }

    /* treat steed's saddle as extended part of hero's inventory */
    if (u.usteed && !rn2(4) && (otmp = which_armor(u.usteed, W_SADDLE)) != 0
        && !otmp->cursed) { /* skip if already cursed */
        if (otmp->blessed)
            unbless(otmp);
        else
            curse(otmp);
        if (!Blind) {
#if 0 /*JP:T*/
            pline("%s %s.", Yobjnam2(otmp, "glow"),
                  hcolor(otmp->cursed ? NH_BLACK : (const char *) "brown"));
#else
            pline("%sは%s輝いた．", xname(otmp),
                  hcolor_adv(otmp->cursed ? NH_BLACK : (const char *)"茶色の"));
#endif
            otmp->bknown = 1; /* ok to bypass set_bknown() here */
        }
    }
}

/* remove a random INTRINSIC ability */
void
attrcurse()
{
    switch (rnd(11)) {
    case 1:
        if (HFire_resistance & INTRINSIC) {
            HFire_resistance &= ~INTRINSIC;
/*JP
            You_feel("warmer.");
*/
            You("暖かさを感じた．");
            break;
        }
        /*FALLTHRU*/
    case 2:
        if (HTeleportation & INTRINSIC) {
            HTeleportation &= ~INTRINSIC;
/*JP
            You_feel("less jumpy.");
*/
            You("ちょっと落ちついた．");
            break;
        }
        /*FALLTHRU*/
    case 3:
        if (HPoison_resistance & INTRINSIC) {
            HPoison_resistance &= ~INTRINSIC;
/*JP
            You_feel("a little sick!");
*/
            You("少し気分が悪くなった！");
            break;
        }
        /*FALLTHRU*/
    case 4:
        if (HTelepat & INTRINSIC) {
            HTelepat &= ~INTRINSIC;
            if (Blind && !Blind_telepat)
                see_monsters(); /* Can't sense mons anymore! */
/*JP
            Your("senses fail!");
*/
            Your("五感は麻痺した！");
            break;
        }
        /*FALLTHRU*/
    case 5:
        if (HCold_resistance & INTRINSIC) {
            HCold_resistance &= ~INTRINSIC;
/*JP
            You_feel("cooler.");
*/
            You("涼しさを感じた．");
            break;
        }
        /*FALLTHRU*/
    case 6:
        if (HInvis & INTRINSIC) {
            HInvis &= ~INTRINSIC;
/*JP
            You_feel("paranoid.");
*/
            You("妄想を抱いた．");
            break;
        }
        /*FALLTHRU*/
    case 7:
        if (HSee_invisible & INTRINSIC) {
            HSee_invisible &= ~INTRINSIC;
#if 0 /*JP:T*/
            You("%s!", Hallucination ? "tawt you taw a puttie tat"
                                     : "thought you saw something");
#else
            if(Hallucination)
                You("だれ蟹みら，れている．");
            else
                You("誰かに見られているような気がした！");
#endif
            break;
        }
        /*FALLTHRU*/
    case 8:
        if (HFast & INTRINSIC) {
            HFast &= ~INTRINSIC;
/*JP
            You_feel("slower.");
*/
            You("遅くなったような気がした．");
            break;
        }
        /*FALLTHRU*/
    case 9:
        if (HStealth & INTRINSIC) {
            HStealth &= ~INTRINSIC;
/*JP
            You_feel("clumsy.");
*/
            You("不器用になったような気がした．");
            break;
        }
        /*FALLTHRU*/
    case 10:
        /* intrinsic protection is just disabled, not set back to 0 */
        if (HProtection & INTRINSIC) {
            HProtection &= ~INTRINSIC;
/*JP
            You_feel("vulnerable.");
*/
            You("無防備になった気がした．");
            break;
        }
        /*FALLTHRU*/
    case 11:
        if (HAggravate_monster & INTRINSIC) {
            HAggravate_monster &= ~INTRINSIC;
/*JP
            You_feel("less attractive.");
*/
            You("魅力が失せたような気がした．");
            break;
        }
        /*FALLTHRU*/
    default:
        break;
    }
}

/*sit.c*/
