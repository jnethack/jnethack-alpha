/* NetHack 5.0	lock.c	$NHDT-Date: 1741793439 2025/03/12 07:30:39 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.145 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-                */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* occupation callbacks */
staticfn int picklock(void);
staticfn int forcelock(void);

staticfn const char *lock_action(void);
staticfn boolean obstructed(coordxy, coordxy, boolean);
staticfn void chest_shatter_msg(struct obj *);

boolean
picking_lock(coordxy *x, coordxy *y)
{
    if (go.occupation == picklock) {
        *x = u.ux + u.dx;
        *y = u.uy + u.dy;
        return TRUE;
    } else {
        *x = *y = 0;
        return FALSE;
    }
}

boolean
picking_at(coordxy x, coordxy y)
{
    return (boolean) (go.occupation == picklock
                      && gx.xlock.door == &levl[x][y]);
}

/* produce an occupation string appropriate for the current activity */
staticfn const char *
lock_action(void)
{
    /* "unlocking"+2 == "locking" */
    static const char *const actions[] = {
#if 0 /*JP*/
        "unlocking the door",   /* [0] */
        "unlocking the chest",  /* [1] */
        "unlocking the box",    /* [2] */
        "picking the lock"      /* [3] */
#else 
        "扉の鍵をはずす", 
        "宝箱の鍵をはずす",
        "箱の鍵をはずす",
        "鍵をはずす"
#endif
    };

    /* if the target is currently unlocked, we're trying to lock it now */
    if (gx.xlock.door && !(gx.xlock.door->doormask & D_LOCKED))
#if 0 /*JP*/
        return actions[0] + 2; /* "locking the door" */
#else /* 英語は un を取れば逆の意味になるが，日本語はそうはいかないのでリテラルを書く */
        return "扉に鍵をかける";
#endif
    else if (gx.xlock.box && !gx.xlock.box->olocked)
#if 0 /*JP*/
        return gx.xlock.box->otyp == CHEST ? actions[1] + 2 : actions[2] + 2;
#else
        return gx.xlock.box->otyp == CHEST ? "宝箱に鍵をかける" : "箱に鍵をかける";
#endif
    /* otherwise we're trying to unlock it */
    else if (gx.xlock.picktyp == LOCK_PICK)
        return actions[3]; /* "picking the lock" */
    else if (gx.xlock.picktyp == CREDIT_CARD)
        return actions[3]; /* same as lock_pick */
    else if (gx.xlock.door)
        return actions[0]; /* "unlocking the door" */
    else if (gx.xlock.box)
        return gx.xlock.box->otyp == CHEST ? actions[1] : actions[2];
    else
        return actions[3];
}

/* try to open/close a lock */
staticfn int
picklock(void)
{
    if (gx.xlock.box) {
        if (gx.xlock.box->where != OBJ_FLOOR
            || gx.xlock.box->ox != u.ux || gx.xlock.box->oy != u.uy) {
            return ((gx.xlock.usedtime = 0)); /* you or it moved */
        }
    } else { /* door */
        if (gx.xlock.door != &(levl[u.ux + u.dx][u.uy + u.dy])) {
            return ((gx.xlock.usedtime = 0)); /* you moved */
        }
        switch (gx.xlock.door->doormask) {
        case D_NODOOR:
/*JP
            pline("This doorway has no door.");
*/
            pline("出入口には扉がない．");
            return ((gx.xlock.usedtime = 0));
        case D_ISOPEN:
/*JP
            You("cannot lock an open door.");
*/
            pline("開いてる扉に鍵をかけられない．");
            return ((gx.xlock.usedtime = 0));
        case D_BROKEN:
/*JP
            pline("This door is broken.");
*/
            pline("扉は壊れている．");
            return ((gx.xlock.usedtime = 0));
        }
    }

    if (gx.xlock.usedtime++ >= 50 || nohands(gy.youmonst.data)) {
/*JP
        You("give up your attempt at %s.", lock_action());
*/
        pline("%sのをあきらめた．", lock_action());
        exercise(A_DEX, TRUE); /* even if you don't succeed */
        return ((gx.xlock.usedtime = 0));
    }

    if (rn2(100) >= gx.xlock.chance)
        return 1; /* still busy */

    /* using the Master Key of Thievery finds traps if its bless/curse
       state is adequate (non-cursed for rogues, blessed for others;
       checked when setting up 'xlock') */
    if ((!gx.xlock.door ? (int) gx.xlock.box->otrapped
                       : (gx.xlock.door->doormask & D_TRAPPED) != 0)
        && gx.xlock.magic_key) {
        gx.xlock.chance += 20; /* less effort needed next time */
        if (!gx.xlock.door) {
            if (!gx.xlock.box->tknown)
/*JP
                You("find a trap!");
*/
                You("罠を見つけた！");
            gx.xlock.box->tknown = 1;
        }
/*JP
        if (y_n("Do you want to try to disarm it?") == 'y') {
*/
        if (y_n("罠を外しますか？") == 'y') {
            const char *what;
            boolean alreadyunlocked;

            /* disarming while using magic key always succeeds */
            if (gx.xlock.door) {
                gx.xlock.door->doormask &= ~D_TRAPPED;
/*JP
                what = "door";
*/
                what = "扉";
                alreadyunlocked = !(gx.xlock.door->doormask & D_LOCKED);
            } else {
                gx.xlock.box->otrapped = 0;
                gx.xlock.box->tknown = 0;
/*JP
                what = (gx.xlock.box->otyp == CHEST) ? "chest" : "box";
*/
                what = (gx.xlock.box->otyp == CHEST) ? "宝箱" : "箱";
                alreadyunlocked = !gx.xlock.box->olocked;
            }
#if 0 /*JP:T*/
            You("succeed in disarming the trap.  The %s is still %slocked.",
                what, alreadyunlocked ? "un" : "");
#else
            You("罠を外した．%sは鍵が%sままだ．",
                what, alreadyunlocked ? "開いた" : "かかった");
#endif
            exercise(A_WIS, TRUE);
        } else {
/*JP
            You("stop %s.", lock_action());
*/
            You("%sのをやめた．", lock_action());
            exercise(A_WIS, FALSE);
        }
        return ((gx.xlock.usedtime = 0));
    }

/*JP
    You("succeed in %s.", lock_action());
*/
    You("%sのに成功した．", lock_action());
    if (gx.xlock.door) {
        if (gx.xlock.door->doormask & D_TRAPPED) {
/*JP
            b_trapped("door", FINGER);
*/
            b_trapped("扉", FINGER);
            gx.xlock.door->doormask = D_NODOOR;
            unblock_point(u.ux + u.dx, u.uy + u.dy);
            if (*in_rooms(u.ux + u.dx, u.uy + u.dy, SHOPBASE))
                add_damage(u.ux + u.dx, u.uy + u.dy, SHOP_DOOR_COST);
            newsym(u.ux + u.dx, u.uy + u.dy);
        } else if (gx.xlock.door->doormask & D_LOCKED)
            gx.xlock.door->doormask = D_CLOSED;
        else
            gx.xlock.door->doormask = D_LOCKED;
    } else {
        gx.xlock.box->olocked = !gx.xlock.box->olocked;
        gx.xlock.box->lknown = 1;
        if (gx.xlock.box->otrapped)
            (void) chest_trap(gx.xlock.box, FINGER, FALSE);
    }
    exercise(A_DEX, TRUE);
    return ((gx.xlock.usedtime = 0));
}

