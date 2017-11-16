/* NetHack 3.6	steal.c	$NHDT-Date: 1446713643 2015/11/05 08:54:03 $  $NHDT-Branch: master $:$NHDT-Revision: 1.65 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2016            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_PTR int NDECL(stealarm);

STATIC_DCL const char *FDECL(equipname, (struct obj *));

STATIC_OVL const char *
equipname(otmp)
register struct obj *otmp;
{
    return ((otmp == uarmu)
/*JP
                ? "shirt"
*/
                ? "シャツ"
                : (otmp == uarmf)
/*JP
                      ? "boots"
*/
                      ? "靴"
                      : (otmp == uarms)
/*JP
                            ? "shield"
*/
                            ? "盾"
                            : (otmp == uarmg)
/*JP
                                  ? "gloves"
*/
                                  ? "小手"
                                  : (otmp == uarmc)
                                        ? cloak_simple_name(otmp)
                                        : (otmp == uarmh)
                                              ? helm_simple_name(otmp)
                                              : suit_simple_name(otmp));
}

/* proportional subset of gold; return value actually fits in an int */
long
somegold(lmoney)
long lmoney;
{
#ifdef LINT /* long conv. ok */
    int igold = 0;
#else
    int igold = (lmoney >= (long) LARGEST_INT) ? LARGEST_INT : (int) lmoney;
#endif

    if (igold < 50)
        ; /* all gold */
    else if (igold < 100)
        igold = rn1(igold - 25 + 1, 25);
    else if (igold < 500)
        igold = rn1(igold - 50 + 1, 50);
    else if (igold < 1000)
        igold = rn1(igold - 100 + 1, 100);
    else if (igold < 5000)
        igold = rn1(igold - 500 + 1, 500);
    else if (igold < 10000)
        igold = rn1(igold - 1000 + 1, 1000);
    else
        igold = rn1(igold - 5000 + 1, 5000);

    return (long) igold;
}

/*
 * Find the first (and hopefully only) gold object in a chain.
 * Used when leprechaun (or you as leprechaun) looks for
 * someone else's gold.  Returns a pointer so the gold may
 * be seized without further searching.
 * May search containers too.
 * Deals in gold only, as leprechauns don't care for lesser coins.
*/
struct obj *
findgold(chain)
register struct obj *chain;
{
    while (chain && chain->otyp != GOLD_PIECE)
        chain = chain->nobj;
    return chain;
}

/*
 * Steal gold coins only.  Leprechauns don't care for lesser coins.
*/
void
stealgold(mtmp)
register struct monst *mtmp;
{
    register struct obj *fgold = g_at(u.ux, u.uy);
    register struct obj *ygold;
    register long tmp;
#if 0 /*JP*/
    struct monst *who;
    const char *whose, *what;
#endif

    /* skip lesser coins on the floor */
    while (fgold && fgold->otyp != GOLD_PIECE)
        fgold = fgold->nexthere;

    /* Do you have real gold? */
    ygold = findgold(invent);

    if (fgold && (!ygold || fgold->quan > ygold->quan || !rn2(5))) {
        obj_extract_self(fgold);
        add_to_minv(mtmp, fgold);
        newsym(u.ux, u.uy);
#if 0 /*JP*/
        if (u.usteed) {
            who = u.usteed;
            whose = s_suffix(y_monnam(who));
            what = makeplural(mbodypart(who, FOOT));
        } else {
            who = &youmonst;
            whose = "your";
            what = makeplural(body_part(FOOT));
        }
        /* [ avoid "between your rear regions" :-] */
        if (slithy(who->data))
            what = "coils";
        /* reduce "rear hooves/claws" to "hooves/claws" */
        if (!strncmp(what, "rear ", 5))
            what += 5;
        pline("%s quickly snatches some gold from %s %s %s!", Monnam(mtmp),
              (Levitation || Flying) ? "beneath" : "between", whose, what);
#else /*JP:足が無くても「足元」でよしとする*/
        pline("%sは素早くあなたの%sから金をひったくった！", Monnam(mtmp),
              (Levitation || Flying) ? "下" : "足元");
#endif
        if (!ygold || !rn2(5)) {
            if (!tele_restrict(mtmp))
                (void) rloc(mtmp, TRUE);
            monflee(mtmp, 0, FALSE, FALSE);
        }
    } else if (ygold) {
        const int gold_price = objects[GOLD_PIECE].oc_cost;

        tmp = (somegold(money_cnt(invent)) + gold_price - 1) / gold_price;
        tmp = min(tmp, ygold->quan);
        if (tmp < ygold->quan)
            ygold = splitobj(ygold, tmp);
        else
            setnotworn(ygold);
        freeinv(ygold);
        add_to_minv(mtmp, ygold);
/*JP
        Your("purse feels lighter.");
*/
        Your("財布は軽くなった．");
        if (!tele_restrict(mtmp))
            (void) rloc(mtmp, TRUE);
        monflee(mtmp, 0, FALSE, FALSE);
        context.botl = 1;
    }
}

