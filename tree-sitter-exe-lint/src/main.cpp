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
#ifdef _DEBUG
const int c_dbg = 0;
#else
const int c_dbg = 0;
#endif
// you can navigate using bread crumb

namespace tree_sitter_memory_management{
    class MemoryManager{
    public:
        template<typename T>
        T* newInstance(){
            return new T();
        }
    } memoryManager;
};

#define addp(s) if(true){ data_type* ndt = data_type::newInstance();\
ndt->content =  s;\
dt->sq.push_back(ndt);\
dt->isSequential = true;\
}\
\
\

namespace tree_sitter_declaration_space {
    using namespace tree_sitter_memory_management;
    class data_type;
    using Seq = vector<data_type*>;
    class data_type{
    public:
        data_type(){
            type = "";
            sq = Seq();
            isSequential = false; 
            content = "";
        }
        str type; 
        Seq sq;
        bool isSequential;
        str content; 
        static data_type * newInstance(){
            return memoryManager.newInstance<data_type>();
        } 
        void add(data_type * d){
            sq.pb(d);
            isSequential = true;
        }
        void addAll(const Seq & s){
            each(d, s){
                sq.pb(d);
            }
            isSequential = true;
        }
        void show(){
            cout << content ;
            rep(i, 0, sq.size()){
                sq[i]->show();
            }
        }
        data_type * operator[](int idx){
            return sq[idx];
        }
        size_t size(){
            return sq.size();
        }
        bool isString(){
            if(size() == 0){
                return true;
            }
            if(size() == 1 && sq[0]->size() == 1 && !sq[0]->isSequential){
                return true;
            }
            return false;
        }
        str toString(){
            str ret = "";
            ret += content;
            rep(i, 0, sz(*this)){
                ret += sq[i]->toString();
            }
            return ret;
        }
    };
    class DataTypeStream{
    public:
        DataTypeStream(){
            dt = data_type::newInstance();
        }
        data_type * dt;
        DataTypeStream& operator<<(str s){
            addp(s);
            return *this;
        }        
        DataTypeStream& operator<<(data_type * d){
            dt->add(d);
            dt->isSequential = true;
            return *this;
        }
        data_type * getDataType(){
            return dt;
        }
        Seq getSeq(){
            return dt->sq;
        }
    };
};

using namespace tree_sitter_declaration_space;



namespace tree_sitter_util {
    str to_string(stringstream & x){
        #undef str 
        return x.str();
        #define str string
    }
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
    using namespace tree_sitter_declaration_space;
    class Scope{
    public:
        str * buffer;
        TSParser * tsParser;
        str getUnusedVariableName(){
            // todo
            return "";
        }
        void copyParser(Scope & x){
            tsParser = x.tsParser;
        }
        TSNode getParsedNode(){
            str & buffer = *this->buffer;
            TSParser * tsParser = this->tsParser;
            TSTree * tree = ts_parser_parse_string(
                tsParser,
                NULL,
                buffer.c_str(),
                sz(buffer)
            );
            return ts_tree_root_node(tree);
        }
    };
    
    
    
    class ReturnValue{
    public:
        ReturnValue(){
            dt = nullptr;
        }
        data_type* dt;
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

    // vs Explode(str s, str delim){
    //     vs ret;
    //     int curp = 0;
    //     int nextp = 0;
    //     while(nextp != -1){
    //         nextp = s.find(delim, curp);
    //         if(nextp != -1){
    //             ret.pb(s.substr(curp, nextp - curp));
    //             curp = nextp + sz(delim);
    //         }else{
    //             ret.pb(s.substr(curp));
    //         }
    //     }
    //     return ret;
    // }
    Seq Explode(const str & x, str delim){
        Seq ret;
        int curp = 0;
        int nextp = 0;
        while(nextp != -1){
            nextp = x.find(delim, curp);
            if(nextp != -1){
                data_type * tp = data_type::newInstance();
                tp->content = x.substr(curp, nextp - curp);
                ret.pb(tp);
                curp = nextp + sz(delim);
            }else{
                data_type * tp = data_type::newInstance();
                tp->content = x.substr(curp);
                ret.pb(tp);
            }
        }
        return ret;
    }
    Seq Explode(Seq& st, str delim){
        Seq ret;
        rep(i, 0, sz(st)){
            if(st[i]->type == "" && st[i]->isString()){
                Seq ss = Explode(st[i]->toString(), delim);
                ret.ins(ret.end(), ss.begin(), ss.end());
            }else{
                ret.pb(st[i]);
            }
        }
        return ret;
    }

    Seq replace(Seq & st, str delim, data_type * end){
        Seq ret;
        rep(i, 0, sz(st)){
            if(st[i]->type == "" && st[i]->isString()){
                Seq ss = Explode(st[i]->toString(), delim);
                rep(j, 0, sz(ss)){
                    ret.pb(ss[j]);
                    if(j+1 != sz(ss)){
                        ret.pb(end);
                    }
                }
            }else{
                ret.pb(st[i]);
            }
        }
        return ret;
    }