void
breakchestlock(struct obj *box, boolean destroyit)
{
    if (!destroyit) { /* bill for the box but not for its contents */
        struct obj *hide_contents = box->cobj;

        box->cobj = 0;
        costly_alteration(box, COST_BRKLCK);
        box->cobj = hide_contents;
        box->olocked = 0;
        box->obroken = 1;
        box->lknown = 1;
    } else { /* #force has destroyed this box (at <u.ux,u.uy>) */
        struct obj *otmp;
        struct monst *shkp = (*u.ushops && costly_spot(u.ux, u.uy))
                                 ? shop_keeper(*u.ushops)
                                 : 0;
        boolean costly = (boolean) (shkp != 0),
                peaceful_shk = costly && (boolean) shkp->mpeaceful;
        long loss = 0L;

/*JP
        pline("In fact, you've totally destroyed %s.", the(xname(box)));
*/
        pline("実際のところ，%sを完全に壊してしまった．", xname(box));
        /* Put the contents on ground at the hero's feet. */
        while ((otmp = box->cobj) != 0) {
            obj_extract_self(otmp);
            if (!rn2(3) || otmp->oclass == POTION_CLASS) {
                chest_shatter_msg(otmp);
                if (costly)
                    loss += stolen_value(otmp, u.ux, u.uy, peaceful_shk,
                                         TRUE);
                if (otmp->quan == 1L) {
                    obfree(otmp, (struct obj *) 0);
                    continue;
                }
                /* this works because we're sure to have at least 1 left;
                   otherwise it would fail since otmp is not in inventory */
                useup(otmp);
            }
            if (box->otyp == ICE_BOX && otmp->otyp == CORPSE) {
                otmp->age = svm.moves - otmp->age; /* actual age */
                start_corpse_timeout(otmp);
            }
            place_object(otmp, u.ux, u.uy);
            stackobj(otmp);
        }
        if (costly)
            loss += stolen_value(box, u.ux, u.uy, peaceful_shk, TRUE);
        if (loss)
/*JP
            You("owe %ld %s for objects destroyed.", loss, currency(loss));
*/
            You("器物破損で%ld%sの借りをつくった．", loss, currency(loss));
        delobj(box);
    }
}

/* try to force a locked chest */
staticfn int
forcelock(void)
{
    if ((gx.xlock.box->ox != u.ux) || (gx.xlock.box->oy != u.uy))
        return ((gx.xlock.usedtime = 0)); /* you or it moved */

    if (gx.xlock.usedtime++ >= 50 || !uwep || nohands(gy.youmonst.data)) {
/*JP
        You("give up your attempt to force the lock.");
*/
        pline("鍵をこじ開けるのをあきらめた．");
        if (gx.xlock.usedtime >= 50) /* you made the effort */
            exercise((gx.xlock.picktyp) ? A_DEX : A_STR, TRUE);
        return ((gx.xlock.usedtime = 0));
    }

    if (gx.xlock.picktyp) { /* blade */
        if (rn2(1000 - (int) uwep->spe) > (992 - greatest_erosion(uwep) * 10)
            && !uwep->cursed && !obj_resists(uwep, 0, 99)) {
            /* for a +0 weapon, probability that it survives an unsuccessful
             * attempt to force the lock is (.992)^50 = .67
             */
#if 0 /*JP:T*/
            pline("%sour %s broke!", (uwep->quan > 1L) ? "One of y" : "Y",
                  xname(uwep));
#else
            pline("%sは壊れてしまった！",xname(uwep));
#endif
            useup(uwep);
/*JP
            You("give up your attempt to force the lock.");
*/
            pline("鍵をこじ開けるのをあきらめた．");
            exercise(A_DEX, TRUE);
            return ((gx.xlock.usedtime = 0));
        }
    } else             /* blunt */
        wake_nearby(FALSE); /* due to hammering on the container */

    if (rn2(100) >= gx.xlock.chance)
        return 1; /* still busy */

/*JP
    You("succeed in forcing the lock.");
*/
    pline("鍵をこじ開けた．");
    exercise(gx.xlock.picktyp ? A_DEX : A_STR, TRUE);
    /* breakchestlock() might destroy xlock.box; if so, xlock context will
       be cleared (delobj -> obfree -> maybe_reset_pick); but it might not,
       so explicitly clear that manually */
    breakchestlock(gx.xlock.box, (boolean) (!gx.xlock.picktyp && !rn2(3)));
    reset_pick(); /* lock-picking context is no longer valid */

    return 0;
}

void
reset_pick(void)
{
    gx.xlock.usedtime = gx.xlock.chance = gx.xlock.picktyp = 0;
    gx.xlock.magic_key = FALSE;
    gx.xlock.door = (struct rm *) 0;
    gx.xlock.box = (struct obj *) 0;
}

/* level change or object deletion; context may no longer be valid */
void
maybe_reset_pick(struct obj *container) /* passed from obfree() */
{
    /*
     * If a specific container, only clear context if it is for that
     * particular container (which is being deleted).  Other stuff on
     * the current dungeon level remains valid.
     * However if 'container' is Null, clear context if not carrying
     * gx.xlock.box (which might be Null if context is for a door).
     * Used for changing levels, where a floor container or a door is
     * being left behind and won't be valid on the new level but a
     * carried container will still be.  There might not be any context,
     * in which case redundantly clearing it is harmless.
     */
    if (container ? (container == gx.xlock.box)
                  : (!gx.xlock.box || !carried(gx.xlock.box)))
        reset_pick();
}

