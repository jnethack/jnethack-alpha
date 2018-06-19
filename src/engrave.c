/* NetHack 3.6	engrave.c	$NHDT-Date: 1456304550 2016/02/24 09:02:30 $  $NHDT-Branch: NetHack-3.6.0 $:$NHDT-Revision: 1.61 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2016            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"

STATIC_VAR NEARDATA struct engr *head_engr;

char *
random_engraving(outbuf)
char *outbuf;
{
    const char *rumor;

    /* a random engraving may come from the "rumors" file,
       or from the "engrave" file (formerly in an array here) */
    if (!rn2(4) || !(rumor = getrumor(0, outbuf, TRUE)) || !*rumor)
        (void) get_rnd_text(ENGRAVEFILE, outbuf);

    wipeout_text(outbuf, (int) (strlen(outbuf) / 4), 0);
    return outbuf;
}

/* Partial rubouts for engraving characters. -3. */
static const struct {
    char wipefrom;
    const char *wipeto;
} rubouts[] = { { 'A', "^" },
                { 'B', "Pb[" },
                { 'C', "(" },
                { 'D', "|)[" },
                { 'E', "|FL[_" },
                { 'F', "|-" },
                { 'G', "C(" },
                { 'H', "|-" },
                { 'I', "|" },
                { 'K', "|<" },
                { 'L', "|_" },
                { 'M', "|" },
                { 'N', "|\\" },
                { 'O', "C(" },
                { 'P', "F" },
                { 'Q', "C(" },
                { 'R', "PF" },
                { 'T', "|" },
                { 'U', "J" },
                { 'V', "/\\" },
                { 'W', "V/\\" },
                { 'Z', "/" },
                { 'b', "|" },
                { 'd', "c|" },
                { 'e', "c" },
                { 'g', "c" },
                { 'h', "n" },
                { 'j', "i" },
                { 'k', "|" },
                { 'l', "|" },
                { 'm', "nr" },
                { 'n', "r" },
                { 'o', "c" },
                { 'q', "c" },
                { 'w', "v" },
                { 'y', "v" },
                { ':', "." },
                { ';', ",:" },
                { ',', "." },
                { '=', "-" },
                { '+', "-|" },
                { '*', "+" },
                { '@', "0" },
                { '0', "C(" },
                { '1', "|" },
                { '6', "o" },
                { '7', "/" },
                { '8', "3o" } };

/* degrade some of the characters in a string */
void
wipeout_text(engr, cnt, seed)
char *engr;
int cnt;
unsigned seed; /* for semi-controlled randomization */
{
#if 0 /*JP*/
    char *s;
#else
    unsigned char *s;
#endif
    int i, j, nxt, use_rubout, lth = (int) strlen(engr);

    if (lth && cnt > 0) {
        while (cnt--) {
            /* pick next character */
            if (!seed) {
                /* random */
                nxt = rn2(lth);
                use_rubout = rn2(4);
            } else {
                /* predictable; caller can reproduce the same sequence by
                   supplying the same arguments later, or a pseudo-random
                   sequence by varying any of them */
                nxt = seed % lth;
                seed *= 31, seed %= (BUFSZ - 1);
                use_rubout = seed & 3;
            }
#if 0 /*JP*/
            s = &engr[nxt];
#else /*JP: 日本語の場合はjrubout()を使って消す*/
            if (!seed)
                j = rn2(2);
            else {
                seed *= 31,  seed %= (BUFSZ-1);
                j = seed % 2;
            }

            if(jrubout(engr, nxt, use_rubout, j)){
                continue;
            }

            s = (unsigned char *)&engr[nxt];
#endif
            if (*s == ' ')
                continue;

            /* rub out unreadable & small punctuation marks */
            if (index("?.,'`-|_", *s)) {
                *s = ' ';
                continue;
            }

            if (!use_rubout)
                i = SIZE(rubouts);
            else
                for (i = 0; i < SIZE(rubouts); i++)
                    if (*s == rubouts[i].wipefrom) {
                        /*
                         * Pick one of the substitutes at random.
                         */
                        if (!seed)
                            j = rn2(strlen(rubouts[i].wipeto));
                        else {
                            seed *= 31, seed %= (BUFSZ - 1);
                            j = seed % (strlen(rubouts[i].wipeto));
                        }
                        *s = rubouts[i].wipeto[j];
                        break;
                    }

            /* didn't pick rubout; use '?' for unreadable character */
            if (i == SIZE(rubouts))
                *s = '?';
        }
    }

    /* trim trailing spaces */
    while (lth && engr[lth - 1] == ' ')
        engr[--lth] = '\0';
}

/* check whether hero can reach something at ground level */
boolean
can_reach_floor(check_pit)
boolean check_pit;
{
    struct trap *t;

    if (u.uswallow)
        return FALSE;
    /* Restricted/unskilled riders can't reach the floor */
    if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
        return FALSE;
    if (check_pit && !Flying
        && (t = t_at(u.ux, u.uy)) != 0 && uteetering_at_seen_pit(t))
        return FALSE;

    return (boolean) ((!Levitation || Is_airlevel(&u.uz)
                       || Is_waterlevel(&u.uz))
                      && (!u.uundetected || !is_hider(youmonst.data)
                          || u.umonnum == PM_TRAPPER));
}

/* give a message after caller has determined that hero can't reach */
void
cant_reach_floor(x, y, up, check_pit)
int x, y;
boolean up, check_pit;
{
#if 0 /*JP*/
    You("can't reach the %s.",
        up ? ceiling(x, y)
           : (check_pit && can_reach_floor(FALSE))
               ? "bottom of the pit"
               : surface(x, y));
#else
    You("%sに届かない．",
        up ? ceiling(x, y)
           : (check_pit && can_reach_floor(FALSE))
               ? "落し穴の底"
               : surface(x, y));
#endif
}