    Seq Implode(const Seq & s, data_type* delim){
        Seq ret; 
        rep(i, 0, sz(s)){
            ret.pb(s[i]);
            if(i + 1 != sz(s)){
                ret.pb(delim);
            }
        }
        return ret;
    }
}
using namespace  tree_sitter_simplified_proc;

namespace tree_sitter_debug_namespace {
    class debugString{
    public:
        str s;
        debugString(str x) : s(x){
        }
        debugString(){
            s = "";
        }
        str operator()(){
            return s;
        }
        debugString operator+(debugString & x){
            return debugString(s + x.s);
        }
        debugString operator+(str x){
            return debugString(s + x);
        }
        debugString& operator+=(str x){
            s += x;
            return *this;
        }
    };

    ostream & operator<<(ostream & os, debugString & x){
        if(c_dbg){
            os << x();
        }
        return os;
    }
};

using namespace tree_sitter_debug_namespace;

extern "C" {
     TSLanguage* tree_sitter_cpp();
}


#define repb(i, x) getch(x, [&](TSNode i){ str p_content = substring(scope, x);
#define repe });


debugString debug_buffer; // for c_dbg


//function prototype void parse(TSNode & node, Scope & scope, ReturnValue & retval)

using parse_proc = function<void(TSNode&, Scope & , ReturnValue &)>;
using parse_proc_ref = parse_proc &;
using const_parse_proc_ref = const parse_proc &;
class statement_handler;
map<string, statement_handler*> statement_handlers;

#define prel(x, y) data_type* dt = data_type::newInstance();\
dt->type = type(x);\
y.dt = nullptr;\
\
\



#define adds(s)             dt->isSequential = true;\
dt->sq.push_back(s.dt);\
\


#define posl(x) x.dt = dt;



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
        prel(node, retVal);
        int curp = start(node);
        repb(child, node)   
            addp(substring(scope, curp, start(child)));
            parse(child, scope, retVal);
            adds(retVal);
            curp = end(child);
        repe

        if(curp != end(node)){
            addp(substring(scope, curp, end(node)));
        }
        posl(retVal);
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
        prel(node, retVal);
        int curp = start(node);
        str text = substring(scope, node);
        assert(cc(node) == 0);
        if(replacingTypes.count(text)){
            Scope nScope;
            nScope.copyParser(scope);
            nScope.buffer = &replacingTypes[text];
            TSNode nRoot = nScope.getParsedNode();
            ReturnValue nRetVal;
            parse(nRoot, nScope, nRetVal); 
            data_type * tmp = (*nRetVal.dt)[1];
            dt->add((*tmp)[1]);
            dt->isSequential = true;
        }else{
            int vcnt = -1;
            rep(i, 0, sz(text)){
                if(text[i] == 'v'){
                    ++vcnt;
                }else{
                    break;
                }
            }
            
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
                        str buffer = to_string(ss) + " x;";
                        Scope nScope;
                        nScope.copyParser(scope);
                        nScope.buffer = &buffer;
                        TSNode nRoot = nScope.getParsedNode();
                        ReturnValue nRetVal;
                        parse(nRoot, nScope, nRetVal); 
                        data_type * tmp = (*nRetVal.dt)[1];
                        dt->add((*tmp)[1]);
                        dt->isSequential = true;
                        posl(retVal);
                        return;
                    }
                }
            }
            addp(text);
        }
        posl(retVal);
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
        prel(node, retVal);
        if(replacingTypes.count(text)){
            addp(replacingTypes[text]);
        }else{
            addp(text);
        }
        posl(retVal);
    }
}field_identifier_handler;

//call_expression
class call_expression : public statement_handler{
public:
    call_expression(){
        this->registerHandler(getAllTypes());
        replacingTypes["mp"] = "make_pair";
        replacingFieldTypes["sz"] = "(int)((${0}).size())";
        replacingTypes["bg"] = "begin";
        replacingTypes["ed"] = "end";
        replacingTypes["pct"] = "__builtin_popcount";
        replacingFieldTypes["all"] = "begin(${0}), end(${0})";
        replacingFieldTypes["rall"] = "rbegin(${0}), rend(${0})";
        replacingFieldTypes["bits"] = "(${0} == 0 ? 0 : 31-__builtin_clz(${0}))";
        replacingFieldTypes["p2"] = "(1<<(${0}))";
        replacingFieldTypes["msk2"] = "((1<<(${0}))-1)";
    }
    //get maximum ${x} x's value
    int getMaxArgumentValue(str & x){
        int ret = 0;
        rep(i, 0, sz(x)){
            if(x[i] == '$'){
                if(i + 1 < sz(x) && x[i+1] == '{'){
                    bool hit = false;
                    str tmp = "";
                    rep(j, i+2, sz(x)){
                        if(x[j] == '}'){
                            hit = true;
                            break;
                        }
                        tmp += x[j];
                    }
                    if(hit && numberOnly(tmp)){
                        ret = max((ll)ret, stoll(tmp)); // TODO: we can use chmax here
                    }
                }
            }
        }
        return ret+1;
    }
    bool numberOnly(const str & x){
        rep(i, 0, sz(x)){
            if(!isdigit(x[i])){
                return false;
            }
        }
        return true;
    }

