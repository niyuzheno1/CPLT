#include <tree_sitter\api.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <set>
#include <map>
using namespace std;
const int c_dbg = 1;
// you can navigate using bread crumb

namespace tree_sitter_util {
    template<typename T>
    set<T> toSet(vector<T> & t){
        set<T> s;
        for(auto & i : t)
            s.insert(i);
        return s;
    }
    template<typename T>
    set<T> mergeSet(set<T> & s1, set<T> & s2){
        set<T> s;
        for(auto & i : s1)
            s.insert(i);
        for(auto & i : s2)
            s.insert(i);
        return s;
    }
    template<typename T>
    string toString(vector<T> & t){
        stringstream ss;
        ss << "[";
        for(int i = 0; i < t.size(); i++){
            ss << t[i];
            if(i != t.size() - 1)
                ss << ", ";
        }
        ss << "]";
        return ss.str();
    }
}

using namespace tree_sitter_util;

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
        TemporaryBuffer() : _buffer(), _currentLine(), newline(true), firstchar(true) {}

        string _buffer;
        string _currentLine;
        bool newline = true;
        bool firstchar = true;
        void write(string & tmp, Scope & scope){
            for(int i = 0; i < tmp.length(); ++i){
                // if tmp[i] is space
                if(firstchar){
                    _buffer += string(scope.level, '\t');
                    firstchar = false;
                }
                if(tmp[i] == ' ' || tmp[i] == '\t'){
                    if(!newline)  _currentLine += tmp[i];
                }else if(tmp[i] == '\n'){
                    //add level \t 
                    _buffer += _currentLine;
                    _buffer += '\n';
                    _currentLine = "";
                    newline = true;
                    firstchar = true;
                }else{
                    _currentLine += tmp[i];
                    newline = false;
                }
            } 
        }
        void writeMandatory(string & tmp, Scope & scope){
            string ept = "\n";
            write(ept, scope);
            _buffer += tmp;
            write(ept, scope);
        }
        void show(){
            cout << _buffer<<endl;
        }
        string getBuffer(){
            if(_currentLine != ""){
                _buffer += _currentLine;            
            }
            return _buffer;
        }
    };
    
    class ReturnValue{
    public:
        ReturnValue() : tb(){
    
        }
        TemporaryBuffer tb;
    };
}

namespace tree_sitter_simplified_proc {
    using namespace tree_sitter_data_structures;
    inline const char * type(TSNode & x){
        return ts_node_type(x);
    }
    inline bool is_type(TSNode & x, const char * tp){
        return strcmp(tp, type(x)) == 0;
    }
    inline bool is_type_of(TSNode & x, set<string> & tp){
        return tp.find(type(x)) != tp.end();
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

string debug_buffer; // for c_dbg




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

//function prototype void parse(TSNode & node, Scope & scope, ReturnValue & retval)

using parse_proc = function<void(TSNode&, Scope & , ReturnValue &)>;
using parse_proc_ref = parse_proc &;
using const_parse_proc_ref = const parse_proc &;
class statement_handler;
map<string, statement_handler*> statement_handlers;

class MemoryManager{
public:
    template<typename T>
    T* newInstance(){
        return new T();
    }
} memoryManager;

class statement_handler{
public:
    void registerHandler(vector<string> & x){
        for(auto i : x){
            statement_handlers[i] = this;
        }
    }
    virtual vector<string> getAllTypes() = 0;
    virtual void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) = 0;
};

class compound_statement : public statement_handler{
public:
    compound_statement(){
        this->registerHandler(getAllTypes());
    }
    vector<string> getAllTypes() override{
        return {"compound_statement", "declaration_list", "field_declaration_list"};
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        int curp = start(node);
        repb(child, node)   
            if(is_type(child, "}")) scope.level--;
            retVal.tb.write(substring(scope, curp, start(child)), scope);
            parse(child, scope, retVal);
            curp = end(child);
            if(is_type(child, "{")) scope.level++;
        repe
        if(curp != end(node)){
            retVal.tb.write( substring(scope, curp, end(node)) , scope);
        }
    }

}compound_like_statement_handler;

class repeat_statement : public statement_handler{
public:
    repeat_statement(){
        this->registerHandler(getAllTypes());
    }
    vector<string> getAllTypes() override{
        return {"repeat_statement"};
    }
    vector<string> getRepeatKeyTypes(){
        return {  "rep_perm", "rep_scomb",  "rep_mcomb",  "rep_sarr", "rep_marr",  "rep", "rrep",  "REP",  "RREP", "rep_dist" };
    }
    vector<string> getImportantToken(){
        return {"(", ")", "[", "]", ","};
    }

