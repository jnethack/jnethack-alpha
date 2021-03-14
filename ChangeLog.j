	* 次の問題を修正
	  * rumorsの末尾にゴミが表示される (#41899)
	* X11のロケールを内部でja_JP.EUC-JPに固定する (#40804)

Sat Mar 13 2021  Kentaro Shirakata  <argrath@ub32.org>

	* 次の問題を修正
	  * X11環境で日本語入力ができない (#40562)
	  * 一部のオプションで英語のフルネームを使えない (#40657)
	  * X11版でascii_mapが正しく表示されない (#40530)
	  * X11でascii_map表示するとリソースエラーが発生する (#40803)
	  * 同じ階に寺院があるときのメッセージが文字化け (#40957)
	* X11LARGETILEを削除(#40681)
	* JNetHack.adを最新に更新(#40531)

Sat Apr 18 2020  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.6ベース JNetHackテストリリース(3.6.6-0.1)
	* 次の問題を修正
	  * HPが0になったときに命の魔除けが効かない (#40316)

Sun Dec  8 2019  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.2ベース JNetHackテストリリース(3.6.2-0.1)
	* 次の問題を修正
	  * 銀が苦手な怪物を銀の武器で攻撃したときのメッセージがおかしい
	    (#39767)

Sun Jun  2 2019  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.1-0.4リリース
	* 次の問題を修正
	  * 死体に蘇生の杖を振るとクラッシュする (#39081)
	  * 炎でスライム化の解除ができない (#39134)
	* unixでのデフォルト文字コードをUTF-8に変更(#39273)

Sun Dec 26 2018  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.1-0.3リリース
	* MacOSでのビルドに対応
	* 願いの処理を拡充
	* ビルド方法の文書を追加(japanese/build.md)
	* AppVeyorでのビルドに対応
	* 翻訳追加修正

Sun Oct  7 2018  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.1-0.2リリース
	* 次の問題を修正
	  * #wizwhereコマンドでクラッシュする (#38569)
	  * 2バイト文字表示時に属性変更が行われない (#38566)
	  * 正規表現が正しく動作しない (#38565)
	  * 怪物/アイテムデータが3.6.1に更新されていない (#38576)
	  * ウィザードモードで魔法の罠を願えない (#38584)
	  * タイル作成時に警告が出る (#38608)
	  * 祭壇捧げメニューで食べ物や魔除けがフィルタされていない (#38620)
	* VisualStudio 2015 IDEでのビルドに対応
	* コンパイル警告を抑制
	* 翻訳追加修正

Thu Jun 21 2018  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.1ベース JNetHackテストリリース(3.6.1-0.1)

Fri Apr 29 2018  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.9リリース
	* ISO-2022-JP対応を削除
	* UTF-8出力に対応 (#37910)
	* 内部コードと表示コードが違う時の問題を修正:
	  * -uオプションで日本語を使うと文字化けする (#37911)
	  * -sオプションで日本語を使うと文字化けする (#37912)
	  * 日本語入力が正しく動作しない (#37913)
	* 翻訳追加修正

Fri Feb 17 2017  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.8リリース
	* 画面上の怪物を/コマンドで見た際に文字化けする問題を修正 (#36909)
	* NPCの僧侶/尼僧/法王が神の名を冠していない問題を修正 (#36910)
	* ウィザードモードでイェンダーの魔除けが願えない問題を修正 (#36939)
	* オーク感知中のスティングの輝き表示が無い問題を修正 (#36953)
	* 願いで得たものを取り落とした時のメッセージがおかしい問題を修正
	  (#36991)
	* 怪物名の付いた物が願えない問題を修正 (#36992)
	* 翻訳追加修正

Wed Dec  7 2016  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.7リリース
	* 一部の死因で「死んだ」が表示されない問題を修正 (#36579)
	* Windows10のttyで日本語が入力できない問題を修正 (#36599)
	* 盗賊クエストとジョウビレックスフロアで「障害発生」が出る問題を修正
	  (#36640)
	* 翻訳追加修正

Sun Jun 11 2016  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.6リリース
	* GCCでの警告を抑制
	* 翻訳追加修正

Sun Apr  3 2016  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.5リリース
	* 氷の上を歩くと「障害発生」が出る問題を修正 (#36207)
	* クエスト最下層でクエスト聖器が生成されない問題を修正 (#36215)
	* 店でアイテムを売るときに「障害発生」が出る問題を修正 (#36223)
	* 店主がお金を支払えないときにクラッシュする問題を修正 (#36224)
	* 翻訳追加修正

Thu Mar 24 2016  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.4リリース
	* 一部のファイルの文字コードがEUC-JPになっていた問題を修正 (#36189)
	* 翻訳追加修正

Fri Mar 18 2016  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.3リリース
	* unix/X11対応
	* JNetHack.adを現状に合うように更新
	* VS2013のIDEでビルド出来るように調整
	* 噂、神託、ヘルプの翻訳を追加
	* 翻訳追加修正

Sun Feb 21 2016  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.2リリース
	* windows/gui対応
	* 鉱山最下層や倉庫番最終面で「障害発生」が出る問題を修正 (#36038)
	* win/X11/winX.c のパッチ適用漏れを修正 (#36039)
	* 翻訳追加修正

Sun Feb 14 2016  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.0ベース JNetHackテストリリース(3.6.0-0.1)

Sun Apr  8 2013  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.11リリース
	* 占いクッキー専用メッセージ処理が動作していなかった問題を修正
	  (#16381)
	* Windows版でセーブファイル名に日本語が使えるように修正 (#18272)
	* 物を拾うときにバッファオーバーランする問題を修正 (#26691)
	* 眠っている騎乗モンスターを蹴るとクラッシュする問題を修正 (#30923)
	* 翻訳追加修正

Sun May  4 2008  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.10リリース
	* 床の長いメッセージを読むとクラッシュする問題を修正
	* 一部のアイテムが日本語で願えない問題を修正
	* 翻訳追加修正

Wed May  2 2007  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.9リリース
	* WindowCEに一部対応
	* 翻訳追加修正

Sat Apr  1 2006  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.8リリース
	* 一部のアイテムが願えない問題を修正
	* シャツの上からシャツを着ようとするとクラッシュする問題を修正
	* 日本語での名付けで聖器が作成できない問題を修正
	* 翻訳追加修正

Sun Jul 31 2005  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.7リリース
	* DOS版でスコア表示が文字化けしていた問題を修正
	* アエスキュラピスの杖が願えない問題を修正
	* 翻訳追加修正

Mon Apr 28 2005  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.6リリース
	* アイテムや地形の詳細表示がされない問題を修正
	* クエストに関する表示が一部乱れる問題を修正
	* 翻訳追加修正

Mon Jan  9 2005  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.5リリース
	* 存在しないオプションをdefault.nhに設定したときにクラッシュする
	  問題を修正
	* 日本語でデータベースが呼び出せない場合がある問題を修正
	* 翻訳追加修正

Mon Aug  2 2004  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.4リリース
	* 複数形で願った時の処理を#ifdefで分離
	* 変身と名前返答の時に日本語が使えなかった問題を修正
	* 翻訳追加修正

Thu Apr 29 2004  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.3リリース
	* 虐殺で日本語が使えなかった問題を修正
	* 乗馬用アイテムの効果が効いていなかった問題を修正
	* 翻訳追加修正

Thu Feb 19 2004  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.2リリース
	* win/ttyがBCCでコンパイルできなくなっていた問題を修正
	* 翻訳追加修正

Wed Dec 31 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.1リリース
	* NetHack-3.4.3ベースに対応

Thu Dec  5 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.2-0.2リリース
	* win/ttyでスコア一覧表示が乱れる問題を修正
	* 素手で酸のブロッブを殴るとまれにクラッシュする問題を修正
	* 翻訳追加修正

Thu Nov 11 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.2-0.1リリース
	* NetHack-3.4.2ベースに対応

Wed Sep  3 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.4リリース
	* 翻訳追加修正

Sat Aug  9 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.3リリース
	* Reportパッチを分離
	* コマンドラインで指定した名前の漢字コードが変換されない問題を修正
	* 翻訳追加修正

Thu Jun  9 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.2リリース
	* winnt/ttyで日本語入力が出来ない問題を修正
	* 翻訳追加修正

Thu Apr 10 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.1リリース
	* NetHack-3.4.1ベースに対応

Sat Feb 14 2003  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.10リリース
	* unix/X11対応開始
	* Radarパッチを分離
	* Newbieパッチを分離
	* CenterPopupsパッチを分離
	* MinGWでのコンパイルに対応
	* jjjを廃止
	* 公式セキュリティパッチ(C340-137)を適用
	* winnt/win32で多くのウィンドウを表示すると落ちる問題(W340-14)を修正
	* 翻訳追加修正

Fri Dec 20 2002  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.9リリース
	* 死の谷で上り階段が生成されないことがある問題(C340-27)を修正
	* 武闘家でレベル13の時冷気耐性がつかない問題を修正
	* Windows/GUI版がWin95系で文字化けする問題を修正
	* Boland C++でのコンパイルに対応
	* Fighterパッチを分離
	* 本家でGnome対応されているので旧GTK_GRAPHICS関係を削除
	* 翻訳追加修正

Sat Sep 16 2002  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.8リリース
	* ボーパルブレードで幽霊を切ると落ちる問題を修正
	* 怪物が人間に変身したときに職業表示が文字化けする問題を修正
	* 翻訳追加修正

Mon Aug  5 2002  Kentaro Shirakata  <argrath@ub32.org>
	* dos(djgpp)対応開始

Sat Aug  3 2002  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.7リリース
	* jGuidebookを追加
	* data.base,quest.datをそれぞれjdata.base,jquest.datに変更
	* rogueレベルで'@'が文字化けする問題を修正
	* 地雷でその上にある巨石/像が壊れると落ちる問題(C340-2)を修正
	* 翻訳追加修正

Sun Jul  7 2002  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.6リリース
	* フロストブランドの攻撃が命中すると落ちる問題を修正
	* 数字を"/"コマンドで調べると落ちる問題を修正
	* 音に関するメッセージが全く表示されていなかった問題を修正
	* 翻訳追加修正

Fri Jun 14 2002  Kentaro Shirakata  <argrath@ub32.org>
	* JnetHack-3.4.0-0.5リリース
	* winnt/ttyで表示が乱れる問題を仮修正
	* アイテム命名が反映されないことがある問題を修正
	* 翻訳追加修正

Fri May 24 2002  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.4リリース
	* トリックの鞄を#lootしたときのダメージがおかしいバグを修正
	* "."で休憩できないバグを修正
	* プレイヤーがモンスターを飲み込むと無条件で石化するのを修正
	* 翻訳追加修正

Thu May 16 2002  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.3リリース
	* winnt/tty対応開始
	* data.baseを処理
	* 翻訳追加修正

Sun May 12 2002  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.2リリース
	* winnt/win32対応開始
	* default.nhを読もうとするとエラーになるのを修正
	* 翻訳追加修正

Thu May  9 2002  Kentaro Shirakata  <argrath@ub32.org>
	* NetHack-3.4.0ベース JNetHackテストリリース(3.4.0-0.1)

Sun Apr    2 2000	Issei Numata  <issei@jnethack.org>
	* JNetHack-1.2.0βバージョンリリース

Tue Mar   14 2000	Hideaki YOSHIFUJI (吉藤英明) <yoshfuji@ecei.tohoku.ac.jp>
	* IPV6に対応

Fri Mar   10 2000	Issei Numata  <issei@jnethack.org>
	* JNetHack-1.2.0αバージョンリリース

Wed Mar   1 2000	Issei Numata  <issei@jnethack.org>
	* オープニングメッセージを変更
	* NetHack-3.3.0ベース JNetHack開発開始

Sat Feb  12 2000	Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.5 リリース

Mon Feb  7 2000		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.4 リリース

Mon Feb  7 2000		Issei Numata  <issei@jnethack.org>
	* NetHack-3.2.3に対応

Wed Dec  1 1999		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.3 リリース

Sun Nov 21 1999		Issei Numata  <issei@jnethack.org>
	* プロキシをオプションで指定できるよう変更

Thu Nov 19 1999		Issei Numata  <issei@jnethack.org>
	* 3Dマップ対応

Wed Nov 17 1999		Issei Numata  <issei@jnethack.org>
	* bonesシステム導入

Tue Nov 9 1999		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.2 リリース

Tue Nov 9 1999		Issei Numata  <issei@jnethack.org>
	* proxyを使えるように変更
	* 細かいバグ修正

Mon Nov 8 1999		Issei Numata  <issei@jnethack.org>
	* httpでスコアを転送できるように変更

Tue Nov 2 1999		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.1 リリース

Mon Nov 1 1999		Issei Numata  <issei@jnethack.org>
	* 西暦2000年問題に対処
	* menuに HP/MP barを追加
	* 細かいバグ修正

Thu Oct 28 1999		Issei Numata  <issei@jnethack.org>
	* monster detectionなどで画面が止まらない問題を修正
	* 伝統的文字モードを選択可能に

Mon Oct 24 1999		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.0 リリース
	* 細かいバグ修正

Sat Oct 23 1999		Issei Numata  <issei@jnethack.org>
	* credit画像追加

Thu Oct 21 1999		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.0.7.19991021リリース

Tue Oct 19 1999		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.0.7.19991019リリース

Sat Oct 16 1999		Issei Numata  <issei@jnethack.org>
	* warningを減らす
	* 細かいバグ修正

Sat Oct 9 1999		Issei Numata  <issei@jnethack.org>
	* newbieモード追加

Sat Oct 9 1999		Issei Numata  <issei@jnethack.org>
	* jnethack-1.0.6.3公開

Fri Oct 8 1999		Issei Numata  <issei@jnethack.org>
	* タイルを使用しないときはradarを非表示にするように修正
	* radarの上で ESCを押したら非表示に修正
	* optionに radarの on/offの切替を追加
	* スコアサーバに接続するとき20秒でタイムアウトするよう変更
	* その他細いバグを修正

Fri Oct 8 1999		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* スコア表示のバグを修正

Tue Oct 7 1999		ITAKURA Mitsuhiro <ita@gold.koma.jaeri.go.jp>	
	* 透明ピックスマップの高速化

Tue Oct 7 1999		Issei Numata  <issei@jnethack.org>
	* optionに homeurlを追加
	* デフォルトのオプション変更
	  perm_invent -> off
	  TEXT_COLORをデフォルトで #define

Tue Oct 7 1999		Issei Numata  <issei@jnethack.org>
	* 透明ピックスマップに対応
	* JNetHack.ad を HACKDIRに置けるように変更

Tue Oct 6 1999		Issei Numata  <issei@jnethack.org>
	* jnethack-1.0.6.1公開

Tue Oct 6 1999		Issei Numata  <issei@jnethack.org>
	* Linuxでも簡単にコンパイルできるよう修正
	* ripのcolormapが正しく設定されない問題を修正
	* レーダーの上でもキーを受けつけるよう修正
	* X版はオプションの有無にかかわらず、number padを移動に
	  使えるように修正

Tue Oct 5 1999		Issei Numata  <issei@jnethack.org>
	* jnethack-1.0.6公開

Mon Oct 4 1999		Issei Numata  <issei@jnethack.org>
	* デフォルトのオプション値変更
	  autopickup -> off
	  perm_invent -> on
	  mail -> off
	  windowtype 'tty' -> 'x11'

Sun Oct 3 1999		Issei Numata  <issei@jnethack.org>
	* レーダー表示追加
	* 色数が少ないモニタでも動作するよう変更

Thu Oct 1 1999		Issei Numata  <issei@jnethack.org>
	* 標準タイルを32x32のものに変更
	* スコア表示を変更

Wed Sep 30 1999		Issei Numata  <issei@jnethack.org>
	* スコアをホストに転送するシステム作成

Wed Sep 24 1997		Issei Numata  <issei@nna.so-net.or.jp>
	* class名を NetHackから JNetHackへ変更

Mon Sep 8 1997		uchidak@creo.co.jp
Mon Aug 25 1997
	* メッセージの修正

Tue Jul 22 1997		KIHARA Hideto <kihara@shiratori.riec.tohoku.ac.jp>
	* メッセージの修正

Thu Jul 3 1997		Kentaro Shirakata <shira@kdel.info.eng.osaka-cu.ac.jp>
	* nethack-3.2.2ベースのjnethackの開発に着手
	* quest.txtの訳語修正．

Tue Dec 24 1996		Issei Numata  <issei@nna.so-net.or.jp>
	* jnethack-1.0.5.4.1 release
	* 性別女でKを選ぶとPになってしまうバグを本当に修正．

Sat Nov 23 1996		Issei Numata  <issei@nna.so-net.or.jp>
	* jnethack-1.0.5.4 release
	* ドキュメントの修正(とてもここに名前は書き切れません)
	* メッセージの修正(とてもここに名前は書き切れません)

Fri Sep  6 1996		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.5.2 release

Fri Aug 23 1996		Issei Numata  <issei@jaist.ac.jp>
	* Ｆの死体や幽霊が例の場所に現われない問題の修正．

Thu Aug 22 1996		Issei Numata  <issei@jaist.ac.jp>
	* X11で perminventをオプションで指定するとアイテムを拾ったときに
	  落ちる問題の修正
	* ユーティリティrecoverで回復できない問題の修正． (NetHackのバグ) 
	* Class Genocideしたときのメッセージが英語の修正． 
	* 聖器(アーティファクト)を願って失敗したときのメッセージが英語． 

Wed Aug 21 1996		Issei Numata  <issei@jaist.ac.jp>
	* japanese/jlib.c の is_kanji1の修正
	* 職業 Fの Questでの Guardのメッセージが変
	* 職業 Fで Neutralの神が英語
	* 薬がジュースや水になるときのメッセージ．
	* 毒の攻撃を食って死ぬと怪物の名前．
	* たまにマップの文字が化ける(とくに jisで遊んでいる時)問題．
	* 聖器(アーティファクト)に触れて死んだときの死因が英語
	* 落書の文字が化ける

Mon Aug 19 1996		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.5.1 release

Mon Aug 19 1996		Issei Numata  <issei@jaist.ac.jp>
	* いくつかの日本語関係の表示のバグを解決

Fri Aug 16 1996		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.5 release

Thu Aug 15 1996		Issei Numata  <issei@jaist.ac.jp>
	* 短いファイルネームのシステムに対応できるよう，jtrns_*.cを改名

Wed Aug 14 1996		Issei Numata  <issei@jaist.ac.jp>
	* DOS/Vで tile表示を可能に．

Thu Aug  8 1996		Issei Numata  <issei@jaist.ac.jp>
	* Jだけのオリジナル職業(F)を追加

Fri Aug  2 1996		juan@super.win.or.jp (Kenji Gotoh)
	* sjisで wishingするとき旨くいかないことがある問題を修正

Wed Jul 31 1996		Satoh Yoshiyuki <yo-satou@system.hitachi.co.jp>
	* いくつかのメッセージの間違いを修正

Tue Jul 30 1996		Hiramoto Kouji (平本光二 ) <hiramoto@Nurse.Angel.nanako.or.jp>
	* PCでスタート時に英語のメッセージが出る問題の修正

Mon Jul 29 1996		Hideto KIHARA <kihara@shiratori.riec.tohoku.ac.jp>
	* いくつかのメッセージの間違いを修正

Thu Jul 25 1996		fukui@nm.nmcc.co.jp (Terushige Fukui)
	* 存在しないものを wishしたときに落ちる問題を修正

Thu Jul 25 1996		Issei Numata  <issei@jaist.ac.jp>
	* 日本語による wishingを一部可能に

Wed Jul 24 1996		Hideto KIHARA <kihara@shiratori.riec.tohoku.ac.jp>
	* jtrns_monの副作用の問題を修正

Tue Jul 23 1996		juan@super.win.or.jp (Kenji Gotoh)
	* ステータス欄を全て書き換えてしまう問題を修正

Mon Jul 22 1996		Naoki Hamada <nao@sbl.cl.nec.co.jp>
	* DEC graphics で漢字が化ける問題を修正

Mon Jul 22 1996		Hideto KIHARA <kihara@shiratori.riec.tohoku.ac.jp>
	* メッセージのまちがいを修正
	* jconjのバグの修正
	* 「ら」抜き言葉の修正

Sat Jul 20 1996		shira@kdel.info.eng.osaka-cu.ac.jp (Kentaro Shirakata)
	* PCでコンパイルエラーが出るのを修正

Fri Jul 19 1996		Issei Numata  <issei@jaist.ac.jp>
	* いくつかの Warningを削除
	* formatの間違いを修整

Thu Jul 18 1996		Issei Numata  <issei@jaist.ac.jp>
	* 言語モードの切替がうまくいかないのを修整

Thu Jul 18 1996		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.4-alpha release

Fry Jun 21 1996		Issei Numata  <issei@jaist.ac.jp>
	* nethack-3.2.1ベースのjnethackの開発に着手

Thu May  2 1996		Issei Numata  <issei@jaist.ac.jp>
	* nethack-3.2.0ベースのjnethackの開発に着手

Sun Jun  4 1995		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.3 release

Sun Jun  4 1995		nishi@bsd1.kb.nec.co.jp
	* nh14.bdfをパッケージに追加

Fri Jun  2 1995		Issei Numata <issei@jaist.ac.jp>
	* スコアリストのメッセージのバグを修整．
	* ドキュメントの間違いを修整．

Thu Feb 23 1995		Hiramoto Kouji (平本光二 ) <hiramoto@kubota.co.jp>
	* resotre.c メッセージを修整．

Thu Feb 21 1995		Hiramoto Kouji (平本光二 ) <hiramoto@kubota.co.jp>
	* cmd.c 「あなたは」が重複するのを修整．

Thu Jan 5  1995		asami@cs.berkeley.edu
	* 全般的にメッセージの誤訳を修整．

Tue Dec 27 1994		藤枝 和宏 (FUJIEDA Kazuhiro) <fujieda@jaist.ac.jp>
	* fountain.c 流し台の水を飲んで薬が出てきたときのメッセージを修整

Tue Dec 20 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.2 release

Thu Oct 27 1994		narusawa@nsis.cl.nec.co.jp (Masaru Narusawa)
	* NEWOS用のパッチ追加．

Fri Oct 14 1994		Issei Numata <issei@jaist.ac.jp>
	* X11版で長いメッセージが出たときにpanicになる問題を修正．
	* 自分自身をgenocideしたときのメッセージを修正．

Mon Sep 19 1994		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* mhitm.c ニンフが他のモンスターを攻撃したときにcodeを吐く問題を
	  修正．

Mon Sep 12 1994		Issei Numata <issei@jaist.ac.jp>
	* オリジナルモードで死んだときスコアファイルが英名になるのを修正．
	* 怪物の攻撃で病気になったときのメッセージを修正．
	* helpに英名-和名の変換テーブル追加．

Thu Sep  8 1994		Issei Numata <issei@jaist.ac.jp>
	* コカトリスの死体で死んだときのメッセージを修正．
	* メデューサで石化したときのメッセージを修正．
	* 迷宮から脱出したときのメッセージを修正．
	* たくさんの誤訳，誤字を修正．
	* quest.txtを整形．
	* jisモードのとき \033$@, \033$Jも有効になるように修正．

Mon Aug 29 1994		Issei Numata <issei@jaist.ac.jp>
	* 環境変数と設定ファイル両方を設定した場合のエラーを修正．

Mon Aug 29 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.1 release

Tue Aug 23 1994		Issei Numata <issei@jaist.ac.jp>
	* vorpale bladeが首を切り落したときのメッセージを正しく表示する
 	  よう修正．

Sat Aug 20 1994		Issei Numata <issei@jaist.ac.jp>
	* 細いメッセージの修正．

Fri Aug 19 1994		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* data.base改訂．

Wed Aug 17 1994		Masayuki Koba <masa-k@sag.hitachi-sk.co.jp>
	* NetHack.ad 16ドットフォントをデフォルトで使うよう修正．

Sat Aug 13 1994		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* 迷宮から抜けたときのメッセージの不具合を修正．

Sun Aug  7 1994		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* DJGPP版パッケージ追加．

Sun Aug  7 1994		高田　幸治 <HAE00146@niftyserve.or.jp>
	* MSC版パッケージ追加．

Thu Aug  4 1994		Yoshiki Ogino <azrael@cosmos.ces.kyutech.ac.jp>
	* questpgr.c アライメント表示のバグを修正．
 
Thu Aug  4 1994		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* mk_artifact以外で，artifactを得たとき正しく作成されないのを修正．
	* class genocideで殺すことができなかったモンスターの名前を正しく
	  表示できるよう修正．

Mon Aug  1 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.0 release

Mon Aug  1 1994		Issei Numata <issei@jaist.ac.jp>
	* 細かいメッセージの修正．

Sun Jul 31 1994		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* 刃のない武器で攻撃されたときのメッセージを修正．

Mon Jul 25 1994		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* magicbane関係のメッセージを修正．

Mon Jul 25 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.3.1 beta test starts

Sut Jul 23 1994		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* artifact.c magicbaneの訳を修正．

Mon Jul 25 1994		Issei Numata <issei@jaist.ac.jp>
	* topl.c 長い文字列のフォールディングを修正．
	* 長い名前をつけた際に文字化けするのを修正．

Sut Jul 23 1994		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* jtrns_{obj,mon}.dat 細い訳語の間違いを修正．

Sut Jul 23 1994		Issei Numata <issei@jaist.ac.jp>
	* jGuideBook.txtを JNetHackにあわせ修正．

Fri Jul 22 1994		shira@kdel.info.osaka-cu.ac.jp (Kentaro Shirakata)
	* jGuideBook.txtをパッケージに追加．

Thu Jul 21 1994		Issei Numata <issei@jaist.ac.jp>
	* jtrns_obj.dat 武器を強くする巻物->武器に魔法をかける巻物へ変更．
	* getline.c kinput2から漢字を入力できるように変更．

Thu Jul 21 1994		Yoshiaki KASAHARA <kasahara@csce.kyushu-u.ac.jp>
	* 麻痺->眩暈に変更．

Thu Jul 21 1994		Issei Numata <issei@jaist.ac.jp>
	* objnam.c nemesisの死体名が正しく表示されるよう修正．

Thu Jul 21 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.3.0 beta test starts	

Tue Jul 19 1994		s92458km@sfc.keio.ac.jp (the kunedog)
	* quest.txt Turist の翻訳追加

Fri Jul 15 1994		Issei Numata <issei@jaist.ac.jp>
	* priest.c 天使の名前が正しく表示されるよう修正．
	* end.c topten.c 死んだときのメッセージの不具合を修正．
	* objnam.c 長い名前の文字でcoreを吐く問題を修正．

Thu Jul 14 1994		Issei Numata <issei@jaist.ac.jp>
	* jconj.c jtrns_obj.dat 聖器の名前が正しく表示されないのを修正．

Thu Jul 14 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.4 beta test starts

Thu Jul 14 1994		Issei Numata <issei@jaist.ac.jp>
	* termcap.c -decで文字が化けるのを修正．
	* 聖器に全部和名を与えた．

Wed Jul 13 1994		Issei Numata <issei@jaist.ac.jp>
	* jtrns_{obj,dat}をソースに埋めこみ，makedefsで対応するよう修正．
	* warningを減らした．

Tue Jul 12 1994		Hideharu Miyazaki <hideha-m@ascii.co.jp>
	* printf系の引数の間違いを修正
	* extern.h locomotion2のプロトタイプを追加
	* コメントアウトのミスを修正

Sun Jul 10 1994		Issei Numata <issei@jaist.ac.jp>
	* eat.c コカトリスで石化したときのメッセージの不具合を修正．

Thu Jul 7 1994		<mya@jaist.ac.jp
	* quest.txt Knight の翻訳追加

Thu Jul 7 1994		<nishi@aries.bsd1.kb.nec.co.jp>
	* options.c mallocが0クリアしない問題を修正

Wed Jul 6 1994		Yoshiaki KASAHARA <kasahara@csce.kyushu-u.ac.jp>
	* config.h の不具合を修正

Tue Jul 5 1994		Issei Numata <issei@jaist.ac.jp>
	* invent.c 発見済み宝石が正しく表示されるように修正
	* objnam.c 名づけられたアイテムが日本語らしくなるよう修正
	* end.c ペットに殺されたときの墓石のメッセージを修正
	* wintty.c -decでダンジョンがたまに文字化けするのを修正
	* メッセージやアイテムの細い修正

Sun Jul 2 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.3 beta test starts

Sun Jul 2 1994		Issei Numata <issei@jaist.ac.jp>
	* メッセージやアイテムの細い修正

Tue Jun 28 1994		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* quest.txt Elfの翻訳追加

Mon Jun 27 1994		Issei Numata <issei@jaist.ac.jp>
	* quest.txt Rogueの翻訳追加

Sun Jun 26 1994		Issei Numata <issei@jaist.ac.jp>
	* デフォルトのプログラム名を nethack -> jnethackへ変更

Sun Jun 26 1994		HAMADA Naoki <nao@jaist.ac.jp>
	* dialog.c Ctrl-m アクションを追加

Thu Jun 23 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.2 beta test starts

Thu Jun 23 1994		<hideki@bsd1.kb.nec.co.jp>
	* win/X11/winmesg.c X11R6でコアを吐く問題を修正．

Thu Jun 23 1994		Issei Numata <issei@jaist.ac.jp>
	* win/X11/*.c リソースに書かなくても国際化に対応するよう修正．

Wed Jun 22 1994		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* files.c .jnethackrcを先に見るようにした．
	* pager.c 罠の名前が正しく出るように変更．
	* jtrns_obj.dat	訂正
	* pray.c 「献上する」を正しく表示されるよう変更．
	
Tue Jun 21 1994		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* shk.c 店のなかでものを壊したときにcoreを吐くのを修正

Mon Jun 16 1994		Issei Numata <issei@jaist.ac.jp>
	* data.baseの翻訳着手

Mon Jun 16 1994		Issei Numata <issei@jaist.ac.jp>
	* do_name.c hallucination時のbogus monsterを正しく表示するよう修正
	* sys/unixmain.c -sオプション時に文字化けするのを回避

Mon Jun 15 1994		Issei Numata <issei@jaist.ac.jp>
	* jconj.c 終止形からも活用できるようにjconj_adj 修正

Mon Jun 14 1994		Issei Numata <issei@jaist.ac.jp>
	* jconj.c isspace_8 修正

Mon Jun 14 1994		Koji Kishi <kis@rqa.sony.co.jp>
	* engrave.c jconj.c sony newsでiskanji2がかちあう問題を回避

Mon Jun 14 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.1 beta test starts

Tue Jun 14 1994		Issei Numata <issei@jaist.ac.jp>
	* end.c 終了時にcoreを吐くのを修正

Tue Jun 14 1994		HAMADA Naoki <nao@jaist.ac.jp>
	* shk.c shopkeeperのセリフ修正

Mon Jun 14 1994		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* dokick.c 物を蹴って死んだときのメッセージ訂正

Mon Jun 13 1994		Issei Numata <issei@jaist.ac.jp>
	* -sオプションで漢字コードを正しく認識するように改良
	* 終了時に画面が乱れるのを修正

Mon Jun 13 1994		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.0 beta test starts

Mon Jun 13 1994		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* dotogglelangの戻り値を 0に設定

Mon Jun 13 1994		Issei Numata <issei@jaist.ac.jp>
	* JIS/SJIS端末で表示ができるように改良
	* .nethackrcで漢字コードを選択できるように改良

Sun Jun 12 1994		Issei Numata <issei@jaist.ac.jp>
	* quest.txt に Cavemanの翻訳追加

Sat Jun 11 1994		Issei Numata <issei@jaist.ac.jp>
	* JNetHack-0.1.9 test starts.

Sat Jun 11 1994		Issei Numata <issei@jaist.ac.jp>	
	* quest.txt に Barbarianの翻訳追加
	* アイテム名，怪物名 オジナル<->日本語の切り替えサポート
	* jtrns_obj.dat改訂 

Fri Jun 10 1994		Issei Numata <issei@jaist.ac.jp>
	* 発見済みアイテムの英名/和名併記
	* 「水の薬」->「水」,「フルーツジュースの薬」->「フルーツジュース」
	* rumor.fal改訂

Thu Jun  9 1994		Tomoyuki Shiraishi <tshira@mtl.t.u-tokyo.ac.jp>
	* quest.txt に Healerの翻訳追加

Tue Jun  7 1994		Issei Numata <issei@jaist.ac.jp>
	* JNetHack-0.1.8 test starts.

