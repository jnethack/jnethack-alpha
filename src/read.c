/* NetHack 5.0	read.c	$NHDT-Date: 1762577372 2025/11/07 20:49:32 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.323 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

/* JNetHack Copyright */
/* (c) Issei Numata, Naoki Hamada, Shigehiro Miyashita, 1994-2000  */
/* For 3.4-, Copyright (c) SHIRAKATA Kentaro, 2002-                */
/* JNetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#define Your_Own_Role(mndx)  ((mndx) == gu.urole.mnum)
#define Your_Own_Race(mndx)  ((mndx) == gu.urace.mnum)

staticfn boolean learnscrolltyp(short);
staticfn void cap_spe(struct obj *);
staticfn char *erode_obj_text(struct obj *, char *);
staticfn char *hawaiian_design(struct obj *, char *);
staticfn int read_ok(struct obj *);
staticfn void stripspe(struct obj *);
staticfn void p_glow1(struct obj *);
staticfn void p_glow2(struct obj *, const char *);
staticfn void p_glow3(struct obj *, const char *);
staticfn void forget(int);
staticfn int maybe_tame(struct monst *, struct obj *);
staticfn boolean can_center_cloud(coordxy, coordxy);
staticfn void display_stinking_cloud_positions(boolean);
staticfn void seffect_enchant_armor(struct obj **);
staticfn boolean disintegrate_cursed_armor(void);
staticfn void seffect_destroy_armor(struct obj **);
staticfn void seffect_confuse_monster(struct obj **);
staticfn void seffect_scare_monster(struct obj **);
staticfn void seffect_remove_curse(struct obj **);
staticfn void seffect_create_monster(struct obj **);
staticfn void seffect_enchant_weapon(struct obj **);
staticfn void seffect_taming(struct obj **);
staticfn void seffect_genocide(struct obj **);
staticfn void seffect_light(struct obj **);
staticfn void seffect_charging(struct obj **);
staticfn void seffect_amnesia(struct obj **);
staticfn void seffect_fire(struct obj **);
staticfn void seffect_earth(struct obj **);
staticfn void seffect_punishment(struct obj **);
staticfn void seffect_stinking_cloud(struct obj **);
staticfn void seffect_blank_paper(struct obj **);
staticfn void seffect_teleportation(struct obj **);
staticfn void seffect_gold_detection(struct obj **);
staticfn void seffect_food_detection(struct obj **);
staticfn void seffect_identify(struct obj **);
staticfn void seffect_magic_mapping(struct obj **);
#ifdef MAIL_STRUCTURES
staticfn void seffect_mail(struct obj **);
#endif /* MAIL_STRUCTURES */
staticfn void set_lit(coordxy, coordxy, genericptr);
staticfn void do_class_genocide(void);
staticfn void do_stinking_cloud(struct obj *, boolean);
staticfn boolean create_particular_parse(char *,
                                       struct _create_particular_data *);
staticfn boolean create_particular_creation(struct _create_particular_data *);

staticfn boolean
learnscrolltyp(short scrolltyp)
{
    if (!objects[scrolltyp].oc_name_known) {
        makeknown(scrolltyp);
        more_experienced(0, 10);
        return TRUE;
    } else
        return FALSE;
}

/* also called from teleport.c for scroll of teleportation */
void
learnscroll(struct obj *sobj)
{
    /* it's implied that sobj->dknown is set;
       we couldn't be reading this scroll otherwise */
    if (sobj->oclass != SPBOOK_CLASS)
        (void) learnscrolltyp(sobj->otyp);
}

/* max spe is +99, min is -99 */
staticfn void
cap_spe(struct obj *obj)
{
    if (obj) {
        if (abs(obj->spe) > SPE_LIM)
            obj->spe = sgn(obj->spe) * SPE_LIM;
    }
}

staticfn char *
erode_obj_text(struct obj *otmp, char *buf)
{
    int erosion = greatest_erosion(otmp);

    if (erosion)
        wipeout_text(buf, (int) (strlen(buf) * erosion / (2 * MAX_ERODE)),
                     otmp->o_id ^ (unsigned) ubirthday);
    return buf;
}

char *
tshirt_text(struct obj *tshirt, char *buf)
{
    static const char *const shirt_msgs[] = {
        /* Scott Bigham */
#if 0 /*JP:T*/
      "I explored the Dungeons of Doom and all I got was this lousy T-shirt!",
        "Is that Mjollnir in your pocket or are you just happy to see me?",
      "It's not the size of your sword, it's how #enhance'd you are with it.",
        "Madame Elvira's House O' Succubi Lifetime Customer",
        "Madame Elvira's House O' Succubi Employee of the Month",
        "Ludios Vault Guards Do It In Small, Dark Rooms",
        "Yendor Military Soldiers Do It In Large Groups",
        "I survived Yendor Military Boot Camp",
        "Ludios Accounting School Intra-Mural Lacrosse Team",
        "Oracle(TM) Fountains 10th Annual Wet T-Shirt Contest",
        "Hey, black dragon!  Disintegrate THIS!",
        "I'm With Stupid -->",
        "Don't blame me, I voted for Izchak!",
        "Don't Panic", /* HHGTTG */
        "Furinkan High School Athletic Dept.",                /* Ranma 1/2 */
        "Hel-LOOO, Nurse!",                                   /* Animaniacs */
#else
        "私は運命の迷宮を調査していたが，手に入れたのはきたないＴシャツだけだった！",
        "ポケットにミュルニールが入っているの？それとも私に会えてうれしい？",
        "剣の大きさが問題なのではない．それが如何に#enhanceされているかなのだ．",
        "マダム・エルバイラのサキュバス館 永久顧客",
        "マダム・エルバイラのサキュバス館 今月の最優秀功労者",
        "ローディオス金庫の番人，それは暗く小さい部屋の中",
        "イェンダー軍兵士，それは巨大な団体の中",
        "私はイェンダー軍の新兵訓練所を乗り越えました",
        "ローディオス会計学校 室内ラクロスチーム",
        "Oracle(TM) の泉 第１０回濡れＴシャツコンテスト",
        "おい，黒ドラゴン！こいつを分解しろ！",
        "馬鹿と一緒にいます → ",
        "私は悪くない！Izchakに投票したもの！",
        "あ わ て る な", /* 銀河ヒッチハイクガイド */
        "風林館高校陸上部",                                   /* Ranma 1/2 */
        "Ｈｅｌ−ＬＯＯＯ，Ｎｕｒｓｅ！",                     /* Animaniacs */
#endif
        "=^.^=",
#if 0 /*JP:T*/
        "100% goblin hair - do not wash",
        "Aberzombie and Fitch",
        "cK -- Cockatrice touches the Kop",
        "Don't ask me, I only adventure here",
        "Down with pants!",
        "d, your dog or a killer?",
        "FREE PUG AND NEWT!",
        "Go team ant!",
        "Got newt?",
        "Hello, my darlings!", /* Charlie Drake */
        "Hey!  Nymphs!  Steal This T-Shirt!",
        "I <3 Dungeon of Doom",
        "I <3 Maud",
        /* note: there is a similarly worded apron (alchemy smock) slogan */
        "I am a Valkyrie.  If you see me running, try to keep up.",
        "I am not a pack rat - I am a collector",
        "I bounced off a rubber tree",         /* Monkey Island */
        "Plunder Island Brimstone Beach Club", /* Monkey Island */
#else
        "ゴブリン毛100% - 洗えません",
        "アバゾンビ&フィッチ",
        "cK -- コカトリスが警官にタッチ",
        "質問しないで; 私はここを探検しているだけ",
        "ズボンなんていらない！",
        "dはあなたの犬か殺人者か？",
        "FREE PUG AND NEWT!",
        "Go team ant!",
        "Got newt?",
        "こんにちはダーリン！", /* Charlie Drake */
        "やあ！ニンフ！このＴシャツを盗んで！",
        "I <3 Dungeon of Doom",
        "I <3 Maud",
        /* note: there is a similarly worded apron (alchemy smock) slogan */
        "私はバルキリーだ．私が走っているのを見たら，ついてこい．",
        "私はゴミ集めではない．私は収集家だ",
        "私はゴムの木に跳ね返った",         /* Monkey Island */
        "略奪島硫黄海岸クラブ", /* Monkey Island */
#endif
#if 0 /*JP:T*/
        "If you can read this, I can hit you with my polearm",
        "I'm confused!",
        "I scored with the princess",
        "I want to live forever or die in the attempt.",
        "Lichen Park",
        "LOST IN THOUGHT - please send search party",
        "Meat is Mordor",
        "Minetown Better Business Bureau",
        "Minetown Watch",
        /* Discworld riff; unfortunately long */
        ("Ms. Palm's House of Negotiable Affection--A Very Reputable"
            " House Of Disrepute"),
        "Protection Racketeer",
        "Real men love Crom",
        "Somebody stole my Mojo!",
        "The Hellhound Gang",
        "The Werewolves",
#else
        "これが読めるなら、私の長斧が届くということだ",
        "私は混乱しています！",
        "I scored with the princess",
        "私は永遠に生きるか、そのために死にたい．",
        "Lichen Park",
        "考え込んでいます - 捜索隊を出してください",
        "肉はモルドール",
        "鉱山街商業改善協会",
        "鉱山街の見張り",
        /* Discworld riff; unfortunately long */
        "パーム女史の交渉技術の家 -- とても評判の悪評の家",
        "鉱山ダッシュ中",
        "本物の男はクロムを愛する",
        "誰かが俺のアレを盗んだ！",
        "The Hellhound Gang",
        "The Werewolves",
#endif
#if 0 /*JP:T*/
        "They Might Be Storm Giants",
        "Weapons don't kill people, I kill people",
        "White Zombie",
        "You're killing me!",
        "Anhur State University - Home of the Fighting Fire Ants!",
        "FREE HUGS",
        "Serial Ascender",
        "Real men are valkyries",
        "Young Men's Cavedigging Association",
        "Occupy Fort Ludios",
        "I couldn't afford this T-shirt so I stole it!",
        "Mind flayers suck",
        "I'm not wearing any pants",
        "Down with the living!",
        "Pudding farmer",
        "Vegetarian",
        "Hello, I'm War!",
        "It is better to light a candle than to curse the darkness",
        "It is easier to curse the darkness than to light a candle",
#else
        "ゼイ・マイト・ビー・ストーム・ジャイアンツ",
        "武器が人を殺すのではない，私が人を殺すのだ",
        "White Zombie",
        "いい加減にして！",
        "アンフル州立大学 - 火蟻との戦いの本拠地！",
        "FREE HUGS",
        "特別昇天者",
        "本当の男はバルキリーだ",
        "青年洞窟掘削連盟",
        "ローディオス砦を占拠せよ",
        "このＴシャツを買うお金がなかったのでこれは盗んだ！",
        "マインドフレヤ氏ね",
        "私はパンツを穿いていません",
        "生活をぶっ壊せ！",
        "プリン農家",
        "ベジタリアン",
        "やあ，私が『戦争』だ！",
        "暗いと不平を言うよりも，すすんであかりをつけましょう",
        "すすんであかりをつけるより，暗いと不平を言う方が簡単",
#endif
        /* expanded "rock--paper--scissors" featured in TV show "Big Bang
           Theory" although they didn't create it (and an actual T-shirt
           with pentagonal diagram showing which choices defeat which) */
/*JP
        "rock--paper--scissors--lizard--Spock!",
*/
        "岩--紙--ハサミ--トカゲ--スポック！",
        /* "All men must die -- all men must serve" challenge and response
           from book series _A_Song_of_Ice_and_Fire_ by George R.R. Martin,
           TV show "Game of Thrones" (probably an actual T-shirt too...) */
        "/Valar morghulis/ -- /Valar dohaeris/",
    };

    Strcpy(buf, shirt_msgs[tshirt->o_id % SIZE(shirt_msgs)]);
    return erode_obj_text(tshirt, buf);
}

char *
hawaiian_motif(struct obj *shirt, char *buf)
{
    static const char *const hawaiian_motifs[] = {
        /* birds */
        "flamingo",
        "parrot",
        "toucan",
        "bird of paradise", /* could be a bird or a flower */
        /* sea creatures */
        "sea turtle",
        "tropical fish",
        "jellyfish",
        "giant eel",
        "water nymph",
        /* plants */
        "plumeria",
        "orchid",
        "hibiscus flower",
        "palm tree",
        /* other */
        "hula dancer",
        "sailboat",
        "ukulele",
    };

    /* a tourist's starting shirt always has the same o_id; we need some
       additional randomness or else its design will never differ */
    unsigned motif = shirt->o_id ^ (unsigned) ubirthday;

    Strcpy(buf, hawaiian_motifs[motif % SIZE(hawaiian_motifs)]);
    return buf;
}

staticfn char *
hawaiian_design(struct obj *shirt, char *buf)
{
    static const char *const hawaiian_bgs[] = {
        /* solid colors */
        "purple",
        "yellow",
        "red",
        "blue",
        "orange",
        "black",
        "green",
        /* adjectives */
        "abstract",
        "geometric",
        "patterned",
        "naturalistic",
    };

    /* This hash method is slightly different than the one in hawaiian_motif;
       using the same formula in both cases may lead to some shirt combos
       never appearing, if the sizes of the two lists have common factors. */
    unsigned bg = shirt->o_id ^ (unsigned) ~ubirthday;

    Sprintf(buf, "%s on %s background",
            makeplural(hawaiian_motif(shirt, buf)),
            an(hawaiian_bgs[bg % SIZE(hawaiian_bgs)]));
    return buf;
}

char *
apron_text(struct obj *apron, char *buf)
{
    static const char *const apron_msgs[] = {
/*JP
        "Kiss the cook",
*/
        "コックにキスせよ",
/*JP
        "I'm making SCIENCE!",
*/
        "私は*科学*を行っている！",
/*JP
        "Don't mess with the chef",
*/
        "シェフには手を出すな",
/*JP
        "Don't make me poison you",
*/
        "あなたに毒を盛らさせないで",
/*JP
        "Gehennom's Kitchen",
*/
        "ゲヘナキッチン",
/*JP
        "Rat: The other white meat",
*/
        "ネズミ: もう一つの白身肉",
/*JP
        "If you can't stand the heat, get out of Gehennom!",
*/
        "熱に耐えられないなら、ゲヘナから出て行け！",
/*JP
        "If we weren't meant to eat animals, why are they made out of meat?",
*/
        "もし我々が肉を食べる運命ではないのなら，なぜ動物には肉があるの？",
/*JP
        "If you don't like the food, I'll stab you",
*/
        "食べ物が気に入らないなら，刺すよ",
        /* In the movie "The Sum of All Fears", a Russian worker in a weapons
           facility wears a T-shirt that a translator says reads, "I am a
           bomb technician, if you see me running ... try to catch up."
           In nethack, the quote is far more suitable to an alchemy smock
           (particularly since so many of these others are about cooking)
           than a T-shirt and is paraphrased to simplify/shorten it.
           [later... turns out that this is already a T-shirt message:
            "I am a Valkyrie.  If you see me running, try to keep up."
           so this one has been revised a little:  added alchemist prefix,
           changed "keep up" to original source's "catch up"] */
        "I am an alchemist; if you see me running, try to catch up...",
    };

    Strcpy(buf, apron_msgs[apron->o_id % SIZE(apron_msgs)]);
    return erode_obj_text(apron, buf);
}

static const char *const candy_wrappers[] = {
    "",                         /* (none -- should never happen) */
    "Apollo",                   /* Lost */
    "Moon Crunchy",             /* South Park */
    "Snacky Cake",    "Chocolate Nuggie", "The Small Bar",
    "Crispy Yum Yum", "Nilla Crunchie",   "Berry Bar",
    "Choco Nummer",   "Om-nom", /* Cat Macro */
    "Fruity Oaty",              /* Serenity */
    "Wonka Bar",                /* Charlie and the Chocolate Factory */
};

/* return the text of a candy bar's wrapper */
const char *
candy_wrapper_text(struct obj *obj)
{
    /* modulo operation is just bullet proofing; 'spe' is already in range */
    return candy_wrappers[obj->spe % SIZE(candy_wrappers)];
}

/* assign a wrapper to a candy bar stack */
void
assign_candy_wrapper(struct obj *obj)
{
    if (obj->otyp == CANDY_BAR) {
        /* skips candy_wrappers[0] */
        obj->spe = 1 + rn2(SIZE(candy_wrappers) - 1);
    }
    return;
}

/* getobj callback for object to read */
staticfn int
read_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_EXCLUDE;

    if (obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS)
        return GETOBJ_SUGGEST;

    return GETOBJ_DOWNPLAY;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* the #read command; read a scroll or spell book or various other things */
