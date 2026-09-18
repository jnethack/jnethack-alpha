/* NetHack 5.0	do_name.c	$NHDT-Date: 1737013431 2025/01/15 23:43:51 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.326 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Pasi Kallinen, 2018. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-                */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

staticfn char *nextmbuf(void);
staticfn char *name_from_player(char *, const char *, const char *);
staticfn void do_mgivenname(void);
staticfn boolean alreadynamed(struct monst *, char *, char *) NONNULLPTRS;
staticfn void do_oname(struct obj *) NONNULLARG1;
staticfn char *docall_xname(struct obj *) NONNULLARG1;
staticfn void namefloorobj(void);

#define NUMMBUF 5

/* manage a pool of BUFSZ buffers, so callers don't have to */
staticfn char *
nextmbuf(void)
{
    static char NEARDATA bufs[NUMMBUF][BUFSZ];
    static int bufidx = 0;

    bufidx = (bufidx + 1) % NUMMBUF;
    return bufs[bufidx];
}

/* allocate space for a monster's name; removes old name if there is one */
void
new_mgivenname(
    struct monst *mon,
    int lth) /* desired length (caller handles adding 1 for terminator) */
{
    if (lth) {
        /* allocate mextra if necessary; otherwise get rid of old name */
        if (!mon->mextra)
            mon->mextra = newmextra();
        else
            free_mgivenname(mon); /* has mextra, might also have name */
        MGIVENNAME(mon) = (char *) alloc((unsigned) lth);
    } else {
        /* zero length: the new name is empty; get rid of the old name */
        if (has_mgivenname(mon))
            free_mgivenname(mon);
    }
}

/* release a monster's name; retains mextra even if all fields are now null */
void
free_mgivenname(struct monst *mon)
{
    if (has_mgivenname(mon)) {
        free((genericptr_t) MGIVENNAME(mon));
        MGIVENNAME(mon) = (char *) 0;
    }
}

/* allocate space for an object's name; removes old name if there is one */
void
new_oname(
    struct obj *obj,
    int lth) /* desired length (caller handles adding 1 for terminator) */
{
    if (lth) {
        /* allocate oextra if necessary; otherwise get rid of old name */
        if (!obj->oextra)
            obj->oextra = newoextra();
        else
            free_oname(obj); /* already has oextra, might also have name */
        ONAME(obj) = (char *) alloc((unsigned) lth);
    } else {
        /* zero length: the new name is empty; get rid of the old name */
        if (has_oname(obj))
            free_oname(obj);
    }
}

/* release an object's name; retains oextra even if all fields are now null */
void
free_oname(struct obj *obj)
{
    if (has_oname(obj)) {
        free((genericptr_t) ONAME(obj));
        ONAME(obj) = (char *) 0;
    }
}

/*  safe_oname() always returns a valid pointer to
 *  a string, either the pointer to an object's name
 *  if it has one, or a pointer to an empty string
 *  if it doesn't.
 */
const char *
safe_oname(struct obj *obj)
{
    if (has_oname(obj))
        return ONAME(obj);
    return "";
}

/* get a name for a monster or an object from player;
   truncate if longer than PL_PSIZ, then return it */
staticfn char *
name_from_player(
    char *outbuf,       /* output buffer, assumed to be at least BUFSZ long;
                         * anything longer than PL_PSIZ will be truncated */
    const char *prompt,
    const char *defres) /* only used if EDIT_GETLIN is enabled; only useful
                         * if windowport xxx's xxx_getlin() supports that */
{
    outbuf[0] = '\0';
#ifdef EDIT_GETLIN
    if (defres && *defres)
        Strcpy(outbuf, defres); /* default response from getlin() */
#else
    nhUse(defres);
#endif
    getlin(prompt, outbuf);
    if (!*outbuf || *outbuf == '\033')
        return NULL;

    /* strip leading and trailing spaces, condense internal sequences */
    (void) mungspaces(outbuf);
    if (strlen(outbuf) >= PL_PSIZ)
#if 0 /*JP*/
        outbuf[PL_PSIZ - 1] = '\0';
#else
        truncate_japanese(outbuf, PL_PSIZ - 1);
#endif
    return outbuf;
}

/* historical note: this returns a monster pointer because it used to
   allocate a new bigger block of memory to hold the monster and its name */
struct monst *
christen_monst(struct monst *mtmp, const char *name)
{
    int lth;
    char buf[PL_PSIZ];

    /* dogname & catname are PL_PSIZ arrays; object names have same limit */
    lth = (name && *name) ? ((int) strlen(name) + 1) : 0;
    if (lth > PL_PSIZ) {
        lth = PL_PSIZ;
#if 0 /*JP:T*/
        name = strncpy(buf, name, PL_PSIZ - 1);
        buf[PL_PSIZ - 1] = '\0';
#else
        strscpy_japanese(buf, name, PL_PSIZ);
        name = buf;
#endif
    }
    new_mgivenname(mtmp, lth); /* removes old name if one is present */
    if (lth)
        Strcpy(MGIVENNAME(mtmp), name);
    /* if 'mtmp' is leashed, persistent inventory window needs updating */
    if (mtmp->mleashed)
        update_inventory(); /* x - leash (attached to Fido) */
    return mtmp;
}

/* check whether user-supplied name matches or nearly matches an unnameable
   monster's name, or is an attempt to delete the monster's name; if so, give
   alternate reject message for do_mgivenname() */
staticfn boolean
alreadynamed(struct monst *mtmp, char *monnambuf, char *usrbuf)
{
#if 0 /*JP*//*unused*/
    char pronounbuf[10], *p;
#else
    char *p;
#endif

    if (!*usrbuf) { /* attempt to erase existing name */
        boolean name_not_title = (has_mgivenname(mtmp)
                                  || type_is_pname(mtmp->data)
                                  || mtmp->isshk);
        pline("%s would rather keep %s existing %s.", upstart(monnambuf),
              is_rider(mtmp->data) ? "its" : mhis(mtmp),
              name_not_title ? "name" : "title");
        return TRUE;
    } else if (fuzzymatch(usrbuf, monnambuf, " -_", TRUE)
               /* catch trying to name "the Oracle" as "Oracle" */
               || (!strncmpi(monnambuf, "the ", 4)
                   && fuzzymatch(usrbuf, monnambuf + 4, " -_", TRUE))
               /* catch trying to name "invisible Orcus" as "Orcus" */
               || ((p = strstri(monnambuf, "invisible ")) != 0
                   && fuzzymatch(usrbuf, p + 10, " -_", TRUE))
               /* catch trying to name "the priest of Crom" as "Crom" */
               || ((p = strstri(monnambuf, " of ")) != 0
                   && fuzzymatch(usrbuf, p + 4, " -_", TRUE))) {
        if (is_rider(mtmp->data)) {
            /* avoid gendered pronoun for riders */
/*JP
            pline("%s is already called that.", upstart(monnambuf));
*/
            pline("%sはすでにそう呼ばれている．", monnambuf);
        } else {
#if 0 /*JP:T*/
            pline("%s is already called %s.",
                  upstart(strcpy(pronounbuf, mhe(mtmp))), monnambuf);
#else
        pline("それは既に%sと呼ばれている．",
              monnambuf);
#endif
        }
        return TRUE;
#if 0 /*JP*//*日本語では使わない*/
    } else if (mtmp->data == &mons[PM_JUIBLEX]
               && strstri(monnambuf, "Juiblex")
               && !strcmpi(usrbuf, "Jubilex")) {
        pline("%s doesn't like being called %s.", upstart(monnambuf), usrbuf);
        return TRUE;
#endif
    }
    return FALSE;
}

