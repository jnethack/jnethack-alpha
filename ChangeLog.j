	* 次の問題を修正
	  * 地図上の位置を指定しようとすると落ちる (#31)
	  * 外部ファイルを表示すると落ちる (#32)

2026-09-20  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-5.0.0ベース JNetHackテストリリース(5.0.0-0.1)

2026-09-10  Kentaro Shirakata  <argrath@ub32.org>

	* 次の問題を修正
	  * 厄介者の召喚で怪物が召喚されない (OSDN:47879)
	  * Windows版のデフォルトのデータ保存位置が"NetHack"のまま (OSDN:48255)
	* 配布物からポータブル設定を削除	  
	* 翻訳追加修正

2023-02-23  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.7ベース JNetHackテストリリース(3.6.7-0.1)
	* 翻訳追加修正

2022-10-23  SHIRAKATA Kentaro  <argrath@ub32.org>

	* 次の問題を修正
	  * ubuntu 22.04でビルドが失敗する (OSDN:45951)
	* 翻訳追加修正

2022-01-03  SHIRAKATA Kentaro  <argrath@ub32.org>

	* 次の問題を修正
	  * unix系でビルド・プレイできない (OSDN:43519)

2022-01-01  SHIRAKATA Kentaro  <argrath@ub32.org>

	* 次の問題を修正
	  * win32/ttyで「あなたは誰?」に日本語を使うと文字化け (OSDN:42760)
	* ソースUTF-8化への準備作業
	* 翻訳追加修正

2021-08-17  Kentaro Shirakata  <argrath@ub32.org>

	* 次の問題を修正
	  * rumorsの末尾にゴミが表示される (OSDN:41899)
	  * Win10で画面左端の文字が乱れる (OSDN:42072)
	  * exeファイルのプロパティが未調整 (OSDN:42037)
	  * 「ゲームのオプション一覧」を表示させると画面が乱れる (OSDN:42529)
	  * 「ゲームのオプション一覧(長文)」で表示が乱れる (OSDN:42547)
	  * ローグレベルで表示が乱れる (OSDN:42549)
	* X11のロケールを内部でja_JP.EUC-JPに固定する (OSDN:40804)
	* 翻訳追加修正

2021-03-13  Kentaro Shirakata  <argrath@ub32.org>

	* 次の問題を修正
	  * X11環境で日本語入力ができない (OSDN:40562)
	  * 一部のオプションで英語のフルネームを使えない (OSDN:40657)
	  * X11版でascii_mapが正しく表示されない (OSDN:40530)
	  * X11でascii_map表示するとリソースエラーが発生する (OSDN:40803)
	  * 同じ階に寺院があるときのメッセージが文字化け (OSDN:40957)
	* X11LARGETILEを削除(OSDN:40681)
	* JNetHack.adを最新に更新(OSDN:40531)

2020-04-18  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.6ベース JNetHackテストリリース(3.6.6-0.1)
	* 次の問題を修正
	  * HPが0になったときに命の魔除けが効かない (OSDN:40316)

2019-12-08  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.2ベース JNetHackテストリリース(3.6.2-0.1)
	* 次の問題を修正
	  * 銀が苦手な怪物を銀の武器で攻撃したときのメッセージがおかしい
	    (OSDN:39767)

2019-06-02  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.1-0.4リリース
	* 次の問題を修正
	  * 死体に蘇生の杖を振るとクラッシュする (OSDN:39081)
	  * 炎でスライム化の解除ができない (OSDN:39134)
	* unixでのデフォルト文字コードをUTF-8に変更(OSDN:39273)

2018-12-26  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.1-0.3リリース
	* MacOSでのビルドに対応
	* 願いの処理を拡充
	* ビルド方法の文書を追加(japanese/build.md)
	* AppVeyorでのビルドに対応
	* 翻訳追加修正

2018-10-07  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.1-0.2リリース
	* 次の問題を修正
	  * #wizwhereコマンドでクラッシュする (OSDN:38569)
	  * 2バイト文字表示時に属性変更が行われない (OSDN:38566)
	  * 正規表現が正しく動作しない (OSDN:38565)
	  * 怪物/アイテムデータが3.6.1に更新されていない (OSDN:38576)
	  * ウィザードモードで魔法の罠を願えない (OSDN:38584)
	  * タイル作成時に警告が出る (OSDN:38608)
	  * 祭壇捧げメニューで食べ物や魔除けがフィルタされていない (OSDN:38620)
	* VisualStudio 2015 IDEでのビルドに対応
	* コンパイル警告を抑制
	* 翻訳追加修正

2018-06-21  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.1ベース JNetHackテストリリース(3.6.1-0.1)

2018-04-29  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.9リリース
	* ISO-2022-JP対応を削除
	* UTF-8出力に対応 (OSDN:37910)
	* 内部コードと表示コードが違う時の問題を修正:
	  * -uオプションで日本語を使うと文字化けする (OSDN:37911)
	  * -sオプションで日本語を使うと文字化けする (OSDN:37912)
	  * 日本語入力が正しく動作しない (OSDN:37913)
	* 翻訳追加修正

2017-02-17  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.8リリース
	* 画面上の怪物を/コマンドで見た際に文字化けする問題を修正 (OSDN:36909)
	* NPCの僧侶/尼僧/法王が神の名を冠していない問題を修正 (OSDN:36910)
	* ウィザードモードでイェンダーの魔除けが願えない問題を修正 (OSDN:36939)
	* オーク感知中のスティングの輝き表示が無い問題を修正 (OSDN:36953)
	* 願いで得たものを取り落とした時のメッセージがおかしい問題を修正
	  (OSDN:36991)
	* 怪物名の付いた物が願えない問題を修正 (OSDN:36992)
	* 翻訳追加修正

2016-12-07  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.7リリース
	* 一部の死因で「死んだ」が表示されない問題を修正 (OSDN:36579)
	* Windows10のttyで日本語が入力できない問題を修正 (OSDN:36599)
	* 盗賊クエストとジョウビレックスフロアで「障害発生」が出る問題を修正
	  (OSDN:36640)
	* 翻訳追加修正

2016-06-11  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.6リリース
	* GCCでの警告を抑制
	* 翻訳追加修正

2016-04-03  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.5リリース
	* 氷の上を歩くと「障害発生」が出る問題を修正 (OSDN:36207)
	* クエスト最下層でクエスト聖器が生成されない問題を修正 (OSDN:36215)
	* 店でアイテムを売るときに「障害発生」が出る問題を修正 (OSDN:36223)
	* 店主がお金を支払えないときにクラッシュする問題を修正 (OSDN:36224)
	* 翻訳追加修正

2016-03-24  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.4リリース
	* 一部のファイルの文字コードがEUC-JPになっていた問題を修正 (OSDN:36189)
	* 翻訳追加修正

2016-03-18  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.3リリース
	* unix/X11対応
	* JNetHack.adを現状に合うように更新
	* VS2013のIDEでビルド出来るように調整
	* 噂、神託、ヘルプの翻訳を追加
	* 翻訳追加修正

2016-02-21  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.6.0-0.2リリース
	* windows/gui対応
	* 鉱山最下層や倉庫番最終面で「障害発生」が出る問題を修正 (OSDN:36038)
	* win/X11/winX.c のパッチ適用漏れを修正 (OSDN:36039)
	* 翻訳追加修正

2016-02-14  Kentaro Shirakata  <argrath@ub32.org>

	* NetHack-3.6.0ベース JNetHackテストリリース(3.6.0-0.1)

2013-04-08  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.11リリース
	* 占いクッキー専用メッセージ処理が動作していなかった問題を修正
	  (OSDN:16381)
	* Windows版でセーブファイル名に日本語が使えるように修正 (OSDN:18272)
	* 物を拾うときにバッファオーバーランする問題を修正 (OSDN:26691)
	* 眠っている騎乗モンスターを蹴るとクラッシュする問題を修正 (OSDN:30923)
	* 翻訳追加修正

2008-05-04  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.10リリース
	* 床の長いメッセージを読むとクラッシュする問題を修正
	* 一部のアイテムが日本語で願えない問題を修正
	* 翻訳追加修正

2007-05-02  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.9リリース
	* WindowCEに一部対応
	* 翻訳追加修正

2006-04-01  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.8リリース
	* 一部のアイテムが願えない問題を修正
	* シャツの上からシャツを着ようとするとクラッシュする問題を修正
	* 日本語での名付けで聖器が作成できない問題を修正
	* 翻訳追加修正

2005-07-31  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.7リリース
	* DOS版でスコア表示が文字化けしていた問題を修正
	* アエスキュラピスの杖が願えない問題を修正
	* 翻訳追加修正

2005-04-28  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.6リリース
	* アイテムや地形の詳細表示がされない問題を修正
	* クエストに関する表示が一部乱れる問題を修正
	* 翻訳追加修正

2005-01-09  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.5リリース
	* 存在しないオプションをdefault.nhに設定したときにクラッシュする
	  問題を修正
	* 日本語でデータベースが呼び出せない場合がある問題を修正
	* 翻訳追加修正

2004-08-02  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.4リリース
	* 複数形で願った時の処理を#ifdefで分離
	* 変身と名前返答の時に日本語が使えなかった問題を修正
	* 翻訳追加修正

2004-04-29  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.3リリース
	* 虐殺で日本語が使えなかった問題を修正
	* 乗馬用アイテムの効果が効いていなかった問題を修正
	* 翻訳追加修正

2004-02-19  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.2リリース
	* win/ttyがBCCでコンパイルできなくなっていた問題を修正
	* 翻訳追加修正

2003-12-31  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.3-0.1リリース
	* NetHack-3.4.3ベースに対応

2003-12-05  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.2-0.2リリース
	* win/ttyでスコア一覧表示が乱れる問題を修正
	* 素手で酸のブロッブを殴るとまれにクラッシュする問題を修正
	* 翻訳追加修正

2003-11-11  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.2-0.1リリース
	* NetHack-3.4.2ベースに対応

2003-09-03  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.4リリース
	* 翻訳追加修正

2003-08-09  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.3リリース
	* Reportパッチを分離
	* コマンドラインで指定した名前の漢字コードが変換されない問題を修正
	* 翻訳追加修正

2003-06-09  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.2リリース
	* winnt/ttyで日本語入力が出来ない問題を修正
	* 翻訳追加修正

2003-04-10  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.1-0.1リリース
	* NetHack-3.4.1ベースに対応

2003-02-14  Kentaro Shirakata  <argrath@ub32.org>

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

2002-12-20  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.9リリース
	* 死の谷で上り階段が生成されないことがある問題(C340-27)を修正
	* 武闘家でレベル13の時冷気耐性がつかない問題を修正
	* Windows/GUI版がWin95系で文字化けする問題を修正
	* Boland C++でのコンパイルに対応
	* Fighterパッチを分離
	* 本家でGnome対応されているので旧GTK_GRAPHICS関係を削除
	* 翻訳追加修正

2002-09-16  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.8リリース
	* ボーパルブレードで幽霊を切ると落ちる問題を修正
	* 怪物が人間に変身したときに職業表示が文字化けする問題を修正
	* 翻訳追加修正

2002-08-05  Kentaro Shirakata  <argrath@ub32.org>
	* dos(djgpp)対応開始

2002-08-03  Kentaro Shirakata  <argrath@ub32.org>

	* JNetHack-3.4.0-0.7リリース
	* jGuidebookを追加
	* data.base,quest.datをそれぞれjdata.base,jquest.datに変更
	* rogueレベルで'@'が文字化けする問題を修正
	* 地雷でその上にある巨石/像が壊れると落ちる問題(C340-2)を修正
	* 翻訳追加修正

2002-07-07  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.6リリース
	* フロストブランドの攻撃が命中すると落ちる問題を修正
	* 数字を"/"コマンドで調べると落ちる問題を修正
	* 音に関するメッセージが全く表示されていなかった問題を修正
	* 翻訳追加修正

2002-06-14  Kentaro Shirakata  <argrath@ub32.org>
	* JnetHack-3.4.0-0.5リリース
	* winnt/ttyで表示が乱れる問題を仮修正
	* アイテム命名が反映されないことがある問題を修正
	* 翻訳追加修正

2002-05-24  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.4リリース
	* トリックの鞄を#lootしたときのダメージがおかしいバグを修正
	* "."で休憩できないバグを修正
	* プレイヤーがモンスターを飲み込むと無条件で石化するのを修正
	* 翻訳追加修正

2002-05-16  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.3リリース
	* winnt/tty対応開始
	* data.baseを処理
	* 翻訳追加修正

2002-05-12  Kentaro Shirakata  <argrath@ub32.org>
	* JNetHack-3.4.0-0.2リリース
	* winnt/win32対応開始
	* default.nhを読もうとするとエラーになるのを修正
	* 翻訳追加修正

2002-05-09  Kentaro Shirakata  <argrath@ub32.org>
	* NetHack-3.4.0ベース JNetHackテストリリース(3.4.0-0.1)

2000-04-02	Issei Numata  <issei@jnethack.org>
	* JNetHack-1.2.0βバージョンリリース

2000-03-14	Hideaki YOSHIFUJI (吉藤英明) <yoshfuji@ecei.tohoku.ac.jp>
	* IPV6に対応

2000-03-10	Issei Numata  <issei@jnethack.org>
	* JNetHack-1.2.0αバージョンリリース

2000-03-01	Issei Numata  <issei@jnethack.org>
	* オープニングメッセージを変更
	* NetHack-3.3.0ベース JNetHack開発開始

2000-02-12	Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.5 リリース

2000-02-07		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.4 リリース

2000-02-07		Issei Numata  <issei@jnethack.org>
	* NetHack-3.2.3に対応

1999-12-01		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.3 リリース

1999-11-21		Issei Numata  <issei@jnethack.org>
	* プロキシをオプションで指定できるよう変更

1999-11-19		Issei Numata  <issei@jnethack.org>
	* 3Dマップ対応

1999-11-17		Issei Numata  <issei@jnethack.org>
	* bonesシステム導入

1999-11-09		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.2 リリース

1999-11-09		Issei Numata  <issei@jnethack.org>
	* proxyを使えるように変更
	* 細かいバグ修正

1999-11-08		Issei Numata  <issei@jnethack.org>
	* httpでスコアを転送できるように変更

1999-11-02		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.1 リリース

1999-11-01		Issei Numata  <issei@jnethack.org>
	* 西暦2000年問題に対処
	* menuに HP/MP barを追加
	* 細かいバグ修正

1999-10-28		Issei Numata  <issei@jnethack.org>
	* monster detectionなどで画面が止まらない問題を修正
	* 伝統的文字モードを選択可能に

1999-10-24		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.1.0 リリース
	* 細かいバグ修正

1999-10-23		Issei Numata  <issei@jnethack.org>
	* credit画像追加

1999-10-21		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.0.7.19991021リリース

1999-10-19		Issei Numata  <issei@jnethack.org>
	* JNetHack-1.0.7.19991019リリース

1999-10-16		Issei Numata  <issei@jnethack.org>
	* warningを減らす
	* 細かいバグ修正

1999-10-09		Issei Numata  <issei@jnethack.org>
	* newbieモード追加

1999-10-09		Issei Numata  <issei@jnethack.org>
	* jnethack-1.0.6.3公開

1999-10-08		Issei Numata  <issei@jnethack.org>
	* タイルを使用しないときはradarを非表示にするように修正
	* radarの上で ESCを押したら非表示に修正
	* optionに radarの on/offの切替を追加
	* スコアサーバに接続するとき20秒でタイムアウトするよう変更
	* その他細いバグを修正

1999-10-08		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* スコア表示のバグを修正

1999-10-07		ITAKURA Mitsuhiro <ita@gold.koma.jaeri.go.jp>	
	* 透明ピックスマップの高速化

1999-10-07		Issei Numata  <issei@jnethack.org>
	* optionに homeurlを追加
	* デフォルトのオプション変更
	  perm_invent -> off
	  TEXT_COLORをデフォルトで #define

1999-10-07		Issei Numata  <issei@jnethack.org>
	* 透明ピックスマップに対応
	* JNetHack.ad を HACKDIRに置けるように変更

1999-10-06		Issei Numata  <issei@jnethack.org>
	* jnethack-1.0.6.1公開

1999-10-06		Issei Numata  <issei@jnethack.org>
	* Linuxでも簡単にコンパイルできるよう修正
	* ripのcolormapが正しく設定されない問題を修正
	* レーダーの上でもキーを受けつけるよう修正
	* X版はオプションの有無にかかわらず、number padを移動に
	  使えるように修正

1999-10-05		Issei Numata  <issei@jnethack.org>
	* jnethack-1.0.6公開

1999-10-04		Issei Numata  <issei@jnethack.org>
	* デフォルトのオプション値変更
	  autopickup -> off
	  perm_invent -> on
	  mail -> off
	  windowtype 'tty' -> 'x11'

1999-10-03		Issei Numata  <issei@jnethack.org>
	* レーダー表示追加
	* 色数が少ないモニタでも動作するよう変更

1999-10-01		Issei Numata  <issei@jnethack.org>
	* 標準タイルを32x32のものに変更
	* スコア表示を変更

1999-09-30		Issei Numata  <issei@jnethack.org>
	* スコアをホストに転送するシステム作成

1997-09-24		Issei Numata  <issei@nna.so-net.or.jp>
	* class名を NetHackから JNetHackへ変更

1997-09-08		uchidak@creo.co.jp
1997-08-25
	* メッセージの修正

1997-07-22		KIHARA Hideto <kihara@shiratori.riec.tohoku.ac.jp>
	* メッセージの修正

1997-07-03		Kentaro Shirakata <shira@kdel.info.eng.osaka-cu.ac.jp>
	* nethack-3.2.2ベースのjnethackの開発に着手
	* quest.txtの訳語修正．

1996-12-24		Issei Numata  <issei@nna.so-net.or.jp>
	* jnethack-1.0.5.4.1 release
	* 性別女でKを選ぶとPになってしまうバグを本当に修正．

1996-11-23		Issei Numata  <issei@nna.so-net.or.jp>
	* jnethack-1.0.5.4 release
	* ドキュメントの修正(とてもここに名前は書き切れません)
	* メッセージの修正(とてもここに名前は書き切れません)

1996-09-06		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.5.2 release

1996-08-23		Issei Numata  <issei@jaist.ac.jp>
	* Ｆの死体や幽霊が例の場所に現われない問題の修正．

1996-08-22		Issei Numata  <issei@jaist.ac.jp>
	* X11で perminventをオプションで指定するとアイテムを拾ったときに
	  落ちる問題の修正
	* ユーティリティrecoverで回復できない問題の修正． (NetHackのバグ) 
	* Class Genocideしたときのメッセージが英語の修正． 
	* 聖器(アーティファクト)を願って失敗したときのメッセージが英語． 

1996-08-21		Issei Numata  <issei@jaist.ac.jp>
	* japanese/jlib.c の is_kanji1の修正
	* 職業 Fの Questでの Guardのメッセージが変
	* 職業 Fで Neutralの神が英語
	* 薬がジュースや水になるときのメッセージ．
	* 毒の攻撃を食って死ぬと怪物の名前．
	* たまにマップの文字が化ける(とくに jisで遊んでいる時)問題．
	* 聖器(アーティファクト)に触れて死んだときの死因が英語
	* 落書の文字が化ける

1996-08-19		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.5.1 release

1996-08-19		Issei Numata  <issei@jaist.ac.jp>
	* いくつかの日本語関係の表示のバグを解決

1996-08-16		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.5 release

1996-08-15		Issei Numata  <issei@jaist.ac.jp>
	* 短いファイルネームのシステムに対応できるよう，jtrns_*.cを改名

1996-08-14		Issei Numata  <issei@jaist.ac.jp>
	* DOS/Vで tile表示を可能に．

1996-08-08		Issei Numata  <issei@jaist.ac.jp>
	* Jだけのオリジナル職業(F)を追加

1996-08-02		juan@super.win.or.jp (Kenji Gotoh)
	* sjisで wishingするとき旨くいかないことがある問題を修正

1996-07-31		Satoh Yoshiyuki <yo-satou@system.hitachi.co.jp>
	* いくつかのメッセージの間違いを修正

1996-07-30		Hiramoto Kouji (平本光二 ) <hiramoto@Nurse.Angel.nanako.or.jp>
	* PCでスタート時に英語のメッセージが出る問題の修正

1996-07-29		Hideto KIHARA <kihara@shiratori.riec.tohoku.ac.jp>
	* いくつかのメッセージの間違いを修正

1996-07-25		fukui@nm.nmcc.co.jp (Terushige Fukui)
	* 存在しないものを wishしたときに落ちる問題を修正

1996-07-25		Issei Numata  <issei@jaist.ac.jp>
	* 日本語による wishingを一部可能に

1996-07-24		Hideto KIHARA <kihara@shiratori.riec.tohoku.ac.jp>
	* jtrns_monの副作用の問題を修正

1996-07-23		juan@super.win.or.jp (Kenji Gotoh)
	* ステータス欄を全て書き換えてしまう問題を修正

1996-07-22		Naoki Hamada <nao@sbl.cl.nec.co.jp>
	* DEC graphics で漢字が化ける問題を修正

1996-07-22		Hideto KIHARA <kihara@shiratori.riec.tohoku.ac.jp>
	* メッセージのまちがいを修正
	* jconjのバグの修正
	* 「ら」抜き言葉の修正

1996-07-20		shira@kdel.info.eng.osaka-cu.ac.jp (Kentaro Shirakata)
	* PCでコンパイルエラーが出るのを修正

1996-07-19		Issei Numata  <issei@jaist.ac.jp>
	* いくつかの Warningを削除
	* formatの間違いを修整

1996-07-18		Issei Numata  <issei@jaist.ac.jp>
	* 言語モードの切替がうまくいかないのを修整

1996-07-18		Issei Numata  <issei@jaist.ac.jp>
	* jnethack-1.0.4-alpha release

1996-06-21		Issei Numata  <issei@jaist.ac.jp>
	* nethack-3.2.1ベースのjnethackの開発に着手

1996-05-02		Issei Numata  <issei@jaist.ac.jp>
	* nethack-3.2.0ベースのjnethackの開発に着手

1995-06-04		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.3 release

1995-06-04		nishi@bsd1.kb.nec.co.jp
	* nh14.bdfをパッケージに追加

1995-06-02		Issei Numata <issei@jaist.ac.jp>
	* スコアリストのメッセージのバグを修整．
	* ドキュメントの間違いを修整．

1995-02-23		Hiramoto Kouji (平本光二 ) <hiramoto@kubota.co.jp>
	* resotre.c メッセージを修整．

1995-02-21		Hiramoto Kouji (平本光二 ) <hiramoto@kubota.co.jp>
	* cmd.c 「あなたは」が重複するのを修整．

1995-01-05		asami@cs.berkeley.edu
	* 全般的にメッセージの誤訳を修整．

1994-12-27		藤枝 和宏 (FUJIEDA Kazuhiro) <fujieda@jaist.ac.jp>
	* fountain.c 流し台の水を飲んで薬が出てきたときのメッセージを修整

1994-12-20		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.2 release

1994-10-27		narusawa@nsis.cl.nec.co.jp (Masaru Narusawa)
	* NEWOS用のパッチ追加．

1994-10-14		Issei Numata <issei@jaist.ac.jp>
	* X11版で長いメッセージが出たときにpanicになる問題を修正．
	* 自分自身をgenocideしたときのメッセージを修正．

1994-09-19		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* mhitm.c ニンフが他のモンスターを攻撃したときにcodeを吐く問題を
	  修正．

1994-09-12		Issei Numata <issei@jaist.ac.jp>
	* オリジナルモードで死んだときスコアファイルが英名になるのを修正．
	* 怪物の攻撃で病気になったときのメッセージを修正．
	* helpに英名-和名の変換テーブル追加．

1994-09-08		Issei Numata <issei@jaist.ac.jp>
	* コカトリスの死体で死んだときのメッセージを修正．
	* メデューサで石化したときのメッセージを修正．
	* 迷宮から脱出したときのメッセージを修正．
	* たくさんの誤訳，誤字を修正．
	* quest.txtを整形．
	* jisモードのとき \033$@, \033$Jも有効になるように修正．

1994-08-29		Issei Numata <issei@jaist.ac.jp>
	* 環境変数と設定ファイル両方を設定した場合のエラーを修正．

1994-08-29		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.1 release

1994-08-23		Issei Numata <issei@jaist.ac.jp>
	* vorpale bladeが首を切り落したときのメッセージを正しく表示する
 	  よう修正．

1994-08-20		Issei Numata <issei@jaist.ac.jp>
	* 細いメッセージの修正．

1994-08-19		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* data.base改訂．

1994-08-17		Masayuki Koba <masa-k@sag.hitachi-sk.co.jp>
	* NetHack.ad 16ドットフォントをデフォルトで使うよう修正．

1994-08-13		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* 迷宮から抜けたときのメッセージの不具合を修正．

1994-08-07		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* DJGPP版パッケージ追加．

1994-08-07		高田　幸治 <HAE00146@niftyserve.or.jp>
	* MSC版パッケージ追加．

1994-08-04		Yoshiki Ogino <azrael@cosmos.ces.kyutech.ac.jp>
	* questpgr.c アライメント表示のバグを修正．
 
1994-08-04		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* mk_artifact以外で，artifactを得たとき正しく作成されないのを修正．
	* class genocideで殺すことができなかったモンスターの名前を正しく
	  表示できるよう修正．

1994-08-01		Issei Numata <issei@jaist.ac.jp>
	* jnethack-1.0.0 release

1994-08-01		Issei Numata <issei@jaist.ac.jp>
	* 細かいメッセージの修正．

1994-07-31		Kentaro Shirakata <shira@kdel.info.osaka-cu.ac.jp>
	* 刃のない武器で攻撃されたときのメッセージを修正．

1994-07-25		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* magicbane関係のメッセージを修正．

1994-07-25		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.3.1 beta test starts

1994-07-23		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* artifact.c magicbaneの訳を修正．

1994-07-25		Issei Numata <issei@jaist.ac.jp>
	* topl.c 長い文字列のフォールディングを修正．
	* 長い名前をつけた際に文字化けするのを修正．

1994-07-23		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* jtrns_{obj,mon}.dat 細い訳語の間違いを修正．

1994-07-23		Issei Numata <issei@jaist.ac.jp>
	* jGuideBook.txtを JNetHackにあわせ修正．

1994-07-22		shira@kdel.info.osaka-cu.ac.jp (Kentaro Shirakata)
	* jGuideBook.txtをパッケージに追加．

1994-07-21		Issei Numata <issei@jaist.ac.jp>
	* jtrns_obj.dat 武器を強くする巻物->武器に魔法をかける巻物へ変更．
	* getline.c kinput2から漢字を入力できるように変更．

1994-07-21		Yoshiaki KASAHARA <kasahara@csce.kyushu-u.ac.jp>
	* 麻痺->眩暈に変更．

1994-07-21		Issei Numata <issei@jaist.ac.jp>
	* objnam.c nemesisの死体名が正しく表示されるよう修正．

1994-07-21		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.3.0 beta test starts	

1994-07-19		s92458km@sfc.keio.ac.jp (the kunedog)
	* quest.txt Turist の翻訳追加

1994-07-15		Issei Numata <issei@jaist.ac.jp>
	* priest.c 天使の名前が正しく表示されるよう修正．
	* end.c topten.c 死んだときのメッセージの不具合を修正．
	* objnam.c 長い名前の文字でcoreを吐く問題を修正．

1994-07-14		Issei Numata <issei@jaist.ac.jp>
	* jconj.c jtrns_obj.dat 聖器の名前が正しく表示されないのを修正．

1994-07-14		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.4 beta test starts

1994-07-14		Issei Numata <issei@jaist.ac.jp>
	* termcap.c -decで文字が化けるのを修正．
	* 聖器に全部和名を与えた．

1994-07-13		Issei Numata <issei@jaist.ac.jp>
	* jtrns_{obj,dat}をソースに埋めこみ，makedefsで対応するよう修正．
	* warningを減らした．

1994-07-12		Hideharu Miyazaki <hideha-m@ascii.co.jp>
	* printf系の引数の間違いを修正
	* extern.h locomotion2のプロトタイプを追加
	* コメントアウトのミスを修正

1994-07-10		Issei Numata <issei@jaist.ac.jp>
	* eat.c コカトリスで石化したときのメッセージの不具合を修正．

1994-07-07		<mya@jaist.ac.jp
	* quest.txt Knight の翻訳追加

1994-07-07		<nishi@aries.bsd1.kb.nec.co.jp>
	* options.c mallocが0クリアしない問題を修正

1994-07-06		Yoshiaki KASAHARA <kasahara@csce.kyushu-u.ac.jp>
	* config.h の不具合を修正

1994-07-05		Issei Numata <issei@jaist.ac.jp>
	* invent.c 発見済み宝石が正しく表示されるように修正
	* objnam.c 名づけられたアイテムが日本語らしくなるよう修正
	* end.c ペットに殺されたときの墓石のメッセージを修正
	* wintty.c -decでダンジョンがたまに文字化けするのを修正
	* メッセージやアイテムの細い修正

1994-07-02		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.3 beta test starts

1994-07-02		Issei Numata <issei@jaist.ac.jp>
	* メッセージやアイテムの細い修正

1994-06-28		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* quest.txt Elfの翻訳追加

1994-06-27		Issei Numata <issei@jaist.ac.jp>
	* quest.txt Rogueの翻訳追加

1994-06-26		Issei Numata <issei@jaist.ac.jp>
	* デフォルトのプログラム名を nethack -> jnethackへ変更

1994-06-26		HAMADA Naoki <nao@jaist.ac.jp>
	* dialog.c Ctrl-m アクションを追加

1994-06-23		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.2 beta test starts

1994-06-23		<hideki@bsd1.kb.nec.co.jp>
	* win/X11/winmesg.c X11R6でコアを吐く問題を修正．

1994-06-23		Issei Numata <issei@jaist.ac.jp>
	* win/X11/*.c リソースに書かなくても国際化に対応するよう修正．

1994-06-22		Kazuhiro Fujieda <fujieda@jaist.ac.jp>
	* files.c .jnethackrcを先に見るようにした．
	* pager.c 罠の名前が正しく出るように変更．
	* jtrns_obj.dat	訂正
	* pray.c 「献上する」を正しく表示されるよう変更．
	
1994-06-21		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* shk.c 店のなかでものを壊したときにcoreを吐くのを修正

1994-06-16		Issei Numata <issei@jaist.ac.jp>
	* data.baseの翻訳着手

1994-06-16		Issei Numata <issei@jaist.ac.jp>
	* do_name.c hallucination時のbogus monsterを正しく表示するよう修正
	* sys/unixmain.c -sオプション時に文字化けするのを回避

1994-06-15		Issei Numata <issei@jaist.ac.jp>
	* jconj.c 終止形からも活用できるようにjconj_adj 修正

1994-06-14		Issei Numata <issei@jaist.ac.jp>
	* jconj.c isspace_8 修正

1994-06-14		Koji Kishi <kis@rqa.sony.co.jp>
	* engrave.c jconj.c sony newsでiskanji2がかちあう問題を回避

1994-06-14		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.1 beta test starts

1994-06-14		Issei Numata <issei@jaist.ac.jp>
	* end.c 終了時にcoreを吐くのを修正

1994-06-14		HAMADA Naoki <nao@jaist.ac.jp>
	* shk.c shopkeeperのセリフ修正

1994-06-14		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* dokick.c 物を蹴って死んだときのメッセージ訂正

1994-06-13		Issei Numata <issei@jaist.ac.jp>
	* -sオプションで漢字コードを正しく認識するように改良
	* 終了時に画面が乱れるのを修正

1994-06-13		Issei Numata <issei@jaist.ac.jp>
	* jnethack-0.2.0 beta test starts

1994-06-13		Yasuhito Futatsuki <futatuki@jaist.ac.jp>
	* dotogglelangの戻り値を 0に設定

1994-06-13		Issei Numata <issei@jaist.ac.jp>
	* JIS/SJIS端末で表示ができるように改良
	* .nethackrcで漢字コードを選択できるように改良

1994-06-12		Issei Numata <issei@jaist.ac.jp>
	* quest.txt に Cavemanの翻訳追加

1994-06-11		Issei Numata <issei@jaist.ac.jp>
	* JNetHack-0.1.9 test starts.

1994-06-11		Issei Numata <issei@jaist.ac.jp>	
	* quest.txt に Barbarianの翻訳追加
	* アイテム名，怪物名 オジナル<->日本語の切り替えサポート
	* jtrns_obj.dat改訂 

1994-06-10		Issei Numata <issei@jaist.ac.jp>
	* 発見済みアイテムの英名/和名併記
	* 「水の薬」->「水」,「フルーツジュースの薬」->「フルーツジュース」
	* rumor.fal改訂

1994-06-09		Tomoyuki Shiraishi <tshira@mtl.t.u-tokyo.ac.jp>
	* quest.txt に Healerの翻訳追加

1994-06-07		Issei Numata <issei@jaist.ac.jp>
	* JNetHack-0.1.8 test starts.

