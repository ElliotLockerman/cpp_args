
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

int main() {

    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    
}