/* allow player to assign a name to some chosen monster */
staticfn void
do_mgivenname(void)
{
    char buf[BUFSZ], monnambuf[BUFSZ], qbuf[QBUFSZ];
    coord cc;
    int cx, cy;
    struct monst *mtmp = 0;
    boolean do_swallow = FALSE;

    if (Hallucination) {
/*JP
        You("would never recognize it anyway.");
*/
        You("それを認識できない．");
        return;
    }
    cc.x = u.ux;
    cc.y = u.uy;
/*JP
    if (getpos(&cc, FALSE, "the monster you want to name") < 0
*/
    if (getpos(&cc, FALSE, "あなたが名づけたい怪物") < 0
        || !isok(cc.x, cc.y))
        return;
    cx = cc.x, cy = cc.y;

    if (u_at(cx, cy)) {
        if (u.usteed && canspotmon(u.usteed)) {
            mtmp = u.usteed;
        } else {
/*JP
            pline("This %s creature is called %s and cannot be renamed.",
*/
            pline("この%s生き物は%sと呼ばれていて，名前は変更できない．",
                  beautiful(), svp.plname);
            return;
        }
    } else
        mtmp = m_at(cx, cy);

    /* Allow you to name the monster that has swallowed you */
    if (!mtmp && u.uswallow) {
        int glyph = glyph_at(cx, cy);

        if (glyph_is_swallow(glyph)) {
            mtmp = u.ustuck;
            do_swallow = TRUE;
        }
    }

    if (!do_swallow && (!mtmp
        || (!sensemon(mtmp)
            && (!(cansee(cx, cy) || see_with_infrared(mtmp))
                || mtmp->mundetected || M_AP_TYPE(mtmp) == M_AP_FURNITURE
                || M_AP_TYPE(mtmp) == M_AP_OBJECT
                || (mtmp->minvis && !See_invisible))))) {

/*JP
        pline("I see no monster there.");
*/
        pline("そこに怪物はいない．");
        return;
    }
    /* special case similar to the one in lookat() */
/*JP
    Sprintf(qbuf, "What do you want to call %s?",
*/
    Sprintf(qbuf, "%sを何と呼びますか？",
            distant_monnam(mtmp, ARTICLE_THE, monnambuf));
    /* use getlin() to get a name string from the player */
    if (!name_from_player(buf, qbuf,
                          has_mgivenname(mtmp) ? MGIVENNAME(mtmp) : NULL))
        return;

    /* Unique monsters have their own specific names or titles.
     * Shopkeepers, temple priests and other minions use alternate
     * name formatting routines which ignore any user-supplied name.
     *
     * Don't say a new name is being rejected if it happens to match
     * the existing name, or if the player is trying to remove the
     * monster's existing name without assigning a new one.
     */
    if ((mtmp->data->geno & G_UNIQ) && !mtmp->ispriest) {
        if (!alreadynamed(mtmp, monnambuf, buf))
/*JP
            pline("%s doesn't like being called names!", upstart(monnambuf));
*/
            pline("%sはあだ名で呼ばれるのが嫌いなようだ！", Monnam(mtmp));
    } else if (mtmp->isshk
               && !(Deaf || helpless(mtmp)
                    || mtmp->data->msound <= MS_ANIMAL)) {
        if (!alreadynamed(mtmp, monnambuf, buf)) {
            SetVoice(mtmp, 0, 80, 0);
/*JP
            verbalize("I'm %s, not %s.", shkname(mtmp), buf);
*/
            verbalize("私は%sだ，%sではない．", shkname(mtmp), buf);
        }
    } else if (mtmp->ispriest || mtmp->isminion || mtmp->isshk
               || mtmp->data == &mons[PM_GHOST] || has_ebones(mtmp)) {
        if (!alreadynamed(mtmp, monnambuf, buf))
/*JP
            pline("%s will not accept the name %s.", upstart(monnambuf), buf);
*/
            pline("%sは%sという名前を受けいれなかった．", monnambuf, buf);
    } else {
        (void) christen_monst(mtmp, buf);
    }
}

/*
 * This routine used to change the address of 'obj' so be unsafe if not
 * used with extreme care.  Applying a name to an object no longer
 * allocates a replacement object, so that old risk is gone.
 */
staticfn void
do_oname(struct obj *obj)
{
    char *bufp, buf[BUFSZ], bufcpy[BUFSZ], qbuf[QBUFSZ];
    const char *aname;
    short objtyp = STRANGE_OBJECT;

    /* Do this now because there's no point in even asking for a name */
    if (obj->otyp == SPE_NOVEL) {
/*JP
        pline("%s already has a published name.", Ysimple_name2(obj));
*/
        pline("%sにはすでに出版時の名前がある．", Ysimple_name2(obj));
        return;
    }

#if 0 /*JP:T*/
    Sprintf(qbuf, "What do you want to name %s ",
            is_plural(obj) ? "these" : "this");
    (void) safe_qbuf(qbuf, qbuf, "?", obj, xname, simpleonames, "item");
#else
    (void) safe_qbuf(qbuf, "", "を何と名づけますか？", obj, xname,
                     simpleonames, "それ");
#endif
    /* use getlin() to get a name string from the player */
    if (!name_from_player(buf, qbuf, safe_oname(obj)))
        return;

    /*
     * We don't violate illiteracy conduct here, although it is
     * arguable that we should for anything other than "X".  Doing so
     * would make attaching player's notes to hero's inventory have an
     * in-game effect, which may or may not be the correct thing to do.
     *
     * We do violate illiteracy in oname() if player creates Sting or
     * Orcrist, clearly being literate (no pun intended...).
     */

    if (obj->oartifact) {
        /* this used to give "The artifact seems to resist the attempt."
           but resisting is definite, no "seems to" about it */
#if 0 /*JP:T*/
        pline("%s resists the attempt.",
              /* any artifact should always pass the has_oname() test
                 but be careful just in case */
              has_oname(obj) ? ONAME(obj) : "The artifact");
#else
        pline("%sは名づけを拒否した．",
              /* any artifact should always pass the has_oname() test
                 but be careful just in case */
              has_oname(obj) ? ONAME(obj) : "聖器");
#endif
        return;
    }

    /* relax restrictions over proper capitalization for artifacts */
    if ((aname = artifact_name(buf, &objtyp, TRUE)) != 0
        && (restrict_name(obj, aname) || exist_artifact(obj->otyp, aname))) {
        /* substitute canonical spelling before slippage */
        Strcpy(buf, aname);
        /* this used to change one letter, substituting a value
           of 'a' through 'y' (due to an off by one error, 'z'
           would never be selected) and then force that to
           upper case if such was the case of the input;
           now, the hand slip scuffs one or two letters as if
           the text had been trodden upon, sometimes picking
           punctuation instead of an arbitrary letter;
           unfortunately, we have to cover the possibility of
           it targeting spaces so failing to make any change
           (we know that it must eventually target a nonspace
           because buf[] matches a valid artifact name) */
        Strcpy(bufcpy, buf);
        /* for "the Foo of Bar", only scuff "Foo of Bar" part */
        bufp = !strncmpi(buf, "the ", 4) ? (buf + 4) : buf;
        do {
            wipeout_text(bufp, rnd_on_display_rng(2), (unsigned) 0);
        } while (!strcmp(buf, bufcpy));
/*JP
        pline("While engraving, your %s slips.", body_part(HAND));
*/
        pline("刻んでいる間に%sが滑ってしまった．", body_part(HAND));
        display_nhwindow(WIN_MESSAGE, FALSE);
/*JP
        You("engrave: \"%s\".", buf);
*/
        You("刻んだ: 「%s」．",buf);
        /* violate illiteracy conduct since hero attempted to write
           a valid artifact name */
        u.uconduct.literate++;
    } else if (obj->otyp == objtyp) {
        /* artifact_name() always returns non-Null when it sets objtyp */
        assert(aname != 0);

        /* artifact_name() found a match and restrict_name() didn't reject
           it; since 'obj' is the right type, naming will change it into an
           artifact so use canonical capitalization (Sting or Orcrist) */
        Strcpy(buf, aname);
    }

    obj = oname(obj, buf, ONAME_VIA_NAMING | ONAME_KNOW_ARTI);
    nhUse(obj);
}

struct obj *
oname(
    struct obj *obj,  /* item to assign name to */
    const char *name, /* name to assign */
    unsigned oflgs)   /* flags, mostly for artifact creation */
{
    int lth;
    char buf[PL_PSIZ];
    boolean via_naming = (oflgs & ONAME_VIA_NAMING) != 0,
            skip_inv_update = (oflgs & ONAME_SKIP_INVUPD) != 0;

    lth = *name ? (int) (strlen(name) + 1) : 0;
    if (lth > PL_PSIZ) {
        lth = PL_PSIZ;
#if 0 /*JP*/
        name = strncpy(buf, name, PL_PSIZ - 1);
        buf[PL_PSIZ - 1] = '\0';
#else
        {
            int end = lth - 1;
            end -= offset_in_kanji(name, end);
            name = strncpy(buf, name, end);
            buf[end] = '\0';
            lth = end + 1;
        }
#endif
    }
    /* If named artifact exists in the game, do not create another.
       Also trying to create an artifact shouldn't de-artifact
       it (e.g. Excalibur from prayer). In this case the object
       will retain its current name. */
    if (obj->oartifact || (lth && exist_artifact(obj->otyp, name)))
        return obj;

    new_oname(obj, lth); /* removes old name if one is present */
    if (lth)
        Strcpy(ONAME(obj), name);

    if (lth)
        artifact_exists(obj, name, TRUE, oflgs);
    if (obj->oartifact) {
        /* can't dual-wield with artifact as secondary weapon */
        if (obj == uswapwep)
            untwoweapon();
        /* activate warning if you've just named your weapon "Sting" */
        if (obj == uwep)
            set_artifact_intrinsic(obj, TRUE, W_WEP);
        /* if obj is owned by a shop, increase your bill */
        if (obj->unpaid)
            alter_cost(obj, 0L);
        if (via_naming) {
            /* violate illiteracy conduct since successfully wrote arti-name */
            if (!u.uconduct.literate++)
                livelog_printf(LL_CONDUCT | LL_ARTIFACT,
                               "became literate by naming %s",
                               bare_artifactname(obj));
            else
                livelog_printf(LL_ARTIFACT,
                               "chose %s to be named \"%s\"",
                               ansimpleoname(obj), bare_artifactname(obj));
        }
    }
    if (carried(obj) && !skip_inv_update)
        update_inventory();
    return obj;
}

