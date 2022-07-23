template for statement handler
class x_statement : public statement_handler{
public:
    x_statement(){
        this->registerHandler(getAllTypes());
    }
    vs getAllTypes() override{
        return {};
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
             addp( substring(scope, curp, end(node)) );
         }
         posl(retVal);
     }
}x_statement_handler;