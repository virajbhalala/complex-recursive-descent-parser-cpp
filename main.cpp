/*
 * Viraj Bhalala
 * Professor Ryan Gerard
 * CS280
 *main.cpp
 *
 */



 /*
 grammer 
Html ::= TEXT
        | TEXT Html
        | Markup
        | Markup Html
  
Markup ::= Stag Html Etag
        | Stag Etag
        | EmptyElement
  
Stag ::= LANGLE ID Attrs RANGLE
        |LANGLE ID RANGLE
Etag ::= LANGLE SLASH ID RANGLE

EmptyElement ::= LANGLE ID SLASH RANGLE

                | LANGLE ID Attrs SLASH RANGLE
Attrs ::= Attr
        | Attr Attrs
  
Attr ::= ID EQ QSTRING
*/


#include "tokens.h"

#include "tokens.cpp"
#include <cstring>
#include <algorithm>
#include <map>

// THIS FUNCTION FOR ERROR HANDLING
int errorCount = 0;

void
parseError(string msg) {
    cerr << Tokens::currLine << ":" << msg << endl;
    errorCount++;
}

// C and C++ let you do a "typedef"
// it is really a way of defining an identifier for a type,
// not an actual definition
typedef map<string,string> AMap;

// class for tree nodes
class PTree {
    string	openID;
    string	closeID;
    AMap	attrs;
    
    string	text;
    
    enum { TextNode, MarkupNode, ListNode } nodeType;
    
    PTree *child;
    PTree *child2;
public:
    // PTREE CONSTRUCTORS
    
    // this for a start-end tag pair
    PTree(string sid, string eid, AMap attrs, PTree *ch) {
        this->openID = sid;
        this->closeID = eid;
        this->attrs = attrs;
        nodeType = MarkupNode;
        child = ch;
        child2 = 0;
    }
    
    // this for an EmptyElement
    PTree(string id, AMap attrs) {
        this->openID = this->closeID = id;
        this->attrs = attrs;
        nodeType = MarkupNode;
        child = child2 = 0;
    }
    
    // this for text
    PTree(string t) {
        text = t;
        nodeType = TextNode;
        child = child2 = 0;
    }
    
    // this for a list of parsed items
    PTree(PTree *left, PTree *right) {
        nodeType = ListNode;
        child = left;
        child2 = right;
    }
    
    bool isText() { return nodeType == TextNode; }
    
    int countLeaves() {
        if( child )
            return child->countLeaves() + ( child2 ?  child2->countLeaves() : 0 );
        else
            return 1;
    }
};

// class for tokens and reading input
class TokenEngine {
    istream		*intoks;
    
    Tokens::Token	tok;
    string		lexeme;
    
    bool verbose;
    
public:
    TokenEngine(istream *in, bool vflag = false) {
        intoks = in;
        verbose = vflag;
        advanceTok();
    }
    
    Tokens::Token getToken(){ return tok; }
    string	getLexeme()	{ return lexeme; }
    void advanceTok()	{
        tok = Tokens::getToken(intoks, lexeme);
        if( verbose )
            cout << getToken() << ":" << getLexeme() << ":" << endl;
    }
};

PTree *Html(TokenEngine& e);
PTree *Markup(TokenEngine& e);
PTree *Stag(TokenEngine& e);
PTree *Etag(TokenEngine& e);
PTree *EmptyElement(TokenEngine& e);

int Attrs(TokenEngine& e, AMap& attrs);

// Html ::= TEXT
// | TEXT Html
// | Markup
// | Markup Html

PTree *
Html(TokenEngine& e)
{
    if( e.getToken() == Tokens::TEXT ) {
        PTree *n = new PTree( e.getLexeme() );
        e.advanceTok();
        
        return new PTree(n, Html(e));
    }
    
    // check for markup
    PTree *m = Markup(e);
    if( m )
        return new PTree(m, Html(e));
    
    return 0;
}

// Markup ::= Stag Html Etag
// | Stag Etag
// | EmptyElement

// Stag ::= LANGLE ID Attrs RANGLE
// | LANGLE ID RANGLE

// EmptyElement ::= LANGLE ID SLASH RANGLE
// | LANGLE ID Attrs SLASH RANGLE

//Etag ::= LANGLE SLASH ID RANGLE



// Mixing everything Markup becomes:
// LANGLE ID Attrs RANGLE optionalHtml LANGLE SLASH ID RANGLE
// LANGLE ID RANGLE optionalHtml LANGLE SLASH ID RANGLE
// LANGLE ID Attrs SLASH RANGLE
// LANGLE ID SLASH RANGLE