boolean
objtyp_is_callable(int i)
{
    if (objects[i].oc_uname)
        return TRUE;

    switch(objects[i].oc_class) {
    case AMULET_CLASS:
        /* 5.0: calling these used to be allowed but that enabled the
           player to tell whether two unID'd amulets of yendor were both
           fake or one was real by calling them distinct names and then
           checking discoveries to see whether first name was replaced
           by second or both names stuck; with more than two available
           to work with, if they weren't all fake it was possible to
           determine which one was the real one */
        if (i == AMULET_OF_YENDOR || i == FAKE_AMULET_OF_YENDOR)
            break; /* return FALSE */
        FALLTHROUGH;
        /*FALLTHRU*/
    case SCROLL_CLASS:
    case POTION_CLASS:
    case WAND_CLASS:
    case RING_CLASS:
    case GEM_CLASS:
    case SPBOOK_CLASS:
    case ARMOR_CLASS:
    case TOOL_CLASS:
    case VENOM_CLASS:
        if (OBJ_DESCR(objects[i]))
            return TRUE;
        break;
    default:
        break;
    }
    return FALSE;
}

/* getobj callback for object to name (specific item) - anything but gold */
int
name_ok(struct obj *obj)
{
    if (!obj || obj->oclass == COIN_CLASS)
        return GETOBJ_EXCLUDE;

    if (!obj->dknown || obj->oartifact || obj->otyp == SPE_NOVEL)
        return GETOBJ_DOWNPLAY;

    return GETOBJ_SUGGEST;
}

/* getobj callback for object to call (name its type) */
int
call_ok(struct obj *obj)
{
    if (!obj || !objtyp_is_callable(obj->otyp))
        return GETOBJ_EXCLUDE;

    /* not a likely candidate if not seen yet since naming will fail,
       or if it has been discovered and doesn't already have a name;
       when something has been named and then becomes discovered, it
       remains a likely candidate until player renames it to <space>
       to remove that no longer needed name */
    if (!obj->dknown || (objects[obj->otyp].oc_name_known
                         && !objects[obj->otyp].oc_uname))
        return GETOBJ_DOWNPLAY;

    return GETOBJ_SUGGEST;
}

/* #call / #name command - player can name monster or object or type of obj */
int
docallcmd(void)
{
    struct obj *obj;
    winid win;
    anything any;
    menu_item *pick_list = 0;
    struct _cmd_queue cq, *cmdq;
    char ch = 0;
    /* if player wants a,b,c instead of i,o when looting, do that here too */
    boolean abc = flags.lootabc;
    int clr = NO_COLOR;

    if ((cmdq = cmdq_pop()) != 0) {
        cq = *cmdq;
        free((genericptr_t) cmdq);
        if (cq.typ == CMDQ_KEY)
            ch = cq.key;
        else
            cmdq_clear(CQ_CANNED);
        goto docallcmd;
    }
    win = create_nhwindow(NHW_MENU);
    start_menu(win, MENU_BEHAVE_STANDARD);
    any = cg.zeroany;
    any.a_char = 'm'; /* group accelerator 'C' */
    add_menu(win, &nul_glyphinfo, &any, abc ? 0 : any.a_char, 'C',
/*JP
             ATR_NONE, clr, "a monster", MENU_ITEMFLAGS_NONE);
*/
             ATR_NONE, clr, "怪物", MENU_ITEMFLAGS_NONE);
    if (gi.invent) {
        /* we use y and n as accelerators so that we can accept user's
           response keyed to old "name an individual object?" prompt */
        any.a_char = 'i'; /* group accelerator 'y' */
        add_menu(win, &nul_glyphinfo, &any, abc ? 0 : any.a_char, 'y',
/*JP
                 ATR_NONE, clr, "a particular object in inventory",
*/
                 ATR_NONE, clr, "持ち物の中の一つのアイテム",
                 MENU_ITEMFLAGS_NONE);
        any.a_char = 'o'; /* group accelerator 'n' */
        add_menu(win, &nul_glyphinfo, &any, abc ? 0 : any.a_char, 'n',
/*JP
                 ATR_NONE, clr, "the type of an object in inventory",
*/
                 ATR_NONE, clr, "持ち物の中の一つのアイテムの種類",
                 MENU_ITEMFLAGS_NONE);
    }
    any.a_char = 'f'; /* group accelerator ',' (or ':' instead?) */
    add_menu(win, &nul_glyphinfo, &any, abc ? 0 : any.a_char, ',',
/*JP
             ATR_NONE, clr, "the type of an object upon the floor",
*/
             ATR_NONE, clr, "床の上にある一つのアイテムの種類",
             MENU_ITEMFLAGS_NONE);
    any.a_char = 'd'; /* group accelerator '\' */
    add_menu(win, &nul_glyphinfo, &any, abc ? 0 : any.a_char, '\\',
/*JP
             ATR_NONE, clr, "the type of an object on discoveries list",
*/
             ATR_NONE, clr, "発見物一覧にある一つのアイテムの種類",
             MENU_ITEMFLAGS_NONE);
    any.a_char = 'a'; /* group accelerator 'l' */
    add_menu(win, &nul_glyphinfo, &any, abc ? 0 : any.a_char, 'l',
/*JP
             ATR_NONE, clr, "record an annotation for the current level",
*/
             ATR_NONE, clr, "現在の階に対するメモの記録",
             MENU_ITEMFLAGS_NONE);
/*JP
    end_menu(win, "What do you want to name?");
*/
    end_menu(win, "どれに名前をつけますか？");
    if (select_menu(win, PICK_ONE, &pick_list) > 0) {
        ch = pick_list[0].item.a_char;
        free((genericptr_t) pick_list);
    } else
        ch = 'q';
    destroy_nhwindow(win);

 docallcmd:
    switch (ch) {
    default:
    case 'q':
        break;
    case 'm': /* name a visible monster */
        do_mgivenname();
        break;
    case 'i': /* name an individual object in inventory */
        obj = getobj("name", name_ok, GETOBJ_PROMPT);
        if (obj)
            do_oname(obj);
        break;
    case 'o': /* name a type of object in inventory */
        obj = getobj("call", call_ok, GETOBJ_NOFLAGS);
        if (obj) {
            /* behave as if examining it in inventory;
               this might set dknown if it was picked up
               while blind and the hero can now see */
            (void) xname(obj);

            if (!obj->dknown) {
/*JP
                You("would never recognize another one.");
*/
                You("他に認識できない．");
#if 0
            } else if (call_ok(obj) == GETOBJ_EXCLUDE) {
                You("know those as well as you ever will.");
#endif
            } else {
                docall(obj);
            }
        }
        break;
    case 'f': /* name a type of object visible on the floor */
        namefloorobj();
        break;
    case 'd': /* name a type of object on the discoveries list */
        rename_disco();
        break;
    case 'a': /* annotate level */
        donamelevel();
        break;
    }
    return ECMD_OK;
}

/* for use by safe_qbuf() */
staticfn char *
docall_xname(struct obj *obj)
{
    struct obj otemp;

    otemp = *obj;
    otemp.oextra = (struct oextra *) 0;
    otemp.quan = 1L;
    /* in case water is already known, convert "[un]holy water" to "water" */
    otemp.blessed = otemp.cursed = 0;
    /* remove attributes that are doname() caliber but get formatted
       by xname(); most of these fixups aren't really needed because the
       relevant type of object isn't callable so won't reach this far */
    if (otemp.oclass == WEAPON_CLASS)
        otemp.opoisoned = 0; /* not poisoned */
    else if (otemp.oclass == POTION_CLASS)
        otemp.odiluted = 0; /* not diluted */
    else if (otemp.otyp == TOWEL || otemp.otyp == STATUE)
        otemp.spe = 0; /* not wet or historic */
    else if (otemp.otyp == TIN)
        otemp.known = 0; /* suppress tin type (homemade, &c) and mon type */
    else if (otemp.otyp == FIGURINE)
        otemp.corpsenm = NON_PM; /* suppress mon type */
    else if (otemp.otyp == HEAVY_IRON_BALL)
        otemp.owt = objects[HEAVY_IRON_BALL].oc_weight; /* not "very heavy" */
    else if (otemp.oclass == FOOD_CLASS && otemp.globby)
        otemp.owt = 120; /* 6*20, neither a small glob nor a large one */

    return an(xname(&otemp));
}