const char *
surface(x, y)
register int x, y;
{
    register struct rm *lev = &levl[x][y];

    if (x == u.ux && y == u.uy && u.uswallow && is_animal(u.ustuck->data))
/*JP
        return "maw";
*/
        return "胃袋";
    else if (IS_AIR(lev->typ) && Is_airlevel(&u.uz))
/*JP
        return "air";
*/
        return "空中";
    else if (is_pool(x, y))
#if 0 /*JP*/
        return (Underwater && !Is_waterlevel(&u.uz))
            ? "bottom" : hliquid("water");
#else
        return (Underwater && !Is_waterlevel(&u.uz))
            ? "水の底" : hliquid("水中");
#endif
    else if (is_ice(x, y))
/*JP
        return "ice";
*/
        return "氷";
    else if (is_lava(x, y))
/*JP
        return hliquid("lava");
*/
        return hliquid("溶岩");
    else if (lev->typ == DRAWBRIDGE_DOWN)
/*JP
        return "bridge";
*/
        return "橋";
    else if (IS_ALTAR(levl[x][y].typ))
/*JP
        return "altar";
*/
        return "祭壇";
    else if (IS_GRAVE(levl[x][y].typ))
/*JP
        return "headstone";
*/
        return "墓石";
    else if (IS_FOUNTAIN(levl[x][y].typ))
/*JP
        return "fountain";
*/
        return "泉";
    else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz))
             || IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
/*JP
        return "floor";
*/
        return "床";
    else
/*JP
        return "ground";
*/
        return "地面";
}

const char *
ceiling(x, y)
register int x, y;
{
    register struct rm *lev = &levl[x][y];
    const char *what;

    /* other room types will no longer exist when we're interested --
     * see check_special_room()
     */
    if (*in_rooms(x, y, VAULT))
/*JP
        what = "vault's ceiling";
*/
        what = "倉庫の天井";
    else if (*in_rooms(x, y, TEMPLE))
/*JP
        what = "temple's ceiling";
*/
        what = "寺院の天井";
    else if (*in_rooms(x, y, SHOPBASE))
/*JP
        what = "shop's ceiling";
*/
        what = "店の天井";
    else if (Is_waterlevel(&u.uz))
        /* water plane has no surface; its air bubbles aren't below sky */
/*JP
        what = "water above";
*/
        what = "水の上方";
    else if (IS_AIR(lev->typ))
/*JP
        what = "sky";
*/
        what = "空";
    else if (Underwater)
/*JP
        what = "water's surface";
*/
        what = "水面";
    else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz))
             || IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
/*JP
        what = "ceiling";
*/
        what = "天井";
    else
/*JP
        what = "rock cavern";
*/
        what = "洞窟の天井";

    return what;
}

struct engr *
engr_at(x, y)
xchar x, y;
{
    register struct engr *ep = head_engr;

    while (ep) {
        if (x == ep->engr_x && y == ep->engr_y)
            return ep;
        ep = ep->nxt_engr;
    }
    return (struct engr *) 0;
}

/* Decide whether a particular string is engraved at a specified
 * location; a case-insensitive substring match is used.
 * Ignore headstones, in case the player names herself "Elbereth".
 *
 * If strict checking is requested, the word is only considered to be
 * present if it is intact and is the entire content of the engraving.
 */
int
sengr_at(s, x, y, strict)
const char *s;
xchar x, y;
boolean strict;
{
    register struct engr *ep = engr_at(x, y);

    if (ep && ep->engr_type != HEADSTONE && ep->engr_time <= moves) {
        return strict ? (fuzzymatch(ep->engr_txt, s, "", TRUE))
                      : (strstri(ep->engr_txt, s) != 0);
    }

    return FALSE;
}

void
u_wipe_engr(cnt)
int cnt;
{
    if (can_reach_floor(TRUE))
        wipe_engr_at(u.ux, u.uy, cnt, FALSE);
}

void
wipe_engr_at(x, y, cnt, magical)
xchar x, y, cnt, magical;
{
    register struct engr *ep = engr_at(x, y);

    /* Headstones are indelible */
    if (ep && ep->engr_type != HEADSTONE) {
        debugpline1("asked to erode %d characters", cnt);
        if (ep->engr_type != BURN || is_ice(x, y) || (magical && !rn2(2))) {
            if (ep->engr_type != DUST && ep->engr_type != ENGR_BLOOD) {
                cnt = rn2(1 + 50 / (cnt + 1)) ? 0 : 1;
                debugpline1("actually eroding %d characters", cnt);
            }
            wipeout_text(ep->engr_txt, (int) cnt, 0);
            while (ep->engr_txt[0] == ' ')
                ep->engr_txt++;
            if (!ep->engr_txt[0])
                del_engr(ep);
        }
    }
}

void
read_engr_at(x, y)
int x, y;
{
    register struct engr *ep = engr_at(x, y);
    int sensed = 0;
    char buf[BUFSZ];

    /* Sensing an engraving does not require sight,
     * nor does it necessarily imply comprehension (literacy).
     */
    if (ep && ep->engr_txt[0]) {
        switch (ep->engr_type) {
        case DUST:
            if (!Blind) {
                sensed = 1;
#if 0 /*JP*/
                pline("%s is written here in the %s.", Something,
                      is_ice(x, y) ? "frost" : "dust");
#else
                pline("何かの文字が%sに書いてある．",
                      is_ice(x, y) ? "霜" : "ほこり");
#endif
            }
            break;
        case ENGRAVE:
        case HEADSTONE:
            if (!Blind || can_reach_floor(TRUE)) {
                sensed = 1;
/*JP
                pline("%s is engraved here on the %s.", Something,
*/
                pline("何かの文字が%sに刻まれている．",
                      surface(x, y));
            }
            break;
        case BURN:
            if (!Blind || can_reach_floor(TRUE)) {
                sensed = 1;
#if 0 /*JP*/
                pline("Some text has been %s into the %s here.",
                      is_ice(x, y) ? "melted" : "burned", surface(x, y));
#else
                pline("何かの文字が%s%sいる．",
                      surface(x,y),
                      is_ice(x,y) ? "に刻まれて" : "に焼き付けられて");
#endif
            }
            break;
        case MARK:
            if (!Blind) {
                sensed = 1;
/*JP
                pline("There's some graffiti on the %s here.", surface(x, y));
*/
                pline("%sに落書がある．", surface(x,y));
            }
            break;
        case ENGR_BLOOD:
            /* "It's a message!  Scrawled in blood!"
             * "What's it say?"
             * "It says... `See you next Wednesday.'" -- Thriller
             */
            if (!Blind) {
                sensed = 1;
/*JP
                You_see("a message scrawled in blood here.");
*/
                You("血文字がなぐり書きされているのを見つけた．");
            }
            break;
        default:
            impossible("%s is written in a very strange way.", Something);
            sensed = 1;
        }
        if (sensed) {
            char *et;
/*JP
            unsigned maxelen = BUFSZ - sizeof("You feel the words: \"\". ");
*/
            unsigned maxelen = BUFSZ - sizeof("あなたは次のように感じた：「」");
            if (strlen(ep->engr_txt) > maxelen) {
                (void) strncpy(buf, ep->engr_txt, (int) maxelen);
                buf[maxelen] = '\0';
                et = buf;
            } else
                et = ep->engr_txt;
/*JP
            You("%s: \"%s\".", (Blind) ? "feel the words" : "read", et);
*/
            You("%s：「%s」", (Blind) ? "次のように感じた" : "読んだ",  et);
            if (context.run > 1)
                nomul(0);
        }
    }
}

