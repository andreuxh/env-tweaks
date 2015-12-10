/*
  Copyright (c) 2015 Hugues Andreux
  
  Permission is hereby granted, free of charge, to any person obtaining a copy 
  of this software and associated documentation files (the "Software"), to 
  deal in the Software without restriction, including without limitation the 
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
  sell copies of the Software, and to permit persons to whom the Software is 
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in 
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
  IN THE SOFTWARE.
*/

// "envtweaks.hh""

#include "loadsym.hh"

#include <stdlib.h>
#include <wordexp.h>

//#include <unordered_set> //XXX
#include <set>
#include <utility>
#include <mutex>

class EnvTweaks
{
    using EnvPair = std::pair<std::string, std::string>;
    //XXX std::hash not specialized for pairs
    //  - I won't bother doing it right now
    //using Environ = std::unordered_set<EnvPair>;
    using Environ = std::set<EnvPair>;

public:
    EnvTweaks()
      : getenv_(LOADSYM(getenv))
    {
        wordexp("", &wordexp_buf_, WRDE_NOCMD);
    }

    ~EnvTweaks()
    {
        wordfree(&wordexp_buf_);
    }

    char *getEnv(const char *name);

private:
    char *(*getenv_)(const char*);
    Environ environ_;
    std::mutex mutex_;
    wordexp_t wordexp_buf_;
};

// "envtweak.cc"

#include <string.h>

char *EnvTweaks::getEnv(const char *name)
{
    char n_alnum = name[strspn(name, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                              "_abcdefghijklmnopqrstuvwxyz")];

    if (n_alnum) {
        std::unique_lock<std::mutex> lock(mutex_);
        int experr = wordexp(name, &wordexp_buf_, WRDE_REUSE);
        if (experr) {
            return NULL;
        }
        EnvPair env_pair(name, wordexp_buf_.we_wordv[0]);
        for (size_t i = 1; i < wordexp_buf_.we_wordc; i++)
        {
            env_pair.second += "";
            env_pair.second += wordexp_buf_.we_wordv[i];
        }
        auto inserted = environ_.insert(env_pair);
        return const_cast<char *>(inserted.first->second.c_str());
    }
    else {
        return getenv_(name);
    }
}

// "getenv.cc"

namespace { EnvTweaks envTweaks; }

extern "C" {

char *getenv(const char *name)
{
    return envTweaks.getEnv(name);
}

}
