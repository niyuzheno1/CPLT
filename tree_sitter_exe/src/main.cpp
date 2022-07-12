#include <tree_sitter\api.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>
using namespace std;
// you can navigate using bread crumb

namespace tree_sitter_data_structures {
    class Scope{
    public:
        string * buffer;
        TSParser * tsParser;
        bool isInit;
        bool isTopLevel;
        int level = 0;
    };

    class TemporaryBuffer{
    public:
        string _buffer;
        string _currentLine;
        bool newline = true;
        void write(string & tmp, Scope & scope){
            for(int i = 0; i < tmp.length(); ++i){
                // if tmp[i] is space
                if(tmp[i] == ' ' || tmp[i] == '\t'){
                    if(!newline)  _currentLine += tmp[i];
                }else if(tmp[i] == '\n'){
                    //add level \t 
                    _buffer += string(scope.level, '\t');
                    _buffer += _currentLine;
                    _buffer += '\n';
                    _currentLine = "";
                    newline = true;
                }else{
                    _currentLine += tmp[i];
                    newline = false;
                }
            } 
        }
        void show(){
            cout << _buffer<<endl;
        }
    };
    
    class ReturnValue{
    public:
        TemporaryBuffer tb;
    };
}

namespace tree_sitter_simplified_proc {
    using namespace tree_sitter_data_structures;
    inline const char * type(TSNode & x){
        return ts_node_type(x);
    }

    // cc = child count
    inline int cc(TSNode & x) {
        return  ts_node_child_count(x);
    }

    inline TSNode ch(TSNode & x, int i) {
        return ts_node_child(x, i);
    }


    void getch(TSNode & x, function<void(TSNode)> y){
    for(int i=0;i<cc(x);i++){
        y(ch(x,i));
    }
}
    inline int start(TSNode & x) {
        return ts_node_start_byte(x);
    }

    inline int end(TSNode & x) {
        return ts_node_end_byte(x);
    }
    string substring(Scope & scope, int st, int ed){
        return scope.buffer->substr(st, ed-st);
    }

    string substring(Scope & scope, TSNode x){
        return substring(scope, start(x), end(x));
    }
}

using namespace  tree_sitter_simplified_proc;


extern "C" {
     TSLanguage* tree_sitter_cpp();
}


#define repb(i, x) getch(x, [&](TSNode i){ string p_content = substring(scope, x);
#define repe });




void getParsedNode(TSNode & root /*Out*/, Scope & scope /*In*/ ){
    string& buffer = *scope.buffer;
    TSParser * tsParser = scope.tsParser;
    TSTree * tree = ts_parser_parse_string(
        tsParser,
        NULL,
        buffer.c_str(),
		buffer.size()
    );
	root = ts_tree_root_node(tree);
}


void parse(TSNode & node, Scope & scope, ReturnValue & retval){
    string nodeType = type(node);
    string content = substring(scope, start(node), end(node));
    int curp = start(node);
    repb(child, node)
        retval.tb.write(substring(scope, curp, start(child)), scope);
        parse(child, scope, retval);
        curp = end(child);
    repe
    if(curp != end(node)){
        retval.tb.write( substring(scope, curp, end(node)) , scope);
    }
}





int main(){
    string buffer;
    //get from iostream 
    while(!cin.eof()){
        string tmp = "";    
        getline(cin, tmp);
        buffer += tmp + "\n";
    }
    TSParser * tsParser = ts_parser_new();
    ts_parser_set_language(tsParser, tree_sitter_cpp());
	
    TSNode root;
    Scope scope;
    scope.buffer = &buffer;
    scope.tsParser = tsParser;
    getParsedNode(root, scope);
    ReturnValue retval;
    parse(root, scope, retval);
    retval.tb.show();
    return 0;
}