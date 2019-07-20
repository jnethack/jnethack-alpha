[![Linux/Mac](https://travis-ci.org/jnethack/jnethack-alpha.svg?branch=develop)](https://travis-ci.org/jnethack/jnethack-alpha)[![Windows](https://ci.appveyor.com/api/projects/status/2no8rc2q162ssb2c/branch/develop?svg=true)](https://ci.appveyor.com/project/argrath/jnethack-alpha/branch/develop)

# JNetHack

JNetHackはNetHackを日本語化したものです。

## 今回のリリースノート

NetHack 3.6.2 ベースの最初のリリースです。

詳細についてはChangeLog.jを参照してください。

## JNetHack 3.6.* の変更点

JNetHack 3.4.* から変更された主な点は以下の通りです。

 * ソースコード配布はShift_JISに変更しています。Unix環境でビルドする場合はEUC-JPに変換してください。
 * 願いや虐殺でユーザー入力を行う場合、日本語のみを受け付けます。
 * 入出力文字コードとしてJISは指定できなくなりました。その代わりUTF-8を指定できるようになっています。
 * Mac/UNIX環境ではデフォルトの入出力文字コードはUTF-8になっています。他の文字コードにする場合はkcodeオプションを使ってください。

## その他のドキュメント

 * ChangeLog.j: 変更履歴
 * READMEj1.txt: JNetHack 1.* 時代のREADME
 * japanese/build.md: ビルド方法

## レポジトリ

JNetHack では二つのレポジトリを使っています。

### メインレポジトリ https://osdn.net/projects/jnethack/scm/git/source/

ある程度確認したソースコードが登録されます。原則としてrebaseはしません。

### 開発レポジトリ https://github.com/jnethack/jnethack-alpha

開発中のソースが登録されます。頻繁にrebaseされます。
TravisCIとAppVeyorでビルドの確認をしています。