void
docall(struct obj *obj)
{
    char buf[BUFSZ], qbuf[QBUFSZ];
    char **uname_p;
    boolean had_name = FALSE;

    if (!obj->dknown)
        return; /* probably blind; Blind || Hallucination for 'fromsink' */
    flush_screen(1); /* buffered updates might matter to player's response */

    if (obj->oclass == POTION_CLASS && obj->fromsink)
        /* fromsink: kludge, meaning it's sink water */
/*JP
        Sprintf(qbuf, "Call a stream of %s fluid:",
*/
        Sprintf(qbuf, "%s液体:",
                OBJ_DESCR(objects[obj->otyp]));
    else
#if 0 /*JP:T*/
        (void) safe_qbuf(qbuf, "Call ", ":", obj,
                         docall_xname, simpleonames, "thing");
#else
        (void) safe_qbuf(qbuf, "", "に何と名前を付ける？", obj,
                         docall_xname, simpleonames, "これ");
#endif
    /* pointer to old name */
    uname_p = &(objects[obj->otyp].oc_uname);
    /* use getlin() to get a name string from the player */
    if (!name_from_player(buf, qbuf, *uname_p))
        return;

    /* clear old name */
    if (*uname_p) {
        had_name = TRUE;
        free((genericptr_t) *uname_p), *uname_p = NULL; /* clear oc_uname */
    }

    /* strip leading and trailing spaces; uncalls item if all spaces */
    (void) mungspaces(buf);
    if (!*buf) {
        if (had_name) /* possibly remove from disco[]; old *uname_p is gone */
            undiscover_object(obj->otyp);
    } else {
        *uname_p = dupstr(buf);
        discover_object(obj->otyp, FALSE, TRUE, TRUE); /* possibly add to disco[] */
    }
    if (obj->where == OBJ_INVENT || carrying(obj->otyp))
        update_inventory();
}

staticfn void
namefloorobj(void)
{
    coord cc;
    int glyph;
    char buf[BUFSZ];
    struct obj *obj = 0;
#if 0 /*JP*/
    boolean fakeobj = FALSE, use_plural;
#else
    boolean fakeobj = FALSE;
#endif

    cc.x = u.ux, cc.y = u.uy;
    /* "dot for under/over you" only makes sense when the cursor hasn't
       been moved off the hero's '@' yet, but there's no way to adjust
       the help text once getpos() has started */
#if 0 /*JP:T*/
    Sprintf(buf, "object on map (or '.' for one %s you)",
            (u.uundetected && hides_under(gy.youmonst.data))
              ? "over" : "under");
#else
    Sprintf(buf, "地図上の物(あるいは'.'であなたのいる場所)");
#endif
    if (getpos(&cc, FALSE, buf) < 0 || cc.x <= 0)
        return;
    if (u_at(cc.x, cc.y)) {
        obj = vobj_at(u.ux, u.uy);
    } else {
        glyph = glyph_at(cc.x, cc.y);
        if (glyph_is_object(glyph))
            fakeobj = object_from_map(glyph, cc.x, cc.y, &obj);
        /* else 'obj' stays null */
    }
    if (!obj) {
        /* "under you" is safe here since there's no object to hide under */
#if 0 /*JP:T*/
        There("doesn't seem to be any object %s.",
              u_at(cc.x, cc.y) ? "under you" : "there");
#else
        pline("%sには何もないようだ．",
              u_at(cc.x, cc.y) ? "あなたの下" : "そこ");
#endif
        return;
    }
    /* note well: 'obj' might be an instance of STRANGE_OBJECT if target
       is a mimic; passing that to xname (directly or via simpleonames)
       would yield "glorkum" so we need to handle it explicitly; it will
       always fail the Hallucination test and pass the !callable test,
       resulting in the "can't be assigned a type name" message */
    Strcpy(buf, (obj->otyp != STRANGE_OBJECT)
                 ? simpleonames(obj)
                 : obj_descr[STRANGE_OBJECT].oc_name);
#if 0 /*JP*/
    use_plural = (obj->quan > 1L);
#endif
    if (Hallucination) {
        const char *unames[6];
        char tmpbuf[BUFSZ];

        /* straight role name */
        unames[0] = ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
                     ? gu.urole.name.f
                     : gu.urole.name.m;
        /* random rank title for hero's role

           note: the 30 is hardcoded in xlev_to_rank, so should be
           hardcoded here too */
        unames[1] = rank_of(rn2_on_display_rng(30) + 1,
                            Role_switch, flags.female);
        /* random fake monster */
        unames[2] = bogusmon(tmpbuf, (char *) 0);
        /* increased chance for fake monster */
        unames[3] = unames[2];
        /* traditional */
        unames[4] = roguename();
        /* silly */
/*JP
        unames[5] = "Wibbly Wobbly";
*/
        unames[5] = "うろうろ";
#if 0 /*JP:T*/
        pline("%s %s to call you \"%s.\"",
              The(buf), use_plural ? "decide" : "decides",
              unames[rn2_on_display_rng(SIZE(unames))]);
#else
        pline("%sはあなたを「%s」と呼ぶことに決めた．",
              buf,
              unames[rn2_on_display_rng(SIZE(unames))]);
#endif
    } else if (call_ok(obj) == GETOBJ_EXCLUDE) {
#if 0 /*JP:T*/
        pline("%s %s can't be assigned a type name.",
              use_plural ? "Those" : "That", buf);
#else
        pline("%sに種類の名前を割り当てることはできない．",
              buf);
#endif
    } else if (!obj->dknown) {
#if 0 /*JP:T*/
        You("don't know %s %s well enough to name %s.",
            use_plural ? "those" : "that", buf, use_plural ? "them" : "it");
#else
        You("名前を付けられるほど%sのことをよく知らない．",
            buf);
#endif
    } else {
        docall(obj);
    }
    if (fakeobj) {
        obj->where = OBJ_FREE; /* object_from_map() sets it to OBJ_FLOOR */
        dealloc_obj(obj); /* has no contents */
    }
}

static const char *const ghostnames[] = {
    /* these names should have length < PL_NSIZ */
    /* Capitalize the names for aesthetics -dgk */
    "Adri",    "Andries",       "Andreas",     "Bert",    "David",  "Dirk",
    "Emile",   "Frans",         "Fred",        "Greg",    "Hether", "Jay",
    "John",    "Jon",           "Karnov",      "Kay",     "Kenny",  "Kevin",
    "Maud",    "Michiel",       "Mike",        "Peter",   "Robert", "Ron",
    "Tom",     "Wilmar",        "Nick Danger", "Phoenix", "Jiro",   "Mizue",
    "Stephan", "Lance Braccus", "Shadowhawk",  "Murphy"
};

/* ghost names formerly set by x_monnam(), now by makemon() instead */
const char *
rndghostname(void)
{
    return rn2(7) ? ROLL_FROM(ghostnames)
                  : (const char *) svp.plname;
}

/*
 * Monster naming functions:
 * x_monnam is the generic monster-naming function.
 *                seen        unseen       detected               named
 * mon_nam:     the newt        it      the invisible orc       Fido
 * noit_mon_nam:your newt (as if detected) your invisible orc   Fido
 * some_mon_nam:the newt    someone     the invisible orc       Fido
 *          or              something
 * l_monnam:    newt            it      invisible orc           dog called Fido
 * Monnam:      The newt        It      The invisible orc       Fido
 * noit_Monnam: Your newt (as if detected) Your invisible orc   Fido
 * Some_Monnam: The newt    Someone     The invisible orc       Fido
 *          or              Something
 * Adjmonnam:   The poor newt   It      The poor invisible orc  The poor Fido
 * Amonnam:     A newt          It      An invisible orc        Fido
 * a_monnam:    a newt          it      an invisible orc        Fido
 * m_monnam:    newt            xan     orc                     Fido
 * y_monnam:    your newt     your xan  your invisible orc      Fido
 * YMonnam:     Your newt     Your xan  Your invisible orc      Fido
 * noname_monnam(mon,article):
 *              article newt    art xan art invisible orc       art dog
 */

/*
 * article
 *
 * ARTICLE_NONE, ARTICLE_THE, ARTICLE_A: obvious
 * ARTICLE_YOUR: "your" on pets, "the" on everything else
 *
 * If the monster would be referred to as "it" or if the monster has a name
 * _and_ there is no adjective, "invisible", "saddled", etc., override this
 * and always use no article.
 *
 * suppress
 *
 * SUPPRESS_IT, SUPPRESS_INVISIBLE, SUPPRESS_HALLUCINATION, SUPPRESS_SADDLE.
 * SUPPRESS_MAPPEARANCE: if monster is mimicking another monster (cloned
 *              Wizard or quickmimic pet), describe the real monster rather
 *              than its current form;
 * EXACT_NAME: combination of all the above
 * SUPPRESS_NAME: omit monster's assigned name (unless uniq w/ pname).
 * AUGMENT_IT: not suppression but shares suppression bitmask; if result
 *              would have been "it", return "someone" if humanoid or
 *              "something" otherwise.
 *
 * Bug: if the monster is a priest or shopkeeper, not every one of these
 * options works, since those are special cases.
 */
