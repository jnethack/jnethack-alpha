/* NetHack 3.6	pray.c	$NHDT-Date: 1446854232 2015/11/06 23:57:12 $  $NHDT-Branch: master $:$NHDT-Revision: 1.87 $ */
/* Copyright (c) Benson I. Margulies, Mike Stephenson, Steve Linhart, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2016            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_PTR int NDECL(prayer_done);
STATIC_DCL struct obj *NDECL(worst_cursed_item);
STATIC_DCL int NDECL(in_trouble);
STATIC_DCL void FDECL(fix_worst_trouble, (int));
STATIC_DCL void FDECL(angrygods, (ALIGNTYP_P));
STATIC_DCL void FDECL(at_your_feet, (const char *));
STATIC_DCL void NDECL(gcrownu);
STATIC_DCL void FDECL(pleased, (ALIGNTYP_P));
STATIC_DCL void FDECL(godvoice, (ALIGNTYP_P, const char *));
STATIC_DCL void FDECL(god_zaps_you, (ALIGNTYP_P));
STATIC_DCL void FDECL(fry_by_god, (ALIGNTYP_P, BOOLEAN_P));
STATIC_DCL void FDECL(gods_angry, (ALIGNTYP_P));
STATIC_DCL void FDECL(gods_upset, (ALIGNTYP_P));
STATIC_DCL void FDECL(consume_offering, (struct obj *));
STATIC_DCL boolean FDECL(water_prayer, (BOOLEAN_P));
STATIC_DCL boolean FDECL(blocked_boulder, (int, int));

/* simplify a few tests */
#define Cursed_obj(obj, typ) ((obj) && (obj)->otyp == (typ) && (obj)->cursed)

/*
 * Logic behind deities and altars and such:
 * + prayers are made to your god if not on an altar, and to the altar's god
 *   if you are on an altar
 * + If possible, your god answers all prayers, which is why bad things happen
 *   if you try to pray on another god's altar
 * + sacrifices work basically the same way, but the other god may decide to
 *   accept your allegiance, after which they are your god.  If rejected,
 *   your god takes over with your punishment.
 * + if you're in Gehennom, all messages come from Moloch
 */

/*
 *      Moloch, who dwells in Gehennom, is the "renegade" cruel god
 *      responsible for the theft of the Amulet from Marduk, the Creator.
 *      Moloch is unaligned.
 */
/*JP
static const char *Moloch = "Moloch";
*/
static const char *Moloch = "モーロック";

static const char *godvoices[] = {
/*JP
    "booms out", "thunders", "rings out", "booms",
*/
    "響きわたった", "雷のように響いた", "とどろいた", "響いた",
};

/* values calculated when prayer starts, and used when completed */
static aligntyp p_aligntyp;
static int p_trouble;
static int p_type; /* (-1)-3: (-1)=really naughty, 3=really good */

#define PIOUS 20
#define DEVOUT 14
#define FERVENT 9
#define STRIDENT 4

/*
 * The actual trouble priority is determined by the order of the
 * checks performed in in_trouble() rather than by these numeric
 * values, so keep that code and these values synchronized in
 * order to have the values be meaningful.
 */

#define TROUBLE_STONED 14
#define TROUBLE_SLIMED 13
#define TROUBLE_STRANGLED 12
#define TROUBLE_LAVA 11
#define TROUBLE_SICK 10
#define TROUBLE_STARVING 9
#define TROUBLE_REGION 8 /* stinking cloud */
#define TROUBLE_HIT 7
#define TROUBLE_LYCANTHROPE 6
#define TROUBLE_COLLAPSING 5
#define TROUBLE_STUCK_IN_WALL 4
#define TROUBLE_CURSED_LEVITATION 3
#define TROUBLE_UNUSEABLE_HANDS 2
#define TROUBLE_CURSED_BLINDFOLD 1

#define TROUBLE_PUNISHED (-1)
#define TROUBLE_FUMBLING (-2)
#define TROUBLE_CURSED_ITEMS (-3)
#define TROUBLE_SADDLE (-4)
#define TROUBLE_BLIND (-5)
#define TROUBLE_POISONED (-6)
#define TROUBLE_WOUNDED_LEGS (-7)
#define TROUBLE_HUNGRY (-8)
#define TROUBLE_STUNNED (-9)
#define TROUBLE_CONFUSED (-10)
#define TROUBLE_HALLUCINATION (-11)


#define ugod_is_angry() (u.ualign.record < 0)
#define on_altar() IS_ALTAR(levl[u.ux][u.uy].typ)
#define on_shrine() ((levl[u.ux][u.uy].altarmask & AM_SHRINE) != 0)
#define a_align(x, y) ((aligntyp) Amask2align(levl[x][y].altarmask & AM_MASK))

/* critically low hit points if hp <= 5 or hp <= maxhp/N for some N */
boolean
critically_low_hp(only_if_injured)
boolean only_if_injured; /* determines whether maxhp <= 5 matters */
{
    int divisor, hplim, curhp = Upolyd ? u.mh : u.uhp,
                        maxhp = Upolyd ? u.mhmax : u.uhpmax;

    if (only_if_injured && !(curhp < maxhp))
        return FALSE;
    /* if maxhp is extremely high, use lower threshold for the division test
       (golden glow cuts off at 11+5*lvl, nurse interaction at 25*lvl; this
       ought to use monster hit dice--and a smaller multiplier--rather than
       ulevel when polymorphed, but polyself doesn't maintain that) */
    hplim = 15 * u.ulevel;
    if (maxhp > hplim)
        maxhp = hplim;
    /* 7 used to be the unconditional divisor */
    switch (xlev_to_rank(u.ulevel)) { /* maps 1..30 into 0..8 */
    case 0:
    case 1:
        divisor = 5;
        break; /* explvl 1 to 5 */
    case 2:
    case 3:
        divisor = 6;
        break; /* explvl 6 to 13 */
    case 4:
    case 5:
        divisor = 7;
        break; /* explvl 14 to 21 */
    case 6:
    case 7:
        divisor = 8;
        break; /* explvl 22 to 29 */
    default:
        divisor = 9;
        break; /* explvl 30+ */
    }
    /* 5 is a magic number in TROUBLE_HIT handling below */
    return (boolean) (curhp <= 5 || curhp * divisor <= maxhp);
}

/*
 * Return 0 if nothing particular seems wrong, positive numbers for
 * serious trouble, and negative numbers for comparative annoyances.
 * This returns the worst problem. There may be others, and the gods
 * may fix more than one.
 *
 * This could get as bizarre as noting surrounding opponents, (or
 * hostile dogs), but that's really hard.
 *
 * We could force rehumanize of polyselfed people, but we can't tell
 * unintentional shape changes from the other kind. Oh well.
 * 3.4.2: make an exception if polymorphed into a form which lacks
 * hands; that's a case where the ramifications override this doubt.
 */
STATIC_OVL int
in_trouble()
{
    struct obj *otmp;
    int i, j, count = 0;

    /*
     * major troubles
     */
    if (Stoned)
        return TROUBLE_STONED;
    if (Slimed)
        return TROUBLE_SLIMED;
    if (Strangled)
        return TROUBLE_STRANGLED;
    if (u.utrap && u.utraptype == TT_LAVA)
        return TROUBLE_LAVA;
    if (Sick)
        return TROUBLE_SICK;
    if (u.uhs >= WEAK)
        return TROUBLE_STARVING;
    if (region_danger())
        return TROUBLE_REGION;
    if (critically_low_hp(FALSE))
        return TROUBLE_HIT;
    if (u.ulycn >= LOW_PM)
        return TROUBLE_LYCANTHROPE;
    if (near_capacity() >= EXT_ENCUMBER && AMAX(A_STR) - ABASE(A_STR) > 3)
        return TROUBLE_COLLAPSING;

    for (i = -1; i <= 1; i++)
        for (j = -1; j <= 1; j++) {
            if (!i && !j)
                continue;
            if (!isok(u.ux + i, u.uy + j)
                || IS_ROCK(levl[u.ux + i][u.uy + j].typ)
                || (blocked_boulder(i, j) && !throws_rocks(youmonst.data)))
                count++;
        }
    if (count == 8 && !Passes_walls)
        return TROUBLE_STUCK_IN_WALL;

    if (Cursed_obj(uarmf, LEVITATION_BOOTS)
        || stuck_ring(uleft, RIN_LEVITATION)
        || stuck_ring(uright, RIN_LEVITATION))
        return TROUBLE_CURSED_LEVITATION;
    if (nohands(youmonst.data) || !freehand()) {
        /* for bag/box access [cf use_container()]...
           make sure it's a case that we know how to handle;
           otherwise "fix all troubles" would get stuck in a loop */
        if (welded(uwep))
            return TROUBLE_UNUSEABLE_HANDS;
        if (Upolyd && nohands(youmonst.data)
            && (!Unchanging || ((otmp = unchanger()) != 0 && otmp->cursed)))
            return TROUBLE_UNUSEABLE_HANDS;
    }
    if (Blindfolded && ublindf->cursed)
        return TROUBLE_CURSED_BLINDFOLD;

    /*
     * minor troubles
     */
    if (Punished || (u.utrap && u.utraptype == TT_BURIEDBALL))
        return TROUBLE_PUNISHED;
    if (Cursed_obj(uarmg, GAUNTLETS_OF_FUMBLING)
        || Cursed_obj(uarmf, FUMBLE_BOOTS))
        return TROUBLE_FUMBLING;
    if (worst_cursed_item())
        return TROUBLE_CURSED_ITEMS;
    if (u.usteed) { /* can't voluntarily dismount from a cursed saddle */
        otmp = which_armor(u.usteed, W_SADDLE);
        if (Cursed_obj(otmp, SADDLE))
            return TROUBLE_SADDLE;
    }

    if (Blinded > 1 && haseyes(youmonst.data)
        && (!u.uswallow
            || !attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND)))
        return TROUBLE_BLIND;
    for (i = 0; i < A_MAX; i++)
        if (ABASE(i) < AMAX(i))
            return TROUBLE_POISONED;
    if (Wounded_legs && !u.usteed)
        return TROUBLE_WOUNDED_LEGS;
    if (u.uhs >= HUNGRY)
        return TROUBLE_HUNGRY;
    if (HStun & TIMEOUT)
        return TROUBLE_STUNNED;
    if (HConfusion & TIMEOUT)
        return TROUBLE_CONFUSED;
    if (HHallucination & TIMEOUT)
        return TROUBLE_HALLUCINATION;
    return 0;
}

/* select an item for TROUBLE_CURSED_ITEMS */
STATIC_OVL struct obj *
worst_cursed_item()
{
    register struct obj *otmp;

    /* if strained or worse, check for loadstone first */
    if (near_capacity() >= HVY_ENCUMBER) {
        for (otmp = invent; otmp; otmp = otmp->nobj)
            if (Cursed_obj(otmp, LOADSTONE))
                return otmp;
    }
    /* weapon takes precedence if it is interfering
       with taking off a ring or putting on a shield */
    if (welded(uwep) && (uright || bimanual(uwep))) { /* weapon */
        otmp = uwep;
        /* gloves come next, due to rings */
    } else if (uarmg && uarmg->cursed) { /* gloves */
        otmp = uarmg;
        /* then shield due to two handed weapons and spells */
    } else if (uarms && uarms->cursed) { /* shield */
        otmp = uarms;
        /* then cloak due to body armor */
    } else if (uarmc && uarmc->cursed) { /* cloak */
        otmp = uarmc;
    } else if (uarm && uarm->cursed) { /* suit */
        otmp = uarm;
    } else if (uarmh && uarmh->cursed) { /* helmet */
        otmp = uarmh;
    } else if (uarmf && uarmf->cursed) { /* boots */
        otmp = uarmf;
    } else if (uarmu && uarmu->cursed) { /* shirt */
        otmp = uarmu;
    } else if (uamul && uamul->cursed) { /* amulet */
        otmp = uamul;
    } else if (uleft && uleft->cursed) { /* left ring */
        otmp = uleft;
    } else if (uright && uright->cursed) { /* right ring */
        otmp = uright;
    } else if (ublindf && ublindf->cursed) { /* eyewear */
        otmp = ublindf;                      /* must be non-blinding lenses */
        /* if weapon wasn't handled above, do it now */
    } else if (welded(uwep)) { /* weapon */
        otmp = uwep;
        /* active secondary weapon even though it isn't welded */
    } else if (uswapwep && uswapwep->cursed && u.twoweap) {
        otmp = uswapwep;
        /* all worn items ought to be handled by now */
    } else {
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (!otmp->cursed)
                continue;
            if (otmp->otyp == LOADSTONE || confers_luck(otmp))
                break;
        }
    }
    return otmp;
}