int
doread(void)
{
/*JP
    static const char find_any_braille[] = "feel any Braille writing.";
*/
    static const char find_any_braille[] = "点字はどうも書いてないようだ．";
    struct obj *scroll;
    boolean confused, nodisappear;
    int otyp;

    /*
     * Reading while blind is allowed in most cases, including the
     * Book of the Dead but not regular spellbooks.  For scrolls, the
     * description has to have been seen or magically learned (so only
     * when scroll->dknown is true):  hero recites the label while
     * holding the unfurled scroll.  We deliberately don't require
     * free hands because that would cripple scroll of remove curse,
     * but we ought to be requiring hands or at least limbs.  The
     * recitation could be sub-vocal; actual speech isn't required.
     *
     * Reading while confused is allowed and can produce alternate
     * outcome.
     *
     * Reading while stunned is currently allowed but probably should
     * be prevented....
     */

    gk.known = FALSE;
    if (check_capacity((char *) 0))
        return ECMD_OK;

    scroll = getobj("read", read_ok, GETOBJ_PROMPT);
    if (!scroll)
        return ECMD_CANCEL;
    otyp = scroll->otyp;
    scroll->pickup_prev = 0; /* no longer 'just picked up' */

    /* outrumor has its own blindness check */
    if (otyp == FORTUNE_COOKIE) {
        if (flags.verbose)
/*JP
            You("break up the cookie and throw away the pieces.");
*/
            You("クッキーを割り，かけらを投げすてた．");
        outrumor(bcsign(scroll), BY_COOKIE);
        if (!Blind)
            if (!u.uconduct.literate++)
                livelog_printf(LL_CONDUCT,
                               "became literate by reading a fortune cookie");
        useup(scroll);
        return ECMD_TIME;
    } else if (otyp == T_SHIRT || otyp == ALCHEMY_SMOCK
               || otyp == HAWAIIAN_SHIRT) {
        char buf[BUFSZ], *mesg;
#if 0 /*JP*/
        const char *endpunct;
#endif

        if (Blind) {
            You_cant(find_any_braille);
            return ECMD_OK;
        }
        /* can't read shirt worn under suit (under cloak is ok though) */
        if ((otyp == T_SHIRT || otyp == HAWAIIAN_SHIRT) && uarm
            && scroll == uarmu) {
#if 0 /*JP*/
            pline("%s shirt is obscured by %s%s.",
                  scroll->unpaid ? "That" : "Your", shk_your(buf, uarm),
                  suit_simple_name(uarm));
#else
            pline("シャツは%sで隠されている．",
                  suit_simple_name(uarm));
#endif
            return ECMD_OK;
        }
        if (otyp == HAWAIIAN_SHIRT) {
            pline("%s features %s.", flags.verbose ? "The design" : "It",
                  hawaiian_design(scroll, buf));
            return ECMD_TIME;
        }
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT, "became literate by reading %s",
                           (scroll->otyp == T_SHIRT) ? "a T-shirt"
                           : "an apron");

        /* populate 'buf[]' */
        mesg = (otyp == T_SHIRT) ? tshirt_text(scroll, buf)
                                 : apron_text(scroll, buf);
#if 0 /*JP*/
        endpunct = "";
#endif
        if (flags.verbose) {
#if 0 /*JP*/
            int ln = (int) strlen(mesg);

            /* we will be displaying a sentence; need ending punctuation */
            if (ln > 0 && !strchr(".!?", mesg[ln - 1]))
                endpunct = ".";
#endif
/*JP
            pline("It reads:");
*/
            pline("それを読んだ：");
        }
#if 0 /*JP*/
        pline("\"%s\"%s", mesg, endpunct);
#else
        pline("「%s」", mesg);
#endif
        return ECMD_TIME;
    } else if ((otyp == DUNCE_CAP || otyp == CORNUTHAUM)
        /* note: "DUNCE" isn't directly connected to tourists but
           if everyone could read it, they would always be able to
           trivially distinguish between the two types of conical hat;
           limiting this to tourists is better than rejecting it */
               && Role_if(PM_TOURIST)) {
        /* another note: the misspelling, "wizzard", is correct;
           that's what is written on Rincewind's pointy hat from
           Pratchett's Discworld series, along with a lot of stars;
           rather than inked on or painted on, treat them as stitched
           or even separate pieces of fabric which have been attached
           (don't recall whether the books mention anything like that...) */
        const char *cap_text = (otyp == DUNCE_CAP) ? "DUNCE" : "WIZZARD";

        if (scroll->o_id % 3) {
            /* no need to vary this when blind; "on this ___" is important
               because it suggests that there might be something on others */
            You_cant("find anything to read on this %s.",
                     simpleonames(scroll));
            return ECMD_OK;
        }
        pline("%s on the %s.  It reads:  %s.",
              !Blind ? "There is writing" : "You feel lettering",
              simpleonames(scroll), cap_text);
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT, "became literate by reading %s",
                           (otyp == DUNCE_CAP) ? "a dunce cap"
                                               : "a cornuthaum");

        /* yet another note: despite the fact that player will recognize
           the object type, don't make it become a discovery for hero */
        trycall(scroll);
        return ECMD_TIME;
    } else if (otyp == CREDIT_CARD) {
        static const char *const card_msgs[] = {
#if 0 /*JP:T*/
            "Leprechaun Gold Tru$t - Shamrock Card",
            "Magic Memory Vault Charge Card",
            "Larn National Bank",                /* Larn */
            "First Bank of Omega",               /* Omega */
            "Bank of Zork - Frobozz Magic Card", /* Zork */
            "Ankh-Morpork Merchant's Guild Barter Card",
            "Ankh-Morpork Thieves' Guild Unlimited Transaction Card",
            "Ransmannsby Moneylenders Association",
            "Bank of Gehennom - 99% Interest Card",
            "Yendorian Express - Copper Card",
            "Yendorian Express - Silver Card",
            "Yendorian Express - Gold Card",
            "Yendorian Express - Mithril Card",
            "Yendorian Express - Platinum Card", /* must be last */
#else
            "レプラコーン信用金庫 - クローバーカード",
            "魔法記念倉庫クレジットカード", "ラーン国営銀行", /* Larn */
            "オメガ第一銀行",               /* Omega */
            "ゾーク銀行 - Bank of Zork - フロボズ魔法カード", /* Zork */
            "アンクモルポーク商人ギルド 交易カード",
            "アンクモルポーク盗賊ギルド 無制限取引カード",
            "ランスマンスベイ貸金業協会",
            "ゲヘナ銀行 - 利子99%カード",
            "イェンダー印エキスプレスカード - カッパーカード",
            "イェンダー印エキスプレスカード - シルバーカード",
            "イェンダー印エキスプレスカード - ゴールドカード",
            "イェンダー印エキスプレスカード - ミスリルカード",
            "イェンダー印エキスプレスカード - プラチナカード", /* must be last */
#endif
        };

        if (Blind) {
/*JP
            You("feel the embossed numbers:");
*/
            You("浮き彫りにされている番号を感じた:");
        } else {
            if (flags.verbose)
/*JP
                pline("It reads:");
*/
                pline("それを読んだ：");
/*JP
            pline("\"%s\"",
*/
            pline("「%s」",
                  scroll->oartifact
                      ? card_msgs[SIZE(card_msgs) - 1]
                      : card_msgs[scroll->o_id % (SIZE(card_msgs) - 1)]);
        }
        /* Make a credit card number */
        pline("\"%d0%d %ld%d1 0%d%d0\"%s",
              (((int) scroll->o_id % 89) + 10),
              ((int) scroll->o_id % 4),
              ((((long) scroll->o_id * 499L) % 899999L) + 100000L),
              ((int) scroll->o_id % 10),
              (!((int) scroll->o_id % 3)),
              (((int) scroll->o_id * 7) % 10),
              (flags.verbose || Blind) ? "." : "");
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT,
                           "became literate by reading a credit card");

        return ECMD_TIME;
    } else if (otyp == CAN_OF_GREASE) {
/*JP
        pline("This %s has no label.", singular(scroll, xname));
*/
        pline("この%sにはラベルがない．", singular(scroll, xname));
        return ECMD_OK;
    } else if (otyp == MAGIC_MARKER) {
        static const int red_mons[] = {
            PM_FIRE_ANT, PM_PYROLISK, PM_HELL_HOUND, PM_IMP,
            PM_LARGE_MIMIC, PM_LEOCROTTA, PM_SCORPION, PM_XAN,
            PM_GIANT_BAT, PM_WATER_MOCCASIN, PM_FLESH_GOLEM,
            PM_BARBED_DEVIL, PM_MARILITH, PM_PIRANHA
        };
        char buf[BUFSZ];
        struct permonst *pm = &mons[red_mons[scroll->o_id % SIZE(red_mons)]];

        if (Blind) {
            You_cant(find_any_braille);
            return ECMD_OK;
        }
        if (flags.verbose)
/*JP
            pline("It reads:");
*/
            pline("それを読んだ：");
        Sprintf(buf, "%s", pmname(pm, NEUTRAL));
#if 0 /*JP:T*/
        pline("\"Magic Marker(TM) %s Red Ink Marker Pen.  Water Soluble.\"",
              upwords(buf));
#else
        pline("「魔法のマーカ(TM) %s赤インクマーカペン．水性．」",
              upwords(buf));
#endif
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT,
                           "became literate by reading a magic marker");

        return ECMD_TIME;
    } else if (scroll->oclass == COIN_CLASS) {
        if (Blind)
/*JP
            You("feel the embossed words:");
*/
            You("浮き彫りにされている文字を感じた:");
        else if (flags.verbose)
/*JP
            You("read:");
*/
            pline("それを読んだ：");
        pline("\"1 Zorkmid.  857 GUE.  In Frobs We Trust.\"");
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT,
                           "became literate by reading a coin's engravings");

        return ECMD_TIME;
    } else if (is_art(scroll, ART_ORB_OF_FATE)) {
        if (Blind)
/*JP
            You("feel the engraved signature:");
*/
            You("彫り込まれているサインを感じた：");
        else
/*JP
            pline("It is signed:");
*/
            pline("サインがある：");
/*JP
        pline("\"Odin.\"");
*/
        pline("「オーディン」");
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT,
                   "became literate by reading the divine signature of Odin");

        return ECMD_TIME;
    } else if (otyp == CANDY_BAR) {
        const char *wrapper = candy_wrapper_text(scroll);

        if (Blind) {
            You_cant(find_any_braille);
            return ECMD_OK;
        }
        if (!*wrapper) {
            pline("The candy bar's wrapper is blank.");
            return ECMD_OK;
        }
#if 0 /*JP:T*/
        pline("The wrapper reads: \"%s\".", wrapper);
#else
        pline("包み紙の文字を読んだ：「%s」", wrapper);
#endif
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT,
                           "became literate by reading a candy bar wrapper");

        return ECMD_TIME;
    } else if (scroll->oclass != SCROLL_CLASS
               && scroll->oclass != SPBOOK_CLASS) {
/*JP
        pline(silly_thing_to, "read");
*/
        pline(silly_thing_to, "読む");
        return ECMD_OK;
    } else if (Blind && otyp != SPE_BOOK_OF_THE_DEAD) {
        const char *what = 0;

        if (otyp == SPE_NOVEL)
            /* unseen novels are already distinguishable from unseen
               spellbooks so this isn't revealing any extra information */
/*JP
            what = "words";
*/
            what = "文字";
        else if (scroll->oclass == SPBOOK_CLASS)
/*JP
            what = "mystic runes";
*/
            what = "神秘的なルーン文字";
        else if (!scroll->dknown)
/*JP
            what = "formula on the scroll";
*/
            what = "巻物の呪文";
        if (what) {
/*JP
            pline("Being blind, you cannot read the %s.", what);
*/
            pline("目が見えないので，あなたは%sを読むことができない．", what);
            return ECMD_OK;
        }
    }

    confused = (Confusion != 0);
#ifdef MAIL_STRUCTURES
    if (otyp == SCR_MAIL) {
        confused = FALSE; /* override */
        /* reading mail is a convenience for the player and takes
           place outside the game, so shouldn't affect gameplay;
           on the other hand, it starts by explicitly making the
           hero actively read something, which is pretty hard
           to simply ignore; as a compromise, if the player has
           maintained illiterate conduct so far, and this mail
           scroll didn't come from bones, ask for confirmation */
        if (!u.uconduct.literate) {
#if 0 /*JP:T*/
            if (!scroll->spe && y_n(
             "Reading mail will violate \"illiterate\" conduct.  Read anyway?"
                                   ) != 'y')
#else
            if (!scroll->spe && y_n(
             "メールを読むと「文字を読まない」挑戦に違反するけど，それでも読む？"
                                   ) != 'y')
#endif
                return ECMD_OK;
        }
    }
#endif

    /* Actions required to win the game aren't counted towards conduct */
    /* Novel conduct is handled in read_tribute so exclude it too */
    if (otyp != SPE_BOOK_OF_THE_DEAD && otyp != SPE_NOVEL
        && otyp != SPE_BLANK_PAPER && otyp != SCR_BLANK_PAPER)
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT, "became literate by reading %s",
                           (scroll->oclass == SPBOOK_CLASS) ? "a book"
                           : (scroll->oclass == SCROLL_CLASS) ? "a scroll"
                             : something);

    if (scroll->oclass == SPBOOK_CLASS) {
        return study_book(scroll) ? ECMD_TIME : ECMD_OK;
    }
    scroll->in_use = TRUE; /* scroll, not spellbook, now being read */
    if (otyp != SCR_BLANK_PAPER) {
        boolean silently = !can_chant(&gy.youmonst);

        /* a few scroll feedback messages describe something happening
           to the scroll itself, so avoid "it disappears" for those */
        nodisappear = (otyp == SCR_FIRE
                       || (otyp == SCR_REMOVE_CURSE && scroll->cursed));
        if (Blind)
#if 0 /*JP:T*/
            pline(nodisappear
                      ? "You %s the formula on the scroll."
                      : "As you %s the formula on it, the scroll disappears.",
                  silently ? "cogitate" : "pronounce");
#else
            pline(nodisappear
                      ? "あなたは呪文を%sた．"
                      : "呪文を%sると，巻物は消えた．",
                  silently ? "念じ" : "唱え");
#endif
        else
#if 0 /*JP:T*/
            pline(nodisappear ? "You read the scroll."
                              : "As you read the scroll, it disappears.");
#else
            pline(nodisappear ? "あなたは巻物を読んだ．"
                              : "巻物を読むと，それは消えた．");
#endif
        if (confused) {
            if (Hallucination)
/*JP
                pline("Being so trippy, you screw up...");
*/
                pline("とてもへろへろなので，くしゃくしゃにしてしまった．．．");
            else
#if 0 /*JP:T*/
                pline("Being confused, you %s the magic words...",
                      silently ? "misunderstand" : "mispronounce");
#else
                pline("混乱しているので，呪文を間違ってしまった．．．");
#endif
        }
    }
    if (!seffects(scroll)) {
        if (!objects[otyp].oc_name_known) {
            if (gk.known)
                learnscroll(scroll);
            else
                trycall(scroll);
        }
        scroll->in_use = FALSE;
        if (otyp != SCR_BLANK_PAPER)
            useup(scroll);
    }
    return ECMD_TIME;
}

RESTORE_WARNING_FORMAT_NONLITERAL

staticfn void
stripspe(struct obj *obj)
{
    if (obj->blessed || obj->spe <= 0) {
        pline1(nothing_happens);
    } else {
        /* order matters: message, shop handling, actual transformation */
/*JP
        pline("%s briefly.", Yobjnam2(obj, "vibrate"));
*/
        Your("%sは小刻みに振動した．",xname(obj));
        costly_alteration(obj, COST_UNCHRG);
        obj->spe = 0;
        if (obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN)
            obj->age = 0;
    }
}

staticfn void
p_glow1(struct obj *otmp)
{
/*JP
    pline("%s briefly.", Yobjnam2(otmp, Blind ? "vibrate" : "glow"));
*/
    Your("%sは小刻みに%s．", xname(otmp), Blind ? "振動した" : "輝いた");
}

staticfn void
p_glow2(struct obj *otmp, const char *color)
{
#if 0 /*JP:T*/
    pline("%s%s%s for a moment.", Yobjnam2(otmp, Blind ? "vibrate" : "glow"),
          Blind ? "" : " ", Blind ? "" : hcolor(color));
#else
    Your("%sは一瞬%s%s．", xname(otmp),
         Blind ? "" : hcolor_adv(color),
         Blind ? "振動した" : "輝いた");
#endif
}

staticfn void
p_glow3(struct obj *otmp, const char *color)
{
    pline("%s feebly%s%s for a moment.",
          Yobjnam2(otmp, Blind ? "vibrate" : "glow"),
          Blind ? "" : " ", Blind ? "" : hcolor(color));
}

/* getobj callback for object to charge */
int
charge_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_EXCLUDE;

    if (obj->oclass == WAND_CLASS)
        return GETOBJ_SUGGEST;

    if (obj->oclass == RING_CLASS && objects[obj->otyp].oc_charged
        && obj->dknown && objects[obj->otyp].oc_name_known)
        return GETOBJ_SUGGEST;

    if (is_weptool(obj)) /* specific check before general tools */
        return GETOBJ_EXCLUDE;

    if (obj->oclass == TOOL_CLASS) {
        /* suggest tools that aren't oc_charged but can still be recharged */
        if (obj->otyp == BRASS_LANTERN
            || (obj->otyp == OIL_LAMP)
            /* only list magic lamps if they are not identified yet */
            || (obj->otyp == MAGIC_LAMP
                && !objects[MAGIC_LAMP].oc_name_known)) {
            return GETOBJ_SUGGEST;
        }
        /* suggest chargeable tools only if discovered, to prevent leaking
           info (e.g. revealing if an unidentified 'flute' is magic or not) */
        if (objects[obj->otyp].oc_charged) {
            return (obj->dknown && objects[obj->otyp].oc_name_known)
                     ? GETOBJ_SUGGEST : GETOBJ_DOWNPLAY;
        }
        return GETOBJ_EXCLUDE;
    }
    /* why are weapons/armor considered charged anyway?
     * make them selectable even so for "feeling of loss" message */
    return GETOBJ_EXCLUDE_SELECTABLE;
}

/* recharge an object; curse_bless is -1 if the recharging implement
   was cursed, +1 if blessed, 0 otherwise. */