char *
x_monnam(
    struct monst *mtmp,
    int article,
    const char *adjective,
    int suppress,
    boolean called)
{
    char *buf = nextmbuf();
    struct permonst *mdat = mtmp->data;
    const char *pm_name;
    boolean do_hallu, do_invis, do_it, do_saddle, do_mappear,
            do_exact, do_name, augment_it;
#if 0 /*JP*//*unused*/
    boolean name_at_start, has_adjectives, insertbuf2,
            mappear_as_mon = (M_AP_TYPE(mtmp) == M_AP_MONSTER);
#else
    boolean name_at_start,
            mappear_as_mon = (M_AP_TYPE(mtmp) == M_AP_MONSTER);
#endif
#if 0 /*JP*//*unused*/
    char *bp, buf2[BUFSZ];
#else
    char buf2[BUFSZ];
#endif

    if (mtmp == &gy.youmonst)
        return strcpy(buf, "you"); /* ignore article, "invisible", &c */

    if (program_state.gameover)
        suppress |= SUPPRESS_HALLUCINATION;
    if (article == ARTICLE_YOUR && !mtmp->mtame)
        article = ARTICLE_THE;

    if (u.uswallow && mtmp == u.ustuck) {
        /*
         * This monster has become important, for the moment anyway.
         * As the hero's consumer, it is worthy of ARTICLE_THE.
         * Also, suppress invisible as that particular characteristic
         * is unimportant now and you can see its interior anyway.
         */
        article = ARTICLE_THE;
        suppress |= SUPPRESS_INVISIBLE;
    }
    do_hallu = Hallucination && !(suppress & SUPPRESS_HALLUCINATION);
    do_invis = mtmp->minvis && !(suppress & SUPPRESS_INVISIBLE);
    do_it = !canspotmon(mtmp) && article != ARTICLE_YOUR
            && !program_state.gameover && mtmp != u.usteed
            && !engulfing_u(mtmp) && !(suppress & SUPPRESS_IT);
    do_saddle = !(suppress & SUPPRESS_SADDLE);
    do_mappear = mappear_as_mon && !(suppress & SUPPRESS_MAPPEARANCE);
    do_exact = (suppress & EXACT_NAME) == EXACT_NAME;
    do_name = !(suppress & SUPPRESS_NAME) || type_is_pname(mdat);
    augment_it = (suppress & AUGMENT_IT) != 0;

    buf[0] = '\0';

    /* unseen monsters, etc.; usually "it" but sometimes more specific;
       when hallucinating, the more specific values might be inverted */
    if (do_it) {
        /* !is_animal excludes all Y; !mindless excludes Z, M, \' */
        boolean s_one = humanoid(mdat) && !is_animal(mdat) && !mindless(mdat);

#if 0 /*JP:T*/
        Strcpy(buf, !augment_it ? "it"
                    : (!do_hallu ? s_one : !rn2(2)) ? "someone"
                      : "something");
#else
        Strcpy(buf, !augment_it ? "何者か"
                    : (!do_hallu ? s_one : !rn2(2)) ? "だれか"
                      : "何か");
#endif
        return buf;
    }

    /* priests and minions: don't even use this function */
    if ((mtmp->ispriest || mtmp->isminion) && !do_mappear) {
        char *name;
        long save_prop = EHalluc_resistance;
        unsigned save_invis = mtmp->minvis;

        /* when true name is wanted, explicitly block Hallucination */
        if (!do_hallu)
            EHalluc_resistance = 1L;
        if (!do_invis)
            mtmp->minvis = 0;
        /* EXACT_NAME will force "of <deity>" on the Astral Plane */
        name = priestname(mtmp, article, do_exact, buf2);
        EHalluc_resistance = save_prop;
        mtmp->minvis = save_invis;
#if 0 /*JP*/
        if (article == ARTICLE_NONE && !strncmp(name, "the ", 4))
            name += 4;
#endif
        return strcpy(buf, name);
    }

    /* 'pm_name' is the base part of most names */
    if (do_mappear) {
        /*assert(ismnum(mtmp->mappearance));*/
        pm_name = pmname(&mons[mtmp->mappearance], Mgender(mtmp));
    } else {
        pm_name = mon_pmname(mtmp);
    }

    /* Shopkeepers: use shopkeeper name.  For normal shopkeepers, just
     * "Asidonhopo"; for unusual ones, "Asidonhopo the invisible
     * shopkeeper" or "Asidonhopo the blue dragon".  If hallucinating,
     * none of this applies.
     */
    if (mtmp->isshk && !do_hallu && !do_mappear) {
#if 0 /*JP*/
        if (adjective && article == ARTICLE_THE) {
            /* pathological case: "the angry Asidonhopo the blue dragon"
               sounds silly */
            Strcpy(buf, "the ");
            Strcat(strcat(buf, adjective), " ");
            Strcat(buf, shkname(mtmp));
        } else {
            Strcat(buf, shkname(mtmp));
            if (mdat != &mons[PM_SHOPKEEPER] || do_invis){
                Strcat(buf, " the ");
                if (do_invis)
                    Strcat(buf, "invisible ");
                Strcat(buf, pm_name);
            }
        }
        return buf;
#else
        if (mdat == &mons[PM_SHOPKEEPER] && !do_invis){
            Strcpy(buf, shkname(mtmp));
        } else {
            Sprintf(buf, "%sという名の%s%s",
                    shkname(mtmp), do_invis ? "姿の見えない" : "",
                    pm_name);
        }
        return buf;
#endif
    }

    /* Put the adjectives in the buffer */
    if (adjective)
/*JP
        Strcat(strcat(buf, adjective), " ");
*/
        Strcat(buf, adjective);
    if (do_invis)
/*JP
        Strcat(buf, "invisible ");
*/
        Strcat(buf, "姿の見えない");
    if (do_saddle && (mtmp->misc_worn_check & W_SADDLE) && !Blind
        && !Hallucination)
/*JP
        Strcat(buf, "saddled ");
*/
        Strcat(buf, "鞍のついている");
#if 0 /*JP*/
    has_adjectives = (buf[0] != '\0');
#endif

    /* Put the actual monster name or type into the buffer now.
       Remember whether the buffer starts with a personal name. */
    if (do_hallu) {
        char rnamecode;
        char *rname = rndmonnam(&rnamecode);

        Strcat(buf, rname);
        name_at_start = bogon_is_pname(rnamecode);
    } else if (do_name && has_mgivenname(mtmp)) {
        char *name = MGIVENNAME(mtmp);

#if 0
      /* hardfought */
      if (has_ebones(mtmp)) {
#endif
        if (mdat == &mons[PM_GHOST]) {
/*JP
            Sprintf(eos(buf), "%s ghost", s_suffix(name));
*/
            Sprintf(eos(buf), "%sの幽霊", name);
#if 0 /*JP*/
            name_at_start = TRUE;
#endif
        } else if (called) {
/*JP
            Sprintf(eos(buf), "%s called %s", pm_name, name);
*/
            Sprintf(eos(buf), "%sという名の%s", name, pm_name);
#if 0 /*JP*/
            name_at_start = (boolean) type_is_pname(mdat);
#endif
#if 0 /*JP*//*定冠詞の処理は不要*/
        } else if (is_mplayer(mdat) && (bp = strstri(name, " the ")) != 0) {
            /* <name> the <adjective> <invisible> <saddled> <rank> */
            char pbuf[BUFSZ];

            Strcpy(pbuf, name);
            pbuf[bp - name + 5] = '\0'; /* adjectives right after " the " */
            if (has_adjectives)
                Strcat(pbuf, buf);
            Strcat(pbuf, bp + 5); /* append the rest of the name */
            Strcpy(buf, pbuf);
            article = ARTICLE_NONE;
            name_at_start = TRUE;
#endif
        } else {
            Strcat(buf, name);
#if 0 /*JP*/
            name_at_start = TRUE;
#endif
        }
#if 0 /* hardfought */
      }
#endif
    } else if (is_mplayer(mdat) && !In_endgame(&u.uz)) {
        char pbuf[BUFSZ];

        Strcpy(pbuf, rank_of((int) mtmp->m_lev, monsndx(mdat),
                             (boolean) mtmp->female));
#if 0 /*JP*/
        Strcat(buf, lcase(pbuf));
#else
        Strcat(buf, pbuf);
#endif
#if 0 /*JP*/
        name_at_start = FALSE;
#endif
    } else {
        Strcat(buf, pm_name);
#if 0 /*JP*/
        name_at_start = (boolean) type_is_pname(mdat);
#endif
    }

#if 0 /*JP*//*日本語に冠詞はない*/
    if (name_at_start && (article == ARTICLE_YOUR || !has_adjectives)) {
        if (mdat == &mons[PM_WIZARD_OF_YENDOR])
            article = ARTICLE_THE;
        else
            article = ARTICLE_NONE;
    } else if ((mdat->geno & G_UNIQ) != 0 && article == ARTICLE_A) {
        article = ARTICLE_THE;
    }

    insertbuf2 = TRUE;
    buf2[0] = '\0'; /* lint suppression */
    switch (article) {
    case ARTICLE_YOUR:
        Strcpy(buf2, "your ");
        break;
    case ARTICLE_THE:
        Strcpy(buf2, "the ");
        break;
    case ARTICLE_A:
        /* avoid an() here */
        (void) just_an(buf2, buf); /* copy "a " or "an " into buf2[] */
        break;
    case ARTICLE_NONE:
    default:
        insertbuf2 = FALSE;
        break;
    }
    if (insertbuf2) {
        Strcat(buf2, buf); /* buf2[] isn't viable to return,  */
        Strcpy(buf, buf2); /* so transfer the result to buf[] */
    }
#endif
    return buf;
}

