

#ifndef CMDLINE_H
#define CMDLINE_H

#include "format.h"

#include <iomanip>
#include <sstream>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <map>
#include <set>
#include <string>
using std::cout;
using std::endl;
using std::initializer_list;
using std::map;
using std::set;
using std::string;

namespace Details {
// snprintf
template <typename... Args>
std::string format(const std::string &fmtStr, Args... args);

template <typename T>
T str2(const string &val);
} // namespace Details

class CmdLine {
public:
    CmdLine(){}
    CmdLine(const string &appName) : appName_(appName){}
    CmdLine(const string &appName, const string &appInfo, const string &appVersion)
        : appName_(appName), appInfo_(appInfo), appVersion_(appVersion) {}

public:
    using IsMustOffer = bool;
    static const CmdLine::IsMustOffer MustOffer    = true;
    static const CmdLine::IsMustOffer NotMustOffer = false;

    string usage(void);

    template <typename T, IsMustOffer isMustOffer, typename... Choice>
    void regist(const string &flag, const string &name, const string &desc, const string &defaultVal, Choice &&... choice);

    template <typename T>
    T get(const string &optName);

    void parse(int argc, char **argv);

private:
    bool isOption(const string &val);
    bool isAtChoiceList(const string &optStr, const string &valStr);
    void RegistHelp(void);

private:
    struct Items {
        inline bool operator < (const Items& rhs) const noexcept {
            return this->argvFlag_ < rhs.argvFlag_;
        }
        string argvFlag_;
        string argvName_;
        string argvDesc_;
        string argvValue_;
        IsMustOffer isMustOffer_ = false;
        set<string> argvValueChoice_;
        bool isRadioOpt_ = false;
    };

private:
    string programName_;
    string appName_;
    string appInfo_;
    string appVersion_;
    map<string, Items> args_;
    map<string, string> flagNamePair_;
};


template <typename T>
T CmdLine::get(const string& optName) {
    string name;
    if (!this->isOption("-" + optName)) {
        if (!this->isOption("--" + optName)) {
            throw std::runtime_error(format("Can not distinguish option name: " + optName));
        } else {
            name = flagNamePair_["--" + optName];
        }
    } else {
        name = "-" + optName;
    }
    return Details::str2<T>(args_[name].argvValue_);
}

template <typename T, CmdLine::IsMustOffer isMustOffer, typename... Choice>
void CmdLine::regist(const string &flag, const string &name, const string &desc,
            const string &defaultVal, Choice &&... choice) {
    Items tmp;
    tmp.argvFlag_    = "-" + flag;
    tmp.argvName_    = "--" + name;
    tmp.argvDesc_    = desc;
    tmp.isMustOffer_ = isMustOffer;
    tmp.argvValue_   = defaultVal;
    (void)std::initializer_list<int>{
        (tmp.argvValueChoice_.insert(choice), 0)...};
    tmp.isRadioOpt_ = ((tmp.argvValueChoice_.empty()) ? false : true);
    args_.insert(std::make_pair(tmp.argvFlag_, tmp));

    flagNamePair_[tmp.argvFlag_] = tmp.argvName_;
    flagNamePair_[tmp.argvName_] = tmp.argvFlag_;
}



void CmdLine::RegistHelp(void) {

    Items tmp;
    tmp.argvFlag_ = "-h";
    tmp.argvName_ = "--help";
    tmp.argvDesc_ = "Print this help message.";
    tmp.isMustOffer_ = false;
    tmp.isRadioOpt_  = false;
    args_[tmp.argvFlag_] = tmp;
}

bool CmdLine::isOption(const string &val) {
    for(auto it : args_){
        if (it.first.compare(val) == 0 || it.second.argvName_.compare(val)==0) return true;
    }
    return false;
}

bool CmdLine::isAtChoiceList(const string &optStr, const string &valStr) {
    auto item = args_[optStr];
    if (item.isRadioOpt_) {
        if (item.argvValueChoice_.find(valStr) != item.argvValueChoice_.end()) {
            return true;
        } else
            return false;
    } else
        return true;
}

void CmdLine::parse(int argc, char **argv) {

    this->RegistHelp();

    programName_ = argv[0];

    if (argc > 1 && (string(argv[1]).compare("-h") == 0 || string(argv[1]).compare("--help") == 0)) {
        std::cout << this->usage();
        exit(EXIT_SUCCESS);
    }

    for (int i = 1; i < argc; i += 2) {
        string optStr{argv[i]};
        string valStr;
        // find a registed option
        if (this->isOption(optStr)) {
            // not find value of current opton
            if (i + 1 >= argc || ((valStr = argv[i + 1]), this->isOption(valStr))) {
                throw std::runtime_error(format("Missing value of option: %s. %sFrom %s:%d\n", optStr.c_str(), this->usage().c_str(), __FILE__, __LINE__));
            }
            // find value of current option
            else {
                if (!this->isAtChoiceList(optStr, valStr)) {
                    throw std::runtime_error(format("Wrong value of Option: %s. %sFrom%s:%d\n", optStr.c_str(), this->usage().c_str(), __FILE__, __LINE__));
                } else {
                    args_[optStr].argvValue_ = valStr;
                }
            }
        }
        // not find a registed valid option
        else {
            throw std::runtime_error(format("Wrong Option name: %s. %sFrom%s:%d\n", optStr.c_str(), this->usage().c_str(), __FILE__, __LINE__));
        }
    }

    for(auto it : args_){
        if (it.second.isMustOffer_ && it.second.argvValue_.empty()) {
            std::cout << "Missing Option: " + it.first + this->usage();
            exit(EXIT_FAILURE);
        }
    }
}

string CmdLine::usage(void) {

    string rst = "\n\nUsage: " + programName_ + " [option=value]\nOptions:\n";

    std::stringstream ss;
    for (auto it : args_) {
        auto item = it.second;
        ss << "    " << std::setw(25) << std::left
           << item.argvFlag_ + "," + item.argvName_ << item.argvDesc_
           << ((item.argvValueChoice_.size() > 0) ? ([&]() {
                  string rst = " The accepted options: ";
                  for (auto it : item.argvValueChoice_) rst+= it + ", ";
                  return rst + '\n';
              }())
                                                  : "\n");
    }
    rst += ss.str();

    rst += "Program " + appName_ + "\n\n";
    return rst;
}

namespace Details{
template <typename... Args>
std::string format(const std::string &fmtStr, Args... args)
{
    size_t len = snprintf(nullptr, 0, fmtStr.c_str(), args...) + 1;
    if (len <= 0) { throw std::runtime_error("Error during formatting."); }
    std::unique_ptr<char[]> buf(new char[len]);
    snprintf(buf.get(), len, fmtStr.c_str(), args...);
    return std::string(buf.get(), buf.get() + len - 1);
}

template <>
float str2<float>(const string &val) {
    return std::strtof(val.c_str(), nullptr);
}

template <>
double str2<double>(const string &val) {
    return std::strtod(val.c_str(), nullptr);
}

template <>
int str2<int>(const string &val) {
    return int(str2<double>(val));
}

template <>
size_t str2<size_t>(const string &val) {
    return size_t(str2<double>(val));
}

template <>
string str2<string>(const string &val) {
    return val;
}
}


#endif