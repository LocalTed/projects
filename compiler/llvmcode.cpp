#include "llvmcode.hpp"
extern Symtab symtable;


string LlvmCodeHandler::freshVar(){
    return "%var_" + std::to_string(this->RegNum++);
}

string LlvmCodeHandler::globalFreshVar(){
    return "@var_" + std::to_string(this->RegNum++);
}


void LlvmCodeHandler::handle_binop(Node* res_exp, Node* L_exp, Node* R_exp, std::string op){ 
    res_exp->var = freshVar();
    string llvm_op;
    if (op == "*") {
        llvm_op = "mul";
    }
    else if (op == "/") {
        if (res_exp->type == "INT") {
            llvm_op = "sdiv";
        }
        else { //res_exp->type = "byte"
            llvm_op = "udiv";
        }
        // Check devision by zero
        string R_input = freshVar();
        if(R_exp->type == "BYTE" && !dynamic_cast<IdNode*>(R_exp))
            codeBuffer.emit(R_input + " = zext i8 " + R_exp->var + " to i32"); 
        else codeBuffer.emit(R_input + " = add i32 " + R_exp->var + ", 0"); 
            
        codeBuffer.emit("call void @check_division(i32 " + R_input + ")");
        
    }
    else if (op == "+") {
        llvm_op = "add";
    }
    else if (op == "-") {
        llvm_op = "sub";
    }

    //adjusting sizes
    string L_input = freshVar();
    string R_input = freshVar();
    if(res_exp->type == "INT"){
        if(L_exp->type == "BYTE" && !dynamic_cast<IdNode*>(L_exp))
            codeBuffer.emit(L_input + " = zext i8 " + L_exp->var + " to i32"); 
        else codeBuffer.emit(L_input + " = add i32 " + L_exp->var + ", 0"); 
        if(R_exp->type == "BYTE" && !dynamic_cast<IdNode*>(R_exp))
            codeBuffer.emit(R_input + " = zext i8 " + R_exp->var + " to i32"); 
        else codeBuffer.emit(R_input + " = add i32 " + R_exp->var + ", 0"); 

    codeBuffer.emit(res_exp->var + " = " + llvm_op + " i32 " + L_input + ", " + R_input); //var3 add i32 var1, var2
    }
    else {
        if(L_exp->type == "BYTE" && dynamic_cast<IdNode*>(L_exp))
            codeBuffer.emit(L_input + " = trunc i32 " + L_exp->var + " to i8"); 
        else codeBuffer.emit(L_input + " = add i8 " + L_exp->var + ", 0"); 
        if(R_exp->type == "BYTE" && dynamic_cast<IdNode*>(R_exp))
            codeBuffer.emit(R_input + " = trunc i32 " + R_exp->var + " to i8"); 
        else codeBuffer.emit(R_input + " = add i8 " + R_exp->var + ", 0"); 
        
        codeBuffer.emit(res_exp->var + " = " + llvm_op + " i8 " + L_input + ", " + R_input);
    }
}


void LlvmCodeHandler::handle_relop(Node* res_exp, Node* L_exp, Node* R_exp, string op){ 
    string llvm_relop = "";
    bool sign;
    if (op == "=="){
        llvm_relop = "eq";
    }
    else if (op == "!=") {
        llvm_relop = "ne";
    }
    else if (op == "<"){
        llvm_relop = "lt";
    }
    else if (op == ">") {
        llvm_relop = "gt";
    }
    else if (op == "<=" ) {
        llvm_relop = "le";
    }
    else if (op == ">=") {
        llvm_relop = "ge";
    }

    if (L_exp->type == "INT" || R_exp->type == "INT")
        sign = true;
    else
        sign = false;
    
    if (llvm_relop != "eq" && llvm_relop != "ne") {
        if (sign)
            llvm_relop = "s" + llvm_relop;
        else
            llvm_relop = "u" + llvm_relop;
    }

    string L_input = freshVar();
    string R_input = freshVar();
    if(L_exp->type == "BYTE" && !dynamic_cast<IdNode*>(L_exp))
        codeBuffer.emit(L_input + " = zext i8 " + L_exp->var + " to i32"); 
    else codeBuffer.emit(L_input + " = add i32 " + L_exp->var + ", 0"); 
    if(R_exp->type == "BYTE" && !dynamic_cast<IdNode*>(R_exp))
        codeBuffer.emit(R_input + " = zext i8 " + R_exp->var + " to i32"); 
    else codeBuffer.emit(R_input + " = add i32 " + R_exp->var + ", 0"); 

    res_exp->var = this->freshVar();
    this->codeBuffer.emit(res_exp->var + " = " + "icmp " + llvm_relop + " i32 " + L_input + ", " + R_input);

    //int next_instr = this->codeBuffer.emit("br i1 " + var + ", label @, label @");

    /* Entry is exp1 */
    //res_exp.start_label = L_exp.start_label;
    /* nextlist of exp1 is exp2 entry*/

}

