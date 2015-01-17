/*
 Viraj Bhalala
 CS 280 - 003
 * main.cpp
 *
 */


#include "tokens.h"
#include "tokens.cpp"
#include <sstream>
#include "vector"
#include <cstring>
#include <algorithm>
#include <map>
#include <math.h>

int errorCount = 0;

void
parseError(string msg) {
    cerr << Tokens::currLine << ":" << msg << endl;
    errorCount++;
}


typedef map<string,string> AMap;

// class for tree nodes
class PTree {
    string	openID;
    string	closeID;
    AMap	attrs;
    
    string	text;
    
    enum { TextNode, StagNode, EtagNode, EmptyElementNode, MarkupNode, ListNode } nodeType;
    
    PTree *child;
    PTree *child2;
public:
    // PTREE CONSTRUCTORS
    PTree(string sid, string eid, AMap attrs) {
        this->openID = sid;
        this->closeID = eid;
        this->attrs = attrs;
        nodeType = StagNode;
        child = child2 = 0;
    }
    
    PTree(string id, AMap attrs) {
        this->openID = this->closeID = id;
        this->attrs = attrs;
        nodeType = EmptyElementNode;
        child = child2 = 0;
    }
    
    PTree(string id, PTree *ch) {
        this->openID = this->closeID = id;
        this->attrs = attrs;
        nodeType = EtagNode;
        child = child2 = 0;
    }
    
    PTree(string t) {
        text = t;
        nodeType = TextNode;
        child = child2 = 0;
    }
    
    PTree(PTree *left, PTree *right = 0) {
        nodeType = ListNode;
        child = left;
        child2 = right;
    }
    
    PTree(PTree *start, PTree *body, PTree *end) {
        openID = start->openID;
        attrs = start->attrs;
        if( end )
            closeID = end->closeID;
        nodeType = MarkupNode;
        child = body;
        child2 = 0;
        
        delete start;
        if( end )
            delete end;
    }
    
    bool isStag() { return nodeType == StagNode; }
    bool isEtag() { return nodeType == EtagNode; }
    bool isEmptyElement() { return nodeType == EmptyElementNode; }
    bool isMarkup() { return nodeType == MarkupNode; }
    
    string getStartTag() { return openID; }
    string getText() { return text; }
    
    // if the right child is actually an etag, pull it out and return it
    PTree *extractEtag() {
        if( nodeType == ListNode ) {
            if( child2 ) {
                if( child2->isEtag() ) {
                    PTree *rval = child2;
                    child2 = 0;
                    return rval;
                }
                else
                    return child2->extractEtag();
            }
        }
        return 0;
    }
    
    void traverse(int level=0) {
        indent( level );
        switch(nodeType) {
            case MarkupNode:
                cout << "Markup: begin,end == " << openID << "," << closeID << endl;
                break;
                
            case TextNode:
                cout << "Text:" << text << ":" << endl;
                break;
                
            case StagNode:
                cout << "Start tag:" << openID <<endl;
                printattrs(level);
                break;
                
            case EtagNode:
                cout << "End tag:" << openID <<endl;
                break;
                
            case EmptyElementNode:
                cout << "EmptyElement:" << openID << endl;
                printattrs(level);
                break;
                
            case ListNode:
                cout << "List:" << endl;
                break;
        }
        if( child ) {
            indent( level );
            cout << "child 1:";
            child->traverse(level+1);
        }
        if( child2 ) {
            indent( level );
            cout << "child 2:";
            child2->traverse(level+1);
        }
    }
    
    int countLeaves() {
        if( child == 0 && child2 == 0 )
            return 1;
        
        return (child ? child->countLeaves() : 0)  + (child2 ?  child2->countLeaves() : 0);
    }
    
private:
    void indent(int n) {
        for( int i=0; i<n; i++ ) cout << "  ";
        cout << n << ":";
    }
    
    void printattrs(int n) {
        for( AMap::iterator it = attrs.begin(); it != attrs.end(); it++ ) {
            indent( n );
            cout << " " << it->first <<"=" <<it->second << endl;
        }
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
        if( isVerbose() )
            cout << getToken() << ":" << getLexeme() << ":" << endl;
    }
    
    bool isVerbose() { return verbose; }
};

PTree *Html(TokenEngine& e);
PTree *Markup(TokenEngine& e);
PTree *Stag(TokenEngine& e);
PTree *HtmlTag(TokenEngine& e);

int Attrs(TokenEngine& e, AMap& attrs);

PTree *MarkupStarted(TokenEngine& e, PTree *stag);

// Html ::= TEXT
// | TEXT Html
// | Markup
// | Markup Html


bool inH= false;
string HText;