/* pick a tool for autounlock */
struct obj *
autokey(boolean opening) /* True: key, pick, or card; False: key or pick */
{
    struct obj *o, *key, *pick, *card, *akey, *apick, *acard;

    /* mundane item or regular artifact or own role's quest artifact */
    key = pick = card = (struct obj *) 0;
    /* other role's quest artifact (Rogue's Key or Tourist's Credit Card) */
    akey = apick = acard = (struct obj *) 0;
    for (o = gi.invent; o; o = o->nobj) {
        if (any_quest_artifact(o) && !is_quest_artifact(o)) {
            switch (o->otyp) {
            case SKELETON_KEY:
                if (!akey)
                    akey = o;
                break;
            case LOCK_PICK:
                if (!apick)
                    apick = o;
                break;
            case CREDIT_CARD:
                if (!acard)
                    acard = o;
                break;
            default:
                break;
            }
        } else {
            switch (o->otyp) {
            case SKELETON_KEY:
                if (!key || is_magic_key(&gy.youmonst, o))
                    key = o;
                break;
            case LOCK_PICK:
                if (!pick)
                    pick = o;
                break;
            case CREDIT_CARD:
                if (!card)
                    card = o;
                break;
            default:
                break;
            }
        }
    }
    if (!opening)
        card = acard = 0;
    /* only resort to other role's quest artifact if no other choice */
    if (!key && !pick && !card)
        key = akey;
    if (!pick && !card)
        pick = apick;
    if (!card)
        card = acard;
    return key ? key : pick ? pick : card ? card : 0;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* for doapply(); if player gives a direction or resumes an interrupted
   previous attempt then it usually costs hero a move even if nothing
   ultimately happens; when told "can't do that" before being asked for
   direction or player cancels with ESC while giving direction, it doesn't */
#define PICKLOCK_LEARNED_SOMETHING (-1) /* time passes */
#define PICKLOCK_DID_NOTHING 0          /* no time passes */
#define PICKLOCK_DID_SOMETHING 1

/* player is applying a key, lock pick, or credit card */
int
pick_lock(
    struct obj *pick,
    coordxy rx, coordxy ry, /* coordinates of door/container, for autounlock:
                             * doesn't prompt for direction if these are set */
    struct obj *container)  /* container, for autounlock */
{
    struct obj dummypick;
    int picktyp, c, ch;
    coord cc;
    struct rm *door;
    struct obj *otmp;
    char qbuf[QBUFSZ];
    boolean autounlock = (rx != 0 || container != NULL);

    /* 'pick' might be Null [called by do_loot_cont() for AUTOUNLOCK_UNTRAP] */
    if (!pick) {
        dummypick = cg.zeroobj;
        pick = &dummypick; /* pick->otyp will be STRANGE_OBJECT */
    }
    picktyp = pick->otyp;

    /* check whether we're resuming an interrupted previous attempt */
    if (gx.xlock.usedtime && picktyp == gx.xlock.picktyp) {
/*JP
        static char no_longer[] = "Unfortunately, you can no longer %s %s.";
*/
        static char no_longer[] = "ざんねんながら，あなたは%s%s．";

        if (nohands(gy.youmonst.data)) {
/*JP
            const char *what = (picktyp == LOCK_PICK) ? "pick" : "key";
*/
            const char *what = (picktyp == LOCK_PICK) ? "鍵開け器具" : "鍵";

            if (picktyp == CREDIT_CARD)
/*JP
                what = "card";
*/
                what = "カード";
/*JP
            pline(no_longer, "hold the", what);
*/
            pline(no_longer, what, "をつかめない");
            reset_pick();
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (u.uswallow || (gx.xlock.box && !can_reach_floor(TRUE))) {
/*JP
            pline(no_longer, "reach the", "lock");
*/
            pline(no_longer, "鍵に", "届かない");
            reset_pick();
            return PICKLOCK_LEARNED_SOMETHING;
        } else {
            const char *action = lock_action();

/*JP
            You("resume your attempt at %s.", action);
*/
            pline("%sのを再開した．", action);
            gx.xlock.magic_key = is_magic_key(&gy.youmonst, pick);
            set_occupation(picklock, action, 0);
            return PICKLOCK_DID_SOMETHING;
        }
    }

    if (nohands(gy.youmonst.data)) {
/*JP
        You_cant("hold %s -- you have no hands!", doname(pick));
*/
        You("%sをつかむことができない！手がないんだもの！", xname(pick));
        return PICKLOCK_DID_NOTHING;
    } else if (u.uswallow) {
#if 0 /*JP:T*/
        You_cant("%sunlock %s.", (picktyp == CREDIT_CARD) ? "" : "lock or ",
                 mon_nam(u.ustuck));
#else
        You_cant("%sを%sない．", mon_nam(u.ustuck),
                 (picktyp == CREDIT_CARD) ? "開けられ" : "開け閉めでき");
#endif
        return PICKLOCK_DID_NOTHING;
    }

    if (pick != &dummypick && picktyp != SKELETON_KEY
        && picktyp != LOCK_PICK && picktyp != CREDIT_CARD) {
        impossible("picking lock with object %d?", picktyp);
        return PICKLOCK_DID_NOTHING;
    }
    ch = 0; /* lint suppression */

    if (rx != 0) { /* autounlock; caller has provided coordinates */
        cc.x = rx;
        cc.y = ry;
#if 0 /*JP:T*/
    } else if (!get_adjacent_loc((char *) 0, "Invalid location!",
                                 u.ux, u.uy, &cc)) {
#else
    } else if (!get_adjacent_loc((char *) 0, "位置がおかしい！",
                                 u.ux, u.uy, &cc)) {
#endif
        return PICKLOCK_DID_NOTHING;
    }

    if (u_at(cc.x, cc.y)) { /* pick lock on a container */
        const char *verb;
        char qsfx[QBUFSZ];
#if 0 /*JP*/
        boolean it;
#endif
        int count;

        if (u.dz < 0 && !autounlock) { /* beware stale u.dz value */
#if 0 /*JP:T*/
            There("isn't any sort of lock up %s.",
                  Levitation ? "here" : "there");
#else
            pline("%sには鍵をかけるような物はない．",
                  Levitation ? "ここ" : "そこ");
#endif
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (is_lava(u.ux, u.uy)) {
/*JP
            pline("Doing that would probably melt %s.", yname(pick));
*/
            pline("そんなことをしたら%sが溶けてしまう．", yname(pick));
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (is_pool(u.ux, u.uy) && !Underwater) {
/*JP
            pline_The("%s has no lock.", hliquid("water"));
*/
            pline_The("%sに錠前はない．", hliquid("水"));
            return PICKLOCK_LEARNED_SOMETHING;
        }

        count = 0;
        c = 'n'; /* in case there are no boxes here */
        for (otmp = svl.level.objects[cc.x][cc.y]; otmp;
             otmp = otmp->nexthere) {
            /* autounlock on boxes: only the one that was just discovered to
               be locked; don't include any other boxes which might be here */
            if (autounlock && otmp != container)
                continue;
            if (Is_box(otmp)) {
                ++count;
                if (!can_reach_floor(TRUE)) {
/*JP
                    You_cant("reach %s from up here.", the(xname(otmp)));
*/
                    You("ここから%sに届かない．", the(xname(otmp)));
                    return PICKLOCK_LEARNED_SOMETHING;
                }
#if 0 /*JP*/
                it = 0;
#endif
                if (otmp->obroken)
/*JP
                    verb = "fix";
*/
                    verb = "修復する";
                else if (!otmp->olocked)
/*JP
                    verb = "lock", it = 1;
*/
                    verb = "鍵をかける";
                else if (picktyp != LOCK_PICK)
/*JP
                    verb = "unlock", it = 1;
*/
                    verb = "鍵をはずす";
                else
/*JP
                    verb = "pick";
*/
                    verb = "こじあける";

                if (autounlock && (flags.autounlock & AUTOUNLOCK_UNTRAP) != 0
                    && could_untrap(FALSE, TRUE)
#if 0 /*JP:T*/
                    && (c = otmp->tknown ? (otmp->otrapped ? 'y' : 'n')
                            : ynq(safe_qbuf(qbuf, "Check ", " for a trap?",
                                          otmp, yname, ysimple_name, "this")))
#else
                    && (c = otmp->tknown ? (otmp->otrapped ? 'y' : 'n')
                            : ynq(safe_qbuf(qbuf, "", "の罠を調べる？",
                                          otmp, yname, ysimple_name, "これ")))
#endif
                       != 'n') {
                    if (c == 'q')
                        return PICKLOCK_DID_NOTHING; /* c == 'q' */
                    /* c == 'y' */
                    untrap(FALSE, 0, 0, otmp);
                    return PICKLOCK_DID_SOMETHING; /* even if no trap found */
                } else if (autounlock
                          && (flags.autounlock & AUTOUNLOCK_APPLY_KEY) != 0) {
                    c = 'q';
                    if (pick != &dummypick) {
                        Sprintf(qbuf, "Unlock it with %s?", yname(pick));
                        c = ynq(qbuf);
                    }
                    if (c != 'y')
                        return PICKLOCK_DID_NOTHING;
                } else {
                    /* "There is <a box> here; <verb> <it|its lock>?" */
#if 0 /*JP:T*/
                    Sprintf(qsfx, " here; %s %s?",
                            verb, it ? "it" : "its lock");
#else
                Sprintf(qsfx, "がある．%s？", verb);
#endif
#if 0 /*JP:T*/
                    (void) safe_qbuf(qbuf, "There is ", qsfx, otmp, doname,
                                     ansimpleoname, "a box");
#else
                (void) safe_qbuf(qbuf, "ここには", qsfx, otmp, doname,
                                 ansimpleoname, "箱");
#endif
                    otmp->lknown = 1;

                    c = ynq(qbuf);
                    if (c == 'q')
                        return PICKLOCK_DID_NOTHING;
                    if (c == 'n')
                        continue; /* try next box */
                }

                if (otmp->obroken) {
#if 0 /*JP:T*/
                    You_cant("fix its broken lock with %s.",
                             ansimpleoname(pick));
#else
                    You_cant("壊れた鍵を%sで修復できない．",
                             ansimpleoname(pick));
#endif
                    return PICKLOCK_LEARNED_SOMETHING;
                } else if (picktyp == CREDIT_CARD && !otmp->olocked) {
                    /* credit cards are only good for unlocking */
#if 0 /*JP:T*/
                    You_cant("do that with %s.",
                             an(simple_typename(picktyp)));
#else
                    pline("%sじゃそんなことはできない．",
                          simple_typename(picktyp));
#endif
                    return PICKLOCK_LEARNED_SOMETHING;
                } else if (autounlock
                           && !touch_artifact(pick, &gy.youmonst)) {
                    /* note: for !autounlock, apply already did touch check */
                    return PICKLOCK_DID_SOMETHING;
                }
                switch (picktyp) {
                case CREDIT_CARD:
                    ch = ACURR(A_DEX) + 20 * Role_if(PM_ROGUE);
                    break;
                case LOCK_PICK:
                    ch = 4 * ACURR(A_DEX) + 25 * Role_if(PM_ROGUE);
                    break;
                case SKELETON_KEY:
                    ch = 75 + ACURR(A_DEX);
                    break;
                default:
                    ch = 0;
                }
                if (otmp->cursed)
                    ch /= 2;

                gx.xlock.box = otmp;
                gx.xlock.door = 0;
                break;
            }
        }
        if (c != 'y') {
            if (!count)
/*JP
                There("doesn't seem to be any sort of lock here.");
*/
                pline("ここには鍵をかけるような物はないようだ．");
            return PICKLOCK_LEARNED_SOMETHING; /* decided against all boxes */
        }

    /* not the hero's location; pick the lock in an adjacent door */
    } else {
        struct monst *mtmp;

        if (u.utrap && u.utraptype == TT_PIT) {
/*JP
            You_cant("reach over the edge of the pit.");
*/
            pline("落し穴の中からでは届かない．");
            /* this used to return PICKLOCK_LEARNED_SOMETHING but the
               #open command doesn't use a turn for similar situation */
            return PICKLOCK_DID_NOTHING;
        }

        door = &levl[cc.x][cc.y];
        mtmp = m_at(cc.x, cc.y);
        if (mtmp && canseemon(mtmp) && M_AP_TYPE(mtmp) != M_AP_FURNITURE
            && M_AP_TYPE(mtmp) != M_AP_OBJECT) {
            if (picktyp == CREDIT_CARD
                && (mtmp->isshk || mtmp->data == &mons[PM_ORACLE])) {
                SetVoice(mtmp, 0, 80, 0);
/*JP
                verbalize("No checks, no credit, no problem.");
*/
                verbalize("いつもニコニコ現金払い．");
            } else {
#if 0 /*JP:T*/
                pline("I don't think %s would appreciate that.",
                      mon_nam(mtmp));
#else
                pline("%sがその価値を認めるとは思えない．", mon_nam(mtmp));
#endif
            }
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (mtmp && is_door_mappear(mtmp)) {
            /* "The door actually was a <mimic>!" */
            stumble_onto_mimic(mtmp);
            /* mimic might keep the key (50% chance, 10% for PYEC or MKoT) */
            maybe_absorb_item(mtmp, pick, 50, 10);
            return PICKLOCK_LEARNED_SOMETHING;
        }
        if (!IS_DOOR(door->typ)) {
            int res = PICKLOCK_DID_NOTHING, oldglyph = door->glyph;
            schar oldlastseentyp = update_mapseen_for(cc.x, cc.y);

            /* this is probably only relevant when blind */
            feel_location(cc.x, cc.y);
            if (door->glyph != oldglyph
                || svl.lastseentyp[cc.x][cc.y] != oldlastseentyp)
                res = PICKLOCK_LEARNED_SOMETHING;

            if (is_drawbridge_wall(cc.x, cc.y) >= 0)
/*JP
                You("%s no lock on the drawbridge.", Blind ? "feel" : "see");
*/
                pline("跳ね橋には鍵がない%s．", Blind ? "ようだ" : "ように見える");
            else
/*JP
                You("%s no door there.", Blind ? "feel" : "see");
*/
                pline("ここには扉がない%s．", Blind ? "ようだ" : "ように見える");
            return res;
        }
        switch (door->doormask) {
        case D_NODOOR:
/*JP
            pline("This doorway has no door.");
*/
            pline("この出入口には扉がない．");
            return PICKLOCK_LEARNED_SOMETHING;
        case D_ISOPEN:
/*JP
            You("cannot lock an open door.");
*/
            pline("開いてる扉には鍵をかけられない．");
            return PICKLOCK_LEARNED_SOMETHING;
        case D_BROKEN:
/*JP
            pline("This door is broken.");
*/
            pline("この扉は壊れている．");
            return PICKLOCK_LEARNED_SOMETHING;
        default:
            if ((flags.autounlock & AUTOUNLOCK_UNTRAP) != 0
                && could_untrap(FALSE, FALSE)
/*JP
                && (c = ynq("Check this door for a trap?")) != 'n') {
*/
                && (c = ynq("この扉の罠を調べる？")) != 'n') {
                if (c == 'q')
                    return PICKLOCK_DID_NOTHING;
                /* c == 'y' */
                untrap(FALSE, cc.x, cc.y, (struct obj *) 0);
                return PICKLOCK_DID_SOMETHING; /* even if no trap found */
            }
            /* credit cards are only good for unlocking */
            if (picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
/*JP
                You_cant("lock a door with a credit card.");
*/
                You("クレジットカードで扉に鍵をかけることはできない．");
                return PICKLOCK_LEARNED_SOMETHING;
            }

#if 0 /*JP:T*/
            Sprintf(qbuf, "%s it%s%s?",
                    (door->doormask & D_LOCKED) ? "Unlock" : "Lock",
                    autounlock ? " with " : "",
                    autounlock ? yname(pick) : "");
#else
            Sprintf(qbuf, "%s%s%sますか？",
                    autounlock ? yname(pick) : "",
                    autounlock ? "で" : "",
                    (door->doormask & D_LOCKED) ? "はずし" : "かけ");
#endif
            c = ynq(qbuf);
            if (c != 'y')
                return PICKLOCK_DID_NOTHING;

            /* note: for !autounlock, 'apply' already did touch check */
            if (autounlock && !touch_artifact(pick, &gy.youmonst))
                return PICKLOCK_DID_SOMETHING;

            switch (picktyp) {
            case CREDIT_CARD:
                ch = 2 * ACURR(A_DEX) + 20 * Role_if(PM_ROGUE);
                break;
            case LOCK_PICK:
                ch = 3 * ACURR(A_DEX) + 30 * Role_if(PM_ROGUE);
                break;
            case SKELETON_KEY:
                ch = 70 + ACURR(A_DEX);
                break;
            default:
                ch = 0;
            }
            gx.xlock.door = door;
            gx.xlock.box = 0;
        }
    }
    svc.context.move = 0;
    gx.xlock.chance = ch;
    gx.xlock.picktyp = picktyp;
    gx.xlock.magic_key = is_magic_key(&gy.youmonst, pick);
    gx.xlock.usedtime = 0;
    set_occupation(picklock, lock_action(), 0);
    return PICKLOCK_DID_SOMETHING;
}

/* is hero wielding a weapon that can #force? */
boolean
u_have_forceable_weapon(void)
{
    if (!uwep /* proper type test */
        || ((uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
            ? (objects[uwep->otyp].oc_skill < P_DAGGER
               || objects[uwep->otyp].oc_skill == P_FLAIL
               || objects[uwep->otyp].oc_skill > P_LANCE)
            : uwep->oclass != ROCK_CLASS))
        return FALSE;
    return TRUE;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* the #force command - try to force a chest with your weapon */
int
doforce(void)
{
    struct obj *otmp;
    int c, picktyp;
    char qbuf[QBUFSZ];

    /*
     * TODO?
     *  allow force with edged weapon to be performed on doors.
     */

    if (u.uswallow) {
/*JP
        You_cant("force anything from inside here.");
*/
        You_cant("内側からこじ開けることはできない．");
        return ECMD_OK;
    }
    if (!u_have_forceable_weapon()) {
        boolean use_plural = uwep && uwep->quan > 1;

#if 0 /*JP:T*/
        You_cant("force anything %s weapon%s.",
                 !uwep ? "when not wielding a"
                 : (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))
                   ? (use_plural ? "without proper" : "without a proper")
                   : (use_plural ? "with those" : "with that"),
                 use_plural ? "s" : "");
#else
        You_cant("%sで鍵をこじ開けることはできない．",
                 !uwep ? "装備している武器なし"
                 : (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))
                   ? "適切な武器なし"
                   : "その武器");
#endif
        return ECMD_OK;
    }
    if (!can_reach_floor(TRUE)) {
        cant_reach_floor(u.ux, u.uy, FALSE, TRUE, FALSE);
        return ECMD_OK;
    }

    picktyp = is_blade(uwep) && !is_pick(uwep);
    if (gx.xlock.usedtime && gx.xlock.box && picktyp == gx.xlock.picktyp) {
/*JP
        You("resume your attempt to force the lock.");
*/
        pline("鍵をこじあけるのを再開した．");
/*JP
        set_occupation(forcelock, "forcing the lock", 0);
*/
        set_occupation(forcelock, "鍵をこじあける", 0);
        return ECMD_TIME;
    }

    /* A lock is made only for the honest man, the thief will break it. */
    gx.xlock.box = (struct obj *) 0;
    for (otmp = svl.level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
        if (Is_box(otmp)) {
            if (otmp->obroken || !otmp->olocked) {
                /* force doname() to omit known "broken" or "unlocked"
                   prefix so that the message isn't worded redundantly;
                   since we're about to set lknown, there's no need to
                   remember and then reset its current value */
                otmp->lknown = 0;
#if 0 /*JP:T*/
                There("is %s here, but its lock is already %s.",
                      doname(otmp), otmp->obroken ? "broken" : "unlocked");
#else
                pline("ここには%sがある．しかしその鍵はもう%s．",
                      doname(otmp),
                      otmp->obroken ? "壊れている" : "はずされている");
#endif
                otmp->lknown = 1;
                continue;
            }
#if 0 /*JP:T*/
            (void) safe_qbuf(qbuf, "There is ", " here; force its lock?",
                             otmp, doname, ansimpleoname, "a box");
#else
            (void) safe_qbuf(qbuf, "ここには", "がある．鍵をこじ開けますか？",
                             otmp, doname, ansimpleoname, "箱");
#endif
            otmp->lknown = 1;

            c = ynq(qbuf);
            if (c == 'q')
                return ECMD_OK;
            if (c == 'n')
                continue;

            if (picktyp)
/*JP
                You("force %s into a crack and pry.", yname(uwep));
*/
                You("%sをすき間に差しこんでこじあけようとした．",xname(uwep));
            else
/*JP
                You("start bashing it with %s.", yname(uwep));
*/
                pline("%sで殴りつけた．", xname(uwep));
            gx.xlock.box = otmp;
            gx.xlock.chance = objects[uwep->otyp].oc_wldam * 2;
            gx.xlock.picktyp = picktyp;
            gx.xlock.magic_key = FALSE;
            gx.xlock.usedtime = 0;
            break;
        }

    if (gx.xlock.box)
/*JP
        set_occupation(forcelock, "forcing the lock", 0);
*/
        set_occupation(forcelock, "鍵をこじあける", 0);
    else
/*JP
        You("decide not to force the issue.");
*/
        pline("それは無意味な行為だ．");
    return ECMD_TIME;
}

boolean
stumble_on_door_mimic(coordxy x, coordxy y)
{
    struct monst *mtmp;

    if ((mtmp = m_at(x, y)) && is_door_mappear(mtmp)
        && !Protection_from_shape_changers) {
        stumble_onto_mimic(mtmp);
        return TRUE;
    }
    return FALSE;
}

/* the #open command - try to open a door */
int
doopen(void)
{
    return doopen_indir(0, 0);
}

/* try to open a door in direction u.dx/u.dy */
int
doopen_indir(coordxy x, coordxy y)
{
    coord cc;
    struct rm *door;
    boolean portcullis;
    const char *dirprompt;
    int res = ECMD_OK;

    if (nohands(gy.youmonst.data)) {
/*JP
        You_cant("open anything -- you have no hands!");
*/
        You("何も開けることができない！手がないんだもの！");
        return ECMD_OK;
    }

    dirprompt = NULL; /* have get_adjacent_loc() -> getdir() use default */
    if (u.utrap && u.utraptype == TT_PIT && container_at(u.ux, u.uy, FALSE))
/*JP
        dirprompt = "Open where? [.>]";
*/
        dirprompt = "どこを開ける？ [.>]";

    if (x > 0 && y >= 0) {
        /* nonzero <x,y> is used when hero in amorphous form tries to
           flow under a closed door at <x,y>; the test here was using
           'y > 0' but that would give incorrect results if doors are
           ever allowed to be placed on the top row of the map */
        cc.x = x;
        cc.y = y;
    } else if (!get_adjacent_loc(dirprompt, (char *) 0, u.ux, u.uy, &cc)) {
        return ECMD_OK;
    }

    /* open at yourself/up/down: switch to loot unless there is a closed
       door here (possible with Passes_walls) and direction isn't 'down' */
    if (u_at(cc.x, cc.y) && (u.dz > 0 || !closed_door(u.ux, u.uy)))
        return doloot();

    /* this used to be done prior to get_adjacent_loc() but doing so was
       incorrect once open at hero's spot became an alternate way to loot */
    if (u.utrap && u.utraptype == TT_PIT) {
/*JP
        You_cant("reach over the edge of the pit.");
*/
        pline("落し穴の中から届かない．");
        return ECMD_OK;
    }

    if (stumble_on_door_mimic(cc.x, cc.y))
        return ECMD_TIME;

    /* when choosing a direction is impaired, use a turn
       regardless of whether a door is successfully targeted */
    if (Confusion || Stunned)
        res = ECMD_TIME;

    door = &levl[cc.x][cc.y];
    portcullis = (is_drawbridge_wall(cc.x, cc.y) >= 0);
    /* this used to be 'if (Blind)' but using a key skips that so we do too */
    {
        int oldglyph = door->glyph;
        schar oldlastseentyp = update_mapseen_for(cc.x, cc.y);

        newsym(cc.x, cc.y);
        if (door->glyph != oldglyph
            || svl.lastseentyp[cc.x][cc.y] != oldlastseentyp)
            res = ECMD_TIME; /* learned something */
    }

    if (portcullis || !IS_DOOR(door->typ)) {
        /* closed portcullis or spot that opened bridge would span */
        if (is_db_wall(cc.x, cc.y) || door->typ == DRAWBRIDGE_UP)
/*JP
            There("is no obvious way to open the drawbridge.");
*/
            pline("跳ね橋を降ろす明白な方法はない．");
        else if (portcullis || door->typ == DRAWBRIDGE_DOWN)
/*JP
            pline_The("drawbridge is already open.");
*/
            pline_The("跳ね橋はもう開いている．");
        else if (container_at(cc.x, cc.y, TRUE))
#if 0 /*JP:T*/
            pline("%s like something lootable over there.",
                  Blind ? "Feels" : "Seems");
#else
            pline("ここには何か入れ物があるようだ．");
#endif
        else
/*JP
            You("%s no door there.", Blind ? "feel" : "see");
*/
            pline("そこには扉はないよう%s．", Blind ? "だ" : "に見える");
        return res;
    }

    if (!(door->doormask & D_CLOSED)) {
        const char *mesg;
        boolean locked = FALSE;

        switch (door->doormask) {
        case D_BROKEN:
/*JP
            mesg = " is broken";
*/
            mesg = "扉は壊れている";
            break;
        case D_NODOOR:
/*JP
            mesg = "way has no door";
*/
            mesg = "出入口には扉がない";
            break;
        case D_ISOPEN:
/*JP
            mesg = " is already open";
*/
            mesg = "扉はもう開いている";
            break;
        default:
/*JP
            mesg = " is locked";
*/
            mesg = "扉には鍵が掛かっている";
            locked = TRUE;
            break;
        }
        set_msg_xy(cc.x, cc.y);
/*JP
        pline("This door%s.", mesg);
*/
        pline("%s．", mesg);
        if (locked && flags.autounlock) {
            struct obj *unlocktool;

            u.dz = 0; /* should already be 0 since hero moved toward door */
            if ((flags.autounlock & AUTOUNLOCK_APPLY_KEY) != 0
                && (unlocktool = autokey(TRUE)) != 0) {
                res = pick_lock(unlocktool, cc.x, cc.y,
                                (struct obj *) 0) ? ECMD_TIME : ECMD_OK;
            } else if ((flags.autounlock & AUTOUNLOCK_KICK) != 0
                       && !u.usteed /* kicking is different when mounted */
/*JP
                       && ynq("Kick it?") == 'y') {
*/
                       && ynq("蹴る？") == 'y') {
                cmdq_add_ec(CQ_CANNED, dokick);
                cmdq_add_dir(CQ_CANNED,
                             sgn(cc.x - u.ux), sgn(cc.y - u.uy), 0);
                /* this was 'ECMD_TIME', but time shouldn't elapse until
                   the canned kick takes place */
                res = ECMD_OK;
            }
        }
        return res;
    }

    if (verysmall(gy.youmonst.data)) {
/*JP
        pline("You're too small to pull the door open.");
*/
        You("扉を押すには小さすぎる．");
        return res;
    }

    /* door is known to be CLOSED */
    if (rnl(20) < (ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 3) {
        set_msg_xy(cc.x, cc.y);
/*JP
        pline_The("door opens.");
*/
        pline("扉は開いた．");
        if (door->doormask & D_TRAPPED) {
/*JP
            b_trapped("door", FINGER);
*/
            b_trapped("扉", FINGER);
            door->doormask = D_NODOOR;
            if (*in_rooms(cc.x, cc.y, SHOPBASE))
                add_damage(cc.x, cc.y, SHOP_DOOR_COST);
        } else
            door->doormask = D_ISOPEN;
        feel_newsym(cc.x, cc.y); /* the hero knows she opened it */
        recalc_block_point(cc.x, cc.y); /* vision: new see through there */
    } else {
        exercise(A_STR, TRUE);
        set_msg_xy(cc.x, cc.y);
/*JP
        pline_The("door resists!");
*/
        pline("なかなか開かない！");
    }

    return ECMD_TIME;
}

staticfn boolean
obstructed(coordxy x, coordxy y, boolean quietly)
{
    struct monst *mtmp = m_at(x, y);

    if (mtmp && M_AP_TYPE(mtmp) != M_AP_FURNITURE) {
        if (M_AP_TYPE(mtmp) == M_AP_OBJECT)
            goto objhere;
        if (!quietly) {
            char *Mn = Some_Monnam(mtmp); /* Monnam, Someone or Something */

            if ((mtmp->mx != x || mtmp->my != y) && canspotmon(mtmp))
                /* s_suffix() returns a modifiable buffer */
/*JP
                Mn = strcat(s_suffix(Mn), " tail");
*/
                Mn = strcat(s_suffix(Mn), "尻尾");

#if 0 /*JP:T*/
            pline("%s blocks the way!", Mn);
#else
            pline("%sが立ちふさがっている！", Mn);
#endif
        }
        if (!canspotmon(mtmp))
            map_invisible(x, y);
        return TRUE;
    }
    if (OBJ_AT(x, y)) {
 objhere:
        if (!quietly)
/*JP
            pline("%s's in the way.", Something);
*/
            pline("何かが出入口にある．");
        return TRUE;
    }
    return FALSE;
}

/* the #close command - try to close a door */
int
doclose(void)
{
    coordxy x, y;
    struct rm *door;
    boolean portcullis;
    int res = ECMD_OK;

    if (nohands(gy.youmonst.data)) {
/*JP
        You_cant("close anything -- you have no hands!");
*/
        You("閉めることができない！手がないんだもの！");
        return ECMD_OK;
    }

    if (u.utrap && u.utraptype == TT_PIT) {
/*JP
        You_cant("reach over the edge of the pit.");
*/
        pline("落し穴の中から届かない．");
        return ECMD_OK;
    }

    if (!getdir((char *) 0))
        return ECMD_CANCEL;

    x = u.ux + u.dx;
    y = u.uy + u.dy;
    if (u_at(x, y) && !Passes_walls) {
/*JP
        You("are in the way!");
*/
        pline("あなたが出入口にいるので閉まらない！");
        return ECMD_TIME;
    }

    if (!isok(x, y))
        goto nodoor;

    if (stumble_on_door_mimic(x, y))
        return ECMD_TIME;

    /* when choosing a direction is impaired, use a turn
       regardless of whether a door is successfully targeted */
    if (Confusion || Stunned)
        res = ECMD_TIME;

    door = &levl[x][y];
    portcullis = (is_drawbridge_wall(x, y) >= 0);
    if (Blind) {
        int oldglyph = door->glyph;
        schar oldlastseentyp = update_mapseen_for(x, y);

        feel_location(x, y);
        if (door->glyph != oldglyph
            || svl.lastseentyp[x][y] != oldlastseentyp)
            res = ECMD_TIME; /* learned something */
    }

    if (portcullis || !IS_DOOR(door->typ)) {
        /* is_db_wall: closed portcullis */
        if (is_db_wall(x, y) || door->typ == DRAWBRIDGE_UP)
/*JP
            pline_The("drawbridge is already closed.");
*/
            pline_The("跳ね橋はもう閉じている．");
        else if (portcullis || door->typ == DRAWBRIDGE_DOWN)
/*JP
            There("is no obvious way to close the drawbridge.");
*/
            pline("跳ね橋を閉める明白な方法はない．");
        else {
 nodoor:
/*JP
            You("%s no door there.", Blind ? "feel" : "see");
*/
            pline("そこに扉はないよう%s．", Blind ? "だ" : "に見える");
        }
        return res;
    }

    if (door->doormask == D_NODOOR) {
/*JP
        pline("This doorway has no door.");
*/
        pline("出入口には扉がない．");
        return res;
    } else if (obstructed(x, y, FALSE)) {
        return res;
    } else if (door->doormask == D_BROKEN) {
/*JP
        pline("This door is broken.");
*/
        pline("扉は壊れている．");
        return res;
    } else if (door->doormask & (D_CLOSED | D_LOCKED)) {
/*JP
        pline("This door is already closed.");
*/
        pline("扉はもう閉じている．");
        return res;
    }

    if (door->doormask == D_ISOPEN) {
        if (verysmall(gy.youmonst.data) && !u.usteed) {
/*JP
            pline("You're too small to push the door closed.");
*/
            You("小さすぎて扉を閉められない．");
            return res;
        }
        if (u.usteed
            || rn2(25) < (ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 3) {
/*JP
            pline_The("door closes.");
*/
            pline("扉は閉じた．");
            door->doormask = D_CLOSED;
            feel_newsym(x, y); /* the hero knows she closed it */
            block_point(x, y); /* vision:  no longer see there */
        } else {
            exercise(A_STR, TRUE);
/*JP
            pline_The("door resists!");
*/
            pline("なかなか閉まらない！");
        }
    }

    return ECMD_TIME;
}

/* box obj was hit with spell or wand effect otmp;
   returns true if something happened */
boolean
boxlock(struct obj *obj, struct obj *otmp) /* obj *is* a box */
{
    boolean res = 0;

    switch (otmp->otyp) {
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        if (!obj->olocked) { /* lock it; fix if broken */
            Soundeffect(se_klunk, 50);
/*JP
            pline("Klunk!");
*/
            pline("カチ！");
            obj->olocked = 1;
            obj->obroken = 0;
            if (Role_if(PM_WIZARD))
                obj->lknown = 1;
            else
                obj->lknown = 0;
            res = 1;
        } /* else already closed and locked */
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (obj->olocked) { /* unlock; isn't broken so doesn't need fixing */
            Soundeffect(se_klick, 50);
/*JP
            pline("Klick!");
*/
            pline("コンコン！");
            obj->olocked = 0;
            res = 1;
            if (Role_if(PM_WIZARD))
                obj->lknown = 1;
            else
                obj->lknown = 0;
        } else /* silently fix if broken */
            obj->obroken = 0;
        break;
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
        /* maybe start unlocking chest, get interrupted, then zap it;
           we must avoid any attempt to resume unlocking it */
        if (gx.xlock.box == obj)
            reset_pick();
        break;
    }
    return res;
}

/* Door/secret door was hit with spell or wand effect otmp;
   returns true if something happened */
boolean
doorlock(struct obj *otmp, coordxy x, coordxy y)
{
    struct rm *door = &levl[x][y];
    boolean res = TRUE;
    int loudness = 0;
    const char *msg = (const char *) 0;
/*JP
    const char *dustcloud = "A cloud of dust";
*/
    const char *dustcloud = "ほこり";
/*JP
    const char *quickly_dissipates = "quickly dissipates";
*/
    const char *quickly_dissipates = "あっと言うまに飛び散った";
    boolean mysterywand = (otmp->oclass == WAND_CLASS && !otmp->dknown);

    if (door->typ == SDOOR) {
        switch (otmp->otyp) {
        case WAN_OPENING:
        case SPE_KNOCK:
        case WAN_STRIKING:
        case SPE_FORCE_BOLT:
            door->typ = DOOR;
            door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
            newsym(x, y);
            if (cansee(x, y))
/*JP
                pline("A door appears in the wall!");
*/
                pline("壁から扉が現れた！");
            if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK)
                return TRUE;
            break; /* striking: continue door handling below */
        case WAN_LOCKING:
        case SPE_WIZARD_LOCK:
        default:
            return FALSE;
        }
    }

    switch (otmp->otyp) {
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        if (Is_rogue_level(&u.uz)) {
            boolean vis = cansee(x, y);

            /* Can't have real locking in Rogue, so just hide doorway */
            if (vis) {
/*JP
                pline("%s springs up in the older, more primitive doorway.",
*/
                pline("古くさい，原始的な出入口に%sが立ちこめた．",
                      dustcloud);
            } else {
                Soundeffect(se_swoosh, 25);
/*JP
                You_hear("a swoosh.");
*/
                You_hear("シューッという音を聞いた．");
            }
            if (obstructed(x, y, mysterywand)) {
                if (vis)
/*JP
                    pline_The("cloud %s.", quickly_dissipates);
*/
                    pline("ほこりは%s．",quickly_dissipates);
                return FALSE;
            }
            block_point(x, y);
            door->typ = SDOOR, door->doormask = D_NODOOR;
            if (vis)
/*JP
                pline_The("doorway vanishes!");
*/
                pline("出入口は消えた！");
            newsym(x, y);
            return TRUE;
        }
        if (obstructed(x, y, mysterywand))
            return FALSE;
        /* Don't allow doors to close over traps.  This is for pits */
        /* & trap doors, but is it ever OK for anything else? */
        if (t_at(x, y)) {
            /* maketrap() clears doormask, so it should be NODOOR */
#if 0 /*JP:T*/
            pline("%s springs up in the doorway, but %s.", dustcloud,
                  quickly_dissipates);
#else
            pline("%sが出入口に立ちこめた，しかし%s．", dustcloud,
                  quickly_dissipates);
#endif
            return FALSE;
        }

        switch (door->doormask & ~D_TRAPPED) {
        case D_CLOSED:
/*JP
            msg = "The door locks!";
*/
            msg = "扉に鍵がかかった！";
            break;
        case D_ISOPEN:
/*JP
            msg = "The door swings shut, and locks!";
*/
            msg = "扉は勢いよく閉まり，鍵がかかった！";
            break;
        case D_BROKEN:
/*JP
            msg = "The broken door reassembles and locks!";
*/
            msg = "壊れた扉が再構成され，鍵がかかった！";
            break;
        case D_NODOOR:
            msg =
/*JP
               "A cloud of dust springs up and assembles itself into a door!";
*/
               "ほこりがたちこめ，集まって扉になった！";
            break;
        default:
            res = FALSE;
            break;
        }
        block_point(x, y);
        door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
        newsym(x, y);
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (door->doormask & D_LOCKED) {
/*JP
            msg = "The door unlocks!";
*/
            msg = "扉の鍵ははずれた！";
            door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
        } else
            res = FALSE;
        break;
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        if (door->doormask & (D_LOCKED | D_CLOSED)) {
            /* sawit: closed door location is more visible than open */
            boolean sawit, seeit;

            if (door->doormask & D_TRAPPED) {
                struct monst *mtmp = m_at(x, y);

                sawit = mtmp ? canseemon(mtmp) : cansee(x, y);
                door->doormask = D_NODOOR;
                unblock_point(x, y);
                newsym(x, y);
                seeit = mtmp ? canseemon(mtmp) : cansee(x, y);
                if (mtmp) {
                    (void) mb_trapped(mtmp, sawit || seeit);
                } else {
                    /* for mtmp, mb_trapped() does is own wake_nearto() */
                    loudness = 40;
                    if (flags.verbose) {
                        Soundeffect(se_kaboom_door_explodes, 75);
                        if ((sawit || seeit) && !Unaware) {
/*JP
                            pline("KABOOM!!  You see a door explode.");
*/
                            pline("ちゅどーん！扉が爆発した．");
                        } else if (!Deaf) {
                            Soundeffect(se_explosion, 75);
#if 0 /*JP:T*/
                            You_hear("a %s explosion.",
                                     (distu(x, y) > 7 * 7) ? "distant"
                                                           : "nearby");
#else
                            You_hear("%sくの爆発音を聞いた．",
                                     (distu(x, y) > 7 * 7) ? "遠"
                                                           : "近");
#endif
                        }
                    }
                }
                break;
            }
            sawit = cansee(x, y);
            door->doormask = D_BROKEN;
            recalc_block_point(x, y);
            seeit = cansee(x, y);
            newsym(x, y);
            if (flags.verbose) {
                if ((sawit || seeit) && !Unaware) {
/*JP
                    pline_The("door crashes open!");
*/
                    pline("扉は壊れ開いた！");
                } else if (!Deaf) {
                    Soundeffect(se_crashing_sound, 100);
/*JP
                    You_hear("a crashing sound.");
*/
                    You_hear("何かが壊れる音を聞いた．");
                }
            }
            /* force vision recalc before printing more messages */
            if (gv.vision_full_recalc)
                vision_recalc(0);
            loudness = 20;
        } else
            res = FALSE;
        break;
    default:
        impossible("magic (%d) attempted on door.", otmp->otyp);
        break;
    }
    if (msg && cansee(x, y))
        pline1(msg);
    if (loudness > 0) {
        /* door was destroyed */
        wake_nearto(x, y, loudness);
        if (*in_rooms(x, y, SHOPBASE))
            add_damage(x, y, 0L);
    }

    if (res && picking_at(x, y)) {
        /* maybe unseen monster zaps door you're unlocking */
        stop_occupation();
        reset_pick();
    }
    return res;
}

staticfn void
chest_shatter_msg(struct obj *otmp)
{
    const char *disposition;
    const char *thing;
    long save_HBlinded, save_BBlinded;

    if (otmp->oclass == POTION_CLASS) {
#if 0 /*JP:T*/
        You("%s %s shatter!", Blind ? "hear" : "see", an(bottlename()));
#else
        if (Blind)
            You_hear("%sが割れる音を聞いた！", bottlename());
        else
            pline("%sが割れた！", bottlename());
#endif
        if (!breathless(gy.youmonst.data) || haseyes(gy.youmonst.data))
            potionbreathe(otmp);
        return;
    }
    /* We have functions for distant and singular names, but not one */
    /* which does _both_... */
    save_HBlinded = HBlinded,  save_BBlinded = BBlinded;
    HBlinded = 1L,  BBlinded = 0L;
    thing = singular(otmp, xname);
    HBlinded = save_HBlinded,  BBlinded = save_BBlinded;
    switch (objects[otmp->otyp].oc_material) {
    case PAPER:
/*JP
        disposition = "is torn to shreds";
*/
        disposition = "は寸断された";
        break;
    case WAX:
/*JP
        disposition = "is crushed";
*/
        disposition = "を床にぶちまけた";
        break;
    case VEGGY:
/*JP
        disposition = "is pulped";
*/
        disposition = "はどろどろになった";
        break;
    case FLESH:
/*JP
        disposition = "is mashed";
*/
        disposition = "はどろどろになった";
        break;
    case GLASS:
/*JP
        disposition = "shatters";
*/
        disposition = "は割れた";
        break;
    case WOOD:
/*JP
        disposition = "splinters to fragments";
*/
        disposition = "はかけらになった";
        break;
    default:
/*JP
        disposition = "is destroyed";
*/
        disposition = "は壊れた";
        break;
    }
/*JP
    pline("%s %s!", An(thing), disposition);
*/
    pline("%s%s！", thing, disposition);
}

/*lock.c*/
