/* NetHack 3.6	mhitu.c	$NHDT-Date: 1513297347 2017/12/15 00:22:27 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.149 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2019            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

STATIC_VAR NEARDATA struct obj *mon_currwep = (struct obj *) 0;

STATIC_DCL boolean FDECL(u_slip_free, (struct monst *, struct attack *));
STATIC_DCL int FDECL(passiveum, (struct permonst *, struct monst *,
                                 struct attack *));
STATIC_DCL void FDECL(mayberem, (struct obj *, const char *));
STATIC_DCL boolean FDECL(diseasemu, (struct permonst *));
STATIC_DCL int FDECL(hitmu, (struct monst *, struct attack *));
STATIC_DCL int FDECL(gulpmu, (struct monst *, struct attack *));
STATIC_DCL int FDECL(explmu, (struct monst *, struct attack *, BOOLEAN_P));
STATIC_DCL void FDECL(missmu, (struct monst *, BOOLEAN_P, struct attack *));
STATIC_DCL void FDECL(mswings, (struct monst *, struct obj *));
STATIC_DCL void FDECL(wildmiss, (struct monst *, struct attack *));
STATIC_DCL void FDECL(hitmsg, (struct monst *, struct attack *));

/* See comment in mhitm.c.  If we use this a lot it probably should be */
/* changed to a parameter to mhitu. */
static int dieroll;

STATIC_OVL void
hitmsg(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    int compat;
    const char *pfmt = 0;
    char *Monst_name = Monnam(mtmp);

    /* Note: if opposite gender, "seductively" */
    /* If same gender, "engagingly" for nymph, normal msg for others */
    if ((compat = could_seduce(mtmp, &youmonst, mattk)) != 0
        && !mtmp->mcan && !mtmp->mspec_used) {
#if 0 /*JP:T*/
        pline("%s %s you %s.", Monst_name,
              Blind ? "talks to" : "smiles at",
              (compat == 2) ? "engagingly" : "seductively");
#else
        pline("%sはあなた%s%s．", Monst_name,
              (compat == 2) ? "を引きつけるように" : "に好意をよせるように",
              Blind ? "話しかけた" : "微笑んだ");
#endif
    } else {
        switch (mattk->aatyp) {
        case AT_BITE:
/*JP
            pfmt = "%s bites!";
*/
            pfmt = "%sは噛みついた！";
            break;
        case AT_KICK:
#if 0 /*JP:T*/
            pline("%s kicks%c", Monst_name,
                  thick_skinned(youmonst.data) ? '.' : '!');
#else
            pline("%sは蹴とばした%s",Monnam(mtmp), 
                  thick_skinned(youmonst.data) ? "．" : "！");
#endif
            break;
        case AT_STNG:
/*JP
            pfmt = "%s stings!";
*/
            pfmt = "%sは突きさした！";
            break;
        case AT_BUTT:
/*JP
            pfmt = "%s butts!";
*/
            pfmt = "%sは頭突きをくらわした！";
            break;
        case AT_TUCH:
/*JP
            pfmt = "%s touches you!";
*/
            pfmt = "%sはあなたに触れた！";
            break;
        case AT_TENT:
/*JP
            pfmt = "%s tentacles suck you!";
*/
            pfmt = "%sの触手があなたの体液を吸いとった！";
#if 0 /*JP*/
            Monst_name = s_suffix(Monst_name);
#endif
            break;
        case AT_EXPL:
        case AT_BOOM:
/*JP
            pfmt = "%s explodes!";
*/
            pfmt = "%sは爆発した！";
            break;
        default:
/*JP
            pfmt = "%s hits!";
*/
            pfmt = "%sの攻撃は命中した！";
        }
        if (pfmt)
            pline(pfmt, Monst_name);
    }
}

/* monster missed you */
STATIC_OVL void
missmu(mtmp, nearmiss, mattk)
struct monst *mtmp;
boolean nearmiss;
struct attack *mattk;
{
    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    if (could_seduce(mtmp, &youmonst, mattk) && !mtmp->mcan)
/*JP
        pline("%s pretends to be friendly.", Monnam(mtmp));
*/
        pline("%sは友好的なふりをしている．",Monnam(mtmp));
    else
#if 0 /*JP*/
        pline("%s %smisses!", Monnam(mtmp),
              (nearmiss && flags.verbose) ? "just " : "");
#else
        pline("%sの攻撃は%sた．", Monnam(mtmp),
              (nearmiss && flags.verbose) ? "空を切っ" : "はずれ");
#endif

    stop_occupation();
}

/* monster swings obj */
STATIC_OVL void
mswings(mtmp, otemp)
struct monst *mtmp;
struct obj *otemp;
{
    if (flags.verbose && !Blind && mon_visible(mtmp)) {
#if 0 /*JP*/
        pline("%s %s %s%s %s.", Monnam(mtmp),
              (objects[otemp->otyp].oc_dir & PIERCE) ? "thrusts" : "swings",
              (otemp->quan > 1L) ? "one of " : "", mhis(mtmp), xname(otemp));
#else
        pline("%sは%s%s．", Monnam(mtmp),
              xname(otemp),
              (objects[otemp->otyp].oc_dir & PIERCE) ? "を突いた" : "を振りまわした");
#endif
    }
}

/* return how a poison attack was delivered */
const char *
mpoisons_subj(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    if (mattk->aatyp == AT_WEAP) {
        struct obj *mwep = (mtmp == &youmonst) ? uwep : MON_WEP(mtmp);
        /* "Foo's attack was poisoned." is pretty lame, but at least
           it's better than "sting" when not a stinging attack... */
/*JP
        return (!mwep || !mwep->opoisoned) ? "attack" : "weapon";
*/
        return (!mwep || !mwep->opoisoned) ? "攻撃" : "武器";
    } else {
#if 0 /*JP*/
        return (mattk->aatyp == AT_TUCH) ? "contact"
                  : (mattk->aatyp == AT_GAZE) ? "gaze"
                       : (mattk->aatyp == AT_BITE) ? "bite" : "sting";
#else
        return (mattk->aatyp == AT_TUCH) ? "接触"
                  : (mattk->aatyp == AT_GAZE) ? "にらみ"
                       : (mattk->aatyp == AT_BITE) ? "噛みつき" : "突きさし";
#endif
    }
}

/* called when your intrinsic speed is taken away */
void
u_slow_down()
{
    HFast = 0L;
    if (!Fast)
/*JP
        You("slow down.");
*/
        You("動きが遅くなった．");
    else /* speed boots */
/*JP
        Your("quickness feels less natural.");
*/
        You("速さについていけなくなった．");
    exercise(A_DEX, FALSE);
}

/* monster attacked your displaced image */
STATIC_OVL void
wildmiss(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    int compat;
    const char *Monst_name; /* Monnam(mtmp) */

    /* no map_invisible() -- no way to tell where _this_ is coming from */

    if (!flags.verbose)
        return;
    if (!cansee(mtmp->mx, mtmp->my))
        return;
    /* maybe it's attacking an image around the corner? */

    compat = ((mattk->adtyp == AD_SEDU || mattk->adtyp == AD_SSEX)
              && could_seduce(mtmp, &youmonst, (struct attack *) 0));
    Monst_name = Monnam(mtmp);

    if (!mtmp->mcansee || (Invis && !perceives(mtmp->data))) {
#if 0 /*JP:T*/
        const char *swings = (mattk->aatyp == AT_BITE) ? "snaps"
                             : (mattk->aatyp == AT_KICK) ? "kicks"
                               : (mattk->aatyp == AT_STNG
                                  || mattk->aatyp == AT_BUTT
                                  || nolimbs(mtmp->data)) ? "lunges"
                                 : "swings";
#else
        const char *swings = (mattk->aatyp == AT_BITE) ? "噛みつく"
                             : (mattk->aatyp == AT_KICK) ? "蹴とばす"
                               : (mattk->aatyp == AT_STNG
                                  || mattk->aatyp == AT_BUTT
                                  || nolimbs(mtmp->data)) ? "突進する"
                                 : "振り回す";
#endif

        if (compat)
/*JP
            pline("%s tries to touch you and misses!", Monst_name);
*/
            pline("%sはあなたに触ろうとしたが失敗した！", Monst_name);
        else
            switch (rn2(3)) {
            case 0:
/*JP
                pline("%s %s wildly and misses!", Monst_name, swings);
*/
                pline("%sは激しく%sが，はずした！", Monst_name, swings);
                break;
            case 1:
/*JP
                pline("%s attacks a spot beside you.", Monst_name);
*/
                pline("%sの攻撃はあなたの脇腹をかすめた．", Monst_name);
                break;
            case 2:
#if 0 /*JP*/
                pline("%s strikes at %s!", Monst_name,
                      (levl[mtmp->mux][mtmp->muy].typ == WATER)
                        ? "empty water"
                        : "thin air");
#else
                pline("%sは%sを打ちつけた！", Monst_name,
                      (levl[mtmp->mux][mtmp->muy].typ == WATER)
                        ? "水"
                        : "何もないところ");
#endif
                break;
            default:
/*JP
                pline("%s %s wildly!", Monst_name, swings);
*/
                pline("%sは激しく%s！", Monst_name, swings);
                break;
            }

    } else if (Displaced) {
        if (compat)
#if 0 /*JP*/
            pline("%s smiles %s at your %sdisplaced image...", Monst_name,
                  (compat == 2) ? "engagingly" : "seductively",
                  Invis ? "invisible " : "");
#else
            pline("%sは%sあなたの幻影に対して%s微笑んだ．．．", Monst_name,
                  Invis ? "透明な" : "",
                  (compat == 2) ? "魅力的に" : "誘惑的に");
#endif
        else
#if 0 /*JP*/
            pline("%s strikes at your %sdisplaced image and misses you!",
                  /* Note: if you're both invisible and displaced,
                   * only monsters which see invisible will attack your
                   * displaced image, since the displaced image is also
                   * invisible.
                   */
                  Monst_name, Invis ? "invisible " : "");
#else
            pline("%sはあなたの%s幻影を攻撃し，はずした！",
                  Monst_name, Invis ? "透明な" : "");
#endif

    } else if (Underwater) {
        /* monsters may miss especially on water level where
           bubbles shake the player here and there */
        if (compat)
/*JP
            pline("%s reaches towards your distorted image.", Monst_name);
*/
            pline("%sはあなたの歪んだ幻影の方へ向かった．", Monst_name);
        else
/*JP
            pline("%s is fooled by water reflections and misses!",
*/
            pline("%sは水の反射にだまされ，はずした！",
                  Monst_name);

    } else
        impossible("%s attacks you without knowing your location?",
                   Monst_name);
}

void
expels(mtmp, mdat, message)
struct monst *mtmp;
struct permonst *mdat; /* if mtmp is polymorphed, mdat != mtmp->data */
boolean message;
{
    if (message) {
        if (is_animal(mdat)) {
/*JP
            You("get regurgitated!");
*/
            You("吐きだされた！");
        } else {
            char blast[40];
            struct attack *attk = attacktype_fordmg(mdat, AT_ENGL, AD_ANY);

            blast[0] = '\0';
            if (!attk) {
                impossible("Swallower has no engulfing attack?");
            } else {
                if (is_whirly(mdat)) {
                    switch (attk->adtyp) {
                    case AD_ELEC:
/*JP
                        Strcpy(blast, " in a shower of sparks");
*/
                        Strcpy(blast, "の火花の雨の中から");
                        break;
                    case AD_COLD:
/*JP
                        Strcpy(blast, " in a blast of frost");
*/
                        Strcpy(blast, "の冷気の風の中から");
                        break;
                    }
                } else {
/*JP
                    Strcpy(blast, " with a squelch");
*/
                    Strcpy(blast, "から吐き出されるように");
                }
/*JP
                You("get expelled from %s%s!", mon_nam(mtmp), blast);
*/
                You("%s%s脱出した！", mon_nam(mtmp), blast);
            }
        }
    }
    unstuck(mtmp); /* ball&chain returned in unstuck() */
    mnexto(mtmp);
    newsym(u.ux, u.uy);
    spoteffects(TRUE);
    /* to cover for a case where mtmp is not in a next square */
    if (um_dist(mtmp->mx, mtmp->my, 1))
/*JP
        pline("Brrooaa...  You land hard at some distance.");
*/
        pline("ブロロロ．．遠くに着陸するのは難しい．");
}

/* select a monster's next attack, possibly substituting for its usual one */
struct attack *
getmattk(magr, mdef, indx, prev_result, alt_attk_buf)
struct monst *magr, *mdef;
int indx, prev_result[];
struct attack *alt_attk_buf;
{
    struct permonst *mptr = magr->data;
    struct attack *attk = &mptr->mattk[indx];
    struct obj *weap = (magr == &youmonst) ? uwep : MON_WEP(magr);

    /* prevent a monster with two consecutive disease or hunger attacks
       from hitting with both of them on the same turn; if the first has
       already hit, switch to a stun attack for the second */
    if (indx > 0 && prev_result[indx - 1] > 0
        && (attk->adtyp == AD_DISE || attk->adtyp == AD_PEST
            || attk->adtyp == AD_FAMN)
        && attk->adtyp == mptr->mattk[indx - 1].adtyp) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_STUN;

    /* make drain-energy damage be somewhat in proportion to energy */
    } else if (attk->adtyp == AD_DREN && mdef == &youmonst) {
        int ulev = max(u.ulevel, 6);

        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        /* 3.6.0 used 4d6 but since energy drain came out of max energy
           once current energy was gone, that tended to have a severe
           effect on low energy characters; it's now 2d6 with ajustments */
        if (u.uen <= 5 * ulev && attk->damn > 1) {
            attk->damn -= 1; /* low energy: 2d6 -> 1d6 */
            if (u.uenmax <= 2 * ulev && attk->damd > 3)
                attk->damd -= 3; /* very low energy: 1d6 -> 1d3 */
        } else if (u.uen > 12 * ulev) {
            attk->damn += 1; /* high energy: 2d6 -> 3d6 */
            if (u.uenmax > 20 * ulev)
                attk->damd += 3; /* very high energy: 3d6 -> 3d9 */
            /* note: 3d9 is slightly higher than previous 4d6 */
        }

    } else if (attk->aatyp == AT_ENGL && magr->mspec_used) {
        /* can't re-engulf yet; switch to simpler attack */
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        if (attk->adtyp == AD_ACID || attk->adtyp == AD_ELEC
            || attk->adtyp == AD_COLD || attk->adtyp == AD_FIRE) {
            attk->aatyp = AT_TUCH;
        } else {
            attk->aatyp = AT_CLAW; /* attack message will be "<foo> hits" */
            attk->adtyp = AD_PHYS;
        }
        attk->damn = 1; /* relatively weak: 1d6 */
        attk->damd = 6;

    /* barrow wight, Nazgul, erinys have weapon attack for non-physical
       damage; force physical damage if attacker has been cancelled or
       if weapon is sufficiently interesting; a few unique creatures
       have two weapon attacks where one does physical damage and other
       doesn't--avoid forcing physical damage for those */
    } else if (indx == 0 && magr != &youmonst
               && attk->aatyp == AT_WEAP && attk->adtyp != AD_PHYS
               && !(mptr->mattk[1].aatyp == AT_WEAP
                    && mptr->mattk[1].adtyp == AD_PHYS)
               && (magr->mcan
                   || (weap && ((weap->otyp == CORPSE
                                 && touch_petrifies(&mons[weap->corpsenm]))
                                || weap->oartifact == ART_STORMBRINGER
                                || weap->oartifact == ART_VORPAL_BLADE)))) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_PHYS;
    }
    return attk;
}

