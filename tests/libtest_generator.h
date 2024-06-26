
//              Copyright Catch2 Authors
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE.txt or copy at
//        https://www.boost.org/LICENSE_1_0.txt)

// SPDX-License-Identifier: BSL-1.0

// 300-Gen-OwnGenerator.cpp
// Shows how to define a custom generator.

// Specifically we will implement a random number generator for integers
// It will have infinite capacity and settable lower/upper bound

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>

#include "libtest.h"

#include <filesystem>

namespace lython {

class FileTestCaseGenerator final : public Catch::Generators::IGenerator<TestCase> 
{    
public:
    FileTestCaseGenerator(String const& folder, String const& name);

    TestCase const& get() const override;

    bool next() override;

    std::string stringifyImpl() const override;

    int i = 0;
    std::filesystem::path path;
    Array<TestCase> cases;
};

Catch::Generators::GeneratorWrapper<TestCase> filecase(String const& folder, String const& name);

class FolderTestCaseGenerator final : public Catch::Generators::IGenerator<TestCase> 
{    
public:
    FolderTestCaseGenerator(String const& folder);

    TestCase const& get() const override;

    bool next() override;

    std::string stringifyImpl() const override;

    int i = 0;
    Array<std::filesystem::path> folders;
    Array<TestCase> cases;
};

Catch::Generators::GeneratorWrapper<TestCase> foldercase(String const& folder);


class AllTestCaseGenerator final : public Catch::Generators::IGenerator<TestCase> 
{    
public:
    AllTestCaseGenerator();

    TestCase const& get() const override;

    bool next() override;

    std::string stringifyImpl() const override;

    int i = 0;
    Array<std::filesystem::path> folders;
    Array<TestCase> cases;
};

Catch::Generators::GeneratorWrapper<TestCase> allcase();


}
