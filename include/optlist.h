/* NetHack 5.0	optlist.h */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OPTLIST_H
#define OPTLIST_H

/*
 *  NOTE:  If you add (or delete) an option, please review:
 *             doc/options.txt
 *
 *         It contains how-to info and outlines some required/suggested
 *         updates that should accompany your change.
 */

#define BACKWARD_COMPAT

extern int optfn_boolean(int, int, boolean, char *, char *);

enum OptType { BoolOpt, CompOpt, OthrOpt };
enum Y_N { No, Yes };
enum Off_On { Off, On };
/* Advanced options are only shown in the full, traditional options menu */
enum OptSection {
    OptS_General, OptS_Behavior, OptS_Map, OptS_Status, OptS_Advanced
};
enum menu_terminology_preference {
    Term_False, Term_Off, Term_Disabled, Term_Excluded, num_terms
};

struct allopt_t {
    const char *name;
    enum OptSection section;
    int minmatch;
    int expectedbuf;
    int idx;
    enum optset_restrictions setwhere;
    enum OptType opttyp;
    enum Y_N negateok;
    enum Y_N valok;
    enum Y_N dupeok;
    enum Y_N pfx;
    enum menu_terminology_preference termpref;
    boolean opt_in_out, *addr;
    int (*optfn)(int, int, boolean, char *, char *);
    const char *alias;
    const char *descr;
    const char *prefixgw;
    boolean initval, has_handler, dupdetected, disregarded;
};

#endif /* OPTLIST_H */

#if defined(NHOPT_PROTO) || defined(NHOPT_ENUM) || defined(NHOPT_PARSE)
/* clang-format off */
/* *INDENT-OFF* */

#define NoAlias ((const char *) 0)

#if defined(NHOPT_PROTO)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc) /*empty*/
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z)               \
static int optfn_##a(int, int, boolean, char *, char *);
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z)               \
static int pfxfn_##a(int, int, boolean, char *, char *);
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z)               \
static int optfn_##a(int, int, boolean, char *, char *);

#elif defined(NHOPT_ENUM)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc) opt_##a,
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z)   opt_##a,
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z)   pfx_##a,
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z)   opt_##a,

