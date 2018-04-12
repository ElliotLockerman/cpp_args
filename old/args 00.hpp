
#include <iostream>
#include <string>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <sstream>

#pragma once

template<typename T>
class Arg {
public:
    Arg(const char *_tag, bool _short_tag, const char *_description, T _def)
    : tag(_tag), short_tag(_short_tag), description(_description), def(_def) {}

}


class ArgParse {
public:
    ArgParse(int _argc, const char **_argv) : argc(_argc), argv(_argv) {
        for (int i=1; i<argc; i+=2) {
            assert(argv[i][0] != '\0');
            if (argv[i][0] != '-') { 
                std::cerr << "Position " << i << " must be argument name" << std::endl;
                std::abort();
            }

            std::pair<std::string, std::string> p;
            assert(argv[i][1] != '\0');
            if (argv[i][1] == '-') {
                p.first = std::string(argv[i][2]);
            } else {
                p.first = std::string(argv[i][1]);
                if (argv[i][2] != '\0') {
                    std::cerr << "Short argument name at position " << i << " should be 1 letter" << std::endl;
                    std::abort();
                }
            }

            i++;
            assert(argv[i][0] != '\0');
            if (argv[i][0] == '-') {
                std::cerr << "Argument at position " << i << " must be value, not key" << std::endl;
                std::abort
            }

            p.second = std::string(argv[i]);

            kw_args.insert(p);
        }
    }

    template<typename T>
    T parse(const char *_tag, bool short, const char *usage, T deft) const {

        if (!arg) { return deft; }

        istringstream is(arg);
        assert(is);
        
        T tmp;
        is >> tmp;

        if (!is) {
            std::cerr << msg;
            std::abort();
        }

        return tmp;
    }

    const char* find(const char *_tag, bool _short_tag) {
        for (int i=1; i<argc; i++) {
        }
    }

private:
    int argc;
    const char **argv;

    // Don't support positional args or flags yet
    std::unordered_map<std::string, const char*> kw_args;

};



