

#include <cctype>
using namespace std;

#include "tokens.h"

namespace Tokens {
    
    int currLine = 1;
    
    string tokenName(Token t)
    {
        switch (t) {
            case TEXT: return "TEXT";
            case LANGLE: return "LANGLE";
            case RANGLE: return "RANGLE";
            case SLASH: return "SLASH";
            case ID: return "ID";
            case EQ: return "EQ";
            case QSTRING: return "QSTRING";
            case OTHER: return "OTHER";
            case END: return "END";
        }
    }
    
    
    //Lexer method to identify token (called from main)
    Token getToken(istream *in, string& lexeme)
    {
        enum LexState { STARTING, INTEXT, INID, INQSTRING, INCOMMENT, INOTHER } lstate = STARTING;
        static int	angleNesting = 0;
        int commentEnd = 0;
        lexeme = "";
        
        char ch;
        
        while( (in->get(ch)) && in->good() ) {
            if( ch == '\n' ) currLine++;
            
            switch( lstate ) {
                case STARTING:
                    LexState nextState;
                    
                    if( ch == '<' ) {
                        // check to see if this might be a comment
                        if( in->peek() != '!' ) {
                            ++angleNesting;
                            lexeme = ch;
                            return LANGLE;
                        }
                        
                        // ok, at this point we know we have a < and the next character on input is a !
                        in->get(ch); // eat up the !
                        
                        if( in->peek() == '-' && in->get(ch) ) {
                            if( in->peek() == '-' && in->get(ch) ) {
                                lstate = INCOMMENT;
                                commentEnd = 0;
                                continue;
                            }
                            else {
                                // here for < ! dash not-dash
                                // need to push 2 things back...
                                in->putback('-');
                            }
                        }
                        // here for < ! not-dash
                        in->putback('!');
                        ++angleNesting;
                        lexeme = '<';
                        return LANGLE;
                    }
                    else if( ch == '>' ) {
                        if( angleNesting )
                            --angleNesting;
                        lexeme = ch;
                        return RANGLE;
                    }
                    else if( ch == '/' ) {
                        if( angleNesting ) {
                            lexeme = ch;
                            return SLASH;
                        }
                        nextState = INTEXT;
                    }
                    else if( ch == '=' ) {
                        if( angleNesting ) {
                            lexeme = ch;
                            return EQ;
                        }
                        nextState = INTEXT;
                    }
                    else if( ch == '\"' ) {
                        nextState = INQSTRING;
                    }
                    else if( angleNesting ) {
                        if( isspace(ch) )
                            continue;
                        else if( isalpha(ch) )
                            nextState = INID;
                        else
                            nextState = INOTHER;
                    }
                    else {
                        nextState = INTEXT;
                    }
                    lexeme = ch;
                    lstate = nextState;
                    break;
                    
                case INCOMMENT:
                    switch( commentEnd ) {
                        case 0:
                            if( ch == '-' )
                                commentEnd = 1;
                            break;
                            
                        case 1:
                            if( ch == '-' )
                                commentEnd = 2;
                            else
                                commentEnd = 0;
                            break;
                            
                        case 2:
                            if( ch == '>' )
                                lstate = STARTING;
                            else
                                commentEnd = 0;
                            break;
                    }
                    break;
                    
                case INTEXT:
                    if( ch == '<' ) {
                        in->putback(ch);
                        return TEXT;
                    }
                    lexeme += ch;
                    break;
                    
                case INID:
                    if( !isalpha(ch) && !isdigit(ch) && ch != '.' && ch != '-' ) {
                        in->putback(ch);
                        return ID;
                    }
                    lexeme += ch;
                    break;
                    
                case INQSTRING:
                    lexeme += ch;
                    if( ch == '\n' ) {
                        // in the middle of a quoted string, end of line? that's bad...
                        return OTHER;
                    }
                    if( ch == '\"' ) {
                        return QSTRING;
                    }
                    break;
                    
                case INOTHER:
                    if( isspace(ch) || ch == '<' || ch == '>' || ch == '/' || ch == '"' ) {
                        in->putback(ch);
                        return OTHER;
                    }
                    lexeme += ch;
                    break;
            }
        }
        
        // what about end of file?
        if( lexeme.length() ) {
            switch( lstate ) {
                case INTEXT:
                    return TEXT;
                case INID:
                    return ID;
                case INCOMMENT:
                    break;
                default:
                    return OTHER;
            }
        }
        
        return END;
    }
    
}
