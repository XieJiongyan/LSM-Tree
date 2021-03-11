# project 1 文档

> 1.利用跳表实现LSM Tree系统的内存存储。  
> 2.在内存存储层次实现基本操作(PUT, GET, DELETE)。  
> 3.实现硬盘的分层存储，在内存超过一定阈值后将数据写入硬盘。   
> 4.实现合并操作。   
> 5.保证程序的正确性（可通过提供的测试程序，但最终测试会增加测试），并进行性能测试和分析。为了大家能够顺利完成作业，建议大家先完成内存部分，并进行简单测试，最后再完成剩余任务。    
> 注意事项：1.请保证可以在不同平台进行编译，不要使用平台相关代码（比如windows.h），不要使用绝对路径。    
> 2.编译时请使用c++17标准。   
> 提交材料：   
> 1.项目源代码（不包括可执行程序），注意，在最终测试时我们会使用不同的测试代码。    
> 2.设计文档，其中包括设计和测试两部分。设计部分，包括对整个系统设计的理解，以及自己实现的独特的地方；测试部分至少应包括前述性能测试和分析。将源代码和设计文档使用zip格式打包压缩后，重命名为“学号-姓名.zip”提交。整个文件大小不应超过5MB。

如作业要求，该文档主要包括两部分：设计文档与测试文档。

# 设计文档
设计文档主要包括两部分内容，跳表Skiplist与日志结构合并树lsmTree(log-structured Merge Tree)。

## 跳表Skiplist
跳表是一种随机性有序存储结构，由William Pugh 在1989年首次提出。具体来说，跳表与平衡二叉树起到了类似的作用。  
本Skiplist实现的主要功能如下：
1. 构造函数与析构函数
2. 增添键值对
3. 判断有无某键值对
4. 读取某键对应的值
5. 删除某键
次要功能：
6. 迭代器
7. 返回占用的总大小以及底层大小
8. 清楚全部数据
9. 得到skiplist的不同键数，即底层节点总数

### 跳表Skiplist的构造函数
```cpp
  /*
   * Constructor: Skiplist
   * Usage: Skiplist<Key, Value> Skiplist;
   * -------------------------------------
   * 默认构造函数
   */
  Skiplist() : _quadlist{ std::vector< SkiplistEntry<Key, Value>* >() }, _size{ sizeof(_size) + sizeof(_amount) + sizeof(_bottomsize) }\
    , _amount{ 0 }, _bottomsize(0) {};
```
Skiplist的构造函数与map很相似：比如
```cpp
Skiplist<int, std::string> skiplist;
```
Skiplist的成员如下：
1. `vector<std::vector< SkiplistEntry<Key, Value>* > _quadlist;                 //储存跳表的数据`
2. `size_t _size;                                                        //整个跳表的大小`
3. `size_t _bottomsize;                                                  //跳表最下面一层的大小`
4. `unsigned _amount;                                                    //跳表节点数量`  
后三者的存储是显然的。而对于`_quadlist`,他是一种四链表结构，`vector`的每个元素指向每一层的第一个节点。  
而对于四链表的每个节点，具有四个指针，分别指向它左侧，右侧，上侧，下侧的节点，如果某一侧没有节点，那么该指针为`nullptr`.
### 跳表Skiplist的put操作，增添一个键值对，或者更新一个键值对
```cpp
template<typename Key, typename Value>
bool Skiplist<Key, Value>::put(const Key& fkey, const Value& fvalue) {
```
put操作首先需要调用查询过程，平均时间复杂度为O(logn),如果查询到，那么将这一列的键全部更新。
```cpp
        for (; ithis != nullptr; ithis = ithis->_pdown)
          ithis->value = fvalue;
```
如果查询不到，那么分两种情况考虑，插入位置在某个节点后或者不在任何节点后，都可以递归地按照$^1/_2$的概率向上扩展。
### 跳表Skiplist的查找操作
```cpp
bool Skiplist<Key, Value>::exist(const Key& fkey) const{
```
由于Skiplist每一列的高度是不一样的，所以我们从高到低依次查询。并且不断减少查询空间
```cpp
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
```
![avatar](fig/fig99.png)  
一个跳表中查询第5个节点的过程（略丑）
### 跳表Skiplist的删除操作
做法比较简单：查询到一个节点，然后删除这一列。

## 日志结构合并树lsmTree(log-structured Merge Tree)
日志结构合并树是一种内存-外存兼顾的数据结构。在本次project里，至少支持以下三种操作
1. 插入
2. 查询
3. 删除

lsmtree 包括内存和外存两部分。  
内存主要包括两部分：memtable和logtable，memtable是内存中的数据存储部分，logtable是外存的记录，存储着节点的key和offset。  
外存主要由SStable构成。SStable顺序存储着key和value
### lsmtree的插入
lsmtree插入时，首先插入到memtable中，如果memtable已经满了，那么就开始以下的过程，最终会将memtable中的所有数据全部存储到外存中。  
首先看sstable第一层是否已满，如果未满那么插入到未满的一个位置中，插入过程结束。  
否则将第一层的所有数据存入memtable中，并且删除logtable中的相应数据，以及删除相应文件中的内容。
接下来，对于每一层，进行如下操作：
1. 将每层中与memtable有交集的部分插入memtable中。
2. 将memtable中的一部分输出到空的sstable中，直到memtable为空或者这一层没有空余的位置  
    1. 如果memtable空了，那么退出循环，插入过程结束
    2. 如果这一层没有空余位置了，那么转到下一层
### lsmtree的查找
lsmtree的查找首先在memtable中查找，如果没找到，则依序遍历logtable，找到包含查找键值的logtable，然后在logtable中二分查找相应的key。如果找到，则查询相应文件，并且读取输出，否则继续遍历其他logtable
### lsmtree的删除
这里lsmtree的删除比较简单，直接在memtable中插入对应键值key的键值对<key-"">就行，因为这里判断是否存在是看get是否是""(空std::string)

# 测试文档
由于本proj skiplist做的比较多，因此先给出对skiplist的测试程序：
```cpp
#include <iostream>
#include "skiplist.h"
using namespace::std;

signed main() {
  {
    Skiplist<int, int> skiplist{};
    skiplist.put(1, 1);
    skiplist.put(2, 2);
    skiplist.put(3, 3);
    cout << skiplist.get(3) << endl;
    skiplist.put(4, 10);
    skiplist.put(5, 8);
    cout << skiplist.get(4) << endl;
    cout << skiplist.get(6) << endl;
    skiplist.erase(5);
    for (auto i(skiplist.begin()); i != skiplist.end(); i++)
      cout << "<" << i->key << ", " << i->value << ">  ";
  }
  {
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
  }
  return 0;
}
```
其次便是通过了正确性和persistent检验。
之后是对lsmtree的检验文档，不过检验不够充分。
### 性能测试
1. 每种操作的平均时间
    1. 测试环境：windows 普通硬盘
    2. 测试参数：每次操作1000次取平均值，重复30次（其实只会在内存中进行）
    3. 测试结果见下
    4. 分析：由于是在内存中完成，所以没有很大的波动，读取相对十分稳定，插入和删除时间较长
![avatar](fig/fig98.png)
1. 每秒put操作次数（其实有些put是远远超过1s的）（具体put内容与上不同）
    1. 测试环境：windows 普通硬盘
    2. 测试参数：put数据是<ulong,100个字符的string>,后者不随时间改变
    3. 测试结果见下
    4. 分析：一旦涉及到合并操作，会使运行时间大幅提升
![avatar](fig/fig97.png)
