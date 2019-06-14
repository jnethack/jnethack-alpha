/* NetHack 3.6	sounds.c	$NHDT-Date: 1542765362 2018/11/21 01:56:02 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.81 $ */
/*      Copyright (c) 1989 Janet Walz, Mike Threepoint */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-2019            */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL boolean FDECL(mon_is_gecko, (struct monst *));
STATIC_DCL int FDECL(domonnoise, (struct monst *));
STATIC_DCL int NDECL(dochat);
STATIC_DCL int FDECL(mon_in_room, (struct monst *, int));

/* this easily could be a macro, but it might overtax dumb compilers */
STATIC_OVL int
mon_in_room(mon, rmtyp)
struct monst *mon;
int rmtyp;
{
    int rno = levl[mon->mx][mon->my].roomno;
    if (rno >= ROOMOFFSET)
        return rooms[rno - ROOMOFFSET].rtype == rmtyp;
    return FALSE;
}

void
dosounds()
{
    register struct mkroom *sroom;
    register int hallu, vx, vy;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
    int xx;
#endif
    struct monst *mtmp;

    if (Deaf || !flags.acoustics || u.uswallow || Underwater)
        return;

    hallu = Hallucination ? 1 : 0;

    if (level.flags.nfountains && !rn2(400)) {
#if 0 /*JP*/
        static const char *const fountain_msg[4] = {
            "bubbling water.", "water falling on coins.",
            "the splashing of a naiad.", "a soda fountain!",
#else
        static const char *const fountain_msg[4] = {
            "ゴボゴボという音を聞いた．", "ピチャピチャという音を聞いた．",
            "バシャバシャという音を聞いた．", "炭酸飲料のシューという音を聞いた！",
#endif
        };
        You_hear1(fountain_msg[rn2(3) + hallu]);
    }
    if (level.flags.nsinks && !rn2(300)) {
#if 0 /*JP*/
        static const char *const sink_msg[3] = {
            "a slow drip.", "a gurgling noise.", "dishes being washed!",
#else
        static const char *const sink_msg[3] = {
            "水がぽたぽたと落ちる音を聞いた．",
            "がらがらという音を聞いた．",
            "皿を洗う音を聞いた！",
#endif
        };
        You_hear1(sink_msg[rn2(2) + hallu]);
    }
    if (level.flags.has_court && !rn2(200)) {
#if 0 /*JP*/
        static const char *const throne_msg[4] = {
            "the tones of courtly conversation.",
            "a sceptre pounded in judgment.",
            "Someone shouts \"Off with %s head!\"", "Queen Beruthiel's cats!",
#else
        static const char *const throne_msg[4] = {
            "上品な話し声を聞いた．",
            "裁判で笏を突く音を聞いた．",
            "だれかが「そのものの首をはねよ！」と叫ぶ声を聞いた．",
            "ベルシエル王妃の猫の声を聞いた！",
#endif
        };
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((mtmp->msleeping || is_lord(mtmp->data)
                 || is_prince(mtmp->data)) && !is_animal(mtmp->data)
                && mon_in_room(mtmp, COURT)) {
                /* finding one is enough, at least for now */
                int which = rn2(3) + hallu;

                if (which != 2)
                    You_hear1(throne_msg[which]);
                else
                    pline(throne_msg[2], uhis());
                return;
            }
        }
    }
    if (level.flags.has_swamp && !rn2(200)) {
#if 0 /*JP*/
        static const char *const swamp_msg[3] = {
            "hear mosquitoes!", "smell marsh gas!", /* so it's a smell...*/
            "hear Donald Duck!",
#else
        static const char *const swamp_msg[3] = {
            "蚊の羽音を聞いた．",
            "腐った匂いがした！",       /* so it's a smell...*/
            "ドナルドダックの声を聞いた！",
#endif
        };
        You1(swamp_msg[rn2(2) + hallu]);
        return;
    }
    if (level.flags.has_vault && !rn2(200)) {
        if (!(sroom = search_special(VAULT))) {
            /* strange ... */
            level.flags.has_vault = 0;
            return;
        }
        if (gd_sound())
            switch (rn2(2) + hallu) {
            case 1: {
                boolean gold_in_vault = FALSE;

                for (vx = sroom->lx; vx <= sroom->hx; vx++)
                    for (vy = sroom->ly; vy <= sroom->hy; vy++)
                        if (g_at(vx, vy))
                            gold_in_vault = TRUE;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
                /* Bug in aztec assembler here. Workaround below */
                xx = ROOM_INDEX(sroom) + ROOMOFFSET;
                xx = (xx != vault_occupied(u.urooms));
                if (xx)
#else
                if (vault_occupied(u.urooms)
                    != (ROOM_INDEX(sroom) + ROOMOFFSET))
#endif /* AZTEC_C_WORKAROUND */
                {
                    if (gold_in_vault)
#if 0 /*JP*/
                        You_hear(!hallu
                                     ? "someone counting money."
                                     : "the quarterback calling the play.");
#else
                        You_hear(!hallu
                                 ? "誰かがお金を数えている音を聞いた．"
                                 : "クォータバックが指示をする声を聞いた．");
#endif
                    else
/*JP
                        You_hear("someone searching.");
*/
                        You_hear("誰かが捜索している音を聞いた．");
                    break;
                }
            }
                /*FALLTHRU*/
            case 0:
/*JP
                You_hear("the footsteps of a guard on patrol.");
*/
                You_hear("警備員のパトロールする音を聞いた．");
                break;
            case 2:
/*JP
                You_hear("Ebenezer Scrooge!");
*/
                You_hear("こち亀の両さんの声を聞いた！");
                break;
            }
        return;
    }
    if (level.flags.has_beehive && !rn2(200)) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((mtmp->data->mlet == S_ANT && is_flyer(mtmp->data))
                && mon_in_room(mtmp, BEEHIVE)) {
                switch (rn2(2) + hallu) {
                case 0:
/*JP
                    You_hear("a low buzzing.");
*/
                        You_hear("ぶーんという音を聞いた．");
                    break;
                case 1:
/*JP
                    You_hear("an angry drone.");
*/
                        You_hear("興奮した雄バチの音を聞いた．");
                    break;
                case 2:
#if 0 /*JP*/
                    You_hear("bees in your %sbonnet!",
                             uarmh ? "" : "(nonexistent) ");
#else
                        You_hear("ハチがあなたの帽子%sの中にいる音を聞いた！",
                            uarmh ? "" : "(被ってないけど)");
#endif
                    break;
                }
                return;
            }
        }
    }
    if (level.flags.has_morgue && !rn2(200)) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((is_undead(mtmp->data) || is_vampshifter(mtmp))
                && mon_in_room(mtmp, MORGUE)) {
#if 0 /*JP*/
                const char *hair = body_part(HAIR); /* hair/fur/scales */
#endif

                switch (rn2(2) + hallu) {
                case 0:
/*JP
                    You("suddenly realize it is unnaturally quiet.");
*/
                    You("不自然なくらい静かなのに気づいた．");
                    break;
                case 1:
#if 0 /*JP*/
                    pline_The("%s on the back of your %s %s up.", hair,
                              body_part(NECK), vtense(hair, "stand"));
#else
                    pline("あなたの%sのうしろの%sが逆立った．",
                          body_part(NECK), body_part(HAIR));
#endif
                    break;
                case 2:
#if 0 /*JP*/
                    pline_The("%s on your %s %s to stand up.", hair,
                              body_part(HEAD), vtense(hair, "seem"));
#else
                    pline("あなたの%sの%sは逆立った．",
                          body_part(HEAD), body_part(HAIR));
#endif
                    break;
                }
                return;
            }
        }
    }
    if (level.flags.has_barracks && !rn2(200)) {
        static const char *const barracks_msg[4] = {
#if 0 /*JP*/
            "blades being honed.", "loud snoring.", "dice being thrown.",
            "General MacArthur!",
#else
            "刃物を研ぐ音を聞いた．",
            "大きないびきを聞いた．",
            "ダイスが振られる音を聞いた．",
            "マッカーサー将軍の声を聞いた！",
#endif
        };
        int count = 0;

        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (is_mercenary(mtmp->data)
#if 0 /* don't bother excluding these */
                && !strstri(mtmp->data->mname, "watch")
                && !strstri(mtmp->data->mname, "guard")
#endif
                && mon_in_room(mtmp, BARRACKS)
                /* sleeping implies not-yet-disturbed (usually) */
                && (mtmp->msleeping || ++count > 5)) {
                You_hear1(barracks_msg[rn2(3) + hallu]);
                return;
            }
        }
    }
    if (level.flags.has_zoo && !rn2(200)) {
        static const char *const zoo_msg[3] = {
#if 0 /*JP*/
            "a sound reminiscent of an elephant stepping on a peanut.",
            "a sound reminiscent of a seal barking.", "Doctor Dolittle!",
#else
            "象がピーナッツの上で踊るような音を聞いた．",
            "アシカが吠えるような音を聞いた．",
            "ドリトル先生の声を聞いた！",
#endif
        };
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if ((mtmp->msleeping || is_animal(mtmp->data))
                && mon_in_room(mtmp, ZOO)) {
                You_hear1(zoo_msg[rn2(2) + hallu]);
                return;
            }
        }
    }
    if (level.flags.has_shop && !rn2(200)) {
        if (!(sroom = search_special(ANY_SHOP))) {
            /* strange... */
            level.flags.has_shop = 0;
            return;
        }
        if (tended_shop(sroom)
            && !index(u.ushops, (int) (ROOM_INDEX(sroom) + ROOMOFFSET))) {
            static const char *const shop_msg[3] = {
#if 0 /*JP*/
                "someone cursing shoplifters.",
                "the chime of a cash register.", "Neiman and Marcus arguing!",
#else
                "誰かが泥棒をののしる声を聞いた．",
                "レジのチーンという音を聞いた．",
                "イトーとヨーカドーの議論を聞いた！",
#endif
            };
            You_hear1(shop_msg[rn2(2) + hallu]);
        }
        return;
    }
    if (level.flags.has_temple && !rn2(200)
        && !(Is_astralevel(&u.uz) || Is_sanctum(&u.uz))) {
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (mtmp->ispriest && inhistemple(mtmp)
                /* priest must be active */
                && mtmp->mcanmove && !mtmp->msleeping
                /* hero must be outside this temple */
                && temple_occupied(u.urooms) != EPRI(mtmp)->shroom)
                break;
        }
        if (mtmp) {
            /* Generic temple messages; no attempt to match topic or tone
               to the pantheon involved, let alone to the specific deity.
               These are assumed to be coming from the attending priest;
               asterisk means that the priest must be capable of speech;
               pound sign (octathorpe,&c--don't go there) means that the
               priest and the altar must not be directly visible (we don't
               care if telepathy or extended detection reveals that the
               priest is not currently standing on the altar; he's mobile). */
            static const char *const temple_msg[] = {
#if 0 /*JP*/
                "*someone praising %s.", "*someone beseeching %s.",
                "#an animal carcass being offered in sacrifice.",
                "*a strident plea for donations.",
#else
                "*誰かが%sを賛美しているのを聞いた．",
                "*誰かが%sを熱望しているのを聞いた．",
                "#動物の死体を生け贄に捧げる声を聞いた．",
                "*執拗に寄付を要求しているのを聞いた．",
#endif
            };
            const char *msg;
            int trycount = 0, ax = EPRI(mtmp)->shrpos.x,
                ay = EPRI(mtmp)->shrpos.y;
            boolean speechless = (mtmp->data->msound <= MS_ANIMAL),
                    in_sight = canseemon(mtmp) || cansee(ax, ay);

            do {
                msg = temple_msg[rn2(SIZE(temple_msg) - 1 + hallu)];
                if (index(msg, '*') && speechless)
                    continue;
                if (index(msg, '#') && in_sight)
                    continue;
                break; /* msg is acceptable */
            } while (++trycount < 50);
            while (!letter(*msg))
                ++msg; /* skip control flags */
            if (index(msg, '%'))
                You_hear(msg, halu_gname(EPRI(mtmp)->shralign));
            else
                You_hear1(msg);
            return;
        }
    }
    if (Is_oracle_level(&u.uz) && !rn2(400)) {
        /* make sure the Oracle is still here */
        for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
            if (DEADMONSTER(mtmp))
                continue;
            if (mtmp->data == &mons[PM_ORACLE])
                break;
        }
        /* and don't produce silly effects when she's clearly visible */
        if (mtmp && (hallu || !canseemon(mtmp))) {
            static const char *const ora_msg[5] = {
#if 0 /*JP*/
                "a strange wind.",     /* Jupiter at Dodona */
                "convulsive ravings.", /* Apollo at Delphi */
                "snoring snakes.",     /* AEsculapius at Epidaurus */
                "someone say \"No more woodchucks!\"",
                "a loud ZOT!" /* both rec.humor.oracle */
#else
                "奇妙な風の音を聞いた．",
                "半狂乱の声を聞いた．",
                "蛇のいびきを聞いた．",
                "誰かが「もうウッドチャックはいらない！」と言っている声を聞いた．",
                "大きなＺＯＴを聞いた！"
#endif
            };
            You_hear1(ora_msg[rn2(3) + hallu * 2]);
        }
        return;
    }
}