void
make_engr_at(x, y, s, e_time, e_type)
int x, y;
const char *s;
long e_time;
xchar e_type;
{
    struct engr *ep;
    unsigned smem = strlen(s) + 1;

    if ((ep = engr_at(x, y)) != 0)
        del_engr(ep);
    ep = newengr(smem);
    (void) memset((genericptr_t)ep, 0, smem + sizeof(struct engr));
    ep->nxt_engr = head_engr;
    head_engr = ep;
    ep->engr_x = x;
    ep->engr_y = y;
    ep->engr_txt = (char *) (ep + 1);
    Strcpy(ep->engr_txt, s);
    /* engraving Elbereth shows wisdom */
    if (!in_mklev && !strcmp(s, "Elbereth"))
        exercise(A_WIS, TRUE);
    ep->engr_time = e_time;
    ep->engr_type = e_type > 0 ? e_type : rnd(N_ENGRAVE - 1);
    ep->engr_lth = smem;
}

/* delete any engraving at location <x,y> */
void
del_engr_at(x, y)
int x, y;
{
    register struct engr *ep = engr_at(x, y);

    if (ep)
        del_engr(ep);
}

/*
 * freehand - returns true if player has a free hand
 */
int
freehand()
{
    return (!uwep || !welded(uwep)
            || (!bimanual(uwep) && (!uarms || !uarms->cursed)));
}

static NEARDATA const char styluses[] = { ALL_CLASSES, ALLOW_NONE,
                                          TOOL_CLASS,  WEAPON_CLASS,
                                          WAND_CLASS,  GEM_CLASS,
                                          RING_CLASS,  0 };

/* Mohs' Hardness Scale:
 *  1 - Talc             6 - Orthoclase
 *  2 - Gypsum           7 - Quartz
 *  3 - Calcite          8 - Topaz
 *  4 - Fluorite         9 - Corundum
 *  5 - Apatite         10 - Diamond
 *
 * Since granite is an igneous rock hardness ~ 7, anything >= 8 should
 * probably be able to scratch the rock.
 * Devaluation of less hard gems is not easily possible because obj struct
 * does not contain individual oc_cost currently. 7/91
 *
 * steel      - 5-8.5   (usu. weapon)
 * diamond    - 10                      * jade       -  5-6      (nephrite)
 * ruby       -  9      (corundum)      * turquoise  -  5-6
 * sapphire   -  9      (corundum)      * opal       -  5-6
 * topaz      -  8                      * glass      - ~5.5
 * emerald    -  7.5-8  (beryl)         * dilithium  -  4-5??
 * aquamarine -  7.5-8  (beryl)         * iron       -  4-5
 * garnet     -  7.25   (var. 6.5-8)    * fluorite   -  4
 * agate      -  7      (quartz)        * brass      -  3-4
 * amethyst   -  7      (quartz)        * gold       -  2.5-3
 * jasper     -  7      (quartz)        * silver     -  2.5-3
 * onyx       -  7      (quartz)        * copper     -  2.5-3
 * moonstone  -  6      (orthoclase)    * amber      -  2-2.5
 */

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
doengrave()
{
    boolean dengr = FALSE;    /* TRUE if we wipe out the current engraving */
    boolean doblind = FALSE;  /* TRUE if engraving blinds the player */
    boolean doknown = FALSE;  /* TRUE if we identify the stylus */
    boolean eow = FALSE;      /* TRUE if we are overwriting oep */
    boolean jello = FALSE;    /* TRUE if we are engraving in slime */
    boolean ptext = TRUE;     /* TRUE if we must prompt for engrave text */
    boolean teleengr = FALSE; /* TRUE if we move the old engraving */
    boolean zapwand = FALSE;  /* TRUE if we remove a wand charge */
    xchar type = DUST;        /* Type of engraving made */
    char buf[BUFSZ];          /* Buffer for final/poly engraving text */
    char ebuf[BUFSZ];         /* Buffer for initial engraving text */
    char fbuf[BUFSZ];         /* Buffer for "your fingers" */
    char qbuf[QBUFSZ];        /* Buffer for query text */
    char post_engr_text[BUFSZ]; /* Text displayed after engraving prompt */
    const char *everb;          /* Present tense of engraving type */
    const char *eloc; /* Where the engraving is (ie dust/floor/...) */
    char *sp;         /* Place holder for space count of engr text */
    int len;          /* # of nonspace chars of new engraving text */
    int maxelen;      /* Max allowable length of engraving text */
    struct engr *oep = engr_at(u.ux, u.uy);
    /* The current engraving */
    struct obj *otmp; /* Object selected with which to engrave */
    char *writer;

    multi = 0;              /* moves consumed */
    nomovemsg = (char *) 0; /* occupation end message */

    buf[0] = (char) 0;
    ebuf[0] = (char) 0;
    post_engr_text[0] = (char) 0;
    maxelen = BUFSZ - 1;
    if (is_demon(youmonst.data) || youmonst.data->mlet == S_VAMPIRE)
        type = ENGR_BLOOD;

    /* Can the adventurer engrave at all? */

    if (u.uswallow) {
        if (is_animal(u.ustuck->data)) {
/*JP
            pline("What would you write?  \"Jonah was here\"?");
*/
            pline("何を書くんだい？「ヨナはここにいる」？");
            return 0;
        } else if (is_whirly(u.ustuck->data)) {
            cant_reach_floor(u.ux, u.uy, FALSE, FALSE);
            return 0;
        } else
            jello = TRUE;
    } else if (is_lava(u.ux, u.uy)) {
/*JP
        You_cant("write on the %s!", surface(u.ux, u.uy));
*/
        You("%sに届かない．", surface(u.ux,u.uy));
        return 0;
    } else if (is_pool(u.ux, u.uy) || IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
/*JP
        You_cant("write on the %s!", surface(u.ux, u.uy));
*/
        You("%sには書けない！", surface(u.ux, u.uy));
        return 0;
    }
    if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) /* in bubble */) {
/*JP
        You_cant("write in thin air!");
*/
        You("空中には書けない！");
        return 0;
    } else if (!accessible(u.ux, u.uy)) {
        /* stone, tree, wall, secret corridor, pool, lava, bars */
/*JP
        You_cant("write here.");
*/
        You_cant("ここには書けない．");
        return 0;
    }
    if (cantwield(youmonst.data)) {
/*JP
        You_cant("even hold anything!");
*/
        You("何かを持つことすらできない！");
        return 0;
    }
    if (check_capacity((char *) 0))
        return 0;

    /* One may write with finger, or weapon, or wand, or..., or...
     * Edited by GAN 10/20/86 so as not to change weapon wielded.
     */

    otmp = getobj(styluses, "write with");
    if (!otmp) /* otmp == zeroobj if fingers */
        return 0;

    if (otmp == &zeroobj) {
/*JP
        Strcat(strcpy(fbuf, "your "), body_part(FINGERTIP));
*/
        Strcat(strcpy(fbuf, "あなたの"), body_part(FINGERTIP));
        writer = fbuf;
    } else
        writer = yname(otmp);

    /* There's no reason you should be able to write with a wand
     * while both your hands are tied up.
     */
    if (!freehand() && otmp != uwep && !otmp->owornmask) {
/*JP
        You("have no free %s to write with!", body_part(HAND));
*/
        pline("%sの自由が効かないので書けない！", body_part(HAND));
        return 0;
    }

    if (jello) {
/*JP
        You("tickle %s with %s.", mon_nam(u.ustuck), writer);
*/
                You("%sで%sをくすぐった．", writer, mon_nam(u.ustuck));
/*JP
        Your("message dissolves...");
*/
                Your("メッセージは消えた．．．");
        return 0;
    }
    if (otmp->oclass != WAND_CLASS && !can_reach_floor(TRUE)) {
        cant_reach_floor(u.ux, u.uy, FALSE, TRUE);
        return 0;
    }
    if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
