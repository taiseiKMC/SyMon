# SyMon
* install
  * cmake に 3.25 を要求する
    * ubuntu20.04 の普通の apt で取れるバージョンは 3.16 で古く cmake 不可
    * apt-add-repository でアップグレードして解決
    ```
    % echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' 
      | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
    % wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null
      | gpg --dearmor -
      | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
    ```
  * boost が要る(書いてる)
  * PPL が要る
    * GCC 環境下には全て PPL があるそう
    * `apt install libppl-dev`
  * tree-sitter が要る
    * github から clone, release-0.26 へ checkout, make, make install を実行
    * cargo install tree-sitter-cli をしないと tree-sitter-symon の install でコケるかも
  * tree-sitter-symon が要る
    * grammar を定義
    * https://github.com/MasWag/tree-sitter-symon

  * option
    * formatter : https://github.com/MasWag/symon-format
    * emacs mode : https://github.com/MasWag/symon-mode

* docs: https://maswag.github.io/SyMon/
  * リンク無いな...
  * というかここに install 手順あるじゃん

* symbolic monitoring tool
  * 時系列データの仕様検証ツール
  * 非決定的なパラメータを記述可能

* Syntax
  * number_of_variables == number_of_variables がない
  * != と <> がある?
  * +, *, ? は何？ ~~二項演算子ではないのか~~ 四則演算かなこれ
  * one_of {} and {} とあるが、 one_of {} or {} と example にはある
  * メタ変数と式が区別つかんな...
  * メタ変数も色々記述不足感
  * timing_constraint
    * [a,b], [a,b), (a,b],(a,b)がある
    * (comparator, n) がある

* モードがある (FULL PARAMETRIC になるにつれ重くなる)
  * non-parametric and Boolean mode (default)
    * 数値や文字列が扱えない, カウンターのみ
    * 定数扱えない？？
  * data-parametric mode.
    * 時間制約に対するパラメーターが扱えない
  * fully parametric mode.
* `-n` experimental syntax がある？ <- というかこっちがチュートリアルに書いている方
  * もともと dot ファイルで Automaton を与えていたところを, 専用正規表現もどきに置き換えようとしているっぽい
  * -n でない場合、.symon ファイルに書いていた signature のフォーマットと automaton を別ファイル .sig と .dot で与える

* test
  * レシピは以下
    ```
    cd build
    cmake ..
    make unit_test
    ./unit_test
    ```
  * dataparametric_monitor のテストがなさそう

## Issues
https://github.com/MasWag/SyMon/issues/
* #5: Use PPLRational in DataParametricMonitor
  * ppl_rational.cc を実装してあって、これを使って DataParametricMonitor も置き換えて欲しい
  * PPLRational の中身も Parma_Polyhedra_Library::Coefficient を使って分数にしてある(だけ)
* #6 : Support unobservable transitions in DataParametricMonitor
  * any unobservable edges in the automaton  とは?
  epsilon遷移のこと
* #7 : Support unobservable transitions in BooleanMonitor
* #8 :Support updates by expressions in BooleanMonitor
  * 今、変数の update に変数を使わないといけず、定数を与えられない. これを更新する
  * parametricmonitor 含め `data( n | | str := "foo" )` みたいな string の定数による update が symon 記法でかけない
    * そもそも
      * tree-sitter-symon の grammer で定義されてない
      * string の Update で stringExpr を受け取れず, データ構造的に保持できない
* #9 : Support more general form of numeric guards in BooleanMonitor
  * BooleanMonitor が `<expression> <op> <constant>` の形式しかサポートしていない. `<expression> <op> <expression>` をサポートしたい
* #10 : Add filter operator
  * filter 演算子？を追加する
    * event と field のペアの列を与えて絞り込み、マッチングする
* #11 : Support non-integer timing constraints
  * 少数を timing_constraint としてサポートしたい

## Source
* main.cc
  * monitor は printer を保持
    * notifyObserver を呼ぶと printer が出力する
  * monitor と TimedWordSubject はどちらも SingleSubject を継承
    * SingleSubject は observer を高々1つしか持たない observer パターン実装
* printer : Observer を継承, notify(T) を実装
* TimedWordSubject : 時系列データをパースして、1つずつ event として monitor に与える
  * notifyObserver で event を与え, monitor の notify が実行される
* clockVariableSize : 多分 variable の変数の数, clockValuation とも？
* cval : clock variable
* nval : number variables

* reset : ある遷移に乗じて特定の clock 変数を 0 に初期化することな気がする

* ppl_rational.cc
  * 分数, Parma_Polyhedra_Library::Coefficient(多分整数) を使って定義してある
  * 少数をパースしてそう
* printer.hh
  * cout と printf が混じってる...

* automaton.hh
  * Automaton : state, initialstate の集合
  * AutomatonState : isMatch(受理状態フラグ？) と next (Action -> vector<Transition>(stringConstraint, numberConstraint, guard 等) の map)
    * 非決定性のために Transition は vector になっているらしい
  * stringConstraint : string の制約
  * numberConstraint : number の制約
  * guard : 多分 clock に関する制約

* string, "" なのか '' なのかどっち？

* symbolic_number_constraint.hh
  * 凸包を扱う都合上、!= は扱えない(エラーハンドリングしたほうが良い)

* parametric_timing_constraint_helper.hh
  * comparator_t には NE がないが、grammer.json には <> が既定されているなぁ

* symon_parser.hh
  * TimeConstraint の型で処理を分岐する箇所がある
    * SFINAE とかで内部分岐というよりは外側で分岐した方が良い気がする

# MTG Note
## 10/19
#5-#8 は fully-parametric では実装済み, boolean monitor では扱えない
* 具体例を作ってちゃんと走らせる までやる

* symon_parser : parse して automaton を構築

* 11月末2週間前がどんなに遅くても発注期限
  * だめなら FalCAuN の Matlab class の実装をして欲しいそう
    * シミュレーションしたい class を用意するのにリードタイムが必要
* 早めに見切りをつけたい

# QA
* parameter と variable は違う？
  * dot ファイルには x0 だったり p0 だったりで記述してあるが... 違いは？
    * 非決定的な変数は dot では p0 で記述するが、symon 記法だと区別せず variable で書く みたいな感じ？


b src/parametric_timing_constraint_helper.hh:134
r -pnf ../example/decimal-timing-constraint/decimal-timing-constraint.symon < ../example/decimal-timing-constraint/decimal-timing-constraint.txt