static const char *const h_sounds[] = {
#if 0 /*JP*/
    "beep",   "boing",   "sing",   "belche", "creak",   "cough",
    "rattle", "ululate", "pop",    "jingle", "sniffle", "tinkle",
    "eep",    "clatter", "hum",    "sizzle", "twitter", "wheeze",
    "rustle", "honk",    "lisp",   "yodel",  "coo",     "burp",
    "moo",    "boom",    "murmur", "oink",   "quack",   "rumble",
    "twang",  "bellow",  "toot",   "gargle", "hoot",    "warble"
#else
    "ピーッと鳴いた",
    "騒ぎたてた",
    "歌った",
    "げっぷをした",
    "キーキーと鳴いた",
    "せき込んだ",

    "カタカタ鳴った",
    "ウォンウォン鳴いた",
    "ポンと鳴った",
    "チャリンと鳴った",
    "クンクン鳴いた",
    "チリンチリンと鳴った",

    "イーッと鳴いた",
    "カチャカチャ音を立てた",
    "ハミングした",
    "ジューッと音を立てた",
    "ピヨピヨ鳴いた",
    "ゼイゼイ息をした",

    "カサカサ音を立てた",
    "警笛を鳴らした",
    "舌足らずにしゃべった",
    "ヨーデルを歌った",
    "クークー鳴いた",
    "げっぷをした",

    "モーと鳴いた",
    "爆発音を響かせた",
    "ブツブツつぶやいた",
    "ブーブー鳴いた",
    "クワックワッと鳴いた",
    "ゴロゴロ鳴った",

    "ポロロンと鳴った",
    "どなった",
    "ラッパの音を鳴らした",
    "ガラガラ声を出した",
    "ホーホー鳴いた",
    "さえずった"
#endif
};

