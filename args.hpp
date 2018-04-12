
#pragma once


#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cassert>



class KVArgBase;
class PosArgBase;
class FlagArg;

class ParserBase {
public:
    virtual ~ParserBase() {}
    virtual void add_pos_arg(PosArgBase *pos_arg) = 0;
    virtual void add_kv_arg(KVArgBase *kv_arg) = 0;
    virtual void add_flag_arg(FlagArg *flag_arg) = 0;
};


class ArgBase {
public:
    ArgBase(const char* _name, const char *_desc) 
    : name(_name), desc(_desc) {}

    virtual ~ArgBase() {}

    const char *get_desc() const { return desc; }
    const char *get_name() const { return name; }

    bool was_found() const { return found; }

protected:
    bool found = false;
    const char *name;
    const char *desc;
};



class PosArgBase : public ArgBase {
public:
    PosArgBase(ParserBase& parser, const char* _name, const char *_desc) 
    : ArgBase(_name, _desc) {
        parser.add_pos_arg(this);
    }

    virtual bool parse(std::string str) = 0;
};

template<typename T>
class PosArg : public PosArgBase {
public:
    PosArg(ParserBase& parser, const char* _name, const char *_desc) 
    : PosArgBase(parser, _name, _desc) {}


    bool parse(std::string str) {
        found = true;

        std::istringstream is(str);
        assert(is);

        is >> val;

        if (!is) { return false; }

        return true;
    }


    T value() const {
        assert(found);
        return val;
    }

    T operator*() const {
        return value();
    }

private:
    T val{};
};



class KVArgBase : public ArgBase {
public:
    KVArgBase(ParserBase& parser, const std::string& _k, bool _short_k, const char* _desc) 
    : ArgBase(_k.c_str(), _desc), k(_k), short_k(_short_k) {
        if (k.size() == 0) {
            fprintf(stderr, "ArgParse config error: key cannot be empty\n");
            std::abort();
        }

        parser.add_kv_arg(this);
    }

    virtual bool parse(std::string str) = 0;

    const std::string& get_key() const { return k; }
    bool get_short_key() const { return short_k; }

protected:
    std::string k;
    bool short_k;   
};


template<typename T>
class KVArg : public KVArgBase {
public:
    KVArg(ParserBase& parser, const std::string& _k, bool _short_k, const char* _desc)
    : KVArgBase(parser, _k, _short_k, _desc) { }

    bool parse(std::string str) {
        found = true;

        std::istringstream is(str);
        assert(is);

        is >> val;

        if (!is) { return false; }

        return true;
    }


    T value() const {
        assert(found);
        return val;
    }

    T value_or(T def) const {
        return found ? val : def;
    }


private:
    T val{};
};

class FlagArg : public ArgBase {
public:
    FlagArg(ParserBase& parser, const std::string& _k, bool _short_k, const char *_desc) 
    : ArgBase(_k.c_str(), _desc), k(_k), short_k(_short_k) {
        parser.add_flag_arg(this);
    }

    void parse() {
        found = true;
    }

    bool value() const {
        return found;
    }

    bool operator*() const {
        return value();
    }

    const std::string& get_key() const { return k; }
    bool get_short_key() const { return short_k; }

protected:
    std::string k;
    bool short_k;
};




class ArgParse : public ParserBase {
public:
    ArgParse(const char* _app_name, int argc, char **argv) 
    : app_name(_app_name) {
        for (int i=1; i<argc; i++) { args.push_back(argv[i]); }
    }

    void add_pos_arg(PosArgBase *pos_arg) {
        pos_args.push_back(pos_arg);
    }


    void add_kv_arg(KVArgBase *kv_arg) {
        std::string k = kv_arg->get_key();
        bool short_k = kv_arg->get_short_key();

        configs++;
        if (k.size() == 0) {
            fprintf(stderr, "ArgParse config error: config number %d's key cannot be empty", configs);
            std::abort();
        }

        // kv_keys.push_back(kv_arg);

        if (kv_keys.count(k) != 0 || flag_keys.count(k) != 0) {
            fprintf(stderr, "ArgParse config error: config number %d's key %s duplicated", configs, k.c_str());
            std::abort(); 
        }
        kv_keys[k] = kv_arg;

        if (short_k) {
            char c = k[0];
            if (kv_short_keys.count(c) != 0 || flag_short_keys.count(c) != 0) {
                fprintf(stderr, "ArgParse config error: config number %d's short key %c is a duplicate", configs, c);
                std::abort();
            }
            kv_short_keys[c] = kv_arg;
        }

    }


