
#ifndef LYTHON_SMALLDICT_HEADER
#define LYTHON_SMALLDICT_HEADER

namespace lython {

template <typename Key, typename Value>
class SmallDict {
    public:
    struct _Item {
        _Item(): key(Key()) {}

        _Item(Key const& k, Value const& v): key(k), value(v) {}

        Key const key;
        Value     value;

        _Item& operator=(_Item const& i) {
            Key& mutkey = (Key&)key;
            mutkey      = i.key;
            value       = i.value;
            return *this;
        }
    };

    using Item    = _Item;
    using Storage = std::vector<Item>;

    SmallDict(int buckets = 128): _storage(buckets) {}

    bool get(const Key& name, Value& v) const {
        for (Item const& item: _storage) {
            if (name == item.key) {
                v = item.value;
                return true;
            }
        }
        return false;
    }

    bool insert(const Key& name, const Value& value) {
        for (Item const& item: _storage) {
            if (name == item.key) {
                return false;
            }
        }

        _storage.emplace_back(name, value);
        return true;
    }

    bool upsert(const Key& name, const Value& value) {
        for (Item& item: _storage) {
            if (name == item.key) {
                item.value = value;
                return true;
            }
        }

        _storage.emplace_back(name, value);
        return true;
    }

    void clear() { _storage.clear(); }

    void reserve(int n) { _storage.resize(n); }

    bool has_key(const Key& name) const {
        for (Item const& item: _storage) {
            if (name == item.key) {
                return true;
            }
        }
        return false;
    }

    private:
    int     _size = 0;
    Storage _storage;
};
}  // namespace lython

#endif