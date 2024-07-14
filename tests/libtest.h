#pragma once

#include "dtypes.h"


namespace lython {


#define KW_TESTING_TYPE(field, attr, type)    \
    struct Test ## field {              \
        Test ## field(type const& v): \
            attr(v)                     \
        {}                              \
        type attr;                    \
    };

KW_TESTING_TYPE(Name, name, String);
KW_TESTING_TYPE(Code, code, String);
KW_TESTING_TYPE(Call, call, String);
KW_TESTING_TYPE(Expected, expected, String);
KW_TESTING_TYPE(Errors, errors, Array<String>);

struct Section {
    String name;
    Array<String> content;
};

struct TestCase {
    TestCase(String const& c, String const& call, String const& t = ""):
        code(c), call(call), expected_type(t) {}

    TestCase(TestName const& n, String const& c, String const& call, String const& t):
        name(n.name), code(c), call(call), expected_type(t)
    {}

    TestCase(String const& c, Array<String> const& errors = Array<String>(), String const& t = ""):
        code(c), errors(errors), expected_type(t)
    {}

    TestCase(String const& c, TestErrors const& errors, String const& t = ""):
        code(c), errors(errors.errors), expected_type(t)
    {}

    TestCase(TestName const& n, String const& c, Array<String> const& errors, String const& t = ""):
        name(n.name), code(c), errors(errors), expected_type(t)
    {}

    Array<String> get_all(String const& name) const {
        for(Section const& section: values) {
            if (section.name == name) {
                return section.content;
            }
        }
        std::cout << "Key `" << name << "` not found\n";
        return Array<String>();
    }

    String get_one(String const& name) const {
        Array<String> content = get_all(name);

        if (content.size() > 1) {
            std::cout << "Key `" << name << "` has " << content.size() << " values but only one is needed\n";
            return "";
        }

        if (content.size() == 0) {
            std::cout << "Key `" << name << "` has no values\n";
            return "";
        }

        return content[0];
    }

    int            version;
    String         name;
    Array<Section> values;

    String        get_code         () const { return get_one("code");             }
    String        get_call         () const { return get_one("call");             }
    String        get_expected_type() const { return get_one("expected");         }
    Array<String> get_errors       () const { return get_all("error");            }
    String        get_result       () const { return get_one("result");           }


    bool has(String const& name) const {
        return get_all(name).size() > 0;
    }

    // Deprecated
    String code;
    String call;
    String expected_type;
    Array<String> errors;

    bool operator==(TestCase const& other) const {
        return code == other.code && call == other.call && expected_type == other.expected_type;
    }
};

// Legacy name
using VMTestCase = TestCase;

void write_case(std::ostream& out, int i, VMTestCase const& testcase) ;

Array<VMTestCase> load_cases(std::istream& in);

Array<VMTestCase> get_test_cases(String const& folder, String const& name, Array<VMTestCase> const&  maybe_cases);
Array<VMTestCase> get_test_cases(String const& folder, String const& name);


Array<VMTestCase>  transition(String const& folder, String const& name, Array<VMTestCase> const& cases);

// Name Error
String NE(String const &name);

// Not Callable
String NC(std::string const &name);

// Type Error
String TE(String const &lhs_v, String const &lhs_t, String const &rhs_v, String const &rhs_t);

// Attribute Error
String AE(String const &name, String const &attr);

// UnsipportedOperand
String UO(String const &op, String const &lhs, String const &rhs);

// Import Error
String IE(String const &import, String const &name);

// ModuleNotFoundError
String MNFE(String const &module);

}