const char *
growl_sound(mtmp)
register struct monst *mtmp;
{
    const char *ret;

    switch (mtmp->data->msound) {
    case MS_MEW:
    case MS_HISS:
/*JP
        ret = "hiss";
*/
            ret = "シーッと鳴いた";
        break;
    case MS_BARK:
    case MS_GROWL:
/*JP
        ret = "growl";
*/
            ret = "はげしく吠えた";
        break;
    case MS_ROAR:
/*JP
        ret = "roar";
*/
            ret = "吠えた";
        break;
    case MS_BUZZ:
/*JP
        ret = "buzz";
*/
            ret = "ブーッと鳴いた";
        break;
    case MS_SQEEK:
/*JP
        ret = "squeal";
*/
            ret = "キーキー鳴いた";
        break;
    case MS_SQAWK:
/*JP
        ret = "screech";
*/
            ret = "金切り声を立てた";
        break;
    case MS_NEIGH:
/*JP
        ret = "neigh";
*/
            ret = "いなないた";
        break;
    case MS_WAIL:
/*JP
        ret = "wail";
*/
            ret = "悲しく鳴いた";
        break;
    case MS_SILENT:
/*JP
        ret = "commotion";
*/
                ret = "興奮した";
        break;
    default:
/*JP
        ret = "scream";
*/
                ret = "金切り声をあげた";
    }
    return ret;
}

/* the sounds of a seriously abused pet, including player attacking it */
void
growl(mtmp)
register struct monst *mtmp;
{
    register const char *growl_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
        return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        growl_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        growl_verb = growl_sound(mtmp);
    if (growl_verb) {
/*JP
        pline("%s %s!", Monnam(mtmp), vtense((char *) 0, growl_verb));
*/
        pline("%sは%s！", Monnam(mtmp), growl_verb);
        if (context.run)
            nomul(0);
        wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 18);
    }
}

/* the sounds of mistreated pets */
void
yelp(mtmp)
register struct monst *mtmp;
{
    register const char *yelp_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
        return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        yelp_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        switch (mtmp->data->msound) {
        case MS_MEW:
/*JP
            yelp_verb = (!Deaf) ? "yowl" : "arch";
*/
            yelp_verb = (!Deaf) ? "悲しく鳴いた" : "arch";
            break;
        case MS_BARK:
        case MS_GROWL:
/*JP
            yelp_verb = (!Deaf) ? "yelp" : "recoil";
*/
            yelp_verb = (!Deaf) ? "キャンキャン鳴いた" : "尻込みした";
            break;
        case MS_ROAR:
/*JP
            yelp_verb = (!Deaf) ? "snarl" : "bluff";
*/
            yelp_verb = (!Deaf) ? "うなった" : "やせ我慢した";
            break;
        case MS_SQEEK:
/*JP
            yelp_verb = (!Deaf) ? "squeal" : "quiver";
*/
            yelp_verb = (!Deaf) ? "キーキー鳴いた" : "震えた";
            break;
        case MS_SQAWK:
/*JP
            yelp_verb = (!Deaf) ? "screak" : "thrash";
*/
            yelp_verb = (!Deaf) ? "金切り声を立てた" : "のたうち回った";
            break;
        case MS_WAIL:
/*JP
            yelp_verb = (!Deaf) ? "wail" : "cringe";
*/
            yelp_verb = (!Deaf) ? "悲しく鳴いた" : "身をすくめた";
            break;
        }
    if (yelp_verb) {
/*JP
        pline("%s %s!", Monnam(mtmp), vtense((char *) 0, yelp_verb));
*/
        pline("%sは%s！", Monnam(mtmp), yelp_verb);
        if (context.run)
            nomul(0);
        wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 12);
    }
}

/* the sounds of distressed pets */
void
whimper(mtmp)
register struct monst *mtmp;
{
    register const char *whimper_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
        return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
        whimper_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
        switch (mtmp->data->msound) {
        case MS_MEW:
        case MS_GROWL:
/*JP
            whimper_verb = "whimper";
*/
            whimper_verb = "クンクン鳴いた";
            break;
        case MS_BARK:
/*JP
            whimper_verb = "whine";
*/
            whimper_verb = "クーンと鳴いた";
            break;
        case MS_SQEEK:
/*JP
            whimper_verb = "squeal";
*/
            whimper_verb = "キーキー鳴いた";
            break;
        }
    if (whimper_verb) {
/*JP
        pline("%s %s.", Monnam(mtmp), vtense((char *) 0, whimper_verb));
*/
        pline("%sは%s．", Monnam(mtmp), whimper_verb);
        if (context.run)
            nomul(0);
        wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 6);
    }
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleeping || !mtmp->mcanmove
        || !(carnivorous(mtmp->data) || herbivorous(mtmp->data)))
        return;

    /* presumably nearness and soundok checks have already been made */
    if (!is_silent(mtmp->data) && mtmp->data->msound <= MS_ANIMAL)
        (void) domonnoise(mtmp);
    else if (mtmp->data->msound >= MS_HUMANOID) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
