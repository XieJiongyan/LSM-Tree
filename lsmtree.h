#ifndef _lsmtree_h_
#define _lsmtree_h_
#include "skiplist.h"
#include <vector>
#include <string>
#include <fstream>
#include <tuple>

#define _CRT_SECURE_NO_WARNINGS

class Lsmtree {
  using ulong = uint64_t;
  using str = std::string;
public:
  /*
   * Method: Constructor
   * Usage: Lsmtable<Key, Value> lsmtable;
   * -------------------------------------
   * ���캯��
   */
  Lsmtree() :_memtable(Lsmtree::Lsmlist()) {
    _logtable.resize(2);
  }
  /*
   * Method: put(Key, Value)
   * Usage: lsmtable.put(key, value);
   * --------------------------------
   * ��lsmtree������һ����ֵ�ԣ�������Ϻ�
   * ���memtable��С��������С����ô�����ݴ����ļ���.
   * ���Ѿ����ڼ�ֵ�ԣ���ô����false(��δ���в��룩
   * ���򣬷���true
   */
  bool put(ulong, str);
  /*
   * Method: read(ulong)
   * Usage: ... = lsmtree.read(ulong);
   * ---------------------------------
   * ��ȡָ����uint64_t�Ķ�Ӧ��string
   */
  str get(ulong) const;
  /*
   * Method: del(ulong)
   * Usage: lsmtree.del(ulong);
   * --------------------------
   * ɾ��ָ���ļ�ֵ��
   */
  bool del(ulong);
  /*
   * Method: directly_put(Key, Value);
   * Usage: lsmtable.directly_put(key, value)
   * ----------------------------------------
   * ֱ����_memtable������һ����ֵ�ԣ������������Ӱ��
   */
  bool directly_put(ulong, str);
  /*
   * Method: write();
   *         write(unsigned);
   * Usage: lsmtable.write();
   * Usage: lsmtable.write(ifile);
   * ------------------------
   * ��_memtable�е�����д���ļ���,����֮�ⲻ���������Ӱ�졣
   * ��ʱ�޷�����log
   * write(ifile)�������iflie���ļ��У�ifile < 1000,��0����
   */
  bool write();
  bool write(unsigned);
  /*
   * Method: read();
             read(unsigend);
   * Usage: lsmtable.read();
   * Usage: lsmtable.read(ifile);
   * -----------------------
   * ���ļ��е����ݶ���_contable��
   */
  bool read();
  bool read(unsigned);
  /*
   * Method: inread(unsigned);
   * Usage: lsmtable.inread(ifile);
   * ----------------------------
   * ���ļ�����_memtable��
   */
  bool inread(unsigned);
  struct Lsmlist {
    Skiplist<ulong, str> skiplist;
    size_t skiplist_size;//skiplistת�����ļ���Ĵ�С
    ulong minkey;
    ulong maxkey;
    Lsmtree::Lsmlist() : skiplist(Skiplist<ulong, str>()), skiplist_size(0), \
      minkey{ 0x1111ffff }, maxkey{ 0 }{}
  };
  struct loglist {
    std::vector<ulong> vkey;
    std::vector<unsigned> voffset;
    ulong minkey;
    ulong maxkey;
    Lsmtree::loglist() : minkey(0), maxkey(0) {}
  };
  std::vector<Lsmtree::loglist> _logtable;
  Lsmtree::Lsmlist _memtable; //memory table
  Lsmtree::Lsmlist _contable; // contemprary table
private:
  const size_t _maxsize{ 2000000 };
};
#endif