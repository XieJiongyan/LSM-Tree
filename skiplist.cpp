#pragma once
#include "skiplist.h"

#ifndef _SKIPLIST_CPP_
#define _SKIPLIST_CPP_
template <typename Key, typename Value>
Skiplist<Key, Value>::~Skiplist() {
  for (auto& i : _quadlist)
    delete i;
}

template<typename Key, typename Value>
bool Skiplist<Key, Value>::clear() {
  for (auto& i : _quadlist)
    delete i;
  _quadlist.clear();
  _size = sizeof(_size) + sizeof(_amount) + sizeof(_bottomsize);
  _amount = 0;
  _bottomsize = 0;
  return true;
}
template<typename Key, typename Value>
bool Skiplist<Key, Value>::exist(const Key& fkey) const{
  SkiplistEntry<Key, Value>* left(nullptr), * right(nullptr);
  for (int ifloor(_quadlist.size() - 1); ifloor >= 0; ifloor--) {
    if (left != nullptr) left = left->_pdown;
    if (right != nullptr) right = right->_pdown;
    for (auto ithis(left == nullptr ? _quadlist[ifloor] : left); ithis != right; ithis = ithis->_pright) {
      if (ithis->key == fkey) {
        return true;
      }
      else if (ithis->key < fkey)
        left = ithis;
      else {
        right = ithis;
        break;
      }
    }
  }
  return false;
}

template<typename Key, typename Value>
const Value& Skiplist<Key, Value>::get(const Key& fkey) const{
  SkiplistEntry<Key, Value>* left(nullptr), * right(nullptr);
  for (int ifloor(_quadlist.size() - 1); ifloor >= 0; ifloor--) {
    if (left != nullptr) left = left->_pdown;
    if (right != nullptr) right = right->_pdown;
    for (auto ithis(left == nullptr ? _quadlist[ifloor] : left); ithis != right; ithis = ithis->_pright) {
      if (ithis->key == fkey) return ithis->value;
      else if (ithis->key < fkey)
        left = ithis;
      else {
        right = ithis;
        break;
      }
    }
  }
  return Value();
}

template<typename Key, typename Value>
bool Skiplist<Key, Value>::put(const Key& fkey, const Value& fvalue) {
  SkiplistEntry<Key, Value>* left(nullptr), * right(nullptr);
  for (int ifloor(_quadlist.size() - 1); ifloor >= 0; ifloor--) {
    if (left != nullptr) left = left->_pdown;
    if (right != nullptr) right = right->_pdown;
    for (auto ithis(left == nullptr ? _quadlist[ifloor] : left); ithis != right; ithis = ithis->_pright) {
      if (ithis->key == fkey) {
        for (; ithis != nullptr; ithis = ithis->_pdown)
          ithis->value = fvalue;
        return true;
      }
      else if (ithis->key < fkey)
        left = ithis;
      else {
        right = ithis;
        break;
      }
    }
  }
  SkiplistEntry<Key, Value>* pdeal;
  if (left == nullptr) {
    pushfront(fkey, fvalue, 0);
    pdeal = _quadlist[0];
  }
  else {
    insertback(left, fkey, fvalue);
    pdeal = left->_pright;
  }
  _amount++;
  auto thsize{ sizeof(*pdeal) };
  _size += thsize;
  _bottomsize += thsize;
  static std::random_device rd;  //Will be used to obtain a seed for the random number engine
  static std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  static std::uniform_real_distribution<> dis(0.0, 1.0);
  for (auto i(0); dis(gen) < 0.5; i++) {
    _size += thsize;
    update(pdeal, i);
    pdeal = pdeal->_pup;
  }
  return true;
}

template <typename Key, typename Value>
bool Skiplist<Key, Value>::erase(const Key& fkey) {
  SkiplistEntry<Key, Value>* left(nullptr), * right(nullptr);
  for (int ifloor(_quadlist.size() - 1); ifloor >= 0; ifloor--) {
    if (left != nullptr) left = left->_pdown;
    if (right != nullptr) right = right->_pdown;
    for (auto ithis(left == nullptr ? _quadlist[ifloor] : left); ithis != right; ithis = ithis->_pright) {
      if (ithis->key == fkey) {
        _amount--;
        _size -= (ifloor + 1) * sizeof(*ithis);
        _bottomsize -= sizeof(*ithis);
        for (; ifloor >= 0; ifloor--) {
          ithis->_pup = nullptr;
          if (ithis->_pleft != nullptr)
            ithis->_pleft->_pright = ithis->_pright;
          else if (ithis->_pright != nullptr) _quadlist[ifloor] = ithis->_pright;
          else if (ifloor == _quadlist.size() - 1) _quadlist.pop_back();
          else throw "skiplist erase error";
          if (ithis->_pright != nullptr)
            ithis->_pright->_pleft = ithis->_pleft;
          ithis->_pleft = nullptr;
          ithis->_pright = nullptr;
          auto ithisc(ithis);
          ithis = ithis->_pdown;
          delete ithisc;
        }
        return true;
      }
      else if (ithis->key < fkey)
        left = ithis;
      else {
        right = ithis;
        break;
      }
    }
  }
  return false;
}

template<typename Key, typename Value>
void SkiplistEntry<Key, Value>::insertback(Key fkey, Value fvalue) {
  auto prightc(_pright);
  _pright = new SkiplistEntry<Key, Value>(fkey, fvalue);
  if (prightc != nullptr) _pright->_pright = prightc;
  _pright->_pleft = this;
}

template<typename Key, typename Value>
bool Skiplist<Key, Value>::insertback(SkiplistEntry<Key, Value>*& fpentry, Key fkey, Value fvalue) {
  auto pc(fpentry->_pright);
  fpentry->_pright = new SkiplistEntry<Key, Value>(fkey, fvalue);
  fpentry->_pright->_pright = pc;
  fpentry->_pright->_pleft = fpentry;
  if (pc != nullptr)
    pc->_pleft = fpentry->_pright;
  return true;
}
template<typename Key, typename Value>
bool Skiplist<Key, Value>::update(SkiplistEntry<Key, Value>* fpentry, unsigned floor) {
  auto ckey(fpentry->key);
  auto cvalue(fpentry->value);
  auto cpentry(fpentry);
  for (; cpentry->_pleft != nullptr && cpentry->_pup == nullptr; cpentry = cpentry->_pleft) continue;
  if (cpentry->_pup == nullptr) {
    pushfront(ckey, cvalue, floor + 1);
    _quadlist[floor + 1]->_pdown = fpentry;
    fpentry->_pup = _quadlist[floor + 1];
  }
  else {
    insertback(cpentry->_pup, ckey, cvalue);
    cpentry->_pup->_pright->_pdown = fpentry;
    fpentry->_pup = cpentry->_pup->_pright;
  }
  return true;
}

template<typename Key, typename Value>
bool Skiplist<Key, Value>::pushfront(Key fkey, Value fvalue, unsigned floor) {
  if (floor >= _quadlist.size()) _quadlist.resize(floor + 1);
  auto prightc(_quadlist[floor]);
  _quadlist[floor] = new SkiplistEntry<Key, Value>(fkey, fvalue);
  _quadlist[floor]->_pright = prightc;
  if (prightc != nullptr)
    prightc->_pleft = _quadlist[floor];
  return true;
}
#endif