# rbtree
`set`, `map`, `multiset`, `multimap` が可乱択で！！  
赤黒木は2-3木の二分木への自然な変換らしいですよ．知らんけど．  

## 目次
ちょっと記述が多いのでメンバ関数とインナークラスを以下に表にする．

### 型エイリアス
|名前|説明|
|-|-|
|`key_type`|キーの型 `key_t`|
|`value_type`|要素の型。`set`/`multiset`では`const key_t`、`map`/`multimap`では`std::pair<const key_t, val_t>`|
|`mapped_type`|値の型 `val_t`（`set`/`multiset`では`null_t`）|
|`key_compare`|比較関数の型 `Cmp`|
|`allocator_type`|アロケータの型 `Allocator`|
|`reference`|参照型 `value_type&`|
|`const_reference`|const参照型 `const value_type&`|
|`size_type`|サイズの型 `std::size_t`|
|`difference_type`|差の型 `std::ptrdiff_t`|
|`pointer`|ポインタ型 `value_type*`|
|`const_pointer`|constポインタ型|
|`iterator`|イテレータ型|
|`const_iterator`|constイテレータ型|
|`reverse_iterator`|逆イテレータ型|
|`const_reverse_iterator`|const逆イテレータ型|
|`set<key_t, cmp, Allocator>`|`rbtree<key_t, null_t, false, cmp, Allocator>`のエイリアス|
|`map<key_t, val_t, cmp, Allocator>`|`rbtree<key_t, val_t, false, cmp, Allocator>`のエイリアス|
|`multiset<key_t, cmp, Allocator>`|`rbtree<key_t, null_t, true, cmp, Allocator>`のエイリアス|
|`multimap<key_t, val_t, cmp, Allocator>`|`rbtree<key_t, val_t, true, cmp, Allocator>`のエイリアス|