void
recharge(struct obj *obj, int curse_bless)
{
    int n;
    boolean is_cursed, is_blessed;

    is_cursed = curse_bless < 0;
    is_blessed = curse_bless > 0;

    if (obj->oclass == WAND_CLASS) {
        int lim = (obj->otyp == WAN_WISHING)
                      ? 1
                      : (objects[obj->otyp].oc_dir != NODIR) ? 8 : 15;

        /* undo any prior cancellation, even when is_cursed */
        if (obj->spe == -1)
            obj->spe = 0;

        /*
         * Recharging might cause wands to explode.
         *      v = number of previous recharges
         *            v = percentage chance to explode on this attempt
         *                    v = cumulative odds for exploding
         *      0 :   0       0
         *      1 :   0.29    0.29
         *      2 :   2.33    2.62
         *      3 :   7.87   10.28
         *      4 :  18.66   27.02
         *      5 :  36.44   53.62
         *      6 :  62.97   82.83
         *      7 : 100     100
         */
        n = (int) obj->recharged;
        if (n > 0 && (obj->otyp == WAN_WISHING
                      || (n * n * n > rn2(7 * 7 * 7)))) { /* recharge_limit */
            wand_explode(obj, rnd(lim));
            return;
        }
        /* didn't explode, so increment the recharge count */
        obj->recharged = (unsigned) (n + 1);

        /* now handle the actual recharging */
        if (is_cursed) {
            stripspe(obj);
        } else {
            n = (lim == 1) ? 1 : rn1(5, lim + 1 - 5);
            if (!is_blessed)
                n = rnd(n);

            if (obj->spe < n)
                obj->spe = n;
            else
                obj->spe++;
            if (obj->otyp == WAN_WISHING && obj->spe > 3) {
                /* wands can't give more than three wishes; this code is
                   currently unreachable but left in case the rules for
                   wands of wishing change in future */
                wand_explode(obj, 1);
                return;
            }
            if (lim == 1)
                p_glow3(obj, NH_BLUE);
            else if (obj->spe >= lim)
                p_glow2(obj, NH_BLUE);
            else
                p_glow1(obj);
#if 0 /*[shop price doesn't vary by charge count]*/
            /* update shop bill to reflect new higher price */
            if (obj->unpaid)
                alter_cost(obj, 0L);
#endif
        }

    } else if (obj->oclass == RING_CLASS && objects[obj->otyp].oc_charged) {
        /* charging does not affect ring's curse/bless status */
        int s = is_blessed ? rnd(3) : is_cursed ? -rnd(2) : 1;
        boolean is_on = (obj == uleft || obj == uright);

        /* destruction depends on current state, not adjustment */
        if (obj->spe > rn2(7) || obj->spe <= -5) {
#if 0 /*JP:T*/
            pline("%s momentarily, then %s!", Yobjnam2(obj, "pulsate"),
                  otense(obj, "explode"));
#else
            Your("%sは一瞬脈動し，爆発した！", xname(obj));
#endif
            if (is_on)
                Ring_gone(obj);
            s = rnd(3 * abs(obj->spe)); /* amount of damage */
            useup(obj), obj = 0;
/*JP
            losehp(Maybe_Half_Phys(s), "exploding ring", KILLED_BY_AN);
*/
            losehp(Maybe_Half_Phys(s), "指輪の爆発で", KILLED_BY_AN);
        } else {
            long mask = is_on ? (obj == uleft ? LEFT_RING : RIGHT_RING) : 0L;

#if 0 /*JP:T*/
            pline("%s spins %sclockwise for a moment.", Yname2(obj),
                  s < 0 ? "counter" : "");
#else
            Your("%sは一瞬%s時計回りに回転した．", xname(obj),
                 s < 0 ? "反" : "");
#endif
            if (s < 0)
                costly_alteration(obj, COST_DECHNT);
            /* cause attributes and/or properties to be updated */
            if (is_on)
                Ring_off(obj);
            obj->spe += s; /* update the ring while it's off */
            if (is_on)
                setworn(obj, mask), Ring_on(obj);
            /* oartifact: if a touch-sensitive artifact ring is
               ever created the above will need to be revised  */
            /* update shop bill to reflect new higher price */
            if (s > 0 && obj->unpaid)
                alter_cost(obj, 0L);
        }

    } else if (obj->oclass == TOOL_CLASS) {
        int rechrg = (int) obj->recharged;

        if (objects[obj->otyp].oc_charged) {
            /* tools don't have a limit, but the counter used does */
            if (rechrg < 7) /* recharge_limit */
                obj->recharged++;
        }
        switch (obj->otyp) {
        case BELL_OF_OPENING:
            if (is_cursed)
                stripspe(obj);
            else if (is_blessed)
                obj->spe += rnd(3);
            else
                obj->spe += 1;
            if (obj->spe > 5)
                obj->spe = 5;
            break;
        case MAGIC_MARKER:
        case TINNING_KIT:
        case EXPENSIVE_CAMERA:
            if (is_cursed) {
                stripspe(obj);
            } else if (rechrg && obj->otyp == MAGIC_MARKER) {
                /* previously recharged */
                obj->recharged = 1; /* override increment done above */
                if (obj->spe < 3)
/*JP
                    Your("marker seems permanently dried out.");
*/
                    Your("マーカは完全に乾ききってしまった．");
                else
                    pline1(nothing_happens);
            } else if (is_blessed) {
                n = rn1(16, 15); /* 15..30 */
                if (obj->spe + n <= 50)
                    obj->spe = 50;
                else if (obj->spe + n <= 75)
                    obj->spe = 75;
                else {
                    int chrg = (int) obj->spe;
                    if ((chrg + n) > 127)
                        obj->spe = 127;
                    else
                        obj->spe += n;
                }
                p_glow2(obj, NH_BLUE);
            } else {
                n = rn1(11, 10); /* 10..20 */
                if (obj->spe + n <= 50)
                    obj->spe = 50;
                else {
                    int chrg = (int) obj->spe;

                    if (chrg + n > SPE_LIM)
                        obj->spe = SPE_LIM;
                    else
                        obj->spe += n;
                }
                p_glow2(obj, NH_WHITE);
            }
            break;
        case OIL_LAMP:
        case BRASS_LANTERN:
            if (is_cursed) {
                stripspe(obj);
                if (obj->lamplit) {
                    if (!Blind)
#if 0 /*JP:T*/
                        pline("%s out!", Tobjnam(obj, "go"));
#else
                        pline("%sは消えた！", xname(obj));
#endif
                    end_burn(obj, TRUE);
                }
            } else if (is_blessed) {
                obj->spe = 1;
                obj->age = 1500;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe = 1;
                obj->age += 750;
                if (obj->age > 1500)
                    obj->age = 1500;
                p_glow1(obj);
            }
            break;
        case CRYSTAL_BALL:
            if (obj->spe == -1) /* like wands, first uncancel */
                obj->spe = 0;

            if (is_cursed) {
                /* cursed scroll removes charges and curses ball */
                /*stripspe(obj); -- doesn't do quite what we want...*/
                if (!obj->cursed) {
                    p_glow2(obj, NH_BLACK);
                    curse(obj);
                } else {
                    pline("%s briefly.", Yobjnam2(obj, "vibrate"));
                }
                if (obj->spe > 0)
                    costly_alteration(obj, COST_UNCHRG);
                obj->spe = 0;
            } else if (is_blessed) {
                /* blessed scroll sets charges to max and blesses ball */
                obj->spe = 7;
                p_glow2(obj, !obj->blessed ? NH_LIGHT_BLUE : NH_BLUE);
                if (!obj->blessed)
                    bless(obj);
                /* [shop price stays the same regardless of charges or BUC] */
            } else {
                /* uncursed scroll increments charges and uncurses ball */
                if (obj->spe < 7 || obj->cursed) {
                    n = rnd(2);
                    obj->spe = min(obj->spe + n, 7);
                    if (!obj->cursed) {
                        p_glow1(obj);
                    } else {
                        p_glow2(obj, NH_AMBER);
                        uncurse(obj);
                    }
                } else {
                    /* charges at max and ball not being uncursed */
                    pline1(nothing_happens);
                }
            }
            break;
        case HORN_OF_PLENTY:
        case BAG_OF_TRICKS:
        case CAN_OF_GREASE:
            if (is_cursed) {
                stripspe(obj);
            } else if (is_blessed) {
                if (obj->spe <= 10)
                    obj->spe += rn1(10, 6);
                else
                    obj->spe += rn1(5, 6);
                if (obj->spe > 50)
                    obj->spe = 50;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe += rn1(5, 2);
                if (obj->spe > 50)
                    obj->spe = 50;
                p_glow1(obj);
            }
            break;
        case MAGIC_FLUTE:
        case MAGIC_HARP:
        case FROST_HORN:
        case FIRE_HORN:
        case DRUM_OF_EARTHQUAKE:
            if (is_cursed) {
                stripspe(obj);
            } else if (is_blessed) {
                obj->spe += d(2, 4);
                if (obj->spe > 20)
                    obj->spe = 20;
                p_glow2(obj, NH_BLUE);
            } else {
                obj->spe += rnd(4);
                if (obj->spe > 20)
                    obj->spe = 20;
                p_glow1(obj);
            }
            break;
        default:
            goto not_chargable;
            /*NOTREACHED*/
            break;
        } /* switch */

    } else {
 not_chargable:
/*JP
        You("have a feeling of loss.");
*/
        You("なにか損した気分になった．");
    }

    /* prevent enchantment from getting out of range */
    cap_spe(obj);
}

/*
 * Forget some things (e.g. after reading a scroll of amnesia).  When called,
 * the following are always forgotten:
 *      - felt ball & chain
 *      - skill training
 *
 * Other things are subject to flags:
 *      howmuch & ALL_SPELLS    = forget all spells
 */
staticfn void
forget(int howmuch)
{
    struct monst *mtmp;

    if (Punished)
        u.bc_felt = 0; /* forget felt ball&chain */

    if (howmuch & ALL_SPELLS)
        losespells();

    /* Forget some skills. */
    drain_weapon_skill(rnd(howmuch ? 5 : 3));

    /* forget having seen monsts (affects recognizing unseen ones by sound) */
    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
        if (mtmp != u.usteed && mtmp != u.ustuck)
            mtmp->meverseen = 0;
    /* [perhaps ought to forget having seen every monster on every level] */
    for (mtmp = gm.migrating_mons; mtmp; mtmp = mtmp->nmon)
        mtmp->meverseen = 0;
}

/* monster is hit by scroll of taming's effect */
staticfn int
maybe_tame(struct monst *mtmp, struct obj *sobj)
{
    int was_tame = mtmp->mtame;
    unsigned was_peaceful = mtmp->mpeaceful;

    if (sobj->cursed) {
        setmangry(mtmp, FALSE);
        if (was_peaceful && !mtmp->mpeaceful)
            return -1;
    } else {
        /* for a shopkeeper, tamedog() will call make_happy_shk() but
           not tame the target, so call it even if taming gets resisted */
        if (!resist(mtmp, sobj->oclass, 0, NOTELL) || mtmp->isshk)
            (void) tamedog(mtmp, sobj, FALSE);

        if ((!was_peaceful && mtmp->mpeaceful) || was_tame != mtmp->mtame)
            return 1;
    }
    return 0;
}

/* Can a stinking cloud physically exist at a certain position?
 * NOT the same thing as can_center_cloud.
 */
boolean
valid_cloud_pos(coordxy x, coordxy y)
{
    if (!isok(x,y))
        return FALSE;
    return ACCESSIBLE(levl[x][y].typ) || is_pool(x, y) || is_lava(x, y);
}

/* Callback for getpos_sethilite, also used in determining whether a scroll
 * should have its regular effects, or not because it was out of range.
 */
staticfn boolean
can_center_cloud(coordxy x, coordxy y)
{
    if (!valid_cloud_pos(x, y))
        return FALSE;
    return (cansee(x, y) && distu(x, y) < 32);
}

staticfn void
display_stinking_cloud_positions(boolean on_off)
{
    coordxy x, y, dx, dy;
    int dist = 6;

    if (on_off) {
        /* on */
        tmp_at(DISP_BEAM, cmap_to_glyph(S_goodpos));
        for (dx = -dist; dx <= dist; dx++)
            for (dy = -dist; dy <= dist; dy++) {
                x = u.ux + dx;
                y = u.uy + dy;
                /* hero's location is allowed but highlighting the hero's
                   spot makes map harder to read (if using '$' rather than
                   by changing background color) */
                if (u_at(x, y))
                    continue;
                if (can_center_cloud(x, y))
                    tmp_at(x, y);
            }
    } else {
        /* off */
        tmp_at(DISP_END, 0);
    }
}

staticfn void
seffect_enchant_armor(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    schar s;
    boolean special_armor;
    boolean same_color;
    struct obj *otmp = some_armor(&gy.youmonst);
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    boolean old_erodeproof, new_erodeproof;

    if (!otmp) {
#if 0 /*JP:T*/
        strange_feeling(sobj, !Blind
                        ? "Your skin glows then fades."
                        : "Your skin feels warm for a moment.");
#else
            strange_feeling(sobj, !Blind
                            ? "あなたの体は一瞬輝いた．"
                            : "あなたの体は一瞬暖かくなった．");
#endif
        *sobjp = 0; /* useup() in strange_feeling() */
        exercise(A_CON, !scursed);
        exercise(A_STR, !scursed);
        return;
    }
    if (confused) {
        old_erodeproof = (otmp->oerodeproof != 0);
        new_erodeproof = !scursed;
        otmp->oerodeproof = 0; /* for messages */
        if (Blind) {
            otmp->rknown = FALSE;
/*JP
            pline("%s warm for a moment.", Yobjnam2(otmp, "feel"));
*/
            Your("%sは一瞬暖かくなった．", xname(otmp));
        } else {
            otmp->rknown = TRUE;
#if 0 /*JP:T*/
            pline("%s covered by a %s %s %s!", Yobjnam2(otmp, "are"),
                  scursed ? "mottled" : "shimmering",
                  hcolor(scursed ? NH_BLACK : NH_GOLDEN),
                  scursed ? "glow"
                  : (is_shield(otmp) ? "layer" : "shield"));
#else
                Your("%sは%s%s%sで覆われた！", xname(otmp),
                     hcolor_adv(scursed ? NH_BLACK : NH_GOLDEN),
                     scursed ? "光るまだらの" : "ゆらめく",
                     scursed ? "輝き"
                             : "バリア");
#endif
        }
        if (new_erodeproof && (otmp->oeroded || otmp->oeroded2)) {
            otmp->oeroded = otmp->oeroded2 = 0;
#if 0 /*JP:T*/
            pline("%s as good as new!",
                  Yobjnam2(otmp, Blind ? "feel" : "look"));
#else
                Your("%sは新品同様になった！",
                     xname(otmp));
#endif
        }
        if (old_erodeproof && !new_erodeproof) {
            /* restore old_erodeproof before shop charges */
            otmp->oerodeproof = 1;
            costly_alteration(otmp, COST_DEGRD);
        }
        otmp->oerodeproof = new_erodeproof ? 1 : 0;
        return;
    }
    /* elven armor vibrates warningly when enchanted beyond a limit */
    special_armor = is_elven_armor(otmp)
        || (Role_if(PM_WIZARD) && otmp->otyp == CORNUTHAUM);
    if (scursed)
        same_color = (otmp->otyp == BLACK_DRAGON_SCALE_MAIL
                      || otmp->otyp == BLACK_DRAGON_SCALES);
    else
        same_color = (otmp->otyp == SILVER_DRAGON_SCALE_MAIL
                      || otmp->otyp == SILVER_DRAGON_SCALES
                      || otmp->otyp == SHIELD_OF_REFLECTION);
    if (Blind)
        same_color = FALSE;

    /* KMH -- catch underflow */
    s = scursed ? -otmp->spe : otmp->spe;
    if (s > (special_armor ? 5 : 3) && rn2(s)) {
        otmp->in_use = TRUE;
#if 0 /*JP:T*/
        pline("%s violently %s%s%s for a while, then %s.", Yname2(otmp),
              otense(otmp, Blind ? "vibrate" : "glow"),
              (!Blind && !same_color) ? " " : "",
              (Blind || same_color) ? "" : hcolor(scursed ? NH_BLACK
                                                  : NH_SILVER),
              otense(otmp, "evaporate"));
#else
            Your("%sはしばらくの間激しく%s%s，蒸発した．", xname(otmp),
                 (Blind || same_color) ? "" : hcolor_adv(scursed ? NH_BLACK : NH_SILVER),
                 Blind ? "振動し" : "輝き");
#endif
        remove_worn_item(otmp, FALSE);
        useup(otmp);
        return;
    }
    if (s < -100)
        s = -100; /* avoid integer overflow with very negative armor */

    /* Base power of the enchantment:

       2 for -1 to +0 armor;
       1 for +1 to +2 armor;
       0 for +3 to +4 armor, etc.

       When disenchanting, everything is done with reversed signs. */
    s = (4 - s) / 2;

    /* Elven/artifact and nonmagical armor is easier to enchant;
       blessed scrolls are more effective. */
    if (special_armor)
        ++s;
    if (!objects[otmp->otyp].oc_magic)
        ++s;
    if (sblessed)
        ++s;

    if (s <= 0) {
        s = 0;
        if (otmp->spe > 0 && !rn2(otmp->spe))
            s = 1;
    } else {
        s = rnd(s);
    }
    if (s > 11)
        s = 11;    /* unlikely but possible: avoids an overflow later */

    if (scursed)
        s = -s;

    if (s >= 0 && Is_dragon_scales(otmp)) {
        unsigned was_lit = otmp->lamplit;
        int old_light = artifact_light(otmp) ? arti_light_radius(otmp) : 0;

        /* dragon scales get turned into dragon scale mail */
/*JP
        pline("%s merges and hardens!", Yname2(otmp));
*/
        Your("%sは融合し固くなった！", xname(otmp));
        setworn((struct obj *) 0, W_ARM);
        /* assumes same order */
        otmp->otyp += GRAY_DRAGON_SCALE_MAIL - GRAY_DRAGON_SCALES;
        otmp->lamplit = 0; /* don't want bless() or uncurse() to adjust
                            * light radius because scales -> scale_mail will
                            * result in a second increase with own message */
        if (sblessed) {
            otmp->spe++;
            cap_spe(otmp);
            if (!otmp->blessed)
                bless(otmp);
        } else if (otmp->cursed)
            uncurse(otmp);
        otmp->known = 1;
        setworn(otmp, W_ARM);
        if (otmp->unpaid)
            alter_cost(otmp, 0L); /* shop bill */
        otmp->lamplit = was_lit;
        if (old_light)
            maybe_adjust_light(otmp, old_light);
        return;
    }
#if 0 /*JP:T*/
    pline("%s %s%s%s%s for a %s.", Yname2(otmp),
          (s == 0) ? "violently " : "",
          otense(otmp, Blind ? "vibrate" : "glow"),
          (!Blind && !same_color) ? " " : "",
          (Blind || same_color)
          ? "" : hcolor(scursed ? NH_BLACK : NH_SILVER),
          (s * s > 1) ? "while" : "moment");
#else
    pline("%sは%s%s%s%s．", xname(otmp),
          (s * s > 1) ? "しばらくの間" : "一瞬",
          (s == 0) ? "激しく" : "",
          (Blind || same_color)
          ? "" : hcolor(scursed ? NH_BLACK : NH_SILVER),
          Blind ? "振動した" : "輝いた");
#endif
    /* [this cost handling will need updating if shop pricing is
       ever changed to care about curse/bless status of armor] */
    if (s < 0)
        costly_alteration(otmp, COST_DECHNT);
    if (scursed && !otmp->cursed)
        curse(otmp);
    else if (sblessed && !otmp->blessed)
        bless(otmp);
    else if (!scursed && otmp->cursed)
        uncurse(otmp);
    if (s) {
        int oldspe = otmp->spe;
        /* despite being schar, it shouldn't be possible for spe to wrap
           here because it has been capped at 99 and s is quite small;
           however, might need to change s if it takes spe past 99 */
        otmp->spe += s;
        cap_spe(otmp); /* make sure that it doesn't exceed SPE_LIM */
        s = otmp->spe - oldspe; /* cap_spe() might have throttled 's' */
        if (s) /* skip if it got changed to 0 */
            adj_abon(otmp, s); /* adjust armor bonus for Dex or Int+Wis */
        gk.known = otmp->known;
        /* update shop bill to reflect new higher price */
        if (s > 0 && otmp->unpaid)
            alter_cost(otmp, 0L);
    }

    if ((otmp->spe > (special_armor ? 5 : 3))
        && (special_armor || !rn2(7)))
#if 0 /*JP:T*/
        pline("%s %s.", Yobjnam2(otmp, "suddenly vibrate"),
              Blind ? "again" : "unexpectedly");
#else
            Your("%sは突然%s振動した．", xname(otmp),
                 Blind ? "また" : "思いもよらず");
#endif
}

