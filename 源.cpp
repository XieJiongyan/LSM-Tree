#include <iostream>
//#include "skiplist.h"
#include "lsmtree.h"

using namespace::std;

signed main() {
  /*
  Skiplist<int, int> skiplist{};
  cout << "Skiplist initialized size: " << skiplist.size() << endl;
  skiplist.put(1, 1);
  cout << "size: " << skiplist.size() << endl;
  skiplist.put(2, 2);
  cout << "size: " << skiplist.size() << endl;
  skiplist.put(3, 3);
  cout << skiplist.get(3) << endl;
  skiplist.put(4, 10);
  cout << "size: " << skiplist.size() << endl;
  skiplist.put(5, 8);
  cout << "size: " << skiplist.size() << endl;
  cout << skiplist.get(4) << endl;
  cout << skiplist.get(6) << endl;
  skiplist.erase(5);
  cout << "size: " << skiplist.size() << endl;
  for (auto i(skiplist.begin()); i != skiplist.end(); i++)
    cout << "<" << i->key << ", " << i->value << ">  ";
  skiplist.erase(2);
  cout << "size: " << skiplist.size() << endl;
  cout << endl << skiplist.size() << endl;
  skiplist.clear();
  */
  string s1{ "I_am_not_a_dog!" };
  string s2{ "dell" };
  Lsmtree lsmtree1;
  lsmtree1.directly_put(100500, s1);
  lsmtree1.directly_put(123, s2);
  lsmtree1.write();
  //  lsmtree1.write(2);
  lsmtree1.read();
  //  lsmtree1.read(2);
  //Lsmtree lsmtree;
  //for (auto i(0); i < 100; i++)
  //  lsmtree.put(i, std::to_string(i * 10));
  //for (auto i(0); i < 39; i++) {
  //  lsmtree.read(i);
  //  for (auto i(lsmtree._contable.skiplist.begin()); i != lsmtree._contable.skiplist.end(); i++)
  //    cout << i->key << " " << i->value << " >< ";
  //  cout << endl;
  //  lsmtree._contable.skiplist.clear();
  //}
  //lsmtree.read(7);
  //for (auto i(0); i < 10; i++) {
  //  for (auto i2(i * 10); i2 < i * 10 + 10; i2++) {
  //    cout << lsmtree.get(i2) << " ";
  //  }
  //  cout << endl;
  //}
  //for (auto i(10); i < 20; i++)
  //  cout << (lsmtree.del(i) ? "true" : "false") << endl;
  //for (auto i(0); i < 10; i++) {
  //  for (auto i2(i * 10); i2 < i * 10 + 10; i2++) {
  //    cout << lsmtree.get(i2) << " ";
  //  }
  //  cout << endl;
  //}
  //for (auto i(0); i < 35; i++) {
  //  lsmtree.read(i);
  //  for (auto i(lsmtree._contable.skiplist.begin()); i != lsmtree._contable.skiplist.end(); i++)
  //    cout << i->key << " " << i->value << " >< ";
  //  cout << endl;
  //  lsmtree._contable.skiplist.clear();
  //}
  Lsmtree lsmt;
  for (auto i(0); i < 64; i++) {
    for (auto i2(i << 10); i2 < i + 1 << 10; i2++) {
      lsmt.put(i2, std::string(i2 + 1, 's'));
      cout << (lsmt.get(i2) == std::string(i2 + 1, 's')) ? 1 : 0 << ' ';
    }
    cout << "\n\n\nphase: " << i << "\n\n\n";
  }
  return 0;
}