STATIC_OVL void
fix_worst_trouble(trouble)
int trouble;
{
    int i;
    struct obj *otmp = 0;
    const char *what = (const char *) 0;
#if 0 /*JP*/
    static NEARDATA const char leftglow[] = "Your left ring softly glows",
                               rightglow[] = "Your right ring softly glows";
#else
    static NEARDATA const char leftglow[] = "左の指輪",
                               rightglow[] = "右の指輪";
#endif

    switch (trouble) {
    case TROUBLE_STONED:
/*JP
        make_stoned(0L, "You feel more limber.", 0, (char *) 0);
*/
        make_stoned(0L, "軟らかくなったような気がした．", 0, (char *) 0);
        break;
    case TROUBLE_SLIMED:
/*JP
        make_slimed(0L, "The slime disappears.");
*/
        make_slimed(0L, "どろどろ感は消えた．");
        break;
    case TROUBLE_STRANGLED:
        if (uamul && uamul->otyp == AMULET_OF_STRANGULATION) {
/*JP
            Your("amulet vanishes!");
*/
            Your("魔除けは消えさった！");
            useup(uamul);
        }
/*JP
        You("can breathe again.");
*/
        You("また呼吸できるようになった．");
        Strangled = 0;
        context.botl = 1;
        break;
    case TROUBLE_LAVA:
/*JP
        You("are back on solid ground.");
*/
        You("固い地面に戻った．");
        /* teleport should always succeed, but if not,
         * just untrap them.
         */
        if (!safe_teleds(FALSE))
            u.utrap = 0;
        break;
    case TROUBLE_STARVING:
        losestr(-1);
        /*FALLTHRU*/
    case TROUBLE_HUNGRY:
/*JP
        Your("%s feels content.", body_part(STOMACH));
*/
        Your("食欲は満たされた．");
        init_uhunger();
        context.botl = 1;
        break;
    case TROUBLE_SICK:
/*JP
        You_feel("better.");
*/
        You("気分が良くなった．");
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
        break;
    case TROUBLE_REGION:
        /* stinking cloud, with hero vulnerable to HP loss */
        region_safety();
        break;
    case TROUBLE_HIT:
        /* "fix all troubles" will keep trying if hero has
           5 or less hit points, so make sure they're always
           boosted to be more than that */
/*JP
        You_feel("much better.");
*/
        You("とても気分が良くなった．");
        if (Upolyd) {
            u.mhmax += rnd(5);
            if (u.mhmax <= 5)
                u.mhmax = 5 + 1;
            u.mh = u.mhmax;
        }
        if (u.uhpmax < u.ulevel * 5 + 11)
            u.uhpmax += rnd(5);
        if (u.uhpmax <= 5)
            u.uhpmax = 5 + 1;
        u.uhp = u.uhpmax;
        context.botl = 1;
        break;
    case TROUBLE_COLLAPSING:
        /* override Fixed_abil; uncurse that if feasible */
#if 0 /*JP*/
        You_feel("%sstronger.",
                 (AMAX(A_STR) - ABASE(A_STR) > 6) ? "much " : "");
#else
        You_feel("%s強くなったようだ．",
                 (AMAX(A_STR) - ABASE(A_STR) > 6) ? "とても" : "");
#endif
        ABASE(A_STR) = AMAX(A_STR);
        context.botl = 1;
        if (Fixed_abil) {
            if ((otmp = stuck_ring(uleft, RIN_SUSTAIN_ABILITY)) != 0) {
                if (otmp == uleft)
                    what = leftglow;
            } else if ((otmp = stuck_ring(uright, RIN_SUSTAIN_ABILITY))
                       != 0) {
                if (otmp == uright)
                    what = rightglow;
            }
            if (otmp)
                goto decurse;
        }
        break;
    case TROUBLE_STUCK_IN_WALL:
/*JP
        Your("surroundings change.");
*/
        Your("環境が変化した．");
        /* no control, but works on no-teleport levels */
        (void) safe_teleds(FALSE);
        break;
    case TROUBLE_CURSED_LEVITATION:
        if (Cursed_obj(uarmf, LEVITATION_BOOTS)) {
            otmp = uarmf;
        } else if ((otmp = stuck_ring(uleft, RIN_LEVITATION)) != 0) {
            if (otmp == uleft)
                what = leftglow;
        } else if ((otmp = stuck_ring(uright, RIN_LEVITATION)) != 0) {
            if (otmp == uright)
                what = rightglow;
        }
        goto decurse;
    case TROUBLE_UNUSEABLE_HANDS:
        if (welded(uwep)) {
            otmp = uwep;
            goto decurse;
        }
        if (Upolyd && nohands(youmonst.data)) {
            if (!Unchanging) {
/*JP
                Your("shape becomes uncertain.");
*/
                Your("体型は不明確になった．");
                rehumanize(); /* "You return to {normal} form." */
            } else if ((otmp = unchanger()) != 0 && otmp->cursed) {
                /* otmp is an amulet of unchanging */
                goto decurse;
            }
        }
        if (nohands(youmonst.data) || !freehand())
            impossible("fix_worst_trouble: couldn't cure hands.");
        break;
    case TROUBLE_CURSED_BLINDFOLD:
        otmp = ublindf;
        goto decurse;
    case TROUBLE_LYCANTHROPE:
        you_unwere(TRUE);
        break;
    /*
     */
    case TROUBLE_PUNISHED:
/*JP
        Your("chain disappears.");
*/
        Your("鎖は消えた．");
        if (u.utrap && u.utraptype == TT_BURIEDBALL)
            buried_ball_to_freedom();
        else
            unpunish();
        break;
    case TROUBLE_FUMBLING:
        if (Cursed_obj(uarmg, GAUNTLETS_OF_FUMBLING))
            otmp = uarmg;
        else if (Cursed_obj(uarmf, FUMBLE_BOOTS))
            otmp = uarmf;
        goto decurse;
        /*NOTREACHED*/
        break;
    case TROUBLE_CURSED_ITEMS:
        otmp = worst_cursed_item();
        if (otmp == uright)
            what = rightglow;
        else if (otmp == uleft)
            what = leftglow;
    decurse:
        if (!otmp) {
            impossible("fix_worst_trouble: nothing to uncurse.");
            return;
        }
        if (!Blind || (otmp == ublindf && Blindfolded_only)) {
#if 0 /*JP*/
            pline("%s %s.",
                  what ? what : (const char *) Yobjnam2(otmp, "softly glow"),
                  hcolor(NH_AMBER));
#else
            Your("%sは%sやわらかく輝いた．",
                 what ? what : (const char *)xname(otmp),
                 jconj_adj(hcolor(NH_AMBER)));
#endif
            iflags.last_msg = PLNMSG_OBJ_GLOWS;
            otmp->bknown = TRUE;
        }
        uncurse(otmp);
        update_inventory();
        break;
    case TROUBLE_POISONED:
        /* override Fixed_abil; ignore items which confer that */
        if (Hallucination)
/*JP
            pline("There's a tiger in your tank.");
*/
            pline("あなたのタンクの中にトラがいる．");
        else
/*JP
            You_feel("in good health again.");
*/
            You("また健康になったような気がした．");
        for (i = 0; i < A_MAX; i++) {
            if (ABASE(i) < AMAX(i)) {
                ABASE(i) = AMAX(i);
                context.botl = 1;
            }
        }
        (void) encumber_msg();
        break;
    case TROUBLE_BLIND: {
#if 0 /*JP*/
        const char *eyes = body_part(EYE);

        if (eyecount(youmonst.data) != 1)
            eyes = makeplural(eyes);
        Your("%s %s better.", eyes, vtense(eyes, "feel"));
#else
        pline("%sが回復したような気がした．", body_part(EYE));
#endif
        u.ucreamed = 0;
        make_blinded(0L, FALSE);
        break;
    }
    case TROUBLE_WOUNDED_LEGS:
        heal_legs();
        break;
    case TROUBLE_STUNNED:
        make_stunned(0L, TRUE);
        break;
    case TROUBLE_CONFUSED:
        make_confused(0L, TRUE);
        break;
    case TROUBLE_HALLUCINATION:
/*JP
        pline("Looks like you are back in Kansas.");
*/
        pline("見て！カンザスに戻ってきたんだわ．");
        (void) make_hallucinated(0L, FALSE, 0L);
        break;
    case TROUBLE_SADDLE:
        otmp = which_armor(u.usteed, W_SADDLE);
        if (!Blind) {
/*JP
            pline("%s %s.", Yobjnam2(otmp, "softly glow"), hcolor(NH_AMBER));
*/
            pline("%sは%sやわらかく輝いた．", y_monnam(u.usteed), hcolor(NH_AMBER));
            otmp->bknown = TRUE;
        }
        uncurse(otmp);
        break;
    }
}

/* "I am sometimes shocked by...  the nuns who never take a bath without
 * wearing a bathrobe all the time.  When asked why, since no man can see
 * them,
 * they reply 'Oh, but you forget the good God'.  Apparently they conceive of
 * the Deity as a Peeping Tom, whose omnipotence enables Him to see through
 * bathroom walls, but who is foiled by bathrobes." --Bertrand Russell, 1943
 * Divine wrath, dungeon walls, and armor follow the same principle.
 */