/*JP
        You("make a motion towards the altar with %s.", writer);
*/
        You("%sを使って祭壇に書こうとした．", writer);
        altar_wrath(u.ux, u.uy);
        return 0;
    }
    if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
        if (otmp == &zeroobj) { /* using only finger */
/*JP
            You("would only make a small smudge on the %s.",
*/
            You("%sを少しよごすことしかできなかった．",
                surface(u.ux, u.uy));
            return 0;
        } else if (!levl[u.ux][u.uy].disturbed) {
/*JP
            You("disturb the undead!");
*/
            You("不死の者の眠りを妨げた！");
            levl[u.ux][u.uy].disturbed = 1;
            (void) makemon(&mons[PM_GHOUL], u.ux, u.uy, NO_MM_FLAGS);
            exercise(A_WIS, FALSE);
            return 1;
        }
    }

    /* SPFX for items */

    switch (otmp->oclass) {
    default:
    case AMULET_CLASS:
    case CHAIN_CLASS:
    case POTION_CLASS:
    case COIN_CLASS:
        break;
    case RING_CLASS:
        /* "diamond" rings and others should work */
    case GEM_CLASS:
        /* diamonds & other hard gems should work */
        if (objects[otmp->otyp].oc_tough) {
            type = ENGRAVE;
            break;
        }
        break;
    case ARMOR_CLASS:
        if (is_boots(otmp)) {
            type = DUST;
            break;
        }
        /*FALLTHRU*/
    /* Objects too large to engrave with */
    case BALL_CLASS:
    case ROCK_CLASS:
/*JP
        You_cant("engrave with such a large object!");
*/
        pline("そんな大きなものを使って文字を刻めない！");
        ptext = FALSE;
        break;
    /* Objects too silly to engrave with */
    case FOOD_CLASS:
    case SCROLL_CLASS:
    case SPBOOK_CLASS:
#if 0 /*JP*/
        pline("%s would get %s.", Yname2(otmp),
              is_ice(u.ux, u.uy) ? "all frosty" : "too dirty");
#else
        Your("%sは%sなった．", xname(otmp),
             is_ice(u.ux,u.uy) ? "霜だらけに" : "汚なく");
#endif
        ptext = FALSE;
        break;
    case RANDOM_CLASS: /* This should mean fingers */
        break;

    /* The charge is removed from the wand before prompting for
     * the engraving text, because all kinds of setup decisions
     * and pre-engraving messages are based upon knowing what type
     * of engraving the wand is going to do.  Also, the player
     * will have potentially seen "You wrest .." message, and
     * therefore will know they are using a charge.
     */
    case WAND_CLASS:
        if (zappable(otmp)) {
            check_unpaid(otmp);
            if (otmp->cursed && !rn2(WAND_BACKFIRE_CHANCE)) {
                wand_explode(otmp, 0);
                return 1;
            }
            zapwand = TRUE;
            if (!can_reach_floor(TRUE))
                ptext = FALSE;

            switch (otmp->otyp) {
            /* DUST wands */
            default:
                break;
            /* NODIR wands */
            case WAN_LIGHT:
            case WAN_SECRET_DOOR_DETECTION:
            case WAN_CREATE_MONSTER:
            case WAN_WISHING:
            case WAN_ENLIGHTENMENT:
                zapnodir(otmp);
                break;
            /* IMMEDIATE wands */
            /* If wand is "IMMEDIATE", remember to affect the
             * previous engraving even if turning to dust.
             */
            case WAN_STRIKING:
                Strcpy(post_engr_text,
/*JP
                    "The wand unsuccessfully fights your attempt to write!");
*/
                       "あなたが書こうとすると杖は抵抗した！");
                break;
            case WAN_SLOW_MONSTER:
                if (!Blind) {
/*JP
                    Sprintf(post_engr_text, "The bugs on the %s slow down!",
*/
                    Sprintf(post_engr_text, "%sの上の虫の動きが遅くなった！",
                            surface(u.ux, u.uy));
                }
                break;
            case WAN_SPEED_MONSTER:
                if (!Blind) {
/*JP
                    Sprintf(post_engr_text, "The bugs on the %s speed up!",
*/
                    Sprintf(post_engr_text, "%sの上の虫の動きが速くなった！",
                            surface(u.ux, u.uy));
                }
                break;
            case WAN_POLYMORPH:
                if (oep) {
                    if (!Blind) {
                        type = (xchar) 0; /* random */
                        (void) random_engraving(buf);
                    }
                    dengr = TRUE;
                }
                break;
            case WAN_NOTHING:
            case WAN_UNDEAD_TURNING:
            case WAN_OPENING:
            case WAN_LOCKING:
            case WAN_PROBING:
                break;
            /* RAY wands */
            case WAN_MAGIC_MISSILE:
                ptext = TRUE;
                if (!Blind) {
                    Sprintf(post_engr_text,
/*JP
                            "The %s is riddled by bullet holes!",
*/
                            "%sは散弾で細かい穴だらけになった！",
                            surface(u.ux, u.uy));
                }
                break;
            /* can't tell sleep from death - Eric Backus */
            case WAN_SLEEP:
            case WAN_DEATH:
                if (!Blind) {
/*JP
                    Sprintf(post_engr_text, "The bugs on the %s stop moving!",
*/
                    Sprintf(post_engr_text, "%sの上の虫の動きが止まった！",
                            surface(u.ux, u.uy));
                }
                break;
            case WAN_COLD:
                if (!Blind)
                    Strcpy(post_engr_text,
/*JP
                           "A few ice cubes drop from the wand.");
*/
                           "氷のかけらが杖からこぼれ落ちた．");
                if (!oep || (oep->engr_type != BURN))
                    break;
                /*FALLTHRU*/
            case WAN_CANCELLATION:
            case WAN_MAKE_INVISIBLE:
                if (oep && oep->engr_type != HEADSTONE) {
                    if (!Blind)
/*JP
                        pline_The("engraving on the %s vanishes!",
*/
                        pline("%sの上の文字は消えた！",
                                  surface(u.ux, u.uy));
                    dengr = TRUE;
                }
                break;
            case WAN_TELEPORTATION:
                if (oep && oep->engr_type != HEADSTONE) {
                    if (!Blind)
/*JP
                        pline_The("engraving on the %s vanishes!",
*/
                        pline("%sの上の文字は消えた！",
                                  surface(u.ux, u.uy));
                    teleengr = TRUE;
                }
                break;
            /* type = ENGRAVE wands */
            case WAN_DIGGING:
                ptext = TRUE;
                type = ENGRAVE;
                if (!objects[otmp->otyp].oc_name_known) {
                    if (flags.verbose)
/*JP
                        pline("This %s is a wand of digging!", xname(otmp));
*/
                        pline("これは穴掘りの杖だ！");
                    doknown = TRUE;
                }
#if 0 /*JP*/
                Strcpy(post_engr_text,
                       (Blind && !Deaf)
                          ? "You hear drilling!"
                          : Blind
                             ? "You feel tremors."
                             : IS_GRAVE(levl[u.ux][u.uy].typ)
                                 ? "Chips fly out from the headstone."
                                 : is_ice(u.ux, u.uy)
                                    ? "Ice chips fly up from the ice surface!"
                                    : (level.locations[u.ux][u.uy].typ
                                       == DRAWBRIDGE_DOWN)
                                       ? "Splinters fly up from the bridge."
                                       : "Gravel flies up from the floor.");
#else
                Strcpy(post_engr_text,
                       Blind
                          ? "穴が開く音を聞いた！"
                          : IS_GRAVE(levl[u.ux][u.uy].typ)
                             ? "墓石から破片が飛び散った．"
                             : is_ice(u.ux,u.uy)
                                ? "氷の表面から氷のかけらが飛び散った．"
                                : (level.locations[u.ux][u.uy].typ
                                   == DRAWBRIDGE_DOWN)
                                   ? "破片が橋から舞いあがった．"
                                   : "砂利が床から飛び散った．");
#endif
                break;
            /* type = BURN wands */
            case WAN_FIRE:
                ptext = TRUE;
                type = BURN;
                if (!objects[otmp->otyp].oc_name_known) {
                    if (flags.verbose)
/*JP
                        pline("This %s is a wand of fire!", xname(otmp));
*/
                        pline("これは炎の杖だ！");
                    doknown = TRUE;
                }
#if 0 /*JP*/
                Strcpy(post_engr_text, Blind ? "You feel the wand heat up."
                                             : "Flames fly from the wand.");
#else
                Strcpy(post_engr_text, Blind ? "杖が暖かくなったような気がした．"
                                             : "炎が杖から飛び散った．");
#endif
                break;
            case WAN_LIGHTNING:
                ptext = TRUE;
                type = BURN;
                if (!objects[otmp->otyp].oc_name_known) {
                    if (flags.verbose)
/*JP
                        pline("This %s is a wand of lightning!", xname(otmp));
*/
                        pline("これは雷の杖だ！");
                    doknown = TRUE;
                }
                if (!Blind) {
/*JP
                    Strcpy(post_engr_text, "Lightning arcs from the wand.");
*/
                    Strcpy(post_engr_text, "火花が杖から飛び散った．");
                    doblind = TRUE;
                } else
#if 0 /*JP*/
                    Strcpy(post_engr_text, !Deaf
                                ? "You hear crackling!"
                                : "Your hair stands up!");
#else
                    Strcpy(post_engr_text, !Deaf
                                ? "パチパチという音を聞いた！"
                                : "ゾッとした！");
#endif
                break;

            /* type = MARK wands */
            /* type = ENGR_BLOOD wands */
            }
        } else { /* end if zappable */
            /* failing to wrest one last charge takes time */
            ptext = FALSE; /* use "early exit" below, return 1 */
            /* give feedback here if we won't be getting the
               "can't reach floor" message below */
            if (can_reach_floor(TRUE)) {
                /* cancelled wand turns to dust */
                if (otmp->spe < 0)
                    zapwand = TRUE;
                /* empty wand just doesn't write */
                else
/*JP
                    pline_The("wand is too worn out to engrave.");
*/
                    pline_The("杖は文字を刻むには使いすぎている．");
            }
        }
        break;

    case WEAPON_CLASS:
        if (is_blade(otmp)) {
            if ((int) otmp->spe > -3)
                type = ENGRAVE;
            else
/*JP
                pline("%s too dull for engraving.", Yobjnam2(otmp, "are"));
*/
                pline("%sは刃がボロボロで，文字を彫れない．", xname(otmp));
        }
        break;

    case TOOL_CLASS:
        if (otmp == ublindf) {
            pline(
/*JP
                "That is a bit difficult to engrave with, don't you think?");
*/
                "ちょっとそれで彫るのは大変だろう，そう思わない？");
            return 0;
        }
        switch (otmp->otyp) {
        case MAGIC_MARKER:
            if (otmp->spe <= 0)
/*JP
                Your("marker has dried out.");
*/
                Your("マーカは乾ききった．");
            else
                type = MARK;
            break;
        case TOWEL:
            /* Can't really engrave with a towel */
            ptext = FALSE;
            if (oep)
                if (oep->engr_type == DUST
                    || oep->engr_type == ENGR_BLOOD
                    || oep->engr_type == MARK) {
                    if (is_wet_towel(otmp))
                        dry_a_towel(otmp, -1, TRUE);
                    if (!Blind)
/*JP
                        You("wipe out the message here.");
*/
                        You("メッセージを拭きとった．");
                    else
#if 0 /*JP*/
                        pline("%s %s.", Yobjnam2(otmp, "get"),
                              is_ice(u.ux, u.uy) ? "frosty" : "dusty");
#else
                        pline("%sは%sになった．", xname(otmp),
                              is_ice(u.ux,u.uy) ? "霜だらけ" : "ほこりまみれ");
#endif
                    dengr = TRUE;
                } else
/*JP
                    pline("%s can't wipe out this engraving.", Yname2(otmp));
*/
                    pline("この文字は%sでは拭きとれない．", xname(otmp));
            else
#if 0 /*JP*/
                pline("%s %s.", Yobjnam2(otmp, "get"),
                      is_ice(u.ux, u.uy) ? "frosty" : "dusty");
#else
                pline("%sは%sになった．", xname(otmp),
                      is_ice(u.ux,u.uy) ? "霜だらけ" : "ほこりまみれ");
#endif
            break;
        default:
            break;
        }
        break;

    case VENOM_CLASS:
        if (wizard) {
/*JP
            pline("Writing a poison pen letter??");
*/
            pline("ふむ．これこそ本当の毒舌だ．");
            break;
        }
        /*FALLTHRU*/
    case ILLOBJ_CLASS:
        impossible("You're engraving with an illegal object!");
        break;
    }

    if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
        if (type == ENGRAVE || type == 0) {
            type = HEADSTONE;
        } else {
            /* ensures the "cannot wipe out" case */
            type = DUST;
            dengr = FALSE;
            teleengr = FALSE;
            buf[0] = '\0';
        }
    }

    /*
     * End of implement setup
     */

    /* Identify stylus */
    if (doknown) {
        learnwand(otmp);
        if (objects[otmp->otyp].oc_name_known)
            more_experienced(0, 10);
    }
    if (teleengr) {
        rloc_engr(oep);
        oep = (struct engr *) 0;
    }
    if (dengr) {
        del_engr(oep);
        oep = (struct engr *) 0;
    }
    /* Something has changed the engraving here */
    if (*buf) {
        make_engr_at(u.ux, u.uy, buf, moves, type);
/*JP
        pline_The("engraving now reads: \"%s\".", buf);
*/
        pline("刻まれた文字を読んだ：「%s」．", buf);
        ptext = FALSE;
    }
    if (zapwand && (otmp->spe < 0)) {
#if 0 /*JP*/
        pline("%s %sturns to dust.", The(xname(otmp)),
              Blind ? "" : "glows violently, then ");
#else
        pline("%sは%sちりとなった．", xname(otmp),
              Blind ? "" : "激しく輝き，");
#endif
        if (!IS_GRAVE(levl[u.ux][u.uy].typ))
#if 0 /*JP*/
            You(
    "are not going to get anywhere trying to write in the %s with your dust.",
                is_ice(u.ux, u.uy) ? "frost" : "dust");
#else
            You(
                "塵で%sに何か書こうとしたが，できなかった．",
                is_ice(u.ux,u.uy) ? "氷" : "ほこり");
#endif
        useup(otmp);
        otmp = 0; /* wand is now gone */
        ptext = FALSE;
    }
    /* Early exit for some implements. */
    if (!ptext) {
        if (otmp && otmp->oclass == WAND_CLASS && !can_reach_floor(TRUE))
            cant_reach_floor(u.ux, u.uy, FALSE, TRUE);
        return 1;
    }
    /*
     * Special effects should have deleted the current engraving (if
     * possible) by now.
     */
    if (oep) {
        register char c = 'n';

        /* Give player the choice to add to engraving. */
        if (type == HEADSTONE) {
            /* no choice, only append */
            c = 'y';
        } else if (type == oep->engr_type
                   && (!Blind || oep->engr_type == BURN
                       || oep->engr_type == ENGRAVE)) {
/*JP
            c = yn_function("Do you want to add to the current engraving?",
*/
            c = yn_function("何か書き加えますか？",
                            ynqchars, 'y');
            if (c == 'q') {
                pline1(Never_mind);
                return 0;
            }
        }

        if (c == 'n' || Blind) {
            if (oep->engr_type == DUST
                || oep->engr_type == ENGR_BLOOD
                || oep->engr_type == MARK) {
                if (!Blind) {
#if 0 /*JP*/
                    You("wipe out the message that was %s here.",
                        (oep->engr_type == DUST)
                            ? "written in the dust"
                            : (oep->engr_type == ENGR_BLOOD)
                                ? "scrawled in blood"
                                : "written");
#else
                    You("%sメッセージを拭きとった．",
                        (oep->engr_type == DUST)
                            ? "ほこりに書かれている"
                            : (oep->engr_type == BLOOD)
                                ? "血文字でなぐり書きされている"
                                : "書かれている");
#endif
                    del_engr(oep);
                    oep = (struct engr *) 0;
                } else
                    /* Don't delete engr until after we *know* we're engraving
                     */
                    eow = TRUE;
            } else if (type == DUST || type == MARK || type == ENGR_BLOOD) {
#if 0 /*JP*/
                You("cannot wipe out the message that is %s the %s here.",
                    oep->engr_type == BURN
                        ? (is_ice(u.ux, u.uy) ? "melted into" : "burned into")
                        : "engraved in",
                    surface(u.ux, u.uy));
#else
                You("%sメッセージを拭きとれなかった．",
                    oep->engr_type == BURN
                        ? (is_ice(u.ux, u.uy) ? "刻まれている" : "焼き付けられている")
                        : "刻まれている");
#endif
                return 1;
            } else if (type != oep->engr_type || c == 'n') {
                if (!Blind || can_reach_floor(TRUE))
/*JP
                    You("will overwrite the current message.");
*/
                    You("メッセージを上書きしようとした．");
                eow = TRUE;
            }
        }
    }

    eloc = surface(u.ux, u.uy);
    switch (type) {
    default:
/*JP
        everb = (oep && !eow ? "add to the weird writing on"
*/
        everb = (oep && !eow ? "奇妙な文字列に書き加える"
/*JP
                             : "write strangely on");
*/
                             : "奇妙な文字列を書く");
        break;
    case DUST:
/*JP
        everb = (oep && !eow ? "add to the writing in" : "write in");
*/
        everb = (oep && !eow ? "書き加える" : "書く");
/*JP
        eloc = is_ice(u.ux, u.uy) ? "frost" : "dust";
*/
        eloc = is_ice(u.ux,u.uy) ? "霜" : "ほこり";
        break;
    case HEADSTONE:
/*JP
        everb = (oep && !eow ? "add to the epitaph on" : "engrave on");
*/
        everb = (oep && !eow ? "墓碑銘を刻み加える" : "墓碑銘を刻む");
        break;
    case ENGRAVE:
/*JP
        everb = (oep && !eow ? "add to the engraving in" : "engrave in");
*/
        everb = (oep && !eow ? "刻み加える" : "刻む");
        break;
    case BURN:
#if 0 /*JP*/
        everb = (oep && !eow
                     ? (is_ice(u.ux, u.uy) ? "add to the text melted into"
                                           : "add to the text burned into")
                     : (is_ice(u.ux, u.uy) ? "melt into" : "burn into"));
#else
        everb = (oep && !eow
                 ? ( is_ice(u.ux,u.uy) ? "刻み加える"
                                       : "燃えている文字に書き加える")
                 : ( is_ice(u.ux,u.uy) ? "刻む" : "焼印をいれる"));
#endif
        break;
    case MARK:
/*JP
        everb = (oep && !eow ? "add to the graffiti on" : "scribble on");
*/
        everb = (oep && !eow ? "落書に書き加える" : "はしり書きする");
        break;
    case ENGR_BLOOD:
/*JP
        everb = (oep && !eow ? "add to the scrawl on" : "scrawl on");
*/
        everb = (oep && !eow ? "なぐり書きに書き加える" : "なぐり書きする");
        break;
    }

    /* Tell adventurer what is going on */
    if (otmp != &zeroobj)
