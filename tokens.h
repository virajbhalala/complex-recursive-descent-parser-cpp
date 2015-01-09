

#ifndef __proj3__tokens__
#define __proj3__tokens__

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

namespace Tokens {
    enum Token {
        TEXT, LANGLE, RANGLE, SLASH, ID, EQ, QSTRING, OTHER, END,
    };
    
    extern Token getToken(istream*, string&);
    extern string tokenName(Token);
    extern int currLine;
}

#endif /* defined(__proj3__tokens__) */