STATIC_OVL void
god_zaps_you(resp_god)
aligntyp resp_god;
{
    if (u.uswallow) {
        pline(
/*JP
          "Suddenly a bolt of lightning comes down at you from the heavens!");
*/
          "突然空から稲妻が落ちてきた！");
/*JP
        pline("It strikes %s!", mon_nam(u.ustuck));
*/
        pline("稲妻は%sに命中した！", mon_nam(u.ustuck));
        if (!resists_elec(u.ustuck)) {
/*JP
            pline("%s fries to a crisp!", Monnam(u.ustuck));
*/
            pline("%sはパリパリになった！", Monnam(u.ustuck));
            /* Yup, you get experience.  It takes guts to successfully
             * pull off this trick on your god, anyway.
             */
            xkilled(u.ustuck, 0);
        } else
/*JP
            pline("%s seems unaffected.", Monnam(u.ustuck));
*/
            pline("%sは影響を受けないようだ．", Monnam(u.ustuck));
    } else {
/*JP
        pline("Suddenly, a bolt of lightning strikes you!");
*/
        pline("突然，稲妻があなたに命中した！");
        if (Reflecting) {
            shieldeff(u.ux, u.uy);
            if (Blind)
/*JP
                pline("For some reason you're unaffected.");
*/
                pline("なぜかあなたは影響を受けない．");
            else
/*JP
                (void) ureflects("%s reflects from your %s.", "It");
*/
                (void) ureflects("%sは%sによって反射された．", "何か");
        } else if (Shock_resistance) {
            shieldeff(u.ux, u.uy);
/*JP
            pline("It seems not to affect you.");
*/
            pline("稲妻は影響を与えないようだ．");
        } else
            fry_by_god(resp_god, FALSE);
    }

/*JP
    pline("%s is not deterred...", align_gname(resp_god));
*/
    pline("%sはあきらめなかった．．．", align_gname(resp_god));
    if (u.uswallow) {
/*JP
        pline("A wide-angle disintegration beam aimed at you hits %s!",
*/
        pline("あなたを狙った広角粉砕光線が%sに命中した！",
              mon_nam(u.ustuck));
        if (!resists_disint(u.ustuck)) {
/*JP
            pline("%s disintegrates into a pile of dust!", Monnam(u.ustuck));
*/
            pline("%sはちりの山になった！", Monnam(u.ustuck));
            xkilled(u.ustuck, 2); /* no corpse */
        } else
/*JP
            pline("%s seems unaffected.", Monnam(u.ustuck));
*/
            pline("%sは影響を受けないようだ．", Monnam(u.ustuck));
    } else {
/*JP
        pline("A wide-angle disintegration beam hits you!");
*/
        pline("広角粉砕光線があなたに命中した！");

        /* disintegrate shield and body armor before disintegrating
         * the impudent mortal, like black dragon breath -3.
         */
        if (uarms && !(EReflecting & W_ARMS)
            && !(EDisint_resistance & W_ARMS))
            (void) destroy_arm(uarms);
        if (uarmc && !(EReflecting & W_ARMC)
            && !(EDisint_resistance & W_ARMC))
            (void) destroy_arm(uarmc);
        if (uarm && !(EReflecting & W_ARM) && !(EDisint_resistance & W_ARM)
            && !uarmc)
            (void) destroy_arm(uarm);
        if (uarmu && !uarm && !uarmc)
            (void) destroy_arm(uarmu);
        if (!Disint_resistance)
            fry_by_god(resp_god, TRUE);
        else {
/*JP
            You("bask in its %s glow for a minute...", NH_BLACK);
*/
            You("しばらく，その%s輝きで暖まった．．．", NH_BLACK);
/*JP
            godvoice(resp_god, "I believe it not!");
*/
            godvoice(resp_god, "信じられぬ！");
        }
        if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) {
            /* one more try for high altars */
/*JP
            verbalize("Thou cannot escape my wrath, mortal!");
*/
            verbalize("定命の者よ，汝我が怒りから逃がれることならん！");
            summon_minion(resp_god, FALSE);
            summon_minion(resp_god, FALSE);
            summon_minion(resp_god, FALSE);
/*JP
            verbalize("Destroy %s, my servants!", uhim());
*/
            verbalize("%sを殺せ，わが下僕よ！", uhim());
        }
    }
}

STATIC_OVL void
fry_by_god(resp_god, via_disintegration)
aligntyp resp_god;
boolean via_disintegration;
{
#if 0 /*JP*/
    You("%s!", !via_disintegration ? "fry to a crisp"
                                   : "disintegrate into a pile of dust");
#else
    You("%s！", !via_disintegration ? "パリパリになった"
                                    : "ちりの山になった");
#endif
    killer.format = KILLED_BY;
/*JP
    Sprintf(killer.name, "the wrath of %s", align_gname(resp_god));
*/
    Sprintf(killer.name, "%sの怒りに触れ", align_gname(resp_god));
    done(DIED);
}

STATIC_OVL void
angrygods(resp_god)
aligntyp resp_god;
{
    int maxanger;

    if (Inhell)
        resp_god = A_NONE;
    u.ublessed = 0;

    /* changed from tmp = u.ugangr + abs (u.uluck) -- rph */
    /* added test for alignment diff -dlc */
    if (resp_god != u.ualign.type)
        maxanger = u.ualign.record / 2 + (Luck > 0 ? -Luck / 3 : -Luck);
    else
        maxanger = 3 * u.ugangr + ((Luck > 0 || u.ualign.record >= STRIDENT)
                                   ? -Luck / 3
                                   : -Luck);
    if (maxanger < 1)
        maxanger = 1; /* possible if bad align & good luck */
    else if (maxanger > 15)
        maxanger = 15; /* be reasonable */

    switch (rn2(maxanger)) {
    case 0:
    case 1:
#if 0 /*JP*/
        You_feel("that %s is %s.", align_gname(resp_god),
                 Hallucination ? "bummed" : "displeased");
#else
        You("%sが%sいるような気がした．", align_gname(resp_god),
            Hallucination ? "ねだって" : "立腹して");
#endif
        break;
    case 2:
    case 3:
        godvoice(resp_god, (char *) 0);
#if 0 /*JP*/
        pline("\"Thou %s, %s.\"",
              (ugod_is_angry() && resp_god == u.ualign.type)
                  ? "hast strayed from the path"
                  : "art arrogant",
              youmonst.data->mlet == S_HUMAN ? "mortal" : "creature");
#else
        pline("「汝%s，%sよ．」",
              (ugod_is_angry() && resp_god == u.ualign.type)
              ? "その道から踏み出ておる"
              : "傲慢なり",
              youmonst.data->mlet == S_HUMAN ? "定命のもの" : "生物");
#endif
/*JP
        verbalize("Thou must relearn thy lessons!");
*/
        verbalize("汝いま一度学ぶべし！");
        (void) adjattrib(A_WIS, -1, FALSE);
        losexp((char *) 0);
        break;
    case 6:
        if (!Punished) {
            gods_angry(resp_god);
            punish((struct obj *) 0);
            break;
        } /* else fall thru */
    case 4:
    case 5:
        gods_angry(resp_god);
        if (!Blind && !Antimagic)
/*JP
            pline("%s glow surrounds you.", An(hcolor(NH_BLACK)));
*/
            pline("%s光があなたを取り巻いた．", An(hcolor(NH_BLACK)));
        rndcurse();
        break;
    case 7:
    case 8:
        godvoice(resp_god, (char *) 0);
#if 0 /*JP*/
        verbalize("Thou durst %s me?",
                  (on_altar() && (a_align(u.ux, u.uy) != resp_god))
                      ? "scorn"
                      : "call upon");
#else
        verbalize("汝，我%s？",
                  (on_altar() && (a_align(u.ux,u.uy) != resp_god))
                  ? "をさげすみしか？"
                  : "に祈りを求めしか？");
#endif
#if 0 /*JP*/
        pline("\"Then die, %s!\"",
              youmonst.data->mlet == S_HUMAN ? "mortal" : "creature");
#else
        pline("「死ね，%sよ！」",
              youmonst.data->mlet == S_HUMAN ? "定命のもの" : "生物");
#endif
        summon_minion(resp_god, FALSE);
        break;

    default:
        gods_angry(resp_god);
        god_zaps_you(resp_god);
        break;
    }
    u.ublesscnt = rnz(300);
    return;
}

/* helper to print "str appears at your feet", or appropriate */
static void
at_your_feet(str)
const char *str;
{
    if (Blind)
        str = Something;
    if (u.uswallow) {
        /* barrier between you and the floor */
#if 0 /*JP*/
        pline("%s %s into %s %s.", str, vtense(str, "drop"),
              s_suffix(mon_nam(u.ustuck)), mbodypart(u.ustuck, STOMACH));
#else
        pline("%sが%sの%sに落ちた．", str,
              mon_nam(u.ustuck), mbodypart(u.ustuck, STOMACH));
#endif
    } else {
#if 0 /*JP*/
        pline("%s %s %s your %s!", str,
              Blind ? "lands" : vtense(str, "appear"),
              Levitation ? "beneath" : "at", makeplural(body_part(FOOT)));
#else
        pline("%sがあなたの%sに%s！", str,
              Levitation ? "下方" : "足元",
              Blind ? "着地した" : "現われた");
#endif
    }
}

STATIC_OVL void
gcrownu()
{
    struct obj *obj;
    boolean already_exists, in_hand;
    short class_gift;
    int sp_no;
#define ok_wep(o) ((o) && ((o)->oclass == WEAPON_CLASS || is_weptool(o)))

    HSee_invisible |= FROMOUTSIDE;
    HFire_resistance |= FROMOUTSIDE;
    HCold_resistance |= FROMOUTSIDE;
    HShock_resistance |= FROMOUTSIDE;
    HSleep_resistance |= FROMOUTSIDE;
    HPoison_resistance |= FROMOUTSIDE;
    godvoice(u.ualign.type, (char *) 0);

    obj = ok_wep(uwep) ? uwep : 0;
    already_exists = in_hand = FALSE; /* lint suppression */
    switch (u.ualign.type) {
    case A_LAWFUL:
        u.uevent.uhand_of_elbereth = 1;
/*JP
        verbalize("I crown thee...  The Hand of Elbereth!");
*/
        verbalize("汝に．．．エルベレスの御手の栄誉をさずけよう！");
        break;
    case A_NEUTRAL:
        u.uevent.uhand_of_elbereth = 2;
        in_hand = (uwep && uwep->oartifact == ART_VORPAL_BLADE);
        already_exists =
            exist_artifact(LONG_SWORD, artiname(ART_VORPAL_BLADE));
/*JP
        verbalize("Thou shalt be my Envoy of Balance!");
*/
        verbalize("汝，我が調和の使者なり！");
        break;
    case A_CHAOTIC:
        u.uevent.uhand_of_elbereth = 3;
        in_hand = (uwep && uwep->oartifact == ART_STORMBRINGER);
        already_exists =
            exist_artifact(RUNESWORD, artiname(ART_STORMBRINGER));
#if 0 /*JP*/
        verbalize("Thou art chosen to %s for My Glory!",
                  already_exists && !in_hand ? "take lives" : "steal souls");
#else
        verbalize("汝，我が栄光のため%s者として選ばれん！",
                  already_exists && !in_hand ?
                              "生きながらえん" : "魂を奪いしためる");
#endif
        break;
    }

    class_gift = STRANGE_OBJECT;
    /* 3.3.[01] had this in the A_NEUTRAL case below,
       preventing chaotic wizards from receiving a spellbook */
    if (Role_if(PM_WIZARD)
        && (!uwep || (uwep->oartifact != ART_VORPAL_BLADE
                      && uwep->oartifact != ART_STORMBRINGER))
        && !carrying(SPE_FINGER_OF_DEATH)) {
        class_gift = SPE_FINGER_OF_DEATH;
    make_splbk:
        obj = mksobj(class_gift, TRUE, FALSE);
        bless(obj);
        obj->bknown = TRUE;
/*JP
        at_your_feet("A spellbook");
*/
        at_your_feet("魔法書");
        dropy(obj);
        u.ugifts++;
        /* when getting a new book for known spell, enhance
           currently wielded weapon rather than the book */
        for (sp_no = 0; sp_no < MAXSPELL; sp_no++)
            if (spl_book[sp_no].sp_id == class_gift) {
                if (ok_wep(uwep))
                    obj = uwep; /* to be blessed,&c */
                break;
            }
    } else if (Role_if(PM_MONK) && (!uwep || !uwep->oartifact)
               && !carrying(SPE_RESTORE_ABILITY)) {
        /* monks rarely wield a weapon */
        class_gift = SPE_RESTORE_ABILITY;
        goto make_splbk;
    }

    switch (u.ualign.type) {
    case A_LAWFUL:
        if (class_gift != STRANGE_OBJECT) {
            ; /* already got bonus above */
        } else if (obj && obj->otyp == LONG_SWORD && !obj->oartifact) {
            if (!Blind)
/*JP
                Your("sword shines brightly for a moment.");
*/
                Your("剣はしばらくの間明るく輝いた．");
            obj = oname(obj, artiname(ART_EXCALIBUR));
            if (obj && obj->oartifact == ART_EXCALIBUR)
                u.ugifts++;
        }
        /* acquire Excalibur's skill regardless of weapon or gift */
        unrestrict_weapon_skill(P_LONG_SWORD);
        if (obj && obj->oartifact == ART_EXCALIBUR)
            discover_artifact(ART_EXCALIBUR);
        break;
    case A_NEUTRAL:
        if (class_gift != STRANGE_OBJECT) {
            ; /* already got bonus above */
        } else if (obj && in_hand) {
/*JP
            Your("%s goes snicker-snack!", xname(obj));
*/
            Your("%sはサクサクになった！", xname(obj));
            obj->dknown = TRUE;
        } else if (!already_exists) {
            obj = mksobj(LONG_SWORD, FALSE, FALSE);
            obj = oname(obj, artiname(ART_VORPAL_BLADE));
            obj->spe = 1;
/*JP
            at_your_feet("A sword");
*/
            at_your_feet("剣");
            dropy(obj);
            u.ugifts++;
        }
        /* acquire Vorpal Blade's skill regardless of weapon or gift */
        unrestrict_weapon_skill(P_LONG_SWORD);
        if (obj && obj->oartifact == ART_VORPAL_BLADE)
            discover_artifact(ART_VORPAL_BLADE);
        break;
    case A_CHAOTIC: {
        char swordbuf[BUFSZ];

/*JP
        Sprintf(swordbuf, "%s sword", hcolor(NH_BLACK));
*/
        Sprintf(swordbuf, "%s剣", hcolor(NH_BLACK));
        if (class_gift != STRANGE_OBJECT) {
            ; /* already got bonus above */
        } else if (obj && in_hand) {
/*JP
            Your("%s hums ominously!", swordbuf);
*/
            Your("%sは気味の悪い音を立てた！", swordbuf);
            obj->dknown = TRUE;
        } else if (!already_exists) {
            obj = mksobj(RUNESWORD, FALSE, FALSE);
            obj = oname(obj, artiname(ART_STORMBRINGER));
            obj->spe = 1;
            at_your_feet(An(swordbuf));
            dropy(obj);
            u.ugifts++;
        }
        /* acquire Stormbringer's skill regardless of weapon or gift */
        unrestrict_weapon_skill(P_BROAD_SWORD);
        if (obj && obj->oartifact == ART_STORMBRINGER)
            discover_artifact(ART_STORMBRINGER);
        break;
    }
    default:
        obj = 0; /* lint */
        break;
    }

    /* enhance weapon regardless of alignment or artifact status */
    if (ok_wep(obj)) {
        bless(obj);
        obj->oeroded = obj->oeroded2 = 0;
        obj->oerodeproof = TRUE;
        obj->bknown = obj->rknown = TRUE;
        if (obj->spe < 1)
            obj->spe = 1;
        /* acquire skill in this weapon */
        unrestrict_weapon_skill(weapon_type(obj));
    } else if (class_gift == STRANGE_OBJECT) {
        /* opportunity knocked, but there was nobody home... */
/*JP
        You_feel("unworthy.");
*/
        You("価値がないと思った．");
    }
    update_inventory();

    /* lastly, confer an extra skill slot/credit beyond the
       up-to-29 you can get from gaining experience levels */
    add_weapon_skill(1);
    return;
}