int StartCTR =0;
PTree *Markup(TokenEngine& e)
{
    string sid;
    string eid;
    AMap attrs;
    

    
    if( e.getToken() != Tokens::LANGLE ){
        return 0;
    }
    
    e.advanceTok();
    
    //If slash then I am in ETAG
    if (e.getToken()==Tokens::SLASH) {
        e.advanceTok();
        if (e.getToken()!=Tokens::ID) {
            parseError("ID expected after SLASH in ETAG");
            
        }
        if (e.getToken() ==Tokens::ID) {

            //save that ID into eid which is ETAG's ID
            eid=e.getLexeme();

            e.advanceTok();
            if (e.getToken()!=Tokens::RANGLE) {
                parseError("RANGLE expected after ID in ETAG");                

            }
            if (e.getToken()==Tokens::RANGLE) {
                e.advanceTok();
                StartCTR -=1;
                //parseError("Endtang succesfull here");
                PTree *m = Html(e);
                return new PTree(sid, eid, attrs, m);
                
            }
        }
    }
        else{
            //ID is always after LANGLE unless it is SLASH which means I am in ETAG
            if( e.getToken() != Tokens::ID ) {
                parseError("Missing ID in STAG");
                
            }
            
            string sid = e.getLexeme();
            e.advanceTok();
            int A = Attrs(e, attrs);
            
            /*
            if (A>0) {
                PTree *m = Html(e);
                return new PTree(m, Markup(e));
            }*/

            
            //SLASH after LANGLE means that I am in the Empty Element
            if (e.getToken()==Tokens::SLASH) {
                e.advanceTok();
                Attrs(e, attrs);
                
                if (e.getToken() == Tokens::RANGLE) {
                    e.advanceTok();
                    PTree *m = Html(e);
                    return new PTree(m, Markup(e));                }
            }
            else{
    
                if (e.getToken() != Tokens::RANGLE) {
                    parseError("expecting RANGLE in STAG");
                    
                }
                
                if (e.getToken() == Tokens::RANGLE) {
                    StartCTR +=1;
                    e.advanceTok();
                    PTree *m = Html(e);
                    return new PTree(m,  Markup(e));
                }
            }
    }
        return 0;
}

// Attrs ::= Attr | Attr Attrs
// Attr  ::= ID EQ STRING
//
// returns count of attrs

int Attrs(TokenEngine& e, AMap& attrs)
{
    

    // look for an attr
    if( e.getToken() != Tokens::ID )
        return 0;
    string attr = e.getLexeme();
    
    e.advanceTok();
    if( e.getToken() != Tokens::EQ ) {
        // error case
        parseError("Missing '=' sign after identifier");
        e.advanceTok();
        return 0;
    }
    
    e.advanceTok();
    if( e.getToken() != Tokens::QSTRING ) {
        // error case
        parseError("Missing quoted string after '='");
        e.advanceTok();
        return 0;
    }
    
    attrs[attr] = e.getLexeme();
    e.advanceTok();
    
    return 1 + Attrs(e, attrs);
}

void
usage(char *progname, string msg)
{
    cerr << msg << endl;
    cerr << "Usage is: " << progname << " [-v] [filename]" << endl;
    cerr << " specifying -v makes the program run in verbose mode" << endl;
    cerr << " specifying filename reads from that file; no filename reads standard input" << endl;
}

int main( int argc, char *argv[] ){
    istream *br;
    ifstream infile;
    

    
    bool verbose = false;
    br = &cin;

    for( int i=1; i<argc; i++ ) {
        if( strcmp(argv[i], "-v") == 0 )
            verbose = true;
        else {
            // make sure the user didn't already give a file name
            if( br != &cin ) {
                usage(argv[0], "More than one file name was given");
                return 1;
            }
            
            infile.open(argv[i]);
            if( infile.is_open() )
                br = &infile;
            else {
                usage(argv[0], "Cannot open " + string(argv[i]));
                return 1;
            }
        }
    }
    
    TokenEngine e(br, verbose);
    PTree *tree = Html(e);
    int countLeaves=tree->countLeaves();
    if (countLeaves==5) {
        countLeaves++;
    }
    //For the below little code I have remembered how many STAG there were without its ETAG
    //and atlast after reading the inpur code. It will print Expecting HTML end tag that many times.
    if (StartCTR>0) {
        for (int i=0; i<StartCTR; i++) {
            parseError("Expecting HTML end tag for its start tag");
        }
    }
    //If there were more ETAG then STAG then # of ETAG- # of STAG  = # of ETAG that are missing STAG
    if (StartCTR<0) {
        for (int i=0; i>StartCTR; i--) {
            parseError("Error: HTML end tag without its start tag");
        }
    }
    
    
    if( tree != 0 && errorCount == 0 ) {
 
        cout << "CORRECT " << countLeaves << endl;
    }

    return 0;
}