/* steal armor after you finish taking it off */
unsigned int stealoid; /* object to be stolen */
unsigned int stealmid; /* monster doing the stealing */

STATIC_PTR int
stealarm(VOID_ARGS)
{
    register struct monst *mtmp;
    register struct obj *otmp;

    for (otmp = invent; otmp; otmp = otmp->nobj) {
        if (otmp->o_id == stealoid) {
            for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
                if (mtmp->m_id == stealmid) {
                    if (DEADMONSTER(mtmp))
                        impossible("stealarm(): dead monster stealing");
                    if (!dmgtype(mtmp->data, AD_SITM)) /* polymorphed */
                        goto botm;
                    if (otmp->unpaid)
                        subfrombill(otmp, shop_keeper(*u.ushops));
                    freeinv(otmp);
/*JP
                    pline("%s steals %s!", Monnam(mtmp), doname(otmp));
*/
                    pline("%sは%sを盗んだ！", Monnam(mtmp), doname(otmp));
                    (void) mpickobj(mtmp, otmp); /* may free otmp */
                    /* Implies seduction, "you gladly hand over ..."
                       so we don't set mavenge bit here. */
                    monflee(mtmp, 0, FALSE, FALSE);
                    if (!tele_restrict(mtmp))
                        (void) rloc(mtmp, TRUE);
                    break;
                }
            }
            break;
        }
    }
botm:
    stealoid = 0;
    return 0;
}

/* An object you're wearing has been taken off by a monster (theft or
   seduction).  Also used if a worn item gets transformed (stone to flesh). */
void
remove_worn_item(obj, unchain_ball)
struct obj *obj;
boolean unchain_ball; /* whether to unpunish or just unwield */
{
    if (donning(obj))
        cancel_don();
    if (!obj->owornmask)
        return;

    if (obj->owornmask & W_ARMOR) {
        if (obj == uskin) {
            impossible("Removing embedded scales?");
            skinback(TRUE); /* uarm = uskin; uskin = 0; */
        }
        if (obj == uarm)
            (void) Armor_off();
        else if (obj == uarmc)
            (void) Cloak_off();
        else if (obj == uarmf)
            (void) Boots_off();
        else if (obj == uarmg)
            (void) Gloves_off();
        else if (obj == uarmh)
            (void) Helmet_off();
        else if (obj == uarms)
            (void) Shield_off();
        else if (obj == uarmu)
            (void) Shirt_off();
        /* catchall -- should never happen */
        else
            setworn((struct obj *) 0, obj->owornmask & W_ARMOR);
    } else if (obj->owornmask & W_AMUL) {
        Amulet_off();
    } else if (obj->owornmask & W_RING) {
        Ring_gone(obj);
    } else if (obj->owornmask & W_TOOL) {
        Blindf_off(obj);
    } else if (obj->owornmask & W_WEAPON) {
        if (obj == uwep)
            uwepgone();
        if (obj == uswapwep)
            uswapwepgone();
        if (obj == uquiver)
            uqwepgone();
    }

    if (obj->owornmask & (W_BALL | W_CHAIN)) {
        if (unchain_ball)
            unpunish();
    } else if (obj->owornmask) {
        /* catchall */
        setnotworn(obj);
    }
}