/* destroy a random cursed armor worn by hero */
staticfn boolean
disintegrate_cursed_armor(void)
{
    struct obj *armors[10];
    int idx = 0;

    armors[0] = NULL;
    if (uarm && uarm->cursed)
        armors[idx++] = uarm;
    if (uarmc && uarmc->cursed)
        armors[idx++] = uarmc;
    if (uarmh && uarmh->cursed)
        armors[idx++] = uarmh;
    if (uarms && uarms->cursed)
        armors[idx++] = uarms;
    if (uarmg && uarmg->cursed)
        armors[idx++] = uarmg;
    if (uarmf && uarmf->cursed)
        armors[idx++] = uarmf;
    if (uarmu && uarmu->cursed)
        armors[idx++] = uarmu;
    if (!idx)
        return FALSE;

    if (disintegrate_arm(armors[rn2(idx)]))
        return TRUE;

    return FALSE;
}

staticfn void
seffect_destroy_armor(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    struct obj *otmp = some_armor(&gy.youmonst);
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    boolean old_erodeproof, new_erodeproof;

    if (confused) {
        if (!otmp) {
/*JP
            strange_feeling(sobj, "Your bones itch.");
*/
            strange_feeling(sobj, "骨がムズムズする．");
            *sobjp = 0; /* useup() in strange_feeling() */
            exercise(A_STR, FALSE);
            exercise(A_CON, FALSE);
            return;
        }
        old_erodeproof = (otmp->oerodeproof != 0);
        new_erodeproof = scursed;
        otmp->oerodeproof = 0; /* for messages */
        p_glow2(otmp, NH_PURPLE);
        if (old_erodeproof && !new_erodeproof) {
            /* restore old_erodeproof before shop charges */
            otmp->oerodeproof = 1;
            costly_alteration(otmp, COST_DEGRD);
        }
        otmp->oerodeproof = new_erodeproof ? 1 : 0;
        return;
    }

    if (scursed) {
        if (otmp && otmp->cursed) {
            /* armor and scroll both cursed */
/*JP
            pline("%s.", Yobjnam2(otmp, "vibrate"));
*/
            Your("%sは振動した．", xname(otmp));
            if (otmp->spe >= -6) {
                otmp->spe += -1;
                adj_abon(otmp, -1);
            }
            make_stunned((HStun & TIMEOUT) + (long) rn1(10, 10), TRUE);
        } else if (disintegrate_arm(otmp)) {
            gk.known = TRUE;
            return;
        }
    } else {
        boolean gets_choice = (otmp && sobj && sobj->blessed
                               && count_worn_armor() > 1);

        if (gets_choice) {
            struct obj *atmp;

            if (!objects[sobj->otyp].oc_name_known)
                pline("This is %s!", an(actualoname(sobj)));
            gk.known = TRUE;
            atmp = getobj("destroy", any_worn_armor_ok, GETOBJ_PROMPT);
            /* check the return value, if user picked non-valid obj */
            if (any_worn_armor_ok(atmp) == GETOBJ_SUGGEST)
                otmp = atmp;
            if (disintegrate_arm(otmp)) {
                gk.known = TRUE;
                return;
            }
        } else if (sobj->blessed && disintegrate_cursed_armor()) {
            gk.known = TRUE;
            return;
        } else if (!destroy_arm()) {
/*JP
            strange_feeling(sobj, "Your skin itches.");
*/
            strange_feeling(sobj,"皮膚がムズムズする．");
            *sobjp = 0; /* useup() in strange_feeling() */
            exercise(A_STR, FALSE);
            exercise(A_CON, FALSE);
            return;
        } else
            gk.known = TRUE;
    }
}

staticfn void
seffect_confuse_monster(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed,
            scursed = sobj->cursed,
            confused = (Confusion != 0),
            altfeedback = (Blind || Invisible);
    const char *const hands = makeplural(body_part(HAND));

    if (gy.youmonst.data->mlet != S_HUMAN || scursed) {
        if (!HConfusion)
/*JP
            You_feel("confused.");
*/
            You_feel("混乱した．");
        make_confused(HConfusion + rnd(100), FALSE);
    } else if (confused) {
        if (!sblessed) {
#if 0 /*JP:T*/
            Your("%s begin to %s%s.", hands,
                 altfeedback ? "tingle" : "glow ",
                 altfeedback ? "" : hcolor(NH_PURPLE));
#else
            Your("%sは%s%sはじめた．", hands,
                 altfeedback ? "" : hcolor(NH_PURPLE),
                 altfeedback ? "ヒリヒリし" : "輝き");
#endif
            make_confused(HConfusion + rnd(100), FALSE);
        } else {
#if 0 /*JP:T*/
            pline("A %s%s surrounds your %s.",
                  altfeedback ? "" : hcolor(NH_RED),
                  altfeedback ? "faint buzz" : " glow", body_part(HEAD));
#else
            pline("%s%sがあなたの%sを取り巻いた．",
                  altfeedback ? "" : hcolor(NH_RED),
                  altfeedback ? "かすかにブーンと鳴るもの" : "輝くもの", body_part(HEAD));
#endif
            make_confused(0L, TRUE);
        }
    } else {
        /* scroll vs spell */
        int incr = (sobj->oclass == SCROLL_CLASS) ? 3 : 0;

        if (!sblessed) {
            if (altfeedback)
#if 0 /*JP:T*/
                Your("%s tingle%s.", hands, u.umconf ? " even more" : "");
#else
                Your("%sは%sヒリヒリした．", hands, u.umconf ? "さらに" : "");
#endif
            else if (!u.umconf)
#if 0 /*JP:T*/
                Your("%s begin to glow %s.", hands, hcolor(NH_RED));
#else
                Your("%sは%s輝きはじめた．", hands, hcolor_adv(NH_RED));
#endif
            else
                pline_The("%s glow of your %s intensifies.", hcolor(NH_RED),
                          hands);
            incr += rnd(2);
        } else {
            if (altfeedback)
#if 0 /*JP:T*/
                Your("%s tingle %s sharply.", hands,
                     u.umconf ? "even more" : "very");
#else
                Your("%sは%sピリピリする．", hands,
                     u.umconf ? "さらに" : "とても");
#endif
            else
#if 0 /*JP:T*/
                Your("%s glow %s brilliant %s.", hands,
                     u.umconf ? "an even more" : "a", hcolor(NH_RED));
#else
                Your("%sは%s%s明るく輝いた．", hands,
                     u.umconf ? "さらに" : "", hcolor_adv(NH_RED));
#endif
            incr += rn1(8, 2);
        }
        /* after a while, repeated uses become less effective */
        if (u.umconf >= 40)
            incr = 1;
        u.umconf += (unsigned) incr;
    }
}

staticfn void
seffect_scare_monster(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    int otyp = sobj->otyp;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    int ct = 0;
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (cansee(mtmp->mx, mtmp->my)) {
            if (confused || scursed) {
                mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
                mtmp->mcanmove = 1;
            } else if (!resist(mtmp, sobj->oclass, 0, NOTELL))
                monflee(mtmp, 0, FALSE, FALSE);
            if (!mtmp->mtame)
                ct++; /* pets don't laugh at you */
        }
    }
    if (otyp == SCR_SCARE_MONSTER || !ct) {
        if (confused || scursed) {
            Soundeffect(se_sad_wailing, 50);
        } else {
            Soundeffect(se_maniacal_laughter, 50);
        }
#if 0 /*JP:T*/
        You_hear("%s %s.", (confused || scursed) ? "sad wailing"
                 : "maniacal laughter",
                 !ct ? "in the distance" : "close by");
#else
            You_hear("%sくで%sを聞いた．",
                     !ct ? "遠" : "近",
                     (confused || sobj->cursed) ? "悲しく泣き叫ぶ声"
                                                : "狂ったように笑う声");
#endif
    }
}

staticfn void
seffect_remove_curse(struct obj **sobjp)
{
    struct obj *sobj = *sobjp; /* scroll or fake spellbook */
    int otyp = sobj->otyp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    struct obj *obj, *nxto;
    long wornmask;

#if 0 /*JP:T*/
    You_feel(!Hallucination
             ? (!confused ? "like someone is helping you."
                : "like you need some help.")
             : (!confused ? "in touch with the Universal Oneness."
                : "the power of the Force against you!"));
#else
        You(!Hallucination
            ? (!confused ? "誰かがあなたを助けているような気がした．"
                         : "自分が助けを必要としているような気がした．")
            : (!confused ? "宇宙調和の原理に触れているような気がした．"
                         : "フォースの力があなたにはむかっているように感じた！"));
#endif

    if (scursed) {
/*JP
        pline_The("scroll disintegrates.");
*/
        pline("巻物は粉々になった．");
    } else {
        /* 5.0: this used to use a straight
               for (obj = invent; obj; obj = obj->nobj) {}
           traversal, but for the confused case, secondary weapon might
           become cursed and be dropped, moving it from the invent chain
           to the floor chain at hero's spot, so we have to remember the
           next object prior to processing the current one */
        for (obj = gi.invent; obj; obj = nxto) {
            nxto = obj->nobj;
            /* gold isn't subject to cursing and blessing */
            if (obj->oclass == COIN_CLASS)
                continue;
            /* hide current scroll from itself so that perm_invent won't
               show known blessed scroll losing bknown when confused */
            if (obj == sobj && obj->quan == 1L)
                continue;
            wornmask = (obj->owornmask & ~(W_BALL | W_ART | W_ARTI));
            if (wornmask && !sblessed) {
                /* handle a couple of special cases; we don't
                   allow auxiliary weapon slots to be used to
                   artificially increase number of worn items */
                if (obj == uswapwep) {
                    if (!u.twoweap)
                        wornmask = 0L;
                } else if (obj == uquiver) {
                    if (obj->oclass == WEAPON_CLASS) {
                        /* mergeable weapon test covers ammo,
                           missiles, spears, daggers & knives */
                        if (!objects[obj->otyp].oc_merge)
                            wornmask = 0L;
                    } else if (obj->oclass == GEM_CLASS) {
                        /* possibly ought to check whether
                           alternate weapon is a sling... */
                        if (!uslinging())
                            wornmask = 0L;
                    } else {
                        /* weptools don't merge and aren't
                           reasonable quivered weapons */
                        wornmask = 0L;
                    }
                }
            }
            if (sblessed || wornmask || obj->otyp == LOADSTONE
                /* this treats an in-use leash as a worn item but does not
                   do the same for lit lamp/candle [seems inconsistent] */
                || (obj->otyp == LEASH && obj->leashmon)) {
                /* water price varies by curse/bless status */
                boolean shop_h2o = (obj->unpaid && obj->otyp == POT_WATER);

                if (confused) {
                    blessorcurse(obj, 2);
                    /* lose knowledge of this object's curse/bless
                       state (even if it didn't actually change) */
                    obj->bknown = 0;
                    /* blessorcurse() only affects uncursed items
                       so no need to worry about price of water
                       going down (hence no costly_alteration) */
                    if (shop_h2o && (obj->cursed || obj->blessed))
                        alter_cost(obj, 0L); /* price goes up */
                } else if (obj->cursed) {
                    if (shop_h2o)
                        costly_alteration(obj, COST_UNCURS);
                    uncurse(obj);
                    /* if the object was known to be cursed and is now
                       known not to be, make the scroll known; it's
                       trivial to identify anyway by comparing inventory
                       before and after */
                    if (obj->bknown && otyp == SCR_REMOVE_CURSE)
                        learnscrolltyp(SCR_REMOVE_CURSE);
                }
            }
        }
        /* if riding, treat steed's saddle as if part of hero's invent */
        if (u.usteed && (obj = which_armor(u.usteed, W_SADDLE)) != 0) {
            if (confused) {
                blessorcurse(obj, 2);
                obj->bknown = 0; /* skip set_bknown() */
            } else if (obj->cursed) {
                uncurse(obj);
                /* like rndcurse(sit.c), effect on regular inventory
                   doesn't show things glowing but saddle does */
                if (!Blind) {
                    pline("%s %s.", Yobjnam2(obj, "glow"),
                              hcolor("amber"));
                    obj->bknown = Hallucination ? 0 : 1;
                } else {
                    obj->bknown = 0; /* skip set_bknown() */
                }
            }
        }
    }
    if (Punished && !confused)
        unpunish();
    if (u.utrap && u.utraptype == TT_BURIEDBALL) {
        buried_ball_to_freedom();
/*JP
        pline_The("clasp on your %s vanishes.", body_part(LEG));
*/
        pline_The("%sの金具が消えた．", body_part(LEG));
    }
    update_inventory();
}

staticfn void
seffect_create_monster(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);

    if (create_critters(1 + ((confused || scursed) ? 12 : 0)
                        + ((sblessed || rn2(73)) ? 0 : rnd(4)),
                        confused ? &mons[PM_ACID_BLOB]
                        : (struct permonst *) 0,
                        FALSE))
        gk.known = TRUE;
    /* no need to flush monsters; we ask for identification only if the
     * monsters are not visible
     */
}

staticfn void
seffect_enchant_weapon(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    boolean old_erodeproof, new_erodeproof;
    int s;

    /* [What about twoweapon mode?  Proofing/repairing/enchanting both
       would be too powerful, but shouldn't we choose randomly between
       primary and secondary instead of always acting on primary?] */
    if (confused && uwep
        && erosion_matters(uwep) && uwep->oclass != ARMOR_CLASS) {
        old_erodeproof = (uwep->oerodeproof != 0);
        new_erodeproof = !scursed;
        uwep->oerodeproof = 0; /* for messages */
        if (Blind) {
            uwep->rknown = FALSE;
/*JP
            Your("weapon feels warm for a moment.");
*/
            pline("武器が一瞬暖かくなったような気がした．");
        } else {
            uwep->rknown = TRUE;
#if 0 /*JP:T*/
            pline("%s covered by a %s %s %s!", Yobjnam2(uwep, "are"),
                  scursed ? "mottled" : "shimmering",
                  hcolor(scursed ? NH_PURPLE : NH_GOLDEN),
                  scursed ? "glow" : "shield");
#else
                Your("%sは%s%s%sで覆われた！", xname(uwep),
                     hcolor_adv(scursed ? NH_PURPLE : NH_GOLDEN),
                     scursed ? "光るまだらの" : "ゆらめく",
                     scursed ? "輝き" : "バリア");
#endif
        }
        if (new_erodeproof && (uwep->oeroded || uwep->oeroded2)) {
            uwep->oeroded = uwep->oeroded2 = 0;
#if 0 /*JP:T*/
            pline("%s as good as new!",
                  Yobjnam2(uwep, Blind ? "feel" : "look"));
#else
                pline("%sは新品同様になった！", xname(uwep));
#endif
        }
        if (old_erodeproof && !new_erodeproof) {
            /* restore old_erodeproof before shop charges */
            uwep->oerodeproof = 1;
            costly_alteration(uwep, COST_DEGRD);
        }
        uwep->oerodeproof = new_erodeproof ? 1 : 0;
        return;
    }
    s = scursed ? -1
        : !uwep ? 1 /* guard further tests against null pointer */
          : (uwep->spe >= 9) ? (rn2(uwep->spe) == 0) /* usually 0, maybe  1 */
            : sblessed ? rnd(3 - uwep->spe / 3) /* >= 9 case prevents rnd(0) */
              : 1; /* uncursed */
    if (!chwepon(sobj, s))
        *sobjp = 0; /* nothing enchanted: strange_feeling -> useup */
    if (uwep)
        cap_spe(uwep);
}

