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
   * 构造函数
   */
  Lsmtree() :_memtable(Lsmtree::Lsmlist()) {
    _logtable.resize(2);
  }
  /*
   * Method: put(Key, Value)
   * Usage: lsmtable.put(key, value);
   * --------------------------------
   * 在lsmtree中增添一个键值对，增添完毕后，
   * 如果memtable大小超过最大大小，那么将数据存入文件中.
   * 若已经存在键值对，那么返回false(即未进行插入）
   * 否则，返回true
   */
  bool put(ulong, str);
  /*
   * Method: read(ulong)
   * Usage: ... = lsmtree.read(ulong);
   * ---------------------------------
   * 读取指定的uint64_t的对应的string
   */
  str get(ulong) const;
  /*
   * Method: del(ulong)
   * Usage: lsmtree.del(ulong);
   * --------------------------
   * 删除指定的键值对
   */
  bool del(ulong);
  /*
   * Method: directly_put(Key, Value);
   * Usage: lsmtable.directly_put(key, value)
   * ----------------------------------------
   * 直接在_memtable中增添一个键值对，不会产生其他影响
   */
  bool directly_put(ulong, str);
  /*
   * Method: write();
   *         write(unsigned);
   * Usage: lsmtable.write();
   * Usage: lsmtable.write(ifile);
   * ------------------------
   * 将_memtable中的数据写入文件中,除此之外不会产生其他影响。
   * 暂时无法生成log
   * write(ifile)输出到第iflie个文件中，ifile < 1000,从0计数
   */
  bool write();
  bool write(unsigned);
  /*
   * Method: read();
             read(unsigend);
   * Usage: lsmtable.read();
   * Usage: lsmtable.read(ifile);
   * -----------------------
   * 将文件中的数据读入_contable中
   */
  bool read();
  bool read(unsigned);
  /*
   * Method: inread(unsigned);
   * Usage: lsmtable.inread(ifile);
   * ----------------------------
   * 将文件读入_memtable中
   */
  bool inread(unsigned);
  struct Lsmlist {
    Skiplist<ulong, str> skiplist;
    size_t skiplist_size;//skiplist转换成文件后的大小
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