void LlvmCodeHandler::allocate_var(string type, string id, Node* assigningExp) {

    string llvmtype = getLlvmType(type);

    string var = freshVar();
    symtable.tableStack.top()->addEntry(id, type, symtable.offsetsStack.top()++, var);
    int offset = symtable.find(id)->offset;

    string ptrvar = freshVar();
    codeBuffer.emit(ptrvar + " = getelementptr i32, i32* " + symtable.rbpvar + ", i32 " + to_string(offset));

    //type id sc
    if(assigningExp->var == "")
        codeBuffer.emit(var + " = add i32 0, 0");

    //type id assign exp sc
    else {
        if(type == "BOOL")
            codeBuffer.emit(var + " = zext i1 " + assigningExp->var + " to i32");
        else if(type == "BYTE")
            codeBuffer.emit(var + " = zext i8 " + assigningExp->var + " to i32");
        else if(type == "INT"){
            if(assigningExp->type == "BYTE" && !dynamic_cast<IdNode*>(assigningExp))
                codeBuffer.emit(var + " = zext i8 " + assigningExp->var + " to i32");
            else var = assigningExp->var;
        }
    }
        codeBuffer.emit("store i32 " + var + ", i32* " + ptrvar);
        symtable.find(id)->var = ptrvar;
}

//id assign exp sc
void LlvmCodeHandler::change_var_value(string id, Node* assigningExp){

    string var = freshVar();
    string ptrvar = freshVar();
    //int offest = symtable.find(id)->offset;
    //codeBuffer.emit(ptrvar + " = getelementptr i32, i32* " + symtable.rbpvar + ", i32 " + to_string(offest));

    if(assigningExp->type == "BOOL" && !dynamic_cast<IdNode*>(assigningExp))
            codeBuffer.emit(var + " = zext i1 " + assigningExp->var + " to i32");
    else if(assigningExp->type == "BYTE" && !dynamic_cast<IdNode*>(assigningExp))
            codeBuffer.emit(var + " = zext i8 " + assigningExp->var + " to i32");
    else var = assigningExp->var;

    codeBuffer.emit("store i32 " + var + ", i32* " + symtable.find(id)->var);

}