STATIC_OVL void
pleased(g_align)
aligntyp g_align;
{
    /* don't use p_trouble, worst trouble may get fixed while praying */
    int trouble = in_trouble(); /* what's your worst difficulty? */
    int pat_on_head = 0, kick_on_butt;

#if 0 /*JP*/
    You_feel("that %s is %s.", align_gname(g_align),
             (u.ualign.record >= DEVOUT)
                 ? Hallucination ? "pleased as punch" : "well-pleased"
                 : (u.ualign.record >= STRIDENT)
                       ? Hallucination ? "ticklish" : "pleased"
                       : Hallucination ? "full" : "satisfied");
#else
        pline("%sが%sような気がした．", align_gname(g_align),
              (u.ualign.record >= DEVOUT)
                  ? Hallucination ? "くそ機嫌いい" : "ご機嫌麗しい"
                  : (u.ualign.record >= STRIDENT)
                        ? Hallucination ? "くすぐったがっている" : "上機嫌である"
                        : Hallucination ? "腹いっぱいである" : "満足している");
#endif

    /* not your deity */
    if (on_altar() && p_aligntyp != u.ualign.type) {
        adjalign(-1);
        return;
    } else if (u.ualign.record < 2 && trouble <= 0)
        adjalign(1);

    /*
     * Depending on your luck & align level, the god you prayed to will:
     *  - fix your worst problem if it's major;
     *  - fix all your major problems;
     *  - fix your worst problem if it's minor;
     *  - fix all of your problems;
     *  - do you a gratuitous favor.
     *
     * If you make it to the the last category, you roll randomly again
     * to see what they do for you.
     *
     * If your luck is at least 0, then you are guaranteed rescued from
     * your worst major problem.
     */
    if (!trouble && u.ualign.record >= DEVOUT) {
        /* if hero was in trouble, but got better, no special favor */
        if (p_trouble == 0)
            pat_on_head = 1;
    } else {
        int action, prayer_luck;
        int tryct = 0;

        /* Negative luck is normally impossible here (can_pray() forces
           prayer failure in that situation), but it's possible for
           Luck to drop during the period of prayer occupation and
           become negative by the time we get here.  [Reported case
           was lawful character whose stinking cloud caused a delayed
           killing of a peaceful human, triggering the "murderer"
           penalty while successful prayer was in progress.  It could
           also happen due to inconvenient timing on Friday 13th, but
           the magnitude there (-1) isn't big enough to cause trouble.]
           We don't bother remembering start-of-prayer luck, just make
           sure it's at least -1 so that Luck+2 is big enough to avoid
           a divide by zero crash when generating a random number.  */
        prayer_luck = max(Luck, -1); /* => (prayer_luck + 2 > 0) */
        action = rn1(prayer_luck + (on_altar() ? 3 + on_shrine() : 2), 1);
        if (!on_altar())
            action = min(action, 3);
        if (u.ualign.record < STRIDENT)
            action = (u.ualign.record > 0 || !rnl(2)) ? 1 : 0;

        switch (min(action, 5)) {
        case 5:
            pat_on_head = 1;
        case 4:
            do
                fix_worst_trouble(trouble);
            while ((trouble = in_trouble()) != 0);
            break;

        case 3:
            fix_worst_trouble(trouble);
        case 2:
            /* arbitrary number of tries */
            while ((trouble = in_trouble()) > 0 && (++tryct < 10))
                fix_worst_trouble(trouble);
            break;

        case 1:
            if (trouble > 0)
                fix_worst_trouble(trouble);
        case 0:
            break; /* your god blows you off, too bad */
        }
    }

    /* note: can't get pat_on_head unless all troubles have just been
       fixed or there were no troubles to begin with; hallucination
       won't be in effect so special handling for it is superfluous */
    if (pat_on_head)
        switch (rn2((Luck + 6) >> 1)) {
        case 0:
            break;
        case 1:
            if (uwep && (welded(uwep) || uwep->oclass == WEAPON_CLASS
                         || is_weptool(uwep))) {
                char repair_buf[BUFSZ];

                *repair_buf = '\0';
                if (uwep->oeroded || uwep->oeroded2)
#if 0 /*JP*/
                    Sprintf(repair_buf, " and %s now as good as new",
                            otense(uwep, "are"));
#else
                    Sprintf(repair_buf, "さらに新品同様になった．");
#endif

                if (uwep->cursed) {
                    if (!Blind) {
#if 0 /*JP*/
                        pline("%s %s%s.", Yobjnam2(uwep, "softly glow"),
                              hcolor(NH_AMBER), repair_buf);
#else
                        Your("%sは%sやわらかく輝いた．%s", xname(uwep), 
                             jconj_adj(hcolor(NH_AMBER)), repair_buf);
#endif
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                    } else
#if 0 /*JP*/
                        You_feel("the power of %s over %s.", u_gname(),
                                 yname(uwep));
#else
                        pline("%sの力が%sに注がれているのを感じた．", u_gname(),
                              xname(uwep));
#endif
                    uncurse(uwep);
                    uwep->bknown = TRUE;
                    *repair_buf = '\0';
                } else if (!uwep->blessed) {
                    if (!Blind) {
#if 0 /*JP*/
                        pline("%s with %s aura%s.",
                              Yobjnam2(uwep, "softly glow"),
                              an(hcolor(NH_LIGHT_BLUE)), repair_buf);
#else
                        Your("%sは%sやわらかなオーラにつつまれた．%s",
                             xname(uwep), 
                             an(hcolor(NH_LIGHT_BLUE)), repair_buf);
#endif
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                    } else
#if 0 /*JP*/
                        You_feel("the blessing of %s over %s.", u_gname(),
                                 yname(uwep));
#else
                        pline("%sの祝福が%sに注がれているのを感じた．", u_gname(),
                              xname(uwep));
#endif
                    bless(uwep);
                    uwep->bknown = TRUE;
                    *repair_buf = '\0';
                }

                /* fix any rust/burn/rot damage, but don't protect
                   against future damage */
                if (uwep->oeroded || uwep->oeroded2) {
                    uwep->oeroded = uwep->oeroded2 = 0;
                    /* only give this message if we didn't just bless
                       or uncurse (which has already given a message) */
                    if (*repair_buf)
#if 0 /*JP*/
                        pline("%s as good as new!",
                              Yobjnam2(uwep, Blind ? "feel" : "look"));
#else
                        Your("%sは新品同様になった%s！",
                             xname(uwep), Blind ? "ような気がする" : "");
#endif
                }
                update_inventory();
            }
            break;
        case 3:
            /* takes 2 hints to get the music to enter the stronghold;
               skip if you've solved it via mastermind or destroyed the
               drawbridge (both set uopened_dbridge) or if you've already
               travelled past the Valley of the Dead (gehennom_entered) */
            if (!u.uevent.uopened_dbridge && !u.uevent.gehennom_entered) {
                if (u.uevent.uheard_tune < 1) {
                    godvoice(g_align, (char *) 0);
#if 0 /*JP*/
                    verbalize("Hark, %s!", youmonst.data->mlet == S_HUMAN
                                               ? "mortal"
                                               : "creature");
#else
                    verbalize("%sよ，聞け！", youmonst.data->mlet == S_HUMAN
                                                  ? "定命の者"
                                                  : "生物");
#endif
                    verbalize(
/*JP
                       "To enter the castle, thou must play the right tune!");
*/
                        "汝城に入らんと欲するならば，正しき調べを奏でるべし！");
                    u.uevent.uheard_tune++;
                    break;
                } else if (u.uevent.uheard_tune < 2) {
/*JP
                    You_hear("a divine music...");
*/
                    You_hear("神の音楽を聞いた．．．");
/*JP
                    pline("It sounds like:  \"%s\".", tune);
*/
                    pline("それは次のように聞こえた:  「%s」", tune);
                    u.uevent.uheard_tune++;
                    break;
                }
            }
        /* Otherwise, falls into next case */
        case 2:
            if (!Blind)
/*JP
                You("are surrounded by %s glow.", an(hcolor(NH_GOLDEN)));
*/
                You("%s輝きにつつまれた．", hcolor(NH_GOLDEN));
            /* if any levels have been lost (and not yet regained),
               treat this effect like blessed full healing */
            if (u.ulevel < u.ulevelmax) {
                u.ulevelmax -= 1; /* see potion.c */
                pluslvl(FALSE);
            } else {
                u.uhpmax += 5;
                if (Upolyd)
                    u.mhmax += 5;
            }
            u.uhp = u.uhpmax;
            if (Upolyd)
                u.mh = u.mhmax;
            ABASE(A_STR) = AMAX(A_STR);
            if (u.uhunger < 900)
                init_uhunger();
            if (u.uluck < 0)
                u.uluck = 0;
            make_blinded(0L, TRUE);
            context.botl = 1;
            break;
        case 4: {
            register struct obj *otmp;
            int any = 0;

            if (Blind)
/*JP
                You_feel("the power of %s.", u_gname());
*/
                You("%sの力を感じた．", u_gname());
            else
/*JP
                You("are surrounded by %s aura.", an(hcolor(NH_LIGHT_BLUE)));
*/
                You("%sオーラにつつまれた．", an(hcolor(NH_LIGHT_BLUE)));
            for (otmp = invent; otmp; otmp = otmp->nobj) {
                if (otmp->cursed) {
                    if (!Blind) {
#if 0 /*JP*/
                        pline("%s %s.", Yobjnam2(otmp, "softly glow"),
                              hcolor(NH_AMBER));
#else
                        Your("%sは%sやわらかく輝いた．", xname(otmp),
                             jconj_adj(hcolor(NH_AMBER)));
#endif
                        iflags.last_msg = PLNMSG_OBJ_GLOWS;
                        otmp->bknown = TRUE;
                        ++any;
                    }
                    uncurse(otmp);
                }
            }
            if (any)
                update_inventory();
            break;
        }
        case 5: {
            static NEARDATA const char msg[] =
/*JP
                "\"and thus I grant thee the gift of %s!\"";
*/
                "「さらに汝に%sをさずけよう！」";

            godvoice(u.ualign.type,
/*JP
                     "Thou hast pleased me with thy progress,");
*/
                     "汝の成長は非常に望ましい，");
            if (!(HTelepat & INTRINSIC)) {
                HTelepat |= FROMOUTSIDE;
/*JP
                pline(msg, "Telepathy");
*/
                pline(msg, "テレパシー");
                if (Blind)
                    see_monsters();
            } else if (!(HFast & INTRINSIC)) {
                HFast |= FROMOUTSIDE;
/*JP
                pline(msg, "Speed");
*/
                pline(msg, "速さ");
            } else if (!(HStealth & INTRINSIC)) {
                HStealth |= FROMOUTSIDE;
/*JP
                pline(msg, "Stealth");
*/
                pline(msg, "忍の力");
            } else {
                if (!(HProtection & INTRINSIC)) {
                    HProtection |= FROMOUTSIDE;
                    if (!u.ublessed)
                        u.ublessed = rn1(3, 2);
                } else
                    u.ublessed++;
/*JP
                pline(msg, "my protection");
*/
                pline(msg, "我が護り");
            }
/*JP
            verbalize("Use it wisely in my name!");
*/
            verbalize("我が名に於いて有効に使うがよい！");
            break;
        }
        case 7:
        case 8:
            if (u.ualign.record >= PIOUS && !u.uevent.uhand_of_elbereth) {
                gcrownu();
                break;
            } /* else FALLTHRU */
        case 6: {
            struct obj *otmp;
            int sp_no, trycnt = u.ulevel + 1;

            /* not yet known spells given preference over already known ones
             */
            /* Also, try to grant a spell for which there is a skill slot */
            otmp = mkobj(SPBOOK_CLASS, TRUE);
            while (--trycnt > 0) {
                if (otmp->otyp != SPE_BLANK_PAPER) {
                    for (sp_no = 0; sp_no < MAXSPELL; sp_no++)
                        if (spl_book[sp_no].sp_id == otmp->otyp)
                            break;
                    if (sp_no == MAXSPELL
                        && !P_RESTRICTED(spell_skilltype(otmp->otyp)))
                        break; /* usable, but not yet known */
                } else {
                    if (!objects[SPE_BLANK_PAPER].oc_name_known
                        || carrying(MAGIC_MARKER))
                        break;
                }
                otmp->otyp = rnd_class(bases[SPBOOK_CLASS], SPE_BLANK_PAPER);
            }
            bless(otmp);
/*JP
            at_your_feet("A spellbook");
*/
            at_your_feet("魔法書");
            place_object(otmp, u.ux, u.uy);
            newsym(u.ux, u.uy);
            break;
        }
        default:
            impossible("Confused deity!");
            break;
        }

    u.ublesscnt = rnz(350);
    kick_on_butt = u.uevent.udemigod ? 1 : 0;
    if (u.uevent.uhand_of_elbereth)
        kick_on_butt++;
    if (kick_on_butt)
        u.ublesscnt += kick_on_butt * rnz(1000);

    return;
}

