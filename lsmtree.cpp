#include <iostream>

#include "mem_table.cpp"

Memtable memtable;
class LSMTree {
   public:
    void insert(KeyValue &kv);
};

void LSMTree::insert(KeyValue &kv) {
    memtable.put(kv);
}