/*
 * mattacku: monster attacks you
 *      returns 1 if monster dies (e.g. "yellow light"), 0 otherwise
 *      Note: if you're displaced or invisible the monster might attack the
 *              wrong position...
 *      Assumption: it's attacking you or an empty square; if there's another
 *              monster which it attacks by mistake, the caller had better
 *              take care of it...
 */
int
mattacku(mtmp)
register struct monst *mtmp;
{
    struct attack *mattk, alt_attk;
    int i, j = 0, tmp, sum[NATTK];
    struct permonst *mdat = mtmp->data;
    boolean ranged = (distu(mtmp->mx, mtmp->my) > 3);
    /* Is it near you?  Affects your actions */
    boolean range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
    /* Does it think it's near you?  Affects its actions */
    boolean foundyou = (mtmp->mux == u.ux && mtmp->muy == u.uy);
    /* Is it attacking you or your image? */
    boolean youseeit = canseemon(mtmp);
    /* Might be attacking your image around the corner, or
     * invisible, or you might be blind....
     */
    boolean skipnonmagc = FALSE;
    /* Are further physical attack attempts useless? */

    if (!ranged)
        nomul(0);
    if (mtmp->mhp <= 0 || (Underwater && !is_swimmer(mtmp->data)))
        return 0;

    /* If swallowed, can only be affected by u.ustuck */
    if (u.uswallow) {
        if (mtmp != u.ustuck)
            return 0;
        u.ustuck->mux = u.ux;
        u.ustuck->muy = u.uy;
        range2 = 0;
        foundyou = 1;
        if (u.uinvulnerable)
            return 0; /* stomachs can't hurt you! */

    } else if (u.usteed) {
        if (mtmp == u.usteed)
            /* Your steed won't attack you */
            return 0;
        /* Orcs like to steal and eat horses and the like */
        if (!rn2(is_orc(mtmp->data) ? 2 : 4)
            && distu(mtmp->mx, mtmp->my) <= 2) {
            /* Attack your steed instead */
            i = mattackm(mtmp, u.usteed);
            if ((i & MM_AGR_DIED))
                return 1;
            /* make sure steed is still alive and within range */
            if ((i & MM_DEF_DIED) || !u.usteed
                || distu(mtmp->mx, mtmp->my) > 2)
                return 0;
            /* Let your steed retaliate */
            return !!(mattackm(u.usteed, mtmp) & MM_DEF_DIED);
        }
    }

    if (u.uundetected && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        u.uundetected = 0;
        if (is_hider(youmonst.data) && u.umonnum != PM_TRAPPER) {
            /* ceiling hider */
            coord cc; /* maybe we need a unexto() function? */
            struct obj *obj;

/*JP
            You("fall from the %s!", ceiling(u.ux, u.uy));
*/
            You("%sから落ちた！", ceiling(u.ux,u.uy));
            /* take monster off map now so that its location
               is eligible for placing hero; we assume that a
               removed monster remembers its old spot <mx,my> */
            remove_monster(mtmp->mx, mtmp->my);
            if (!enexto(&cc, u.ux, u.uy, youmonst.data)
                /* a fish won't voluntarily swap positions
                   when it's in water and hero is over land */
                || (mtmp->data->mlet == S_EEL
                    && is_pool(mtmp->mx, mtmp->my)
                    && !is_pool(u.ux, u.uy))) {
                /* couldn't find any spot for hero; this used to
                   kill off attacker, but now we just give a "miss"
                   message and keep both mtmp and hero at their
                   original positions; hero has become unconcealed
                   so mtmp's next move will be a regular attack */
                place_monster(mtmp, mtmp->mx, mtmp->my); /* put back */
                newsym(u.ux, u.uy); /* u.uundetected was toggled */
/*JP
                pline("%s draws back as you drop!", Monnam(mtmp));
*/
                pline("あなたが落ちることで%sは引き戻された！", Monnam(mtmp));
                return 0;
            }

            /* put mtmp at hero's spot and move hero to <cc.x,.y> */
            newsym(mtmp->mx, mtmp->my); /* finish removal */
            place_monster(mtmp, u.ux, u.uy);
            if (mtmp->wormno) {
                worm_move(mtmp);
                /* tail hasn't grown, so if it now occupies <cc.x,.y>
                   then one of its original spots must be free */
                if (m_at(cc.x, cc.y))
                    (void) enexto(&cc, u.ux, u.uy, youmonst.data);
            }
            teleds(cc.x, cc.y, TRUE); /* move hero */
            set_apparxy(mtmp);
            newsym(u.ux, u.uy);

            if (youmonst.data->mlet != S_PIERCER)
                return 0; /* lurkers don't attack */

            obj = which_armor(mtmp, WORN_HELMET);
            if (obj && is_metallic(obj)) {
#if 0 /*JP*/
                Your("blow glances off %s %s.", s_suffix(mon_nam(mtmp)),
                     helm_simple_name(obj));
#else
                Your("攻撃は%sの%sをかすめただけだった．", mon_nam(mtmp),
                      helm_simple_name(obj));
#endif
            } else {
                if (3 + find_mac(mtmp) <= rnd(20)) {
/*JP
                    pline("%s is hit by a falling piercer (you)!",
*/
                    pline("%sは落ちてきた針(あなた)で傷ついた！",
                          Monnam(mtmp));
                    if ((mtmp->mhp -= d(3, 6)) < 1)
                        killed(mtmp);
                } else
/*JP
                    pline("%s is almost hit by a falling piercer (you)!",
*/
                    pline("%sはもう少しで落ちてきた針(あなた)で傷つくところだった！",
                          Monnam(mtmp));
            }

        } else {
            /* surface hider */
            if (!youseeit) {
/*JP
                pline("It tries to move where you are hiding.");
*/
                pline("何者かがあなたが隠れているところを移動しようとした．");
            } else {
                /* Ugly kludge for eggs.  The message is phrased so as
                 * to be directed at the monster, not the player,
                 * which makes "laid by you" wrong.  For the
                 * parallelism to work, we can't rephrase it, so we
                 * zap the "laid by you" momentarily instead.
                 */
                struct obj *obj = level.objects[u.ux][u.uy];

                if (obj || u.umonnum == PM_TRAPPER
                    || (youmonst.data->mlet == S_EEL
                        && is_pool(u.ux, u.uy))) {
                    int save_spe = 0; /* suppress warning */

                    if (obj) {
                        save_spe = obj->spe;
                        if (obj->otyp == EGG)
                            obj->spe = 0;
                    }
                    /* note that m_monnam() overrides hallucination, which is
                       what we want when message is from mtmp's perspective */
                    if (youmonst.data->mlet == S_EEL
                        || u.umonnum == PM_TRAPPER)
                        pline(
#if 0 /*JP*/
                             "Wait, %s!  There's a hidden %s named %s there!",
                              m_monnam(mtmp), youmonst.data->mname, plname);
#else
                              "待て，%s！%sという名の%sが隠れている！",
                              m_monnam(mtmp), plname, youmonst.data->mname);
#endif
                    else
                        pline(
#if 0 /*JP*/
                          "Wait, %s!  There's a %s named %s hiding under %s!",
                              m_monnam(mtmp), youmonst.data->mname, plname,
                              doname(level.objects[u.ux][u.uy]));
#else
                          "待て，%s！%sという名の%sが%sの下に隠れている！",
                              m_monnam(mtmp), plname, youmonst.data->mname, 
                              doname(level.objects[u.ux][u.uy]));
#endif
                    if (obj)
                        obj->spe = save_spe;
                } else
                    impossible("hiding under nothing?");
            }
            newsym(u.ux, u.uy);
        }
        return 0;
    }

    /* hero might be a mimic, concealed via #monster */
    if (youmonst.data->mlet == S_MIMIC && youmonst.m_ap_type && !range2
        && foundyou && !u.uswallow) {
        boolean sticky = sticks(youmonst.data);

        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (sticky && !youseeit)
/*JP
            pline("It gets stuck on you.");
*/
            pline("何者かがあなたの上にのしかかった．");
        else /* see note about m_monnam() above */
#if 0 /*JP*/
            pline("Wait, %s!  That's a %s named %s!", m_monnam(mtmp),
                  youmonst.data->mname, plname);
#else
            pline("待て，%s！それは%sという名の%sだ！", m_monnam(mtmp), 
                  plname, youmonst.data->mname);
#endif
        if (sticky)
            u.ustuck = mtmp;
        youmonst.m_ap_type = M_AP_NOTHING;
        youmonst.mappearance = 0;
        newsym(u.ux, u.uy);
        return 0;
    }

    /* non-mimic hero might be mimicking an object after eating m corpse */
    if (youmonst.m_ap_type == M_AP_OBJECT && !range2 && foundyou
        && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (!youseeit)
#if 0 /*JP*/
            pline("%s %s!", Something, (likes_gold(mtmp->data)
                                        && youmonst.mappearance == GOLD_PIECE)
                                           ? "tries to pick you up"
                                           : "disturbs you");
#else
            pline("%sは%s！", Something, (likes_gold(mtmp->data)
                                        && youmonst.mappearance == GOLD_PIECE)
                                           ? "あなたを拾おうとした"
                                           : "無視した");
#endif
        else /* see note about m_monnam() above */
#if 0 /*JP*/
            pline("Wait, %s!  That %s is really %s named %s!", m_monnam(mtmp),
                  mimic_obj_name(&youmonst), an(mons[u.umonnum].mname),
                  plname);
#else
            pline("待て，%s！その%sは%sという名の%sだ！", m_monnam(mtmp),
                  mimic_obj_name(&youmonst), plname,
                  mons[u.umonnum].mname);
#endif
        if (multi < 0) { /* this should always be the case */
            char buf[BUFSZ];

#if 0 /*JP*/
            Sprintf(buf, "You appear to be %s again.",
                    Upolyd ? (const char *) an(youmonst.data->mname)
                           : (const char *) "yourself");
#else
            Sprintf(buf, "あなたはふたたび%sになった．",
                    Upolyd ? (const char *) youmonst.data->mname
                           : (const char *) "自分自身");
#endif
            unmul(buf); /* immediately stop mimicking */
        }
        return 0;
    }

    /*  Work out the armor class differential   */
    tmp = AC_VALUE(u.uac) + 10; /* tmp ~= 0 - 20 */
    tmp += mtmp->m_lev;
    if (multi < 0)
        tmp += 4;
    if ((Invis && !perceives(mdat)) || !mtmp->mcansee)
        tmp -= 2;
    if (mtmp->mtrapped)
        tmp -= 2;
    if (tmp <= 0)
        tmp = 1;

    /* make eels visible the moment they hit/miss us */
    if (mdat->mlet == S_EEL && mtmp->minvis && cansee(mtmp->mx, mtmp->my)) {
        mtmp->minvis = 0;
        newsym(mtmp->mx, mtmp->my);
    }

    /*  Special demon handling code */
    if ((mtmp->cham == NON_PM) && is_demon(mdat) && !range2
        && mtmp->data != &mons[PM_BALROG] && mtmp->data != &mons[PM_SUCCUBUS]
        && mtmp->data != &mons[PM_INCUBUS])
        if (!mtmp->mcan && !rn2(13))
            (void) msummon(mtmp);

    /*  Special lycanthrope handling code */
    if ((mtmp->cham == NON_PM) && is_were(mdat) && !range2) {
        if (is_human(mdat)) {
            if (!rn2(5 - (night() * 2)) && !mtmp->mcan)
                new_were(mtmp);
        } else if (!rn2(30) && !mtmp->mcan)
            new_were(mtmp);
        mdat = mtmp->data;

        if (!rn2(10) && !mtmp->mcan) {
            int numseen, numhelp;
#if 0 /*JP*/
            char buf[BUFSZ], genericwere[BUFSZ];
#else
            char genericwere[BUFSZ];
#endif

/*JP
            Strcpy(genericwere, "creature");
*/
            Strcpy(genericwere, "何か");
            numhelp = were_summon(mdat, FALSE, &numseen, genericwere);
            if (youseeit) {
/*JP
                pline("%s summons help!", Monnam(mtmp));
*/
                pline("%sは助けを呼んだ！", Monnam(mtmp));
                if (numhelp > 0) {
                    if (numseen == 0)
/*JP
                        You_feel("hemmed in.");
*/
                        pline("何者かに囲まれたような気がする．");
                } else
/*JP
                    pline("But none comes.");
*/
                    pline("しかし何も来なかった．");
            } else {
                const char *from_nowhere;

                if (!Deaf) {
/*JP
                    pline("%s %s!", Something, makeplural(growl_sound(mtmp)));
*/
                    pline("何かが%s！", growl_sound(mtmp));
                    from_nowhere = "";
                } else
/*JP
                    from_nowhere = " from nowhere";
*/
                    from_nowhere = "どこからともなく";
                if (numhelp > 0) {
                    if (numseen < 1)
/*JP
                        You_feel("hemmed in.");
*/
                        pline("何者かに囲まれたような気がする．");
                    else {
#if 0 /*JP*/
                        if (numseen == 1)
                            Sprintf(buf, "%s appears", an(genericwere));
                        else
                            Sprintf(buf, "%s appear",
                                    makeplural(genericwere));
                        pline("%s%s!", upstart(buf), from_nowhere);
#else
                        pline("%sが%s現れた！",
                              genericwere, from_nowhere);
#endif
                    }
                } /* else no help came; but you didn't know it tried */
            }
        }
    }

    if (u.uinvulnerable) {
        /* monsters won't attack you */
        if (mtmp == u.ustuck)
/*JP
            pline("%s loosens its grip slightly.", Monnam(mtmp));
*/
            pline("%sは締めつけをわずかに緩めた．", Monnam(mtmp));
        else if (!range2) {
            if (youseeit || sensemon(mtmp))
/*JP
                pline("%s starts to attack you, but pulls back.",
*/
                pline("%sはあなたを攻撃しかけたが，ひっこめた．",
                      Monnam(mtmp));
            else
/*JP
                You_feel("%s move nearby.", something);
*/
                pline("何者かがあなたのそばを通りぬけたような気がした．");
        }
        return 0;
    }

    /* Unlike defensive stuff, don't let them use item _and_ attack. */
    if (find_offensive(mtmp)) {
        int foo = use_offensive(mtmp);

        if (foo != 0)
            return (foo == 1);
    }

    for (i = 0; i < NATTK; i++) {
        sum[i] = 0;
        mon_currwep = (struct obj *)0;
        mattk = getmattk(mtmp, &youmonst, i, sum, &alt_attk);
        if ((u.uswallow && mattk->aatyp != AT_ENGL)
            || (skipnonmagc && mattk->aatyp != AT_MAGC))
            continue;

        switch (mattk->aatyp) {
        case AT_CLAW: /* "hand to hand" attacks */
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            if (!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict
                            || !touch_petrifies(youmonst.data))) {
                if (foundyou) {
                    if (tmp > (j = rnd(20 + i))) {
                        if (mattk->aatyp != AT_KICK
                            || !thick_skinned(youmonst.data))
                            sum[i] = hitmu(mtmp, mattk);
                    } else
                        missmu(mtmp, (tmp == j), mattk);
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            /* Note: if displaced, prev attacks never succeeded */
            if ((!range2 && i >= 2 && sum[i - 1] && sum[i - 2])
                || mtmp == u.ustuck)
                sum[i] = hitmu(mtmp, mattk);
            break;

        case AT_GAZE: /* can affect you either ranged or not */
            /* Medusa gaze already operated through m_respond in
               dochug(); don't gaze more than once per round. */
            if (mdat != &mons[PM_MEDUSA])
                sum[i] = gazemu(mtmp, mattk);
            break;

        case AT_EXPL: /* automatic hit if next to, and aimed at you */
            if (!range2)
                sum[i] = explmu(mtmp, mattk, foundyou);
            break;

        case AT_ENGL:
            if (!range2) {
                if (foundyou) {
                    if (u.uswallow
                        || (!mtmp->mspec_used && tmp > (j = rnd(20 + i)))) {
                        /* force swallowing monster to be displayed
                           even when hero is moving away */
                        flush_screen(1);
                        sum[i] = gulpmu(mtmp, mattk);
                    } else {
                        missmu(mtmp, (tmp == j), mattk);
                    }
                } else if (is_animal(mtmp->data)) {
/*JP
                    pline("%s gulps some air!", Monnam(mtmp));
*/
                    pline("%sは息を吸いこんだ！", Monnam(mtmp));
                } else {
                    if (youseeit)
/*JP
                        pline("%s lunges forward and recoils!", Monnam(mtmp));
*/
                        pline("%sは突進し，戻った！", Monnam(mtmp));
                    else
#if 0 /*JP*/
                        You_hear("a %s nearby.",
                                 is_whirly(mtmp->data) ? "rushing noise"
                                                       : "splat");
#else
                        You_hear("すぐそばで%s音を聞いた．",
                                 is_whirly(mtmp->data) ? "突撃してくる"
                                                       : "ピシャッという");
#endif
                }
            }
            break;
        case AT_BREA:
            if (range2)
                sum[i] = breamu(mtmp, mattk);
            /* Note: breamu takes care of displacement */
            break;
        case AT_SPIT:
            if (range2)
                sum[i] = spitmu(mtmp, mattk);
            /* Note: spitmu takes care of displacement */
            break;
        case AT_WEAP:
            if (range2) {
                if (!Is_rogue_level(&u.uz))
                    thrwmu(mtmp);
            } else {
                int hittmp = 0;

                /* Rare but not impossible.  Normally the monster
                 * wields when 2 spaces away, but it can be
                 * teleported or whatever....
                 */
                if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)) {
                    mtmp->weapon_check = NEED_HTH_WEAPON;
                    /* mon_wield_item resets weapon_check as appropriate */
                    if (mon_wield_item(mtmp) != 0)
                        break;
                }
                if (foundyou) {
                    mon_currwep = MON_WEP(mtmp);
                    if (mon_currwep) {
                        hittmp = hitval(mon_currwep, &youmonst);
                        tmp += hittmp;
                        mswings(mtmp, mon_currwep);
                    }
                    if (tmp > (j = dieroll = rnd(20 + i)))
                        sum[i] = hitmu(mtmp, mattk);
                    else
                        missmu(mtmp, (tmp == j), mattk);
                    /* KMH -- Don't accumulate to-hit bonuses */
                    if (mon_currwep)
                        tmp -= hittmp;
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;
        case AT_MAGC:
            if (range2)
                sum[i] = buzzmu(mtmp, mattk);
            else
                sum[i] = castmu(mtmp, mattk, TRUE, foundyou);
            break;

        default: /* no attack */
            break;
        }
        if (context.botl)
            bot();
        /* give player a chance of waking up before dying -kaa */
        if (sum[i] == 1) { /* successful attack */
            if (u.usleep && u.usleep < monstermoves && !rn2(10)) {
                multi = -1;
/*JP
                nomovemsg = "The combat suddenly awakens you.";
*/
                nomovemsg = "あなたは起こされた．";
            }
        }
        if (sum[i] == 2)
            return 1; /* attacker dead */
        if (sum[i] == 3)
            break; /* attacker teleported, no more attacks */
        /* sum[i] == 0: unsuccessful attack */
    }
    return 0;
}