/*JP
        verbalize("I'm hungry.");
*/
        verbalize("はらぺこだ．");
    }
}

/* return True if mon is a gecko or seems to look like one (hallucination) */
STATIC_OVL boolean
mon_is_gecko(mon)
struct monst *mon;
{
    int glyph;

    /* return True if it is actually a gecko */
    if (mon->data == &mons[PM_GECKO])
        return TRUE;
    /* return False if it is a long worm; we might be chatting to its tail
       (not strictly needed; long worms are MS_SILENT so won't get here) */
    if (mon->data == &mons[PM_LONG_WORM])
        return FALSE;
    /* result depends upon whether map spot shows a gecko, which will
       be due to hallucination or to mimickery since mon isn't one */
    glyph = glyph_at(mon->mx, mon->my);
    return (boolean) (glyph_to_mon(glyph) == PM_GECKO);
}

STATIC_OVL int
domonnoise(mtmp)
register struct monst *mtmp;
{
    char verbuf[BUFSZ];
    register const char *pline_msg = 0, /* Monnam(mtmp) will be prepended */
        *verbl_msg = 0,                 /* verbalize() */
        *verbl_msg_mcan = 0;            /* verbalize() if cancelled */
    struct permonst *ptr = mtmp->data;
    int msound = ptr->msound;

    /* presumably nearness and sleep checks have already been made */
    if (Deaf)
        return 0;
    if (is_silent(ptr))
        return 0;

    /* leader might be poly'd; if he can still speak, give leader speech */
    if (mtmp->m_id == quest_status.leader_m_id && msound > MS_ANIMAL)
        msound = MS_LEADER;
    /* make sure it's your role's quest guardian; adjust if not */
    else if (msound == MS_GUARDIAN && ptr != &mons[urole.guardnum])
        msound = mons[genus(monsndx(ptr), 1)].msound;
    /* some normally non-speaking types can/will speak if hero is similar */
    else if (msound == MS_ORC         /* note: MS_ORC is same as MS_GRUNT */
             && (same_race(ptr, youmonst.data)           /* current form, */
                 || same_race(ptr, &mons[Race_switch]))) /* unpoly'd form */
        msound = MS_HUMANOID;
    /* silliness, with slight chance to interfere with shopping */
    else if (Hallucination && mon_is_gecko(mtmp))
        msound = MS_SELL;

    /* be sure to do this before talking; the monster might teleport away, in
     * which case we want to check its pre-teleport position
     */
    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    switch (msound) {
    case MS_ORACLE:
        return doconsult(mtmp);
    case MS_PRIEST:
        priest_talk(mtmp);
        break;
    case MS_LEADER:
    case MS_NEMESIS:
    case MS_GUARDIAN:
        quest_chat(mtmp);
        break;
    case MS_SELL: /* pitch, pay, total */
        if (!Hallucination || (mtmp->isshk && !rn2(2))) {
            shk_chat(mtmp);
        } else {
            /* approximation of GEICO's advertising slogan (it actually
               concludes with "save you 15% or more on car insurance.") */
/*JP
            Sprintf(verbuf, "15 minutes could save you 15 %s.",
*/
            Sprintf(verbuf, "15分頂ければ15%s節約できます．",
                    currency(15L)); /* "zorkmids" */
            verbl_msg = verbuf;
        }
        break;
    case MS_VAMPIRE: {
        /* vampire messages are varied by tameness, peacefulness, and time of
         * night */
        boolean isnight = night();
        boolean kindred = (Upolyd && (u.umonnum == PM_VAMPIRE
                                      || u.umonnum == PM_VAMPIRE_LORD));
        boolean nightchild =
            (Upolyd && (u.umonnum == PM_WOLF || u.umonnum == PM_WINTER_WOLF
                        || u.umonnum == PM_WINTER_WOLF_CUB));
#if 0 /*JP*/
        const char *racenoun =
            (flags.female && urace.individual.f)
                ? urace.individual.f
                : (urace.individual.m) ? urace.individual.m : urace.noun;
#else
        const char *racenoun = (flags.female) ? "あなた" : "おまえ" ;
#endif

        if (mtmp->mtame) {
            if (kindred) {
#if 0 /*JP*/
                Sprintf(verbuf, "Good %s to you Master%s",
                        isnight ? "evening" : "day",
                        isnight ? "!" : ".  Why do we not rest?");
#else
                Sprintf(verbuf, "ご主人様，%s%s",
                        isnight ? "こんばんは" : "こんにちは",
                        isnight ? "!" : "．お休みになりませんか？");
#endif
                verbl_msg = verbuf;
            } else {
#if 0 /*JP*/
                Sprintf(verbuf, "%s%s",
                        nightchild ? "Child of the night, " : "",
                        midnight()
                         ? "I can stand this craving no longer!"
                         : isnight
                          ? "I beg you, help me satisfy this growing craving!"
                          : "I find myself growing a little weary.");
#else
                Sprintf(verbuf,"%s%s",
                        nightchild ? "夜の仔よ，" : "",
                        midnight()
                        ? "私はこれ以上渇望を抑えられない！"
                        : isnight
                         ? "ふくれあがる渇望を満たすのを助けてくれないか？頼む！"
                         : "私は少々疲れたようだ．");
#endif
                verbl_msg = verbuf;
            }
        } else if (mtmp->mpeaceful) {
            if (kindred && isnight) {
#if 0 /*JP*/
                Sprintf(verbuf, "Good feeding %s!",
                        flags.female ? "sister" : "brother");
#else
                Sprintf(verbuf, "よう兄弟！");
#endif
                verbl_msg = verbuf;
            } else if (nightchild && isnight) {
/*JP
                Sprintf(verbuf, "How nice to hear you, child of the night!");
*/
                Sprintf(verbuf, "夜の仔よ，元気そうだな！");
                verbl_msg = verbuf;
            } else
/*JP
                verbl_msg = "I only drink... potions.";
*/
                verbl_msg = "私は薬．．しか飲まない．";
        } else {
            int vampindex;
            static const char *const vampmsg[] = {
                /* These first two (0 and 1) are specially handled below */
/*JP
                "I vant to suck your %s!",
*/
                "お前の%sをよこせ！",
/*JP
                "I vill come after %s without regret!",
*/
                "存分に%sを追撃させてもらおう！",
                /* other famous vampire quotes can follow here if desired */
            };
            if (kindred)
                verbl_msg =
/*JP
                    "This is my hunting ground that you dare to prowl!";
*/
                    "おまえがうろついているこのあたりは私の狩場だ！";
            else if (youmonst.data == &mons[PM_SILVER_DRAGON]
                     || youmonst.data == &mons[PM_BABY_SILVER_DRAGON]) {
                /* Silver dragons are silver in color, not made of silver */
#if 0 /*JP*/
                Sprintf(verbuf, "%s!  Your silver sheen does not frighten me!",
                        youmonst.data == &mons[PM_SILVER_DRAGON]
                            ? "Fool"
                            : "Young Fool");
                verbl_msg = verbuf;
#else
                verbl_msg = "ばかめ！おまえの銀の輝きなど怖くないぞ！";
#endif
            } else {
                vampindex = rn2(SIZE(vampmsg));
                if (vampindex == 0) {
                    Sprintf(verbuf, vampmsg[vampindex], body_part(BLOOD));
                    verbl_msg = verbuf;
                } else if (vampindex == 1) {
                    Sprintf(verbuf, vampmsg[vampindex],
                            Upolyd ? an(mons[u.umonnum].mname)
                                   : an(racenoun));
                    verbl_msg = verbuf;
                } else
                    verbl_msg = vampmsg[vampindex];
            }
        }
    } break;
    case MS_WERE:
        if (flags.moonphase == FULL_MOON && (night() ^ !rn2(13))) {
#if 0 /*JP*/
            pline("%s throws back %s head and lets out a blood curdling %s!",
                  Monnam(mtmp), mhis(mtmp),
                  ptr == &mons[PM_HUMAN_WERERAT] ? "shriek" : "howl");
#else
            pline("%sは頭をのけぞらし背筋が凍るような%sをあげた！",
                  Monnam(mtmp),
                  ptr == &mons[PM_HUMAN_WERERAT] ? "金切り声" : "咆哮");
#endif
            wake_nearto(mtmp->mx, mtmp->my, 11 * 11);
        } else
            pline_msg =
/*JP
                "whispers inaudibly.  All you can make out is \"moon\".";
*/
                "聞きとれないような声でささやいた．かろうじて『月』という言葉だけが聞きとれた．";
        break;
    case MS_BARK:
        if (flags.moonphase == FULL_MOON && night()) {
/*JP
            pline_msg = "howls.";
*/
            pline_msg = "吠えた．";
        } else if (mtmp->mpeaceful) {
            if (mtmp->mtame
                && (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                    || moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
/*JP
                pline_msg = "whines.";
*/
                pline_msg = "クンクン鳴いた．";
            else if (mtmp->mtame && EDOG(mtmp)->hungrytime > moves + 1000)
/*JP
                pline_msg = "yips.";
*/
                pline_msg = "キャンキャン鳴いた．";
            else {
                if (mtmp->data
                    != &mons[PM_DINGO]) /* dingos do not actually bark */
/*JP
                    pline_msg = "barks.";
*/
                    pline_msg = "ワンワン吠えた．";
            }
        } else {
/*JP
            pline_msg = "growls.";
*/
            pline_msg = "うなった．";
        }
        break;
    case MS_MEW:
        if (mtmp->mtame) {
            if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped
                || mtmp->mtame < 5)
/*JP
                pline_msg = "yowls.";
*/
                pline_msg = "悲しく鳴いた．";
            else if (moves > EDOG(mtmp)->hungrytime)
/*JP
                pline_msg = "meows.";
*/
                pline_msg = "ニャーンと鳴いた．";
            else if (EDOG(mtmp)->hungrytime > moves + 1000)
/*JP
                pline_msg = "purrs.";
*/
                pline_msg = "ゴロゴロと鳴いた．";
            else
/*JP
                pline_msg = "mews.";
*/
                pline_msg = "ニャーニャー鳴いた．";
            break;
        }
        /*FALLTHRU*/
    case MS_GROWL:
/*JP
        pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
*/
        pline_msg = mtmp->mpeaceful ? "うなった．" : "激しくうなった！";
        break;
    case MS_ROAR:
/*JP
        pline_msg = mtmp->mpeaceful ? "snarls." : "roars!";
*/
        pline_msg = mtmp->mpeaceful ? "うなった．" : "とても激しく吠えた！";
        break;
    case MS_SQEEK:
/*JP
        pline_msg = "squeaks.";
*/
        pline_msg = "キーキー鳴いた．";
        break;
    case MS_SQAWK:
        if (ptr == &mons[PM_RAVEN] && !mtmp->mpeaceful)
/*JP
            verbl_msg = "Nevermore!";
*/
            verbl_msg = "「もう二度と！」";
        else
/*JP
            pline_msg = "squawks.";
*/
            pline_msg = "キーキー鳴いた．";
        break;
    case MS_HISS:
        if (!mtmp->mpeaceful)
/*JP
            pline_msg = "hisses!";
*/
            pline_msg = "シーッと鳴いた！";
        else
            return 0; /* no sound */
        break;
    case MS_BUZZ:
/*JP
        pline_msg = mtmp->mpeaceful ? "drones." : "buzzes angrily.";
*/
        pline_msg = mtmp->mpeaceful ? "ぶーんと鳴った．" : "ぶんぶん鳴った．";
        break;
    case MS_GRUNT:
/*JP
        pline_msg = "grunts.";
*/
        pline_msg = "ぶーぶー鳴いた．";
        break;
    case MS_NEIGH:
        if (mtmp->mtame < 5)
/*JP
            pline_msg = "neighs.";
*/
            pline_msg = "いなないた．";
        else if (moves > EDOG(mtmp)->hungrytime)
/*JP
            pline_msg = "whinnies.";
*/
            pline_msg = "ヒヒーンと鳴いた．";
        else
/*JP
            pline_msg = "whickers.";
*/
            pline_msg = "ヒヒヒーンと鳴いた．";
        break;
    case MS_WAIL:
/*JP
        pline_msg = "wails mournfully.";
*/
        pline_msg = "悲しげに鳴いた．";
        break;
    case MS_GURGLE:
/*JP
        pline_msg = "gurgles.";
*/
        pline_msg = "ごろごろ喉を鳴らした．";
        break;
    case MS_BURBLE:
/*JP
        pline_msg = "burbles.";
*/
        pline_msg = "ぺちゃくちゃしゃべった．";
        break;
    case MS_SHRIEK:
/*JP
        pline_msg = "shrieks.";
*/
        pline_msg = "金切り声をあげた．";
        aggravate();
        break;
    case MS_IMITATE:
/*JP
        pline_msg = "imitates you.";
*/
        pline_msg = "あなたの真似をした．";
        break;
    case MS_BONES:
/*JP
        pline("%s rattles noisily.", Monnam(mtmp));
*/
        pline("%sはケタケタと笑いだした．",Monnam(mtmp));
/*JP
        You("freeze for a moment.");
*/
        You("一瞬凍りついた．");
        nomul(-2);
/*JP
        multi_reason = "scared by rattling";
*/
        multi_reason = "骨のカタカタ言う音におびえた隙に";
        nomovemsg = 0;
        break;
    case MS_LAUGH: {
        static const char *const laugh_msg[4] = {
/*JP
            "giggles.", "chuckles.", "snickers.", "laughs.",
*/
            "くすくす笑った．", "くすっと笑った．", "ばかにしたように笑った．", "笑った．",
        };
        pline_msg = laugh_msg[rn2(4)];
    } break;
    case MS_MUMBLE:
/*JP
        pline_msg = "mumbles incomprehensibly.";
*/
        pline_msg = "不可解な言葉をつぶやいた．";
        break;
    case MS_DJINNI:
        if (mtmp->mtame) {
/*JP
            verbl_msg = "Sorry, I'm all out of wishes.";
*/
            verbl_msg = "私を助けてくれたことを感謝する！";
        } else if (mtmp->mpeaceful) {
            if (ptr == &mons[PM_WATER_DEMON])
/*JP
                pline_msg = "gurgles.";
*/
                pline_msg = "ゴボゴボゴボゴボ．";
            else
/*JP
                verbl_msg = "I'm free!";
*/
                    verbl_msg = "やっと自由になった！";
        } else {
            if (ptr != &mons[PM_PRISONER])
/*JP
                verbl_msg = "This will teach you not to disturb me!";
*/
                verbl_msg = "じゃまをしないでくれ！";
#if 0
            else
                verbl_msg = "??????????";
#endif
        }
        break;
    case MS_BOAST: /* giants */
        if (!mtmp->mpeaceful) {
            switch (rn2(4)) {
            case 0:
#if 0 /*JP*/
                pline("%s boasts about %s gem collection.", Monnam(mtmp),
                      mhis(mtmp));
#else
                pline("%sは自分の宝石のコレクションを自慢した．", Monnam(mtmp));
#endif
                break;
            case 1:
/*JP
                pline_msg = "complains about a diet of mutton.";
*/
                pline_msg = "毎日羊ばかり食べている事を愚痴った．";
                break;
            default:
/*JP
                pline_msg = "shouts \"Fee Fie Foe Foo!\" and guffaws.";
*/
                pline_msg = "『わっはっはっは！』とばか笑いした．";
                wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
                break;
            }
            break;
        }
        /*FALLTHRU*/
    case MS_HUMANOID:
        if (!mtmp->mpeaceful) {
            if (In_endgame(&u.uz) && is_mplayer(ptr))
                mplayer_talk(mtmp);
            else
/*JP
                pline_msg = "threatens you.";
*/
                pline_msg = "あなたを威嚇した．";
            break;
        }
        /* Generic peaceful humanoid behaviour. */
        if (mtmp->mflee)
/*JP
            pline_msg = "wants nothing to do with you.";
*/
            pline_msg = "あなたと関わりたくないようだ．";
        else if (mtmp->mhp < mtmp->mhpmax / 4)
/*JP
            pline_msg = "moans.";
*/
            pline_msg = "うめき声をあげた．";
        else if (mtmp->mconf || mtmp->mstun)
/*JP
            verbl_msg = !rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?";
*/
            verbl_msg = !rn2(3) ? "へ？" : rn2(2) ? "何？" : "え？";
        else if (!mtmp->mcansee)
/*JP
            verbl_msg = "I can't see!";
*/
            verbl_msg = "何も見えない！";
        else if (mtmp->mtrapped) {
            struct trap *t = t_at(mtmp->mx, mtmp->my);

            if (t)
                t->tseen = 1;
/*JP
            verbl_msg = "I'm trapped!";
*/
            verbl_msg = "罠にはまってしまった！";
        } else if (mtmp->mhp < mtmp->mhpmax / 2)
/*JP
            pline_msg = "asks for a potion of healing.";
*/
            pline_msg = "回復の薬を持ってないか尋ねた．";
        else if (mtmp->mtame && !mtmp->isminion
                 && moves > EDOG(mtmp)->hungrytime)
/*JP
            verbl_msg = "I'm hungry.";
*/
            verbl_msg = "腹が減ったな．";
        /* Specific monsters' interests */
        else if (is_elf(ptr))
/*JP
            pline_msg = "curses orcs.";
*/
            pline_msg = "オークを呪った．";
        else if (is_dwarf(ptr))
/*JP
            pline_msg = "talks about mining.";
*/
            pline_msg = "採掘について話した．";
        else if (likes_magic(ptr))
/*JP
            pline_msg = "talks about spellcraft.";
*/
            pline_msg = "魔術の鍛錬について話した．";
        else if (ptr->mlet == S_CENTAUR)
/*JP
            pline_msg = "discusses hunting.";
*/
            pline_msg = "猟について議論した．";
        else
            switch (monsndx(ptr)) {
            case PM_HOBBIT:
                pline_msg =
                    (mtmp->mhpmax - mtmp->mhp >= 10)
/*JP
                        ? "complains about unpleasant dungeon conditions."
*/
                        ? "不愉快な迷宮の状態について不満を述べた．"
/*JP
                        : "asks you about the One Ring.";
*/
                        : "「一つの指輪」について尋ねた．";
                break;
            case PM_ARCHEOLOGIST:
                pline_msg =
/*JP
                "describes a recent article in \"Spelunker Today\" magazine.";
*/
                "「日刊洞窟」の最新の記事を執筆している．";
                break;
            case PM_TOURIST:
/*JP
                verbl_msg = "Aloha.";
*/
                verbl_msg = "アローハ．";
                break;
            default:
/*JP
                pline_msg = "discusses dungeon exploration.";
*/
                pline_msg = "迷宮探検について議論した．";
                break;
            }
        break;
    case MS_SEDUCE: {
        int swval;

        if (SYSOPT_SEDUCE) {
            if (ptr->mlet != S_NYMPH
                && could_seduce(mtmp, &youmonst, (struct attack *) 0) == 1) {
                (void) doseduce(mtmp);
                break;
            }
            swval = ((poly_gender() != (int) mtmp->female) ? rn2(3) : 0);
        } else
            swval = ((poly_gender() == 0) ? rn2(3) : 0);
        switch (swval) {
        case 2:
#if 0 /*JP*/
            verbl_msg = "Hello, sailor.";
#else
            switch (poly_gender()) {
              case 0:
                verbl_msg = "こんにちは．あらいい男ね．";
                break;
              case 1:
                verbl_msg = "こんにちは，お嬢さん．";
                break;
              default:
                verbl_msg = "こんにちは．";
                break;
            }
#endif
            break;
        case 1:
/*JP
            pline_msg = "comes on to you.";
*/
            pline_msg = "あなたのほうへやってきた．";
            break;
        default:
/*JP
            pline_msg = "cajoles you.";
*/
            pline_msg = "あなたをおだてた．";
        }
    } break;
    case MS_ARREST:
        if (mtmp->mpeaceful)
#if 0 /*JP*/
            verbalize("Just the facts, %s.", flags.female ? "Ma'am" : "Sir");
#else
            verbalize("事実だけが知りたいんですよ，%s．", flags.female ? "奥さん" : "旦那");
#endif
        else {
            static const char *const arrest_msg[3] = {
#if 0 /*JP*/
                "Anything you say can be used against you.",
                "You're under arrest!", "Stop in the name of the Law!",
#else
                "おまえの言うことはおまえにとって不利な証拠となることがある！",
                "おまえを逮捕する！",
                "法の名のもと直ちに中止せよ！",
#endif
            };
            verbl_msg = arrest_msg[rn2(3)];
        }
        break;
    case MS_BRIBE:
        if (mtmp->mpeaceful && !mtmp->mtame) {
            (void) demon_talk(mtmp);
            break;
        }
    /* fall through */
    case MS_CUSS:
        if (!mtmp->mpeaceful)
            cuss(mtmp);
        else if (is_lminion(mtmp))
/*JP
            verbl_msg = "It's not too late.";
*/
            verbl_msg = "まだ遅くはない．";
        else
/*JP
            verbl_msg = "We're all doomed.";
*/
            verbl_msg = "みんなもうおしまいだ．";
        break;
    case MS_SPELL:
        /* deliberately vague, since it's not actually casting any spell */
/*JP
        pline_msg = "seems to mutter a cantrip.";
*/
        pline_msg = "ぶつぶつとつぶやいている．";
        break;
    case MS_NURSE:
/*JP
        verbl_msg_mcan = "I hate this job!";
*/
        verbl_msg_mcan = "こんな仕事嫌い！";
        if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
/*JP
            verbl_msg = "Put that weapon away before you hurt someone!";
*/
            verbl_msg = "武器をおさめなさい！それは人を傷つけるものよ！";
        else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
            verbl_msg = Role_if(PM_HEALER)
/*JP
                            ? "Doc, I can't help you unless you cooperate."
*/
                            ? "先生，あなたの協力なしではどうしようもありませんわ．"
/*JP
                            : "Please undress so I can examine you.";
*/
                            : "服を脱いでください．あなたを診察しますわ．";
        else if (uarmu)
/*JP
            verbl_msg = "Take off your shirt, please.";
*/
            verbl_msg = "シャツを脱いでください．";
        else
/*JP
            verbl_msg = "Relax, this won't hurt a bit.";
*/
            verbl_msg = "おちついて．ちっとも痛くないわよ．";
        break;
    case MS_GUARD:
        if (money_cnt(invent))
/*JP
            verbl_msg = "Please drop that gold and follow me.";
*/
            verbl_msg = "金を置いてついてきてください．";
        else
/*JP
            verbl_msg = "Please follow me.";
*/
            verbl_msg = "ついてきてください．";
        break;
    case MS_SOLDIER: {
        static const char
            *const soldier_foe_msg[3] =
                {
#if 0 /*JP*/
                  "Resistance is useless!", "You're dog meat!", "Surrender!",
#else
                    "抵抗しても無駄だ！",
                    "犬に喰われちまえ！",
                    "降伏しろ！",
#endif
                },
                   *const soldier_pax_msg[3] = {
#if 0 /*JP*/
                       "What lousy pay we're getting here!",
                       "The food's not fit for Orcs!",
                       "My feet hurt, I've been on them all day!",
#else
                       "なんて金払いが悪いんだ！",
                       "なんだい？オークでも喰えねぇぜ！",
                       "足を怪我した，ずっとこの調子だ！",
#endif
                   };
        verbl_msg = mtmp->mpeaceful ? soldier_pax_msg[rn2(3)]
                                    : soldier_foe_msg[rn2(3)];
        break;
    }
    case MS_RIDER: {
        const char *tribtitle;
        struct obj *book = 0;
        boolean ms_Death = (ptr == &mons[PM_DEATH]);

        /* 3.6 tribute */
        if (ms_Death && !context.tribute.Deathnotice
            && (book = u_have_novel()) != 0) {
            if ((tribtitle = noveltitle(&book->novelidx)) != 0) {
/*JP
                Sprintf(verbuf, "Ah, so you have a copy of /%s/.", tribtitle);
*/
                Sprintf(verbuf, "ああ、/%s/を持っているのか．", tribtitle);
                /* no Death featured in these two, so exclude them */
                if (strcmpi(tribtitle, "Snuff")
                    && strcmpi(tribtitle, "The Wee Free Men"))
/*JP
                    Strcat(verbuf, "  I may have been misquoted there.");
*/
                    Strcat(verbuf, "  ここでは引用が間違っているかもしれない．");
                verbl_msg = verbuf;
            }
            context.tribute.Deathnotice = 1;
        } else if (ms_Death && rn2(3) && Death_quote(verbuf, sizeof verbuf)) {
            verbl_msg = verbuf;
        /* end of tribute addition */

        } else if (ms_Death && !rn2(10)) {
/*JP
            pline_msg = "is busy reading a copy of Sandman #8.";
*/
            pline_msg = "「サンドマン」の8巻を読むのに忙しい．";
        } else
/*JP
            verbl_msg = "Who do you think you are, War?";
*/
            verbl_msg = "ウォーよ，自分が何者か考えたことがあるか？";
        break;
    } /* case MS_RIDER */
    } /* switch */

    if (pline_msg) {
/*JP
        pline("%s %s", Monnam(mtmp), pline_msg);
*/
        pline("%sは%s", Monnam(mtmp), pline_msg);
    } else if (mtmp->mcan && verbl_msg_mcan) {
        verbalize1(verbl_msg_mcan);
    } else if (verbl_msg) {
#if 0 /*JP*//*デスを特別扱いしない*/
        /* more 3.6 tribute */
        if (ptr == &mons[PM_DEATH]) {
            /* Death talks in CAPITAL LETTERS
               and without quotation marks */
            char tmpbuf[BUFSZ];

            pline1(ucase(strcpy(tmpbuf, verbl_msg)));
        } else {
            verbalize1(verbl_msg);
        }
#else
        verbalize1(verbl_msg);
#endif
    }
    return 1;
}

/* #chat command */
int
dotalk()
{
    int result;

    result = dochat();
    return result;
}

STATIC_OVL int
dochat()
{
    struct monst *mtmp;
    int tx, ty;
    struct obj *otmp;

    if (is_silent(youmonst.data)) {
/*JP
        pline("As %s, you cannot speak.", an(youmonst.data->mname));
*/
        pline("あなたは%sなので，話すことができない．", youmonst.data->mname);
        return 0;
    }
    if (Strangled) {
/*JP
        You_cant("speak.  You're choking!");
*/
        You("話せない．あなたは首を絞められている！");
        return 0;
    }
    if (u.uswallow) {
/*JP
        pline("They won't hear you out there.");
*/
        You("外へ向って話をしたが，誰も聞きいれなかった．");
        return 0;
    }
    if (Underwater) {
/*JP
        Your("speech is unintelligible underwater.");
*/
        pline("水面下では，あなたの話はろくに理解されない．");
        return 0;
    }
    if (Deaf) {
/*JP
        pline("How can you hold a conversation when you cannot hear?");
*/
        pline("聞くことができないのにどうやって会話するの？");
        return 0;
    }

    if (!Blind && (otmp = shop_object(u.ux, u.uy)) != (struct obj *) 0) {
        /* standing on something in a shop and chatting causes the shopkeeper
           to describe the price(s).  This can inhibit other chatting inside
           a shop, but that shouldn't matter much.  shop_object() returns an
           object iff inside a shop and the shopkeeper is present and willing
           (not angry) and able (not asleep) to speak and the position
           contains any objects other than just gold.
        */
        price_quote(otmp);
        return 1;
    }

/*JP
    if (!getdir("Talk to whom? (in what direction)")) {
*/
    if (!getdir("誰と話しますか？[方向を入れてね]")) {
        /* decided not to chat */
        return 0;
    }

    if (u.usteed && u.dz > 0) {
        if (!u.usteed->mcanmove || u.usteed->msleeping) {
/*JP
            pline("%s seems not to notice you.", Monnam(u.usteed));
*/
            pline("%sはあなたに気がついていないようだ．", Monnam(u.usteed));
            return 1;
        } else
            return domonnoise(u.usteed);
    }

    if (u.dz) {
/*JP
        pline("They won't hear you %s there.", u.dz < 0 ? "up" : "down");
*/
        pline("%s向かって話をしても意味がない．", u.dz < 0 ? "上に" : "下に");
        return 0;
    }

    if (u.dx == 0 && u.dy == 0) {
        /*
         * Let's not include this.
         * It raises all sorts of questions: can you wear
         * 2 helmets, 2 amulets, 3 pairs of gloves or 6 rings as a marilith,
         * etc...  --KAA
        if (u.umonnum == PM_ETTIN) {
            You("discover that your other head makes boring conversation.");
            return 1;
        }
         */
/*JP
        pline("Talking to yourself is a bad habit for a dungeoneer.");
*/
        pline("一人言は迷宮探検者の悪い癖だ．");
        return 0;
    }

    tx = u.ux + u.dx;
    ty = u.uy + u.dy;

    if (!isok(tx, ty))
        return 0;

    mtmp = m_at(tx, ty);

    if ((!mtmp || mtmp->mundetected)
        && (otmp = vobj_at(tx, ty)) != 0 && otmp->otyp == STATUE) {
        /* Talking to a statue */
        if (!Blind) {
#if 0 /*JP*/
            pline_The("%s seems not to notice you.",
                      /* if hallucinating, you can't tell it's a statue */
                      Hallucination ? rndmonnam((char *) 0) : "statue");
#else
            pline_The("%sはあなたに気がついていないようだ．",
                      Hallucination ? rndmonnam((char *) 0) : "石像");
#endif
        }
        return 0;
    }

    if (!mtmp || mtmp->mundetected || M_AP_TYPE(mtmp) == M_AP_FURNITURE
        || M_AP_TYPE(mtmp) == M_AP_OBJECT)
        return 0;

    /* sleeping monsters won't talk, except priests (who wake up) */
    if ((!mtmp->mcanmove || mtmp->msleeping) && !mtmp->ispriest) {
        /* If it is unseen, the player can't tell the difference between
           not noticing him and just not existing, so skip the message. */
        if (canspotmon(mtmp))
/*JP
            pline("%s seems not to notice you.", Monnam(mtmp));
*/
            pline("%sはあなたに気がついていないようだ．", Monnam(mtmp));
        return 0;
    }

    /* if this monster is waiting for something, prod it into action */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (mtmp->mtame && mtmp->meating) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
/*JP
        pline("%s is eating noisily.", Monnam(mtmp));
*/
        pline("%sはバリバリと物を食べている．", Monnam(mtmp));
        return 0;
    }

    return domonnoise(mtmp);
}