    void handleTemplate(string & head, 
                        string  & templateFormulation,
                        vector<string> & positionArgs,
                        string & body,
                        Scope & scope,
                        ReturnValue & retVal){
    scope.level ++;
    // process the head
    stringstream ss;
    ss << endl;
    if(head == "rep" && templateFormulation == "(,,)"){
        
        // todo if positionArgs[0] is already within the scope
        ss << "for(int " << positionArgs[0] << " = ("<< positionArgs[1] << "); "<< positionArgs[0] << " < (" << positionArgs[2] << "); "<< positionArgs[0] <<"++)";
    }

    retVal.tb.write(ss.str(), scope);
     
    // process the body if we keep the body intact
    retVal.tb.writeMandatory(body, scope);
    
    // process the tail
    scope.level --;
    }

    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        int curp = start(node);
        set<string> importantTypes = toSet(getImportantToken());
        set<string> originatorTypes = toSet(getRepeatKeyTypes());
        string templateFormulation = "";
        string head = "";
        vector<string> positionArgs;
        bool encountered = false;
        string rest;
        repb(child, node)   
            if(is_type(child, ")")) encountered = true;
            if(is_type_of(child, importantTypes)){
                templateFormulation += type(child);
                return;
            }
            if(is_type_of(child, originatorTypes)){
                head = type(child);
                return;
            }
            if(!encountered){
                ReturnValue tmp;
                int curlevel = scope.level;
                scope.level = 0;
                parse(child, scope, tmp);
                scope.level = curlevel;
                curp = end(child);
                positionArgs.push_back(tmp.tb.getBuffer());
            }else{
                ReturnValue tmp;
                scope.level++;
                parse(child, scope, tmp);
                scope.level--;
                curp = end(child);
                rest = tmp.tb.getBuffer();
            }
        repe
        if(c_dbg){
            debug_buffer += "repeat_statemen=" + head + ":" + templateFormulation + "\n";
            debug_buffer += "arguments=" + toString(positionArgs) + "\n";
        } 
        handleTemplate(head, templateFormulation, positionArgs, rest, scope, retVal);
        if(curp != end(node)){
            retVal.tb.write( substring(scope, curp, end(node)) , scope);
        }
    }
}repeat_statement_handler;

map<string, string> allKInds;
void parse(TSNode & node, Scope & scope, ReturnValue & retval){
    string nodeType = type(node);
    string content = substring(scope, start(node), end(node));
    if(statement_handlers.count(nodeType) != 0){
        statement_handlers[nodeType]->handle(node, scope, retval, parse);
        return;
    }
    if(c_dbg){ allKInds[nodeType] = content;} 
    int curp = start(node);
    repb(child, node)
        retval.tb.write(substring(scope, curp, start(child)), scope);
        parse(child, scope, retval);
        curp = end(child);
    repe
    if(curp != end(node)){ retval.tb.write( substring(scope, curp, end(node)) , scope); }
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
    //print all kinds
    if(c_dbg){
        for(auto & x : allKInds){
            debug_buffer += "node type:"+ x.first + "\n";
            debug_buffer += x.second + "\n";
            debug_buffer += "\n";
        }
    }
    if(c_dbg){ 
        cout << debug_buffer;
    }

    return 0;
}