#pragma once
#include <utility>

namespace lython {

template <typename T>
class Optional {
    public:
    // Pointer only
    // nullptr are set to None
    template <typename U = T, std::enable_if_t<std::is_pointer_v<U>, bool> = true>
    Optional(const T& data): _has_data(false) {
        if (data != nullptr) {
            new (&holder.data.value) T(data);
            _has_data = true;
        }
    }

    // Regular
    template <typename U = T, std::enable_if_t<!std::is_pointer_v<U>, bool> = true>
    Optional(const T& data): _has_data(true) {
        new (&holder.data.value) T(data);
    }

    Optional(): _has_data(false) {}

    Optional(const Optional& opt) {
        if (opt._has_data) {
            set_data(opt.holder.data.value);
        }
    }

    // void print(std::ostream &out) {
    //     if (has_value()) {
    //         out << str(value());
    //         return;
    //     }
    //     out << "none";
    //     return;
    // }

    Optional& operator=(const T& data) {
        set_data(data);
        return *this;
    }

    Optional& operator=(Optional const& opt) {
        if (opt._has_data) {
            set_data(opt.holder.data.value);
        }
        return *this;
    }

    bool operator==(Optional const& opt) const {
        if (opt.has_value() == has_value()) {
            if (opt.has_value()) {
                return opt.value() == value();
            }
        }
        return false;
    }

    ~Optional() {
        if (_has_data) {
            holder.data.value.~T();
        }
    }

    bool has_value() const { return _has_data; }

    T const& value() const { return holder.data.value; }

    T& value() { return holder.data.value; }

    T const& fold(T const& default_value) const {
        if (has_value())
            return holder.data.value;

        return default_value;
    }

    private:
    void set_data(const T& data) {
        if (!_has_data) {
            new (&holder.data.value) T(data);
        } else {
            holder.data.value = data;
        }
        _has_data = true;
    }

    struct data_t {
        T value;
    };

    struct no_data_t {};

    union holder_t {
        data_t    data;
        no_data_t nothing;

        holder_t() {}
        ~holder_t() {}
    } holder;

    bool _has_data = false;
};

template <typename T>
Optional<T> none() {
    return Optional<T>();
}

template <typename T>
Optional<T> some(const T& value) {
    return Optional<T>(value);
}

template <typename T>
String str(Optional<T> const& obj) {
    if (obj.has_value()) {
        return "Some(" + str(obj.value()) + ")";
    }

    return "None";
}

}  // namespace lython