staticfn void
seffect_taming(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean confused = (Confusion != 0);
    int candidates, res, results, vis_results;

    if (u.uswallow) {
        candidates = 1;
        results = vis_results = maybe_tame(u.ustuck, sobj);
    } else {
        int i, j, bd = confused ? 5 : 1;
        struct monst *mtmp;

        /* note: maybe_tame() can return either positive or
           negative values, but not both for the same scroll */
        candidates = results = vis_results = 0;
        for (i = -bd; i <= bd; i++)
            for (j = -bd; j <= bd; j++) {
                if (!isok(u.ux + i, u.uy + j))
                    continue;
                if ((mtmp = m_at(u.ux + i, u.uy + j)) != 0
                    || (!i && !j && (mtmp = u.usteed) != 0)) {
                    ++candidates;
                    res = maybe_tame(mtmp, sobj);
                    results += res;
                    if (canspotmon(mtmp))
                        vis_results += res;
                }
            }
    }
    if (!results) {
#if 0 /*JP:T*/
        pline("Nothing interesting %s.",
              !candidates ? "happens" : "seems to happen");
#else
            pline("何も面白いことはおこらなかった%s．",
                  !candidates ? "" : "ようだ");
#endif
    } else {
#if 0 /*JP:T*/
        pline_The("neighborhood %s %sfriendlier.",
                  vis_results ? "is" : "seems",
                  (results < 0) ? "un" : "");
#else
            pline_The("お隣との仲が%sくなった%s．",
                      (results < 0) ? "悪" : "良",
                      vis_results ? "" : "ようだ");
#endif
        if (vis_results > 0)
            gk.known = TRUE;
    }
}

staticfn void
seffect_genocide(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    int otyp = sobj->otyp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean already_known = (sobj->oclass == SPBOOK_CLASS /* spell */
                             || objects[otyp].oc_name_known);

    if (!already_known)
/*JP
        You("have found a scroll of genocide!");
*/
        pline("これは虐殺の巻物だ！");
    gk.known = TRUE;
    if (sblessed)
        do_class_genocide();
    else
        do_genocide((!scursed) | (2 * !!Confusion));
}

staticfn void
seffect_light(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);

    if (!confused) {
        if (!Blind)
            gk.known = TRUE;
        litroom(!scursed, sobj);
        if (!scursed) {
            if (lightdamage(sobj, TRUE, 5))
                gk.known = TRUE;
        }
    } else {
        int pm = scursed ? PM_BLACK_LIGHT : PM_YELLOW_LIGHT;

        if ((svm.mvitals[pm].mvflags & G_GONE)) {
            pline("Tiny lights sparkle in the air momentarily.");
        } else {
            /* surround with cancelled tame lights which won't explode */
            struct monst *mon;
            boolean sawlights = FALSE;
            int i, numlights = rn1(2, 3) + (sblessed * 2);

            for (i = 0; i < numlights; ++i) {
                mon = makemon(&mons[pm], u.ux, u.uy,
                              MM_EDOG | NO_MINVENT | MM_NOMSG);
                if (mon) {
                    initedog(mon, TRUE);
                    mon->msleeping = 0;
                    mon->mcan = TRUE;
                    if (canspotmon(mon))
                        sawlights = TRUE;
                    newsym(mon->mx, mon->my);
                }
            }
            if (sawlights) {
                pline("Lights appear all around you!");
                gk.known = TRUE;
            }
        }
    }
}

staticfn void
seffect_charging(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    int otyp = sobj->otyp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    boolean already_known = (sobj->oclass == SPBOOK_CLASS /* spell */
                             || objects[otyp].oc_name_known);
    struct obj *otmp;

    if (confused) {
        if (scursed) {
/*JP
            You_feel("discharged.");
*/
            You_feel("放出された気がする．");
            u.uen = 0;
        } else {
/*JP
            You_feel("charged up!");
*/
            You_feel("充填されたような気がする！");
            u.uen += d(sblessed ? 6 : 4, 4);
            if (u.uen > u.uenmax) /* if current energy is already at   */
                u.uenmax = u.uen; /* or near maximum, increase maximum */
            else
                u.uen = u.uenmax; /* otherwise restore current to max  */
        }
        disp.botl = TRUE;
        return;
    }
    /* known = TRUE; -- handled inline here */
    if (!already_known) {
/*JP
        pline("This is a charging scroll.");
*/
        pline("これは充填の巻物だ！");
        learnscroll(sobj);
    }
    /* use it up now to prevent it from showing in the
       getobj picklist because the "disappears" message
       was already delivered */
    useup(sobj);
    *sobjp = 0; /* it's gone */
    otmp = getobj("charge", charge_ok, GETOBJ_PROMPT | GETOBJ_ALLOWCNT);
    if (otmp)
        recharge(otmp, scursed ? -1 : sblessed ? 1 : 0);
}

staticfn void
seffect_amnesia(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed;

    gk.known = TRUE;
    forget((!sblessed ? ALL_SPELLS : 0));
    if (Hallucination) /* Ommmmmm! */
/*JP
        Your("mind releases itself from mundane concerns.");
*/
        Your("心は平凡な利害関係から解放された．");
    else if (!strncmpi(svp.plname, "Maud", 4))
#if 0 /*JP:T*/
        pline("As your mind turns inward on itself,"
              " you forget everything else.");
#else
        pline("あなたの心は内側に向き，"
              "全てを忘れてしまった．");
#endif
    else if (rn2(2))
/*JP
        pline("Who was that Maud person anyway?");
*/
        pline("Maudって娘はいったい誰だい？");
    else
/*JP
        pline("Thinking of Maud you forget everything else.");
*/
        pline("Maudを考えること以外，あなたは全てを忘れてしまった．");
    exercise(A_WIS, FALSE);
}

staticfn void
seffect_fire(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    int otyp = sobj->otyp;
    boolean sblessed = sobj->blessed;
    boolean confused = (Confusion != 0);
    boolean already_known = (sobj->oclass == SPBOOK_CLASS /* spell */
                             || objects[otyp].oc_name_known);
    coord cc;
    int dam, cval;

    cc.x = u.ux;
    cc.y = u.uy;
    cval = bcsign(sobj);
    dam = (2 * (rn1(3, 3) + 2 * cval) + 1) / 3;
    useup(sobj);
    *sobjp = 0; /* it's gone */
    if (!already_known)
        (void) learnscrolltyp(SCR_FIRE);
    if (confused) {
        if (Underwater) {
            pline("A little %s around you vaporizes.", hliquid("water"));
        }
        else if (Fire_resistance) {
            shieldeff(u.ux, u.uy);
            monstseesu(M_SEEN_FIRE);
            if (!Blind)
/*JP
                pline("Oh, look, what a pretty fire in your %s.",
*/
                pline("わぁごらん．小さな火が%sにある．",
                      makeplural(body_part(HAND)));
            else
/*JP
                You_feel("a pleasant warmth in your %s.",
*/
                You_feel("%sの中に快適な暖かさを感じた．",
                         makeplural(body_part(HAND)));
        } else {
            monstunseesu(M_SEEN_FIRE);
/*JP
            pline_The("scroll catches fire and you burn your %s.",
*/
            pline("巻物に火が燃えうつり，あなたの%sを焼いた．",
                      makeplural(body_part(HAND)));
/*JP
            losehp(1, "scroll of fire", KILLED_BY_AN);
*/
            losehp(1, "炎の巻物で", KILLED_BY_AN);
        }
        return;
    }
    if (Underwater) {
/*JP
        pline_The("%s around you vaporizes violently!", hliquid("water"));
*/
        pline_The("あなたの回りの%sははげしく沸騰した！", hliquid("水"));
    } else {
        if (sblessed) {
            if (!already_known)
/*JP
                pline("This is a scroll of fire!");
*/
                pline("これは火の巻物だ！");
            dam *= 5;
/*JP
            pline("Where do you want to center the explosion?");
*/
            pline("どこを爆発の中心にしますか？");
            getpos_sethilite(display_stinking_cloud_positions,
                             can_center_cloud);
/*JP
            (void) getpos(&cc, TRUE, "the desired position");
*/
            (void) getpos(&cc, TRUE, "狙いの場所");
            if (!can_center_cloud(cc.x, cc.y)) {
                /* try to reach too far, get burned */
                cc.x = u.ux;
                cc.y = u.uy;
            }
        }
        if (u_at(cc.x, cc.y)) {
/*JP
            pline_The("scroll erupts in a tower of flame!");
*/
            pline("巻物から火柱が立ち昇った！");
            iflags.last_msg = PLNMSG_TOWER_OF_FLAME; /* for explode() */
            burn_away_slime();
        }
    }
#define ZT_SPELL_O_FIRE 11 /* explained in splatter_burning_oil(explode.c) */
    explode(cc.x, cc.y, ZT_SPELL_O_FIRE, dam, SCROLL_CLASS, EXPL_FIERY);
#undef ZT_SPELL_O_FIRE
}

staticfn void
seffect_earth(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);

    /* TODO: handle steeds */
    if (!Is_rogue_level(&u.uz) && has_ceiling(&u.uz)
        && (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
        coordxy x, y;
        int nboulders = 0;

        /* Identify the scroll */
        if (u.uswallow) {
/*JP
            You_hear("rumbling.");
*/
            You_hear("ゴロゴロという音を聞いた．");
        } else {
            if (!avoid_ceiling(&u.uz)) {
#if 0 /*JP:T*/
                pline_The("%s rumbles %s you!", ceiling(u.ux, u.uy),
                          sblessed ? "around" : "above");
#else
                pline("あなたの%sの%sからゴロゴロと音が聞こえてきた！",
                      sblessed ? "まわり" : "真上", ceiling(u.ux,u.uy));
#endif
            } else {
                char matbuf[BUFSZ];
                const char *const avalanche = "avalanche";

                Sprintf(matbuf, "%s",
                        sblessed ? makeplural(avalanche) : an(avalanche));
                pline("%s of boulders %s %s you!",
                      upstart(matbuf), vtense(matbuf, "materialize"),
                      sblessed ? "around" : "above");
            }
        }
        gk.known = 1;
        sokoban_guilt();

        /* Loop through the surrounding squares */
        if (!scursed)
            for (x = u.ux - 1; x <= u.ux + 1; x++) {
                for (y = u.uy - 1; y <= u.uy + 1; y++) {
                    /* Is this a suitable spot? */
                    if (isok(x, y) && !closed_door(x, y)
                        && !IS_OBSTRUCTED(levl[x][y].typ)
                        && !IS_AIR(levl[x][y].typ)
                        && (x != u.ux || y != u.uy)) {
                        nboulders +=
                            drop_boulder_on_monster(x, y, confused, TRUE);
                    }
                }
            }
        /* Attack the player */
        if (!sblessed) {
            drop_boulder_on_player(confused, !scursed, TRUE, FALSE);
        } else if (!nboulders)
/*JP
            pline("But nothing else happens.");
*/
            pline("しかし他に何もおこらなかった．");
    }
}

staticfn void
seffect_punishment(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean sblessed = sobj->blessed;
    boolean confused = (Confusion != 0);

    gk.known = TRUE;
    if (confused || sblessed) {
/*JP
        You_feel("guilty.");
*/
        You("罪を感じた．");
        return;
    }
    punish(sobj);
}

staticfn void
seffect_stinking_cloud(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    int otyp = sobj->otyp;
    boolean already_known = (sobj->oclass == SPBOOK_CLASS /* spell */
                             || objects[otyp].oc_name_known);

    if (!already_known)
/*JP
        You("have found a scroll of stinking cloud!");
*/
        You("悪臭雲の巻物を発見した！");
    gk.known = TRUE;
    do_stinking_cloud(sobj, already_known);
}

staticfn void
seffect_blank_paper(struct obj **sobjp UNUSED)
{
    if (Blind)
/*JP
        You("don't remember there being any magic words on this scroll.");
*/
        You("巻物に呪文が書いてなかったことを思いだした．");
    else
/*JP
        pline("This scroll seems to be blank.");
*/
        pline("この巻物には何も書いてないように見える．");
    gk.known = TRUE;
}

staticfn void
seffect_teleportation(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);

    if (confused || scursed) {
        level_tele();
        /* gives "materialize on different/same level!" message, must
           be a teleport scroll */
        gk.known = TRUE;
    } else {
        scrolltele(sobj);
        /* this will call learnscroll() as appropriate, and has results
           which maybe shouldn't result in the scroll becoming known;
           either way, no need to set gk.known here */
    }
}

staticfn void
seffect_gold_detection(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);

    if ((confused || scursed) ? trap_detect(sobj) : gold_detect(sobj))
        *sobjp = 0; /* failure: strange_feeling() -> useup() */
}

staticfn void
seffect_food_detection(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;

    if (food_detect(sobj))
        *sobjp = 0; /* nothing detected: strange_feeling -> useup */
}

staticfn void
seffect_identify(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    int otyp = sobj->otyp;
    boolean is_scroll = (sobj->oclass == SCROLL_CLASS);
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    boolean already_known = (sobj->oclass == SPBOOK_CLASS /* spell */
                             || objects[otyp].oc_name_known);

    if (is_scroll) { /* scroll of identify */
        /* known = TRUE; -- handled inline here */
        /* use up the scroll first, before learnscrolltyp() -> makeknown()
           performs perm_invent update; also simplifies empty invent check */
        useup(sobj);
        *sobjp = 0; /* it's gone */
        /* scroll just identifies itself for any scroll read while confused
           or for cursed scroll read without knowing identify yet */
        if (confused || (scursed && !already_known))
/*JP
            You("identify this as an identify scroll.");
*/
            You("これは識別の巻物だと識別した．");
        else if (!already_known)
/*JP
            pline("This is an identify scroll.");
*/
            pline("これは識別の巻物だ．");
        if (!already_known)
            (void) learnscrolltyp(SCR_IDENTIFY);
        if (confused || (scursed && !already_known))
            return;
    }

    if (gi.invent) {
        int cval = 1;
        if (sblessed || (!scursed && !rn2(5))) {
            cval = rn2(5);
            /* note: if cval==0, identify all items */
            if (cval == 1 && sblessed && Luck > 0)
                ++cval;
        }
        identify_pack(cval, !already_known);
    } else {
        /* spell cast with inventory empty or scroll read when it's
           the only item leaving empty inventory after being used up */
#if 0 /*JP:T*/
        pline("You're not carrying anything%s to be identified.",
              (is_scroll) ? " else" : "");
#else
        pline("あなたは%s識別できるものをなにも持っていない．",
              (is_scroll) ? "他に" : "");
#endif
    }
}

staticfn void
seffect_magic_mapping(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean is_scroll = (sobj->oclass == SCROLL_CLASS);
    boolean sblessed = sobj->blessed;
    boolean scursed = sobj->cursed;
    boolean confused = (Confusion != 0);
    int cval;

    if (is_scroll) {
        if (svl.level.flags.nommap) {
/*JP
            Your("mind is filled with crazy lines!");
*/
            Your("心にただならぬ線！線！線！が浮びあがった！");
            if (Hallucination)
/*JP
                pline("Wow!  Modern art.");
*/
                pline("ワォ！モダンアートだ！");
            else
/*JP
                Your("%s spins in bewilderment.", body_part(HEAD));
*/
                You("当惑して目がまわった．");
            make_confused(HConfusion + rnd(30), FALSE);
            return;
        }
        if (sblessed) {
            coordxy x, y;

            for (x = 1; x < COLNO; x++)
                for (y = 0; y < ROWNO; y++)
                    if (levl[x][y].typ == SDOOR) {
                        cvt_sdoor_to_door(&levl[x][y]);
                        if (Is_rogue_level(&u.uz))
                            unblock_point(x, y);
                    }
            /* do_mapping() already reveals secret passages */
        }
        gk.known = TRUE;
    }

    if (svl.level.flags.nommap) {
#if 0 /*JP:T*/
        Your("%s spins as %s blocks the spell!", body_part(HEAD),
             something);
#else
            You("何かが呪文をさえぎり，目がまわった！");
#endif
        make_confused(HConfusion + rnd(30), FALSE);
        return;
    }
/*JP
    pline("A map coalesces in your mind!");
*/
    pline("地図があなたの心に融合した！");
    cval = (scursed && !confused);
    if (cval)
        HConfusion = 1; /* to screw up map */
    notice_mon_off();
    do_mapping();
    notice_mon_on();
    if (cval) {
        HConfusion = 0; /* restore */
/*JP
        pline("Unfortunately, you can't grasp the details.");
*/
        pline("残念ながら，あなたは詳細を得ることができなかった．");
    }
}