STATIC_OVL boolean
diseasemu(mdat)
struct permonst *mdat;
{
    if (Sick_resistance) {
/*JP
        You_feel("a slight illness.");
*/
        You("すこし気分が悪くなったような気がした．");
        return FALSE;
    } else {
        make_sick(Sick ? Sick / 3L + 1L : (long) rn1(ACURR(A_CON), 20),
                  mdat->mname, TRUE, SICK_NONVOMITABLE);
        return TRUE;
    }
}

/* check whether slippery clothing protects from hug or wrap attack */
STATIC_OVL boolean
u_slip_free(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    struct obj *obj = (uarmc ? uarmc : uarm);

    if (!obj)
        obj = uarmu;
    if (mattk->adtyp == AD_DRIN)
        obj = uarmh;

    /* if your cloak/armor is greased, monster slips off; this
       protection might fail (33% chance) when the armor is cursed */
    if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK)
        && (!obj->cursed || rn2(3))) {
#if 0 /*JP*/
        pline("%s %s your %s %s!", Monnam(mtmp),
              (mattk->adtyp == AD_WRAP) ? "slips off of"
                                        : "grabs you, but cannot hold onto",
              obj->greased ? "greased" : "slippery",
              /* avoid "slippery slippery cloak"
                 for undiscovered oilskin cloak */
              (obj->greased || objects[obj->otyp].oc_name_known)
                  ? xname(obj)
                  : cloak_simple_name(obj));
#else
        pline("%sは%s%s%s！", Monnam(mtmp),
              obj->greased ? "油の塗られた" : "滑りやすい",
              (obj->greased || objects[obj->otyp].oc_name_known)
                  ? xname(obj)
                  : cloak_simple_name(obj),
              (mattk->adtyp == AD_WRAP) ? "で滑った"
                                        : "をつかまえようとしたが，できなかった");
#endif

        if (obj->greased && !rn2(2)) {
/*JP
            pline_The("grease wears off.");
*/
            pline("脂は落ちてしまった．");
            obj->greased = 0;
            update_inventory();
        }
        return TRUE;
    }
    return FALSE;
}

/* armor that sufficiently covers the body might be able to block magic */
int
magic_negation(mon)
struct monst *mon;
{
    struct obj *o;
    long wearmask;
    int armpro, mc = 0;
    boolean is_you = (mon == &youmonst),
            gotprot = is_you ? (EProtection != 0L)
                             /* high priests have innate protection */
                             : (mon->data == &mons[PM_HIGH_PRIEST]);

    for (o = is_you ? invent : mon->minvent; o; o = o->nobj) {
        /* a_can field is only applicable for armor (which must be worn) */
        if ((o->owornmask & W_ARMOR) != 0L) {
            armpro = objects[o->otyp].a_can;
            if (armpro > mc)
                mc = armpro;
        }
        /* if we've already confirmed Protection, skip additional checks */
        if (is_you || gotprot)
            continue;

        /* omit W_SWAPWEP+W_QUIVER; W_ART+W_ARTI handled by protects() */
        wearmask = W_ARMOR | W_ACCESSORY;
        if (o->oclass == WEAPON_CLASS || is_weptool(o))
            wearmask |= W_WEP;
        if (protects(o, ((o->owornmask & wearmask) != 0L) ? TRUE : FALSE))
            gotprot = TRUE;
    }

    if (gotprot) {
        /* extrinsic Protection increases mc by 1 */
        if (mc < 3)
            mc += 1;
    } else if (mc < 1) {
        /* intrinsic Protection is weaker (play balance; obtaining divine
           protection is too easy); it confers minimum mc 1 instead of 0 */
        if ((is_you && ((HProtection && u.ublessed > 0) || u.uspellprot))
            /* aligned priests and angels have innate intrinsic Protection */
            || (mon->data == &mons[PM_ALIGNED_PRIEST] || is_minion(mon->data)))
            mc = 1;
    }
    return mc;
}

/*
 * hitmu: monster hits you
 *        returns 2 if monster dies (e.g. "yellow light"), 1 otherwise
 *        3 if the monster lives but teleported/paralyzed, so it can't keep
 *             attacking you
 */
STATIC_OVL int
hitmu(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
    struct permonst *mdat = mtmp->data;
    int uncancelled, ptmp;
    int dmg, armpro, permdmg;
    char buf[BUFSZ];
    struct permonst *olduasmon = youmonst.data;
    int res;

    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    /*  If the monster is undetected & hits you, you should know where
     *  the attack came from.
     */
    if (mtmp->mundetected && (hides_under(mdat) || mdat->mlet == S_EEL)) {
        mtmp->mundetected = 0;
        if (!(Blind ? Blind_telepat : Unblind_telepat)) {
            struct obj *obj;
#if 0 /*JP*/
            const char *what;
#endif

            if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0) {
#if 0 /*JP*/
                if (Blind && !obj->dknown)
                    what = something;
                else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
                    what = "the water";
                else
                    what = doname(obj);

                pline("%s was hidden under %s!", Amonnam(mtmp), what);
#else
                if (Blind && !obj->dknown)
                    pline("%sが何かの下に隠れている！", Amonnam(mtmp));
                else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
                    pline("%sが水中に隠れている！", Amonnam(mtmp));
                else
                    pline("%sが%sの下に隠れている！", Amonnam(mtmp), doname(obj));
#endif
            }
            newsym(mtmp->mx, mtmp->my);
        }
    }

    /*  First determine the base damage done */
    dmg = d((int) mattk->damn, (int) mattk->damd);
    if ((is_undead(mdat) || is_vampshifter(mtmp)) && midnight())
        dmg += d((int) mattk->damn, (int) mattk->damd); /* extra damage */

    /*  Next a cancellation factor.
     *  Use uncancelled when cancellation factor takes into account certain
     *  armor's special magic protection.  Otherwise just use !mtmp->mcan.
     */
    armpro = magic_negation(&youmonst);
    uncancelled = !mtmp->mcan && (rn2(10) >= 3 * armpro);

    permdmg = 0;
    /*  Now, adjust damages via resistances or specific attacks */
    switch (mattk->adtyp) {
    case AD_PHYS:
        if (mattk->aatyp == AT_HUGS && !sticks(youmonst.data)) {
            if (!u.ustuck && rn2(2)) {
                if (u_slip_free(mtmp, mattk)) {
                    dmg = 0;
                } else {
                    u.ustuck = mtmp;
/*JP
                    pline("%s grabs you!", Monnam(mtmp));
*/
                    pline("%sにつかまえられている！", Monnam(mtmp));
                }
            } else if (u.ustuck == mtmp) {
                exercise(A_STR, FALSE);
#if 0 /*JP*/
                You("are being %s.", (mtmp->data == &mons[PM_ROPE_GOLEM])
                                         ? "choked"
                                         : "crushed");
#else
                You("%sている．", (mtmp->data == &mons[PM_ROPE_GOLEM])
                                         ? "首を絞められ"
                                         : "押しつぶされ");
#endif
            }
        } else { /* hand to hand weapon */
            struct obj *otmp = mon_currwep;

            if (mattk->aatyp == AT_WEAP && otmp) {
                struct obj *marmg;
                int tmp;

                if (otmp->otyp == CORPSE
                    && touch_petrifies(&mons[otmp->corpsenm])) {
                    dmg = 1;
#if 0 /*JP*/
                    pline("%s hits you with the %s corpse.", Monnam(mtmp),
                          mons[otmp->corpsenm].mname);
#else
                    pline("%sは%sの死体で攻撃した．", Monnam(mtmp),
                          mons[otmp->corpsenm].mname);
#endif
                    if (!Stoned)
                        goto do_stone;
                }
                dmg += dmgval(otmp, &youmonst);
                if ((marmg = which_armor(mtmp, W_ARMG)) != 0
                    && marmg->otyp == GAUNTLETS_OF_POWER)
                    dmg += rn1(4, 3); /* 3..6 */
                if (dmg <= 0)
                    dmg = 1;
                if (!(otmp->oartifact
                      && artifact_hit(mtmp, &youmonst, otmp, &dmg, dieroll)))
                    hitmsg(mtmp, mattk);
                if (!dmg)
                    break;
                if (objects[otmp->otyp].oc_material == SILVER
                    && Hate_silver) {
/*JP
                    pline_The("silver sears your flesh!");
*/
                    pline("あなたの体は銀で焼かれた！");
                    exercise(A_CON, FALSE);
                }
                /* this redundancy necessary because you have
                   to take the damage _before_ being cloned;
                   need to have at least 2 hp left to split */
                tmp = dmg;
                if (u.uac < 0)
                    tmp -= rnd(-u.uac);
                if (tmp < 1)
                    tmp = 1;
                if (u.mh - tmp > 1
                    && (objects[otmp->otyp].oc_material == IRON
                        /* relevant 'metal' objects are scalpel and tsurugi */
                        || objects[otmp->otyp].oc_material == METAL)
                    && (u.umonnum == PM_BLACK_PUDDING
                        || u.umonnum == PM_BROWN_PUDDING)) {
                    if (tmp > 1)
                        exercise(A_STR, FALSE);
                    /* inflict damage now; we know it can't be fatal */
                    u.mh -= tmp;
                    context.botl = 1;
                    dmg = 0; /* don't inflict more damage below */
                    if (cloneu())
/*JP
                        You("divide as %s hits you!", mon_nam(mtmp));
*/
                        pline("%sの攻撃によってあなたは分裂した！", mon_nam(mtmp));
                }
                rustm(&youmonst, otmp);
            } else if (mattk->aatyp != AT_TUCH || dmg != 0
                       || mtmp != u.ustuck)
                hitmsg(mtmp, mattk);
        }
        break;
    case AD_DISE:
        hitmsg(mtmp, mattk);
        if (!diseasemu(mdat))
            dmg = 0;
        break;
    case AD_FIRE:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
/*JP
            pline("You're %s!", on_fire(youmonst.data, mattk));
*/
            pline("あなたは%s！", on_fire(youmonst.data, mattk));
            if (completelyburns(youmonst.data)) { /* paper or straw golem */
/*JP
                You("go up in flames!");
*/
                You("炎に包まれた！");
                /* KMH -- this is okay with unchanging */
                rehumanize();
                break;
            } else if (Fire_resistance) {
/*JP
                pline_The("fire doesn't feel hot!");
*/
                pline("火はぜんぜん熱くない！");
                dmg = 0;
            }
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(SCROLL_CLASS, AD_FIRE);
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(POTION_CLASS, AD_FIRE);
            if ((int) mtmp->m_lev > rn2(25))
                destroy_item(SPBOOK_CLASS, AD_FIRE);
            burn_away_slime();
        } else
            dmg = 0;
        break;
    case AD_COLD:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
/*JP
            pline("You're covered in frost!");
*/
            You("氷で覆われた！");
            if (Cold_resistance) {
/*JP
                pline_The("frost doesn't seem cold!");
*/
                pline("氷は冷さを感じさせない！");
                dmg = 0;
            }
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(POTION_CLASS, AD_COLD);
        } else
            dmg = 0;
        break;
    case AD_ELEC:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
