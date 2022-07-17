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
#include <assert.h>
#include <clay.h>
using namespace std;
const int c_dbg = 1;
// you can navigate using bread crumb

str to_string(stringstream & x){
    #undef str 
    return x.str();
    #define str string
}

namespace tree_sitter_util {
    template<typename T>
    set<T> toSet(vector<T> & t){
        set<T> s;
        each(i, t) s.insert(i);
        return s;
    }
    template<typename T>
    set<T> mergeSet(set<T> & s1, set<T> & s2){
        set<T> s;
        each(i, s1) s.insert(i);
        each(i, s2) s.insert(i);
        return s;
    }
    template<typename T>
    str toString(vector<T> & t){
        stringstream ss;
        ss << "[";
        rep(i, 0, sz(t)){
            ss << t[i];
            if(i != sz(t) - 1)
                ss << ", ";
        }
        ss << "]";
        //fix later
        return to_string(ss);
    }
}

using namespace tree_sitter_util;

namespace tree_sitter_data_structures {
    class Scope{
    public:
        str * buffer;
        TSParser * tsParser;
        bool isInit;
        bool isTopLevel;
        int level = 0;
        str getUnusedVariableName(){
            // todo
            return "";
        }
    };

    class TemporaryBuffer{
    public:
        TemporaryBuffer() : _buffer(), _currentLine(), newline(true), firstchar(true) {}

