
#include <string>
#include <cstdio>

#include "args.hpp"
#include "../../qprint/qprint.hpp"

int main(int argc, char **argv) {

    ArgParse parser("test 1", argc, argv);
    PosArg<std::string> pos(parser, "pos", "positional argument");
    KVArg<std::string> key(parser, "kv", true, "key-value argument");
    FlagArg flag(parser, "flag", true, "flag argument");
    if (!parser.parse()) { return -1; }

    qprint("Arguments:\n");
    qprint("\tpos arg\t{}: {}\n", pos.get_name(), pos.value());
    qprint("\tkv arg \t{}: {}\n", key.get_key(), key.value());
    qprint("\tflag   \t{}: {}\n", flag.get_name(), flag.value());

}