PTree *
Html(TokenEngine& e)
{
    if( e.isVerbose() )
        cerr << "Entered Html()" << endl;
    
    if( e.getToken() == Tokens::TEXT ) {
        if (inH==true) {
            HText=e.getLexeme();
            inH=false;
        }
        
        PTree *n = new PTree( e.getLexeme() );
        e.advanceTok();
        
        if( e.isVerbose() )
            cerr << "recognized TEXT " << n->getText().substr(0,10) << endl;
        
        PTree *htmlfollow = Html(e);
        
        if( e.isVerbose() )
            cerr << "returning list of TEXT " << n->getText().substr(0,10) << " and html" << endl;
        
        return new PTree(n, htmlfollow);
    }
    
    // check for markup
    PTree *m = Markup(e);
    if( m ) {
        if( m->isMarkup() || m->isEmptyElement() ) {
            if( e.isVerbose() )
                cerr << "recognized complete Markup, returning list of markup and html" << endl;
            return new PTree(m, Html(e));
        }
        else {
            if( e.isVerbose() )
                cerr << "it wasn't markup, returning it" << endl;
            return m;
        }
    }
    
    if( e.isVerbose() )
        cerr << "returning null from html" << endl;
    
    return 0;
}

// Markup ::= Stag Html Etag
// 	| Stag Etag
// 	| EmptyElement
//



map<string,int> tracker;
map<string,int> counters;
map<string,int> htag;

string endT;
string startT;

PTree *Markup(TokenEngine& e)
{

    if( e.isVerbose() )
        cerr << "Entered Markup()" << endl;
    
    PTree *firstTag = HtmlTag(e);
    
    if( !firstTag ) {
        return 0;
    }
    //empty element then just return it
    if( firstTag->isEmptyElement() )
        return firstTag;
    
    if( !firstTag->isStag() ) {
        if( e.isVerbose() )
            cerr << "first markup element was not an stag" << endl;
        return firstTag;
    }

    if( e.isVerbose() ){
        cerr << "Found stag" << endl;
    }

    PTree *html = Html(e);


    if( e.isVerbose() ){
        cerr << "Returned from html" << endl;
    }
    PTree *etag;
    
    if( html->isEtag() ) {
        etag = html;
        html = 0;
    }
    else {
        etag = html->extractEtag();
        
        if( etag == 0 )
            etag = HtmlTag(e);
    }
    
    if( e.isVerbose() )
        cerr << "at end of markup" << endl;
    
    if( !etag ) {
        parseError("Expecting an HTML end tag for start tag " + firstTag->getStartTag());
        return 0;
    }
    else if( etag->isStag() ) {
        // nested...
        PTree *nest = MarkupStarted(e, etag);
        return nest;
    }

    else
        return new PTree(firstTag, html, etag);

}

PTree *MarkupStarted(TokenEngine& e, PTree *stag)
{

    PTree *html = Html(e);
    
    
    if( e.isVerbose() )
        cerr << "Returned from html in markupstarted" << endl;
    
    PTree *etag = HtmlTag(e);
    
    if( e.isVerbose() )
        cerr << "at end of markupstarted" << endl;
    
    if( !etag ) {
        parseError("Expecting an HTML end tag");
        return 0;
    }
    else if( etag->isStag() ) {
        // nested...
        PTree *nest = MarkupStarted(e, etag);
        return nest;

    }
    else
        return new PTree(stag, html, etag);
}

// Stag ::= LANGLE ID Attrs RANGLE
// 		| LANGLE ID RANGLE
// EmptyElement ::= LANGLE ID SLASH RANGLE
// 		| LANGLE ID Attrs SLASH RANGLE
// Etag ::= LANGLE SLASH ID RANGLE
//

int level =0;
int temp=0;
int Hlevel=0;
double HNum=0.0;
vector<string> HTextList;
vector<int> HLevelList;
int lowest=1;