/*JP
        You("%s the %s with %s.", everb, eloc, doname(otmp));
*/
        You("%sで%sに%s．", doname(otmp), eloc, jpast(everb));
    else
#if 0 /*JP*/
        You("%s the %s with your %s.", everb, eloc, body_part(FINGERTIP));
#else
        You("%sで%sに%s．", body_part(FINGER), eloc, jpast(everb));
#endif

    /* Prompt for engraving! */
/*JP
    Sprintf(qbuf, "What do you want to %s the %s here?", everb, eloc);
*/
    Sprintf(qbuf,"%sに何と%sか？", eloc, jpolite(everb));
    getlin(qbuf, ebuf);
    /* convert tabs to spaces and condense consecutive spaces to one */
    mungspaces(ebuf);

    /* Count the actual # of chars engraved not including spaces */
    len = strlen(ebuf);
    for (sp = ebuf; *sp; sp++)
        if (*sp == ' ')
            len -= 1;

    if (len == 0 || index(ebuf, '\033')) {
        if (zapwand) {
            if (!Blind)
#if 0 /*JP*/
                pline("%s, then %s.", Tobjnam(otmp, "glow"),
                      otense(otmp, "fade"));
#else
                pline("%sは輝いたが，すぐに消えた．", xname(otmp));
#endif
            return 1;
        } else {
            pline1(Never_mind);
            return 0;
        }
    }

    /* A single `x' is the traditional signature of an illiterate person */
    if (len != 1 || (!index(ebuf, 'x') && !index(ebuf, 'X')))
        u.uconduct.literate++;

    /* Mix up engraving if surface or state of mind is unsound.
       Note: this won't add or remove any spaces. */
    for (sp = ebuf; *sp; sp++) {
        if (*sp == ' ')
            continue;
        if (((type == DUST || type == ENGR_BLOOD) && !rn2(25))
            || (Blind && !rn2(11)) || (Confusion && !rn2(7))
            || (Stunned && !rn2(4)) || (Hallucination && !rn2(2)))
#if 0 /*JP*/
            *sp = ' ' + rnd(96 - 2); /* ASCII '!' thru '~'
                                        (excludes ' ' and DEL) */
#else /*JP:日本語でランダム化 */
            {
                if(is_kanji1(ebuf, sp-ebuf))
                    jrndm_replace(sp);
                else if(is_kanji2(ebuf, sp-ebuf))
                    jrndm_replace(sp-1);
                else
                    *sp = '!' + rn2(93); /* ASCII-code only */
            }
#endif
    }

    /* Previous engraving is overwritten */
    if (eow) {
        del_engr(oep);
        oep = (struct engr *) 0;
    }

    /* Figure out how long it took to engrave, and if player has
     * engraved too much.
     */
    switch (type) {
    default:
        multi = -(len / 10);
        if (multi)
/*JP
            nomovemsg = "You finish your weird engraving.";
*/
            nomovemsg = "あなたは奇妙な刻みを終えた．";
        break;
    case DUST:
        multi = -(len / 10);
        if (multi)
/*JP
            nomovemsg = "You finish writing in the dust.";
*/
            nomovemsg = "あなたはほこりに書き終えた．";
        break;
    case HEADSTONE:
    case ENGRAVE:
        multi = -(len / 10);
        if (otmp->oclass == WEAPON_CLASS
            && (otmp->otyp != ATHAME || otmp->cursed)) {
            multi = -len;
            maxelen = ((otmp->spe + 3) * 2) + 1;
            /* -2 => 3, -1 => 5, 0 => 7, +1 => 9, +2 => 11
             * Note: this does not allow a +0 anything (except an athame)
             * to engrave "Elbereth" all at once.
             * However, you can engrave "Elb", then "ere", then "th".
             */
/*JP
            pline("%s dull.", Yobjnam2(otmp, "get"));
*/
            Your("%sは刃こぼれした．", xname(otmp));
            costly_alteration(otmp, COST_DEGRD);
            if (len > maxelen) {
                multi = -maxelen;
                otmp->spe = -3;
            } else if (len > 1)
                otmp->spe -= len >> 1;
            else
                otmp->spe -= 1; /* Prevent infinite engraving */
        } else if (otmp->oclass == RING_CLASS || otmp->oclass == GEM_CLASS) {
            multi = -len;
        }
        if (multi)
/*JP
            nomovemsg = "You finish engraving.";
*/
            nomovemsg = "あなたは刻み終えた．";
        break;
    case BURN:
        multi = -(len / 10);
        if (multi)
            nomovemsg = is_ice(u.ux, u.uy)
/*JP
                          ? "You finish melting your message into the ice."
*/
                          ? "氷へメッセージを刻み終えた．"
/*JP
                          : "You finish burning your message into the floor.";
*/
                          : "床へメッセージを焼きいれ終えた．";
        break;
    case MARK:
        multi = -(len / 10);
        if (otmp->otyp == MAGIC_MARKER) {
            maxelen = otmp->spe * 2; /* one charge / 2 letters */
            if (len > maxelen) {
/*JP
                Your("marker dries out.");
*/
                Your("マーカは乾ききった．");
                otmp->spe = 0;
                multi = -(maxelen / 10);
            } else if (len > 1)
                otmp->spe -= len >> 1;
            else
                otmp->spe -= 1; /* Prevent infinite graffiti */
        }
        if (multi)
/*JP
            nomovemsg = "You finish defacing the dungeon.";
*/
            nomovemsg = "あなたは迷宮への落書を書き終えた．";
        break;
    case ENGR_BLOOD:
        multi = -(len / 10);
        if (multi)
/*JP
            nomovemsg = "You finish scrawling.";
*/
            nomovemsg = "はしり書きを書き終えた．";
        break;
    }

    /* Chop engraving down to size if necessary */
    if (len > maxelen) {
        for (sp = ebuf; maxelen && *sp; sp++)
            if (*sp == ' ')
                maxelen--;
        if (!maxelen && *sp) {
#if 1 /*JP*//*漢字の1バイト目だけが残らないように*/
            if(is_kanji2(ebuf, sp - ebuf))
                --sp;
#endif
            *sp = '\0';
            if (multi)
/*JP
                nomovemsg = "You cannot write any more.";
*/
                nomovemsg = "これ以上何も書けなかった．";
/*JP
            You("are only able to write \"%s\".", ebuf);
*/
            You("「%s」とまでしか書けなかった．", ebuf);
        }
    }

    if (oep) /* add to existing engraving */
        Strcpy(buf, oep->engr_txt);
    (void) strncat(buf, ebuf, BUFSZ - (int) strlen(buf) - 1);
    /* Put the engraving onto the map */
    make_engr_at(u.ux, u.uy, buf, moves - multi, type);

    if (post_engr_text[0])
        pline("%s", post_engr_text);
    if (doblind && !resists_blnd(&youmonst)) {
/*JP
        You("are blinded by the flash!");
*/
        You("まばゆい光で目がくらんだ！");
        make_blinded((long) rnd(50), FALSE);
        if (!Blind)
            Your1(vision_clears);
    }
    return 1;
}

