#include "lsmtree.h"

bool Lsmtree::del(ulong fkey) {
  bool rev{ get(fkey) != "" };
  put(fkey, "");
  return rev;
}
std::string Lsmtree::get(ulong fkey) const {
  if (_memtable.skiplist.exist(fkey))
    return _memtable.skiplist.get(fkey);
  else {
    for (auto& i : _logtable) {
      if (!i.vkey.empty() && i.maxkey >= fkey && i.minkey <= fkey) {
        auto iout(std::find(i.vkey.begin(), i.vkey.end(), fkey) - i.vkey.begin());
        if (iout == i.vkey.size())
          continue;
        int ifile(&i - &_logtable[0]);
        std::string filename("out" + std::to_string(ifile) + ".hex");
        std::ifstream fin(filename, std::ios::in | std::ios::binary);
        unsigned lastoffset((iout == 0 ? 0 : i.voffset[iout - 1]) + sizeof(ulong));
        fin.seekg(lastoffset);
        //std::cout << fin.tellg() << std::endl;
        char* buffer = new char[i.voffset[iout] - lastoffset + 1]{};
        fin.read(buffer, i.voffset[iout] - lastoffset);
        str restr(buffer);
        delete[] buffer;
        fin.close();
        //std::cout << fin.tellg() << std::endl;
        return restr;
      }
    }
  }
  return "";
}
bool Lsmtree::put(ulong fkey, str flong) {
  directly_put(fkey, flong);
  if (_memtable.skiplist_size >= _maxsize) {
    for (auto i(0); i < 2; i++)
      if (_logtable[i].vkey.empty()) {
        write(i);
        _memtable.skiplist.clear();
        _memtable.skiplist_size = 0;
        return true;//如果有空位，直接放。
      }
    //如果第一层已经满了，注意这里第一层只有两个文件
    for (auto i(0); i < 2; i++) {
      inread(i);
    }
    for (auto ifloor(1); _memtable.skiplist.amount(); ifloor++) {//从第二层开始，直到后来
      if (_logtable.size() < (2 << ifloor))
        _logtable.resize(2 << ifloor);
      int st{ 1 << ifloor };//用于记录起始插入位置
      int lt{ 2 << ifloor }; // 用于把之后的log移到后面
      //读取这一层和_memtable有交集的所有数据，全部插入_
      for (auto ifile(1 << ifloor); ifile < 2 << ifloor; ifile++) {
        if (!_logtable[ifile].vkey.empty() && \
            (_logtable[ifile].minkey <= _memtable.maxkey || \
             _logtable[ifile].maxkey >= _memtable.minkey)) {
          inread(ifile);
        }
      }
      //左推
      for (auto ifile(1 << ifloor); ifile < 2 << ifloor; ifile++) {
        if (!_logtable[ifile].vkey.empty() && _logtable[ifile].maxkey < _memtable.minkey) {
          if (ifile != st) {
            _logtable[st].vkey = _logtable[ifile].vkey;
            _logtable[st].voffset = _logtable[ifile].voffset;
            _logtable[st].minkey = _logtable[ifile].minkey;
            _logtable[st].maxkey = _logtable[ifile].maxkey;
          }
          st++;
        }
        else if (!_logtable[ifile].vkey.empty())
          break;
      }//右推
      for (auto ifile((2 << ifloor) - 1); ifile >= 1 << ifloor; ifile--) {
        if (!_logtable[ifile].vkey.empty() && _logtable[ifile].minkey > _memtable.maxkey) {
          lt--;
          if (ifile != lt - 1) {
            _logtable[lt].vkey = _logtable[ifile].vkey;
            _logtable[lt].voffset = _logtable[ifile].voffset;
            _logtable[lt].minkey = _logtable[ifile].minkey;
            _logtable[lt].maxkey = _logtable[ifile].maxkey;
          }
        }
        else if (!_logtable[ifile].vkey.empty())
          break;
      }
      //插入
      for (; st < lt && _memtable.skiplist.amount(); st++) {
        std::string filename("out" + std::to_string(st) + ".hex");
        std::ofstream fout(filename, std::ios::out | std::ios::binary);
        int il(0), tl(_memtable.skiplist.amount());
        //       if (_logtable.size() <= ifile)
          //       _logtable.resize(ifile + 1);
        //       int totalsize(0);//记录现在的总共大小
        _logtable[st].vkey.resize(tl);//dotnts
        _logtable[st].voffset.resize(tl);
        //for (auto i(_memtable.skiplist.begin()); i != _memtable.skiplist.end(); i++, il++) {
        while (_memtable.skiplist.amount() && il < tl) {
          auto first_entry(_memtable.skiplist.begin());
          fout.write((char*)&(first_entry->key), sizeof(ulong));
          _logtable[st].vkey[il] = (first_entry->key);
          char* buffer = const_cast<char*>((first_entry->value).c_str());
          fout.write(buffer, strlen(buffer));
          _logtable[st].voffset[il] = static_cast<int>(fout.tellp());
          il++;
          _memtable.skiplist.erase(first_entry->key);
          if (_logtable[st].voffset[il - 1] >= Lsmtree::_maxsize)
            break;
        }
        _logtable[st].vkey.resize(il);//dotnts
        _logtable[st].voffset.resize(il);
        _logtable[st].minkey = _logtable[st].vkey[0];
        _logtable[st].maxkey = _logtable[st].vkey[il - 1];
        fout.close();
      }
    }
  }
  return true;
}
bool Lsmtree::write(unsigned ifile) {
  std::string filename("out" + std::to_string(ifile) + ".hex");
  std::ofstream fout(filename, std::ios::out | std::ios::binary);
  int il(0);
  if (_logtable.size() <= ifile)
    _logtable.resize(ifile + 1);
  _logtable[ifile].vkey.resize(_memtable.skiplist.amount());
  _logtable[ifile].voffset.resize(_memtable.skiplist.amount());
  for (auto i(_memtable.skiplist.begin()); i != _memtable.skiplist.end(); i++, il++) {
    fout.write((char*)&(i->key), sizeof(ulong));
    int coff(fout.tellp());
    _logtable[ifile].vkey[il] = (i->key);
    char* buffer = const_cast<char*>((i->value).c_str());
    fout.write(buffer, strlen(buffer));
    _logtable[ifile].voffset[il] = static_cast<int>(fout.tellp());
  }
  _logtable[ifile].minkey = _logtable[ifile].vkey[0];
  _logtable[ifile].maxkey = _logtable[ifile].vkey[il - 1];
  fout.close();
  return true;
}
bool Lsmtree::read() {
  std::ifstream fin("out0001.hex", std::ios::in | std::ios::binary);
  unsigned lastoffset{ 0 };
  for (unsigned i(0); i < _logtable[0].vkey.size(); i++) {
    uint64_t cl{};
    char* buffer = new char[_logtable[0].voffset[i] - lastoffset - sizeof(ulong) + 1]{};
    fin.read((char*)&cl, sizeof(ulong));
    fin.read(buffer, _logtable[0].voffset[i] - lastoffset - sizeof(ulong));
    lastoffset = _logtable[0].voffset[i];
    _contable.skiplist.put(cl, buffer);
    delete[] buffer;
  }
  fin.close();
  return true;
}
bool Lsmtree::read(unsigned ifile) {
  std::string filename("out" + std::to_string(ifile) + ".hex");
  std::ifstream fin(filename, std::ios::in | std::ios::binary);
  unsigned lastoffset{ 0 };
  for (unsigned i(0); i < _logtable[ifile].vkey.size(); i++) {
    uint64_t cl{};
    char* buffer = new char[_logtable[ifile].voffset[i] - lastoffset - sizeof(ulong) + 1]{};
    fin.read((char*)&cl, sizeof(ulong));
    fin.read(buffer, _logtable[ifile].voffset[i] - lastoffset - sizeof(ulong));
    lastoffset = _logtable[ifile].voffset[i];
    _contable.skiplist.put(cl, buffer);
    delete[] buffer;
  }
  fin.close();
  return true;
}
bool Lsmtree::inread(const unsigned ifile) {
  std::string filename("out" + std::to_string(ifile) + ".hex");
  std::ifstream fin(filename, std::ios::in | std::ios::binary);
  unsigned lastoffset{ 0 };
  for (unsigned i(0); i < _logtable[ifile].vkey.size(); lastoffset = _logtable[ifile].voffset[i++]) {
    if (_memtable.skiplist.exist(_logtable[ifile].vkey[i])) {
      fin.seekg(_logtable[ifile].voffset[i]);
      continue;
    }
    uint64_t cl{};
    char* buffer = new char[_logtable[ifile].voffset[i] - lastoffset - sizeof(ulong) + 1]{};
    fin.read((char*)&cl, sizeof(ulong));
    fin.read(buffer, _logtable[ifile].voffset[i] - lastoffset - sizeof(ulong));
    directly_put(cl, buffer);
    delete[] buffer;
  }
  fin.close();
  _logtable[ifile].vkey.clear();
  _logtable[ifile].voffset.clear();
  _logtable[ifile].minkey = 0;
  _logtable[ifile].maxkey = 0;
  int a(remove(filename.c_str()));
  return true;
}
bool Lsmtree::write() {
  std::ofstream fout("out0001.hex", std::ios::out | std::ios::binary);
  int il(0);
  _logtable[0].vkey.resize(_memtable.skiplist.amount());
  _logtable[0].voffset.resize(_memtable.skiplist.amount());
  for (auto i(_memtable.skiplist.begin()); i != _memtable.skiplist.end(); i++, il++) {
    fout.write((char*)&(i->key), sizeof(ulong));
    int coff(fout.tellp());
    _logtable[0].vkey[il] = (i->key);
    char* buffer = const_cast<char*>((i->value).c_str());
    fout.write(buffer, strlen(buffer));
    _logtable[0].voffset[il] = static_cast<int>(fout.tellp());
  }
  _logtable[0].minkey = _logtable[0].vkey[0];
  _logtable[0].maxkey = _logtable[0].vkey[il - 1];
  fout.close();
  return true;
}