PTree *HtmlTag(TokenEngine& e)
{
    bool isEmpty = false;
    bool isEnd = false;
    string theTag;
    AMap attrs;
    
    if( e.isVerbose() )
        cerr << "Entering HtmlTag()" << endl;
    
    string tag;
    
    if( e.getToken() != Tokens::LANGLE ) {
        return 0;
    }
    
    e.advanceTok();
    if( e.getToken() == Tokens::SLASH ) {
        isEnd = true;
        e.advanceTok();
    }
    else {
        if( e.getToken() != Tokens::ID ) {
            
            parseError("Expecting identifier after '<'");
            e.advanceTok();
            return 0;
        }
        //counters
        startT=e.getLexeme();
        theTag = e.getLexeme();
        e.advanceTok();
        
        //counter map is H tag counter
        // tracker map keeps track if Start tag and end tag does not match
        //in STAG
        if(e.getToken() != Tokens::SLASH){
            //counters
            if (counters.find(startT)!=counters.end()) {
                counters[startT]+=1;
            }
            else{
                counters[startT]=1;
            }
            //tracker
            if (tracker.find(startT)!=tracker.end()) {
                tracker[startT]+=1;
            }
            else{
                tracker[startT]=1;
            }
            temp++;
            if (temp>level) {
                level=temp;
            }
            // htag map
            if (startT.substr(0,1)=="h" || startT.substr(0,1)=="H" ) {
                inH=true;
            }
            
        }

    }
    
    if( isEnd ) {
        if( e.getToken() != Tokens::ID ) {
            parseError("Expecting identifier after '/'");
            e.advanceTok();
            return 0;
        }
        
        
        //in ETAG
        endT = e.getLexeme();
        if (tracker.find(endT)!=tracker.end()) {
            tracker[endT]-=1;
        }
        else
            tracker[endT]=-1;
        
        temp--;
        
        // htag map <TEXT, H levels>
        // Htext is the current TEXT
        //HtextList is text array HLevelList is level array
        //Hnum is number after h in header tag
        //lowest is lowest Hnum
        
        
        if (endT.substr(0,1)=="h" || endT.substr(0,1)=="H" ) {
            inH=true;
            stringstream(endT.substr(1,2)) >> HNum;
            if (HNum>=lowest) {
                Hlevel= Hlevel + int(pow(10.0, (6.0-HNum)));
                HLevelList.push_back(Hlevel);
                lowest=HNum;
            }
            else{
                lowest=HNum;
                Hlevel= ((Hlevel/(int(pow(10.0, (6.0-HNum)))))*(int(pow(10.0, (6.0-HNum))))) +int(pow(10.0, (6.0-HNum)));
                HLevelList.push_back(Hlevel);
            }
            
            
            //storint TEXT into vector HtextList
            
            HTextList.push_back(HText);
            inH=false;
        }
        
        theTag = e.getLexeme();
        e.advanceTok();
    }
    else {
        Attrs(e, attrs);
    }
    
    // finish up?
    if( e.getToken() == Tokens::SLASH ) {
        isEmpty = true;
        e.advanceTok();
    }
    
    if( e.getToken() != Tokens::RANGLE ) {
        parseError("Expecting '>'");
        e.advanceTok();
        return 0;
    }
    
    e.advanceTok();
    
    PTree *tagnode;
    if( isEnd )
        tagnode = new PTree(theTag, 0);
    else if( isEmpty )
        tagnode = new PTree(theTag, attrs);
    else
        tagnode = new PTree(theTag, theTag, attrs);
    
    return tagnode;
}

// Attrs ::= Attr | Attr Attrs
// Attr  ::= ID EQ STRING
//
// returns count of attrs
int
Attrs(TokenEngine& e, AMap& attrs)
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
string deleteZeroes(int);
string deleteZeroes(int x) {
    while(x%10==0)
        x /= 10;
    stringstream ss;
    ss << x%10;
    x /= 10;
    while(x) {
        ss << "." << x%10;
        x/=10;
    }
    string result = ss.str();
    reverse(result.begin(), result.end());
    return result;
}


int
main( int argc, char *argv[] )
{
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

    /*
     plan for STAG and ETAG match?:
     find lone start and endtag
     make vector for each
     then print out
     */
    
    vector<string> StartList;
    vector<string> EndList;

    //
    bool MatchError=false;
    if( tree != 0 && errorCount == 0 ) {
        for (map<string, int>:: iterator i =tracker.begin(); i !=tracker.end(); i++) {
            while (i->second >0){
                //parseError("Expecting HTML end tag for its start tag " + i->first);
                StartList.push_back(i->first);
                tracker[i->first]--;
                MatchError=true;
            }
            while (i->second<0) {
                EndList.push_back(i->first);
                tracker[i->first]++;
                MatchError=true;

            }
        
        }
        for (int j =0; j<StartList.size(); j++) {
            cout<< "start tag " << StartList.at(j) << " does not match end tag " << EndList.at(j) << "\n"<<endl;
        }
    }

    if( tree != 0 && errorCount == 0 && MatchError==false) {
        cout<< "Maximum nesting depth is " << level << " \n";

        for (map<string, int>:: iterator i =counters.begin(); i !=counters.end(); i++) {
            cout<< i->first << ": "<< i->second << "\n";
        }
        cout << "============================ \n";
        for (int i =0; i<HTextList.size(); i++) {
            string s;
            s=deleteZeroes(HLevelList.at(i));
            cout<< s<< ": " <<HTextList.at(i) << "\n";
        }
    }
    if( tree && verbose )
        tree->traverse();
    return 0;
}
