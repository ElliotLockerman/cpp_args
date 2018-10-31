
#include <string>
#include <cstdio>

#include "args.hpp"

int main(int argc, char **argv) {

    ArgParse parser("test 1", argc, argv);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", "k", "key-value argument");
    FlagArg flag(parser, "flag", "", "flag argument");
    if (!parser.parse()) { return -1; }

    printf("Arguments:\n");
    printf("\tpos arg\t%s: %s\n", pos.get_name(), pos.value().c_str());
    printf("\tkv arg \t%s: %s\n", key.get_key(), key.value().c_str());
    printf("\tflag   \t%s: %d\n", flag.get_name(), flag.value());

}