    data_type* toFun(str functionName, data_type* _args){
        str x = replacingFieldTypes[functionName];
        data_type * args = (*_args)[0];
        Seq sq; 
        rep(i, 0, sz(*args)){
            data_type* cur = (*args)[i];
            if(cur->type == "") continue;
            if(cur->type == "(") continue;
            if(cur->type == ")") continue;
            if(cur->type == ",") continue;
            sq.push_back(cur);
        }
        int mx = getMaxArgumentValue(x);
        Seq cur;
        data_type * rxt = data_type::newInstance();
        rxt->content = x;
        cur.push_back(rxt);
        assert(mx == sz(sq));
        rep(i, 0 , mx){
            str deliminator = "${" + to_string(i) + "}";
            cur = replace(cur, deliminator, sq[i]);
            //cur = Implode(Explode(cur, deliminator), sq[i]);
        }
        
        // replace "args" with args 
        data_type * ret = data_type::newInstance();
        ret->addAll(cur);
        return ret;
    }

    map<str, str> replacingTypes;
    map<str, str> replacingFieldTypes;
    vs getAllTypes() override{
        return { "call_expression" };
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        prel(node, retVal);
        int curp = start(node);
        bool sentenced = false;
        ReturnValue tmp;
        str funName = "";
        data_type * tdt = data_type::newInstance();
        repb(child, node)
            str ctext = substring(scope, child);   
            if(is_type(child, "identifier")){
                if(replacingFieldTypes.count(ctext)){
                    funName = ctext;
                    sentenced = true;
                }
            }else{
                if(sentenced)  {
                    parse(child, scope, tmp);
                    tdt->add(tmp.dt);
                }
            }
            curp = end(child);
        repe
        if(sentenced){
            auto args = tdt;
            dt->add(toFun(funName, args));
            addp( substring(scope, curp, end(node)) );
            posl(retVal);
            return;
        }
        curp = start(node);
        repb(child, node)
            str ctext = substring(scope, child);   
            addp(substring(scope, curp, start(child)));
            if(is_type(child, "identifier")){
                if(replacingTypes.count(ctext)){
                    addp(replacingTypes[ctext]);
                }else{
                    addp(ctext);
                }
            }else{
                parse(child, scope, retVal);
                adds(retVal);
            }
            curp = end(child);
        repe
        if(curp != end(node)){
            addp( substring(scope, curp, end(node)) );
        }
        posl(retVal);
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
        prel(node, retVal);
        str text = substring(scope, node);
        assert(cc(node) == 0);
        if(replacingTypes.count(text)){
            addp(replacingTypes[text]);
        }else{
            addp(text);
        }
        posl(retVal);
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
                        str &  templateFormulation,
                        vector<data_type*> & positionArgs,
                        data_type* body,
                        Scope & scope,
                        data_type * retVal){
    
        // process the head
        DataTypeStream ss;
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
        retVal->addAll(ss.getSeq());
        // process the body if we keep the body intact
        retVal->add(body);
        
        // process the tail
    }

    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        prel(node, retVal);
        int curp = start(node);
        set<str> importantTypes = toSet(getImportantToken());
        set<str> originatorTypes = toSet(getRepeatKeyTypes());
        str templateFormulation = "";
        str head = "";
        vector<data_type*> positionArgs;
        bool encountered = false;
        data_type* rest;
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
                parse(child, scope, tmp);
                curp = end(child);
                positionArgs.pb(tmp.dt);
            }else{
                ReturnValue tmp;
                parse(child, scope, tmp);
                curp = end(child);
                rest = tmp.dt;
            }
        repe
        handleTemplate(head, templateFormulation, positionArgs, rest, scope, dt);
        if(curp != end(node)){
            addp( substring(scope, curp, end(node)) );
        }
        posl(retVal);
    }
}repeat_statement_handler;

map<str, str> allKinds;
void parse(TSNode & node, Scope & scope, ReturnValue & retval){
    prel(node, retval);
    str nodeType = type(node);
    str content = substring(scope, start(node), end(node));
    if(c_dbg){ allKinds[nodeType] = content;} 
    if(statement_handlers.count(nodeType) != 0){
        statement_handlers[nodeType]->handle(node, scope, retval, parse);
        return;
    }
    int curp = start(node);
    repb(child, node)
        addp(substring(scope, curp, start(child)));
        parse(child, scope, retval);
        adds(retval);
        curp = end(child);
    repe
    if(curp != end(node)){ 
        addp( substring(scope, curp, end(node)) );
    }
    posl(retval);
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
    root = scope.getParsedNode();
    ReturnValue retval;
    parse(root, scope, retval);
    retval.dt->show();
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