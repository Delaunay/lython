#ifndef LYTHON_CONSTANT_HEADER
#define LYTHON_CONSTANT_HEADER

#include <iostream>

namespace lython {

struct ConstantValue {
    public:
    enum Type
    {
        TInvalid,
        TInt,
        TFloat,
        TDouble,
        TString
    };

    ConstantValue() = default;

    ConstantValue(int32 v): kind(TInt) { value.integer = v; }

    ConstantValue(float32 v): kind(TFloat) { value.single = v; }

    ConstantValue(float64 v): kind(TDouble) { value.decimal = v; }

    ConstantValue(String const &v): kind(TInvalid) { set_string(v); }

    // ConstantValue(ConstantValue const &v): kind(TInvalid) { copy_union(v.kind, v.value); }

    ~ConstantValue() { remove_str(); }

    // ConstantValue &operator=(ConstantValue const &v) {
    //     copy_union(v.kind, v.value);
    //     return *this;
    // }

    ConstantValue &operator=(String const &v) {
        set_string(v);
        return *this;
    }

    ConstantValue &operator=(int32 v) {
        kind          = TInt;
        value.integer = v;
        return *this;
    }

    ConstantValue &operator=(float32 v) {
        kind         = TFloat;
        value.single = v;
        return *this;
    }

    ConstantValue &operator=(float64 v) {
        kind          = TDouble;
        value.decimal = v;
        return *this;
    }

    void print(std::ostream &out) const;

    private:
    // ast.Str, ast.Bytes, ast.NameConstant, ast.Ellipsis
    union ValueVariant {
        ValueVariant() {}
        ~ValueVariant() {}

        int32   integer;
        float64 decimal;
        float32 single;
        String  string;
    };

    ValueVariant value;
    Type         kind = TInvalid;

    void set_string(const String &data) {
        if (kind != Type::TString) {
            new (&value.string) String(data);
        } else {
            value.string = data;
        }
        kind = Type::TString;
    }

    void remove_str() {
        if (kind == TString) {
            value.string.~String();
        }
    }

    void copy_union(Type k, ValueVariant const &v) {
        switch (kind) {
        case TInvalid:
            break;

        case TInt:
            remove_str();
            value.integer = v.integer;
            break;

        case TFloat:
            remove_str();
            value.single = v.single;
            break;

        case TDouble:
            remove_str();
            value.decimal = v.decimal;
            break;

        case TString:
            set_string(v.string);
            break;
        }
        kind = k;
    }
};

} // namespace lython

#endif