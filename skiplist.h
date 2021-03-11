#ifndef _skiplist_h_
#define _skiplist_h_
#include <vector>
#include <iostream>
#include <random>
#include <ctime>

//用户无需调用的skiplist节点Entry类
template <typename Key, typename Value>
class SkiplistEntry;

/*
 * 跳表Skiplist。Method:
 * Skiplist<Key, Vaule> skiplist; // 创建一个空的跳表，键类型为Key，值类型为Value
 * skiplist.put(key, kalue);      // 在跳表中增添一个键值对
 * if (skiplist.exist(key))       // 查询跳表中有无key，若有则返回对应的value
 *     ... = skiplist.get(key);   // 注意，每次get前需要判断exist,否则可能返回错误
 * skiplist.delete(key);          // 在跳表中删除一个键值对
 */
template <typename Key, typename Value>
class Skiplist {
  friend SkiplistEntry<Key, Value>;
public:
  /*
   * Constructor: Skiplist
   * Usage: Skiplist<Key, Value> Skiplist;
   * -------------------------------------
   * 默认构造函数
   */
  Skiplist() : _quadlist{ std::vector< SkiplistEntry<Key, Value>* >() }, _size{ sizeof(_size) + sizeof(_amount) + sizeof(_bottomsize) }\
    , _amount{ 0 }, _bottomsize(0) {};

  /*
   * Destructor: ~Skiplist
   * Usage: (usually implicit)
   * -------------------------
   * 析构函数
   */
  ~Skiplist();

  /*
   * Method: exist(Key)
   * Usage: if(skiplist.exist(Key)) ...
   * ----------------------------------
   * 查询跳表里有没有Key对应的键值对
   */
  bool exist(const Key&) const;

  /*
   * Method: get(Key)
   * Usage: value = skiplist.get(Key);
   * ---------------------------------
   * 第一个实现的外部接口函数，获得Key对应的Value，时间复杂度为O(logn)
   * 注意必须在exist返回true时才能得到正确结果
   */
  const Value& get(const Key&) const;

  /*
   * Method: put(Key, Value)
   * Usage: skiplist.put(Key, Value);
   * --------------------------------
   * 在跳表中增添一个键值对
   */
  bool put(const Key&, const Value&);

  /*
   * Method: erase(Key)
   * Usage: skiplist.erase(Key);
   * ---------------------------
   * 在跳表中删除一个键值对，如果删除成功，返回true，否则返回false
   */
  bool erase(const Key&);

  /*
   * 容器内的iterator类。支持自增++,判断相等==,!=,复制构造函数，解引用->
   * ----------------------------------------------------------------
   * Method: for (auto i(skiplist.begin(); i != skiplist.end(); i++)
   *             function(i->key , i->value);
   * ----------------------------------------------------------------
   */
  struct Iterator {
    friend Skiplist<Key, Value>;
    Iterator() : pt{ nullptr } {
    }
    Iterator(const Skiplist<Key, Value>::Iterator& it) : pt(it.pt) {
    }
    Iterator& operator=(const Skiplist<Key, Value>::Iterator& it) {
      pt = it.pt;
      return *this;
    }
    Skiplist<Key, Value>::Iterator& operator++() {
      if (pt != nullptr)
        pt = pt->_pright;
      return *this;
    }
    Skiplist<Key, Value>::Iterator operator++(int) {
      auto rev{ *this };
      if (pt != nullptr)
        pt = pt->_pright;
      return rev;
    }

    bool operator==(const Skiplist<Key, Value>::Iterator& ptc) const {
      if (pt != nullptr && ptc != nullptr)
        return pt->key == ptc.pt->key && pt->value == ptc.pt->value;
      else return pt == nullptr && ptc.pt == nullptr;
    }
    bool operator!=(const Skiplist<Key, Value>::Iterator& it) const {
      if (pt != nullptr && it.pt != nullptr) return pt->key != it.pt->key || pt->value != it.pt->value;
      else return pt != it.pt;
    }