char *
l_monnam(struct monst *mtmp)
{
    return x_monnam(mtmp, ARTICLE_NONE, (char *) 0,
                    (has_mgivenname(mtmp)) ? SUPPRESS_SADDLE : 0, TRUE);
}

char *
mon_nam(struct monst *mtmp)
{
    return x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                    (has_mgivenname(mtmp)) ? SUPPRESS_SADDLE : 0, FALSE);
}

/* print the name as if mon_nam() (y_monnam() if tame) was called, but
   assume that the player can always see the monster--used for probing and
   for monsters aggravating the player with a cursed potion of invisibility;
   also used for pet moving "reluctantly" onto cursed object when that pet
   can be seen either before or after it moves */
char *
noit_mon_nam(struct monst *mtmp)
{
    return x_monnam(mtmp, ARTICLE_YOUR, (char *) 0,
                    (has_mgivenname(mtmp) ? (SUPPRESS_SADDLE | SUPPRESS_IT)
                                          : SUPPRESS_IT),
                    FALSE);
}

/* in between noit_mon_nam() and mon_nam(); if the latter would pick "it",
   use "someone" (for humanoids) or "something" (for others) instead */
char *
some_mon_nam(struct monst *mtmp)
{
    return x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                    (has_mgivenname(mtmp) ? (SUPPRESS_SADDLE | AUGMENT_IT)
                                          : AUGMENT_IT),
                    FALSE);
}

char *
Monnam(struct monst *mtmp)
{
    char *bp = mon_nam(mtmp);

    *bp = highc(*bp);
    return bp;
}

char *
noit_Monnam(struct monst *mtmp)
{
    char *bp = noit_mon_nam(mtmp);

    *bp = highc(*bp);
    return bp;
}

char *
Some_Monnam(struct monst *mtmp)
{
    char *bp = some_mon_nam(mtmp);

    *bp = highc(*bp);
    return bp;
}

/* return "a dog" rather than "Fido", honoring hallucination and visibility */
char *
noname_monnam(struct monst *mtmp, int article)
{
    return x_monnam(mtmp, article, (char *) 0, SUPPRESS_NAME, FALSE);
}

/* monster's own name -- overrides hallucination and [in]visibility
   so shouldn't be used in ordinary messages (mainly for disclosure) */
char *
m_monnam(struct monst *mtmp)
{
    return x_monnam(mtmp, ARTICLE_NONE, (char *) 0, EXACT_NAME, FALSE);
}

/* pet name: "your little dog" */
char *
y_monnam(struct monst *mtmp)
{
    int prefix, suppression_flag;

    prefix = mtmp->mtame ? ARTICLE_YOUR : ARTICLE_THE;
    suppression_flag = (has_mgivenname(mtmp)
                        /* "saddled" is redundant when mounted */
                        || mtmp == u.usteed)
                           ? SUPPRESS_SADDLE
                           : 0;

    return x_monnam(mtmp, prefix, (char *) 0, suppression_flag, FALSE);
}

/* y_monnam() for start of sentence */
char *
YMonnam(struct monst *mtmp)
{
    char *bp = y_monnam(mtmp);

    *bp = highc(*bp);
    return bp;
}

char *
Adjmonnam(struct monst *mtmp, const char *adj)
{
    char *bp = x_monnam(mtmp, ARTICLE_THE, adj,
                        has_mgivenname(mtmp) ? SUPPRESS_SADDLE : 0, FALSE);

    *bp = highc(*bp);
    return bp;
}

char *
a_monnam(struct monst *mtmp)
{
    return x_monnam(mtmp, ARTICLE_A, (char *) 0,
                    has_mgivenname(mtmp) ? SUPPRESS_SADDLE : 0, FALSE);
}

char *
Amonnam(struct monst *mtmp)
{
    char *bp = a_monnam(mtmp);

    *bp = highc(*bp);
    return bp;
}

/* used for monster ID by the '/', ';', and 'C' commands to block remote
   identification of the endgame altars via their attending priests */
char *
distant_monnam(
    struct monst *mon,
    int article, /* only ARTICLE_NONE and ARTICLE_THE are handled here */
    char *outbuf)
{
    /* high priest(ess)'s identity is concealed on the Astral Plane,
       unless you're adjacent (overridden for hallucination which does
       its own obfuscation) */
    if (mon->data == &mons[PM_HIGH_CLERIC] && !Hallucination
        && Is_astralevel(&u.uz) && !m_next2u(mon)) {
#if 0 /*JP:T*/
        Strcpy(outbuf, article == ARTICLE_THE ? "the " : "");
        Strcat(outbuf, mon->female ? "high priestess" : "high priest");
#else
        Strcpy(outbuf, "法王");
#endif
    } else {
        Strcpy(outbuf, x_monnam(mon, article, (char *) 0, 0, TRUE));
    }
    return outbuf;
}

/* returns mon_nam(mon) relative to other_mon; normal name unless they're
   the same, in which case the reference is to {him|her|it} self */
char *
mon_nam_too(struct monst *mon, struct monst *other_mon)
{
    char *outbuf;

    if (mon != other_mon) {
        outbuf = mon_nam(mon);
    } else {
        outbuf = nextmbuf();
#if 0 /*JP*/
        switch (pronoun_gender(mon, PRONOUN_HALLU)) {
        case 0:
            Strcpy(outbuf, "himself");
            break;
        case 1:
            Strcpy(outbuf, "herself");
            break;
        default:
        case 2:
            Strcpy(outbuf, "itself");
            break;
        case 3: /* could happen when hallucinating */
            Strcpy(outbuf, "themselves");
            break;
        }
#else
        Strcpy(outbuf, "自分自身");
#endif
    }
    return outbuf;
}

#if 0 /*JP*//*unused*/
/* construct "<monnamtext> <verb> <othertext> {him|her|it}self" which might
   be distorted by Hallu; if that's plural, adjust monnamtext and verb */
char *
monverbself(
    struct monst *mon,
    char *monnamtext, /* modifiable 'mbuf' with adequate room at end */
    const char *verb,
    const char *othertext)
{
    char *verbs, selfbuf[40]; /* sizeof "themselves" suffices */

    /* "himself"/"herself"/"itself", maybe "themselves" if hallucinating */
    Strcpy(selfbuf, mon_nam_too(mon, mon));
    /* verb starts plural; this will yield singular except for "themselves" */
    verbs = vtense(selfbuf, verb);
    if (!strcmp(verb, verbs)) { /* a match indicates that it stayed plural */
        monnamtext = makeplural(monnamtext);
        /* for "it", makeplural() produces "them" but we want "they" */
        if (!strcmpi(monnamtext, genders[3].he)) {
            boolean capitaliz = (monnamtext[0] == highc(monnamtext[0]));

            Strcpy(monnamtext, genders[3].him);
            if (capitaliz)
                monnamtext[0] = highc(monnamtext[0]);
        }
    }
    Strcat(strcat(monnamtext, " "), verbs);
    if (othertext && *othertext)
        Strcat(strcat(monnamtext, " "), othertext);
    Strcat(strcat(monnamtext, " "), selfbuf);
    Sprintf(eos(monnamtext), "は%s自分自身に%s", othertext, verb);
    return monnamtext;
}
#endif

/* for debugging messages, where data might be suspect and we aren't
   taking what the hero does or doesn't know into consideration */