/* Returns 1 when something was stolen (or at least, when N should flee now)
 * Returns -1 if the monster died in the attempt
 * Avoid stealing the object stealoid
 * Nymphs and monkeys won't steal coins
 */
int
steal(mtmp, objnambuf)
struct monst *mtmp;
char *objnambuf;
{
    struct obj *otmp;
    int tmp, could_petrify, armordelay, olddelay, named = 0, retrycnt = 0;
    boolean monkey_business, /* true iff an animal is doing the thievery */
        was_doffing;

    if (objnambuf)
        *objnambuf = '\0';
    /* the following is true if successful on first of two attacks. */
    if (!monnear(mtmp, u.ux, u.uy))
        return 0;

    /* food being eaten might already be used up but will not have
       been removed from inventory yet; we don't want to steal that,
       so this will cause it to be removed now */
    if (occupation)
        (void) maybe_finished_meal(FALSE);

    if (!invent || (inv_cnt(FALSE) == 1 && uskin)) {
    nothing_to_steal:
        /* Not even a thousand men in armor can strip a naked man. */
        if (Blind)
/*JP
            pline("Somebody tries to rob you, but finds nothing to steal.");
*/
            pline("誰かがあなたから盗もうとしたが，盗むものがないことに気がついた．");
        else
/*JP
            pline("%s tries to rob you, but there is nothing to steal!",
*/
            pline("%sはあなたから盗もうとしたが，盗むものがないことに気がついた！",
                  Monnam(mtmp));
        return 1; /* let her flee */
    }

    monkey_business = is_animal(mtmp->data);
    if (monkey_business || uarmg) {
        ; /* skip ring special cases */
    } else if (Adornment & LEFT_RING) {
        otmp = uleft;
        goto gotobj;
    } else if (Adornment & RIGHT_RING) {
        otmp = uright;
        goto gotobj;
    }

retry:
    tmp = 0;
    for (otmp = invent; otmp; otmp = otmp->nobj)
        if ((!uarm || otmp != uarmc) && otmp != uskin
            && otmp->oclass != COIN_CLASS)
            tmp += (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) ? 5 : 1;
    if (!tmp)
        goto nothing_to_steal;
    tmp = rn2(tmp);
    for (otmp = invent; otmp; otmp = otmp->nobj)
        if ((!uarm || otmp != uarmc) && otmp != uskin
            && otmp->oclass != COIN_CLASS) {
            tmp -= (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) ? 5 : 1;
            if (tmp < 0)
                break;
        }
    if (!otmp) {
        impossible("Steal fails!");
        return 0;
    }
    /* can't steal ring(s) while wearing gloves */
    if ((otmp == uleft || otmp == uright) && uarmg)
        otmp = uarmg;
    /* can't steal gloves while wielding - so steal the wielded item. */
    if (otmp == uarmg && uwep)
        otmp = uwep;
    /* can't steal armor while wearing cloak - so steal the cloak. */
    else if (otmp == uarm && uarmc)
        otmp = uarmc;
    /* can't steal shirt while wearing cloak or suit */
    else if (otmp == uarmu && uarmc)
        otmp = uarmc;
    else if (otmp == uarmu && uarm)
        otmp = uarm;

gotobj:
    if (otmp->o_id == stealoid)
        return 0;

    if (otmp->otyp == BOULDER && !throws_rocks(mtmp->data)) {
        if (!retrycnt++)
            goto retry;
        goto cant_take;
    }
    /* animals can't overcome curse stickiness nor unlock chains */
    if (monkey_business) {
        boolean ostuck;
        /* is the player prevented from voluntarily giving up this item?
           (ignores loadstones; the !can_carry() check will catch those) */
        if (otmp == uball)
            ostuck = TRUE; /* effectively worn; curse is implicit */
        else if (otmp == uquiver || (otmp == uswapwep && !u.twoweap))
            ostuck = FALSE; /* not really worn; curse doesn't matter */
        else
            ostuck = ((otmp->cursed && otmp->owornmask)
                      /* nymphs can steal rings from under
                         cursed weapon but animals can't */
                      || (otmp == uright && welded(uwep))
                      || (otmp == uleft && welded(uwep) && bimanual(uwep)));

        if (ostuck || can_carry(mtmp, otmp) == 0) {
#if 0 /*JP*/
            static const char *const how[] = { "steal", "snatch", "grab",
                                               "take" };
        cant_take:
            pline("%s tries to %s %s%s but gives up.", Monnam(mtmp),
                  how[rn2(SIZE(how))],
                  (otmp->owornmask & W_ARMOR) ? "your " : "",
                  (otmp->owornmask & W_ARMOR) ? equipname(otmp)
                                              : yname(otmp));
#else
        cant_take:
            pline("%sは%sを盗もうとしたが諦めた．", Monnam(mtmp),
                  (otmp->owornmask & W_ARMOR) ? equipname(otmp)
                                              : yname(otmp));
#endif
            /* the fewer items you have, the less likely the thief
               is going to stick around to try again (0) instead of
               running away (1) */
            return !rn2(inv_cnt(FALSE) / 5 + 2);
        }
    }

    if (otmp->otyp == LEASH && otmp->leashmon) {
        if (monkey_business && otmp->cursed)
            goto cant_take;
        o_unleash(otmp);
    }

    was_doffing = doffing(otmp);
    /* stop donning/doffing now so that afternmv won't be clobbered
       below; stop_occupation doesn't handle donning/doffing */
    olddelay = stop_donning(otmp);
    /* you're going to notice the theft... */
    stop_occupation();

    if (otmp->owornmask & (W_ARMOR | W_ACCESSORY)) {
        switch (otmp->oclass) {
        case TOOL_CLASS:
        case AMULET_CLASS:
        case RING_CLASS:
        case FOOD_CLASS: /* meat ring */
            remove_worn_item(otmp, TRUE);
            break;
        case ARMOR_CLASS:
            armordelay = objects[otmp->otyp].oc_delay;
            if (olddelay > 0 && olddelay < armordelay)
                armordelay = olddelay;
            if (monkey_business) {
                /* animals usually don't have enough patience
                   to take off items which require extra time */
                if (armordelay >= 1 && !olddelay && rn2(10))
                    goto cant_take;
                remove_worn_item(otmp, TRUE);
                break;
            } else {
                int curssv = otmp->cursed;
                int slowly;
                boolean seen = canspotmon(mtmp);

                otmp->cursed = 0;
                /* can't charm you without first waking you */
                if (Unaware)
                    unmul((char *) 0);
                slowly = (armordelay >= 1 || multi < 0);
                if (flags.female)
#if 0 /*JP*/
                    pline("%s charms you.  You gladly %s your %s.",
                          !seen ? "She" : Monnam(mtmp),
                          curssv ? "let her take"
                                 : !slowly ? "hand over"
                                           : was_doffing ? "continue removing"
                                                         : "start removing",
                          equipname(otmp));
#else
                    pline("%sはあなたを魅了した．あなたはよろこんで%sを%sた．",
                          !seen ? "彼女" : Monnam(mtmp),
                          equipname(otmp),
                          curssv ? "はずしてもらっ"
                                 : !slowly ? "はずして手渡し"
                                           : was_doffing ? "はずし続け"
                                                         : "はずし始め");
#endif
                else
#if 0 /*JP*/
                    pline("%s seduces you and %s off your %s.",
                          !seen ? "She" : Adjmonnam(mtmp, "beautiful"),
                          curssv
                              ? "helps you to take"
                              : !slowly ? "you take"
                                        : was_doffing ? "you continue taking"
                                                      : "you start taking",
                          equipname(otmp));
#else
                    pline("%sはあなたを誘惑した．あなたは%sを%s．",
                          !seen ? "彼女" : Adjmonnam(mtmp, "美しい"),
                          equipname(otmp),
                          curssv
                              ? "はずしてもらっ"
                              : !slowly ? "はずし"
                                        : was_doffing ? "はずし続け"
                                                      : "はずし始め");
#endif
                named++;
                /* the following is to set multi for later on */
                nomul(-armordelay);
/*JP
                multi_reason = "taking off clothes";
*/
                multi_reason = "服を脱いでいる時に";
                nomovemsg = 0;
                remove_worn_item(otmp, TRUE);
                otmp->cursed = curssv;
                if (multi < 0) {
                    /*
                    multi = 0;
                    afternmv = 0;
                    */
                    stealoid = otmp->o_id;
                    stealmid = mtmp->m_id;
                    afternmv = stealarm;
                    return 0;
                }
            }
            break;
        default:
            impossible("Tried to steal a strange worn thing. [%d]",
                       otmp->oclass);
        }
    } else if (otmp->owornmask)
        remove_worn_item(otmp, TRUE);

    /* do this before removing it from inventory */
    if (objnambuf)
        Strcpy(objnambuf, yname(otmp));
    /* set mavenge bit so knights won't suffer an
     * alignment penalty during retaliation;
     */
    mtmp->mavenge = 1;

    if (otmp->unpaid)
        subfrombill(otmp, shop_keeper(*u.ushops));
    freeinv(otmp);
/*JP
    pline("%s stole %s.", named ? "She" : Monnam(mtmp), doname(otmp));
*/
    pline("%sは%sを盗んだ．", named ? "彼女" : Monnam(mtmp), doname(otmp));
    could_petrify =
        (otmp->otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]));
    (void) mpickobj(mtmp, otmp); /* may free otmp */
    if (could_petrify && !(mtmp->misc_worn_check & W_ARMG)) {
        minstapetrify(mtmp, TRUE);
        return -1;
    }
    return (multi < 0) ? 0 : 1;
}