void LlvmCodeHandler::handleOrAnd(Node* L_exp){
    
    if(L_exp->type != "BOOL"){
        return;
    }
    
    if(dynamic_cast<IdNode*>(L_exp)){  //if id
        string toTrunc = freshVar();
        codeBuffer.emit(toTrunc + " = trunc i32 " + L_exp->var + " to i1");
        L_exp->var = toTrunc;
    }

    string truelab = codeBuffer.freshLabel();
    string falselab = codeBuffer.freshLabel();
    ((BoolVarNode*)L_exp)->truelab = truelab;
    ((BoolVarNode*)L_exp)->falselab = falselab;

    codeBuffer.emit("br i1 " + L_exp->var + ", label %" + truelab + ", label %" + falselab);
    
    // string skipIfThis;
    // string normalCommand;
    // if(op == "and"){
    //     skipIfThis = "0";
    //     normalCommand = "and i1 " + var1 + ", " + var2;
    // }
    // else {
    //     skipIfThis = "1";
    //     normalCommand = "or i1 " + var1 + ", " + var2;
    // }

    // string normalVar = freshVar();
    // codeBuffer.emit(normalVar + " = " + normalCommand);

    // string checkVar = freshVar();
    // codeBuffer.emit(checkVar + " = icmp eq i1 " + var1 + ", " + skipIfThis);

    // string skipLabel = codeBuffer.freshLabel();
    // string normalLabel = codeBuffer.freshLabel();
    // string phiLabel = codeBuffer.freshLabel();
    // codeBuffer.emit("br i1 " + checkVar + ", label %" + skipLabel + ", label %" + normalLabel);

    // codeBuffer.emit(skipLabel + ":");
    // codeBuffer.emit("br label %" + phiLabel);

    // codeBuffer.emit(normalLabel + ":");
    // codeBuffer.emit("br label %" + phiLabel);

    // string newVar = freshVar();
    // codeBuffer.emit(phiLabel + ":");
    // codeBuffer.emit(newVar + " = phi i1 [" + skipIfThis + ", %" + skipLabel + "], [" + normalVar + ", %" + normalLabel + "]");
    
    // return newVar;
}



string LlvmCodeHandler::getLlvmType(string type){
    string llvmtype;
    if(type == "INT")
        llvmtype = "i32";
    else if(type == "BYTE")
        llvmtype = "i8";
    else if(type == "BOOL")
        llvmtype = "i1";
    else //string
        llvmtype = "i8*";

    return llvmtype;
}

void LlvmCodeHandler::startingPrints() {

    codeBuffer.emit("@.intFormat = internal constant [4 x i8] c\"%d\\0A\\00\"");
    codeBuffer.emit("@.DIV_BY_ZERO_ERROR = internal constant [23 x i8] c\"Error division by zero\\00\"");
    codeBuffer.emit("define void @check_division(i32) {");
    codeBuffer.emit("%valid = icmp eq i32 %0, 0");
    codeBuffer.emit("br i1 %valid, label %ILLEGAL, label %LEGAL");
    codeBuffer.emit("ILLEGAL:");
    codeBuffer.emit("call void @print(i8* getelementptr([23 x i8], [23 x i8]* @.DIV_BY_ZERO_ERROR, i32 0, i32 0))");
    codeBuffer.emit("call void @exit(i32 0)");
    codeBuffer.emit("ret void");
    codeBuffer.emit("LEGAL:");
    codeBuffer.emit("ret void");
    codeBuffer.emit("}");

    codeBuffer.emit("declare i32 @printf(i8*, ...)");
    codeBuffer.emit("declare void @exit(i32)");
    codeBuffer.emit("declare i32 @scanf(i8*, ...)");
    //codeBuffer.emit("@.int_spec = constant [4 x i8] c\"%d\\0A\\00\"");
    codeBuffer.emit("@.str_spec = constant [4 x i8] c\"%s\\0A\\00\"");
    codeBuffer.emit("@.int_spec_2 = constant [3 x i8] c\"%d\\00\"");


    codeBuffer.emit("define void @print(i8*){");
    codeBuffer.emit("call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.str_spec, i32 0, i32 0), i8* %0)");
    codeBuffer.emit("ret void");
    codeBuffer.emit("}");

    codeBuffer.emit("define void @printi(i32){");
    codeBuffer.emit("%format_ptr = getelementptr [4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0");
    codeBuffer.emit("call i32 (i8*, ...) @printf(i8* getelementptr([4 x i8], [4 x i8]* @.intFormat, i32 0, i32 0), i32 %0)");
    codeBuffer.emit("ret void");
    codeBuffer.emit("}");

    codeBuffer.emit("define i32 @readi(i32) {");
    codeBuffer.emit("%ret_val = alloca i32");
    codeBuffer.emit("%spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_spec_2, i32 0, i32 0");
    codeBuffer.emit("call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)");
    codeBuffer.emit("%val = load i32, i32* %ret_val");
    codeBuffer.emit("ret i32 %val");
    codeBuffer.emit("}");
}