### 構築・破棄
|名前|説明|
|-|-|
|[(constructor)](#constructor)</a>|コンストラクタ|
|[(destructor)](#destructor)</a>|デストラクタ|
|[(operator=)](#operator=)</a>|代入演算子|
### イテレータ
|名前|説明|
|-|-|
|[iterator](#iterator)|イテレータクラス|
|[const_iterator](#iterator)|書き換え禁止イテレータクラス|
|[reverse_iterator](#reverse_iterator)|イテレータが逆向きに動くだけ|
|[const_reverse_iterator](#reverse_iterator)|書き込み禁止イテレータが逆向きに動くだけ|
|[begin](#begin)|最小要素を $O(\lg N)$ で取得|
|[cbegin](#cbegin)|最小要素を $O(\lg N)$ で取得|
|[end](#end)|nil番兵 $O(1)$|
|[cend](#cend)|nil番兵 $O(1)$|
|[rbegin](#rbegin)|最大要素|
|[crbegin](#crbegin)|最大要素|
|[rend](#rend)|nil番兵|
|[crend](#crend)|nil番兵|
### 領域
|名前|説明|
|-|-|
|[empty](#empty)|コンテナが空であるかどうかを調べる|
|[size](#size)|要素数を取得する|
### 変更
|名前|説明|
|-|-|
|[clear](#clear)|すべての要素を削除する|
|[insert](#insert)|要素を挿入する|
|[erase](#erase)|要素を削除する|
### アクセス
|名前|説明|
|-|-|
|[count](#count)|指定したキーと同じキーを持つ要素の数を数える|
|[find](#find)|指定したキーと同じ要素を探す|
|[(operator[])](#operator[])|要素アクセス．setならorder_of(i64)でmapで非存在ならデフォルト構築|
|[contains](#contains)|指定したキーを持つ要素があるかどうか調べる|
|[lower_bound](#lower_bound)|指定したキー以上の最小のキーを持つ要素を返す|
|[upper_bound](#upper_bound)|指定したキーより大きい最小のキーを持つ要素を返す|
|[find_by_order](#find_by_order)|順序を指定して要素にアクセスする|
|[order_of](#order_of)|指定したキーまたはイテレータの順序を返す|


## <a id="constructor">コンストラクタ</a>
```C++
rbtree();               // 空のクラスを作成
rbtree(rbtree&&);       // move
rbtree(const rbtree&);  // copy
rbtree(initializer_list<key_t>);       // set
rbtree(initializer_list<std::pair<key_t,val_t>>); // map
explicit rbtree(const Cmp&);           // 比較関数を指定
explicit rbtree(const Allocator&);     // アロケータを指定
explicit rbtree(const Cmp&, const Allocator&); // 比較関数とアロケータを指定
```
move や copy では [operator=](#operator=) を内部的に用いる．

## <a id="destructor">デストラクタ</a>
```C++
~rbtree();
```
内部的に [clear](#clear) を呼び出す．nilは解放しないため，clear後も安全に使用可能．
計算量は $O(N)$.

## <a id="operator=">代入演算子</a>
```C++
rbtree&operator=(rbtree&&);      // move
rbtree&operator=(const rbtree&); // copy
```
move は元のクラスの中身を移し替えるが，元々持っていた要素はすべて解法するため計算量は $O(N)$.  
copy も当然 $O(N)$.

## <a id="iterator">iterator</a>
`iterator` は以下のメンバ関数を持つ `rbtree` のインナークラスである．
```C++
iterator&operator=(const iterator&);   // コピー
bool operator==(const iterator&)const; // 参照するノードが同じときtrue
auto operator<=>(const iterator&)const; // 順序比較（同じ木に属する必要がある）
iterator&operator++();         // ++itr
iterator&operator--();         // --itr
iterator operator++(int32_t);  // itr++
iterator operator--(int32_t);  // itr--
iterator&operator+=(difference_type n); // ランダムアクセス（O(lg N)）
iterator&operator-=(difference_type n); // ランダムアクセス（O(lg N)）
iterator operator+(difference_type n)const; // ランダムアクセス（O(lg N)）
iterator operator-(difference_type n)const; // ランダムアクセス（O(lg N)）
difference_type operator-(const iterator&)const; // イテレータ間の距離（O(lg N)）
auto&operator[](difference_type n); // ランダムアクセス（O(lg N)）
const key_t&operator*();       // set, multiset
value_type&operator*();        // map, multimap
auto operator->();             // ポインタアクセス
auto operator->()const;        // ポインタアクセス（const版）
```
内部的には `node` へのポインタと所属する木へのポインタを持っている．  
`operator++` や `operator--` で全部移動すると移動量は $2N$ 程度になるため $O(N)$.  
`operator++` や `operator--` の1回あたりの最大移動量は $2\lg N$ 程度だが，償却で $O(1)$.  
`operator+=`, `operator-=`, `operator+`, `operator-`, `operator[]` は内部的に `find_by_order` と `order_of` を用いるため，計算量は $O(\lg N)$.

`const_iterator`で書き換え禁止版もある．

イテレータは循環するため，`++nil`なら最小値に移動し，`--nil`なら最大値に移動する．

## <a id="reverse_iterator">reverse_iterator</a>
`reverse_iterator` は [`iterator`](#iterator) と同じクラスの逆向き版．
`const_reverse_iterator`で書き換え禁止版もある．

## <a id="begin">begin</a>
```C++
iterator begin();
const_iterator begin()const;
```
$O(\lg N)$ で最小要素の[イテレータ](#iterator)を返す．

## <a id="cbegin">begin</a>
```C++
const_iterator cbegin()const;
```
$O(\lg N)$ で最小要素の[イテレータ](#iterator)を返す．

## <a id="end">end</a>
```C++
iterator end();
const_iterator end()const;
```
$O(1)$ でnil番兵の[イテレータ](#iterator)を返す．

## <a id="cend">end</a>
```C++
const_iterator cend()const;
```
$O(1)$ でnil番兵の[イテレータ](#iterator)を返す．

## <a id="rbegin">rbegin</a>
```C++
reverse_iterator rbegin();
const_reverse_iterator rbegin()const;
```
$O(\lg N)$ で最大要素の[逆イテレータ](#reverse_iterator)を返す．

## <a id="crbegin">rbegin</a>
```C++
const_reverse_iterator crbegin()const;
```
$O(\lg N)$ で最大要素の[逆イテレータ](#reverse_iterator)を返す．

## <a id="rend">rend</a>
```C++
reverse_iterator rend();
const_reverse_iterator rend()const;
```
$O(1)$ でnil番兵の[逆イテレータ](#reverse_iterator)を返す．

## <a id="crend">rend</a>
```C++
const_reverse_iterator crend()const;
```
$O(1)$ でnil番兵の[逆イテレータ](#reverse_iterator)を返す．

## <a id="empty">empty</a>
```C++
bool empty()const;
```
木が要素を持たないときに `true` を返す．  
$O(1)$.

## <a id="size">size</a>
```C++
size_t size()const;
```
木が持つ要素数を返す．  
$O(1)$.

## <a id="clear">clear</a>
```C++
void clear();
```
木の要素をすべて `free` する．
$O(N)$.  
clear後のコンテナは再利用可能．

## <a id="insert">insert</a>
```C++
template<class... Args>
requires(std::is_constructible_v<value_type, Args...>)
iterator insert(Args&&... args);
```
$O(\lg N)$ で要素を挿入する．  
`std::map` の `insert` と異なり，`key_t`, `val_t` を分けて与えることができる．  
組込み型なら `emplace` に近い感じ．  
しかしこの `rbtree` に `emplace` は無い．  
見ての通り，構築するため．  
`is_multi` が `false` の場合，既に存在するキーを挿入しようとすると，新しい値で上書きされる（`map` の場合）．

## <a id="erase">erase</a>
```C++
iterator erase(iterator&);
iterator erase(iterator&&);
reverse_iterator erase(reverse_iterator&);
reverse_iterator erase(reverse_iterator&&);

template<class... Args>
requires(std::constructible_from<key_t, Args...>)
iterator erase(Args&&... args);
```
イテレータを渡してピンポイントで消すか，キー(を構築できる情報)を指定してその最も古い一つを消すかできる．
キーを渡すとき，`multiset`, `multimap` のときは最も古いものを消すが，`set`, `map` のときはただ一つの要素を消す．  
存在しないキーを与えたときは何もせず，nilへのイテレータを返す．  
要素を削除したときは，削除した要素の次の要素へのイテレータを返す．  
消した要素が最大のときはnilへのイテレータを返す．  
[`reverse_iterator`](#reverse_iterator)を与えたときは，`reverse_iterator`基準で次が返るので，一つ小さい要素を得る．

## <a id="count">count</a>
```C++
template<class... Args>
requires(std::constructible_from<key_t, Args...>)
size_t count(Args&&... args)const;
```
キーの数を数える．  
`set`, `map`では`contains`と同じ．
`multimap`, `multiset` では，内部的に [`order_of`](#order_of)を用いる．  
いずれも計算量は $O(\lg N)$.

## <a id="find">find</a>
```C++
template<class... Args>
requires(std::constructible_from<key_t, Args...>)
iterator find(Args&&... args);

template<class... Args>
requires(std::constructible_from<key_t, Args...>)
const_iterator find(Args&&... args)const;
```
キーを探索する．ただし，`multiset`, `multimap`では最も古い要素を返す．  
計算量は $O(\lg N)$.

## <a id="operator[]">operator[]</a>
```C++
template<class... Args>
requires (!is_set) && (std::constructible_from<key_t, Args...>)
val_t& operator[](Args&&... args);

const key_t& operator[](size_t idx)const requires (is_set);
```
`elsie::set`で使う場合，つまり`rbtree`の`val_t`が`elsie::null_t`でない場合は`find_by_order`．
`size()`以上を指定した場合はUB．負の値ならもちろん逆順になる．これも範囲外ならUB．  
`elsie::map`で使う場合はキーと一致する値を返す．
この場合存在しない要素を指定したらデフォルト構築．
デフォルト構築できない場合は`throw std::invalid_argument`．

## <a id="contains">contains</a>
```C++
template<class... Args>
requires(std::constructible_from<key_t, Args...>)
bool contains(Args&&... args)const;
```
木がキーを持っていれば `true` を返す．  
$O(\lg N)$.

## <a id="lower_bound">lower_bound</a>
```C++
template<class... Args>
requires(std::constructible_from<key_t, Args...>)
iterator lower_bound(Args&&... args);

template<class... Args>
requires(std::constructible_from<key_t, Args...>)
const_iterator lower_bound(Args&&... args)const;
```
キー以上の最小の要素を返す．  
$O(\lg N)$.

## <a id="upper_bound">upper_bound</a>
```C++
template<class... Args>
requires(std::constructible_from<key_t, Args...>)
iterator upper_bound(Args&&... args);

template<class... Args>
requires(std::constructible_from<key_t, Args...>)
const_iterator upper_bound(Args&&... args)const;
```
キーより大きいの最小の要素を返す．  
$O(\lg N)$.

## <a id="find_by_order">find_by_order</a>
```C++
iterator find_by_order(int64_t);
const_iterator find_by_order(int64_t)const;
```
0-indexedな昇順乱択ができる．  
負の数を入れると後ろから1-indexedなアクセスができる．  
つまりｐｙてょｎ．
計算量はもちろん $O(\lg N)$.  
範囲外を参照したらnilが返る．

## <a id="order_of">order_of</a>
```C++
template<class... Args>
requires(std::constructible_from<key_t, Args...>)
size_t order_of(Args&&... args)const;

template<bool is_const, bool is_reverse>
size_t order_of(const iter<is_const, is_reverse>&)const;
```
0-indexedな昇順の番号を返す．  
キーを渡した場合，キーが存在しないときはキー以上の最も小さい要素の番号を返す（`lower_bound` と同じ動作）．  
`lower_bound` が`nil`なら返ってくるのは`size()`と同じ．  
イテレータを渡した場合，そのイテレータが指す要素の順序を返す．  
逆順イテレータのときに得る値は`rbegin()`が0，`rend()`が`size()`になる逆順インデックス．  
計算量は $O(\lg N)$.