#ifdef MAIL_STRUCTURES
staticfn void
seffect_mail(struct obj **sobjp)
{
    struct obj *sobj = *sobjp;
    boolean odd = (sobj->o_id % 2) == 1;

    gk.known = TRUE;
    switch (sobj->spe) {
    case 2:
        /* "stamped scroll" created via magic marker--without a stamp */
#if 0 /*JP:T*/
        pline("This scroll is marked \"%s\".",
              odd ? "Postage Due" : "Return to Sender");
#else
        pline("この巻物には「料金不足」と書いてある．",
              odd ? "料金不足" : "差出人に返送");
#endif
        break;
    case 1:
        /* scroll of mail obtained from bones file or from wishing;
           note to the puzzled: the game Larn actually sends you junk
           mail if you win! */
#if 0 /*JP:T*/
        pline("This seems to be %s.",
              odd ? "a chain letter threatening your luck"
              : "junk mail addressed to the finder of the Eye of Larn");
#else
        pline("これは%sのようだ．",
              odd ? "あなたの幸運をおびやかすチェーンメイル"
              : "Eye of Larnの発見者に宛てられたゴミメイル");
#endif
        break;
    default:
#ifdef MAIL
        readmail(sobj);
#else
        /* unreachable since with MAIL undefined, sobj->spe won't be 0;
           as a precaution, be prepared to give arbitrary feedback;
           caller has already reported that it disappears upon reading */
        pline("That was a scroll of mail?");
#endif
        break;
    }
}
#endif /* MAIL_STRUCTURES */

/* scroll effects; return 1 if we use up the scroll and possibly make it
   become discovered, 0 if caller should take care of those side-effects */
int
seffects(
    struct obj *sobj) /* sobj - scroll or fake spellbook for spell */
{
    int otyp = sobj->otyp;

    if (objects[otyp].oc_magic)
        exercise(A_WIS, TRUE);                       /* just for trying */

    switch (otyp) {
#ifdef MAIL_STRUCTURES
    case SCR_MAIL:
        seffect_mail(&sobj);
        break;
#endif
    case SCR_ENCHANT_ARMOR:
        seffect_enchant_armor(&sobj);
        break;
    case SCR_DESTROY_ARMOR:
        seffect_destroy_armor(&sobj);
        break;
    case SCR_CONFUSE_MONSTER:
    case SPE_CONFUSE_MONSTER:
        seffect_confuse_monster(&sobj);
        break;
    case SCR_SCARE_MONSTER:
    case SPE_CAUSE_FEAR:
        seffect_scare_monster(&sobj);
        break;
    case SCR_BLANK_PAPER:
        seffect_blank_paper(&sobj);
        break;
    case SCR_REMOVE_CURSE:
    case SPE_REMOVE_CURSE:
        seffect_remove_curse(&sobj);
        break;
    case SCR_CREATE_MONSTER:
    case SPE_CREATE_MONSTER:
        seffect_create_monster(&sobj);
        break;
    case SCR_ENCHANT_WEAPON:
        seffect_enchant_weapon(&sobj);
        break;
    case SCR_TAMING:
    case SPE_CHARM_MONSTER:
        seffect_taming(&sobj);
        break;
    case SCR_GENOCIDE:
        seffect_genocide(&sobj);
        break;
    case SCR_LIGHT:
        seffect_light(&sobj);
        break;
    case SCR_TELEPORTATION:
        seffect_teleportation(&sobj);
        break;
    case SCR_GOLD_DETECTION:
        seffect_gold_detection(&sobj);
        break;
    case SCR_FOOD_DETECTION:
    case SPE_DETECT_FOOD:
        seffect_food_detection(&sobj);
        break;
    case SCR_IDENTIFY:
    case SPE_IDENTIFY:
        seffect_identify(&sobj);
        break;
    case SCR_CHARGING:
        seffect_charging(&sobj);
        break;
    case SCR_MAGIC_MAPPING:
    case SPE_MAGIC_MAPPING:
        seffect_magic_mapping(&sobj);
        break;
    case SCR_AMNESIA:
        seffect_amnesia(&sobj);
        break;
    case SCR_FIRE:
        seffect_fire(&sobj);
        break;
    case SCR_EARTH:
        seffect_earth(&sobj);
        break;
    case SCR_PUNISHMENT:
        seffect_punishment(&sobj);
        break;
    case SCR_STINKING_CLOUD:
        seffect_stinking_cloud(&sobj);
        break;
    default:
        impossible("What weird effect is this? (%u)", otyp);
    }
    /* if sobj is gone, we've already called useup() above and the
       update_inventory() that it performs might have come too soon
       (before charging an item, for instance) */
    if (!sobj)
        update_inventory();
    return sobj ? 0 : 1;
}

void
drop_boulder_on_player(
    boolean confused,
    boolean helmet_protects,
    boolean byu,
    boolean skip_uswallow)
{
    int dmg;
    struct obj *otmp2;

    /* hit monster if swallowed */
    if (u.uswallow && !skip_uswallow) {
        drop_boulder_on_monster(u.ux, u.uy, confused, byu);
        return;
    }

    otmp2 = mksobj(confused ? ROCK : BOULDER, FALSE, FALSE);
    if (!otmp2)
        return;
    otmp2->quan = confused ? rn1(5, 2) : 1;
    otmp2->owt = weight(otmp2);
    if (!amorphous(gy.youmonst.data) && !Passes_walls
        && !noncorporeal(gy.youmonst.data) && !unsolid(gy.youmonst.data)) {
/*JP
        You("are hit by %s!", doname(otmp2));
*/
        pline("%sが命中した！", doname(otmp2));
        dmg = (int) (dmgval(otmp2, &gy.youmonst) * otmp2->quan);
        if (uarmh && helmet_protects) {
            if (hard_helmet(uarmh)) {
/*JP
                pline("Fortunately, you are wearing a hard helmet.");
*/
                pline("幸運にも，あなたは固い兜を身につけている．");
                if (dmg > 2)
                    dmg = 2;
            } else if (flags.verbose) {
/*JP
                pline("%s does not protect you.", Yname2(uarmh));
*/
                Your("%sでは守れない．", xname(uarmh));
            }
        }
    } else
        dmg = 0;
    wake_nearto(u.ux, u.uy, 4 * 4);
    /* Must be before the losehp(), for bones files */
/*JP
    if (!flooreffects(otmp2, u.ux, u.uy, "fall")) {
*/
    if (!flooreffects(otmp2, u.ux, u.uy, "落ちる")) {
        place_object(otmp2, u.ux, u.uy);
        stackobj(otmp2);
        newsym(u.ux, u.uy);
    }
    if (dmg)
/*JP
        losehp(Maybe_Half_Phys(dmg), "scroll of earth", KILLED_BY_AN);
*/
        losehp(Maybe_Half_Phys(dmg), "大地の巻物で", KILLED_BY_AN);
}

boolean
drop_boulder_on_monster(coordxy x, coordxy y, boolean confused, boolean byu)
{
    struct obj *otmp2;
    struct monst *mtmp;

    /* Make the object(s) */
    otmp2 = mksobj(confused ? ROCK : BOULDER, FALSE, FALSE);
    if (!otmp2)
        return FALSE; /* Shouldn't happen */
    otmp2->quan = confused ? rn1(5, 2) : 1;
    otmp2->owt = weight(otmp2);

    /* Find the monster here (won't be player) */
    mtmp = m_at(x, y);
    if (mtmp && !amorphous(mtmp->data) && !passes_walls(mtmp->data)
        && !noncorporeal(mtmp->data) && !unsolid(mtmp->data)) {
        struct obj *helmet = which_armor(mtmp, W_ARMH);
        long mdmg;

        if (cansee(mtmp->mx, mtmp->my)) {
/*JP
            pline("%s is hit by %s!", Monnam(mtmp), doname(otmp2));
*/
            pline("%sが%sに命中した！", doname(otmp2), Monnam(mtmp));
            if (mtmp->minvis && !canspotmon(mtmp))
                map_invisible(mtmp->mx, mtmp->my);
        } else if (engulfing_u(mtmp))
#if 0 /*JP:T*/
            You_hear("something hit %s %s over your %s!",
                     s_suffix(mon_nam(mtmp)), mbodypart(mtmp, STOMACH),
                     body_part(HEAD));
#else
            You_hear("何かがあなたの%sの上の%sの%sに当たった音を聞いた！",
                     body_part(HEAD),
                     mon_nam(mtmp), mbodypart(mtmp, STOMACH));
#endif

        mdmg = dmgval(otmp2, mtmp) * otmp2->quan;
        if (helmet) {
            if (hard_helmet(helmet)) {
                if (canspotmon(mtmp))
/*JP
                    pline("Fortunately, %s is wearing a hard helmet.",
*/
                    pline("幸運にも，%sは固い兜を身につけている．",
                          mon_nam(mtmp));
                else if (!Deaf)
/*JP
                    You_hear("a clanging sound.");
*/
                    You_hear("ガランガランという音を聞いた．");
                if (mdmg > 2)
                    mdmg = 2;
            } else {
                if (canspotmon(mtmp))
#if 0 /*JP:T*/
                    pline("%s's %s does not protect %s.", Monnam(mtmp),
                          xname(helmet), mhim(mtmp));
#else
                    pline("%sの%sでは守れない．", Monnam(mtmp),
                          xname(helmet));
#endif
            }
        }
        mtmp->mhp -= mdmg;
        if (DEADMONSTER(mtmp)) {
            if (byu) {
                killed(mtmp);
            } else {
/*JP
                pline("%s is killed.", Monnam(mtmp));
*/
                pline("%sは死んだ．", Monnam(mtmp));
                mondied(mtmp);
            }
        } else {
            wakeup(mtmp, byu);
        }
        wake_nearto(x, y, 4 * 4);
    } else if (engulfing_u(mtmp)) {
        obfree(otmp2, (struct obj *) 0);
        /* fall through to player */
        drop_boulder_on_player(confused, TRUE, FALSE, TRUE);
        return 1;
    }
    /* Drop the rock/boulder to the floor */
/*JP
    if (!flooreffects(otmp2, x, y, "fall")) {
*/
    if (!flooreffects(otmp2, x, y, "落ちる")) {
        place_object(otmp2, x, y);
        stackobj(otmp2);
        newsym(x, y); /* map the rock */
    }
    return TRUE;
}

/* overcharging any wand or zapping/engraving cursed wand */
void
wand_explode(struct obj *obj, int chg /* recharging */)
{
/*JP
    const char *expl = !chg ? "suddenly" : "vibrates violently and";
*/
    const char *expl = !chg ? "突然" : "激しく振動し，";
    int dmg, n, k;

    /* number of damage dice */
    if (!chg)
        chg = 2; /* zap/engrave adjustment */
    n = obj->spe + chg;
    if (n < 2)
        n = 2; /* arbitrary minimum */
    /* size of damage dice */
    switch (obj->otyp) {
    case WAN_WISHING:
        k = 12;
        break;
    case WAN_CANCELLATION:
    case WAN_DEATH:
    case WAN_POLYMORPH:
    case WAN_UNDEAD_TURNING:
        k = 10;
        break;
    case WAN_COLD:
    case WAN_FIRE:
    case WAN_LIGHTNING:
    case WAN_MAGIC_MISSILE:
        k = 8;
        break;
    case WAN_NOTHING:
        k = 4;
        break;
    default:
        k = 6;
        break;
    }
    /* inflict damage and destroy the wand */
    dmg = d(n, k);
    obj->in_use = TRUE; /* in case losehp() is fatal (or --More--^C) */
#if 0 /*JP:T*/
    pline("%s %s explodes!", Yname2(obj), expl);
#else
    pline("%sは%s爆発した！", xname(obj), expl);
#endif
/*JP
    losehp(Maybe_Half_Phys(dmg), "exploding wand", KILLED_BY_AN);
*/
    losehp(Maybe_Half_Phys(dmg), "杖の爆発で", KILLED_BY_AN);
    useup(obj);
    /* obscure side-effect */
    exercise(A_STR, FALSE);
}

/* used to collect gremlins being hit by light so that they can be processed
   after vision for the entire lit area has been brought up to date */
struct litmon {
    struct monst *mon;
    struct litmon *nxt;
};
static struct litmon *gremlins = 0;

/*
 * Low-level lit-field update routine.
 */
staticfn void
set_lit(coordxy x, coordxy y, genericptr_t val)
{
    struct monst *mtmp;
    struct litmon *gremlin;

    if (val) {
        levl[x][y].lit = 1;
        if ((mtmp = m_at(x, y)) != 0 && mtmp->data == &mons[PM_GREMLIN]) {
            gremlin = (struct litmon *) alloc(sizeof *gremlin);
            gremlin->mon = mtmp;
            gremlin->nxt = gremlins;
            gremlins = gremlin;
        }
    } else {
        levl[x][y].lit = 0;
        snuff_light_source(x, y);
    }
}

void
litroom(
    boolean on,      /* True: make nearby area lit; False: cursed scroll */
    struct obj *obj) /* scroll, spellbook (for spell), or wand of light */
{
    struct obj *otmp, *nextobj;
    boolean blessed_effect = (obj && obj->oclass == SCROLL_CLASS
                              && obj->blessed);
    boolean no_op = (u.uswallow || Underwater || Is_waterlevel(&u.uz));
    char is_lit = 0; /* value is irrelevant but assign something anyway; its
                      * address is used as a 'not null' flag for set_lit() */

    /* update object lights and produce message (provided you're not blind) */
    if (!on) {
        int still_lit = 0;

        /*
         * The magic douses lamps,&c too and might curse artifact lights.
         *
         * FIXME?
         *  Shouldn't this affect all lit objects in the area of effect
         *  rather than just those carried by the hero?
         */
        for (otmp = gi.invent; otmp; otmp = nextobj) {
            nextobj = otmp->nobj;
            if (otmp->lamplit) {
                if (!artifact_light(otmp))
                    (void) snuff_lit(otmp);
                else
                    /* wielded Sunsword or worn gold dragon scales/mail;
                       maybe lower its BUC state if not already cursed */
                    impact_arti_light(otmp, TRUE, (boolean) !Blind);

                if (otmp->lamplit)
                    ++still_lit;
            }
        }
        /* scroll of light becomes discovered when not blind, so some
           message to justify that is needed */
        if (!Blind) {
            /* for the still_lit case, we don't know at this point whether
               anything currently visibly lit is going to go dark; if this
               message came after the darkening, we could count visibly
               lit squares before and after to know; we do know that being
               swallowed won't be affected--the interior is still lit */
            if (still_lit)
/*JP
                pline_The("ambient light seems dimmer.");
*/
                pline_The("周囲の光がより暗くなったように感じた．");
            else if (u.uswallow)
/*JP
                pline("It seems even darker in here than before.");
*/
                pline("前より暗くなったように見える．");
            else
/*JP
                You("are surrounded by darkness!");
*/
                You("暗闇に覆われた！");
        }
    } else { /* on */
        if (blessed_effect) {
            /* might bless artifact lights; no effect on ordinary lights */
            for (otmp = gi.invent; otmp; otmp = nextobj) {
                nextobj = otmp->nobj;
                if (otmp->lamplit && artifact_light(otmp))
                    /* wielded Sunsword or worn gold dragon scales/mail;
                       maybe raise its BUC state if not already blessed */
                    impact_arti_light(otmp, FALSE, (boolean) !Blind);
            }
        }
        if (u.uswallow) {
            if (Blind)
                ; /* no feedback */
            else if (digests(u.ustuck->data))
#if 0 /*JP:T*/
                pline("%s %s is lit.", s_suffix(Monnam(u.ustuck)),
                      mbodypart(u.ustuck, STOMACH));
#else
                pline("%sの%sは明るくなった．", Monnam(u.ustuck),
                      mbodypart(u.ustuck, STOMACH));
#endif
            else if (is_whirly(u.ustuck->data))
/*JP
                pline("%s shines briefly.", Monnam(u.ustuck));
*/
                pline("%sはちょっと輝いた．", Monnam(u.ustuck));
            else
/*JP
                pline("%s glistens.", Monnam(u.ustuck));
*/
                pline("%sはきらきら輝いた．", Monnam(u.ustuck));
        } else if (!Blind && (!Is_rogue_level(&u.uz)
                              || levl[u.ux][u.uy].typ != CORR)) {
/*JP
            pline("A lit field %ssurrounds you!", no_op ? "briefly " : "");
*/
            pline("灯りがあなたを%s取り囲んだ！", no_op ? "だいたい" : "");
        }
    }

    /* No-op when swallowed or in water */
    if (no_op)
        return;
    /*
     *  If we are darkening the room and the hero is punished but not
     *  blind, then we have to pick up and replace the ball and chain so
     *  that we don't remember them if they are out of sight.
     */
    if (Punished && !on && !Blind)
        move_bc(1, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);

    if (Is_rogue_level(&u.uz)) {
        /* Can't use do_clear_area because MAX_RADIUS is too small */
        /* rogue lighting must light the entire room */
        int rnum = levl[u.ux][u.uy].roomno - ROOMOFFSET;
        int rx, ry;

        if (rnum >= 0) {
            for (rx = svr.rooms[rnum].lx - 1; rx <= svr.rooms[rnum].hx + 1;
                 rx++)
                for (ry = svr.rooms[rnum].ly - 1;
                     ry <= svr.rooms[rnum].hy + 1; ry++)
                    set_lit(rx, ry,
                            (genericptr_t) (on ? &is_lit : (char *) 0));
            svr.rooms[rnum].rlit = on;
        }
        /* hallways remain dark on the rogue level */
    } else if (is_art(obj, ART_SUNSWORD)) {
        /* Sunsword's #invoke power directed up or down lights hero's spot
           (do_clear_area() rejects radius 0 so call set_lit() directly) */
        set_lit(u.ux, u.uy, (genericptr_t) &is_lit);
    } else {
        do_clear_area(u.ux, u.uy, blessed_effect ? 9 : 5,
                      set_lit, (genericptr_t) (on ? &is_lit : (char *) 0));
    }

