#include "libtest.h"
#include "utilities/strings.h"
#include <regex>
#include <iostream>

namespace lython {

void write_case(std::ostream& out, int i, VMTestCase const& testcase) {

    out << "# >>> case: " << i << "\n";
    out << "# >>> code\n";
    out << testcase.code;
    out << "\n\n";
    out << "# >>> call\n";
    out << testcase.call;
    out << "\n\n";
    out << "# >>> expected\n";
    out << testcase.expected_type;
    out << "\n\n";
}

enum class CaseSection
{
    Case,
    Code,
    Call,
    Expected
};

Array<VMTestCase> load_cases(std::istream& in) {
    std::regex case_regex("# >>> case");
    std::regex code_regex("# >>> code");
    std::regex call_regex("# >>> call");
    std::regex expe_regex("# >>> expected");

    Array<String> buffer;

    Array<VMTestCase> cases;

    int         i = 0;
    VMTestCase  currentcase{"", ""};
    CaseSection section;
    String line;

    auto add_case = [&]() {
        if (i > 0) {
            cases.push_back(currentcase);
            currentcase = VMTestCase("", "");
            i           = 0;
        }
    };

    auto push =
        [&]() {
            if (buffer.size() > 0) {
                String str = join("\n", buffer);
                buffer.resize(0);

                std::cout << str << std::endl;

                switch (section) {
                case CaseSection::Case: {
                    add_case();
                }
                case CaseSection::Code: {
                    currentcase.code = str;
                    i += 1;
                }
                case CaseSection::Call: {
                    currentcase.call = str;
                    i += 1;
                }
                case CaseSection::Expected: {
                    currentcase.expected_type = str;
                    i += 1;
                }
                }
            }
        };

    while (std::getline(in, line)) {
        if (std::regex_search(line, case_regex)) {
            push();
            add_case();
            section = CaseSection::Case;
            continue;
        }
        if (std::regex_search(line, code_regex)) {
            push();
            section = CaseSection::Code;
            continue;
        }
        if (std::regex_search(line, call_regex)) {
            push();
            section = CaseSection::Call;
            continue;
        }
        if (std::regex_search(line, expe_regex)) {
            push();
            section = CaseSection::Expected;
            continue;
        }

        buffer.push_back(line);
    }
    push();
    add_case();

    // kwassert(cases.size() > 0, "Should have cases");
    std::cout << cases.size() << std::endl;
    return cases;
}



Array<VMTestCase> get_test_cases(String const& name, Array<VMTestCase> const& maybe_cases) {
#if 0
    String testfile = [&]() {
        StringStream sspath;
        sspath << reg_modules_path() << "/" << name << ".py";
        return sspath.str();
    }();

    std::ofstream testwriter(testfile.c_str());
    int i = 0;

    for (auto& c: maybe_cases) {
        write_case(testwriter, i, c);
    }
#endif

#if 0
    std::ifstream testfile_fp(testfile.c_str());
    Array<VMTestCase> cases = load_cases(testfile_fp);
    return cases;
#endif
    return maybe_cases;
}

}