/*JP
            You("get zapped!");
*/
            You("電撃をくらった！");
            if (Shock_resistance) {
/*JP
                pline_The("zap doesn't shock you!");
*/
                pline("電撃はしびれを感じさせない！");
                dmg = 0;
            }
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(WAND_CLASS, AD_ELEC);
            if ((int) mtmp->m_lev > rn2(20))
                destroy_item(RING_CLASS, AD_ELEC);
        } else
            dmg = 0;
        break;
    case AD_SLEE:
        hitmsg(mtmp, mattk);
        if (uncancelled && multi >= 0 && !rn2(5)) {
            if (Sleep_resistance)
                break;
            fall_asleep(-rnd(10), TRUE);
            if (Blind)
/*JP
                You("are put to sleep!");
*/
                You("眠りにおちた！");
            else
/*JP
                You("are put to sleep by %s!", mon_nam(mtmp));
*/
                You("%sに眠らされた！", mon_nam(mtmp));
        }
        break;
    case AD_BLND:
        if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind)
/*JP
                pline("%s blinds you!", Monnam(mtmp));
*/
                pline("%sの攻撃で目が見えなくなった！", Monnam(mtmp));
            make_blinded(Blinded + (long) dmg, FALSE);
            if (!Blind)
                Your1(vision_clears);
        }
        dmg = 0;
        break;
    case AD_DRST:
        ptmp = A_STR;
        goto dopois;
    case AD_DRDX:
        ptmp = A_DEX;
        goto dopois;
    case AD_DRCO:
        ptmp = A_CON;
    dopois:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(8)) {
#if 0 /*JP*/
            Sprintf(buf, "%s %s", s_suffix(Monnam(mtmp)),
                    mpoisons_subj(mtmp, mattk));
#else
            Sprintf(buf, "%sの%s", Monnam(mtmp),
                    mpoisons_subj(mtmp, mattk));
#endif
            poisoned(buf, ptmp, mdat->mname, 30, FALSE);
        }
        break;
    case AD_DRIN:
        hitmsg(mtmp, mattk);
        if (defends(AD_DRIN, uwep) || !has_head(youmonst.data)) {
/*JP
            You("don't seem harmed.");
*/
            You("傷ついていないようだ．");
            /* Not clear what to do for green slimes */
            break;
        }
        if (u_slip_free(mtmp, mattk))
            break;

        if (uarmh && rn2(8)) {
            /* not body_part(HEAD) */
/*JP
            Your("%s blocks the attack to your head.",
*/
            Your("%sが頭への攻撃を防いだ．",
                 helm_simple_name(uarmh));
            break;
        }
        /* negative armor class doesn't reduce this damage */
        if (Half_physical_damage)
            dmg = (dmg + 1) / 2;
        mdamageu(mtmp, dmg);
        dmg = 0; /* don't inflict a second dose below */

        if (!uarmh || uarmh->otyp != DUNCE_CAP) {
            /* eat_brains() will miss if target is mindless (won't
               happen here; hero is considered to retain his mind
               regardless of current shape) or is noncorporeal
               (can't happen here; no one can poly into a ghost
               or shade) so this check for missing is academic */
            if (eat_brains(mtmp, &youmonst, TRUE, (int *) 0) == MM_MISS)
                break;
        }
        /* adjattrib gives dunce cap message when appropriate */
        (void) adjattrib(A_INT, -rnd(2), FALSE);
        forget_levels(25);  /* lose memory of 25% of levels */
        forget_objects(25); /* lose memory of 25% of objects */
        break;
    case AD_PLYS:
        hitmsg(mtmp, mattk);
        if (uncancelled && multi >= 0 && !rn2(3)) {
            if (Free_action) {
/*JP
                You("momentarily stiffen.");
*/
                You("一瞬硬直した．");
            } else {
                if (Blind)
/*JP
                    You("are frozen!");
*/
                    You("動けない！");
                else
/*JP
                    You("are frozen by %s!", mon_nam(mtmp));
*/
                    pline("%sによって動けなくなった！", mon_nam(mtmp));
                nomovemsg = You_can_move_again;
                nomul(-rnd(10));
/*JP
                multi_reason = "paralyzed by a monster";
*/
                multi_reason = "怪物に麻痺させられた隙に";
                exercise(A_DEX, FALSE);
            }
        }
        break;
    case AD_DRLI:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(3) && !Drain_resistance) {
/*JP
            losexp("life drainage");
*/
            losexp("生命力を吸収されて");
        }
        break;
    case AD_LEGS: {
        long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
/*JP
        const char *sidestr = (side == RIGHT_SIDE) ? "right" : "left",
*/
        const char *sidestr = (side == RIGHT_SIDE) ? "右" : "左",
                   *Monst_name = Monnam(mtmp), *leg = body_part(LEG);

        /* This case is too obvious to ignore, but Nethack is not in
         * general very good at considering height--most short monsters
         * still _can_ attack you when you're flying or mounted.
         * [FIXME: why can't a flying attacker overcome this?]
         */
        if (u.usteed || Levitation || Flying) {
/*JP
            pline("%s tries to reach your %s %s!", Monst_name, sidestr, leg);
*/
            pline("%sはあなたの%s%sに攻撃しようとした！", Monst_name, sidestr, leg);
            dmg = 0;
        } else if (mtmp->mcan) {
#if 0 /*JP:T*/
            pline("%s nuzzles against your %s %s!", Monnam(mtmp),
                  sidestr, leg);
#else
            pline("%sはあなたの%s%sに鼻をすりよせた！", Monnam(mtmp),
                  sidestr, leg);
#endif
            dmg = 0;
        } else {
            if (uarmf) {
                if (rn2(2) && (uarmf->otyp == LOW_BOOTS
                               || uarmf->otyp == IRON_SHOES)) {
#if 0 /*JP:T*/
                    pline("%s pricks the exposed part of your %s %s!",
                          Monst_name, sidestr, leg);
#else
                    pline("%sはあなたの%s%sをちくりと刺した！",
                          Monst_name, sidestr, leg);
#endif
                } else if (!rn2(5)) {
#if 0 /*JP*/
                    pline("%s pricks through your %s boot!", Monst_name,
                          sidestr);
#else
                    pline("%sはあなたの%sの靴ごしにちくりと刺した！", Monst_name,
                          sidestr);
#endif
                } else {
#if 0 /*JP*/
                    pline("%s scratches your %s boot!", Monst_name,
                          sidestr);
#else
                    pline("%sはあなたの%sの靴をひっかいた！", Monst_name,
                          sidestr);
#endif
                    dmg = 0;
                    break;
                }
            } else
/*JP
                pline("%s pricks your %s %s!", Monst_name, sidestr, leg);
*/
                pline("%sはあなたの%s%sをちくりと刺した！", Monst_name, sidestr, leg);

            set_wounded_legs(side, rnd(60 - ACURR(A_DEX)));
            exercise(A_STR, FALSE);
            exercise(A_DEX, FALSE);
        }
        break;
    }
    case AD_STON: /* cockatrice */
        hitmsg(mtmp, mattk);
        if (!rn2(3)) {
            if (mtmp->mcan) {
                if (!Deaf)
/*JP
                    You_hear("a cough from %s!", mon_nam(mtmp));
*/
                    You_hear("%sがゴホッゴホッという音を聞いた！", mon_nam(mtmp));
            } else {
                if (!Deaf)
/*JP
                    You_hear("%s hissing!", s_suffix(mon_nam(mtmp)));
*/
                    You_hear("%sがシューッという声を聞いた！", mon_nam(mtmp));
                if (!rn2(10)
                    || (flags.moonphase == NEW_MOON && !have_lizard())) {
                do_stone:
                    if (!Stoned && !Stone_resistance
                        && !(poly_when_stoned(youmonst.data)
                             && polymon(PM_STONE_GOLEM))) {
                        int kformat = KILLED_BY_AN;
                        const char *kname = mtmp->data->mname;

                        if (mtmp->data->geno & G_UNIQ) {
                            if (!type_is_pname(mtmp->data))
                                kname = the(kname);
                            kformat = KILLED_BY;
                        }
                        make_stoned(5L, (char *) 0, kformat, kname);
                        return 1;
                        /* done_in_by(mtmp, STONING); */
                    }
                }
            }
        }
        break;
    case AD_STCK:
        hitmsg(mtmp, mattk);
        if (uncancelled && !u.ustuck && !sticks(youmonst.data))
            u.ustuck = mtmp;
        break;
    case AD_WRAP:
        if ((!mtmp->mcan || u.ustuck == mtmp) && !sticks(youmonst.data)) {
            if (!u.ustuck && !rn2(10)) {
                if (u_slip_free(mtmp, mattk)) {
                    dmg = 0;
                } else {
/*JP
                    pline("%s swings itself around you!", Monnam(mtmp));
*/
                    pline("%sはあなたに体を絡みつかせた！", Monnam(mtmp));
                    u.ustuck = mtmp;
                }
            } else if (u.ustuck == mtmp) {
                if (is_pool(mtmp->mx, mtmp->my) && !Swimming && !Amphibious) {
                    boolean moat = (levl[mtmp->mx][mtmp->my].typ != POOL)
                                   && (levl[mtmp->mx][mtmp->my].typ != WATER)
                                   && !Is_medusa_level(&u.uz)
                                   && !Is_waterlevel(&u.uz);

/*JP
                    pline("%s drowns you...", Monnam(mtmp));
*/
                    pline("あなたは%sに絡みつかれて溺れた．．．", Monnam(mtmp));
                    killer.format = KILLED_BY_AN;
#if 0 /*JP*/
                    Sprintf(killer.name, "%s by %s",
                            moat ? "moat" : "pool of water",
                            an(mtmp->data->mname));
#else
                    Sprintf(buf, "%sの%sに絡みつかれて",
                            moat ? "堀" : "池",
                            mtmp->data->mname);
#endif
                    done(DROWNING);
                } else if (mattk->aatyp == AT_HUGS)
/*JP
                    You("are being crushed.");
*/
                    You("つぶされつつある．");
            } else {
                dmg = 0;
                if (flags.verbose)
#if 0 /*JP*/
                    pline("%s brushes against your %s.", Monnam(mtmp),
                          body_part(LEG));
#else
                    pline("%sはあなたの%sに触れた．", Monnam(mtmp),
                          body_part(LEG));
#endif
            }
        } else
            dmg = 0;
        break;
    case AD_WERE:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(4) && u.ulycn == NON_PM
            && !Protection_from_shape_changers && !defends(AD_WERE, uwep)) {
/*JP
            You_feel("feverish.");
*/
            You("熱があるような気がした．");
            exercise(A_CON, FALSE);
            set_ulycn(monsndx(mdat));
            retouch_equipment(2);
        }
        break;
    case AD_SGLD:
        hitmsg(mtmp, mattk);
        if (youmonst.data->mlet == mdat->mlet)
            break;
        if (!mtmp->mcan)
            stealgold(mtmp);
        break;

    case AD_SSEX:
        if (SYSOPT_SEDUCE) {
            if (could_seduce(mtmp, &youmonst, mattk) == 1 && !mtmp->mcan)
                if (doseduce(mtmp))
                    return 3;
            break;
        }
        /*FALLTHRU*/
    case AD_SITM: /* for now these are the same */
    case AD_SEDU:
        if (is_animal(mtmp->data)) {
            hitmsg(mtmp, mattk);
            if (mtmp->mcan)
                break;
            /* Continue below */
        } else if (dmgtype(youmonst.data, AD_SEDU)
                   || (SYSOPT_SEDUCE && dmgtype(youmonst.data, AD_SSEX))) {
#if 0 /*JP:T*/
            pline("%s %s.", Monnam(mtmp),
                  mtmp->minvent
                      ? "brags about the goods some dungeon explorer provided"
                  : "makes some remarks about how difficult theft is lately");
#else
            pline("%sは%s．", Monnam(mtmp),
                  mtmp->minvent
                      ? "ある迷宮探検家が置いてった品物を自慢した"
                      : "最近窃盗がいかに困難か淡々と述べた");
#endif
            if (!tele_restrict(mtmp))
                (void) rloc(mtmp, TRUE);
            return 3;
        } else if (mtmp->mcan) {
            if (!Blind)
#if 0 /*JP*/
                pline("%s tries to %s you, but you seem %s.",
                      Adjmonnam(mtmp, "plain"),
                      flags.female ? "charm" : "seduce",
                      flags.female ? "unaffected" : "uninterested");
#else
                pline("%sはあなたを%sしようとした，しかしあなたは%s．",
                      Adjmonnam(mtmp, "地味な"),
                      flags.female ? "魅了" : "誘惑",
                      flags.female ? "影響を受けない" : "興味がない");
#endif
            if (rn2(3)) {
                if (!tele_restrict(mtmp))
                    (void) rloc(mtmp, TRUE);
                return 3;
            }
            break;
        }
        buf[0] = '\0';
        switch (steal(mtmp, buf)) {
        case -1:
            return 2;
        case 0:
            break;
        default:
            if (!is_animal(mtmp->data) && !tele_restrict(mtmp))
                (void) rloc(mtmp, TRUE);
            if (is_animal(mtmp->data) && *buf) {
                if (canseemon(mtmp))
#if 0 /*JP*/
                    pline("%s tries to %s away with %s.", Monnam(mtmp),
                          locomotion(mtmp->data, "run"), buf);
#else
                    pline("%sは%sを持って逃げ出した．", Monnam(mtmp),
                          buf);
#endif
            }
            monflee(mtmp, 0, FALSE, FALSE);
            return 3;
        }
        break;

    case AD_SAMU:
        hitmsg(mtmp, mattk);
        /* when the Wizard or quest nemesis hits, there's a 1/20 chance
           to steal a quest artifact (any, not just the one for the hero's
           own role) or the Amulet or one of the invocation tools */
        if (!rn2(20))
            stealamulet(mtmp);
        break;

    case AD_TLPT:
        hitmsg(mtmp, mattk);
        if (uncancelled) {
            if (flags.verbose)
/*JP
                Your("position suddenly seems very uncertain!");
*/
                pline("自分のいる位置が突然不明確になった！");
            tele();
        }
        break;
    case AD_RUST:
        hitmsg(mtmp, mattk);
        if (mtmp->mcan)
            break;
        if (u.umonnum == PM_IRON_GOLEM) {
/*JP
            You("rust!");
*/
            You("錆びついた！");
            /* KMH -- this is okay with unchanging */
            rehumanize();
            break;
        }
        erode_armor(&youmonst, ERODE_RUST);
        break;
    case AD_CORR:
        hitmsg(mtmp, mattk);
        if (mtmp->mcan)
            break;
        erode_armor(&youmonst, ERODE_CORRODE);
        break;
    case AD_DCAY:
        hitmsg(mtmp, mattk);
        if (mtmp->mcan)
            break;
        if (u.umonnum == PM_WOOD_GOLEM || u.umonnum == PM_LEATHER_GOLEM) {
/*JP
            You("rot!");
*/
            You("腐った！");
            /* KMH -- this is okay with unchanging */
            rehumanize();
            break;
        }
        erode_armor(&youmonst, ERODE_ROT);
        break;
    case AD_HEAL:
        /* a cancelled nurse is just an ordinary monster,
         * nurses don't heal those that cause petrification */
        if (mtmp->mcan || (Upolyd && touch_petrifies(youmonst.data))) {
            hitmsg(mtmp, mattk);
            break;
        }
        if (!uwep && !uarmu && !uarm && !uarmc
            && !uarms && !uarmg && !uarmf && !uarmh) {
            boolean goaway = FALSE;

/*JP
            pline("%s hits!  (I hope you don't mind.)", Monnam(mtmp));
*/
            pline("%sの攻撃は命中した．(気にしないように．)", Monnam(mtmp));
            if (Upolyd) {
                u.mh += rnd(7);
                if (!rn2(7)) {
                    /* no upper limit necessary; effect is temporary */
                    u.mhmax++;
                    if (!rn2(13))
                        goaway = TRUE;
                }
                if (u.mh > u.mhmax)
                    u.mh = u.mhmax;
            } else {
                u.uhp += rnd(7);
                if (!rn2(7)) {
                    /* hard upper limit via nurse care: 25 * ulevel */
                    if (u.uhpmax < 5 * u.ulevel + d(2 * u.ulevel, 10))
                        u.uhpmax++;
                    if (!rn2(13))
                        goaway = TRUE;
                }
                if (u.uhp > u.uhpmax)
                    u.uhp = u.uhpmax;
            }
            if (!rn2(3))
                exercise(A_STR, TRUE);
            if (!rn2(3))
                exercise(A_CON, TRUE);
            if (Sick)
                make_sick(0L, (char *) 0, FALSE, SICK_ALL);
            context.botl = 1;
            if (goaway) {
                mongone(mtmp);
                return 2;
            } else if (!rn2(33)) {
                if (!tele_restrict(mtmp))
                    (void) rloc(mtmp, TRUE);
                monflee(mtmp, d(3, 6), TRUE, FALSE);
                return 3;
            }
            dmg = 0;
        } else {
            if (Role_if(PM_HEALER)) {
                if (!Deaf && !(moves % 5))
/*JP
                    verbalize("Doc, I can't help you unless you cooperate.");
*/
                    verbalize("ドクター！協力をおねがいしますわ．");
                dmg = 0;
            } else
                hitmsg(mtmp, mattk);
        }
        break;
    case AD_CURS:
        hitmsg(mtmp, mattk);
        if (!night() && mdat == &mons[PM_GREMLIN])
            break;
        if (!mtmp->mcan && !rn2(10)) {
            if (!Deaf) {
                if (Blind)
/*JP
                    You_hear("laughter.");
*/
                    You_hear("笑い声を聞いた．");
                else
/*JP
                    pline("%s chuckles.", Monnam(mtmp));
*/
                    pline("%sはクスクス笑った．", Monnam(mtmp));
            }
            if (u.umonnum == PM_CLAY_GOLEM) {
/*JP
                pline("Some writing vanishes from your head!");
*/
                pline("いくつかの文字があなたの頭から消えた！");
                /* KMH -- this is okay with unchanging */
                rehumanize();
                break;
            }
            attrcurse();
        }
        break;
    case AD_STUN:
        hitmsg(mtmp, mattk);
        if (!mtmp->mcan && !rn2(4)) {
            make_stunned((HStun & TIMEOUT) + (long) dmg, TRUE);
            dmg /= 2;
        }
        break;
    case AD_ACID:
        hitmsg(mtmp, mattk);
        if (!mtmp->mcan && !rn2(3))
            if (Acid_resistance) {
#if 0 /*JP:T*/
                pline("You're covered in %s, but it seems harmless.",
                      hliquid("acid"));
#else
                pline("%sで覆われた．しかし傷つかない．",
                      hliquid("酸"));
#endif
                dmg = 0;
            } else {
/*JP
                pline("You're covered in %s!  It burns!", hliquid("acid"));
*/
                pline("%sで覆われ焼けた！", hliquid("酸"));
                exercise(A_STR, FALSE);
            }
        else
            dmg = 0;
        break;
    case AD_SLOW:
        hitmsg(mtmp, mattk);
        if (uncancelled && HFast && !defends(AD_SLOW, uwep) && !rn2(4))
            u_slow_down();
        break;
    case AD_DREN:
        hitmsg(mtmp, mattk);
        if (uncancelled && !rn2(4)) /* 25% chance */
            drain_en(dmg);
        dmg = 0;
        break;
    case AD_CONF:
        hitmsg(mtmp, mattk);
        if (!mtmp->mcan && !rn2(4) && !mtmp->mspec_used) {
            mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
            if (Confusion)
/*JP
                You("are getting even more confused.");
*/
                You("ますます混乱した．");
            else
/*JP
                You("are getting confused.");
*/
                You("混乱してきた．");
            make_confused(HConfusion + dmg, FALSE);
        }
        dmg = 0;
        break;
    case AD_DETH:
