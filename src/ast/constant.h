#ifndef LYTHON_CONSTANT_HEADER
#define LYTHON_CONSTANT_HEADER

namespace lython {

struct ConstantValue {
public:
    enum Type {
        TInt,
        TFloat,
        TString
    };

    ConstantValue() = default;

    ConstantValue(int v):
        kind(TInt)
    {
        value.integer = v;
    }

    ConstantValue(float v):
        kind(TFloat)
    {
        value.decimal = v;
    }

    ConstantValue(double v):
        kind(TFloat)
    {
        value.decimal = v;
    }

    ConstantValue(String const& v):
        kind(TString)
    {
        value.string = v;
    }

    ConstantValue(ConstantValue const& v){
        copy_union(v.kind, v.value);
    }

    ~ConstantValue() {
        if (kind == TString) {
            value.string.~String();
        }
    }

    ConstantValue& operator= (ConstantValue const& v) {
        copy_union(v.kind, v.value);
        return *this;
    }

    template<typename T>
    ConstantValue& operator= (T v) {
        *this = ConstantValue(v);
        return *this;
    }

    void print(std::ostream& out) const;

private:
    // ast.Str, ast.Bytes, ast.NameConstant, ast.Ellipsis
    union ValueVariant {
        ValueVariant()  {}
        ~ValueVariant() {}

        int integer;
        double decimal;
        String string;
    };

    ValueVariant value;
    Type kind;

    void copy_union(Type k, ValueVariant const& v) {
        kind = k;
        switch (kind) {
        case TString: value.string = v.string;
        case TFloat: value.decimal = v.decimal;
        case TInt: value.integer = v.integer;
        }
    }
};

} // lython

#endif