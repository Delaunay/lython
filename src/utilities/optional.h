#pragma once
#include <utility>

namespace lython{

template<typename T>
class Optional{
public:
    Optional(const T& data):
        _has_data(true){
        holder.data.value = data;
    }

    Optional():
        _has_data(false){
    }

    Optional(Optional&& opt):
        _has_data(opt._has_data){
        if (_has_data){
            holder.data.value = std::move(holder.data.value);
        }
    }

    Optional(const Optional& opt):
        _has_data(opt._has_data){
        if (_has_data){
            holder.data.value = holder.data.value;
        }
    }

    ~Optional(){
        if (_has_data){
            holder.data.value.~T();
        }
    }

    bool has_value(){
        return _has_data;
    }

    T value(){
        return holder.data.value;
    }

private:
    struct data_t {
        T value;
    };

    struct no_data_t{};

    union holder_t {
        data_t data;
        no_data_t nothing;

        holder_t(){}
        ~holder_t(){}
    } holder;

    bool _has_data = false;
};

template<typename T>
Optional<T> none() { return Optional<T>();}

template<typename T>
Optional<T> some(const T& value) { return Optional<T>(value);}

}