/* either blesses or curses water on the altar,
 * returns true if it found any water here.
 */
STATIC_OVL boolean
water_prayer(bless_water)
boolean bless_water;
{
    register struct obj *otmp;
    register long changed = 0;
    boolean other = FALSE, bc_known = !(Blind || Hallucination);

    for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
        /* turn water into (un)holy water */
        if (otmp->otyp == POT_WATER
            && (bless_water ? !otmp->blessed : !otmp->cursed)) {
            otmp->blessed = bless_water;
            otmp->cursed = !bless_water;
            otmp->bknown = bc_known;
            changed += otmp->quan;
        } else if (otmp->oclass == POTION_CLASS)
            other = TRUE;
    }
    if (!Blind && changed) {
#if 0 /*JP*/
        pline("%s potion%s on the altar glow%s %s for a moment.",
              ((other && changed > 1L) ? "Some of the"
                                       : (other ? "One of the" : "The")),
              ((other || changed > 1L) ? "s" : ""), (changed > 1L ? "" : "s"),
              (bless_water ? hcolor(NH_LIGHT_BLUE) : hcolor(NH_BLACK)));
#else
        pline("%s祭壇の薬は一瞬%s輝いた．",
              (other && changed > 1L) ? "いくつかの"
                                      : "",
              jconj_adj(bless_water ? hcolor(NH_LIGHT_BLUE) : hcolor(NH_BLACK)));
#endif
    }
    return (boolean) (changed > 0L);
}

STATIC_OVL void
godvoice(g_align, words)
aligntyp g_align;
const char *words;
{
#if 0 /*JP*/
    const char *quot = "";

    if (words)
        quot = "\"";
    else
        words = "";

    pline_The("voice of %s %s: %s%s%s", align_gname(g_align),
              godvoices[rn2(SIZE(godvoices))], quot, words, quot);
#else
    if (words)
        pline("%sの声が%s: 「%s」", align_gname(g_align),
              godvoices[rn2(SIZE(godvoices))], words);
    else
        pline("%sの声が%s：", align_gname(g_align),
              godvoices[rn2(SIZE(godvoices))]);
#endif
}

STATIC_OVL void
gods_angry(g_align)
aligntyp g_align;
{
/*JP
    godvoice(g_align, "Thou hast angered me.");
*/
    godvoice(g_align, "汝，我を怒らしめたり．");
}

/* The g_align god is upset with you. */
STATIC_OVL void
gods_upset(g_align)
aligntyp g_align;
{
    if (g_align == u.ualign.type)
        u.ugangr++;
    else if (u.ugangr)
        u.ugangr--;
    angrygods(g_align);
}

STATIC_OVL void
consume_offering(otmp)
register struct obj *otmp;
{
    if (Hallucination)
        switch (rn2(3)) {
        case 0:
/*JP
            Your("sacrifice sprouts wings and a propeller and roars away!");
*/
            Your("献上物は羽をはやし，プロペラがまわり，飛んでった！");
            break;
        case 1:
/*JP
            Your("sacrifice puffs up, swelling bigger and bigger, and pops!");
*/
            Your("献上物は噴煙をあげ，どんどん膨れ，そしてはじけた！");
            break;
        case 2:
            Your(
/*JP
     "sacrifice collapses into a cloud of dancing particles and fades away!");
*/
     "献上物は細かく砕け，踊り出し，どこかに行ってしまった！");
            break;
        }
    else if (Blind && u.ualign.type == A_LAWFUL)
/*JP
        Your("sacrifice disappears!");
*/
        Your("献上物は消えた！");
    else
#if 0 /*JP*/
        Your("sacrifice is consumed in a %s!",
             u.ualign.type == A_LAWFUL ? "flash of light" : "burst of flame");
#else
        Your("献上物は%s消えさった！",
             u.ualign.type == A_LAWFUL ? "まばゆい光を放ち" : "炎を上げ");
#endif
    if (carried(otmp))
        useup(otmp);
    else
        useupf(otmp, 1L);
    exercise(A_WIS, TRUE);
}