/* Returns 1 if otmp is free'd, 0 otherwise. */
int
mpickobj(mtmp, otmp)
register struct monst *mtmp;
register struct obj *otmp;
{
    int freed_otmp;
    boolean snuff_otmp = FALSE;

    /* if monster is acquiring a thrown or kicked object, the throwing
       or kicking code shouldn't continue to track and place it */
    if (otmp == thrownobj)
        thrownobj = 0;
    else if (otmp == kickedobj)
        kickedobj = 0;
    /* don't want hidden light source inside the monster; assumes that
       engulfers won't have external inventories; whirly monsters cause
       the light to be extinguished rather than letting it shine thru */
    if (obj_sheds_light(otmp) && attacktype(mtmp->data, AT_ENGL)) {
        /* this is probably a burning object that you dropped or threw */
        if (u.uswallow && mtmp == u.ustuck && !Blind)
/*JP
            pline("%s out.", Tobjnam(otmp, "go"));
*/
            pline("%sは飛びだした．", xname(otmp));
        snuff_otmp = TRUE;
    }
    /* for hero owned object on shop floor, mtmp is taking possession
       and if it's eventually dropped in a shop, shk will claim it */
    if (!mtmp->mtame)
        otmp->no_charge = 0;
    /* Must do carrying effects on object prior to add_to_minv() */
    carry_obj_effects(otmp);
    /* add_to_minv() might free otmp [if merged with something else],
       so we have to call it after doing the object checks */
    freed_otmp = add_to_minv(mtmp, otmp);
    /* and we had to defer this until object is in mtmp's inventory */
    if (snuff_otmp)
        snuff_light_source(mtmp->mx, mtmp->my);
    return freed_otmp;
}

