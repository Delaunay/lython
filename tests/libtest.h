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

    String code;
    String call;
    String expected_type;
    String name;
    Array<String> errors;


    bool operator==(TestCase const& other) const {
        return code == other.code && call == other.call && expected_type == other.expected_type;
    }
};

// Legacy name
using VMTestCase = TestCase;

void write_case(std::ostream& out, int i, VMTestCase const& testcase) ;

Array<VMTestCase> load_cases(std::istream& in);

Array<VMTestCase> get_test_cases(String const& folder, String const& name, Array<VMTestCase> const& maybe_cases);
Array<VMTestCase> get_test_cases(String const& folder, String const& name);

void transition(String const& folder, String const& name, Array<VMTestCase> const& cases);


}