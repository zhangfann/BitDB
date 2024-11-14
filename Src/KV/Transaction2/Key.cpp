#include <vector>
#include <tuple>
#include <cstdint>

enum KeyType{
    TxnNext,
    TxnActive,
    TxnSnapshot,
    TxnUpdate,
    Record
};

struct TxnActiveData{
    int id;
};

struct TxnSnapshotData{
    int id;
};

struct TxnUpdateData{
    int id;
    std::vector<uint8_t> key;
};

struct RecordData{
    std::vector<uint8_t> key;
    int version;
};

union KeyData{
    TxnActiveData txn_active_data;
    TxnSnapshotData txn_snapshot_data;
    TxnUpdateData txn_update_data;
    RecordData record_data;

    KeyData(int id): txn_active_data{id} , txn_snapshot_data{id} {}
    KeyData(int id, const std::vector<uint8_t>& key): txn_update_data{id, key}{}
    KeyData(const std::vector<uint8_t>& key, int version): record_data{key, version}{}

    ~KeyData() = default;
};

class Key{
    KeyType type_;
    KeyData data_;

    Key(KeyType type): type_(type) {}
    Key(KeyType type, int id): type_(type), data_(id){}
    Key(KeyType type, int id, const std::vector<uint8_t>& key): type_(type), data_(id, key){}
    Key(KeyType type, const std::vector<uint8_t>& key, int version): 

    
};

std::optional<Key> decode(std::vector<uint8_t>& key);