int
dosacrifice()
{
    static NEARDATA const char cloud_of_smoke[] =
/*JP
        "A cloud of %s smoke surrounds you...";
*/
        "%s煙があなたを取り囲んだ．．．";
    register struct obj *otmp;
    int value = 0, pm;
    boolean highaltar;
    aligntyp altaralign = a_align(u.ux, u.uy);

    if (!on_altar() || u.uswallow) {
/*JP
        You("are not standing on an altar.");
*/
        You("祭壇の上に立っていない．");
        return 0;
    }
    highaltar = ((Is_astralevel(&u.uz) || Is_sanctum(&u.uz))
                 && (levl[u.ux][u.uy].altarmask & AM_SHRINE));

    otmp = floorfood("sacrifice", 1);
    if (!otmp)
        return 0;
    /*
     * Was based on nutritional value and aging behavior (< 50 moves).
     * Sacrificing a food ration got you max luck instantly, making the
     * gods as easy to please as an angry dog!
     *
     * Now only accepts corpses, based on the game's evaluation of their
     * toughness.  Human and pet sacrifice, as well as sacrificing unicorns
     * of your alignment, is strongly discouraged.
     */
#define MAXVALUE 24 /* Highest corpse value (besides Wiz) */

    if (otmp->otyp == CORPSE) {
        register struct permonst *ptr = &mons[otmp->corpsenm];
        struct monst *mtmp;
        extern const int monstr[];

        /* KMH, conduct */
        u.uconduct.gnostic++;

        /* you're handling this corpse, even if it was killed upon the altar
         */
        feel_cockatrice(otmp, TRUE);
        if (rider_corpse_revival(otmp, FALSE))
            return 1;

        if (otmp->corpsenm == PM_ACID_BLOB
            || (monstermoves <= peek_at_iced_corpse_age(otmp) + 50)) {
            value = monstr[otmp->corpsenm] + 1;
            if (otmp->oeaten)
                value = eaten_stat(value, otmp);
        }

        if (your_race(ptr)) {
            if (is_demon(youmonst.data)) {
/*JP
                You("find the idea very satisfying.");
*/
                You("その考えは素晴しいと思った．");
                exercise(A_WIS, TRUE);
            } else if (u.ualign.type != A_CHAOTIC) {
/*JP
                pline("You'll regret this infamous offense!");
*/
                pline("汝，この侮辱の行ないを後悔するべし！");
                exercise(A_WIS, FALSE);
            }

            if (highaltar
                && (altaralign != A_CHAOTIC || u.ualign.type != A_CHAOTIC)) {
                goto desecrate_high_altar;
            } else if (altaralign != A_CHAOTIC && altaralign != A_NONE) {
                /* curse the lawful/neutral altar */
/*JP
                pline_The("altar is stained with %s blood.", urace.adj);
*/
                pline("祭壇は%sの血で汚れている．", urace.adj);
                levl[u.ux][u.uy].altarmask = AM_CHAOTIC;
                angry_priest();
            } else {
                struct monst *dmon;
                const char *demonless_msg;

                /* Human sacrifice on a chaotic or unaligned altar */
                /* is equivalent to demon summoning */
                if (altaralign == A_CHAOTIC && u.ualign.type != A_CHAOTIC) {
                    pline(
/*JP
                    "The blood floods the altar, which vanishes in %s cloud!",
*/
                    "血が祭壇からあふれ，祭壇は%s雲となり消えた！",
                          an(hcolor(NH_BLACK)));
                    levl[u.ux][u.uy].typ = ROOM;
                    levl[u.ux][u.uy].altarmask = 0;
                    newsym(u.ux, u.uy);
                    angry_priest();
/*JP
                    demonless_msg = "cloud dissipates";
*/
                    demonless_msg = "雲は消えた．";
                } else {
                    /* either you're chaotic or altar is Moloch's or both */
/*JP
                    pline_The("blood covers the altar!");
*/
                    pline("血が祭壇を覆った！");
                    change_luck(altaralign == A_NONE ? -2 : 2);
/*JP
                    demonless_msg = "blood coagulates";
*/
                    demonless_msg = "血がこびりついた";
                }
                if ((pm = dlord(altaralign)) != NON_PM
                    && (dmon = makemon(&mons[pm], u.ux, u.uy, NO_MM_FLAGS))
                           != 0) {
                    char dbuf[BUFSZ];

                    Strcpy(dbuf, a_monnam(dmon));
/*JP
                    if (!strcmpi(dbuf, "it"))
*/
                    if (!strcmpi(dbuf, "何者か"))
/*JP
                        Strcpy(dbuf, "something dreadful");
*/
                        Strcpy(dbuf, "何か恐ろしいもの");
                    else
                        dmon->mstrategy &= ~STRAT_APPEARMSG;
/*JP
                    You("have summoned %s!", dbuf);
*/
                    You("%sを召喚した！", dbuf);
                    if (sgn(u.ualign.type) == sgn(dmon->data->maligntyp))
                        dmon->mpeaceful = TRUE;
/*JP
                    You("are terrified, and unable to move.");
*/
                    You("恐怖で動けなくなった．");
                    nomul(-3);
                    multi_reason = "being terrified of a demon";
                    nomovemsg = 0;
                } else
/*JP
                    pline_The("%s.", demonless_msg);
*/
                    pline("%s．", demonless_msg);
            }

            if (u.ualign.type != A_CHAOTIC) {
                adjalign(-5);
                u.ugangr += 3;
                (void) adjattrib(A_WIS, -1, TRUE);
                if (!Inhell)
                    angrygods(u.ualign.type);
                change_luck(-5);
            } else
                adjalign(5);
            if (carried(otmp))
                useup(otmp);
            else
                useupf(otmp, 1L);
            return 1;
        } else if (has_omonst(otmp)
                   && (mtmp = get_mtraits(otmp, FALSE)) != 0
                   && mtmp->mtame) {
                /* mtmp is a temporary pointer to a tame monster's attributes,
                 * not a real monster */
/*JP
            pline("So this is how you repay loyalty?");
*/
            pline("それでこれがあなたの忠義に報いるものか？");
            adjalign(-3);
            value = -1;
            HAggravate_monster |= FROMOUTSIDE;
        } else if (is_undead(ptr)) { /* Not demons--no demon corpses */
            if (u.ualign.type != A_CHAOTIC)
                value += 1;
        } else if (is_unicorn(ptr)) {
            int unicalign = sgn(ptr->maligntyp);

            if (unicalign == altaralign) {
                /* When same as altar, always a very bad action.
                 */
#if 0 /*JP*/
                pline("Such an action is an insult to %s!",
                      (unicalign == A_CHAOTIC) ? "chaos"
                         : unicalign ? "law" : "balance");
#else
                pline("そのような行動は『%s』に反する！",
                      (unicalign == A_CHAOTIC) ? "混沌"
                         : unicalign ? "秩序" : "調和");
#endif
                (void) adjattrib(A_WIS, -1, TRUE);
                value = -5;
            } else if (u.ualign.type == altaralign) {
                /* When different from altar, and altar is same as yours,
                 * it's a very good action.
                 */
                if (u.ualign.record < ALIGNLIM)
/*JP
                    You_feel("appropriately %s.", align_str(u.ualign.type));
*/
                    You("%sにふさわしいと感じた．", align_str(u.ualign.type));
                else
/*JP
                    You_feel("you are thoroughly on the right path.");
*/
                    You("完全に正しい道を歩んでいるのを感じた．");
                adjalign(5);
                value += 3;
            } else if (unicalign == u.ualign.type) {
                /* When sacrificing unicorn of your alignment to altar not of
                 * your alignment, your god gets angry and it's a conversion.
                 */
                u.ualign.record = -1;
                value = 1;
            } else {
                /* Otherwise, unicorn's alignment is different from yours
                 * and different from the altar's.  It's an ordinary (well,
                 * with a bonus) sacrifice on a cross-aligned altar.
                 */
                value += 3;
            }
        }
    } /* corpse */

    if (otmp->otyp == AMULET_OF_YENDOR) {
        if (!highaltar) {
        too_soon:
            if (altaralign == A_NONE && Inhell)
                /* hero has left Moloch's Sanctum so is in the process
                   of getting away with the Amulet (outside of Gehennom,
                   fall through to the "ashamed" feedback) */
                gods_upset(A_NONE);
            else
#if 0 /*JP*/
                You_feel("%s.",
                         Hallucination
                            ? "homesick"
                            /* if on track, give a big hint */
                            : (altaralign == u.ualign.type)
                               ? "an urge to return to the surface"
                               /* else headed towards celestial disgrace */
                               : "ashamed");
#else
                You_feel("%s．",
                         Hallucination
                            ? "故郷が恋しくなった"
                            /* if on track, give a big hint */
                            : (altaralign == u.ualign.type)
                               ? "地上に帰りたい気持に駆り立てられた"
                               /* else headed towards celestial disgrace */
                               : "恥ずかしい思いがした");
#endif
            return 1;
        } else {
            /* The final Test.  Did you win? */
            if (uamul == otmp)
                Amulet_off();
            u.uevent.ascended = 1;
            if (carried(otmp))
                useup(otmp); /* well, it's gone now */
            else
                useupf(otmp, 1L);
/*JP
            You("offer the Amulet of Yendor to %s...", a_gname());
*/
            You("イェンダーの魔除けを%sに献上した．．．",a_gname());
            if (altaralign == A_NONE) {
                /* Moloch's high altar */
                if (u.ualign.record > -99)
                    u.ualign.record = -99;
                /*[apparently shrug/snarl can be sensed without being seen]*/
                pline("%s shrugs and retains dominion over %s,", Moloch,
                      u_gname());
                pline("then mercilessly snuffs out your life.");
                Sprintf(killer.name, "%s indifference", s_suffix(Moloch));
                killer.format = KILLED_BY;
                done(DIED);
                /* life-saved (or declined to die in wizard/explore mode) */
                pline("%s snarls and tries again...", Moloch);
                fry_by_god(A_NONE, TRUE); /* wrath of Moloch */
                /* declined to die in wizard or explore mode */
                pline(cloud_of_smoke, hcolor(NH_BLACK));
                done(ESCAPED);
            } else if (u.ualign.type != altaralign) {
                /* And the opposing team picks you up and
                   carries you off on their shoulders */
                adjalign(-99);
#if 0 /*JP*/
                pline("%s accepts your gift, and gains dominion over %s...",
                      a_gname(), u_gname());
#else
                pline("%sはあなたの送り物を受けとり，%sの権力を得た．．．",
                      a_gname(), u_gname());
#endif
/*JP
                pline("%s is enraged...", u_gname());
*/
                pline("%sは激怒した．．．", u_gname());
/*JP
                pline("Fortunately, %s permits you to live...", a_gname());
*/
                pline("幸運にも，%sはあなたの存在を許している．．．",a_gname());
                pline(cloud_of_smoke, hcolor(NH_ORANGE));
                done(ESCAPED);
            } else { /* super big win */
                adjalign(10);
                u.uachieve.ascended = 1;
                pline(
/*JP
               "An invisible choir sings, and you are bathed in radiance...");
*/
                "どこからともなく聖歌隊の歌が聞こえ，あなたは光に包まれた．．．");
/*JP
                godvoice(altaralign, "Congratulations, mortal!");
*/
                godvoice(altaralign, "よくやった！定命の者よ！");
                display_nhwindow(WIN_MESSAGE, FALSE);
                verbalize(
/*JP
          "In return for thy service, I grant thee the gift of Immortality!");
*/
          "汝の偉業に対し，不死の体を捧げようぞ！");
#if 0 /*JP*/
                You("ascend to the status of Demigod%s...",
                    flags.female ? "dess" : "");
#else
                You("昇天し，%s神となった．．．",
                    flags.female ? "女" : "");
#endif
                done(ASCENDED);
            }
        }
    } /* real Amulet */

    if (otmp->otyp == FAKE_AMULET_OF_YENDOR) {
        if (!highaltar && !otmp->known)
            goto too_soon;
/*JP
        You_hear("a nearby thunderclap.");
*/
        You("近くに雷が落ちた音を聞いた．");
        if (!otmp->known) {
#if 0 /*JP*/
            You("realize you have made a %s.",
                Hallucination ? "boo-boo" : "mistake");
#else
            You("%sことに気がついた．",
                Hallucination ? "「ハズレ」だった" : "間違いを犯した");
#endif
            otmp->known = TRUE;
            change_luck(-1);
            return 1;
        } else {
            /* don't you dare try to fool the gods */
            if (Deaf)
#if 0 /*JP*/
                pline("Oh, no."); /* didn't hear thunderclap */
#else
                pline("なんてこった．"); /* didn't hear thunderclap */
#endif
            change_luck(-3);
            adjalign(-1);
            u.ugangr += 3;
            value = -3;
        }
    } /* fake Amulet */

    if (value == 0) {
        pline1(nothing_happens);
        return 1;
    }

    if (altaralign != u.ualign.type && highaltar) {
    desecrate_high_altar:
        /*
         * REAL BAD NEWS!!! High altars cannot be converted.  Even an attempt
         * gets the god who owns it truly pissed off.
         */
/*JP
        You_feel("the air around you grow charged...");
*/
        You("回りの空気にエネルギーが満ちていくような気がした．．．");
/*JP
        pline("Suddenly, you realize that %s has noticed you...", a_gname());
*/
        pline("突然，%sがあなたをじっと見ているのに気がついた．．．",a_gname());
        godvoice(altaralign,
/*JP
                 "So, mortal!  You dare desecrate my High Temple!");
*/
                 "定命の者よ！おまえは我が神聖なる寺院を汚すのか！");
        /* Throw everything we have at the player */
        god_zaps_you(altaralign);
    } else if (value
               < 0) { /* I don't think the gods are gonna like this... */
        gods_upset(altaralign);
    } else {
        int saved_anger = u.ugangr;
        int saved_cnt = u.ublesscnt;
        int saved_luck = u.uluck;

        /* Sacrificing at an altar of a different alignment */
        if (u.ualign.type != altaralign) {
            /* Is this a conversion ? */
            /* An unaligned altar in Gehennom will always elicit rejection. */
            if (ugod_is_angry() || (altaralign == A_NONE && Inhell)) {
                if (u.ualignbase[A_CURRENT] == u.ualignbase[A_ORIGINAL]
                    && altaralign != A_NONE) {
/*JP
                    You("have a strong feeling that %s is angry...",
*/
                    You("%sが怒っているのを確信した．．．",
                        u_gname());
                    consume_offering(otmp);
/*JP
                    pline("%s accepts your allegiance.", a_gname());
*/
                    pline("%sはあなたの属性を受けいれた．", a_gname());

                    uchangealign(altaralign, 0);
                    /* Beware, Conversion is costly */
                    change_luck(-3);
                    u.ublesscnt += 300;
                } else {
                    u.ugangr += 3;
                    adjalign(-5);
/*JP
                    pline("%s rejects your sacrifice!", a_gname());
*/
                    pline("%sはあなたの献上物を受けいれない！", a_gname());
/*JP
                    godvoice(altaralign, "Suffer, infidel!");
*/
                    godvoice(altaralign, "異端者よ！失せろ！！");
                    change_luck(-5);
                    (void) adjattrib(A_WIS, -2, TRUE);
                    if (!Inhell)
                        angrygods(u.ualign.type);
                }
                return 1;
            } else {
                consume_offering(otmp);
#if 0 /*JP*/
                You("sense a conflict between %s and %s.", u_gname(),
                    a_gname());
#else
                You("%sと%s間の争いを感じた．", u_gname(),
                    a_gname());
#endif
                if (rn2(8 + u.ulevel) > 5) {
                    struct monst *pri;
/*JP
                    You_feel("the power of %s increase.", u_gname());
*/
                    You("%sの力が増大したような気がした．", u_gname());
                    exercise(A_WIS, TRUE);
                    change_luck(1);
                    /* Yes, this is supposed to be &=, not |= */
                    levl[u.ux][u.uy].altarmask &= AM_SHRINE;
                    /* the following accommodates stupid compilers */
                    levl[u.ux][u.uy].altarmask =
                        levl[u.ux][u.uy].altarmask
                        | (Align2amask(u.ualign.type));
                    if (!Blind)
#if 0 /*JP*/
                        pline_The("altar glows %s.",
                                  hcolor((u.ualign.type == A_LAWFUL)
                                            ? NH_WHITE
                                            : u.ualign.type
                                               ? NH_BLACK
                                               : (const char *) "gray"));
#else
                        pline("祭壇は%s輝いた．",
                              jconj_adj(hcolor((u.ualign.type == A_LAWFUL)
                                            ? NH_WHITE
                                            : u.ualign.type
                                               ? NH_BLACK
                                               : (const char *)"灰色の")));
#endif

                    if (rnl(u.ulevel) > 6 && u.ualign.record > 0
                        && rnd(u.ualign.record) > (3 * ALIGNLIM) / 4)
                        summon_minion(altaralign, TRUE);
                    /* anger priest; test handles bones files */
                    if ((pri = findpriest(temple_occupied(u.urooms)))
                        && !p_coaligned(pri))
                        angry_priest();
                } else {
/*JP
                    pline("Unluckily, you feel the power of %s decrease.",
*/
                    pline("不幸にも，%sの力が減少したのを感じた．",
                          u_gname());
                    change_luck(-1);
                    exercise(A_WIS, FALSE);
                    if (rnl(u.ulevel) > 6 && u.ualign.record > 0
                        && rnd(u.ualign.record) > (7 * ALIGNLIM) / 8)
                        summon_minion(altaralign, TRUE);
                }
                return 1;
            }
        }

        consume_offering(otmp);
        /* OK, you get brownie points. */
        if (u.ugangr) {
            u.ugangr -= ((value * (u.ualign.type == A_CHAOTIC ? 2 : 3))
                         / MAXVALUE);
            if (u.ugangr < 0)
                u.ugangr = 0;
            if (u.ugangr != saved_anger) {
                if (u.ugangr) {
#if 0 /*JP*/
                    pline("%s seems %s.", u_gname(),
                          Hallucination ? "groovy" : "slightly mollified");
#else
                    pline("%sは%sに見える．", u_gname(),
                          Hallucination ? "素敵" : "ちょっと和らいだよう");
#endif

                    if ((int) u.uluck < 0)
                        change_luck(1);
                } else {
#if 0 /*JP*/
                    pline("%s seems %s.", u_gname(),
                          Hallucination ? "cosmic (not a new fact)"
                                        : "mollified");
#else
                    pline("%sは%sに見える．", u_gname(),
                          Hallucination ? "虹色(新事実ではない)"
                                        : "軽蔑したよう");
#endif

                    if ((int) u.uluck < 0)
                        u.uluck = 0;
                }
            } else { /* not satisfied yet */
                if (Hallucination)
/*JP
                    pline_The("gods seem tall.");
*/
                    pline("神はお高くとまっているように見える．");
                else
/*JP
                    You("have a feeling of inadequacy.");
*/
                    You("まだまだだと感じた．");
            }
        } else if (ugod_is_angry()) {
            if (value > MAXVALUE)
                value = MAXVALUE;
            if (value > -u.ualign.record)
                value = -u.ualign.record;
            adjalign(value);
/*JP
            You_feel("partially absolved.");
*/
            You("少しだけゆるしてもらえたような気がした．");
        } else if (u.ublesscnt > 0) {
            u.ublesscnt -= ((value * (u.ualign.type == A_CHAOTIC ? 500 : 300))
                            / MAXVALUE);
            if (u.ublesscnt < 0)
                u.ublesscnt = 0;
            if (u.ublesscnt != saved_cnt) {
                if (u.ublesscnt) {
                    if (Hallucination)
/*JP
                        You("realize that the gods are not like you and I.");
*/
                        You("神とツーカーの仲ではないことを悟った．");
                    else
/*JP
                        You("have a hopeful feeling.");
*/
                        pline("希望が見えてきたような気がした．");
                    if ((int) u.uluck < 0)
                        change_luck(1);
                } else {
                    if (Hallucination)
/*JP
                        pline("Overall, there is a smell of fried onions.");
*/
                        pline("たまねぎを揚げた匂いがした．");
                    else
/*JP
                        You("have a feeling of reconciliation.");
*/
                        You("許された気がした．");
                    if ((int) u.uluck < 0)
                        u.uluck = 0;
                }
            }
        } else {
            int nartifacts = nartifact_exist();

            /* you were already in pretty good standing */
            /* The player can gain an artifact */
            /* The chance goes down as the number of artifacts goes up */
            if (u.ulevel > 2 && u.uluck >= 0
                && !rn2(10 + (2 * u.ugifts * nartifacts))) {
                otmp = mk_artifact((struct obj *) 0, a_align(u.ux, u.uy));
                if (otmp) {
                    if (otmp->spe < 0)
                        otmp->spe = 0;
                    if (otmp->cursed)
                        uncurse(otmp);
                    otmp->oerodeproof = TRUE;
/*JP
                    at_your_feet("An object");
*/
                    at_your_feet("何か");
                    dropy(otmp);
/*JP
                    godvoice(u.ualign.type, "Use my gift wisely!");
*/
                    godvoice(u.ualign.type, "我が与えしもの賢く使うべし！");
                    u.ugifts++;
                    u.ublesscnt = rnz(300 + (50 * nartifacts));
                    exercise(A_WIS, TRUE);
                    /* make sure we can use this weapon */
                    unrestrict_weapon_skill(weapon_type(otmp));
                    if (!Hallucination && !Blind) {
                        otmp->dknown = 1;
                        makeknown(otmp->otyp);
                        discover_artifact(otmp->oartifact);
                    }
                    return 1;
                }
            }
            change_luck((value * LUCKMAX) / (MAXVALUE * 2));
            if ((int) u.uluck < 0)
                u.uluck = 0;
            if (u.uluck != saved_luck) {
                if (Blind)
#if 0 /*JP*/
                    You("think %s brushed your %s.", something,
                        body_part(FOOT));
#else
                    pline("%sがあなたの%sをくすぐったようだ．", something,
                          body_part(FOOT));
#endif
                else
#if 0 /*JP*/
                    You(Hallucination
                    ? "see crabgrass at your %s.  A funny thing in a dungeon."
                            : "glimpse a four-leaf clover at your %s.",
                        makeplural(body_part(FOOT)));
#else
                    You(Hallucination
                    ? "足元にペンペン草をみつけた．迷宮にしては珍しい．"
                            : "四葉のクローバーを足元に見つけた．");
#endif
            }
        }
    }
    return 1;
}

