#include "cg.hpp"
#include "source.hpp"


class LlvmCodeHandler{
    

public:
    CodeBuffer codeBuffer;
    int RegNum = 0;
    stack<string> cont_labels;
    stack<string> break_labels;

    LlvmCodeHandler()=default;
    string freshVar();
    string globalFreshVar();
    void handle_binop(Node* res_exp, Node* L_exp, Node* R_exp, std::string op);
    void handle_relop(Node* res_exp, Node* L_exp, Node* R_exp, string op);
    void allocate_var(string type, string id, Node* assigningExp);
    void change_var_value(string id, Node* assigningExp);
    void handleOrAnd(Node* L_exp);
    string getLlvmType(string type);
    void startingPrints();

};