    void add_flag_arg(FlagArg *flag_arg) {
        std::string k = flag_arg->get_key();
        bool short_k = flag_arg->get_short_key();

        configs++;
        if (k.size() == 0) {
            fprintf(stderr, "ArgParse config error: config number %d's key cannot be empty", configs);
            std::abort();
        }

        // flag_keys.push_back(flag_arg);

        if (flag_keys.count(k) != 0 || kv_keys.count(k) != 0) {
            fprintf(stderr, "ArgParse config error: config number %d's key %s a duplicate", configs, k.c_str());
            std::abort(); 
        }
        flag_keys[k] = flag_arg;

        if (short_k) {
            char c = k[0];
            if (flag_short_keys.count(c) != 0 || kv_short_keys.count(c) != 0) {
                fprintf(stderr, "ArgParse config error: config number %d's short key %c is a duplicate", configs, c);
                std::abort();
            }
            flag_short_keys[c] = flag_arg;
        }

    }


    bool parse() {
        if (args.size() < pos_args.size()) {
            fprintf(stderr, "Not enough arguments\n");
            print_usage();
            return false;
        }

        for (uint32_t i=0; i<args.size(); i++) {
            std::string& arg = args[i];
            if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-') { // If its a regular key
                std::string key = arg.substr(2, std::string::npos);

                auto it = kv_keys.find(key);
                if (it == kv_keys.end()) {
                    auto it2 = flag_keys.find(key);
                    if (it2 == flag_keys.end()) {
                        fprintf(stderr, "Argument key %s invalid\n", key.c_str());
                        print_usage();
                        return false;
                    }

                    it2->second->parse();
                    continue;   
                }

                if (i == args.size() - 1) {
                    fprintf(stderr, "Argument key %s needs value\n", key.c_str());
                    print_usage();
                    return false;
                }
                i++;
                bool good = it->second->parse(args[i]);
                if (!good) {
                    fprintf(stderr, "Could not parse value of argument --%s\n", key.c_str());
                    print_usage();
                    return false;
                }

            } else if (arg.size() >= 2 && arg[0] == '-') { // if is a short key
                if (arg.size() != 2) {
                    fprintf(stderr, "Argument %s is too long to be a short argument\n", arg.c_str());
                    print_usage();
                    return false;
                }

                char key = arg[1];
                
                auto it = kv_short_keys.find(key);
                if (it == kv_short_keys.end()) {
                    auto it2 = flag_short_keys.find(key);
                    if (it2 == flag_short_keys.end()) {
                        fprintf(stderr, "Argument key %c invalid\n", key);
                        print_usage();
                        return false;
                    }

                    it2->second->parse();
                    continue;   
                }

                i++;
                bool good = it->second->parse(args[i]);
                if (!good) {
                    fprintf(stderr, "Could not parse value of argument -%c\n", key);
                    print_usage();
                    return false;
                }
            } else if (consumed_pos_args < pos_args.size()) {

                bool good = pos_args.at(consumed_pos_args)->parse(arg);
                if (!good) {
                    fprintf(stderr, "Could not parse positional argument \"%s\"\n", arg.c_str());
                    print_usage();
                    return false;
                }
                consumed_pos_args++;
            } else {
                fprintf(stderr, "Too many positional arguments\n");
                print_usage();
                return false;
            }
        }

        if (consumed_pos_args < pos_args.size()) {
            fprintf(stderr, "Missing required arguments\n");
            print_usage();
            return false;
        }

        return true;
    }


    void print_usage() const {
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "\t%s: ", app_name);

        for (auto& config : pos_args) {
            fprintf(stderr, "<%s>", config->get_name());
        }

        if (flag_keys.size() > 0) {
            fprintf(stderr, " [FLAGS]");
        }

        if (kv_keys.size() > 0) {
            fprintf(stderr, " [OPTIONS]");
        }

        fprintf(stderr, "\n");


        if (pos_args.size() > 0) {
            fprintf(stderr, "\nARGS:\n");
            for (auto& config : pos_args) {
                fprintf(stderr, "\t%s\t%s\n", config->get_name(), config->get_desc());
            }
        }


        if (kv_keys.size() > 0) {
            fprintf(stderr, "\nOPTIONS:\n");
            for (auto& p : kv_keys) {
                fprintf(stderr, "\t--%s", p.first.c_str());

                if (p.second->get_short_key()) {
                    fprintf(stderr, ", -%c", p.first[0]);
                }

                fprintf(stderr, " <val>\t%s\n", p.second->get_desc());
            }
        }

        if (flag_keys.size() > 0) {
            fprintf(stderr, "\nFLAGS:\n");
            for (auto& p : flag_keys) {
                fprintf(stderr, "\t--%s", p.first.c_str());

                if (p.second->get_short_key()) {
                    fprintf(stderr, ", -%c", p.first[0]);
                }

                fprintf(stderr, "\t%s\n", p.second->get_desc());
            }
        }
    }


private:
    const char* app_name;
    std::vector<std::string> args;

    // Configs
    int configs = 0;
    uint32_t consumed_pos_args = 0;
    std::vector<PosArgBase*> pos_args;

    std::map<std::string, KVArgBase*> kv_keys;
    std::map<char, KVArgBase*> kv_short_keys;

    std::map<std::string, FlagArg*> flag_keys;
    std::map<char, FlagArg*> flag_short_keys;
};