/* determine prayer results in advance; also used for enlightenment */
boolean
can_pray(praying)
boolean praying; /* false means no messages should be given */
{
    int alignment;

    p_aligntyp = on_altar() ? a_align(u.ux, u.uy) : u.ualign.type;
    p_trouble = in_trouble();

    if (is_demon(youmonst.data) && (p_aligntyp != A_CHAOTIC)) {
        if (praying)
#if 0 /*JP*/
            pline_The("very idea of praying to a %s god is repugnant to you.",
                      p_aligntyp ? "lawful" : "neutral");
#else
            pline("%sの神に祈りをささげるのは常識に背く．",
                p_aligntyp ? "秩序" : "中立");
#endif
        return FALSE;
    }

    if (praying)
/*JP
        You("begin praying to %s.", align_gname(p_aligntyp));
*/
        You("%sに祈りを捧げた．", align_gname(p_aligntyp));

    if (u.ualign.type && u.ualign.type == -p_aligntyp)
        alignment = -u.ualign.record; /* Opposite alignment altar */
    else if (u.ualign.type != p_aligntyp)
        alignment = u.ualign.record / 2; /* Different alignment altar */
    else
        alignment = u.ualign.record;

    if ((p_trouble > 0) ? (u.ublesscnt > 200)      /* big trouble */
           : (p_trouble < 0) ? (u.ublesscnt > 100) /* minor difficulties */
              : (u.ublesscnt > 0))                 /* not in trouble */
        p_type = 0;                     /* too soon... */
    else if ((int) Luck < 0 || u.ugangr || alignment < 0)
        p_type = 1; /* too naughty... */
    else /* alignment >= 0 */ {
        if (on_altar() && u.ualign.type != p_aligntyp)
            p_type = 2;
        else
            p_type = 3;
    }

    if (is_undead(youmonst.data) && !Inhell
        && (p_aligntyp == A_LAWFUL || (p_aligntyp == A_NEUTRAL && !rn2(10))))
        p_type = -1;
    /* Note:  when !praying, the random factor for neutrals makes the
       return value a non-deterministic approximation for enlightenment.
       This case should be uncommon enough to live with... */

    return !praying ? (boolean) (p_type == 3 && !Inhell) : TRUE;
}

/* #pray commmand */
int
dopray()
{
    /* Confirm accidental slips of Alt-P */
/*JP
    if (ParanoidPray && yn("Are you sure you want to pray?") != 'y')
*/
    if (ParanoidPray && yn("祈りますか？") != 'y')
        return 0;

    u.uconduct.gnostic++;

    /* set up p_type and p_alignment */
    if (!can_pray(TRUE))
        return 0;

    if (wizard && p_type >= 0) {
/*JP
        if (yn("Force the gods to be pleased?") == 'y') {
*/
        if (yn("無理矢理神に微笑んでもらいますか？") == 'y') {
            u.ublesscnt = 0;
            if (u.uluck < 0)
                u.uluck = 0;
            if (u.ualign.record <= 0)
                u.ualign.record = 1;
            u.ugangr = 0;
            if (p_type < 2)
                p_type = 3;
        }
    }
    nomul(-3);
    multi_reason = "praying";
/*JP
    nomovemsg = "You finish your prayer.";
*/
    nomovemsg = "祈り終えた．";
    afternmv = prayer_done;

    if (p_type == 3 && !Inhell) {
        /* if you've been true to your god you can't die while you pray */
        if (!Blind)
/*JP
            You("are surrounded by a shimmering light.");
*/
            You("かすかな光につつまれた．");
        u.uinvulnerable = TRUE;
    }

    return 1;
}

