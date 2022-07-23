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

bool forced_display_variable_insertion = true;

//linking_begin
//linking_end

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

#define addpp(s, x, y) if(true){ data_type* nndt = data_type::newInstance();\
nndt->content =  x;\
nndt->type = y;\
s->push_back(nndt);\
s->isSequential = true;\
}\
\
\

namespace tree_sitter_declaration_space {
    using namespace tree_sitter_memory_management;
    class data_type;
    using Seq = vector<data_type*>;
    class data_type{
    public:
        data_type * parent;
        data_type(){
            parent = nullptr;
            type = "";
            sq = Seq();
            isSequential = false; 
            content = "";
        }
        str type; 
        virtual str getBaseType(){
            return "";
        }
        Seq sq;
        bool isSequential;
        str content; 
        static data_type * newInstance(){
            return memoryManager.newInstance<data_type>();
        } 
        void add(data_type * d){
            sq.pb(d);
            if(d != nullptr){
                d->parent = this;
            }
            isSequential = true;
        }
        void push_back(data_type * d){
            add(d);
        }
        void addAll(const Seq & s){
            each(d, s){
                sq.pb(d);
                if(d != nullptr) d->parent = this;
            }
            isSequential = true;
        }
        void show(){
            cout << content ;
            rep(i, 0, sq.size()){
                sq[i]->show();
            }
        }

        void show_debug(int level){
#ifdef _DEBUG
            //repeat '\t' level times
            rep(i, 0, level){
                cerr << "\t";
            }
            cerr  << "node_type[" << type << "] :" << content ;
            cerr << endl;
            rep(i, 0, sq.size()){
                sq[i]->show_debug(level + 1);
            }
#endif
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
        
        data_type * get(int idx){
            Seq s;
            each(x, sq){
                if(x->type == "") continue;
                s.pb(x);
            }
            return s[idx];
        }
    };
    using vpdtpdt = vector<pair<data_type*, data_type*>>; // store the usage of other functions/variables/types etc. 

    class function_definition_data_type : public data_type{
    public:
        vpdtpdt usages;
        function_definition_data_type(){
            data_type();
            usages = vpdtpdt();
            type = "function_definition";
        }
        str getBaseType() override{
            return "function_definition";
        }
        static function_definition_data_type * newInstance(){
            return memoryManager.newInstance<function_definition_data_type>();
        }
    };

    class parameter_list_data_type : public data_type{
    public:
        parameter_list_data_type(){
            data_type();
            type = "parameter_list";
            sq = Seq();
            isSequential = false; 
            content = "";
        }
        str getBaseType() override{
            return "parameter_list";
        }
        int getArgsCount(){
            int cnt = 0;
            int flag = 0;
            each(x, sq){
                if(x->type == "(") flag = 1;
                if(x->type == ")") flag = 0;
                if(x->type != "" && x->type != "," && x->type != "(" && flag == 1){
                    cnt++;
                }
            }
            return cnt;
        }
        static parameter_list_data_type * newInstance(){
            return memoryManager.newInstance<parameter_list_data_type>();
        }
    };  

    class class_specifier_data_type : public data_type{
        public:
            class_specifier_data_type(){
                data_type::data_type();
                type = "class_specifier";
            }
            str getBaseType() override{
                return "class_specifier";
            }
            str type_identifier_name;
            static class_specifier_data_type * newInstance(){
                return memoryManager.newInstance<class_specifier_data_type>();
            }

            str getName(){
                each(x, sq){
                    if(x->type == "type_identifier"){
                        return x->toString();
                    }
                }
                return "";
            }
    };
   
    class function_declarator_data_type : public data_type{
    public:
        function_declarator_data_type(){
            data_type();
            name = "";
            args = 0;
            type = "function_declarator";
            sq = Seq();
            isSequential = false; 
            content = "";
        }
        str getBaseType() override{
            return "function_declarator";
        }
        static function_declarator_data_type * newInstance(){
            return memoryManager.newInstance<function_declarator_data_type>();
        }
        string name;
        int args = 0;
    };