/*JP
        pline("%s reaches out with its deadly touch.", Monnam(mtmp));
*/
        pline("%sは死の腕をのばした．", Monnam(mtmp));
        if (is_undead(youmonst.data)) {
            /* Still does normal damage */
/*JP
            pline("Was that the touch of death?");
*/
            pline("今のは死の宣告だったのかな？");
            break;
        }
        switch (rn2(20)) {
        case 19:
        case 18:
        case 17:
            if (!Antimagic) {
                killer.format = KILLED_BY_AN;
/*JP
                Strcpy(killer.name, "touch of death");
*/
                Strcpy(killer.name, "死の宣告で");
                done(DIED);
                dmg = 0;
                break;
            }
            /*FALLTHRU*/
        default: /* case 16: ... case 5: */
/*JP
            You_feel("your life force draining away...");
*/
            You("体力が奪われていくような気がした．．．");
            permdmg = 1; /* actual damage done below */
            break;
        case 4:
        case 3:
        case 2:
        case 1:
        case 0:
            if (Antimagic)
                shieldeff(u.ux, u.uy);
/*JP
            pline("Lucky for you, it didn't work!");
*/
            pline("運のよいことになんともなかった！");
            dmg = 0;
            break;
        }
        break;
    case AD_PEST:
/*JP
        pline("%s reaches out, and you feel fever and chills.", Monnam(mtmp));
*/
        pline("%sは腕をのばした，あなたは悪寒を感じた．", Monnam(mtmp));
        (void) diseasemu(mdat); /* plus the normal damage */
        break;
    case AD_FAMN:
/*JP
        pline("%s reaches out, and your body shrivels.", Monnam(mtmp));
*/
        pline("%sは腕を伸ばした，あなたの体はしなびた．", Monnam(mtmp));
        exercise(A_CON, FALSE);
        if (!is_fainted())
            morehungry(rn1(40, 40));
        /* plus the normal damage */
        break;
    case AD_SLIM:
        hitmsg(mtmp, mattk);
        if (!uncancelled)
            break;
        if (flaming(youmonst.data)) {
/*JP
            pline_The("slime burns away!");
*/
            pline_The("スライムは燃えた！");
            dmg = 0;
        } else if (Unchanging || noncorporeal(youmonst.data)
                   || youmonst.data == &mons[PM_GREEN_SLIME]) {
/*JP
            You("are unaffected.");
*/
            You("影響を受けない．");
            dmg = 0;
        } else if (!Slimed) {
/*JP
            You("don't feel very well.");
*/
            You("気分が悪い．");
            make_slimed(10L, (char *) 0);
            delayed_killer(SLIMED, KILLED_BY_AN, mtmp->data->mname);
        } else
/*JP
            pline("Yuck!");
*/
            pline("ウゲェー！");
        break;
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        hitmsg(mtmp, mattk);
        /* uncancelled is sufficient enough; please
           don't make this attack less frequent */
        if (uncancelled) {
            struct obj *obj = some_armor(&youmonst);

            if (!obj) {
                /* some rings are susceptible;
                   amulets and blindfolds aren't (at present) */
                switch (rn2(5)) {
                case 0:
                    break;
                case 1:
                    obj = uright;
                    break;
                case 2:
                    obj = uleft;
                    break;
                case 3:
                    obj = uamul;
                    break;
                case 4:
                    obj = ublindf;
                    break;
                }
            }
            if (drain_item(obj, FALSE)) {
/*JP
                pline("%s less effective.", Yobjnam2(obj, "seem"));
*/
                Your("%sから魔力が消えたようだ．", xname(obj));
            }
        }
        break;
    default:
        dmg = 0;
        break;
    }
    if ((Upolyd ? u.mh : u.uhp) < 1) {
        /* already dead? call rehumanize() or done_in_by() as appropriate */
        mdamageu(mtmp, 1);
        dmg = 0;
    }

    /*  Negative armor class reduces damage done instead of fully protecting
     *  against hits.
     */
    if (dmg && u.uac < 0) {
        dmg -= rnd(-u.uac);
        if (dmg < 1)
            dmg = 1;
    }

    if (dmg) {
        if (Half_physical_damage
            /* Mitre of Holiness */
            || (Role_if(PM_PRIEST) && uarmh && is_quest_artifact(uarmh)
                && (is_undead(mtmp->data) || is_demon(mtmp->data)
                    || is_vampshifter(mtmp))))
            dmg = (dmg + 1) / 2;

        if (permdmg) { /* Death's life force drain */
            int lowerlimit, *hpmax_p;
            /*
             * Apply some of the damage to permanent hit points:
             *  polymorphed         100% against poly'd hpmax
             *  hpmax > 25*lvl      100% against normal hpmax
             *  hpmax > 10*lvl  50..100%
             *  hpmax >  5*lvl  25..75%
             *  otherwise        0..50%
             * Never reduces hpmax below 1 hit point per level.
             */
            permdmg = rn2(dmg / 2 + 1);
            if (Upolyd || u.uhpmax > 25 * u.ulevel)
                permdmg = dmg;
            else if (u.uhpmax > 10 * u.ulevel)
                permdmg += dmg / 2;
            else if (u.uhpmax > 5 * u.ulevel)
                permdmg += dmg / 4;

            if (Upolyd) {
                hpmax_p = &u.mhmax;
                /* [can't use youmonst.m_lev] */
                lowerlimit = min((int) youmonst.data->mlevel, u.ulevel);
            } else {
                hpmax_p = &u.uhpmax;
                lowerlimit = u.ulevel;
            }
            if (*hpmax_p - permdmg > lowerlimit)
                *hpmax_p -= permdmg;
            else if (*hpmax_p > lowerlimit)
                *hpmax_p = lowerlimit;
            /* else unlikely...
             * already at or below minimum threshold; do nothing */
            context.botl = 1;
        }

        mdamageu(mtmp, dmg);
    }

    if (dmg)
        res = passiveum(olduasmon, mtmp, mattk);
    else
        res = 1;
    stop_occupation();
    return res;
}

/* An interface for use when taking a blindfold off, for example,
 * to see if an engulfing attack should immediately take affect, like
 * a passive attack. TRUE if engulfing blindness occurred */
boolean
gulp_blnd_check()
{
    struct attack *mattk;

    if (!Blinded && u.uswallow
        && (mattk = attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND))
        && can_blnd(u.ustuck, &youmonst, mattk->aatyp, (struct obj *) 0)) {
        ++u.uswldtim; /* compensate for gulpmu change */
        (void) gulpmu(u.ustuck, mattk);
        return TRUE;
    }
    return FALSE;
}

/* monster swallows you, or damage if u.uswallow */
STATIC_OVL int
gulpmu(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    struct trap *t = t_at(u.ux, u.uy);
    int tmp = d((int) mattk->damn, (int) mattk->damd);
    int tim_tmp;
    struct obj *otmp2;
    int i;
    boolean physical_damage = FALSE;

    if (!u.uswallow) { /* swallows you */
        int omx = mtmp->mx, omy = mtmp->my;

        if (!engulf_target(mtmp, &youmonst))
            return 0;
        if ((t && ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT)))
            && sobj_at(BOULDER, u.ux, u.uy))
            return 0;

        if (Punished)
            unplacebc(); /* ball&chain go away */
        remove_monster(omx, omy);
        mtmp->mtrapped = 0; /* no longer on old trap */
        place_monster(mtmp, u.ux, u.uy);
        u.ustuck = mtmp;
        newsym(mtmp->mx, mtmp->my);
        if (is_animal(mtmp->data) && u.usteed) {
            char buf[BUFSZ];

            /* Too many quirks presently if hero and steed
             * are swallowed. Pretend purple worms don't
             * like horses for now :-)
             */
            Strcpy(buf, mon_nam(u.usteed));
#if 0 /*JP*/
            pline("%s lunges forward and plucks you off %s!", Monnam(mtmp),
                  buf);
#else
            pline("%sは突進しあなたを%sから引き落した！", Monnam(mtmp),
                  buf);
#endif
            dismount_steed(DISMOUNT_ENGULFED);
        } else
