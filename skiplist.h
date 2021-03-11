#ifndef _skiplist_h_
#define _skiplist_h_
#include <vector>
#include <iostream>
#include <random>
#include <ctime>

//�û�������õ�skiplist�ڵ�Entry��
template <typename Key, typename Value>
class SkiplistEntry;

/*
 * ����Skiplist��Method:
 * Skiplist<Key, Vaule> skiplist; // ����һ���յ�����������ΪKey��ֵ����ΪValue
 * skiplist.put(key, kalue);      // ������������һ����ֵ��
 * if (skiplist.exist(key))       // ��ѯ����������key�������򷵻ض�Ӧ��value
 *     ... = skiplist.get(key);   // ע�⣬ÿ��getǰ��Ҫ�ж�exist,������ܷ��ش���
 * skiplist.delete(key);          // ��������ɾ��һ����ֵ��
 */
template <typename Key, typename Value>
class Skiplist {
  friend SkiplistEntry<Key, Value>;
public:
  /*
   * Constructor: Skiplist
   * Usage: Skiplist<Key, Value> Skiplist;
   * -------------------------------------
   * Ĭ�Ϲ��캯��
   */
  Skiplist() : _quadlist{ std::vector< SkiplistEntry<Key, Value>* >() }, _size{ sizeof(_size) + sizeof(_amount) + sizeof(_bottomsize) }\
    , _amount{ 0 }, _bottomsize(0) {};

  /*
   * Destructor: ~Skiplist
   * Usage: (usually implicit)
   * -------------------------
   * ��������
   */
  ~Skiplist();

  /*
   * Method: exist(Key)
   * Usage: if(skiplist.exist(Key)) ...
   * ----------------------------------
   * ��ѯ��������û��Key��Ӧ�ļ�ֵ��
   */
  bool exist(const Key&) const;

  /*
   * Method: get(Key)
   * Usage: value = skiplist.get(Key);
   * ---------------------------------
   * ��һ��ʵ�ֵ��ⲿ�ӿں��������Key��Ӧ��Value��ʱ�临�Ӷ�ΪO(logn)
   * ע�������exist����trueʱ���ܵõ���ȷ���
   */
  const Value& get(const Key&) const;

  /*
   * Method: put(Key, Value)
   * Usage: skiplist.put(Key, Value);
   * --------------------------------
   * ������������һ����ֵ��
   */
  bool put(const Key&, const Value&);

  /*
   * Method: erase(Key)
   * Usage: skiplist.erase(Key);
   * ---------------------------
   * ��������ɾ��һ����ֵ�ԣ����ɾ���ɹ�������true�����򷵻�false
   */
  bool erase(const Key&);

  /*
   * �����ڵ�iterator�ࡣ֧������++,�ж����==,!=,���ƹ��캯����������->
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
   * ��������skiplist��ռ�Ĵ�С
   */
  size_t size() const {
    return _size;
  }
  /*
   * Method: bottomsize()
   * usage: ... = skiplist.bottomsize();
   * -----------------------------------
   * ����skiplist�ײ�Ĵ�С�������൱�ڰ����ݴ洢�������������С
   */
  size_t bottomsize() const {
    return _bottomsize;
  }
  /*
   * Method: clear()
   * Usage: skiplist.clear()
   * -----------------------
   * ��������ڵ�ȫ������
   */
  bool clear();
  unsigned amount() const {
    return _amount;
  }
private:
  std::vector< SkiplistEntry<Key, Value>* > _quadlist;                 //�������������
  bool update(SkiplistEntry<Key, Value>* fpentry, unsigned floor);     //��ĳһEntry������չһ��
  bool pushfront(Key, Value, unsigned floor);                          //�����һ��Ϊ�գ���ô����һ������һ��Ԫ��
  bool insertback(SkiplistEntry<Key, Value>*&, Key, Value);            //�ڵ�һ������ָ���Entry�������һ����Ԫ��
  size_t _size;                                                        //��������Ĵ�С
  size_t _bottomsize;                                                  //����������һ��Ĵ�С
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
   *  1:Ĭ�Ϲ��캯��
   *  2:ʹ��key �� value ��ʼ������
   *  3:���ƹ��캯����ǿ���ƣ�һ������ᵼ�¶��delete�����³���bug
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
   * ��������
   */
  ~SkiplistEntry() {
    //    std::cout << key << ' ' << value << std::endl;
    if (_pright != nullptr) {
      delete _pright;
      //����������仰�������׷���bug
      _pright = nullptr;
    }
  }
private:
  /*
   * Method: insertback
   * Usage: skiplistEntry.insertback(Key, Value);
   * --------------------------------------------
   * �ڱ�entry���洴��һ���µ�skiplistEntry����
   */
  void insertback(Key, Value);
public:
  Key key;                                                           //ÿ���ڵ��key,value
  Value value;
private:
  SkiplistEntry<Key, Value>* _pup;                                    //�ֱ𴢴��������ҵ�
  SkiplistEntry<Key, Value>* _pdown;                                  //�ڵ�ָ��
  SkiplistEntry<Key, Value>* _pleft;
  SkiplistEntry<Key, Value>* _pright;
};
#include "skiplist.cpp"

#endif