/* while loading bones, clean up text which might accidentally
   or maliciously disrupt player's terminal when displayed */
void
sanitize_engravings()
{
    struct engr *ep;

    for (ep = head_engr; ep; ep = ep->nxt_engr) {
        sanitize_name(ep->engr_txt);
    }
}

void
save_engravings(fd, mode)
int fd, mode;
{
    struct engr *ep, *ep2;
    unsigned no_more_engr = 0;

    for (ep = head_engr; ep; ep = ep2) {
        ep2 = ep->nxt_engr;
        if (ep->engr_lth && ep->engr_txt[0] && perform_bwrite(mode)) {
            bwrite(fd, (genericptr_t) &ep->engr_lth, sizeof ep->engr_lth);
            bwrite(fd, (genericptr_t) ep, sizeof (struct engr) + ep->engr_lth);
        }
        if (release_data(mode))
            dealloc_engr(ep);
    }
    if (perform_bwrite(mode))
        bwrite(fd, (genericptr_t) &no_more_engr, sizeof no_more_engr);
    if (release_data(mode))
        head_engr = 0;
}

void
rest_engravings(fd)
int fd;
{
    struct engr *ep;
    unsigned lth;

    head_engr = 0;
    while (1) {
        mread(fd, (genericptr_t) &lth, sizeof lth);
        if (lth == 0)
            return;
        ep = newengr(lth);
        mread(fd, (genericptr_t) ep, sizeof (struct engr) + lth);
        ep->nxt_engr = head_engr;
        head_engr = ep;
        ep->engr_txt = (char *) (ep + 1); /* Andreas Bormann */
        /* Mark as finished for bones levels -- no problem for
         * normal levels as the player must have finished engraving
         * to be able to move again.
         */
        ep->engr_time = moves;
    }
}