/*JP
            pline("%s engulfs you!", Monnam(mtmp));
*/
            pline("%sはあなたを飲みこんだ！", Monnam(mtmp));
        stop_occupation();
        reset_occupations(); /* behave as if you had moved */

        if (u.utrap) {
#if 0 /*JP:T*/
            You("are released from the %s!",
                u.utraptype == TT_WEB ? "web" : "trap");
#else
            You("%sから解放された！",
                u.utraptype == TT_WEB ? "くもの巣" : "罠");
#endif
            u.utrap = 0;
        }

        i = number_leashed();
        if (i > 0) {
#if 0 /*JP*/
            const char *s = (i > 1) ? "leashes" : "leash";

            pline_The("%s %s loose.", s, vtense(s, "snap"));
#else
            pline("紐はパチンとはずれた．");
#endif
            unleash_all();
        }

        if (touch_petrifies(youmonst.data) && !resists_ston(mtmp)) {
            /* put the attacker back where it started;
               the resulting statue will end up there */
            remove_monster(mtmp->mx, mtmp->my); /* u.ux,u.uy */
            place_monster(mtmp, omx, omy);
            minstapetrify(mtmp, TRUE);
            /* normally unstuck() would do this, but we're not
               fully swallowed yet so that won't work here */
            if (Punished)
                placebc();
            u.ustuck = 0;
            return (mtmp->mhp > 0) ? 0 : 2;
        }

        display_nhwindow(WIN_MESSAGE, FALSE);
        vision_recalc(2); /* hero can't see anything */
        u.uswallow = 1;
        /* for digestion, shorter time is more dangerous;
           for other swallowings, longer time means more
           chances for the swallower to attack */
        if (mattk->adtyp == AD_DGST) {
            tim_tmp = 25 - (int) mtmp->m_lev;
            if (tim_tmp > 0)
                tim_tmp = rnd(tim_tmp) / 2;
            else if (tim_tmp < 0)
                tim_tmp = -(rnd(-tim_tmp) / 2);
            /* having good armor & high constitution makes
               it take longer for you to be digested, but
               you'll end up trapped inside for longer too */
            tim_tmp += -u.uac + 10 + (ACURR(A_CON) / 3 - 1);
        } else {
            /* higher level attacker takes longer to eject hero */
            tim_tmp = rnd((int) mtmp->m_lev + 10 / 2);
        }
        /* u.uswldtim always set > 1 */
        u.uswldtim = (unsigned) ((tim_tmp < 2) ? 2 : tim_tmp);
        swallowed(1);
        for (otmp2 = invent; otmp2; otmp2 = otmp2->nobj)
            (void) snuff_lit(otmp2);
    }

    if (mtmp != u.ustuck)
        return 0;
    if (u.uswldtim > 0)
        u.uswldtim -= 1;

    switch (mattk->adtyp) {
    case AD_DGST:
        physical_damage = TRUE;
        if (Slow_digestion) {
            /* Messages are handled below */
            u.uswldtim = 0;
            tmp = 0;
        } else if (u.uswldtim == 0) {
/*JP
            pline("%s totally digests you!", Monnam(mtmp));
*/
            pline("%sはあなたを完全に消化した！", Monnam(mtmp));
            tmp = u.uhp;
            if (Half_physical_damage)
                tmp *= 2; /* sorry */
        } else {
#if 0 /*JP*/
            pline("%s%s digests you!", Monnam(mtmp),
                  (u.uswldtim == 2) ? " thoroughly"
                                    : (u.uswldtim == 1) ? " utterly" : "");
#else
            pline("%sはあなたを%s消化している！", Monnam(mtmp),
                  (u.uswldtim == 2) ? "徹底的に"
                                    : (u.uswldtim == 1) ? "完全に" : "");
#endif
            exercise(A_STR, FALSE);
        }
        break;
    case AD_PHYS:
        physical_damage = TRUE;
        if (mtmp->data == &mons[PM_FOG_CLOUD]) {
#if 0 /*JP*/
            You("are laden with moisture and %s",
                flaming(youmonst.data)
                    ? "are smoldering out!"
                    : Breathless ? "find it mildly uncomfortable."
                                 : amphibious(youmonst.data)
                                       ? "feel comforted."
                                       : "can barely breathe!");
#else
            You("湿気に苦しめられ，%s",
                flaming(youmonst.data)
                    ? "くすぶった！"
                    : Breathless ? "少々不快に感じた．"
                                 : amphibious(youmonst.data)
                                       ? "安らぎを感じた．"
                                       : "息をするのがやっとだ！");
#endif
            /* NB: Amphibious includes Breathless */
            if (Amphibious && !flaming(youmonst.data))
                tmp = 0;
        } else {
/*JP
            You("are pummeled with debris!");
*/
            You("瓦礫で痛めつけられた！");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_ACID:
        if (Acid_resistance) {
/*JP
            You("are covered with a seemingly harmless goo.");
*/
            You("無害っぽいねばつくものでおおわれた．");
            tmp = 0;
        } else {
            if (Hallucination)
/*JP
                pline("Ouch!  You've been slimed!");
*/
                pline("げげん！あなたはぬるぬるだ！");
            else
/*JP
                You("are covered in slime!  It burns!");
*/
                You("べとべとになった．そして酸に焼かれた！");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_BLND:
        if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind) {
                long was_blinded = Blinded;
                if (!Blinded)
/*JP
                    You_cant("see in here!");
*/
                    You("何も見えない！");
                make_blinded((long) tmp, FALSE);
                if (!was_blinded && !Blind)
                    Your1(vision_clears);
            } else
                /* keep him blind until disgorged */
                make_blinded(Blinded + 1, FALSE);
        }
        tmp = 0;
        break;
    case AD_ELEC:
        if (!mtmp->mcan && rn2(2)) {
/*JP
            pline_The("air around you crackles with electricity.");
*/
            pline("あなたの回りの空気は電気でピリピリしている．");
            if (Shock_resistance) {
                shieldeff(u.ux, u.uy);
/*JP
                You("seem unhurt.");
*/
                You("傷つかないようだ．");
                ugolemeffects(AD_ELEC, tmp);
                tmp = 0;
            }
        } else
            tmp = 0;
        break;
    case AD_COLD:
        if (!mtmp->mcan && rn2(2)) {
            if (Cold_resistance) {
                shieldeff(u.ux, u.uy);
/*JP
                You_feel("mildly chilly.");
*/
                pline("ひんやりした．");
                ugolemeffects(AD_COLD, tmp);
                tmp = 0;
            } else
/*JP
                You("are freezing to death!");
*/
                You("凍死しそうだ！");
        } else
            tmp = 0;
        break;
    case AD_FIRE:
        if (!mtmp->mcan && rn2(2)) {
            if (Fire_resistance) {
                shieldeff(u.ux, u.uy);
/*JP
                You_feel("mildly hot.");
*/
                pline("ポカポカした．");
                ugolemeffects(AD_FIRE, tmp);
                tmp = 0;
            } else
/*JP
                You("are burning to a crisp!");
*/
                You("燃えてカラカラになった！");
            burn_away_slime();
        } else
            tmp = 0;
        break;
    case AD_DISE:
        if (!diseasemu(mtmp->data))
            tmp = 0;
        break;
    case AD_DREN:
        /* AC magic cancellation doesn't help when engulfed */
        if (!mtmp->mcan && rn2(4)) /* 75% chance */
            drain_en(tmp);
        tmp = 0;
        break;
    default:
        physical_damage = TRUE;
        tmp = 0;
        break;
    }

    if (physical_damage)
        tmp = Maybe_Half_Phys(tmp);

    mdamageu(mtmp, tmp);
    if (tmp)
        stop_occupation();

    if (touch_petrifies(youmonst.data) && !resists_ston(mtmp)) {
#if 0 /*JP*/
        pline("%s very hurriedly %s you!", Monnam(mtmp),
              is_animal(mtmp->data) ? "regurgitates" : "expels");
#else
        pline("%sは慌ててあなたを%sした！", Monnam(mtmp),
              is_animal(mtmp->data)? "吐き戻" : "排出");
#endif
        expels(mtmp, mtmp->data, FALSE);
    } else if (!u.uswldtim || youmonst.data->msize >= MZ_HUGE) {
/*JP
        You("get %s!", is_animal(mtmp->data) ? "regurgitated" : "expelled");
*/
        You("%sされた！", is_animal(mtmp->data)? "吐き戻" : "排出");
        if (flags.verbose
            && (is_animal(mtmp->data)
                || (dmgtype(mtmp->data, AD_DGST) && Slow_digestion)))
/*JP
            pline("Obviously %s doesn't like your taste.", mon_nam(mtmp));
*/
            You("どうも%s好みの味じゃないようだ．", mon_nam(mtmp));
        expels(mtmp, mtmp->data, FALSE);
    }
    return 1;
}

/* monster explodes in your face */
STATIC_OVL int
explmu(mtmp, mattk, ufound)
struct monst *mtmp;
struct attack *mattk;
boolean ufound;
{
    boolean physical_damage = TRUE, kill_agr = TRUE;

    if (mtmp->mcan)
        return 0;

    if (!ufound)
#if 0 /*JP*/
        pline("%s explodes at a spot in %s!",
              canseemon(mtmp) ? Monnam(mtmp) : "It",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "empty water"
                                                      : "thin air");
#else
        pline("%sは何もない%sで爆発した！",
              canseemon(mtmp) ? Monnam(mtmp) : "何者か",
              levl[mtmp->mux][mtmp->muy].typ == WATER ? "水中"
                                                      : "空間");
#endif
    else {
        int tmp = d((int) mattk->damn, (int) mattk->damd);
        boolean not_affected = defends((int) mattk->adtyp, uwep);

        hitmsg(mtmp, mattk);

        switch (mattk->adtyp) {
        case AD_COLD:
            physical_damage = FALSE;
            not_affected |= Cold_resistance;
            goto common;
        case AD_FIRE:
            physical_damage = FALSE;
            not_affected |= Fire_resistance;
            goto common;
        case AD_ELEC:
            physical_damage = FALSE;
            not_affected |= Shock_resistance;
        common:

            if (!not_affected) {
                if (ACURR(A_DEX) > rnd(20)) {
/*JP
                    You("duck some of the blast.");
*/
                    You("衝撃をさけた．");
                    tmp = (tmp + 1) / 2;
                } else {
                    if (flags.verbose)
/*JP
                        You("get blasted!");
*/
                        You("衝撃をくらった！");
                }
                if (mattk->adtyp == AD_FIRE)
                    burn_away_slime();
                if (physical_damage)
                    tmp = Maybe_Half_Phys(tmp);
                mdamageu(mtmp, tmp);
            }
            break;

        case AD_BLND:
            not_affected = resists_blnd(&youmonst);
            if (!not_affected) {
                /* sometimes you're affected even if it's invisible */
                if (mon_visible(mtmp) || (rnd(tmp /= 2) > u.ulevel)) {
/*JP
                    You("are blinded by a blast of light!");
*/
                    You("まばゆい光に目がくらんだ！");
                    make_blinded((long) tmp, FALSE);
                    if (!Blind)
                        Your1(vision_clears);
                } else if (flags.verbose)
/*JP
                    You("get the impression it was not terribly bright.");
*/
                    You("それは恐ろしくまぶしいというほどじゃないと思った．");
            }
            break;

        case AD_HALU:
            not_affected |= Blind || (u.umonnum == PM_BLACK_LIGHT
                                      || u.umonnum == PM_VIOLET_FUNGUS
                                      || dmgtype(youmonst.data, AD_STUN));
            if (!not_affected) {
                boolean chg;
                if (!Hallucination)
/*JP
                    You("are caught in a blast of kaleidoscopic light!");
*/
                    You("万華鏡の光に捕らえられた！");
                /* avoid hallucinating the black light as it dies */
                mondead(mtmp);    /* remove it from map now */
                kill_agr = FALSE; /* already killed (maybe lifesaved) */
                chg =
                    make_hallucinated(HHallucination + (long) tmp, FALSE, 0L);
/*JP
                You("%s.", chg ? "are freaked out" : "seem unaffected");
*/
                You("%s．", chg ? "酔いしれた" : "影響を受けなかった");
            }
            break;

        default:
            break;
        }
        if (not_affected) {
/*JP
            You("seem unaffected by it.");
*/
            You("影響を受けないようだ．");
            ugolemeffects((int) mattk->adtyp, tmp);
        }
    }
    if (kill_agr)
        mondead(mtmp);
    wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
    return (mtmp->mhp > 0) ? 0 : 2;
}

/* monster gazes at you */
int
gazemu(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
    static const char *const reactions[] = {
#if 0 /*JP*/
        "confused",              /* [0] */
        "stunned",               /* [1] */
        "puzzled",   "dazzled",  /* [2,3] */
        "irritated", "inflamed", /* [4,5] */
        "tired",                 /* [6] */
        "dulled",                /* [7] */
#else
        "混乱した",                 /* [0] */
        "朦朧とした",               /* [1] */
        "困惑した",   "幻惑された", /* [2,3] */
        "イライラした", "興奮した", /* [4,5] */
        "疲れた",                   /* [6] */
        "ぼんやりした",             /* [7] */
#endif
    };
    int react = -1;
    boolean cancelled = (mtmp->mcan != 0), already = FALSE;

    /* assumes that hero has to see monster's gaze in order to be
       affected, rather than monster just having to look at hero;
       when hallucinating, hero's brain doesn't register what
       it's seeing correctly so the gaze is usually ineffective
       [this could be taken a lot farther and select a gaze effect
       appropriate to what's currently being displayed, giving
       ordinary monsters a gaze attack when hero thinks he or she
       is facing a gazing creature, but let's not go that far...] */
    if (Hallucination && rn2(4))
        cancelled = TRUE;

    switch (mattk->adtyp) {
    case AD_STON:
        if (cancelled || !mtmp->mcansee) {
            if (!canseemon(mtmp))
                break; /* silently */
#if 0 /*JP*/
            pline("%s %s.", Monnam(mtmp),
                  (mtmp->data == &mons[PM_MEDUSA] && mtmp->mcan)
                      ? "doesn't look all that ugly"
                      : "gazes ineffectually");
#else
            pline("%sは%s．", Monnam(mtmp),
                  (mtmp->data == &mons[PM_MEDUSA] && mtmp->mcan)
                      ? "それほど醜くないことに気がついた"
                      : "無意味ににらんだ");
#endif
            break;
        }
        if (Reflecting && couldsee(mtmp->mx, mtmp->my)
            && mtmp->data == &mons[PM_MEDUSA]) {
            /* hero has line of sight to Medusa and she's not blind */
            boolean useeit = canseemon(mtmp);

            if (useeit)
#if 0 /*JP:T*/
                (void) ureflects("%s gaze is reflected by your %s.",
                                 s_suffix(Monnam(mtmp)));
#else
                (void) ureflects("%sのにらみは%sで反射された．",
                                 Monnam(mtmp));
#endif
            if (mon_reflects(
                    mtmp, !useeit ? (char *) 0
/*JP
                                  : "The gaze is reflected away by %s %s!"))
*/
                                  : "にらみは%sの%sによって反射された！"))
                break;
            if (!m_canseeu(mtmp)) { /* probably you're invisible */
                if (useeit)
#if 0 /*JP*/
                    pline(
                      "%s doesn't seem to notice that %s gaze was reflected.",
                          Monnam(mtmp), mhis(mtmp));
#else
                    pline("にらみが反射していることに%sは気付いていないようだ．",
                          Monnam(mtmp));
#endif
                break;
            }
            if (useeit)
/*JP
                pline("%s is turned to stone!", Monnam(mtmp));
*/
                pline("%sは石になった！", Monnam(mtmp));
            stoned = TRUE;
            killed(mtmp);

            if (mtmp->mhp > 0)
                break;
            return 2;
        }
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && !Stone_resistance) {
/*JP
            You("meet %s gaze.", s_suffix(mon_nam(mtmp)));
*/
            You("%sににらまれた．", mon_nam(mtmp));
            stop_occupation();
            if (poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
                break;
/*JP
            You("turn to stone...");
*/
            You("石になった．．．");
            killer.format = KILLED_BY;
            Strcpy(killer.name, mtmp->data->mname);
            done(STONING);
        }
        break;
    case AD_CONF:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 0; /* "confused" */
                already = (mtmp->mconf != 0);
            } else {
                int conf = d(3, 4);

                mtmp->mspec_used = mtmp->mspec_used + (conf + rn2(6));
                if (!Confusion)
/*JP
                    pline("%s gaze confuses you!", s_suffix(Monnam(mtmp)));
*/
                    pline("%sのにらみであなたは混乱した！", Monnam(mtmp));
                else
/*JP
                    You("are getting more and more confused.");
*/
                    You("ますます混乱した．");
                make_confused(HConfusion + conf, FALSE);
                stop_occupation();
            }
        }
        break;
    case AD_STUN:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 1; /* "stunned" */
                already = (mtmp->mstun != 0);
            } else {
                int stun = d(2, 6);

                mtmp->mspec_used = mtmp->mspec_used + (stun + rn2(6));
/*JP
                pline("%s stares piercingly at you!", Monnam(mtmp));
*/
                pline("%sは冷たいまなざしをあなたに向けた！", Monnam(mtmp));
                make_stunned((HStun & TIMEOUT) + (long) stun, TRUE);
                stop_occupation();
            }
        }
        break;
    case AD_BLND:
        if (canseemon(mtmp) && !resists_blnd(&youmonst)
            && distu(mtmp->mx, mtmp->my) <= BOLT_LIM * BOLT_LIM) {
            if (cancelled) {
                react = rn1(2, 2); /* "puzzled" || "dazzled" */
                already = (mtmp->mcansee == 0);
                /* Archons gaze every round; we don't want cancelled ones
                   giving the "seems puzzled/dazzled" message that often */
                if (mtmp->mcan && mtmp->data == &mons[PM_ARCHON] && rn2(5))
                    react = -1;
            } else {
                int blnd = d((int) mattk->damn, (int) mattk->damd);

/*JP
                You("are blinded by %s radiance!", s_suffix(mon_nam(mtmp)));
*/
                You("%sの光で目が見えなくなった！", mon_nam(mtmp));
                make_blinded((long) blnd, FALSE);
                stop_occupation();
                /* not blind at this point implies you're wearing
                   the Eyes of the Overworld; make them block this
                   particular stun attack too */
                if (!Blind) {
                    Your1(vision_clears);
                } else {
                    long oldstun = (HStun & TIMEOUT), newstun = (long) rnd(3);

                    /* we don't want to increment stun duration every time
                       or sighted hero will become incapacitated */
                    make_stunned(max(oldstun, newstun), TRUE);
                }
            }
        }
        break;
    case AD_FIRE:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = rn1(2, 4); /* "irritated" || "inflamed" */
            } else {
                int dmg = d(2, 6), lev = (int) mtmp->m_lev;

/*JP
                pline("%s attacks you with a fiery gaze!", Monnam(mtmp));
*/
                pline("%sは炎のにらみ攻撃をしてきた！", Monnam(mtmp));
                stop_occupation();
                if (Fire_resistance) {
/*JP
                    pline_The("fire doesn't feel hot!");
*/
                    pline("火はぜんぜん熱くない！");
                    dmg = 0;
                }
                burn_away_slime();
                if (lev > rn2(20))
                    destroy_item(SCROLL_CLASS, AD_FIRE);
                if (lev > rn2(20))
                    destroy_item(POTION_CLASS, AD_FIRE);
                if (lev > rn2(25))
                    destroy_item(SPBOOK_CLASS, AD_FIRE);
                if (dmg)
                    mdamageu(mtmp, dmg);
            }
        }
        break;