    SkiplistEntry<Key, Value>* operator->() {
      return pt;
    }
  private:
    SkiplistEntry<Key, Value>* pt;
  };
  Skiplist<Key, Value>::Iterator begin() const {
    Skiplist<Key, Value>::Iterator rev;
    if (!_quadlist.empty())
      rev.pt = _quadlist[0];
    return rev;
  }
  Iterator end() const {
    return Skiplist<Key, Value>::Iterator();
  }
  /*
   * Method: size()
   * usage: ... = skiplist.size();
   * -----------------------------
   * 返回整个skiplist所占的大小
   */
  size_t size() const {
    return _size;
  }
  /*
   * Method: bottomsize()
   * usage: ... = skiplist.bottomsize();
   * -----------------------------------
   * 返回skiplist底层的大小，大致相当于把数据存储到数组后的数组大小
   */
  size_t bottomsize() const {
    return _bottomsize;
  }
  /*
   * Method: clear()
   * Usage: skiplist.clear()
   * -----------------------
   * 清除跳表内的全部数据
   */
  bool clear();
  unsigned amount() const {
    return _amount;
  }
private:
  std::vector< SkiplistEntry<Key, Value>* > _quadlist;                 //储存跳表的数据
  bool update(SkiplistEntry<Key, Value>* fpentry, unsigned floor);     //将某一Entry向上扩展一层
  bool pushfront(Key, Value, unsigned floor);                          //如果这一层为空，那么在这一层增添一个元素
  bool insertback(SkiplistEntry<Key, Value>*&, Key, Value);            //在第一个参数指向的Entry后面插入一个新元素
  size_t _size;                                                        //整个跳表的大小
  size_t _bottomsize;                                                  //跳表最下面一层的大小
  unsigned _amount;
};

template <typename Key, typename Value>
class SkiplistEntry {
  friend Skiplist<Key, Value>;
protected:
  /*
   * Constructor: SkiplistEntry
   * Usage: SkiplistEntry<Key, Value> skiplistEntry;
   *        SkiplistEntry<Key, Value> skiplistEntry(Key, Value);
   *        SkiplistEntry<Key, Value> skiplistEntry(const skiplistEntry<Key, Value>&);
   * -----------------------------------------------
   *  1:默认构造函数
   *  2:使用key 和 value 初始化对象
   *  3:复制构造函数，强复制，一般情况会导致多次delete，导致出现bug
   */
  SkiplistEntry() :key(Key()), value(Value())\
    , _pup(nullptr), _pdown(nullptr), _pleft(nullptr), _pright(nullptr) {}
  SkiplistEntry(Key pkey, Value pvalue) : key(pkey), value(pvalue)\
    , _pup(nullptr), _pdown(nullptr), _pleft(nullptr), _pright(nullptr) {}
  SkiplistEntry<Key, Value>(const SkiplistEntry<Key, Value>&) = default;
  /*
   * Destructor: ~SkiplistEntry
   * Usage: (usually implicit)
   * ==========================
   * 析构函数
   */
  ~SkiplistEntry() {
    //    std::cout << key << ' ' << value << std::endl;
    if (_pright != nullptr) {
      delete _pright;
      //不加下面这句话，更容易发现bug
      _pright = nullptr;
    }
  }
private:
  /*
   * Method: insertback
   * Usage: skiplistEntry.insertback(Key, Value);
   * --------------------------------------------
   * 在本entry右面创建一个新的skiplistEntry对象
   */
  void insertback(Key, Value);
public:
  Key key;                                                           //每个节点的key,value
  Value value;
private:
  SkiplistEntry<Key, Value>* _pup;                                    //分别储存上下左右的
  SkiplistEntry<Key, Value>* _pdown;                                  //节点指针
  SkiplistEntry<Key, Value>* _pleft;
  SkiplistEntry<Key, Value>* _pright;
};
#include "skiplist.cpp"

#endif