    pair<str,int> getFunctionDeclaration(data_type * dt){
        if(dt->getBaseType() != "function_declarator"){
            each(y, dt->sq){
                auto ret = getFunctionDeclaration(y);
                if(ret.s != -1){
                    return ret;
                }
            }
            return make_pair("", -1);
        }
        function_declarator_data_type* ddt = dynamic_cast<function_declarator_data_type*>(dt);
        if(ddt == nullptr){
            return make_pair("", -1);
        }
        return make_pair(ddt->name, ddt->args);
    }
    str getVariableDeclaration(data_type * dt){
        if(dt->type != "identifier"){
            each(y, dt->sq){
                auto ret = getVariableDeclaration(y);
                if(ret != ""){
                    return ret;
                }
            }
            return "";
        }
        return dt->toString();
    }
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
    using variable_definition = pair<str, int>; //name + int
    using variable_definition_body = vpdtpdt; //data_type* 
    using MVD = map<variable_definition, variable_definition_body>; // -1 = variable
    class Scope{
    public:
        bool isInserting = false;
        MVD vvd;
        MVD types; 
        vpdtpdt vUsage; 
        void add_function(str name, int args, data_type * dt, data_type * dt2){
            vvd[{name, args}].pb(mp(dt, dt2));
        }
        void add_variable(str name, data_type * dt, data_type * dt2){
            vvd[{name, -1}].pb(mp(dt, dt2));
        }
        // include classes and aliases and structs and unions and enums and typedefs
        void add_type(str name, data_type * dt, data_type * dt2){
            types[{name, -1}].pb(mp(dt, dt2));
        }
        Scope * parent;
        Scope(Scope * parent){
            this->parent = parent;
            isInserting = false;
        }

        void passdown(Scope & x){
            x.buffer = this->buffer;
            x.vUsage = vpdtpdt();
            x.tsParser = this->tsParser;
            x.isInserting = this->isInserting;
        }

        vpdtpdt getAllUsage(str name, int args){
            vpdtpdt ret;
            if(vvd.count({name, args}) == 0){
                if(parent != nullptr){
                    ret = parent->getAllUsage(name, args);
                }
            }
            else{
                ret = vvd[{name, args}];
            }
            return ret;
        }
        
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