#ifdef PM_BEHOLDER /* work in progress */
    case AD_SLEE:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && multi >= 0 && !rn2(5) && !Sleep_resistance) {
            if (cancelled) {
                react = 6;                      /* "tired" */
                already = (mtmp->mfrozen != 0); /* can't happen... */
            } else {
                fall_asleep(-rnd(10), TRUE);
#if 0 /*JP:T*/
                pline("%s gaze makes you very sleepy...",
                      s_suffix(Monnam(mtmp)));
#else
                pline("%sのにらみであなたは眠くなった．．．",
                      Monnam(mtmp));
#endif
            }
        }
        break;
    case AD_SLOW:
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && mtmp->mcansee
            && (HFast & (INTRINSIC | TIMEOUT)) && !defends(AD_SLOW, uwep)
            && !rn2(4)) {
            if (cancelled) {
                react = 7; /* "dulled" */
                already = (mtmp->mspeed == MSLOW);
            } else {
                u_slow_down();
                stop_occupation();
            }
        }
        break;
#endif /* BEHOLDER */
    default:
        impossible("Gaze attack %d?", mattk->adtyp);
        break;
    }
    if (react >= 0) {
        if (Hallucination && rn2(3))
            react = rn2(SIZE(reactions));
        /* cancelled/hallucinatory feedback; monster might look "confused",
           "stunned",&c but we don't actually set corresponding attribute */
#if 0 /*JP*/
        pline("%s looks %s%s.", Monnam(mtmp),
              !rn2(3) ? "" : already ? "quite "
                                     : (!rn2(2) ? "a bit " : "somewhat "),
              reactions[react]);
#else
        pline("%sは%s%sようだ．", Monnam(mtmp),
              !rn2(3) ? "" : already ? "かなり"
                                     : (!rn2(2) ? "少し" : "いくらか"),
              reactions[react]);
#endif
    }
    return 0;
}

/* mtmp hits you for n points damage */
void
mdamageu(mtmp, n)
struct monst *mtmp;
int n;
{
    context.botl = 1;
    if (Upolyd) {
        u.mh -= n;
        if (u.mh < 1)
            rehumanize();
    } else {
        u.uhp -= n;
        if (u.uhp < 1)
            done_in_by(mtmp, DIED);
    }
}

/* returns 0 if seduction impossible,
 *         1 if fine,
 *         2 if wrong gender for nymph
 */
int
could_seduce(magr, mdef, mattk)
struct monst *magr, *mdef;
struct attack *mattk;
{
    struct permonst *pagr;
    boolean agrinvis, defperc;
    xchar genagr, gendef;

    if (is_animal(magr->data))
        return 0;
    if (magr == &youmonst) {
        pagr = youmonst.data;
        agrinvis = (Invis != 0);
        genagr = poly_gender();
    } else {
        pagr = magr->data;
        agrinvis = magr->minvis;
        genagr = gender(magr);
    }
    if (mdef == &youmonst) {
        defperc = (See_invisible != 0);
        gendef = poly_gender();
    } else {
        defperc = perceives(mdef->data);
        gendef = gender(mdef);
    }

    if (agrinvis && !defperc
        && (!SYSOPT_SEDUCE || (mattk && mattk->adtyp != AD_SSEX)))
        return 0;

    if (pagr->mlet != S_NYMPH
        && ((pagr != &mons[PM_INCUBUS] && pagr != &mons[PM_SUCCUBUS])
            || (SYSOPT_SEDUCE && mattk && mattk->adtyp != AD_SSEX)))
        return 0;

    if (genagr == 1 - gendef)
        return 1;
    else
        return (pagr->mlet == S_NYMPH) ? 2 : 0;
}

/* Returns 1 if monster teleported */
int
doseduce(mon)
struct monst *mon;
{
    struct obj *ring, *nring;
    boolean fem = (mon->data == &mons[PM_SUCCUBUS]); /* otherwise incubus */
    int attr_tot, tried_gloves = 0;
    char qbuf[QBUFSZ];

    if (mon->mcan || mon->mspec_used) {
#if 0 /*JP*/
        pline("%s acts as though %s has got a %sheadache.", Monnam(mon),
              mhe(mon), mon->mcan ? "severe " : "");
#else
        pline("%sは%s頭が痛いふりをした．", Monnam(mon),
              mon->mcan ? "ひどく" : "");
#endif
        return 0;
    }

    if (unconscious()) {
/*JP
        pline("%s seems dismayed at your lack of response.", Monnam(mon));
*/
        pline("%sは返事がないので気が萎えたようだ．", Monnam(mon));
        return 0;
    }

    if (Blind)
/*JP
        pline("It caresses you...");
*/
        pline("何者かがあなたを抱きしめている．．．");
    else
/*JP
        You_feel("very attracted to %s.", mon_nam(mon));
*/
        You("%sに引きつけられてるような気がした．", mon_nam(mon));
    /* if in the process of putting armor on or taking armor off,
       interrupt that activity now */
    (void) stop_donning((struct obj *) 0);
    /* don't try to take off gloves if cursed weapon blocks them */
    if (welded(uwep))
        tried_gloves = 1;

    for (ring = invent; ring; ring = nring) {
        nring = ring->nobj;
        if (ring->otyp != RIN_ADORNMENT)
            continue;
        if (fem) {
            if (ring->owornmask && uarmg) {
                /* don't take off worn ring if gloves are in the way */
                if (!tried_gloves++)
/*JP
                    mayberem(uarmg, "gloves");
*/
                    mayberem(uarmg, "小手");
                if (uarmg)
                    continue; /* next ring might not be worn */
            }
            if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
                (void) safe_qbuf(qbuf, "\"That ",
                                 " looks pretty.  May I have it?\"", ring,
                                 xname, simpleonames, "ring");
#else
                (void) safe_qbuf(qbuf, "「なんて素敵な",
                                 "でしょう．わたしにくれません？」", ring,
                                 xname, simpleonames, "指輪");
#endif
                makeknown(RIN_ADORNMENT);
                if (yn(qbuf) == 'n')
                    continue;
            } else
#if 0 /*JP*/
                pline("%s decides she'd like %s, and takes it.",
                      Blind ? "She" : Monnam(mon), yname(ring));
#else
                pline("%sは%sがとても気にいって，それを取りあげた．",
                      Blind ? "彼女" : Monnam(mon), xname(ring));
#endif
            makeknown(RIN_ADORNMENT);
            if (ring == uleft || ring == uright)
                Ring_gone(ring);
            if (ring == uwep)
                setuwep((struct obj *) 0);
            if (ring == uswapwep)
                setuswapwep((struct obj *) 0);
            if (ring == uquiver)
                setuqwep((struct obj *) 0);
            freeinv(ring);
            (void) mpickobj(mon, ring);
        } else {
            if (uleft && uright && uleft->otyp == RIN_ADORNMENT
                && uright->otyp == RIN_ADORNMENT)
                break;
            if (ring == uleft || ring == uright)
                continue;
            if (uarmg) {
                /* don't put on ring if gloves are in the way */
                if (!tried_gloves++)
/*JP
                    mayberem(uarmg, "gloves");
*/
                    mayberem(uarmg, "小手");
                if (uarmg)
                    break; /* no point trying further rings */
            }
            if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
                (void) safe_qbuf(qbuf, "\"That ",
                                " looks pretty.  Would you wear it for me?\"",
                                 ring, xname, simpleonames, "ring");
#else
                (void) safe_qbuf(qbuf, "「おやなんてすばらしい",
                                "だ．私のために指にはめてくれないかい？」",
                                 ring, xname, simpleonames, "指輪");
#endif
                makeknown(RIN_ADORNMENT);
                if (yn(qbuf) == 'n')
                    continue;
            } else {
#if 0 /*JP*/
                pline("%s decides you'd look prettier wearing %s,",
                      Blind ? "He" : Monnam(mon), yname(ring));
#else
                pline("%sは%sをつけたあなたがより魅力的だと考え，",
                      Blind ? "彼" : Monnam(mon), xname(ring));
#endif
/*JP
                pline("and puts it on your finger.");
*/
                pline("あなたの指にそれをはめた．");
            }
            makeknown(RIN_ADORNMENT);
            if (!uright) {
#if 0 /*JP*/
                pline("%s puts %s on your right %s.",
                      Blind ? "He" : Monnam(mon), the(xname(ring)),
                      body_part(HAND));
#else
                pline("%sは%sをあなたの右%sにはめた．",
                      Blind ? "彼" : Monnam(mon), the(xname(ring)),
                      body_part(HAND));
#endif
                setworn(ring, RIGHT_RING);
            } else if (!uleft) {
#if 0 /*JP*/
                pline("%s puts %s on your left %s.",
                      Blind ? "He" : Monnam(mon), the(xname(ring)),
                      body_part(HAND));
#else
                pline("%sは%sをあなたの左%sにはめた．",
                      Blind ? "彼" : Monnam(mon), the(xname(ring)),
                      body_part(HAND));
#endif
                setworn(ring, LEFT_RING);
            } else if (uright && uright->otyp != RIN_ADORNMENT) {
#if 0 /*JP*/
                pline("%s replaces %s with %s.", Blind ? "He" : Monnam(mon),
                      yname(uright), yname(ring));
#else
                pline("%sは%sを%sにとりかえた．", Blind ? "彼" : Monnam(mon),
                      yname(uright), xname(ring));
#endif
                Ring_gone(uright);
                setworn(ring, RIGHT_RING);
            } else if (uleft && uleft->otyp != RIN_ADORNMENT) {
#if 0 /*JP*/
                pline("%s replaces %s with %s.", Blind ? "He" : Monnam(mon),
                      yname(uleft), yname(ring));
#else
                pline("%sは%sを%sにとりかえた．", Blind ? "彼" : Monnam(mon),
                      yname(uleft), xname(ring));
#endif
                Ring_gone(uleft);
                setworn(ring, LEFT_RING);
            } else
                impossible("ring replacement");
            Ring_on(ring);
            prinv((char *) 0, ring, 0L);
        }
    }

    if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh && !uarmu)
#if 0 /*JP*/
        pline("%s murmurs sweet nothings into your ear.",
              Blind ? (fem ? "She" : "He") : Monnam(mon));
#else
        pline("%sはあなたの耳もとで甘いささやきをつぶやいた．",
              Blind ? (fem ? "彼女" : "彼") : Monnam(mon));
#endif
    else
#if 0 /*JP*/
        pline("%s murmurs in your ear, while helping you undress.",
              Blind ? (fem ? "She" : "He") : Monnam(mon));
#else
        pline("%sは耳もとであなたの服を脱がせながらささやいた．",
              Blind ? (fem ? "彼女" : "彼") : Monnam(mon));
#endif
    mayberem(uarmc, cloak_simple_name(uarmc));
    if (!uarmc)
/*JP
        mayberem(uarm, "suit");
*/
        mayberem(uarm, "スーツ");
/*JP
    mayberem(uarmf, "boots");
*/
    mayberem(uarmf, "ブーツ");
    if (!tried_gloves)
/*JP
        mayberem(uarmg, "gloves");
*/
        mayberem(uarmg, "小手");
/*JP
    mayberem(uarms, "shield");
*/
    mayberem(uarms, "盾");
    mayberem(uarmh, helm_simple_name(uarmh));
    if (!uarmc && !uarm)
/*JP
        mayberem(uarmu, "shirt");
*/
        mayberem(uarmu, "シャツ");

    if (uarm || uarmc) {
#if 0 /*JP*/
        verbalize("You're such a %s; I wish...",
                  flags.female ? "sweet lady" : "nice guy");
#else
        verbalize("%sだ．．．といいのに．",
                  flags.female ? "チャーミング" : "すてき");
#endif
        if (!tele_restrict(mon))
            (void) rloc(mon, TRUE);
        return 1;
    }
    if (u.ualign.type == A_CHAOTIC)
        adjalign(1);

    /* by this point you have discovered mon's identity, blind or not... */