STATIC_PTR int
prayer_done() /* M. Stephenson (1.0.3b) */
{
    aligntyp alignment = p_aligntyp;

    u.uinvulnerable = FALSE;
    if (p_type == -1) {
#if 0 /*JP*/
        godvoice(alignment,
                 (alignment == A_LAWFUL)
                    ? "Vile creature, thou durst call upon me?"
                    : "Walk no more, perversion of nature!");
#else
        godvoice(alignment,
                 (alignment == A_LAWFUL)
                    ? "卑劣な生物よ，汝，我に祈りを求めたか？"
                    : "動くな！死にぞこないの生物よ！");
#endif
/*JP
        You_feel("like you are falling apart.");
*/
        You("バラバラになったような気がした．");
        /* KMH -- Gods have mastery over unchanging */
        rehumanize();
        /* no Half_physical_damage adjustment here */
/*JP
        losehp(rnd(20), "residual undead turning effect", KILLED_BY_AN);
*/
        losehp(rnd(20), "不死の生物を土に返す力で", KILLED_BY_AN);
        exercise(A_CON, FALSE);
        return 1;
    }
    if (Inhell) {
/*JP
        pline("Since you are in Gehennom, %s won't help you.",
*/
        pline("ゲヘナに%sの力は届かない．",
              align_gname(alignment));
        /* haltingly aligned is least likely to anger */
        if (u.ualign.record <= 0 || rnl(u.ualign.record))
            angrygods(u.ualign.type);
        return 0;
    }

    if (p_type == 0) {
        if (on_altar() && u.ualign.type != alignment)
            (void) water_prayer(FALSE);
        u.ublesscnt += rnz(250);
        change_luck(-3);
        gods_upset(u.ualign.type);
    } else if (p_type == 1) {
        if (on_altar() && u.ualign.type != alignment)
            (void) water_prayer(FALSE);
        angrygods(u.ualign.type); /* naughty */
    } else if (p_type == 2) {
        if (water_prayer(FALSE)) {
            /* attempted water prayer on a non-coaligned altar */
            u.ublesscnt += rnz(250);
            change_luck(-3);
            gods_upset(u.ualign.type);
        } else
            pleased(alignment);
    } else {
        /* coaligned */
        if (on_altar())
            (void) water_prayer(TRUE);
        pleased(alignment); /* nice */
    }
    return 1;
}

/* #turn command */
int
doturn()
{
    /* Knights & Priest(esse)s only please */
    struct monst *mtmp, *mtmp2;
    int once, range, xlev;

    if (!Role_if(PM_PRIEST) && !Role_if(PM_KNIGHT)) {
        /* Try to use the "turn undead" spell.
         *
         * This used to be based on whether hero knows the name of the
         * turn undead spellbook, but it's possible to know--and be able
         * to cast--the spell while having lost the book ID to amnesia.
         * (It also used to tell spelleffects() to cast at self?)
         */
        int sp_no;

        for (sp_no = 0; sp_no < MAXSPELL; ++sp_no) {
            if (spl_book[sp_no].sp_id == NO_SPELL)
                break;
            else if (spl_book[sp_no].sp_id == SPE_TURN_UNDEAD)
                return spelleffects(sp_no, FALSE);
        }
/*JP
        You("don't know how to turn undead!");
*/
        You("不死の生き物を土に戻す方法を知らない！");
        return 0;
    }
    u.uconduct.gnostic++;

    if ((u.ualign.type != A_CHAOTIC
         && (is_demon(youmonst.data) || is_undead(youmonst.data)))
        || u.ugangr > 6) { /* "Die, mortal!" */
/*JP
        pline("For some reason, %s seems to ignore you.", u_gname());
*/
        pline("なぜか，%sはあなたを無視したようだ．", u_gname());
        aggravate();
        exercise(A_WIS, FALSE);
        return 0;
    }
    if (Inhell) {
/*JP
        pline("Since you are in Gehennom, %s won't help you.", u_gname());
*/
        pline("ゲヘナに%sの力は届かない．", u_gname());
        aggravate();
        return 0;
    }
/*JP
    pline("Calling upon %s, you chant an arcane formula.", u_gname());
*/
    pline("%sに祈りを求めると，あなたは不可思議な言葉の聖歌を聞いた．", u_gname());
    exercise(A_WIS, TRUE);

    /* note: does not perform unturn_dead() on victims' inventories */
    range = BOLT_LIM + (u.ulevel / 5); /* 5 to 11 */
    range *= range;
    once = 0;
    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
        mtmp2 = mtmp->nmon;

        if (DEADMONSTER(mtmp))
            continue;
        if (!cansee(mtmp->mx, mtmp->my) || distu(mtmp->mx, mtmp->my) > range)
            continue;

        if (!mtmp->mpeaceful
            && (is_undead(mtmp->data) || is_vampshifter(mtmp)
                || (is_demon(mtmp->data) && (u.ulevel > (MAXULEV / 2))))) {
            mtmp->msleeping = 0;
            if (Confusion) {
                if (!once++)
/*JP
                    pline("Unfortunately, your voice falters.");
*/
                    pline("残念ながら，あなたの声はどもってしまった．");
                mtmp->mflee = 0;
                mtmp->mfrozen = 0;
                mtmp->mcanmove = 1;
            } else if (!resist(mtmp, '\0', 0, TELL)) {
                xlev = 6;
                switch (mtmp->data->mlet) {
                /* this is intentional, lichs are tougher
                   than zombies. */
                case S_LICH:
                    xlev += 2; /*FALLTHRU*/
                case S_GHOST:
                    xlev += 2; /*FALLTHRU*/
                case S_VAMPIRE:
                    xlev += 2; /*FALLTHRU*/
                case S_WRAITH:
                    xlev += 2; /*FALLTHRU*/
                case S_MUMMY:
                    xlev += 2; /*FALLTHRU*/
                case S_ZOMBIE:
                    if (u.ulevel >= xlev && !resist(mtmp, '\0', 0, NOTELL)) {
                        if (u.ualign.type == A_CHAOTIC) {
                            mtmp->mpeaceful = 1;
                            set_malign(mtmp);
                        } else { /* damn them */
                            killed(mtmp);
                        }
                        break;
                    } /* else flee */
                /*FALLTHRU*/
                default:
                    monflee(mtmp, 0, FALSE, TRUE);
                    break;
                }
            }
        }
    }
    nomul(-5);
    multi_reason = "trying to turn the monsters";
    nomovemsg = You_can_move_again;
    return 1;
}

const char *
a_gname()
{
    return a_gname_at(u.ux, u.uy);
}

/* returns the name of an altar's deity */
const char *
a_gname_at(x, y)
xchar x, y;
{
    if (!IS_ALTAR(levl[x][y].typ))
        return (char *) 0;

    return align_gname(a_align(x, y));
}

/* returns the name of the hero's deity */
const char *
u_gname()
{
    return align_gname(u.ualign.type);
}

const char *
align_gname(alignment)
aligntyp alignment;
{
    const char *gnam;

    switch (alignment) {
    case A_NONE:
        gnam = Moloch;
        break;
    case A_LAWFUL:
        gnam = urole.lgod;
        break;
    case A_NEUTRAL:
        gnam = urole.ngod;
        break;
    case A_CHAOTIC:
        gnam = urole.cgod;
        break;
    default:
        impossible("unknown alignment.");
/*JP
        gnam = "someone";
*/
        gnam = "誰か";
        break;
    }
    if (*gnam == '_')
        ++gnam;
    return gnam;
}

static const char *hallu_gods[] = {
    "the Flying Spaghetti Monster", /* Church of the FSM */
    "Eris",                         /* Discordianism */
    "the Martians",                 /* every science fiction ever */
    "Xom",                          /* Crawl */
    "AnDoR dRaKoN",                 /* ADOM */
    "the Central Bank of Yendor",   /* economics */
    "Tooth Fairy",                  /* real world(?) */
    "Om",                           /* Discworld */
    "Yawgmoth",                     /* Magic: the Gathering */
    "Morgoth",                      /* LoTR */
    "Cthulhu",                      /* Lovecraft */
    "the Ori",                      /* Stargate */
    "destiny",                      /* why not? */
    "your Friend the Computer",     /* Paranoia */
};

/* hallucination handling for priest/minion names: select a random god
   iff character is hallucinating */
const char *
halu_gname(alignment)
aligntyp alignment;
{
    const char *gnam = NULL;
    int which;

    if (!Hallucination)
        return align_gname(alignment);

    /* The priest may not have initialized god names. If this is the
     * case, and we roll priest, we need to try again. */
    do
        which = randrole();
    while (!roles[which].lgod);

    switch (rn2(9)) {
    case 0:
    case 1:
        gnam = roles[which].lgod;
        break;
    case 2:
    case 3:
        gnam = roles[which].ngod;
        break;
    case 4:
    case 5:
        gnam = roles[which].cgod;
        break;
    case 6:
    case 7:
        gnam = hallu_gods[rn2(sizeof hallu_gods / sizeof *hallu_gods)];
        break;
    case 8:
        gnam = Moloch;
        break;
    default:
        impossible("rn2 broken in halu_gname?!?");
    }
    if (!gnam) {
        impossible("No random god name?");
#if 0 /*JP*/
        gnam = "your Friend the Computer"; /* Paranoia */
#else
        gnam = "親愛なるコンピュータ"; /* Paranoia */
#endif
    }
    if (*gnam == '_')
        ++gnam;
    return gnam;
}

/* deity's title */
const char *
align_gtitle(alignment)
aligntyp alignment;
{
/*JP
    const char *gnam, *result = "god";
*/
    const char *gnam, *result = "主神";

    switch (alignment) {
    case A_LAWFUL:
        gnam = urole.lgod;
        break;
    case A_NEUTRAL:
        gnam = urole.ngod;
        break;
    case A_CHAOTIC:
        gnam = urole.cgod;
        break;
    default:
        gnam = 0;
        break;
    }
    if (gnam && *gnam == '_')
/*JP
        result = "goddess";
*/
        result = "女神";
    return result;
}

void
altar_wrath(x, y)
register int x, y;
{
    aligntyp altaralign = a_align(x, y);

    if (!strcmp(align_gname(altaralign), u_gname())) {
/*JP
        godvoice(altaralign, "How darest thou desecrate my altar!");
*/
        godvoice(altaralign, "汝，我が祭壇を汚すか！");
        (void) adjattrib(A_WIS, -1, FALSE);
    } else {
/*JP
        pline("A voice (could it be %s?) whispers:", align_gname(altaralign));
*/
        pline("ささやき声(たぶん%s？)が聞こえる:", align_gname(altaralign));
/*JP
        verbalize("Thou shalt pay, infidel!");
*/
        verbalize("異端者よ！報いを受けよ！");
        change_luck(-1);
    }
}

/* assumes isok() at one space away, but not necessarily at two */
STATIC_OVL boolean
blocked_boulder(dx, dy)
int dx, dy;
{
    register struct obj *otmp;
    long count = 0L;

    for (otmp = level.objects[u.ux + dx][u.uy + dy]; otmp;
         otmp = otmp->nexthere) {
        if (otmp->otyp == BOULDER)
            count += otmp->quan;
    }

    switch (count) {
    case 0:
        /* no boulders--not blocked */
        return FALSE;
    case 1:
        /* possibly blocked depending on if it's pushable */
        break;
    default:
        /* more than one boulder--blocked after they push the top one;
           don't force them to push it first to find out */
        return TRUE;
    }

    if (!isok(u.ux + 2 * dx, u.uy + 2 * dy))
        return TRUE;
    if (IS_ROCK(levl[u.ux + 2 * dx][u.uy + 2 * dy].typ))
        return TRUE;
    if (sobj_at(BOULDER, u.ux + 2 * dx, u.uy + 2 * dy))
        return TRUE;

    return FALSE;
}

/*pray.c*/
