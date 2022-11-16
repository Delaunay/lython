
#ifndef LYTHON_HASHTABLE_HEADER
#define LYTHON_HASHTABLE_HEADER

#include <functional>
#include <sstream>
#include <tuple>
#include <vector>

#include <iostream>

#include "siphash.h"

namespace lython {

const char* SALT = "dW8(2!?GTfDFJ@Le";

template <typename T>
struct Hash {
    static uint64_t hash(T const& k) noexcept {
        uint64_t value = 0;
        siphash(
            (const void*)&k, (size_t)sizeof(T), SALT, (uint8_t*)&value, (size_t)sizeof(uint64_t));
        return value;
    }
};

template <>
struct Hash<std::string> {
    static uint64_t hash(std::string const& k) noexcept {
        uint64_t value = 0;
        siphash(
            (const void*)&k[0], (size_t)k.size(), SALT, (uint8_t*)&value, (size_t)sizeof(uint64_t));

        return value;
    }
};

#define FAST_MOD(i, mod) ((i < mod) * i + (i - mod) * (i > mod))
#define REG_MOD(i, mod)  i % mod
#define P2_MOD(i, mod)   (i & (mod - 1))

inline uint64_t fast_mod(uint64_t i, uint64_t mod) { return FAST_MOD(i, mod); }

inline uint64_t reg_mod(uint64_t i, uint64_t mod) { return REG_MOD(i, mod); }

// mod must be a power of 2
inline uint64_t fast_mod_p2(uint64_t i, uint64_t mod) { return P2_MOD(i, mod); }

inline bool is_power2(uint64_t x) { return (x != 0) && ((x & (x - 1)) == 0); }

// TODO: rewrite this with a Storage for Key-Values
// and a HashTable for Key->Index
// the HasTable will be bigger than the number of values so it is useful
// to make it as small as possible
// but then it is not exactly linear probing

/* Hashtable with Open addressing and linear probing.
 *
 * Linear probing improves lookup times due to locality of references
 *
 * The table is rehashed once we reach a load factor of 0.75,
 * rehashing less or more frequently would impact performance negatively.
 *
 * Everytime the table needs to be rehashed the storage size is doubled
 *
 */
template <typename Key, typename Value, typename H = Hash<Key>>
struct HashTable {
    private:
    struct _Item {
        _Item(): key(Key()), used(false), deleted(true) {}

        _Item(Key const& k, Value const& v): key(k), value(v), used(true), deleted(true) {}

        Key const key;
        Value     value;
        bool      used : 1;
        bool      deleted : 1;

        inline void set_hash(uint64_t) {}

        inline uint64_t hash() const { return H::hash(key); }

        _Item& operator=(_Item const& i) {
            Key& mutkey = (Key&)key;
            mutkey      = i.key;
            value       = i.value;
            used        = true;
            deleted     = false;
            return *this;
        }
    };

    // Save the Hash next to the key-value pair
    struct _ItemCached: public _Item {
        _ItemCached(): _Item() {}

        _ItemCached(Key const& k, Value const& v): _Item(k, v) {}

        mutable uint64_t hash_value = 0;

        inline void set_hash(uint64_t h) { hash_value = h; }

        inline uint64_t hash() const {
            if (hash_value == 0) {
                hash_value = H::hash(_Item::key);
            }
            return hash_value;
        }
    };

    using Item    = _ItemCached;
    using Storage = std::vector<Item>;

    // Makes sure we always use a power of 2 as size
    static int round_size(int x) {
        int size = 1;
        while (size < x) {
            size *= 2;
        }
        return size;
    }

    public:
    HashTable(int buckets = 128): _storage(Storage(round_size(buckets))) {}

    float load_factor() const { return float(used) / float(_storage.size()); }

    bool get(const Key& name, Value& v) const {
        Item const* item = _find(name);

        if (item == nullptr) {
            return false;
        }

        v = item->value;
        return true;
    }

    bool insert(const Key& name, const Value& value) { return track_insert(name, value, false); }

    bool upsert(const Key& name, const Value& value) { return track_insert(name, value, true); }

    bool remove(const Key& name) {
        Item* item = (Item*)_find(name);

        if (item == nullptr) {
            return false;
        }

        item->deleted = true;
        used -= 1;
        return true;
    }

    int size() const { return used; }

    void clear() {
        for (auto& item: _storage) {
            item.used = false;
        }
    }

    void reserve(std::size_t n) { resize(n); }

    std::string __str__() const {
        std::stringstream ss;
        bool              comma = false;
        bool              all   = false;
        ss << "{";
        for (auto i = 0; i < _storage.size(); i++) {

            auto& item = _storage[i];

            if (item.used && !item.deleted) {
                if (comma) {
                    ss << ", ";
                }

                ss << item.key << ": " << item.value;
                comma = true;
            }
        }
        ss << "}";
        return ss.str();
    }

    void resize(std::size_t n) { resize_force(n); }

    // force a full resize with all the entries being reinserted
    void resize_force(std::size_t n) {
        Storage storage(round_size(int(n)));
        int     a = 0;
        int     b = 0;

        for (auto& item: _storage) {
            if (!item.used || item.deleted) {
                continue;
            }
            insert(storage, item, false, a, b);
        }

        _storage = storage;
    }

    // Rebuild the hash using a bigger storage
    // used multiplier == 1 to remove deleted entries
    // can speed up insertion if a lot of keys got deleted
    void rehash(float multiplier = 2.f) {
        resize(_storage.size() * uint64_t(multiplier >= 1 ? multiplier : 2));
    }

    private:
    // this cannot be exposed because it returns a pointer
    // that could change once rehash is called
    Item const* _find(const Key& name) const {
        assert(is_power2(_storage.size()));
        uint64_t i = H::hash(name);

        // linear probing
        for (uint64_t offset = 0; offset < _storage.size(); offset++) {
            // for(int m = -1; m < 2; m += 2) {
            auto& item = _storage[P2_MOD((i + offset), _storage.size())];

            // item was not used, so we know there was no collision
            // and no linear probe insert was done after this
            if (!item.used) {
                return nullptr;
            }

            // Item is not deleted and key matches
            if (!item.deleted && item.key == name) {
                return &item;
            }
            //}
        }

        return nullptr;
    }

    // insert a key value pair
    // if the key already exist does not insert
    bool track_insert(const Key& name, const Value& value, bool upsert) {
        if (load_factor() >= 0.75) {
            rehash();
        }

        Item item(name, value);
        return insert(_storage, item, upsert, this->used, this->collision);
    }

    // insert is slower than std::unordered_map
    static bool
    insert(Storage& data, Item const& inserted_item, bool upsert, int& used, int& collision) {

        assert(is_power2(data.size()));
        uint64_t i = inserted_item.hash();

        for (uint64_t offset = 0; offset < data.size(); offset++) {
            Item& item = data[P2_MOD((i + offset), data.size())];

            if (!item.used || item.deleted) {
                item = inserted_item;
                used += 1;
                return true;
            }

            if (item.key == inserted_item.key) {
                if (upsert) {
                    item.value = inserted_item.value;
                    item.set_hash(i);
                    return true;
                }
                return false;
            }

            collision += 1;
            //}
        }

        return false;
    }

    int     used      = 0;
    int     collision = 0;
    Storage _storage;

    public:
    int collided() const { return collision; }
};
}  // namespace lython

#endif