/* to support '#stats' wizard-mode command */
void
engr_stats(hdrfmt, hdrbuf, count, size)
const char *hdrfmt;
char *hdrbuf;
long *count, *size;
{
    struct engr *ep;

    Sprintf(hdrbuf, hdrfmt, (long) sizeof (struct engr));
    *count = *size = 0L;
    for (ep = head_engr; ep; ep = ep->nxt_engr) {
        ++*count;
        *size += (long) sizeof *ep + (long) ep->engr_lth;
    }
}

void
del_engr(ep)
register struct engr *ep;
{
    if (ep == head_engr) {
        head_engr = ep->nxt_engr;
    } else {
        register struct engr *ept;

        for (ept = head_engr; ept; ept = ept->nxt_engr)
            if (ept->nxt_engr == ep) {
                ept->nxt_engr = ep->nxt_engr;
                break;
            }
        if (!ept) {
            impossible("Error in del_engr?");
            return;
        }
    }
    dealloc_engr(ep);
}

/* randomly relocate an engraving */
void
rloc_engr(ep)
struct engr *ep;
{
    int tx, ty, tryct = 200;

    do {
        if (--tryct < 0)
            return;
        tx = rn1(COLNO - 3, 2);
        ty = rn2(ROWNO);
    } while (engr_at(tx, ty) || !goodpos(tx, ty, (struct monst *) 0, 0));

    ep->engr_x = tx;
    ep->engr_y = ty;
}

/* Create a headstone at the given location.
 * The caller is responsible for newsym(x, y).
 */
void
make_grave(x, y, str)
int x, y;
const char *str;
{
    char buf[BUFSZ];

    /* Can we put a grave here? */
    if ((levl[x][y].typ != ROOM && levl[x][y].typ != GRAVE) || t_at(x, y))
        return;
    /* Make the grave */
    levl[x][y].typ = GRAVE;
    /* Engrave the headstone */
    del_engr_at(x, y);
    if (!str)
        str = get_rnd_text(EPITAPHFILE, buf);
    make_engr_at(x, y, str, 0L, HEADSTONE);
    return;
}

/*engrave.c*/