char *
minimal_monnam(struct monst *mon, boolean ckloc)
{
    struct permonst *ptr;
    char *outbuf = nextmbuf();

    if (!mon) {
        Strcpy(outbuf, "[Null monster]");
    } else if ((ptr = mon->data) == 0) {
        Strcpy(outbuf, "[Null mon->data]");
    } else if (ptr < &mons[0]) {
        Sprintf(outbuf, "[Invalid mon->data %s < %s]",
                fmt_ptr((genericptr_t) mon->data),
                fmt_ptr((genericptr_t) &mons[0]));
    } else if (ptr >= &mons[NUMMONS]) {
        Sprintf(outbuf, "[Invalid mon->data %s >= %s]",
                fmt_ptr((genericptr_t) mon->data),
                fmt_ptr((genericptr_t) &mons[NUMMONS]));
    } else if (ckloc && ptr == &mons[PM_LONG_WORM] && mon->mx
               && svl.level.monsters[mon->mx][mon->my] != mon) {
        Sprintf(outbuf, "%s <%d,%d>",
                pmname(&mons[PM_LONG_WORM_TAIL], Mgender(mon)),
                mon->mx, mon->my);
    } else {
        Sprintf(outbuf, "%s%s <%d,%d>",
                mon->mtame ? "tame " : mon->mpeaceful ? "peaceful " : "",
                mon_pmname(mon), mon->mx, mon->my);
        if (mon->cham != NON_PM)
            Sprintf(eos(outbuf), "{%s}",
                    pmname(&mons[mon->cham], Mgender(mon)));
    }
    return outbuf;
}

#ifndef PMNAME_MACROS
int
Mgender(struct monst *mtmp)
{
    int mgender = MALE;

    if (mtmp == &gy.youmonst) {
        if (Upolyd ? u.mfemale : flags.female)
            mgender = FEMALE;
    } else if (mtmp->female) {
        mgender = FEMALE;
    }
    return mgender;
}

const char *
pmname(struct permonst *pm, int mgender)
{
    if (mgender < MALE || mgender >= NUM_MGENDERS || !pm->pmnames[mgender])
        mgender = NEUTRAL;
    return pm->pmnames[mgender];
}
#endif /* PMNAME_MACROS */

/* mons[]->pmname for a monster */
const char *
mon_pmname(struct monst *mon)
{
    /* for neuter, mon->data->pmnames[MALE] will be Null and use [NEUTRAL] */
    return pmname(mon->data, Mgender(mon));
}

/* mons[]->pmname for a corpse or statue or figurine */
const char *
obj_pmname(struct obj *obj)
{
#if 0   /* ignore saved montraits even when they're available; they determine
         * what a corpse would revive as if resurrected (human corpse from
         * slain vampire revives as vampire rather than as human, for example)
         * and don't necessarily reflect the state of the corpse itself */
    if (has_omonst(obj)) {
        struct monst *m = OMONST(obj);

        /* obj->oextra->omonst->data is Null but ...->mnum is set */
        if (ismnum(m->mnum))
            return pmname(&mons[m->mnum], Mgender(m));
    }
#endif
    if ((obj->otyp == CORPSE || obj->otyp == STATUE || obj->otyp == FIGURINE)
        && ismnum(obj->corpsenm)) {
        int cgend = (obj->spe & CORPSTAT_GENDER),
            mgend = ((cgend == CORPSTAT_MALE) ? MALE
                     : (cgend == CORPSTAT_FEMALE) ? FEMALE
                       : NEUTRAL),
            mndx = obj->corpsenm;

        /* mons[].pmnames[] for monster cleric uses "priest" or "priestess"
           or "aligned cleric"; we want to avoid "aligned cleric [corpse]"
           unless it has been explicitly flagged as neuter rather than
           defaulting to random (which fails male or female check above);
           role monster cleric uses "priest" or "priestess" or "cleric"
           without "aligned" prefix so we switch to that; [can't force
           random gender to be chosen here because splitting a stack of
           corpses could cause the split-off portion to change gender, so
           settle for avoiding "aligned"] */
        if (mndx == PM_ALIGNED_CLERIC && cgend == CORPSTAT_RANDOM)
            mndx = PM_CLERIC;

        return pmname(&mons[mndx], mgend);
    }
    impossible("obj_pmname otyp:%i,corpsenm:%i", obj->otyp, obj->corpsenm);
    return "two-legged glorkum-seeker";
}

/* used by bogusmon(next) and also by init_CapMons(rumors.c);
   bogon_is_pname(below) checks a hard-coded subset of these rather than
   use this list.
   Also used in rumors.c */
const char bogon_codes[] = "-_+|="; /* see dat/bonusmon.txt */

/* fake monsters used to be in a hard-coded array, now in a data file */
char *
bogusmon(char *buf, char *code)
{
    char *mnam = buf;

    if (code)
        *code = '\0';
    /* might fail (return empty buf[]) if the file isn't available */
    get_rnd_text(BOGUSMONFILE, buf, rn2_on_display_rng, MD_PAD_BOGONS);
    if (!*mnam) {
        Strcpy(buf, "bogon");
    } else if (strchr(bogon_codes, *mnam)) { /* strip prefix if present */
        if (code)
            *code = *mnam;
        ++mnam;
    }
    return mnam;
}

/* return a random monster name, for hallucination */
char *
rndmonnam(char *code)
{
    static char buf[BUFSZ];
    char *mnam;
    int name;
#define BOGUSMONSIZE 100 /* arbitrary */

    if (code)
        *code = '\0';

    do {
        name = rn2_on_display_rng(SPECIAL_PM + BOGUSMONSIZE - LOW_PM) + LOW_PM;
    } while (name < SPECIAL_PM
             && (type_is_pname(&mons[name]) || (mons[name].geno & G_NOGEN)));

    if (name >= SPECIAL_PM) {
        mnam = bogusmon(buf, code);
    } else {
        mnam = strcpy(buf, pmname(&mons[name], rn2_on_display_rng(2)));
    }
    return mnam;
#undef BOGUSMONSIZE
}

/* check bogusmon prefix to decide whether it's a personal name */
boolean
bogon_is_pname(char code)
{
    if (!code)
        return FALSE;
    return strchr("-+=", code) ? TRUE : FALSE;
}

/* name of a Rogue player */
const char *
roguename(void)
{
    char *i, *opts;

    if ((opts = nh_getenv("ROGUEOPTS")) != 0) {
        for (i = opts; *i; i++)
            if (!strncmp("name=", i, 5)) {
                char *j;
                if ((j = strchr(i + 5, ',')) != 0)
                    *j = (char) 0;
                return i + 5;
            }
    }
    /*JP:Rogueの開発者の名前*/
    return rn2(3) ? (rn2(2) ? "Michael Toy" : "Kenneth Arnold")
                  : "Glenn Wichman";
}

static NEARDATA const char *const hcolors[] = {
#if 0 /*JP:T*/
    "ultraviolet", "infrared", "bluish-orange", "reddish-green", "dark white",
    "light black", "sky blue-pink", "pinkish-cyan", "indigo-chartreuse",
    "salty", "sweet", "sour", "bitter", "umami", /* basic tastes */
    "striped", "spiral", "swirly", "plaid", "checkered", "argyle", "paisley",
    "blotchy", "guernsey-spotted", "polka-dotted", "square", "round",
    "triangular", "cabernet", "sangria", "fuchsia", "wisteria", "lemon-lime",
    "strawberry-banana", "peppermint", "romantic", "incandescent",
    "octarine", /* Discworld: the Colour of Magic */
    "excitingly dull", "mauve", "electric",
    "neon", "fluorescent", "phosphorescent", "translucent", "opaque",
    "psychedelic", "iridescent", "rainbow-colored", "polychromatic",
    "colorless", "colorless green",
    "dancing", "singing", "loving", "loudy", "noisy", "clattery", "silent",
    "apocyan", "infra-pink", "opalescent", "violant", "tuneless",
    "viridian", "aureolin", "cinnabar", "purpurin", "gamboge", "madder",
    "bistre", "ecru", "fulvous", "tekhelet", "selective yellow",
#else /*JP:TODO:追加分は未訳*/
    "紫外色の", "赤外色の", "青色がかったオレンジ色の", "赤みがかった緑色の", "暗い白色の",
    "明るい黒の", "水色がかったピンク色の", "塩辛い", "甘い", "すっぱい", "苦い",
    "しま模様の", "らせん状の", "波状の", "格子模様状の", "チェック状の", "放射状の", "ペーズリー模様の",
    "しみ状の", "青色の斑点状の", "点状の", "四角形状の", "丸状の",
    "三角状の", "カベルネ色の", "サングリア色の", "鮮やかな赤紫色の", "藤色の", "レモンライム色の",
    "苺バナナ色の", "ペパーミント色の", "ロマンチックな色の", "白熱色の",
    "オクタリン色の", /* Discworld: the Colour of Magic */

#endif
};

