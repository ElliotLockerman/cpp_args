
#include <string>
#include <cstdio>
#include <cassert>

#include "args.hpp"

using namespace args;


void test1() {

    const char* argv[] = {"", "pos", "--kv", "val"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "", "flag argument");
   
    auto res = parser.parse();
    assert(res);
    assert(pos.was_found() && pos.value() == "pos");
    assert(key.was_found() && key.value() == "val");
    assert(!flag.was_found());

    printf("%s: ok\n", __func__);
}


void test2() {

    const char* argv[] = {"", "pos", "--kv", "val", "--flag"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "", "flag argument");
   
    auto res = parser.parse();
    assert(res);
    assert(pos.was_found() && pos.value() == "pos");
    assert(key.was_found() && key.value() == "val");
    assert(flag.was_found());

    printf("%s: ok\n", __func__);
}

void test3() {

    const char* argv[] = {"", "pos", "--kv=val", "-f"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "f", "flag argument");
   
    auto res = parser.parse();
    assert(res);
    assert(pos.was_found() && pos.value() == "pos");
    assert(key.was_found() && key.value() == "val");
    assert(flag.was_found());

    printf("%s: ok\n", __func__);
}

void test4() {

    const char* argv[] = {"", "--kv=val", "-f"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "f", "flag argument");
   
    auto res = parser.parse();
    assert(!res);
    assert(res.status == Status::MISSING_ARG);
    assert(res.item == "");

    printf("%s: ok\n", __func__);
}

void test5() {

    const char* argv[] = {"", "pos", "-f", "--kv"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "f", "flag argument");
   
    auto res = parser.parse();
    assert(!res);
    assert(res.status == Status::MISSING_VALUE);
    assert(res.item == "kv");

    printf("%s: ok\n", __func__);
}

void test6() {

    const char* argv[] = {"", "pos", "-f", "--kv=v", "pos"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "f", "flag argument");
   
    auto res = parser.parse();
    assert(!res);
    assert(res.status == Status::EXTRA_ARG);

    printf("%s: ok\n", __func__);
}

void test7() {

    const char* argv[] = {"", "pos", "-f", "--kv=v"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<int> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "f", "flag argument");
   
    auto res = parser.parse();
    assert(!res);
    assert(res.status == Status::ISTREAM_ERROR);
    assert(res.item == "kv");

    printf("%s: ok\n", __func__);
}


void test8() {

    const char* argv[] = {"", "pos", "-f", "--kv=v"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<int> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "", "flag argument");
   
    auto res = parser.parse();
    assert(!res);
    assert(res.status == Status::INVALID_KEY);
    assert(res.item == "f");

    printf("%s: ok\n", __func__);
}

void test9() {

    const char* argv[] = {"", "pos", "0", "--flag", "1", "--kv", "val", "2"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "", "flag argument");
    VarArg<int> nums(parser, "nums", "numbers");
   
    auto res = parser.parse();
    assert(res);
    assert(pos.was_found() && pos.value() == "pos");
    assert(key.was_found() && key.value() == "val");

    auto vec = *nums;
    assert(vec.size() == 3);
    assert(vec.at(0) == 0 && vec.at(1) == 1 && vec.at(2) == 2);

    printf("%s: ok\n", __func__);
}

void test10() {
    printf("\n\n\n");

    const char* argv[] = {"", "--kb", "vb", "posA", "-a", "posB", "0",  "1", "-kva", "2"};
    int argc = std::end(argv) - std::begin(argv);

    Parser parser("test", argc, argv, true);
    PosArg<std::string> posA(parser, "posA", "positional argument");
    PosArg<std::string> posB(parser, "posB", "positional argument");
    KVArg<std::string> keyA(parser, "ka", "k", "key-value argument");
    KVArg<std::string> keyB(parser, "kb", "", "key-value argument");
    FlagArg flagA(parser, "flagA", "a", "flag argument");
    FlagArg flagB(parser, "flagB", "b", "flag argument");
    VarArg<int> nums(parser, "nums", "numbers");
   
    auto res = parser.parse();
    assert(res);
    if (posA.was_found()) { printf("posA: %s\n", posA.value().c_str()); }
    assert(posA.was_found() && posA.value() == "posA");
    assert(posB.was_found() && posB.value() == "posB");
    assert(keyA.was_found() && keyA.value() == "va");
    assert(keyB.was_found() && keyB.value() == "vb");
    assert(flagA.was_found());
    assert(!flagB.was_found());

    auto vec = *nums;
    assert(vec.size() == 3);
    assert(vec.at(0) == 0 && vec.at(1) == 1 && vec.at(2) == 2);

    printf("%s: ok\n", __func__);
}

int main() {

    test1();
    test2();
    test3();
    test4();
    test5();
    test7();
    test8();
    test9();
    test10();

}