bool Lsmtree::directly_put(ulong fkey, str fvalue) {
  if (_memtable.skiplist.put(fkey, fvalue)) {
    _memtable.skiplist_size += sizeof(fkey) + fvalue.size();
    _memtable.minkey = std::min(_memtable.minkey, fkey);
    _memtable.maxkey = std::max(_memtable.maxkey, fkey);
    return true;
  }
  else return false;
}
/*
template<typename Key, typename Value>
Value Lsmtree<Key, Value>::get(Key fkey) {
  for (signed i(0); i < logtable[0]._size; i++) {
    if ((logtable[0].plogstruct + i)->key == fkey) {
      std::ifstream fin("data1_1.dat", std::ios::binary | std::ios::in);
      int extrasize{ 0 };
      if (i + 1 == logtable[0]._size) extrasize = logtable[0]._totaloffset;
      else extrasize = (logtable[0].plogstruct + i + 1)->offset;
      fin.seekg((logtable[0].plogstruct + i)->offset, std::ios::beg);
    }
  }
  return Value();
}

template <typename Key, typename Value>
bool Lsmtree<Key, Value>::put(Key fkey, Value fvalue) {
  bool rev{ _memtable.put(fkey, fvalue) };
  if (_memtable.size() >= _maxsize) {
    logtable.resize(1);
    logtable[0].plogstruct = new logstruct[_memtable.amount()];
    logtable[0]._size = _memtable.amount();
    std::ofstream fout("data1_1.dat", std::ios::binary | std::ios::out);
    auto ilog(logtable[0].plogstruct);
    unsigned totaloffset{ 0 };
    for (auto i(_memtable.begin()); i != _memtable.end(); i++) {
      fout.write((char*)&(i->key), sizeof(i->key));
        fout.write((char*)&(i->value), sizeof(i->value));
      ilog->logkey = i->key; ilog->offset = totaloffset;
      totaloffset += sizeof(i->key) + sizeof(i->value);
      ilog++;
    }
    logtable[0]._totaloffset = totaloffset;
  }
  return rev;
}
*/