void
stealamulet(mtmp)
struct monst *mtmp;
{
    struct obj *otmp = (struct obj *) 0;
    int real = 0, fake = 0;

    /* select the artifact to steal */
    if (u.uhave.amulet) {
        real = AMULET_OF_YENDOR;
        fake = FAKE_AMULET_OF_YENDOR;
    } else if (u.uhave.questart) {
        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (is_quest_artifact(otmp))
                break;
        if (!otmp)
            return; /* should we panic instead? */
    } else if (u.uhave.bell) {
        real = BELL_OF_OPENING;
        fake = BELL;
    } else if (u.uhave.book) {
        real = SPE_BOOK_OF_THE_DEAD;
    } else if (u.uhave.menorah) {
        real = CANDELABRUM_OF_INVOCATION;
    } else
        return; /* you have nothing of special interest */

    if (!otmp) {
        /* If we get here, real and fake have been set up. */
        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (otmp->otyp == real || (otmp->otyp == fake && !mtmp->iswiz))
                break;
    }

    if (otmp) { /* we have something to snatch */
        if (otmp->owornmask)
            remove_worn_item(otmp, TRUE);
        if (otmp->unpaid)
            subfrombill(otmp, shop_keeper(*u.ushops));
        freeinv(otmp);
        /* mpickobj wont merge otmp because none of the above things
           to steal are mergable */
        (void) mpickobj(mtmp, otmp); /* may merge and free otmp */
/*JP
        pline("%s stole %s!", Monnam(mtmp), doname(otmp));
*/
        pline("%sは%sを盗んだ！", Monnam(mtmp), doname(otmp));
        if (can_teleport(mtmp->data) && !tele_restrict(mtmp))
            (void) rloc(mtmp, TRUE);
    }
}

