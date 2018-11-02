
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

namespace args {

////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////

#define panic(...) \
do { \
    fprintf(stderr, __VA_ARGS__); \
    exit(-1); \
} while (0);


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
// Parser 
////////////////////////////////////////////////////////////////////////////////

const char* status_str[] = {
    "SUCCESS",
    "INVALID_KEY",
    "MISSING_VALUE",
    "EXTRA_VALUE",
    "ISTREAM_ERROR",
    "IS_FLAG",
    "MISSING_ARG",
    "EXTRA_ARG",
    "HELP"
};

enum class Status {
    SUCCESS = 0,
    INVALID_KEY,
    MISSING_VALUE,
    EXTRA_VALUE,
    ISTREAM_ERROR,
    IS_FLAG,
    MISSING_ARG,
    EXTRA_ARG,
    HELP
};

static inline std::ostream& operator<<(std::ostream& os, Status s) {
    os << status_str[(int)s];
    return os;
}

struct Result {
    Status status;
    std::string item;

    explicit Result(Status _status, const std::string& _item) : status(_status), item(_item) {}
    explicit Result(Status _status, const char* _item) : status(_status), item(_item) {}
    explicit Result(Status _status, char _item) : status(_status), item(1,_item) {}

    operator bool() { return status == Status::SUCCESS; }
};




class Parser : public ParserBase {
public:
    Parser(const char* _app_name, int argc, const char **argv, bool _silent=false) 
    : app_name(_app_name), silent(_silent) {
        for (int i=1; i<argc; i++) { args.push_back(argv[i]); }
    }

    void add_pos_arg(PosArgBase *pos_arg) override {
        pos_args.push_back(pos_arg);
    }


    void add_kv_arg(KVArgBase *kv_arg) override {
        std::string k = kv_arg->get_key();
        std::string short_k = kv_arg->get_short_key();

        if (k == "help") {
            panic("Parser config error: config number %d's key cannot be \"help\" (configs with builtin help flag", configs);
        }

        if (short_k == "h") {
            panic("Parser config error: config number %d's short key cannot be \"h\" (configs with builtin help flag", configs);
        }

        configs++;
        if (k.size() == 0) {
            panic("Parser config error: config number %d's key cannot be empty", configs);
        }

        if (k.find('=') != std::string::npos) {
            panic("Parser config error: config number %d's key cannot contain \"=\"", configs);
        }

        // kv_keys.push_back(kv_arg);

        if (kv_keys.count(k) != 0 || flag_keys.count(k) != 0) {
            panic("Parser config error: config number %d's key %s duplicated", configs, k.c_str());
        }
        kv_keys[k] = kv_arg;

        if (short_k != "") {
            if (short_k.size() > 1) {
                panic("Parser config error: config number %d's short key %s is %zu characters; A short key must be zero characters (no short key) or one character\n", configs, short_k.c_str(), short_k.size());
            }
            char c = short_k[0];
            if (kv_short_keys.count(c) != 0 || flag_short_keys.count(c) != 0) {
                panic("Parser config error: config number %d's short key %c is a duplicate", configs, c);
            }
            kv_short_keys[c] = kv_arg;
        }

    }


    void add_flag_arg(FlagArg *flag_arg) override {
        std::string k = flag_arg->get_key();
        std::string short_k = flag_arg->get_short_key();

        configs++;
        if (k.size() == 0) {
            panic("Parser config error: config number %d's key cannot be empty", configs);
        }

        // flag_keys.push_back(flag_arg);

        if (flag_keys.count(k) != 0 || kv_keys.count(k) != 0) {
            panic("Parser config error: config number %d's key %s a duplicate", configs, k.c_str());
        }
        flag_keys[k] = flag_arg;

        if (short_k != "") {
            if (short_k.size() > 1) {
                panic("Parser config error: config number %d's short key %s is %zu characters; A short key must be zero characters (no short key) or one character\n", configs, short_k.c_str(), short_k.size());
            }
            char c = k[0];
            if (flag_short_keys.count(c) != 0 || kv_short_keys.count(c) != 0) {
                panic("Parser config error: config number %d's short key %c is a duplicate", configs, c);
            }
            flag_short_keys[c] = flag_arg;
        }

    }


    Result parse() {

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
                auto res = parse_long_arg(args);
                if (!res) {
                    return res;
                }


             // Short key
            } else if (!saw_double_dash && arg.size() >= 2 && arg[0] == '-') {
                auto res = parse_short_arg(args); 
                if (!res) {
                    return res;
                }

            // Positional arg
            } else {
                auto res = parse_positional_arg(args);
                if (!res) {
                    return res;
                }
            }

        }


        if (consumed_pos_args < pos_args.size()) {
            if (!silent) { 
                fprintf(stderr, "Missing required positional argument(s)\n");
                print_usage();
            }
            return Result(Status::MISSING_ARG, "");
        }