const char *
hcolor(const char *colorpref)
{
    return (Hallucination || !colorpref)
        ? hcolors[rn2_on_display_rng(SIZE(hcolors))]
        : colorpref;
}

/* return a random real color unless hallucinating */
const char *
rndcolor(void)
{
    int k = rn2(CLR_MAX);

    return Hallucination ? hcolor((char *) 0)
/*JP
                         : (k == NO_COLOR) ? "colorless"
*/
                         : (k == NO_COLOR) ? "無色の"
                                           : c_obj_colors[k];
}

static NEARDATA const char *const hliquids[] = {
#if 0 /*JP:T*/
    "yoghurt", "oobleck", "clotted blood", "diluted water", "purified water",
    "instant coffee", "tea", "herbal infusion", "liquid rainbow",
    "creamy foam", "mulled wine", "bouillon", "nectar", "grog", "flubber",
    "ketchup", "slow light", "oil", "vinaigrette", "liquid crystal", "honey",
    "caramel sauce", "ink", "aqueous humour", "milk substitute",
    "fruit juice", "glowing lava", "gastric acid", "mineral water",
    "cough syrup", "quicksilver", "sweet vitriol", "grey goo", "pink slime",
    "cosmic latte", "bone oil", "custard", "lard", "vinegar", "creosote",
#else
    "ヨーグルト", "ウーブレック", "血糊", "蒸留水", "精製水",
    "インスタントコーヒー", "紅茶", "ハーブ液", "液体の虹",
    "クリーミーフォーム", "ホットワイン", "ブイヨン", "果汁", "グロッグ", "フラバー",
    "ケチャップ", "低速光", "油", "ビネグレットソース", "液体水晶", "蜂蜜",
    "カラメルソース", "インク", "房水", "代用乳",
    "フルーツジュース", "流れる溶岩", "胃酸", "ミネラルウォーター",
    "咳止めシロップ", "水銀", "ジエチルエーテル", "グレイグー", "ピンクスライム",
    "宇宙ラテ", "骨油", "カスタード", "ラード", "酢", "クレオソート",
#endif
    /* "new coke (tm)", --better not */
};

/* if hallucinating, return a random liquid instead of 'liquidpref' */
const char *
hliquid(
    const char *liquidpref) /* use as-is when not hallucintg (unless empty) */
{
    boolean hallucinate = Hallucination && !program_state.gameover;

    if (hallucinate || !liquidpref || !*liquidpref) {
        int indx, count = SIZE(hliquids);

        /* if we have a non-hallucinatory default value, include it
           among the choices */
        if (liquidpref && *liquidpref)
            ++count;
        indx = rn2_on_display_rng(count);
        if (IndexOk(indx, hliquids))
            return hliquids[indx];
    }
    return liquidpref;
}

/* Aliases for road-runner nemesis
 */
static const char *const coynames[] = {
    "Carnivorous Vulgaris", "Road-Runnerus Digestus", "Eatibus Anythingus",
    "Famishus-Famishus", "Eatibus Almost Anythingus", "Eatius Birdius",
    "Famishius Fantasticus", "Eternalii Famishiis", "Famishus Vulgarus",
    "Famishius Vulgaris Ingeniusi", "Eatius-Slobbius", "Hardheadipus Oedipus",
    "Carnivorous Slobbius", "Hard-Headipus Ravenus", "Evereadii Eatibus",
    "Apetitius Giganticus", "Hungrii Flea-Bagius", "Overconfidentii Vulgaris",
    "Caninus Nervous Rex", "Grotesques Appetitus", "Nemesis Ridiculii",
    "Canis latrans"
};

char *
coyotename(struct monst *mtmp, char *buf)
{
    if (mtmp && buf) {
        Sprintf(buf, "%s - %s",
                x_monnam(mtmp, ARTICLE_NONE, (char *) 0, 0, TRUE),
                mtmp->mcan ? coynames[SIZE(coynames) - 1]
                           : coynames[mtmp->m_id % (SIZE(coynames) - 1)]);
    }
    return buf;
}

char *
rndorcname(char *s)
{
    static const char *const v[] = { "a", "ai", "og", "u" };
    static const char *const snd[] = { "gor", "gris", "un", "bane", "ruk",
                                 "oth","ul", "z", "thos","akh","hai" };
    int i, iend = rn1(2, 3), vstart = rn2(2);

    if (s) {
        *s = '\0';
        for (i = 0; i < iend; ++i) {
            vstart = 1 - vstart;                /* 0 -> 1, 1 -> 0 */
            Sprintf(eos(s), "%s%s", (i > 0 && !rn2(30)) ? "-" : "",
                    vstart ? ROLL_FROM(v) : ROLL_FROM(snd));
        }
    }
    return s;
}

struct monst *
christen_orc(struct monst *mtmp, const char *gang, const char *other)
{
    int sz = 0;
    char buf[BUFSZ], buf2[BUFSZ], *orcname;

    orcname = rndorcname(buf2);
    /* rndorcname() won't return NULL */
    sz = (int) strlen(orcname);
    if (gang)
        sz += (int) (strlen(gang) + sizeof " of " - sizeof "");
    else if (other)
        sz += (int) strlen(other);

    if (sz < BUFSZ) {
        char gbuf[BUFSZ];
        boolean nameit = FALSE;

        if (gang) {
            Sprintf(buf, "%s of %s", upstart(orcname),
                    upstart(strcpy(gbuf, gang)));
            nameit = TRUE;
        } else if (other) {
            Sprintf(buf, "%s%s", upstart(orcname), other);
            nameit = TRUE;
        }
        if (nameit)
            mtmp = christen_monst(mtmp, buf);
    }
    return mtmp;
}

/* Discworld novel titles, in the order that they were published; a subset
   of them have index macros used for variant spellings; if the titles are
   reordered for some reason, make sure that those get renumbered to match */
static const char *const sir_Terry_novels[] = {
    "The Colour of Magic", "The Light Fantastic", "Equal Rites", "Mort",
    "Sourcery", "Wyrd Sisters", "Pyramids", "Guards! Guards!", "Eric",
    "Moving Pictures", "Reaper Man", "Witches Abroad", "Small Gods",
    "Lords and Ladies", "Men at Arms", "Soul Music", "Interesting Times",
    "Maskerade", "Feet of Clay", "Hogfather", "Jingo", "The Last Continent",
    "Carpe Jugulum", "The Fifth Elephant", "The Truth", "Thief of Time",
    "The Last Hero", "The Amazing Maurice and His Educated Rodents",
    "Night Watch", "The Wee Free Men", "Monstrous Regiment",
    "A Hat Full of Sky", "Going Postal", "Thud!", "Wintersmith",
    "Making Money", "Unseen Academicals", "I Shall Wear Midnight", "Snuff",
    "Raising Steam", "The Shepherd's Crown"
};
#define NVL_COLOUR_OF_MAGIC 0
#define NVL_SOURCERY 4
#define NVL_MASKERADE 17
#define NVL_AMAZING_MAURICE 27
#define NVL_THUD 33

const char *
noveltitle(int *novidx)
{
    int j, k = SIZE(sir_Terry_novels);

    j = rn2(k);
    if (novidx) {
        if (*novidx == -1)
            *novidx = j;
        else if (*novidx >= 0 && *novidx < k)
            j = *novidx;
    }
    return sir_Terry_novels[j];
}

/* figure out canonical novel title from player-specified one */
const char *
lookup_novel(const char *lookname, int *idx)
{
    int k;

    /*
     * Accept variant spellings:
     * _The_Colour_of_Magic_ uses British spelling, and American
     * editions keep that, but we also recognize American spelling;
     * _Sourcery_ is a joke rather than British spelling of "sorcery".
     */
    if (!strcmpi(The(lookname), "The Color of Magic"))
        lookname = sir_Terry_novels[NVL_COLOUR_OF_MAGIC];
    else if (!strcmpi(lookname, "Sorcery"))
        lookname = sir_Terry_novels[NVL_SOURCERY];
    else if (!strcmpi(lookname, "Masquerade"))
        lookname = sir_Terry_novels[NVL_MASKERADE];
    else if (!strcmpi(The(lookname), "The Amazing Maurice"))
        lookname = sir_Terry_novels[NVL_AMAZING_MAURICE];
    else if (!strcmpi(lookname, "Thud"))
        lookname = sir_Terry_novels[NVL_THUD];

    for (k = 0; k < SIZE(sir_Terry_novels); ++k) {
        if (!strcmpi(lookname, sir_Terry_novels[k])
            || !strcmpi(The(lookname), sir_Terry_novels[k])) {
            if (idx)
                *idx = k;
            return sir_Terry_novels[k];
        }
    }
    /* name not found; if novelidx is already set, override the name */
    if (idx && IndexOk(*idx, sir_Terry_novels))
        return sir_Terry_novels[*idx];

    return (const char *) 0;
}

/*do_name.c*/