/* when a mimic gets poked with something, it might take that thing
   (at present, only implemented for when the hero does the poking) */
void
maybe_absorb_item(mon, obj, ochance, achance)
struct monst *mon;
struct obj *obj;
int ochance, achance; /* percent chance for ordinary item, artifact */
{
    if (obj == uball || obj == uchain || obj->oclass == ROCK_CLASS
        || obj_resists(obj, 100 - ochance, 100 - achance)
        || !touch_artifact(obj, mon))
        return;

    if (carried(obj)) {
        if (obj->owornmask)
            remove_worn_item(obj, TRUE);
        if (obj->unpaid)
            subfrombill(obj, shop_keeper(*u.ushops));
        if (cansee(mon->mx, mon->my)) {
            const char *MonName = Monnam(mon);

#if 0 /*JP*//*日本語では不要*/
            /* mon might be invisible; avoid "It pulls ... and absorbs it!" */
            if (!strcmp(MonName, "It"))
                MonName = "Something";
#endif
#if 0 /*JP*/
            pline("%s pulls %s away from you and absorbs %s!", MonName,
                  yname(obj), (obj->quan > 1L) ? "them" : "it");
#else
            pline("%sは%sを引っぱり込んで吸収した！", MonName,
                  yname(obj));
#endif
        } else {
            const char *hand_s = body_part(HAND);

            if (bimanual(obj))
                hand_s = makeplural(hand_s);
#if 0 /*JP*/
            pline("%s %s pulled from your %s!", upstart(yname(obj)),
                  otense(obj, "are"), hand_s);
#else
            pline("%sはあなたの%sから引っぱり込まれた！", upstart(yname(obj)),
                  hand_s);
#endif
        }
        freeinv(obj);
    } else {
        /* not carried; presumably thrown or kicked */
        if (canspotmon(mon))
/*JP
            pline("%s absorbs %s!", Monnam(mon), yname(obj));
*/
            pline("%sは%sを吸収した！", Monnam(mon), yname(obj));
    }
    /* add to mon's inventory */
    (void) mpickobj(mon, obj);
}

