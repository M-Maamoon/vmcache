#include <map>
#include <string>

struct KeyValue {
    std::string key;
    std::string value;

    KeyValue(const std::string &key, const std::string &value) : key(key), value(value) {}
};

class Memtable {
public:
    std::string get(const std::string &key);
    void put(KeyValue &kv);
    void remove(const std::string &key);
    bool isFull();

   private:
    std::map<std::string, std::string> mem_table;
    int MAX_SIZE = 1000;
};

std::string Memtable::get(const std::string &key) {
    auto it = mem_table.find(key);
    if (mem_table.find(key) != mem_table.end()) {
        return it->second;
    }
    return "";
}

void Memtable::put(KeyValue &kv) {
    mem_table[kv.key] = kv.value;
}

void Memtable::remove(const std::string &key) {
    mem_table.erase(key);

}

bool Memtable::isFull() {
    return mem_table.size() == MAX_SIZE;
}