#elif defined(NHOPT_PARSE)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc)             \
    { #a, OptS_##sec, 0, b, opt_##a, s, BoolOpt, n, v, d, No, termp, c,  \
      bp, &optfn_boolean, al, desc, (const char *) 0, i, 0, 0 , 0 },
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z) \
    { #a, OptS_##sec, 0, b, opt_##a, s, CompOpt, n, v, d, No, 0, c,  \
      (boolean *) 0, &optfn_##a, al, z, (const char *) 0, Off, h, 0, 0 },
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z) \
    { #a, OptS_##sec, 0, b, pfx_##a, s, CompOpt, n, v, d, Yes, 0, c, \
      (boolean *) 0, &pfxfn_##a, al, z, #a, Off, h, 0, 0 },
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z) \
    { m, OptS_##sec, 0, b, opt_##a, s, OthrOpt, n, v, d, No, 0, c,   \
      (boolean *) 0, &optfn_##a, al, z, (const char *) 0, On, On, 0, 0 },

/* this is not reliable because TILES_IN_GLYPHMAP might be defined
 * in a multi-interface binary but not apply to the current interface */
#ifdef TILES_IN_GLYPHMAP
#define tiled_map_Def On
#define ascii_map_Def Off
#else
#define ascii_map_Def On
#define tiled_map_Def Off
#endif
#endif

/* B:nm, sec, ln, opt_*, setwhere?, on?, negat?, val?, dup?, hndlr? Alias,
            bool_p, term */
/* C:nm, sec, ln, opt_*, setwhere?, negateok?, valok?, dupok?, hndlr? Alias,
            desc */
/* P:pfx, sec, ln, opt_*, setwhere?, negateok?, valok?, dupok?, hndlr? Alias,
            desc*/
    /*
     * Most of the options are in alphabetical order; a few are forced
     * to the top of list so that doset() will list them first and
     * all_options_str() will gather them first to write to the top of
     * a new RC file by #saveoptions.
     *
     * windowtype comes first because its value can affect how wc_ and
     * wc2_ options are processed; playmode (for players who can't or
     * don't know how to specify a command line) and name (ditto, more
     * or less) come next; then role, race, gender, align.  Those will
     * be at the top of the file for #saveoptions constructed RC file.
     */
    NHOPTC(windowtype, Advanced, WINTYPELEN, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "使用するウィンドウシステム (最初に指定する)")
    NHOPTC(playmode, Advanced, 8, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "通常モード，探検モード，デバッグモードのいずれか")
    NHOPTC(name, Advanced, PL_NSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "あなたの名前 (例 name:マーリン-W)")
    NHOPTC(role, Advanced, PL_CSIZ, opt_in, set_gameview,
                Yes, Yes, Yes, No, "character",
                "ゲーム開始時の職業 (例 Barbarian, Valkyrie)")
    NHOPTC(race, Advanced, PL_CSIZ, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "ゲーム開始時の種族 (例 Human, Elf)")
    NHOPTC(gender, Advanced, 8, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "ゲーム開始時の性別(male または female)")
    NHOPTC(alignment, Advanced, 8, opt_in, set_gameview,
                Yes, Yes, Yes, No, "align",
                "ゲーム開始時の属性 (lawful, neutral, chaoticのいずれか)")
    /* end of special ordering; remainder of entries are in alphabetical order
     */
    NHOPTB(accessiblemsg, Advanced, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.accessiblemsg, Term_False,
           "メッセージに位置情報を加える")
    NHOPTB(acoustics, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.acoustics, Term_False,
           "キャラクターがなんでも聞こえるか")
 /* NHOPTC(align) -- moved to top */
    NHOPTC(align_message, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, Yes, NoAlias, "メッセージウィンドウの揃え")
    NHOPTC(align_status, Advanced, 20, opt_in, set_gameview,
                No, Yes, No, Yes, NoAlias, "状況ウィンドウの揃え")
#ifdef WIN32
    NHOPTC(altkeyhandling, Advanced, 20, opt_in, set_in_game,
                No, Yes, No, Yes, "altkeyhandler", "キーの扱いの代替策")
#else
    NHOPTC(altkeyhandling, Advanced, 20, opt_in, set_in_config,
                No, Yes, No, Yes, "altkeyhandler", "(not applicable)")
#endif
#ifdef ALTMETA
    NHOPTB(altmeta, Advanced, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.altmeta, Term_False,
           "treat \"ESC c\" as M-c (Meta+c, 8th bit set)")
#else
    NHOPTB(altmeta, Advanced, 0, opt_out, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(armorstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.armorstatus, Term_False,
                "ステータスに今着ている鎧のまとめを表示")
    NHOPTB(ascii_map, Advanced, 0, opt_in, set_in_game,
                ascii_map_Def, Yes, No, No, NoAlias, &iflags.wc_ascii_map,
                Term_False, "地図をテキストで表示")
    NHOPTO("autocompletions", Advanced, o_autocomplete, BUFSZ, opt_in,
                set_in_game, No, Yes, No, NoAlias, "入力自動補完")
    NHOPTB(autodescribe, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.autodescribe, Term_False,
           "describe terrain under cursor")
    NHOPTB(autodig, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.autodig, Term_False,
           "dig if moving and wielding a digging tool")
    NHOPTB(autoopen, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.autoopen, Term_False,
           "walking into a door attempts to open it")
    NHOPTB(autopickup, Behavior, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.pickup, Term_False,
           "automatically pick up objects")
    NHOPTO("autopickup exceptions", Behavior, o_autopickup_exceptions, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "edit autopickup exceptions")
    NHOPTB(autoquiver, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.autoquiver, Term_False,
           "fill empty quiver automatically when firing")
    NHOPTC(autounlock, Behavior, 80, opt_out, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "action to take when encountering locked door or chest")
    NHOPTB(bgcolors, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.bgcolors, Term_Off,
           "use background color for some map hilighting")
    NHOPTO("bind keys", Advanced, o_bind_keys, BUFSZ, opt_in, set_in_game,
                No, Yes, No, NoAlias, "edit key binds")
#if defined(MICRO) && !defined(AMIGA)
    NHOPTB(BIOS, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.BIOS, Term_False,
           "use IBM ROM BIOS calls")
#else
    NHOPTB(BIOS, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(blind, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "permablind", &u.uroleplay.blind, Term_False,
           "your character is permanently blind")
    NHOPTB(bones, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.bones, Term_False,
           "allow loading bones files")
#ifdef BACKWARD_COMPAT
    NHOPTC(boulder, Advanced, 1, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "廃止予定(代わりにシンボルファイルのS_boulderを使う)")
#endif
    NHOPTC(catname, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "name of your starting pet if it is a kitten")
#ifdef INSURANCE
    NHOPTB(checkpoint, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.ins_chkpt, Term_False,
           "save game state after each level change")
#else
    NHOPTB(checkpoint, Advanced, 0, opt_out, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(cmdassist, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.cmdassist, Term_False,
           "give help for errors on direction input")
    NHOPTB(color, Map, 0, opt_in, set_in_game,
           On, Yes, No, No, "colour", &iflags.wc_color, Term_False,
           "use color in map")
    NHOPTB(confirm, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.confirm, Term_False,
           "ask before hitting tame or peaceful monsters")
#ifdef CRASHREPORT
    NHOPTC(crash_email, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "email address for reporting")
    NHOPTC(crash_name, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "your name for reporting")
    NHOPTC(crash_urlmax, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "length of longest url we can generate")
#endif
#ifdef CURSES_GRAPHICS
    NHOPTC(cursesgraphics, Advanced, 70, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "curses表示シンボルをsymsetに読み込む")
#endif
    NHOPTB(customcolors, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, "customcolours", &iflags.customcolors,
           Term_False, "use custom colors in map")
    NHOPTB(customsymbols, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, "customsymbols", &iflags.customsymbols,
           Term_False, "use custom utf8 symbols in map")
    NHOPTB(dark_room, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.dark_room, Term_False,
           "show floor outside line of sight differently")
    NHOPTB(deaf, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "permadeaf", &u.uroleplay.deaf, Term_False,
           "your character is permanently deaf")
#ifdef BACKWARD_COMPAT
    NHOPTC(DECgraphics, Advanced, 70, opt_in, set_in_config,
                Yes, Yes, No, No, NoAlias,
                "DECGraphics表示シンボルをsymsetに読み込む")
#endif
    NHOPTB(debug_hunger, Advanced, 0, opt_in, set_wiznofuz,
           Off, Yes, No, No, NoAlias, &iflags.debug_hunger, Term_False,
           "no hunger")
    NHOPTB(debug_mongen, Advanced, 0, opt_in, set_wiznofuz,
           Off, Yes, No, No, NoAlias, &iflags.debug_mongen, Term_False,
           "no random monster generation")
    NHOPTB(debug_overwrite_stairs, Advanced, 0, opt_in, set_wiznofuz,
                Off, Yes, No, No, NoAlias, &iflags.debug_overwrite_stairs,
           Term_False, "level generation can overwrite stairs")
    NHOPTC(disclose, Advanced, sizeof flags.end_disclose * 2,
                opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "ゲーム終了時に見る情報の種類")
    NHOPTC(dogname, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "name of your starting pet if it is a little dog")
    NHOPTB(dropped_nopick, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.nopick_dropped, Term_False,
           "don't autopickup dropped items")
    NHOPTC(dungeon, Advanced, MAXDCHARS + 1,opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "迷宮の地図を描画するシンボル文字列")
    NHOPTC(effects, Advanced, MAXECHARS + 1, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "特殊効果を描画するシンボル文字列")
    NHOPTB(eight_bit_tty, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &iflags.wc_eight_bit_input,
           Term_False, "send 8-bit characters directly to terminal")
    NHOPTB(extmenu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.extmenu, Term_False,
           "use menu for getting extended commands")
    NHOPTB(female, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "male", &flags.female, Term_False,
           "deprecated; use gender:female")
    NHOPTB(fireassist, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.fireassist, Term_False,
           "fire-command tries to be helpful")
    NHOPTB(fixinv, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.invlet_constant, Term_False,
           "inventory items keep their letters")
    NHOPTC(font_map, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "マップウィンドウに使用するフォント")
    NHOPTC(font_menu, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "メニューに使うフォント")
    NHOPTC(font_message, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "メッセージウィンドウに使用するフォント")
    NHOPTC(font_size_map, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "マップフォントのサイズ")
    NHOPTC(font_size_menu, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "メニューフォントのサイズ")
    NHOPTC(font_size_message, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "メッセージフォントのサイズ")
    NHOPTC(font_size_status, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "状態フォントのサイズ")
    NHOPTC(font_size_text, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "テキストフォントのサイズ")
    NHOPTC(font_status, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "状態ウィンドウに使用するフォント")
    NHOPTC(font_text, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "テキストウィンドウに使用するフォント")
    NHOPTB(force_invmenu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.force_invmenu, Term_False,
           "持ち物一覧メニューを表示するコマンド")
    NHOPTC(fruit, General, PL_FSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias, "好物の果物の名前")
    NHOPTB(fullscreen, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.wc2_fullscreen, Term_False,
           "toggle fullscreen")
 /* NHOPTC(gender) -- moved to top */
    NHOPTC(glyph, Advanced, 40, opt_in, set_in_game,
                No, Yes, Yes, No, NoAlias,
                "set representation of a glyph to a unicode value and color")
    NHOPTB(goldX, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.goldX, Term_False,
           "classify gold as unknown or uncursed")
    NHOPTB(guicolor, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.wc2_guicolor, Term_False,
           "use color for UI")
    NHOPTB(help, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.help, Term_False,
           "show all available info when using whatis-command")
    NHOPTB(herecmd_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.herecmd_menu, Term_False,
           "show commands available in this location")
#if defined(MACOS9)
    NHOPTC(hicolor, Advanced, 15, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "パレットが同じとき、指示したものだけ反転させる")
#endif
    NHOPTB(hilite_pet, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc_hilite_pet, Term_False,
           "use highlight for pets")
    NHOPTB(hilite_pile, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.hilite_pile, Term_False,
           "highlight piles of items")
#ifdef STATUS_HILITES
    NHOPTC(hilite_status, Advanced, 13, opt_out, set_in_game,
                Yes, Yes, Yes, No, NoAlias,
                "a status highlighting rule (can occur multiple times)")
#else
    NHOPTC(hilite_status, Advanced, 13, opt_out, set_in_config,
                Yes, Yes, Yes, No, NoAlias, "(not available)")
#endif
    NHOPTB(hitpointbar, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc2_hitpointbar, Term_False,
           "show colored bar for hit points")
    NHOPTC(horsename, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "name of your starting pet if it is a pony")
#ifdef BACKWARD_COMPAT
    NHOPTC(IBMgraphics, Advanced, 70, opt_in, set_in_config,
                Yes, Yes, No, No, NoAlias,
                "IBMGraphics表示シンボルをsymsetに読み込む")
#endif
    NHOPTB(idlecheckpoint, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.idlecheckpoint, Term_Off,
           "update checkpoint file if input is idle for 10 seconds")
#ifndef MACOS9
    NHOPTB(ignintr, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.ignintr, Term_False,
           "ignore interrupt signals")
#else
    NHOPTB(ignintr, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(implicit_uncursed, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.implicit_uncursed, Term_False,
           "omit \"uncursed\" from inventory")
#if 0   /* obsolete - pre-OSX Mac */
    NHOPTB(large_font, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.obsolete,
           (char *)0)
#endif
    NHOPTB(legacy, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.legacy, Term_False,
           "show introductory message")
    NHOPTB(lit_corridor, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.lit_corridor, Term_False,
           "show dark corridors as lit if in sight")
    NHOPTB(lootabc, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.lootabc, Term_False,
           "use a/b/c rather than o/i/c when looting")
    NHOPTB(mail, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.biff, Term_False,
           "enable the mail daemon")
    NHOPTC(map_mode, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "ウィンドウ表示時のマップの表示モード")
    NHOPTB(mention_decor, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.mention_decor, Term_False,
           "give feedback when walking over interesting features")
    NHOPTB(mention_map, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.glyph_updates, Term_False,
           "give feedback when interesting map locations change")
    NHOPTB(mention_walls, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.mention_walls, Term_False,
           "give feedback when walking into walls")
    NHOPTC(menu_deselect_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "全てのアイテムを非選択")
    NHOPTC(menu_deselect_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "現在表示されているページのアイテムを非選択")
    NHOPTC(menu_first_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "メニューの最初のページへ")
    NHOPTC(menu_headings, Advanced, 4, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias, "メニュー見出しの表示スタイル")
    NHOPTC(menu_invert_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "全てのアイテムを反転")
    NHOPTC(menu_invert_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "現在表示されているページのアイテムを反転")
    NHOPTC(menu_last_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "メニューの最後のページへ")
    NHOPTC(menu_next_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "次のメニューのページへ")
    NHOPTC(menu_objsyms, Advanced, 12, opt_in, set_in_game,
           Yes, Yes, No, Yes, "use_menu_glyphs",
           "show object symbols in menus")
#ifdef TTY_GRAPHICS
    NHOPTB(menu_overlay, Advanced, 0, opt_in, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.menu_overlay, Term_False,
           "menus overlay and align to right")
#else
    NHOPTB(menu_overlay, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTC(menu_previous_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "前のメニューのページへ")
    NHOPTC(menu_search, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "メニューの検索")
    NHOPTC(menu_select_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "全てのアイテムを選択")
    NHOPTC(menu_select_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "現在表示されている全てのアイテムを選択")
    NHOPTC(menu_shift_left, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "pan current menu page left")
    NHOPTC(menu_shift_right, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "pan current menu page right")
    NHOPTB(menu_tab_sep, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.menu_tab_sep, Term_False,
           "menu formatting")
    NHOPTB(menucolors, Advanced, 0, opt_in, set_in_game,
           Off, Yes, Yes, No, NoAlias, &iflags.use_menu_color, Term_False,
           "use colors in menus")
    NHOPTO("menu colors", Status, o_menu_colors, BUFSZ, opt_in, set_in_game,
                No, Yes, No, NoAlias, "change colors used in menus")
    NHOPTC(menuinvertmode, Advanced, 5, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "experimental behavior of menu inverts")
    NHOPTC(menustyle, Advanced, MENUTYPELEN, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "オブジェクトを選択するときのユーザーインタフェース")
    NHOPTO("message types", Advanced, o_message_types, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "edit message types")
    NHOPTB(mon_movement, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.mon_movement, Term_False,
           "message when hero sees monster movement")
    NHOPTB(monpolycontrol, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.mon_polycontrol, Term_False,
           "control monster polymorphs")
    NHOPTB(montelecontrol, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.mon_telecontrol, Term_False,
           "control monster teleport destinations")
    NHOPTC(monsters, Advanced, MAXMCLASSES, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "モンスターに使用されるシンボル文字列")
    NHOPTC(mouse_support, Advanced, 0, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "ゲームがマウスからのクリック情報を受け取る")
#if PREV_MSGS /* tty or curses */
    NHOPTC(msg_window, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "control of \"view previous message(s)\" (^P) behavior")
#else
    NHOPTC(msg_window, Advanced, 1, opt_in, set_in_config,
                Yes, Yes, No, Yes, NoAlias, "(not applicable)")
#endif
    NHOPTC(msghistory, Advanced, 5, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "先頭行に表示されたメッセージ履歴の保存数")
 /* NHOPTC(name) -- moved to top */
#ifdef NEWS
    NHOPTB(news, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.news, Term_False,
           "show any news at game start")
#else
    NHOPTB(news, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(nudist, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.nudist, Term_False,
           "start your character without armor")
    NHOPTB(null, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.null, Term_False,
           "allow nulls to be sent to terminal")
    NHOPTC(number_pad, General, 1, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "ナンバーパッドを使用する")
    NHOPTC(objects, Advanced, MAXOCLASSES, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "アイテムに使用されるシンボル文字列")
    NHOPTC(packorder, Advanced, MAXOCLASSES, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "背負い袋内の物の順番")
#ifdef CHANGE_COLOR
#ifndef MACOS9     /* not old Mac OS9 */
    NHOPTC(palette, Advanced, 15, opt_in, set_gameview,
                No, Yes, Yes, No, "hicolor",
                "パレット (パレットのRGB色を調整する (色/R-G-B))")
#else
    NHOPTC(palette, Advanced, 15, opt_in, set_in_game,
                No, Yes, Yes, No, "hicolor",
                "パレット (00c/880/-fffはそれぞれ青/黄/反転白を示す)")
#endif
#endif
    /* prior to paranoid_confirmation, 'prayconfirm' was a distinct option */
    NHOPTC(paranoid_confirmation, Advanced, 28, opt_in, set_in_game,
                Yes, Yes, Yes, Yes, "prayconfirm",
                "一部の状況で追加の確認をする")
    NHOPTB(pauper, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.pauper, Term_False,
           "start your character without any items")
    NHOPTB(perm_invent, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &iflags.perm_invent, Term_Off,
                "show persistent inventory window")
    NHOPTC(perminv_mode, Advanced, 20, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "what to show in persistent inventory window")
    NHOPTC(petattr, Advanced, 88, opt_in, set_in_game, /* tty/curses only */
                No, Yes, No, Yes, NoAlias, "ペットをハイライトするための属性")
    /* pettype is ignored for some roles */
    NHOPTC(pettype, Advanced, 4, opt_in, set_gameview,
                Yes, Yes, No, No, "pet", "あなたの選択した初期ペットの種類")
    NHOPTC(pickup_burden, Advanced, 20, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "拾うときに最大荷重になる手前で確認する")
    NHOPTB(pickup_stolen, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.pickup_stolen, Term_False,
           "autopickup stolen items")
    NHOPTB(pickup_thrown, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.pickup_thrown, Term_False,
           "autopickup thrown items")
    NHOPTC(pickup_types, Behavior, MAXOCLASSES, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "自動で拾いあげる物のシンボル")
    NHOPTC(pile_limit, Advanced, 24, opt_in, set_in_game,
                Yes, Yes, No, No, NoAlias,
                "「ここにはたくさんのものがある」のしきい値")
    NHOPTC(player_selection, Advanced, 12, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "キャラクター選択にダイアログや確認画面を使う")
 /* NHOPTC(playmode) -- moved to top */
    NHOPTB(popup_dialog, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc_popup_dialog, Term_False,
           (char *)0)
    NHOPTB(preload_tiles, Advanced, 0, opt_out, set_in_config, /* MSDOS only */
           On, Yes, No, No, NoAlias, &iflags.wc_preload_tiles, Term_False,
           (char *)0)
    NHOPTB(price_quotes, General, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.pricequotes, Term_False,
           "display prices you have seen for unidentified objects")
    NHOPTB(pushweapon, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.pushweapon, Term_False,
           "previous weapon goes to secondary slot")
    NHOPTB(query_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.query_menu, Term_False,
           "use a menu for yes/no queries")
    NHOPTB(quick_farsight, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.quick_farsight, Term_False,
           "skip map browse when forced to looked at map")
 /* NHOPTC(race) -- moved to top */
#ifdef MICRO
    NHOPTB(rawio, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.rawio, Term_False,
           "allow use to raw I/O")
#else
    NHOPTB(rawio, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(reroll, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.reroll, Term_False,
           "allow rerolling of starting inventory and items")
    NHOPTB(rest_on_space, Advanced, 0, opt_in, set_in_game, Off,
           Yes, No, No, NoAlias, &flags.rest_on_space, Term_False,
           "space bar is bound to the rest-command")
    NHOPTC(roguesymset, Advanced, 70, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "symbolsファイルからローグレベルの表示シンボルの設定を読み込む")
 /* NHOPTC(role) -- moved to top */
    NHOPTC(runmode, Advanced, sizeof "teleport", opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "トラベルコマンド等での連続移動時の描画頻度")
    NHOPTB(safe_pet, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.safe_dog, Term_False,
           "prevent you from hitting pets")
    NHOPTB(safe_wait, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.safe_wait, Term_False,
           "prevent waiting next to hostiles")
    NHOPTB(sanity_check, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.sanity_check, Term_False,
           "perform data sanity checks")
    NHOPTC(scores, Advanced, 32, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "ゲーム終了時に見るスコアの種類")
    NHOPTC(scroll_amount, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "scroll_marginに届いたときのマップスクロール量")
    NHOPTC(scroll_margin, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "マップ端からのマップスクロール開始距離")
    NHOPTB(selectsaved, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc2_selectsaved, Term_False,
           (char *)0)
    NHOPTB(showdamage, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.showdamage, Term_False,
           "show damage hero takes in message line")
    NHOPTB(showexp, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showexp, Term_False,
           "show experience points in status line")
    NHOPTB(showrace, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showrace, Term_False,
           "show your character by race rather than role")
#ifdef SCORE_ON_BOTL
    NHOPTB(showscore, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showscore, Term_False,
           "show current score in status line")
#else
    NHOPTB(showscore, Status, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(showvers, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showvers, Term_False,
           "show version info on status line")
    NHOPTB(silent, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.silent, Term_False,
           "don't use terminal bell")
    NHOPTB(softkeyboard, Advanced, 0, opt_in, set_in_config,
                Off, Yes, No, No, NoAlias, &iflags.wc2_softkeyboard,
           Term_False, (char *)0)
    NHOPTC(sortdiscoveries, Advanced, 0, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "preferred order when displaying discovered objects")
    NHOPTC(sortloot, Advanced, 4, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "物体選択リストを説明でソートする")
    NHOPTB(sortpack, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.sortpack, Term_False,
           "group inventory items by type")
    NHOPTC(sortvanquished, Advanced, 0, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "preferred order when displaying vanquished monsters")
    NHOPTC(soundlib, Advanced, WINTYPELEN, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "soundlib interface to use (if any)")
#ifdef SND_LIB_INTEGRATED
    NHOPTB(sounds, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.sounds, Term_Off,
           "use integrated sound effects")
#else
    NHOPTB(sounds, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.sounds, Term_Off,
           "use sounds")
#endif
    NHOPTB(sparkle, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.sparkle, Term_False,
           "display sparkly effect when resisting magic")
    NHOPTB(spot_monsters, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.mon_notices, Term_False,
           "message when hero spots a monster")
    NHOPTB(splash_screen, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc_splash_screen, Term_False,
           (char *)0)
    NHOPTB(standout, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.standout, Term_False,
           "use standout for --more--")
    NHOPTB(status_updates, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.status_updates, Term_False,
           "ステータス行を更新できるようにする")
    NHOPTO("status condition fields", Status, o_status_cond, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "条件付きステータスハイライトを変更")
#ifdef STATUS_HILITES
    NHOPTC(statushilites, Advanced, 20, opt_in, set_in_game,
                Yes, Yes, Yes, No, NoAlias,
                "0=ステータスハイライトなし，Nターン毎にハイライト表示")
    NHOPTO("status highlight rules", Status, o_status_hilites, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "ステータス行ハイライトを変更")
#else
    NHOPTC(statushilites, Advanced, 20, opt_in, set_in_config,
                Yes, Yes, Yes, No, NoAlias, "ハイライトの制御")
#endif
    NHOPTC(statuslines, Status, 20, opt_in, set_in_game,
                No, Yes, No, No, NoAlias, "ステータス表示に2,3行使う")
#ifdef WIN32CON
    NHOPTC(subkeyvalue, Advanced, 7, opt_in, set_in_config,
                No, Yes, Yes, No, NoAlias, "キーマッピングを変更する")
#endif
    NHOPTC(suppress_alert, Advanced, 8, opt_in, set_in_game,
                No, Yes, Yes, No, NoAlias,
                "バージョン間の違いに関する警告メッセージの無効化")
    NHOPTC(symset, Map, 70, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "symbolsファイルから表示シンボルの設定を読み込む")
    NHOPTC(term_cols, Advanced, 6, opt_in, set_in_config,
                No, Yes, No, No, "termcolumns", "number of columns")
    NHOPTC(term_rows, Advanced, 6, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "number of rows")
    NHOPTB(terrainstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.terrainstatus, Term_False,
                "show hero's location as a status field")
    NHOPTC(tile_file, Advanced, 70, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "タイルファイルの名前")
    NHOPTC(tile_height, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "タイルの高さ")
    NHOPTC(tile_width, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "タイルの幅")
    NHOPTB(tiled_map, Advanced, 0, opt_in, set_in_game,
                tiled_map_Def, Yes, No, No, NoAlias, &iflags.wc_tiled_map,
           Term_False, (char *)0)
    NHOPTB(time, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.time, Term_False,
           "display game turns in status line")
#ifdef TIMED_DELAY
    NHOPTB(timed_delay, Map, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.nap, Term_False,
           "use delay when pausing for display effects")
#else
    NHOPTB(timed_delay, Map, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(tips, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.tips, Term_False,
           "show some helpful tips during gameplay")
    NHOPTB(tombstone, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.tombstone, Term_False,
           "show tombstone when your character dies")
    NHOPTB(toptenwin, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.toptenwin, Term_False,
           "show top scores in window")
    NHOPTC(traps, Advanced, MAXTCHARS + 1, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "罠を描画するシンボル文字列")
    NHOPTB(travel, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.travelcmd, Term_False,
           "enable traveling via mouse click")
#ifdef DEBUG
    NHOPTB(travel_debug, Advanced, 0, opt_out, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.trav_debug, Term_False,
           (char *)0)
#else
    NHOPTB(travel_debug, Advanced, 0, opt_out, set_wizonly,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(tutorial, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.tutorial, Term_False,
           "ask if you want the tutorial")
    NHOPTB(use_darkgray, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc2_darkgray, Term_False,
           "use bold black color instead of blue")
    NHOPTB(use_inverse, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.wc_inverse, Term_False,
           "display detected monsters in inverse")
    NHOPTB(use_truecolor, Advanced, 0, opt_in, set_in_config,
                Off, Yes, No, No, "use_truecolour",
           &iflags.use_truecolor, Term_False,
           (char *)0)
    NHOPTC(vary_msgcount, Advanced, 20, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "一度に表示するメッセージの数")
    NHOPTB(verbose, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.verbose, Term_False,
           (char *)0)
    NHOPTC(versinfo, Advanced, 80, opt_out, set_in_game,
           No, Yes, No, Yes, NoAlias, "'showvers'のための追加情報")
#ifdef MSDOS
    NHOPTC(video, Advanced, 20, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "使用するビデオモードを設定する")
#endif
#ifdef VIDEOSHADES
    NHOPTC(videocolors, Advanced, 40, opt_in, set_gameview,
                No, Yes, No, No, "videocolours",
                "内蔵スクリーンルーチン用のカラーマップを用いる")
    NHOPTC(videoshades, Advanced, 32, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "表示にグレイスケールを用いる")
#endif
#ifdef MSDOS
    NHOPTC(video_width, Advanced, 10, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "video width")
    NHOPTC(video_height, Advanced, 10, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "video height")
#endif
#ifdef SND_SPEECH
    NHOPTB(voices, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.voices, Term_Off,
           (char *)0)
#else
    NHOPTB(voices, Advanced, 0, opt_in, set_gameview,
           Off, Yes, No, No, NoAlias, &iflags.voices, Term_Excluded,
           (char *)0)
#endif
#ifdef TTY_TILES_ESCCODES
    NHOPTB(vt_tiledata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.vt_tiledata, Term_False,
           "output special escape codes")
#else
    NHOPTB(vt_tiledata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
#ifdef TTY_SOUND_ESCCODES
    NHOPTB(vt_sounddata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.vt_sounddata, Term_False,
           "output sound data in special escape codes")
#else
    NHOPTB(vt_sounddata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTC(warnings, Advanced, 10, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "display characters for warnings")
    NHOPTB(weaponstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.weaponstatus, Term_False,
                "show currently wielded weapon in a status field")
    NHOPTC(whatis_coord, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "カーソル位置を自動説明するときに座標を表示する")
    NHOPTC(whatis_filter, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "次や手前をターゲットするときに座標位置をフィルタする")
    NHOPTB(whatis_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.getloc_usemenu, Term_False,
           "show menu when getting a map location")
    NHOPTB(whatis_moveskip, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.getloc_moveskip, Term_False,
           "skip same glyph when getting map location")
    NHOPTC(windowborders, Advanced, 9, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias, "0 (off), 1 (on), 2 (auto)")
#ifdef WINCHAIN
    NHOPTC(windowchain, Advanced, WINTYPELEN, opt_in, set_in_sysconf,
                No, Yes, No, No, NoAlias, "使用するウィンドウプロセッサ")
#endif
    NHOPTC(windowcolors, Advanced, 80, opt_in, set_gameview,
                No, Yes, Yes, No, NoAlias,
                "ウィンドウを指定した前景色/背景色で表示する")
 /* NHOPTC(windowtype) -- moved to top */
    NHOPTB(wizmgender, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.wizmgender, Term_False,
           (char *)0)
    NHOPTB(wizweight, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.wizweight, Term_False,
           (char *)0)
    NHOPTB(wraptext, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc2_wraptext, Term_False,
           (char *)0)

    /*
     * Prefix-based Options
     */

    NHOPTP(cond_, Advanced, 0, opt_in, set_hidden,
                Yes, No, Yes, Yes, NoAlias, "prefix for cond_ options")
    NHOPTP(font, Advanced, 0, opt_in, set_hidden,
                Yes, Yes, Yes, No, NoAlias, "prefix for font options")
#if defined(MICRO) && !defined(AMIGA)
    /* included for compatibility with old NetHack.cnf files */
    NHOPTP(IBM_, Advanced, 0, opt_in, set_hidden,
                No, No, Yes, No, NoAlias, "prefix for old micro IBM_ options")
#endif /* MICRO */

#undef NoAlias
#undef NHOPTB
#undef NHOPTC
#undef NHOPTP
#undef NHOPTO

/* *INDENT-ON* */
/* clang-format on */
#endif /* NHOPT_PROTO || NHOPT_ENUM || NHOPT_PARSE */

/*optlist.h*/