/* drop one object taken from a (possibly dead) monster's inventory */
void
mdrop_obj(mon, obj, verbosely)
struct monst *mon;
struct obj *obj;
boolean verbosely;
{
    int omx = mon->mx, omy = mon->my;
    boolean update_mon = FALSE;

    if (obj->owornmask) {
        /* perform worn item handling if the monster is still alive */
        if (mon->mhp > 0) {
            mon->misc_worn_check &= ~obj->owornmask;
            update_mon = TRUE;
            /* don't charge for an owned saddle on dead steed (provided
               that the hero is within the same shop at the time) */
        } else if (mon->mtame && (obj->owornmask & W_SADDLE) && !obj->unpaid
                   && costly_spot(omx, omy)
                   /* being at costly_spot guarantees lev->roomno is not 0 */
                   && index(in_rooms(u.ux, u.uy, SHOPBASE),
                            levl[omx][omy].roomno)) {
            obj->no_charge = 1;
        }
        /* this should be done even if the monster has died */
        if (obj->owornmask & W_WEP)
            setmnotwielded(mon, obj);
        obj->owornmask = 0L;
    }
    /* obj_no_longer_held(obj); -- done by place_object */
    if (verbosely && cansee(omx, omy))
/*JP
        pline("%s drops %s.", Monnam(mon), distant_name(obj, doname));
*/
        pline("%sは%sを置いた．", Monnam(mon), distant_name(obj, doname));
/*JP
    if (!flooreffects(obj, omx, omy, "fall")) {
*/
    if (!flooreffects(obj, omx, omy, "落ちる")) {
        place_object(obj, omx, omy);
        stackobj(obj);
    }
    /* do this last, after placing obj on floor; removing steed's saddle
       throws rider, possibly inflicting fatal damage and producing bones */
    if (update_mon)
        update_mon_intrinsics(mon, obj, FALSE, TRUE);
}

/* some monsters bypass the normal rules for moving between levels or
   even leaving the game entirely; when that happens, prevent them from
   taking the Amulet or invocation tools with them */
void
mdrop_special_objs(mon)
struct monst *mon;
{
    struct obj *obj, *otmp;

    for (obj = mon->minvent; obj; obj = otmp) {
        otmp = obj->nobj;
        /* the Amulet, invocation tools, and Rider corpses resist even when
           artifacts and ordinary objects are given 0% resistance chance */
        if (obj_resists(obj, 0, 0)) {
            obj_extract_self(obj);
            mdrop_obj(mon, obj, FALSE);
        }
    }
}

/* release the objects the creature is carrying */
void
relobj(mtmp, show, is_pet)
struct monst *mtmp;
int show;
boolean is_pet; /* If true, pet should keep wielded/worn items */
{
    struct obj *otmp;
    int omx = mtmp->mx, omy = mtmp->my;

    /* vault guard's gold goes away rather than be dropped... */
    if (mtmp->isgd && (otmp = findgold(mtmp->minvent)) != 0) {
        if (canspotmon(mtmp))
#if 0 /*JP*/
            pline("%s gold %s.", s_suffix(Monnam(mtmp)),
                  canseemon(mtmp) ? "vanishes" : "seems to vanish");
#else
            pline("%sの金は消えた%s．", Monnam(mtmp),
                  canseemon(mtmp) ? "" : "ようだ");
#endif
        obj_extract_self(otmp);
        obfree(otmp, (struct obj *) 0);
    } /* isgd && has gold */

    while ((otmp = (is_pet ? droppables(mtmp) : mtmp->minvent)) != 0) {
        obj_extract_self(otmp);
        mdrop_obj(mtmp, otmp, is_pet && flags.verbose);
    }

    if (show && cansee(omx, omy))
        newsym(omx, omy);
}

/*steal.c*/