#ifdef USER_SOUNDS

extern void FDECL(play_usersound, (const char *, int));

typedef struct audio_mapping_rec {
    struct nhregex *regex;
    char *filename;
    int volume;
    struct audio_mapping_rec *next;
} audio_mapping;

static audio_mapping *soundmap = 0;

char *sounddir = ".";

/* adds a sound file mapping, returns 0 on failure, 1 on success */
int
add_sound_mapping(mapping)
const char *mapping;
{
    char text[256];
    char filename[256];
    char filespec[256];
    int volume;

    if (sscanf(mapping, "MESG \"%255[^\"]\"%*[\t ]\"%255[^\"]\" %d", text,
               filename, &volume) == 3) {
        audio_mapping *new_map;

        if (strlen(sounddir) + strlen(filename) > 254) {
            raw_print("sound file name too long");
            return 0;
        }
        Sprintf(filespec, "%s/%s", sounddir, filename);

        if (can_read_file(filespec)) {
            new_map = (audio_mapping *) alloc(sizeof(audio_mapping));
            new_map->regex = regex_init();
            new_map->filename = dupstr(filespec);
            new_map->volume = volume;
            new_map->next = soundmap;

            if (!regex_compile(text, new_map->regex)) {
                raw_print(regex_error_desc(new_map->regex));
                regex_free(new_map->regex);
                free(new_map->filename);
                free(new_map);
                return 0;
            } else {
                soundmap = new_map;
            }
        } else {
            Sprintf(text, "cannot read %.243s", filespec);
            raw_print(text);
            return 0;
        }
    } else {
        raw_print("syntax error in SOUND");
        return 0;
    }

    return 1;
}

void
play_sound_for_message(msg)
const char *msg;
{
    audio_mapping *cursor = soundmap;

    while (cursor) {
        if (regex_match(msg, cursor->regex)) {
            play_usersound(cursor->filename, cursor->volume);
        }
        cursor = cursor->next;
    }
}

#endif /* USER_SOUNDS */

/*sounds.c*/