        str _buffer;
        str _currentLine;
        bool newline = true;
        bool firstchar = true;
        void write(str & tmp, Scope & scope){
            rep(i, 0, tmp.length()){
                // if tmp[i] is space
                if(firstchar){
                    _buffer += str(scope.level, '\t');
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
        void writeMandatory(str & tmp, Scope & scope){
            str ept = "\n";
            write(ept, scope);
            _buffer += tmp;
            write(ept, scope);
        }
        void show(){
            cout << _buffer<<endl;
        }
        str getBuffer(){
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
    inline bool is_type_of(TSNode & x, set<str> & tp){
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
        rep(i, 0, cc(x)){
            y(ch(x,i));
        }
    }
    inline int start(TSNode & x) {
        return ts_node_start_byte(x);
    }

    inline int end(TSNode & x) {
        return ts_node_end_byte(x);
    }
    str substring(Scope & scope, int st, int ed){
        return scope.buffer->substr(st, ed-st);
    }

    str substring(Scope & scope, TSNode x){
        return substring(scope, start(x), end(x));
    }

    vs Explode(str s, str delim){
        vs ret;
        int curp = 0;
        int nextp = 0;
        while(nextp != -1){
            nextp = s.find(delim, curp);
            if(nextp != -1){
                ret.push_back(s.substr(curp, nextp - curp));
                curp = nextp + sz(delim);
            }else{
                ret.push_back(s.substr(curp));
            }
        }
        return ret;
    }
    str Implode(vs s, str delim){
        str ret;
        rep(i, 0, sz(s)){
            if(i != 0){
                ret += delim;
            }
            ret += s[i];
        }
        return ret;
    }
}
using namespace  tree_sitter_simplified_proc;
extern "C" {
     TSLanguage* tree_sitter_cpp();
}


#define repb(i, x) getch(x, [&](TSNode i){ str p_content = substring(scope, x);
#define repe });

str debug_buffer; // for c_dbg


void getParsedNode(TSNode & root /*Out*/, Scope & scope /*In*/ ){
    str & buffer = *scope.buffer;
    TSParser * tsParser = scope.tsParser;
    TSTree * tree = ts_parser_parse_string(
        tsParser,
        NULL,
        buffer.c_str(),
		sz(buffer)
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
    void registerHandler(vs & x){
        each(i, x) statement_handlers[i] = this;
    }
    virtual vs getAllTypes() = 0;
    virtual void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) = 0;
};
// template for statement handler
// class x_statement : public statement_handler{
// public:
//     x_statement(){
//         this->registerHandler(getAllTypes());
//     }
//     vs getAllTypes() override{
//         return {};
//     }
//     void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
//         int curp = start(node);
//         repb(child, node)   
//             retVal.tb.write(substring(scope, curp, start(child)), scope);
//             parse(child, scope, retVal);
//             curp = end(child);
//         repe
//         if(curp != end(node)){
//             retVal.tb.write( substring(scope, curp, end(node)) , scope);
//         }
//     }
// }x_statement_handler;

class compound_statement : public statement_handler{
public:
    compound_statement(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
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

class type_identifier : public statement_handler{
public:
    type_identifier(){
        replacingTypes["ll"] = "long long";
        replacingTypes["ull"] = "unsigned long long";
        replacingTypes["db"] = "long double";
        replacingTypes["str"] = "string";
        replacingTypes["i64"] = "long long";
        replacingTypes["u64"] = "unsigned long long";
        replacingTypes["i32"] = "int";
        replacingTypes["u32"] = "unsigned int";
        replacingTypes["vi"] = "vector<int>";
        replacingTypes["vb"] = "vector<bool>";
        replacingTypes["vl"] = "vector<long long>";
        replacingTypes["vd"] = "vector<long double>";
        replacingTypes["vs"] = "vector<string>";
        replacingTypes["vpi"] = "vector<pair<int, int>>";
        replacingTypes["vpl"] = "vector<pair<long long, long long>>";
        replacingTypes["vpd"] = "vector<pair<long double, long double>>";
        replacingTypes["pi"] = "pair<int, int>";
        replacingTypes["pl"] = "pair<long long, long long>";
        replacingTypes["pd"] = "pair<long double, long double>";
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return {"type_identifier"};
    }
    map<str, str> replacingTypes;

    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        int curp = start(node);
        str text = substring(scope, node);
        assert(cc(node) == 0);
        if(replacingTypes.count(text)){
            retVal.tb.write(replacingTypes[text], scope);
        }else{
            int vcnt = -1;
            rep(i, 0, sz(text)){
                if(text[i] == 'v'){
                    ++vcnt;
                }else{
                    break;
                }
            }
            //get substring after vcnt to end of text
            
            if(vcnt >= 0){
                str sub = text.substr(vcnt);
                if(sz(sub) > 0){
                    if(sub[0] == 'v' && replacingTypes.count(sub)){
                        stringstream ss;
                        //add vector before 
                        rep(j, 0, vcnt){
                            ss << "vector<";
                        }
                        ss << replacingTypes[sub];
                        //add vector after
                        rep(j, 0, vcnt){
                            ss << ">";
                        }
                        retVal.tb.write(to_string(ss), scope);
                        return;
                    }
                }
            }
            retVal.tb.write( text , scope);
        }
    }

}type_identifier_handler;

//field_identifier
class field_identifier : public statement_handler{
public:
    field_identifier(){
        this->registerHandler(getAllTypes());
        replacingTypes["f"] = "first";
        replacingTypes["s"] = "second";
        replacingTypes["rsz"] = "resize";    
        replacingTypes["ins"] = "insert";
        replacingTypes["pb"] = "push_back";
        replacingTypes["eb"] = "emplace_back";
        replacingTypes["ft"] = "front";
        replacingTypes["bk"] = "back";
        replacingTypes["tp"] = "top";
        replacingTypes["rbg"] = "rbegin";
        replacingTypes["red"] = "rend";
    }
    map<str, str> replacingTypes;
    vs getAllTypes() override{
        return { "field_identifier" };
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        assert(cc(node) == 0);
        str text = substring(scope, node);
        if(replacingTypes.count(text)){
            retVal.tb.write(replacingTypes[text], scope);
        }else{
            retVal.tb.write( text , scope);
        }
    }
}field_identifier_handler;

//call_expression
class call_expression : public statement_handler{
public:
    call_expression(){
        this->registerHandler(getAllTypes());
        replacingTypes["mp"] = "make_pair";
        replacingFieldTypes["sz"] = "(int)(args.size())";
        replacingTypes["bg"] = "begin";
        replacingTypes["ed"] = "end";
        replacingTypes["pct"] = "__builtin_popcount";
        replacingFieldTypes["all"] = "begin(args), end(args)";
        replacingFieldTypes["rall"] = "rbegin(args), rend(args)";
        replacingFieldTypes["bits"] = "(args == 0 ? 0 : 31-__builtin_clz(args))";
        replacingFieldTypes["p2"] = "(1<<(args))";
        replacingFieldTypes["msk2"] = "((1<<(args))-1)";
    }

    str toFun(str functionName, str args){
        string x = replacingFieldTypes[functionName];
        // replace "args" with args 
        return Implode(Explode(x, "args"), args);
    }

    map<str, str> replacingTypes;
    map<str, str> replacingFieldTypes;
    vs getAllTypes() override{
        return { "call_expression" };
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        int curp = start(node);
        bool sentenced = false;
        ReturnValue tmp;
        string funName = "";
        repb(child, node)
            string ctext = substring(scope, child);   
            if(is_type(child, "identifier")){
                if(replacingFieldTypes.count(ctext)){
                    funName = ctext;
                    sentenced = true;
                }
            }else{
                int level  = scope.level;
                scope.level = 0;
                if(sentenced)  parse(child, scope, tmp);
                scope.level = level;
            }
            curp = end(child);
        repe
        if(sentenced){
            string args = tmp.tb.getBuffer();
            retVal.tb.write(toFun(funName, args), scope);
            retVal.tb.write( substring(scope, curp, end(node)) , scope);
            return;
        }
        curp = start(node);
        repb(child, node)
            string ctext = substring(scope, child);   
            retVal.tb.write(substring(scope, curp, start(child)), scope);
            if(is_type(child, "identifier")){
                if(replacingTypes.count(ctext)){
                    retVal.tb.write(replacingTypes[ctext], scope);
                }else{
                    retVal.tb.write(ctext, scope);
                }
            }else{
                parse(child, scope, retVal);
            }
            curp = end(child);
        repe
        if(curp != end(node)){
            retVal.tb.write( substring(scope, curp, end(node)) , scope);
        }
    }
}call_expression_handler;

//identifier
class identifier : public statement_handler{
public:
    identifier(){
        this->registerHandler(getAllTypes());
        replacingTypes["int_inf"] = "1073709056";
        replacingTypes["ll_inf"] = "4611686016279904256LL";
        replacingTypes["double_inf"] = "1e150";
        replacingTypes["PI"] = "3.14159265359";
    }
    vs getAllTypes() override{
        return { "identifier" };
    }
    map<str, str> replacingTypes;
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        string text = substring(scope, node);
        assert(cc(node) == 0);
        if(replacingTypes.count(text)){
            retVal.tb.write(replacingTypes[text], scope);
        }else{
            retVal.tb.write( text , scope);
        }
    }
}identifier_handler;


class repeat_statement : public statement_handler{
public:
    repeat_statement(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return {"repeat_statement"};
    }
    vs getRepeatKeyTypes(){
        return {  "rep_perm", "rep_scomb",  "rep_mcomb",  "rep_sarr", "rep_marr",  "rep", "rrep",  "REP",  "RREP", "rep_dist", "each" };
    }
    vs getImportantToken(){
        return {"(", ")", "[", "]", ","};
    }

    void handleTemplate(str & head, 
                        str  & templateFormulation,
                        vs & positionArgs,
                        str & body,
                        Scope & scope,
                        ReturnValue & retVal){
    scope.level ++;
    // process the head
    stringstream ss;
    ss << endl;
    // todo if positionArgs[0] is already within the scope
    if(head == "rep" && templateFormulation == "()"){
        // todo if we have get unused variable name from scope
        str unusedName = scope.getUnusedVariableName();
        ss << "for(int " << unusedName << " = ("<<0 << "); "<<  unusedName << " < (" <<  positionArgs[0] << "); "<< unusedName <<"++)";
    }
    if(head == "rep" && templateFormulation == "(,)"){
        ss << "for(int " << positionArgs[0] << " = ("<<0 << "); "<< positionArgs[0] << " < (" <<  positionArgs[1] << "); "<< positionArgs[0] <<"++)";
    }
    if(head == "rep" && templateFormulation == "(,,)"){    
        ss << "for(int " << positionArgs[0] << " = ("<< positionArgs[1] << "); "<< positionArgs[0] << " < (" << positionArgs[2] << "); "<< positionArgs[0] <<"++)";
    }
    if (head == "rep" &&  templateFormulation == "(,,,)"){
        ss << "for(int " << positionArgs[0] << " = ("<< positionArgs[1] << "); "<< positionArgs[0] << " < (" << positionArgs[2] << "); "<< positionArgs[0] <<"+= (" << positionArgs[3] << "))";
    }
    if(head == "each" && templateFormulation == "(,)"){
        ss << "for(auto & " << positionArgs[0] << " : " << positionArgs[1] << ")";
    }
    retVal.tb.write(to_string(ss), scope);
    // process the body if we keep the body intact
    retVal.tb.writeMandatory(body, scope);
    
    // process the tail
    scope.level --;
    }

    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        int curp = start(node);
        set<str> importantTypes = toSet(getImportantToken());
        set<str> originatorTypes = toSet(getRepeatKeyTypes());
        str templateFormulation = "";
        str head = "";
        vs positionArgs;
        bool encountered = false;
        str rest;
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
        // if(c_dbg){
        //     debug_buffer += "repeat_statemen=" + head + ":" + templateFormulation + "\n";
        //     debug_buffer += "arguments=" + toString(positionArgs) + "\n";
        // } 
        handleTemplate(head, templateFormulation, positionArgs, rest, scope, retVal);
        if(curp != end(node)){
            retVal.tb.write( substring(scope, curp, end(node)) , scope);
        }
    }
}repeat_statement_handler;

map<str, str> allKinds;
void parse(TSNode & node, Scope & scope, ReturnValue & retval){
    str nodeType = type(node);
    str content = substring(scope, start(node), end(node));
    if(statement_handlers.count(nodeType) != 0){
        statement_handlers[nodeType]->handle(node, scope, retval, parse);
        return;
    }
    if(c_dbg){ allKinds[nodeType] = content;} 
    int curp = start(node);
    repb(child, node)
        retval.tb.write(substring(scope, curp, start(child)), scope);
        parse(child, scope, retval);
        curp = end(child);
    repe
    if(curp != end(node)){ retval.tb.write( substring(scope, curp, end(node)) , scope); }
}





int main(){
    str buffer;
    //get from iostream 
    while(!cin.eof()){
        str tmp = "";    
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
        each(x, allKinds){
            debug_buffer += "node type:"+ x.f + "\n";
            debug_buffer += x.s + "\n";
            debug_buffer += "\n";
        }
    }
    if(c_dbg){ 
        cout << debug_buffer;
    }

    return 0;
}