
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
#include <utility>

////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////

namespace {
template<typename...Args>
void panic(const char* fmt, Args&&...args) {
    fprintf(stderr, fmt, std::forward<Args>(args)...);
    exit(-1);
}
} // anon namespace


////////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////////////////
// Argument classes
////////////////////////////////////////////////////////////////////////////////
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


    bool parse(std::string str) override {
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
    KVArgBase(ParserBase& parser, const char* _k, const char* _short_k, const char* _desc) 
    : ArgBase(_k, _desc), k(_k), short_k(_short_k) {
        parser.add_kv_arg(this);
    }

    virtual bool parse(std::string str) = 0;

    const char* get_key() const { return k; }
    const char* get_short_key() const { return short_k; }

protected:
    const char* k;
    const char* short_k;   
};


template<typename T>
class KVArg : public KVArgBase {
public:
    KVArg(ParserBase& parser, const char* _k, const char* _short_k, const char* _desc)
    : KVArgBase(parser, _k, _short_k, _desc) { }

    bool parse(std::string str) override {
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
    FlagArg(ParserBase& parser, const char* _k, const char* _short_k, const char *_desc) 
    : ArgBase(_k, _desc), k(_k), short_k(_short_k) {
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

    const char* get_key() const { return k; }
    const char* get_short_key() const { return short_k; }

protected:
    const char* k;
    const char* short_k;
};





////////////////////////////////////////////////////////////////////////////////
// Parser proper
////////////////////////////////////////////////////////////////////////////////

class ArgParse : public ParserBase {
public:
    ArgParse(const char* _app_name, int argc, char **argv) 
    : app_name(_app_name) {
        for (int i=1; i<argc; i++) { args.push_back(argv[i]); }
    }

    void add_pos_arg(PosArgBase *pos_arg) override {
        pos_args.push_back(pos_arg);
    }


    void add_kv_arg(KVArgBase *kv_arg) override {
        std::string k = kv_arg->get_key();
        std::string short_k = kv_arg->get_short_key();

        configs++;
        if (k.size() == 0) {
            panic("ArgParse config error: config number %d's key cannot be empty", configs);
        }

        if (k.find('=') != std::string::npos) {
            panic("ArgParse config error: config number %d's key cannot contain \"=\"", configs);
        }

        // kv_keys.push_back(kv_arg);

        if (kv_keys.count(k) != 0 || flag_keys.count(k) != 0) {
            panic("ArgParse config error: config number %d's key %s duplicated", configs, k.c_str());
        }
        kv_keys[k] = kv_arg;

        if (short_k != "") {
            if (short_k.size() > 1) {
                panic("ArgParse config error: config number %d's short key %s is %zu characters; A short key must be zero characters (no short key) or one character\n", configs, short_k.c_str(), short_k.size());
            }
            char c = short_k[0];
            if (kv_short_keys.count(c) != 0 || flag_short_keys.count(c) != 0) {
                panic("ArgParse config error: config number %d's short key %c is a duplicate", configs, c);
            }
            kv_short_keys[c] = kv_arg;
        }

    }


    void add_flag_arg(FlagArg *flag_arg) override {
        std::string k = flag_arg->get_key();
        std::string short_k = flag_arg->get_short_key();

        configs++;
        if (k.size() == 0) {
            panic("ArgParse config error: config number %d's key cannot be empty", configs);
        }

        // flag_keys.push_back(flag_arg);

        if (flag_keys.count(k) != 0 || kv_keys.count(k) != 0) {
            panic("ArgParse config error: config number %d's key %s a duplicate", configs, k.c_str());
        }
        flag_keys[k] = flag_arg;

        if (short_k != "") {
            if (short_k.size() > 1) {
                panic("ArgParse config error: config number %d's short key %s is %zu characters; A short key must be zero characters (no short key) or one character\n", configs, short_k.c_str(), short_k.size());
            }
            char c = k[0];
            if (flag_short_keys.count(c) != 0 || kv_short_keys.count(c) != 0) {
                panic("ArgParse config error: config number %d's short key %c is a duplicate", configs, c);
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
        
        std::reverse(args.begin(), args.end());

        while (!args.empty()) {
            // Parse functions pop, so arg no longer valid after call
            auto& arg = args.back();

            if (!saw_double_dash && strcmp(arg.c_str(), "--") == 0) {
                args.pop_back();
                saw_double_dash = true;
                continue;

            // Long key
            } else if (!saw_double_dash && arg.size() > 2 && arg[0] == '-' && arg[1] == '-') { 
                if (!parse_long_arg(args)) {
                    return false;
                }


             // Short key
            } else if (!saw_double_dash && arg.size() >= 2 && arg[0] == '-') { 
                if (!parse_short_arg(args)) {
                    return false;
                }

            // Positional arg
            } else {
                if (!parse_positional_arg(args)) {
                    return false;
                }
            }

        }


        if (consumed_pos_args < pos_args.size()) {
            fprintf(stderr, "Missing required positional arguments\n");
            print_usage();
            return false;
        }

        return true;
    }

    bool parse_long_arg(std::vector<std::string>& args) {
        auto arg = std::move(args.back());
        args.pop_back();

        auto eq = arg.find('=');
        std::string key;
        std::string value;

        // Get key;
        if (eq != std::string::npos) {
            key = arg.substr(2, eq - key.size() - 2);
        } else {
            key = arg.substr(2, std::string::npos);
        }

        auto it = kv_keys.find(key);
        if (it == kv_keys.end()) {
            auto it2 = flag_keys.find(key);
            if (it2 == flag_keys.end()) {
                fprintf(stderr, "Long argument key --%s invalid\n", key.c_str());
                print_usage();
                return false;
            }

            it2->second->parse();
            return true;   
        }


        // Get value
        if (eq != std::string::npos) {
            value = arg.substr(eq+1, std::string::npos);
        } else {
            if (args.empty()) {
                fprintf(stderr, "Long argument key --%s needs value\n", key.c_str());
                print_usage();
                return false;
            }

            value = std::move(args.back());
            args.pop_back();
        }

        bool good = it->second->parse(value);
        if (!good) {
            fprintf(stderr, "Could not parse value of argument --%s\n", key.c_str());
            print_usage();
            return false;
        }
        

        return true;
    }


    bool parse_short_arg(std::vector<std::string>& args) {
        auto arg = std::move(args.back());
        args.pop_back();

        if (arg.size() != 2) {
            fprintf(stderr, "Short argument key \"%s\" is too long to be a short argument\n", arg.c_str());
            print_usage();
            return false;
        }

        char key = arg[1];
        
        auto it = kv_short_keys.find(key);
        if (it == kv_short_keys.end()) {
            auto it2 = flag_short_keys.find(key);
            if (it2 == flag_short_keys.end()) {
                fprintf(stderr, "Short argument key -%c invalid\n", key);
                print_usage();
                return false;
            }

            it2->second->parse();
            return true;   
        }

        if (args.empty()) {
            fprintf(stderr, "Short argument key -%c needs value\n", key);
            print_usage();
            return false;
        }

        auto value = std::move(args.back());
        args.pop_back();

        bool good = it->second->parse(value);
        if (!good) {
            fprintf(stderr, "Could not parse value of argument -%c\n", key);
            print_usage();
            return false;
        }

        return true;
    }

    bool parse_positional_arg(std::vector<std::string>& args) {
        auto arg = args.back();
        args.pop_back();

        if (consumed_pos_args < pos_args.size()) {
            bool good = pos_args.at(consumed_pos_args)->parse(arg);
            if (!good) {
                fprintf(stderr, "Could not parse positional argument \"%s\"\n", arg.c_str());
                print_usage();
                return false;
            }
            consumed_pos_args++;
            return true;

        // Extranous positional arg
        } else {
            fprintf(stderr, "Too many positional arguments\n");
            print_usage();
            return false;
        }
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

                if (*p.second->get_short_key() != '\0') {
                    fprintf(stderr, ", -%s", p.second->get_short_key());
                }

                fprintf(stderr, " <val>\t%s\n", p.second->get_desc());
            }
        }

        if (flag_keys.size() > 0) {
            fprintf(stderr, "\nFLAGS:\n");
            for (auto& p : flag_keys) {
                fprintf(stderr, "\t--%s", p.first.c_str());

                if (*p.second->get_short_key() != '\0') {
                    fprintf(stderr, ", -%s", p.second->get_short_key());
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

    bool saw_double_dash = false;
};