    /*
     *  If we are not blind, then force a redraw on all positions in sight
     *  by temporarily blinding the hero.  The vision recalculation will
     *  correctly update all previously seen positions *and* correctly
     *  set the waslit bit [could be messed up from above].
     */
    if (!Blind) {
        vision_recalc(2);

        /* replace ball&chain */
        if (Punished && !on)
            move_bc(0, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);
    }

    gv.vision_full_recalc = 1; /* delayed vision recalculation */
    if (gremlins) {
        struct litmon *gremlin;

        /* can't delay vision recalc after all */
        vision_recalc(0);
        /* after vision has been updated, monsters who are affected
           when hit by light can now be hit by it */
        do {
            gremlin = gremlins;
            gremlins = gremlin->nxt;
            light_hits_gremlin(gremlin->mon, rnd(5));
            free((genericptr_t) gremlin);
        } while (gremlins);
    }
    return;
}

staticfn void
do_class_genocide(void)
{
    int i, j, immunecnt, gonecnt, goodcnt, class, feel_dead = 0;
    int ll_done = 0;
    char buf[BUFSZ], promptbuf[QBUFSZ];
    boolean gameover = FALSE; /* true iff killed self */

    buf[0] = '\0'; /* for EDIT_GETLIN */
    for (j = 0; ; j++) {
        if (j >= 5) {
            pline1(thats_enough_tries);
            return;
        }
/*JP
        Strcpy(promptbuf, "What class of monsters do you want to genocide?");
*/
        Strcpy(promptbuf, "どのクラスに属する怪物を虐殺しますか？");
        if (j > 0)
            Snprintf(eos(promptbuf), sizeof promptbuf - strlen(promptbuf),
                     " [enter %s]",
                     iflags.cmdassist
                       ? "the symbol or name representing a class, or '?'"
                       : "'?' to see previous genocides");
        getlin(promptbuf, buf);
        (void) mungspaces(buf);
        /* avoid 'that does not represent any monster' for empty input */
        if (!*buf) {
            pline("%s.", (j + 1 < 5)
                         ? "Type letter (or punctuation)"
                           " or name used for a class of monsters or 'none'"
                         /* next iteration gives "that's enough tries"
                            so don't suggest typing anything this time */
                         : "No class of monsters specified");
            continue; /* try again */
        }
        /* choosing "none" preserves genocideless conduct */
#if 0 /*JP:T*/
        if (*buf == '\033' || !strcmpi(buf, "none")
            || !strcmpi(buf, "'none'") || !strcmpi(buf, "nothing")) {
#else
        if (*buf == '\033' || !strcmpi(buf, "なし")
            || !strcmpi(buf, "「なし」") || !strcmpi(buf, "無し")) {
#endif
            livelog_printf(LL_GENOCIDE,
                           "declined to perform class genocide");
            return;
        }
        /* "?" runs #genocided to show existing genocides, then re-prompts;
           accept "'?'" too because the prompt's hint shows it that way */
        if (!strcmp(buf, "?") || !strcmp(buf, "'?'")) {
            list_genocided('g', FALSE);
            --j; /* don't count this iteration as one of the tries */
            continue;
        }

        class = name_to_monclass(buf, (int *) 0);
        if (class == 0 && (i = name_to_mon(buf, (int *) 0)) != NON_PM)
            class = mons[i].mlet;
        immunecnt = gonecnt = goodcnt = 0;
        for (i = LOW_PM; i < NUMMONS; i++) {
            if (mons[i].mlet == class) {
                if (!(mons[i].geno & G_GENO))
                    immunecnt++;
                else if (svm.mvitals[i].mvflags & G_GENOD)
                    gonecnt++;
                else
                    goodcnt++;
            }
        }
        if (!goodcnt && class != mons[gu.urole.mnum].mlet
            && class != mons[gu.urace.mnum].mlet) {
            if (gonecnt)
/*JP
                pline("All such monsters are already nonexistent.");
*/
                pline("そのような怪物はもういない．");
            else if (immunecnt || class == S_invisible)
/*JP
                You("aren't permitted to genocide such monsters.");
*/
                You("その怪物を虐殺することはできない．");
            else if (wizard && buf[0] == '*') {
                struct monst *mtmp, *mtmp2;

                gonecnt = 0;
                for (mtmp = fmon; mtmp; mtmp = mtmp2) {
                    mtmp2 = mtmp->nmon;
                    if (DEADMONSTER(mtmp))
                        continue;
                    mongone(mtmp);
                    gonecnt++;
                }
/*JP
                pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
*/
                pline("%dの怪物を除いた．", gonecnt);
                return;
            } else
#if 0 /*JP*/
                pline("That %s does not represent any monster.",
                      strlen(buf) == 1 ? "symbol" : "response");
#else
                pline("そのような怪物はいない．");
#endif
            continue;
        }

        for (i = LOW_PM; i < NUMMONS; i++) {
            if (mons[i].mlet == class) {
                char nam[BUFSZ];

                Strcpy(nam, makeplural(mons[i].pmnames[NEUTRAL]));
                /* Although "genus" is Latin for race, the hero benefits
                 * from both race and role; thus genocide affects either.
                 */
                if (Your_Own_Role(i) || Your_Own_Race(i)
                    || ((mons[i].geno & G_GENO)
                        && !(svm.mvitals[i].mvflags & G_GENOD))) {
                    /* This check must be first since player monsters might
                     * have G_GENOD or !G_GENO.
                     */
                    if (!ll_done++) {
                        if (!num_genocides())
                            livelog_printf(LL_CONDUCT | LL_GENOCIDE,
                                     "performed %s first genocide (class %c)",
                                           uhis(), def_monsyms[class].sym);
                        else
                            livelog_printf(LL_GENOCIDE, "genocided class %c",
                                           def_monsyms[class].sym);
                    }

                    svm.mvitals[i].mvflags |= (G_GENOD | G_NOCORPSE);
                    kill_genocided_monsters();
                    update_inventory(); /* eggs & tins */
/*JP
                    pline("Wiped out all %s.", nam);
*/
                    pline("%sを全て排除した．", nam);
                    if (Upolyd && vampshifted(&gy.youmonst)
                        /* current shifted form or base vampire form */
                        && (i == u.umonnum || i == gy.youmonst.cham))
                        polyself(POLY_REVERT); /* vampshifter to vampire */
                    if (Upolyd && i == u.umonnum) {
                        u.mh = -1;
                        if (Unchanging) {
                            if (!feel_dead++)
/*JP
                                urgent_pline("You die.");
*/
                                urgent_pline("あなたは死にました．．．");
                            /* finish genociding this class of
                               monsters before ultimately dying */
                            gameover = TRUE;
                        } else
                            rehumanize();
                    }
                    /* Self-genocide if it matches either your race
                       or role.  Assumption:  male and female forms
                       share same monster class. */
                    if (i == gu.urole.mnum || i == gu.urace.mnum) {
                        u.uhp = -1;
                        if (Upolyd) {
                            if (!feel_dead++)
/*JP
                                You_feel("%s inside.", udeadinside());
*/
                                You("魂が%sような気がした．", udeadinside());
                        } else {
                            if (!feel_dead++)
/*JP
                                urgent_pline("You die.");
*/
                                urgent_pline("あなたは死にました．．．");
                            gameover = TRUE;
                        }
                    }
                } else if (svm.mvitals[i].mvflags & G_GENOD) {
                    if (!gameover)
/*JP
                        pline("%s are already nonexistent.", upstart(nam));
*/
                        pline("%sは既にいない．", nam);
                } else if (!gameover) {
                    /* suppress feedback about quest beings except
                       for those applicable to our own role */
                    if ((mons[i].msound != MS_LEADER
                         || quest_info(MS_LEADER) == i)
                        && (mons[i].msound != MS_NEMESIS
                            || quest_info(MS_NEMESIS) == i)
                        && (mons[i].msound != MS_GUARDIAN
                            || quest_info(MS_GUARDIAN) == i)
                        /* non-leader/nemesis/guardian role-specific monster
                           */
                        && (i != PM_NINJA /* nuisance */
                            || Role_if(PM_SAMURAI))) {
                        boolean named, uniq;

                        named = type_is_pname(&mons[i]) ? TRUE : FALSE;
                        uniq = (mons[i].geno & G_UNIQ) ? TRUE : FALSE;
                        /* one special case */
                        if (i == PM_HIGH_CLERIC)
                            uniq = FALSE;

#if 0 /*JP*/
                        You("aren't permitted to genocide %s%s.",
                            (uniq && !named) ? "the " : "",
                            (uniq || named) ? mons[i].pmnames[NEUTRAL] : nam);
#else
                        You("%sを虐殺できない．",
                            (uniq || named) ? mons[i].pmnames[NEUTRAL] : nam);
#endif
                    }
                }
            }
        }
        if (gameover || u.uhp == -1) {
            svk.killer.format = KILLED_BY_AN;
/*JP
            Strcpy(svk.killer.name, "scroll of genocide");
*/
            Strcpy(svk.killer.name, "虐殺の巻物で");
            if (gameover)
                done(GENOCIDED);
        }
        return;
    }
}

#define REALLY 1
#define PLAYER 2
#define ONTHRONE 4
void
do_genocide(
    int how) /* 0 = no genocide; create monsters (cursed scroll)
              * 1 = normal genocide
              * 3 = forced genocide of player
              * 5 (4 | 1) = normal genocide from throne */
{
    char buf[BUFSZ], realbuf[BUFSZ], promptbuf[QBUFSZ];
    int i, killplayer = 0;
    int mndx;
    struct permonst *ptr;
    const char *which;

    if (how & PLAYER) {
        mndx = u.umonster; /* non-polymorphed mon num */
        ptr = &mons[mndx];
        Strcpy(buf, pmname(ptr, Ugender));
        killplayer++;
    } else {
        buf[0] = '\0'; /* init for EDIT_GETLIN */
        for (i = 0; ; i++) {
            if (i >= 5) {
                /* cursed effect => no free pass (unless rndmonst() fails) */
                if (!(how & REALLY) && (ptr = rndmonst()) != 0)
                    break;

                pline1(thats_enough_tries);
                return;
            }
#if 0 /*JP:T*/
            Strcpy(promptbuf,
                   "What type of monster do you want to genocide?");
#else
            Strcpy(promptbuf,
                   "どの種類の怪物を虐殺しますか？");
#endif
            if (i > 0)
#if 0 /*JP:T*/
                Snprintf(eos(promptbuf), sizeof promptbuf - strlen(promptbuf),
                         " [enter %s]",
                         iflags.cmdassist
                           ? "the name of a type of monster, or '?'"
                           : "'?' to see previous genocides");
#else
                Snprintf(eos(promptbuf), sizeof promptbuf - strlen(promptbuf),
                         " [%s]",
                         iflags.cmdassist
                           ? "怪物の種類の名前か'?'を入れる"
                           : "前に虐殺したものを見るには'?'");
#endif
            getlin(promptbuf, buf);
            (void) mungspaces(buf);
            /* avoid 'such creatures do not exist' for empty input */
            if (!*buf) {
#if 0 /*JP:T*/
                pline("%s.", (i + 1 < 5)
                             ? "Type the name of a type of monster or 'none'"
                             /* next iteration gives "that's enough tries"
                                so don't suggest typing anything this time */
                             : "No type of monster specified");
#else
                pline("%s．", (i + 1 < 5)
                             ? "怪物の種類の名前か，「なし」を入れる"
                             /* next iteration gives "that's enough tries"
                                so don't suggest typing anything this time */
                             : "怪物の種類が指定されていない");
#endif
                continue; /* try again */
            }
            /* choosing "none" preserves genocideless conduct */
#if 0 /*JP:T*/
            if (*buf == '\033' || !strcmpi(buf, "none")
                || !strcmpi(buf, "'none'") || !strcmpi(buf, "nothing")) {
#else
            if (*buf == '\033' || !strcmpi(buf, "なし")
                || !strcmpi(buf, "「なし」") || !strcmpi(buf, "無し")) {
#endif
                /* ... but no free pass if cursed */
                if (!(how & REALLY) && (ptr = rndmonst()) != 0)
                    break; /* remaining checks don't apply */

                livelog_printf(LL_GENOCIDE, "declined to perform genocide");
                return;
            }
            /* "?" or "'?'" runs #genocided to show existing genocides */
            if (!strcmp(buf, "?") || !strcmp(buf, "'?'")) {
                list_genocided('g', FALSE);
                --i; /* don't count this iteration as one of the tries */
                continue;
            }

            mndx = name_to_mon(buf, (int *) 0);
            if (mndx == NON_PM || (svm.mvitals[mndx].mvflags & G_GENOD)) {
#if 0 /*JP*/
                pline("Such creatures %s exist in this world.",
                      (mndx == NON_PM) ? "do not" : "no longer");
#else
                pline("そのような生き物は%sこの世界に存在しない．",
                      (mndx == NON_PM) ? "" : "もはや");
#endif
                continue;
            }
            ptr = &mons[mndx];
            /* first revert if current shifted form or base vampire form */
            if (Upolyd && vampshifted(&gy.youmonst)
                && (mndx == u.umonnum || mndx == gy.youmonst.cham))
                polyself(POLY_REVERT); /* vampshifter (bat, &c) to vampire */
            /* Although "genus" is Latin for race, the hero benefits
             * from both race and role; thus genocide affects either.
             */
            if (Your_Own_Role(mndx) || Your_Own_Race(mndx)) {
                killplayer++;
                break;
            }
            if (is_human(ptr))
                adjalign(-sgn(u.ualign.type));
            if (is_demon(ptr))
                adjalign(sgn(u.ualign.type));

            if (!(ptr->geno & G_GENO)) {
                if (!Deaf) {
                    /* FIXME: unconditional "caverns" will be silly in some
                     * circumstances.  Who's speaking?  Divine pronouncements
                     * aren't supposed to be hampered by deafness....
                     */
                    if (flags.verbose)
#if 0 /*JP:T*/
                        pline("A thunderous voice booms"
                              " through the caverns:");
#else
                        pline("雷のような声が"
                              "洞窟に響いた：");
#endif
                    SetVoice((struct monst *) 0, 0, 80, voice_deity);
                    /* FIXME? shouldn't this override deafness? */
/*JP
                    verbalize("No, mortal!  That will not be done.");
*/
                    pline("「定命の者よ！その望みはかなうまい．」");
                }
                continue;
            }
            /* KMH -- Unchanging prevents rehumanization */
            if (Unchanging && ptr == gy.youmonst.data)
                killplayer++;
            break;
        }
        mndx = monsndx(ptr); /* needed for the 'no free pass' cases */
    }

/*JP
    which = "all ";
*/
    which = "全て";
    Strcpy(realbuf, ptr->pmnames[NEUTRAL]); /* standard singular */
    if (Hallucination) {
        /* hallucinate hero's type */
        if (Upolyd) {
            Strcpy(buf, pmname(gy.youmonst.data,
                               flags.female ? FEMALE : MALE));
        } else {
            Strcpy(buf, (flags.female && gu.urole.name.f) ? gu.urole.name.f
                                                          : gu.urole.name.m);
            buf[0] = lowc(buf[0]);
        }
    } else {
        /* use actual type */
        Strcpy(buf, realbuf);
        if ((ptr->geno & G_UNIQ) && ptr != &mons[PM_HIGH_CLERIC])
#if 0 /*JP*/
            which = !type_is_pname(ptr) ? "the " : "";
#else
            which = "";
#endif
    }

    if (how & REALLY) {
        if (!num_genocides())
            livelog_printf(LL_CONDUCT | LL_GENOCIDE,
                           "performed %s first genocide (%s)",
                           uhis(), makeplural(realbuf));
        else
            livelog_printf(LL_GENOCIDE, "genocided %s", makeplural(realbuf));

        /* setting no-corpse affects wishing and random tin generation */
        svm.mvitals[mndx].mvflags |= (G_GENOD | G_NOCORPSE);
#if 0 /*JP:T*/
        pline("Wiped out %s%s.", which,
              (*which != 'a') ? buf : makeplural(buf));
#else
        pline("%sを%s一掃した．", buf, which);
#endif

        if (killplayer) {
            u.uhp = -1;
            if (how & PLAYER) {
                svk.killer.format = KILLED_BY;
/*JP
                Strcpy(svk.killer.name, "genocidal confusion");
*/
                Strcpy(svk.killer.name, "混乱による自虐的虐殺で");
            } else if (how & ONTHRONE) {
                /* player selected while on a throne */
                svk.killer.format = KILLED_BY_AN;
/*JP
                Strcpy(svk.killer.name, "imperious order");
*/
                Strcpy(svk.killer.name, "傲慢な命令で");
            } else { /* selected player deliberately, not confused */
                svk.killer.format = KILLED_BY_AN;
/*JP
                Strcpy(svk.killer.name, "scroll of genocide");
*/
                Strcpy(svk.killer.name, "虐殺の巻物で");
            }

            /* Polymorphed characters will die as soon as they're rehumanized.
               KMH -- Unchanging prevents rehumanization. */
            if (Upolyd && ptr != gy.youmonst.data) {
                delayed_killer(POLYMORPH, svk.killer.format, svk.killer.name);
/*JP
                You_feel("%s inside.", udeadinside());
*/
                You_feel("魂が%sような気がした．", udeadinside());
            } else {
                done(GENOCIDED);
            }
        } else if (ptr == gy.youmonst.data) {
            rehumanize();
        }
        kill_genocided_monsters();
        update_inventory(); /* in case identified eggs were affected */
    } else {
        int cnt = 0, census = monster_census(FALSE);

        if (!(mons[mndx].geno & G_UNIQ)
            && !(svm.mvitals[mndx].mvflags & (G_GENOD | G_EXTINCT)))
            for (i = rn1(3, 4); i > 0; i--) {
                if (!makemon(ptr, u.ux, u.uy, NO_MINVENT | MM_NOMSG))
                    break; /* couldn't make one */
                ++cnt;
                if (svm.mvitals[mndx].mvflags & G_EXTINCT)
                    break; /* just made last one */
            }
        if (cnt) {
            /* accumulated 'cnt' doesn't take groups into account;
               assume bringing in new mon(s) didn't remove any old ones */
            cnt = monster_census(FALSE) - census;
#if 0 /*JP:T*/
            pline("Sent in %s%s.", (cnt > 1) ? "some " : "",
                  (cnt > 1) ? makeplural(buf) : an(buf));
#else
            pline("%sが送られてきた．", buf);
#endif
        } else
            pline1(nothing_happens);
    }
}

void
punish(struct obj *sobj)
{
    /* angrygods() calls this with NULL sobj arg */
    struct obj *reuse_ball = (sobj && sobj->otyp == HEAVY_IRON_BALL)
                                ? sobj : (struct obj *) 0;
    /* analyzer doesn't know that the one caller that passes a NULL
     * sobj (angrygods) checks !Punished first, so add a guard */
    int cursed_levy = (sobj && sobj->cursed) ? 1 : 0;

    /* KMH -- Punishment is still okay when you are riding */
    if (!reuse_ball)
/*JP
        You("are being punished for your misbehavior!");
*/
        You("不作法のため罰を受けた！");
    if (Punished) {
/*JP
        Your("iron ball gets heavier.");
*/
        Your("鉄球はさらに重くなった．");
        uball->owt += WT_IRON_BALL_INCR * (1 + cursed_levy);
        return;
    }
    if (amorphous(gy.youmonst.data) || is_whirly(gy.youmonst.data)
        || unsolid(gy.youmonst.data)) {
        if (!reuse_ball) {
/*JP
            pline("A ball and chain appears, then falls away.");
*/
            pline("鉄球と鎖が現われたが，するっと抜けた．");
            dropy(mkobj(BALL_CLASS, TRUE));
        } else {
            dropy(reuse_ball);
        }
        return;
    }
    setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
    if (!reuse_ball)
        setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
    else
        setworn(reuse_ball, W_BALL);

    /*
     *  Place ball & chain if not swallowed.  If swallowed, the ball & chain
     *  variables will be set at the next call to placebc().
     */
    if (!u.uswallow) {
        placebc();
        if (Blind)
            set_bc(1);      /* set up ball and chain variables */
        newsym(u.ux, u.uy); /* see ball&chain if can't see self */
    }
}

/* remove the ball and chain */
void
unpunish(void)
{
    struct obj *savechain = uchain;

    /* chain goes away */
    setworn((struct obj *) 0, W_CHAIN); /* sets 'uchain' to Null */
    /* for floor, unhides monster hidden under chain, calls newsym() */
    delobj(savechain);

    /* the chain is gone but the no longer attached ball persists */
    setworn((struct obj *) 0, W_BALL); /* sets 'uball' to Null */
}

/* prompt the player to create a stinking cloud and then create it if they
   give a location */
staticfn void
do_stinking_cloud(struct obj *sobj, boolean mention_stinking)
{
    coord cc;

#if 0 /*JP:T*/
    pline("Where do you want to center the %scloud?",
          mention_stinking ? "stinking " : "");
#else
    pline("%s雲の中心をどこにしますか？",
          mention_stinking ? "悪臭 " : "");
#endif
    cc.x = u.ux;
    cc.y = u.uy;
    getpos_sethilite(display_stinking_cloud_positions, can_center_cloud);
/*JP
    if (getpos(&cc, TRUE, "the desired position") < 0) {
*/
    if (getpos(&cc, TRUE, "狙う場所") < 0) {
        pline1(Never_mind);
        return;
    } else if (!can_center_cloud(cc.x, cc.y)) {
        if (Hallucination)
/*JP
            pline("Ugh... someone cut the cheese.");
*/
            pline("うっ．．．誰かがおならをした．");
        else
#if 0 /*JP:T*/
            pline("%s a whiff of rotten eggs.",
                  sobj->oclass == SCROLL_CLASS ? "The scroll crumbles with"
                                               : "You smell");
#else
            pline(sobj->oclass == SCROLL_CLASS ?
                    "巻物はくさった卵のにおいをさせながら粉々になった．"
                  : "くさった卵のにおいがする．");
#endif
        return;
    }
    (void) create_gas_cloud(cc.x, cc.y, 15 + 10 * bcsign(sobj),
                            8 + 4 * bcsign(sobj));
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to
 * revive one, the disoriented creature becomes a zombie
 */
boolean
cant_revive(
    int *mtype,
    boolean revival,
    struct obj *from_obj)
{
    /* SHOPKEEPERS can be revived now */
    if (*mtype == PM_GUARD || (*mtype == PM_SHOPKEEPER && !revival)
        || *mtype == PM_HIGH_CLERIC || *mtype == PM_ALIGNED_CLERIC
        || *mtype == PM_ANGEL) {
        *mtype = PM_HUMAN_ZOMBIE;
        return TRUE;
    } else if (*mtype == PM_LONG_WORM_TAIL) { /* for create_particular() */
        *mtype = PM_LONG_WORM;
        return TRUE;
    } else if (unique_corpstat(&mons[*mtype])
               && (!from_obj || !has_omonst(from_obj))) {
        /* unique corpses (from bones or wizard mode wish) or
           statues (bones or any wish) end up as shapechangers */
        *mtype = PM_DOPPELGANGER;
        return TRUE;
    }
    return FALSE;
}

staticfn boolean
create_particular_parse(
    char *str,
    struct _create_particular_data *d)
{
    int gender_name_var = NEUTRAL;
    char *bufp = str;
    char *tmpp;
#if 1 /*JP*/
    size_t l;
#endif

    d->quan = 1 + ((gm.multi > 0) ? (int) gm.multi : 0);
    d->monclass = MAXMCLASSES;
    d->which = gu.urole.mnum; /* an arbitrary index into mons[] */
    d->fem = -1;     /* gender not specified */
    d->genderconf = -1;  /* no confusion on which gender to assign */
    d->randmonst = FALSE;
    d->maketame = d->makepeaceful = d->makehostile = FALSE;
    d->sleeping = d->saddled = d->invisible = d->hidden = FALSE;

    /* quantity */
    if (digit(*bufp)) {
        d->quan = atoi(bufp);
        while (digit(*bufp))
            bufp++;
        while (*bufp == ' ')
            bufp++;
    }
#define QUAN_LIMIT (ROWNO * (COLNO - 1))
    /* maximum possible quantity is one per cell: (0..ROWNO-1) x (1..COLNO-1)
       [21*79==1659 for default map size; could subtract 1 for hero's spot] */
    if (d->quan < 1 || d->quan > QUAN_LIMIT)
        d->quan = QUAN_LIMIT - monster_census(FALSE);
#undef QUAN_LIMIT
    /* gear -- extremely limited number of possibilities supported */
    if ((tmpp = strstri(bufp, "saddled ")) != 0) {
        d->saddled = TRUE;
        (void) memset(tmpp, ' ', sizeof "saddled " - 1);
    }
    /* state -- limited number of possibilities supported */
    if ((tmpp = strstri(bufp, "sleeping ")) != 0) {
        d->sleeping = TRUE;
        (void) memset(tmpp, ' ', sizeof "sleeping " - 1);
    }
    if ((tmpp = strstri(bufp, "invisible ")) != 0) {
        d->invisible = TRUE;
        (void) memset(tmpp, ' ', sizeof "invisible " - 1);
    }
    if ((tmpp = strstri(bufp, "hidden ")) != 0) {
        d->hidden = TRUE;
        (void) memset(tmpp, ' ', sizeof "hidden " - 1);
    }
    /* check "female" before "male" to avoid false hit mid-word */
    if ((tmpp = strstri(bufp, "female ")) != 0) {
        d->fem = 1;
        (void) memset(tmpp, ' ', sizeof "female " - 1);
    }
    if ((tmpp = strstri(bufp, "male ")) != 0) {
        d->fem = 0;
        (void) memset(tmpp, ' ', sizeof "male " - 1);
    }
    bufp = mungspaces(bufp); /* after potential memset(' ') */
    /* allow the initial disposition to be specified */
#if 0 /*JP:T*/
    if (!strncmpi(bufp, "tame ", 5)) {
        bufp += 5;
#else
    if (!STRNCMPEX(bufp, "手なずけられた")) {
        bufp += l;
#endif
        d->maketame = TRUE;
#if 0 /*JP:T*/
    } else if (!strncmpi(bufp, "peaceful ", 9)) {
        bufp += 9;
#else
    } else if (!STRNCMPEX(bufp, "友好的な")) {
        bufp += l;
#endif
        d->makepeaceful = TRUE;
#if 0 /*JP:T*/
    } else if (!strncmpi(bufp, "hostile ", 8)) {
        bufp += 8;
#else
    } else if (!STRNCMPEX(bufp, "敵対的な")) {
        bufp += l;
#endif
        d->makehostile = TRUE;
    }
    /* decide whether a valid monster was chosen */
/*JP
    if (wizard && (!strcmp(bufp, "*") || !strcmp(bufp, "random"))) {
*/
    if (wizard && (!strcmp(bufp, "*") || !strcmp(bufp, "ランダム"))) {
        d->randmonst = TRUE;
        return TRUE;
    }
    d->which = name_to_mon(bufp, &gender_name_var);
    /*
     * With the introduction of male and female monster names
     * in 5.0, preserve that detail.
     *
     * If d->fem is already set to MALE or FEMALE at this juncture, it means
     * one of those terms was explicitly specified.
     */
    if (d->fem == MALE || d->fem == FEMALE) {     /* explicitly expressed */
        if ((gender_name_var != NEUTRAL) && (d->fem != gender_name_var)) {
            /* apparent selection incompatibility */
            d->genderconf = gender_name_var;        /* resolve later */
        }
        /* otherwise keep the value of d->fem, as it's okay */
    } else {  /* no explicit gender term was specified */
        d->fem = gender_name_var;
    }
    if (ismnum(d->which))
        return TRUE; /* got one */
    d->monclass = name_to_monclass(bufp, &d->which);

    if (ismnum(d->which)) {
        d->monclass = MAXMCLASSES; /* matters below */
        return TRUE;
    } else if (d->monclass == S_invisible) { /* not an actual monster class */
        d->which = PM_STALKER;
        d->monclass = MAXMCLASSES;
        return TRUE;
    } else if (d->monclass == S_WORM_TAIL) { /* empty monster class */
        d->which = PM_LONG_WORM;
        d->monclass = MAXMCLASSES;
        return TRUE;
    } else if (d->monclass > 0) {
        d->which = gu.urole.mnum; /* reset from NON_PM */
        return TRUE;
    }
    return FALSE;
}

staticfn boolean
create_particular_creation(
    struct _create_particular_data *d)
{
    struct permonst *whichpm = NULL;
    int i, mx, my, firstchoice = NON_PM;
    struct monst *mtmp;
    boolean madeany = FALSE;

    if (!d->randmonst) {
        firstchoice = d->which;
        if (cant_revive(&d->which, FALSE, (struct obj *) 0)
            && firstchoice != PM_LONG_WORM_TAIL) {
            /* wizard mode can override handling of special monsters */
            char buf[BUFSZ];

#if 0 /*JP:T*/
            Sprintf(buf, "Creating %s instead; force %s?",
                    mons[d->which].pmnames[NEUTRAL],
                    mons[firstchoice].pmnames[NEUTRAL]);
#else
            Sprintf(buf, "代わりに%sが作られます；%sにする？",
                    mons[d->which].pmnames[NEUTRAL],
                    mons[firstchoice].pmnames[NEUTRAL]);
#endif
            if (y_n(buf) == 'y')
                d->which = firstchoice;
        }
        whichpm = &mons[d->which];
    }
    for (i = 0; i < d->quan; i++) {
        mmflags_nht mmflags = NO_MM_FLAGS;

        if (d->monclass != MAXMCLASSES)
            whichpm = mkclass(d->monclass, 0);
        else if (d->randmonst)
            whichpm = rndmonst();
        if (d->genderconf == -1) {
            /* no conflict exists between explicit gender term and
               the specified monster name */
            if (d->fem != -1 && (!whichpm || (!is_male(whichpm)
                                              && !is_female(whichpm))))
                mmflags |= (d->fem == FEMALE) ? MM_FEMALE
                               : (d->fem == MALE) ? MM_MALE : 0;
            /* no surprise; "<mon> appears." rather than "<mon> appears!" */
            mmflags |= MM_NOEXCLAM;
        } else {
            /* conundrum alert: an explicit gender term conflicts with an
               explicit gender-tied naming term (i.e. male cavewoman) */

            /* option not gone with: name overrides the explicit gender as
               commented out here */
            /*  d->fem = d->genderconf; */

            /* option chosen: let the explicit gender term (male or female)
               override the gender-tied naming term, so leave d->fem as-is */

            mmflags |= (d->fem == FEMALE) ? MM_FEMALE
                           : (d->fem == MALE) ? MM_MALE : 0;

            /* another option would be to consider it a faulty specification
               and reject the request completely and produce a random monster
               with a gender matching that specified instead (i.e. there is
               no such thing as a male cavewoman) */
            /* whichpm = rndmonst(); */
            /* mmflags |= (d->fem == FEMALE) ? MM_FEMALE : MM_MALE; */
        }
        if (d->invisible)
            mmflags |= MM_MINVIS;

        mtmp = makemon(whichpm, u.ux, u.uy, mmflags);
        if (!mtmp) {
            /* quit trying if creation failed and is going to repeat */
            if (d->monclass == MAXMCLASSES && !d->randmonst)
                break;
            /* otherwise try again */
            continue;
        }
        mx = mtmp->mx, my = mtmp->my;
        if (d->maketame) {
            (void) tamedog(mtmp, (struct obj *) 0, FALSE);
        } else if (d->makepeaceful || d->makehostile) {
            mtmp->mtame = 0; /* sanity precaution */
            mtmp->mpeaceful = d->makepeaceful ? 1 : 0;
            set_malign(mtmp);
        }
        if (d->saddled && can_saddle(mtmp) && !which_armor(mtmp, W_SADDLE)) {
            /* NULL obj arg means put_saddle_on_mon()
             * will create the saddle itself */
            put_saddle_on_mon((struct obj *) 0, mtmp);
        }
        if (d->hidden
           && ((is_hider(mtmp->data) && mtmp->data->mlet != S_MIMIC)
               || (hides_under(mtmp->data) && OBJ_AT(mx, my))
               || (mtmp->data->mlet == S_EEL && is_pool(mx, my))))
            mtmp->mundetected = 1;
        if (d->sleeping)
            mtmp->msleeping = 1;
        /* if asking for 'hidden', show location of every created monster
           that can't be seen--whether that's due to successfully hiding
           or vision issues (line-of-sight, invisibility, blindness) */
        if ((d->hidden || d->invisible) && !canspotmon(mtmp))
            flash_mon(mtmp);

        madeany = TRUE;
        /* in case we got a doppelganger instead of what was asked
           for, make it start out looking like what was asked for */
        if (mtmp->cham != NON_PM && firstchoice != NON_PM
            && mtmp->cham != firstchoice)
            (void) newcham(mtmp, &mons[firstchoice], NO_NC_FLAGS);
    }
    return madeany;
}

/*
 * Make a new monster with the type controlled by the user.
 *
 * Note:  when creating a monster by class letter, specifying the
 * "strange object" (']') symbol produces a random monster rather
 * than a mimic.  This behavior quirk is useful so don't "fix" it
 * (use 'm'--or "mimic"--to create a random mimic).
 *
 * Used in wizard mode only (for ^G command and for scroll or spell
 * of create monster).  Once upon a time, an earlier incarnation of
 * this code was also used for the scroll/spell in explore mode.
 */
boolean
create_particular(void)
{
#define CP_TRYLIM 5
    struct _create_particular_data d;
    char *bufp, buf[BUFSZ], prompt[QBUFSZ];
    int  tryct = CP_TRYLIM, altmsg = 0;

    buf[0] = '\0'; /* for EDIT_GETLIN */
/*JP
    Strcpy(prompt, "Create what kind of monster?");
*/
    Strcpy(prompt, "どの種の怪物を作りますか？");
    do {
        getlin(prompt, buf);
        bufp = mungspaces(buf);
        if (*bufp == '\033')
            return FALSE;

        if (create_particular_parse(bufp, &d))
            break;

        /* no good; try again... */
        if (*bufp || altmsg || tryct < 2) {
/*JP
            pline("I've never heard of such monsters.");
*/
            pline("そのような怪物は聞いたことがない．");
        } else {
/*JP
            pline("Try again (type * for random, ESC to cancel).");
*/
            pline("もう一度どうぞ(ランダムは*，キャンセルはESC)．");
            ++altmsg;
        }
        /* when a second try is needed, expand the prompt */
        if (tryct == CP_TRYLIM)
            Strcat(prompt, " [type name or symbol]");
    } while (--tryct > 0);

    if (!tryct)
        pline1(thats_enough_tries);
    else
        return create_particular_creation(&d);

    return FALSE;
}

/*read.c*/