    using lambda_child_travse = const function<void(TSNode)>&;
    void getch(TSNode & x, lambda_child_travse y){
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
    str trim(str & x){
        int st = 0;
        int ed = sz(x);
        while(st < ed && (isspace(x[st]) || x[st] == '\n' || x[st] == '\r')){
            st++;
        }
        while(st < ed && (isspace(x[ed-1]) || x[ed-1] == '\n' || x[ed-1] == '\r')){
            ed--;
        }
        return x.substr(st, ed-st);
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

//function prototype void parse(TSNode & node, Scope & scope, ReturnValue & retval)

using parse_proc = function<void(TSNode&, Scope & , ReturnValue &)>;
using parse_proc_ref = parse_proc &;
using const_parse_proc_ref = const parse_proc &;
class statement_handler;
map<str, statement_handler*> statement_handlers;

#define prel(x, y) data_type* dt = data_type::newInstance();\
dt->type = type(x);\
y.dt = nullptr;\
\
\

#define prel_typed(x, y, tp) tp* dt = tp::newInstance();\
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
    data_type * addDeclaration(data_type * dt, Scope & scope , bool isFunction){
            data_type * ndt = data_type::newInstance();
            ndt->content = "[insert declaration]";
            ndt->type = "insertion";
            if(isFunction){
                auto fund = getFunctionDeclaration(dt);
                scope.add_function(fund.f, fund.s, dt, ndt);
            }else{
                auto vard = getVariableDeclaration(dt);
                scope.add_variable(vard,  dt, ndt);
            }
            if(scope.isInserting || forced_display_variable_insertion){
                return ndt;
            }
            return dt;
    }
};

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
        if(curp != end(node))  addp(substring(scope, curp, end(node)));
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

    void generateType(data_type * dt, Scope & scope, str & type, const_parse_proc_ref parse){
        Scope nScope(nullptr);
        nScope.copyParser(scope);
        string tmps = type;
        nScope.buffer = &tmps;
        TSNode nRoot = nScope.getParsedNode();
        ReturnValue nRetVal;
        parse(nRoot, nScope, nRetVal); 
        data_type * tmp = nRetVal.dt;
        dt->add(tmp);
        dt->isSequential = true;
    }

    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        prel(node, retVal);
        int curp = start(node);
        str text = substring(scope, node);
        assert(cc(node) == 0);
        if(replacingTypes.count(text)){
            generateType(dt, scope, replacingTypes[text], parse);
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
                        generateType(dt, scope, to_string(ss), parse);
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

    map<str, str> replacingTypes, replacingFieldTypes;
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
        bool repType = (head == "rep" || head == "rrep");
        if(repType && templateFormulation == "()"){
            // todo if we have get unused variable name from scope
            str unusedName = scope.getUnusedVariableName();
            if(head == "rep"){
                ss << "for(int " << unusedName << " = ("<<0 << "); "<<  unusedName << " < (" <<  positionArgs[0] << "); "<< unusedName <<"++)";
            }else if(head == "rrep"){
                ss << "for(int " << unusedName << " = (("<<positionArgs[0] << ")-1); "<<  unusedName << " >= (0); "<< unusedName <<"--)";
            }
        }
        if(repType && templateFormulation == "(,)"){
            if(head == "rep"){
                ss << "for(int " << positionArgs[0] << " = ("<<0 << "); "<< positionArgs[0] << " < (" <<  positionArgs[1] << "); "<< positionArgs[0] <<"++)";
            }else if(head == "rrep"){
                ss << "for(int " << positionArgs[0] << " = (("<< positionArgs[1]  << ")-1); "<< positionArgs[0] << " >= 0; "<< positionArgs[0] <<"--)";
            }
        }
        if(repType && templateFormulation == "(,,)"){
            if(head == "rep"){
                ss << "for(int " << positionArgs[0] << " = ("<< positionArgs[1] << "); "<< positionArgs[0] << " < (" << positionArgs[2] << "); "<< positionArgs[0] <<"++)";
            }else if(head == "rrep"){
                ss << "for(int " << positionArgs[0] << " = (("<< positionArgs[2] << ")-1); "<< positionArgs[0] << " >= (" << positionArgs[1] << "); "<< positionArgs[0] <<"--)";
            }    
        }
        if (repType &&  templateFormulation == "(,,,)"){
            if(head == "rep"){
                ss << "for(int " << positionArgs[0] << " = ("<< positionArgs[1] << "); "<< positionArgs[0] << " < (" << positionArgs[2] << "); "<< positionArgs[0] <<"+= (" << positionArgs[3] << "))";
            }else if(head == "rrep"){
                ss << "for(int " << positionArgs[0] << " = (("<< positionArgs[2] << ")-1); "<< positionArgs[0] << " >= (" << positionArgs[1] << "); "<< positionArgs[0] <<"-= (" << positionArgs[3] << "))";
            }
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

// TODO template_declaration handler
class template_declaration : public statement_handler{
public:
    template_declaration(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return {"template_declaration"};
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
        prel(node, retVal);
        int curp = start(node);
        function_definition_data_type * functionDT = nullptr;
        Scope nScope(&scope);
        scope.passdown(nScope);

        repb(child, node)   
            addp(substring(nScope, curp, start(child)));
            parse(child, nScope, retVal);
            adds(retVal);
            if(is_type(child, "function_definition")){
                functionDT = dynamic_cast<function_definition_data_type*>(retVal.dt);
            }
            curp = end(child);
        repe
        
        if(curp != end(node)) addp(substring(nScope, curp, end(node)));
        if(functionDT){
            retVal.dt  = addDeclaration(dt, scope, true);
        }
        else{
            posl(retVal);
        }

    }
}template_declaration_handler;


class function_declarator : public statement_handler{
public:
    function_declarator(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return {"function_declarator"};
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
         prel_typed(node, retVal, function_declarator_data_type);
         int curp = start(node);
         parameter_list_data_type * fargc = nullptr;
         data_type * fname = nullptr;
         repb(child, node)   
             addp(substring(scope, curp, start(child)));
             parse(child, scope, retVal);
             adds(retVal);
             if(is_type(child, "parameter_list")){
                fargc = dynamic_cast<parameter_list_data_type*>(retVal.dt);
             }
             if(is_type_of(child, set<str>{"identifier"})){
                fname = retVal.dt; 
             }
             curp = end(child);
         repe
         if(curp != end(node)){
             addp( substring(scope, curp, end(node)) );
         }
         if(fname){
            dt->name = fname->toString();
         }
         if(fargc){
            dt->args = fargc->getArgsCount();
         }

         posl(retVal);
     }
}function_declarator_handler;

class function_definition : public statement_handler{
public:
    function_definition(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return { "function_definition" };
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
         prel_typed(node, retVal, function_definition_data_type);
         Scope nScope(&scope);
         scope.passdown(nScope);

         Scope & cScope = [&](){
                if(!is_type(ts_node_parent(node), "template_declaration")){
                    return nScope;
                }else{
                    return  scope;
                }
         }();
         
         
         int curp = start(node);
         repb(child, node)   
             bool isInsertingDeclarationForScope = scope.isInserting;
             bool isInsertingDeclarationForVariable = forced_display_variable_insertion;
             scope.isInserting = false;
             forced_display_variable_insertion = false;  
             addp(substring(cScope, curp, start(child)));
             parse(child, cScope, retVal);
             adds(retVal);
             curp = end(child);
             scope.isInserting = isInsertingDeclarationForScope;
            forced_display_variable_insertion = isInsertingDeclarationForVariable;
         repe
         if(curp != end(node)) addp( substring(cScope, curp, end(node)) );
         dt->usages = cScope.vUsage;
         if(!is_type(ts_node_parent(node), "template_declaration")){
            retVal.dt  = dt;//addDeclaration(dt, scope, true);
         }
         else{
             posl(retVal);
         }
     }
}function_definition_handler;

class parameter_list : public statement_handler{
public:
    parameter_list(){
        this->registerHandler(getAllTypes());
    }
    vector<string> getAllTypes() override{
        return {"parameter_list"};
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
         prel_typed(node, retVal, parameter_list_data_type);
         int curp = start(node);
         
         repb(child, node)   
             addp(substring(scope, curp, start(child)));
             parse(child, scope, retVal);
             adds(retVal);
             curp = end(child);
         repe
         if(curp != end(node)){
             addp( substring(scope, curp, end(node)) );
         }
        int ct = dt->getArgsCount();
         posl(retVal);
     }
    
} parameter_list_handler;


class comment : public statement_handler{
public:
    comment(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return { "comment"};
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
         prel(node, retVal);
         int curp = start(node);
         assert(cc(node) == 0);
         if(trim(substring(scope, node)) == "//linking_begin"){
            scope.isInserting = true;
         }else  if(trim(substring(scope, node)) == "//linking_end"){
            scope.isInserting = false;
         }
         if(curp != end(node)){
             addp( substring(scope, curp, end(node)) );
         }
         posl(retVal);
     }
}comment_statement_handler;

class class_specifier : public statement_handler{
public:
    class_specifier(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return { "class_specifier" };
    }
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
         prel_typed(node, retVal, class_specifier_data_type);
         int curp = start(node);
         repb(child, node)   
             addp(substring(scope, curp, start(child)));
             parse(child, scope, retVal);
             adds(retVal);
             curp = end(child);
         repe
         if(curp != end(node)){
             addp( substring(scope, curp, end(node)) );
         }
         posl(retVal);
     }
}class_specifier_handler;

class declaration : public statement_handler{
public:
    declaration(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return { "declaration", "field_declaration" };
    }

    // TODO: read_declarator
    set<str> getAllIdentifierType(){
        return set<str>({
            "attributed_declarator", 
            "pointer_declarator", 
            "function_declarator", 
            "array_declarator", 
            "parenthesized_declarator", 
            "identifier", 
            "init_declarator", 
            "field_identifier"});
    }    
    void handle(TSNode & node, Scope & scope, ReturnValue & retVal, const_parse_proc_ref parse) {
         prel(node, retVal);
         int curp = start(node);
         vector<data_type*> preType, identifiersList; 
         repb(child, node)   
             addp(substring(scope, curp, start(child)));
             parse(child, scope, retVal);
             adds(retVal);
             curp = end(child);
         repe
         if(curp != end(node)){
             addp( substring(scope, curp, end(node)) );
         }
         set<str> allIdentiferType = getAllIdentifierType();
         bool beginning = true;
         data_type * prec = data_type::newInstance();
         prec->type = "identifier_declarator";
         each(x, dt->sq){
            if(allIdentiferType.count(x->type)){
                beginning = false;
            }
            if(!beginning && x->type != ";" && x->type != ","){
                prec->add(x);
            }
            if(!beginning && x->type == ";" || x->type == ",") {
                identifiersList.push_back(prec);
                prec = data_type::newInstance();
                prec->type = "identifier_declarator";
                continue;
            }
            if(beginning){
                preType.push_back(x);
            }
         }
         //get padding 
         string spd;
         rrep(i, 0, curp){
            if((*scope.buffer)[i] == '\n' ) break;
            spd += (*scope.buffer)[i];
         }
         reverse(spd.begin(), spd.end());
         string ppdx;
         rep(i, 0, sz(spd)){
            if(spd[i] != ' ' && spd[i] != '\t') break;
            ppdx += spd[i];
         }
         if(dt->get(0)->type == "class_specifier"){
            data_type * ndt = data_type::newInstance();
            ndt->addAll(preType);
            addpp(ndt, ";\n" , ";");
            ndt->type = "translation_unit";
            class_specifier_data_type * csdt = dynamic_cast<class_specifier_data_type*>(dt->get(0));
            if(csdt != nullptr){
                each(id, identifiersList){
                    data_type * nxdt = data_type::newInstance();
                    nxdt->type = "declaration";
                    addpp(nxdt, ppdx, "");
                    addpp(nxdt, csdt->getName(), csdt->getName());
                    addpp(nxdt, " " , "");
                    nxdt->add(id);
                    addpp(nxdt, ";\n" , ";");
                    ndt->add(addDeclaration(nxdt, scope, false));
                }
            }
            retVal.dt = ndt;
         }else{
            data_type * ndt = data_type::newInstance();
            ndt->type = "translation_unit";
            rep(i, 0, sz(identifiersList)){
                data_type * nxdt = data_type::newInstance();
                nxdt->type = "declaration";
                auto& id = identifiersList[i];
                string padding = ";";
                if(i != sz(identifiersList)-1){
                    padding += "\n";
                }
                if(i != 0){
                    data_type * idt = data_type::newInstance();
                    idt->content = ppdx;
                    nxdt->add(idt);
                }
                nxdt->addAll(preType);
                nxdt->add(id);
                addpp(nxdt, padding , ";");
                ndt->add(addDeclaration(nxdt, scope, false));
            }
            retVal.dt = ndt;
         }
     }
}declaration_handler;


void parse(TSNode & node, Scope & scope, ReturnValue & retval){
    prel(node, retval);
    str nodeType = type(node);
    str content = substring(scope, start(node), end(node));
    
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
    str buffer, bf;
    //get from iostream 
    while(!cin.eof()){
        str tmp = "";    
        getline(cin, tmp);
        buffer += tmp + "\n";
    }
    TSParser * tsParser = ts_parser_new();
    ts_parser_set_language(tsParser, tree_sitter_cpp());
	
    TSNode root;
    Scope scope(nullptr);
    scope.buffer = &buffer;
    scope.tsParser = tsParser;
    root = scope.getParsedNode();
    ReturnValue retval;
    parse(root, scope, retval);
    retval.dt->show();
    //print all kinds
    retval.dt->show_debug(0);

    return 0;
}