        return Result(Status::SUCCESS, "");
    }

    Result parse_long_arg(std::vector<std::string>& args) {
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

        if (key == "help") {
            print_usage();
            return Result(Status::HELP, "");
        }

        auto it = kv_keys.find(key);
        if (it == kv_keys.end()) {
            auto it2 = flag_keys.find(key);
            if (it2 == flag_keys.end()) {
                if (!silent) { 
                    fprintf(stderr, "Long argument key --%s invalid\n", key.c_str());
                    print_usage();
                }
                return Result(Status::INVALID_KEY, key);
            }

            it2->second->parse();
            return Result(Status::SUCCESS, "");   
        }


        // Get value
        if (eq != std::string::npos) {
            value = arg.substr(eq+1, std::string::npos);
        } else {
            if (args.empty()) {
                if (!silent) { 
                    fprintf(stderr, "Long argument key --%s needs value\n", key.c_str());
                    print_usage();
                }
                return Result(Status::MISSING_VALUE, key);
            }

            value = std::move(args.back());
            args.pop_back();
        }

        bool good = it->second->parse(value);
        if (!good) {
            if (!silent) { 
                fprintf(stderr, "Could not parse value of argument --%s\n", key.c_str());
                print_usage();
            }
            return Result(Status::ISTREAM_ERROR, key);
        }
        

        return Result(Status::SUCCESS, "");
    }


    Result parse_short_arg(std::vector<std::string>& args) {
        auto arg = std::move(args.back());
        args.pop_back();

        char key = arg[1];

        if (key == 'h') {
            print_usage();
            return Result(Status::HELP, "");
        }
        
        auto it = kv_short_keys.find(key);
        if (it == kv_short_keys.end()) {
            auto it2 = flag_short_keys.find(key);
            if (it2 == flag_short_keys.end()) {
                if (!silent) { fprintf(stderr, "Short argument key -%c invalid\n", key);
                print_usage(); }
                return Result(Status::INVALID_KEY, key);
            } else if (arg.size() > 2) {
                if (!silent) { 
                    fprintf(stderr, "Flag -%c doesn't take a value\n", key);
                    print_usage();
                }
                return Result(Status::EXTRA_VALUE, key);
            }

            it2->second->parse();
            return Result(Status::SUCCESS, "");   
        }



        std::string value;
        if (arg.size() > 2) {
            value = arg.substr(2, std::string::npos);
        } else {
            if (args.empty()) {
                if (!silent) { 
                    fprintf(stderr, "Short argument key -%c needs value\n", key);
                    print_usage();
                }
                return Result(Status::MISSING_VALUE, key);
            }
            value = std::move(args.back());
            args.pop_back();
        }


        bool good = it->second->parse(value);
        if (!good) {
            if (!silent) { 
                fprintf(stderr, "Could not parse value of argument -%c\n", key);
                print_usage();
            }
            return Result(Status::ISTREAM_ERROR, key);
        }

        return Result(Status::SUCCESS, ""); 
    }

    Result parse_positional_arg(std::vector<std::string>& args) {
        auto arg = args.back();
        args.pop_back();

        if (consumed_pos_args < pos_args.size()) {
            auto& pos_arg = pos_args.at(consumed_pos_args);
            bool good = pos_arg->parse(arg);
            if (!good) {
                if (!silent) { 
                    fprintf(stderr, "Could not parse positional argument \"%s\"\n", arg.c_str());
                    print_usage();
                }
                return Result(Status::ISTREAM_ERROR, pos_arg->get_name());
            }
            consumed_pos_args++;
            return Result(Status::SUCCESS, ""); 

        // Extranous positional arg
        } else {
            if (!silent) { 
                fprintf(stderr, "Too many positional arguments\n");
                print_usage();
            }
            return Result(Status::EXTRA_ARG, "");
        }
    }


    void print_usage() const {
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "\t%s: ", app_name);

        if (kv_keys.size() > 0) {
            fprintf(stderr, " [OPTIONS]");
        }

        fprintf(stderr, " [FLAGS]");

        for (auto& config : pos_args) {
            fprintf(stderr, "<%s>", config->get_name());
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


        fprintf(stderr, "\nFLAGS:\n");
        for (auto& p : flag_keys) {
            fprintf(stderr, "\t--%s", p.first.c_str());

            if (*p.second->get_short_key() != '\0') {
                fprintf(stderr, ", -%s", p.second->get_short_key());
            }

            fprintf(stderr, "\t%s\n", p.second->get_desc());
        }
        fprintf(stderr, "\t--help, -h\tPrint help message\n");
    }


private:
    const char* app_name;
    bool silent = false;
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



} // namespace parser