/*JP
    pline("Time stands still while you and %s lie in each other's arms...",
*/
    pline("あなたと%sが抱き合うと，時が止まったように感じた．．．",
          noit_mon_nam(mon));
    /* 3.6.1: a combined total for charisma plus intelligence of 35-1
       used to guarantee successful outcome; now total maxes out at 32
       as far as deciding what will happen; chance for bad outcome when
       Cha+Int is 32 or more is 2/35, a bit over 5.7% */
    attr_tot = ACURR(A_CHA) + ACURR(A_INT);
    if (rn2(35) > min(attr_tot, 32)) {
        /* Don't bother with mspec_used here... it didn't get tired! */
/*JP
        pline("%s seems to have enjoyed it more than you...",
*/
        pline("%sはあなたより楽しんだようだ．．．",
              noit_Monnam(mon));
        switch (rn2(5)) {
        case 0:
/*JP
            You_feel("drained of energy.");
*/
            You("体力が消耗したような気がした．");
            u.uen = 0;
            u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
            exercise(A_CON, FALSE);
            if (u.uenmax < 0)
                u.uenmax = 0;
            break;
        case 1:
/*JP
            You("are down in the dumps.");
*/
            You("意気消沈した．");
            (void) adjattrib(A_CON, -1, TRUE);
            exercise(A_CON, FALSE);
            context.botl = 1;
            break;
        case 2:
/*JP
            Your("senses are dulled.");
*/
            Your("五感は鈍った．");
            (void) adjattrib(A_WIS, -1, TRUE);
            exercise(A_WIS, FALSE);
            context.botl = 1;
            break;
        case 3:
            if (!resists_drli(&youmonst)) {
/*JP
                You_feel("out of shape.");
*/
                You("くたびれた．");
/*JP
                losexp("overexertion");
*/
                losexp("過労で");
            } else {
/*JP
                You("have a curious feeling...");
*/
                You("変な感じがした．．．");
            }
            exercise(A_CON, FALSE);
            exercise(A_DEX, FALSE);
            exercise(A_WIS, FALSE);
            break;
        case 4: {
            int tmp;

/*JP
            You_feel("exhausted.");
*/
            You("疲れを感じた．");
            exercise(A_STR, FALSE);
            tmp = rn1(10, 6);
/*JP
            losehp(Maybe_Half_Phys(tmp), "exhaustion", KILLED_BY);
*/
            losehp(Maybe_Half_Phys(tmp), "精力の使いすぎで", KILLED_BY);
            break;
        } /* case 4 */
        } /* switch */
    } else {
        mon->mspec_used = rnd(100); /* monster is worn out */
/*JP
        You("seem to have enjoyed it more than %s...", noit_mon_nam(mon));
*/
        You("%sよりも楽しんだようだ．．．", noit_mon_nam(mon));
        switch (rn2(5)) {
        case 0:
/*JP
            You_feel("raised to your full potential.");
*/
            You("潜在能力が高まったように感じた．");
            exercise(A_CON, TRUE);
            u.uen = (u.uenmax += rnd(5));
            break;
        case 1:
/*JP
            You_feel("good enough to do it again.");
*/
            You("もう一度できると思った．");
            (void) adjattrib(A_CON, 1, TRUE);
            exercise(A_CON, TRUE);
            context.botl = 1;
            break;
        case 2:
/*JP
            You("will always remember %s...", noit_mon_nam(mon));
*/
            You("いつまでも%sを覚えてるだろう．．．", mon_nam(mon));
            (void) adjattrib(A_WIS, 1, TRUE);
            exercise(A_WIS, TRUE);
            context.botl = 1;
            break;
        case 3:
/*JP
            pline("That was a very educational experience.");
*/
            pline("とても貴重な経験だった．");
            pluslvl(FALSE);
            exercise(A_WIS, TRUE);
            break;
        case 4:
/*JP
            You_feel("restored to health!");
*/
            You("とても健康になった！");
            u.uhp = u.uhpmax;
            if (Upolyd)
                u.mh = u.mhmax;
            exercise(A_STR, TRUE);
            context.botl = 1;
            break;
        }
    }

    if (mon->mtame) { /* don't charge */
        ;
    } else if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
        pline("%s demands that you pay %s, but you refuse...",
              noit_Monnam(mon), Blind ? (fem ? "her" : "him") : mhim(mon));
#else
        pline("%sはあなたに金を払うよう要求したが，あなたは拒んだ．．．",
              noit_Monnam(mon));
#endif
    } else if (u.umonnum == PM_LEPRECHAUN) {
/*JP
        pline("%s tries to take your money, but fails...", noit_Monnam(mon));
*/
        pline("%sは金を取ろうとしたが，失敗した．．．", noit_Monnam(mon));
    } else {
        long cost;
        long umoney = money_cnt(invent);

        if (umoney > (long) LARGEST_INT - 10L)
            cost = (long) rnd(LARGEST_INT) + 500L;
        else
            cost = (long) rnd((int) umoney + 10) + 500L;
        if (mon->mpeaceful) {
            cost /= 5L;
            if (!cost)
                cost = 1L;
        }
        if (cost > umoney)
            cost = umoney;
        if (!cost) {
/*JP
            verbalize("It's on the house!");
*/
            verbalize("これはおごり%s！", fem ? "よ" : "さ");
        } else {
#if 0 /*JP*/
            pline("%s takes %ld %s for services rendered!", noit_Monnam(mon),
                  cost, currency(cost));
#else
            pline("%sはサービス料として%ld%s奪い取った！", noit_Monnam(mon),
                  cost, currency(cost));
#endif
            money2mon(mon, cost);
            context.botl = 1;
        }
    }
    if (!rn2(25))
        mon->mcan = 1; /* monster is worn out */
    if (!tele_restrict(mon))
        (void) rloc(mon, TRUE);
    return 1;
}

STATIC_OVL void
mayberem(obj, str)
struct obj *obj;
const char *str;
{
    char qbuf[QBUFSZ];

    if (!obj || !obj->owornmask)
        return;

    if (rn2(20) < ACURR(A_CHA)) {
#if 0 /*JP*/
        Sprintf(qbuf, "\"Shall I remove your %s, %s?\"", str,
                (!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
#else
        Sprintf(qbuf,"「%sを取っていい，%s？」", str,
                (!rn2(2) ? "ねぇ" : flags.female ? "ハニー" : "ダーリン" ));
#endif
        if (yn(qbuf) == 'n')
            return;
    } else {
        char hairbuf[BUFSZ];

#if 0 /*JP*/
        Sprintf(hairbuf, "let me run my fingers through your %s",
                body_part(HAIR));
#else
        Sprintf(hairbuf,
                flags.female ? "なんて綺麗な%sなんだ" : "兜を取ったらなかなかイカスじゃない",
                body_part(HAIR));
#endif
#if 0 /*JP*/
        verbalize("Take off your %s; %s.", str,
                  (obj == uarm)
                     ? "let's get a little closer"
                     : (obj == uarmc || obj == uarms)
                        ? "it's in the way"
                        : (obj == uarmf)
                           ? "let me rub your feet"
                           : (obj == uarmg)
                              ? "they're too clumsy"
                              : (obj == uarmu)
                                 ? "let me massage you"
                                 /* obj == uarmh */
                                 : hairbuf);
#else
        verbalize("%sを脱いで．．．%s．", str,
                  (obj == uarm)
                  ? "もうちょっと寄りそって"
                  : (obj == uarmc || obj == uarms)
                  ? "そうそう"
                  : (obj == uarmf)
                  ? (flags.female ? "綺麗な足だね" : "うふっ，たくましい足ね")
                  : (obj == uarmg)
                  ? (flags.female ? "なんて素敵な手だ" : "たくましい腕ね")
                  : (obj == uarmu)
                  ? (flags.female ? "きれいな体だ" : "たくましいのね")
                  /* obj == uarmh */
                  : hairbuf);
#endif
    }
    remove_worn_item(obj, TRUE);
}

/* FIXME:
 *  sequencing issue:  a monster's attack might cause poly'd hero
 *  to revert to normal form.  The messages for passive counterattack
 *  would look better if they came before reverting form, but we need
 *  to know whether hero reverted in order to decide whether passive
 *  damage applies.
 */
STATIC_OVL int
passiveum(olduasmon, mtmp, mattk)
struct permonst *olduasmon;
struct monst *mtmp;
struct attack *mattk;
{
    int i, tmp;
    struct attack *oldu_mattk = 0;

    /*
     * mattk      == mtmp's attack that hit you;
     * oldu_mattk == your passive counterattack (even if mtmp's attack
     *               has already caused you to revert to normal form).
     */
    for (i = 0; !oldu_mattk; i++) {
        if (i >= NATTK)
            return 1;
        if (olduasmon->mattk[i].aatyp == AT_NONE
            || olduasmon->mattk[i].aatyp == AT_BOOM)
            oldu_mattk = &olduasmon->mattk[i];
    }
    if (oldu_mattk->damn)
        tmp = d((int) oldu_mattk->damn, (int) oldu_mattk->damd);
    else if (oldu_mattk->damd)
        tmp = d((int) olduasmon->mlevel + 1, (int) oldu_mattk->damd);
    else
        tmp = 0;

    /* These affect the enemy even if you were "killed" (rehumanized) */
    switch (oldu_mattk->adtyp) {
    case AD_ACID:
        if (!rn2(2)) {
#if 0 /*JP*/
            pline("%s is splashed by %s%s!", Monnam(mtmp),
                  /* temporary? hack for sequencing issue:  "your acid"
                     looks strange coming immediately after player has
                     been told that hero has reverted to normal form */
                  !Upolyd ? "" : "your ", hliquid("acid"));
#else
            pline("%sは%s%sを食らった！", Monnam(mtmp),
                  /* temporary? hack for sequencing issue:  "your acid"
                     looks strange coming immediately after player has
                     been told that hero has reverted to normal form */
                  !Upolyd ? "" : "あなたの", hliquid("酸"));
#endif
            if (resists_acid(mtmp)) {
/*JP
                pline("%s is not affected.", Monnam(mtmp));
*/
                pline("%sは影響を受けない．", Monnam(mtmp));
                tmp = 0;
            }
        } else
            tmp = 0;
        if (!rn2(30))
            erode_armor(mtmp, ERODE_CORRODE);
        if (!rn2(6))
            acid_damage(MON_WEP(mtmp));
        goto assess_dmg;
    case AD_STON: /* cockatrice */
    {
        long protector = attk_protection((int) mattk->aatyp),
             wornitems = mtmp->misc_worn_check;

        /* wielded weapon gives same protection as gloves here */
        if (MON_WEP(mtmp) != 0)
            wornitems |= W_ARMG;

        if (!resists_ston(mtmp)
            && (protector == 0L
                || (protector != ~0L
                    && (wornitems & protector) != protector))) {
            if (poly_when_stoned(mtmp->data)) {
                mon_to_stone(mtmp);
                return 1;
            }
/*JP
            pline("%s turns to stone!", Monnam(mtmp));
*/
            pline("%sは石になった！", Monnam(mtmp));
            stoned = 1;
            xkilled(mtmp, XKILL_NOMSG);
            if (mtmp->mhp > 0)
                return 1;
            return 2;
        }
        return 1;
    }
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mon_currwep) {
            /* by_you==True: passive counterattack to hero's action
               is hero's fault */
            (void) drain_item(mon_currwep, TRUE);
            /* No message */
        }
        return 1;
    default:
        break;
    }
    if (!Upolyd)
        return 1;

    /* These affect the enemy only if you are still a monster */
    if (rn2(3))
        switch (oldu_mattk->adtyp) {
        case AD_PHYS:
            if (oldu_mattk->aatyp == AT_BOOM) {
/*JP
                You("explode!");
*/
                pline("あなたは爆発した！");
                /* KMH, balance patch -- this is okay with unchanging */
                rehumanize();
                goto assess_dmg;
            }
            break;
        case AD_PLYS: /* Floating eye */
            if (tmp > 127)
                tmp = 127;
            if (u.umonnum == PM_FLOATING_EYE) {
                if (!rn2(4))
                    tmp = 127;
                if (mtmp->mcansee && haseyes(mtmp->data) && rn2(3)
                    && (perceives(mtmp->data) || !Invis)) {
                    if (Blind)
/*JP
                        pline("As a blind %s, you cannot defend yourself.",
*/
                        pline("%sは目が見えないので，あなたはにらみで防御できない．",
                              youmonst.data->mname);
                    else {
                        if (mon_reflects(mtmp,
/*JP
                                         "Your gaze is reflected by %s %s."))
*/
                                         "あなたのにらみは%sの%sで反射した．"))
                            return 1;
/*JP
                        pline("%s is frozen by your gaze!", Monnam(mtmp));
*/
                        pline("%sはあなたのにらみで動けなくなった！", Monnam(mtmp));
                        paralyze_monst(mtmp, tmp);
                        return 3;
                    }
                }
            } else { /* gelatinous cube */
/*JP
                pline("%s is frozen by you.", Monnam(mtmp));
*/
                pline("%sは動けない．", Monnam(mtmp));
                paralyze_monst(mtmp, tmp);
                return 3;
            }
            return 1;
        case AD_COLD: /* Brown mold or blue jelly */
            if (resists_cold(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
/*JP
                pline("%s is mildly chilly.", Monnam(mtmp));
*/
                pline("%sは冷えた．", Monnam(mtmp));
                golemeffects(mtmp, AD_COLD, tmp);
                tmp = 0;
                break;
            }
/*JP
            pline("%s is suddenly very cold!", Monnam(mtmp));
*/
            pline("%sは突然凍りづけになった！", Monnam(mtmp));
            u.mh += tmp / 2;
            if (u.mhmax < u.mh)
                u.mhmax = u.mh;
            if (u.mhmax > ((youmonst.data->mlevel + 1) * 8))
                (void) split_mon(&youmonst, mtmp);
            break;
        case AD_STUN: /* Yellow mold */
            if (!mtmp->mstun) {
                mtmp->mstun = 1;
#if 0 /*JP*/
                pline("%s %s.", Monnam(mtmp),
                      makeplural(stagger(mtmp->data, "stagger")));
#else
                pline("%sは%s．", Monnam(mtmp),
                      jpast(stagger(mtmp->data, "よろめく")));
#endif
            }
            tmp = 0;
            break;
        case AD_FIRE: /* Red mold */
            if (resists_fire(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
#if 0 /*JP*/
                pline("%s is mildly warm.", Monnam(mtmp));
#else
                pline("%sは暖かくなった．", Monnam(mtmp));
#endif
                golemeffects(mtmp, AD_FIRE, tmp);
                tmp = 0;
                break;
            }
#if 0 /*JP*/
            pline("%s is suddenly very hot!", Monnam(mtmp));
#else
            pline("%sは突然熱くなった！", Monnam(mtmp));
#endif
            break;
        case AD_ELEC:
            if (resists_elec(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
#if 0 /*JP*/
                pline("%s is slightly tingled.", Monnam(mtmp));
#else
                pline("%sはちょっとピリピリした．", Monnam(mtmp));
#endif
                golemeffects(mtmp, AD_ELEC, tmp);
                tmp = 0;
                break;
            }
#if 0 /*JP*/
            pline("%s is jolted with your electricity!", Monnam(mtmp));
#else
            pline("%sは電気ショックをうけた！", Monnam(mtmp));
#endif
            break;
        default:
            tmp = 0;
            break;
        }
    else
        tmp = 0;

assess_dmg:
    if ((mtmp->mhp -= tmp) <= 0) {
/*JP
        pline("%s dies!", Monnam(mtmp));
*/
        pline("%sは死んだ！", Monnam(mtmp));
        xkilled(mtmp, XKILL_NOMSG);
        if (mtmp->mhp > 0)
            return 1;
        return 2;
    }
    return 1;
}

struct monst *
cloneu()
{
    struct monst *mon;
    int mndx = monsndx(youmonst.data);

    if (u.mh <= 1)
        return (struct monst *) 0;
    if (mvitals[mndx].mvflags & G_EXTINCT)
        return (struct monst *) 0;
    mon = makemon(youmonst.data, u.ux, u.uy, NO_MINVENT | MM_EDOG);
    if (!mon)
        return NULL;
    mon->mcloned = 1;
    mon = christen_monst(mon, plname);
    initedog(mon);
    mon->m_lev = youmonst.data->mlevel;
    mon->mhpmax = u.mhmax;
    mon->mhp = u.mh / 2;
    u.mh -= mon->mhp;
    context.botl = 1;
    return mon;
}

/*mhitu.c*/
