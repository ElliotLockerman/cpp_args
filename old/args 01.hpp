
#include <iostream>
#include <string>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <sstream>
#include <cstring>

#pragma once

class ArgParse;

class ArgBase {
public:
    explicit ArgBase(const char *_desc) : desc(_desc) {}
    virtual ~ArgBase() {}

    set_parser(ArgParse *_parser) {
        parser = _parser;
    }

    const char *get_desc() { return desc; }

protected:
    const char *desc;
    ArgParse *parser = nullptr;
};


class VKArgBase : public ArgBase {
public:
    VKArgBase(const std::string& _k, bool _short_k, const char* desc) 
    : ArgBase(_desc), k(_k), short_k(_short_k) { }

    const std::string& get_k() { return k; }
    bool get_short_k() { return short_k; }

protected:
    std::string k;
    bool short_k;   
};


template<typename T>
class KVArg : public KVArgBase {
public:
    KVArg(const std::string& _k, bool _short_k, const char* _desc, T _def)
    : VKArgBase(_k, _short_k, desc), def(_def) { }


private:
    T def;
};



class ArgParse {
public:
    ArgParse(int _argc, const char **_argv) : argc(_argc), argv(_argv) {}

    // Keyword argument
    template<typename T>
    void add_arg(std::string _k, bool _short_k, const char *_description, T _def) {
        kv_args_config.push_back(new KVArg<T>(_k, _short_k, _description, _def));
    }

    void parse() {

    }
private:
    int argc;
    const char **argv;

    // Configs
    std::vector<KVArg*> kv_args_config;

    // Preprocessed kw configs
    std::set<std::string> keys;
    std::set<std::string> short_keys

    // Output
    // std::vector<const char *> pos_args_out;
    std::unordered_map<std::string, const char*> kw_args_out;
    // std::vector<const char*> flags_out;



};





/*
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
*/


