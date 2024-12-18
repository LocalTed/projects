#pragma once

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <stack>
#include <vector>
#include <memory>
#include <iostream>
#include "hw3_output.hpp"

using namespace std;

class Node
{
public:
    int lineno;
    string type;
    string next;
    string var;
    Node(int lineno, string type = "", string var = "") : lineno(lineno), type(type), var(var) {};
    virtual ~Node()= default;
    virtual string print_Node(){return type;};
	
};

class Symtab 
{
    public:
    class Table
    {
        public:
        class Entry
        {
            public:
            int offset;
            string name;
            string type;
            string funcArg;
            string var;
            
            Entry(string name, string type, int offset, string funcArg, string var) : name(name), type(type), offset(offset), funcArg(funcArg), var(var) {};
            void printEntry() const
            {
                /*if(this->funcArg != ""){
                    output::printID(name, offset, output::makeFunctionType(funcArg, type));
                }
                else output::printID(name, offset, type);*/
            };
        };

        shared_ptr<Table> parent;
        vector<Entry*> entries;
        bool isWhile = false;

        Table(shared_ptr<Table> parent) : parent(parent), entries() {};

        void addEntry(string name, string type, int offset, string var, string funcArg="")
        {
            entries.push_back(new Entry(name, type, offset, funcArg, var));
        };
        void printTable(){
            for(auto i : entries){
                i->printEntry();
            }
        };
        void deleteTable(){
            for(Entry* entry : entries){
                delete entry;
            }
        };

        Entry* findEntry(string name){
            for(auto i : entries){
                if(i->name == name)
                    return i;
            }
            return nullptr;
        }
        
    }; 

    stack<shared_ptr<Table>> tableStack;
    stack<int> offsetsStack;
    int curr_offset = 0;
    shared_ptr<Table> root = nullptr;
    string rbpvar;
    

    Symtab(){
        offsetsStack.push(0);
        shared_ptr<Table> newTable = make_shared<Table>(root);
        tableStack.push(newTable);

        newTable->addEntry("print", "VOID", 0, "tempvar", "STRING");
        newTable->addEntry("printi", "VOID", 0, "tempvar", "INT");
        newTable->addEntry("readi", "INT", 0, "tempvar", "INT");
    }
    
    shared_ptr<Table> addTable(shared_ptr<Table>& parent)
    {
        shared_ptr<Table> newTable = make_shared<Table>(parent);
        //curr_offset++;
        offsetsStack.push(offsetsStack.top());
        tableStack.push(newTable);
        return newTable;
    };

    void printSymtab(shared_ptr<Table> table)
    {
        shared_ptr<Table> curr = table;
        while(curr != root) {
            curr->printTable();
            curr = curr->parent;
        }
    };

    Table::Entry* find(string name){
        shared_ptr<Table> curr = tableStack.top();
        while(curr != nullptr){
            Table::Entry* entry = curr->findEntry(name);
            if(entry != nullptr)
                return entry;
            curr = curr->parent;
        }
        return nullptr;
    }
    bool isInWhile(){
        shared_ptr<Table> curr = tableStack.top();
        while(curr != nullptr){
            if(curr->isWhile)
                return true;
            curr = curr->parent;
        }
        return false;
    }
    
    
    /*void iteratorPrint(){
        for(auto i:tableStack )
            (*i)->print();
    };*/

};

class BoolVarNode : public Node {
    public:
    string var;
    string truelab;
    string falselab;
    BoolVarNode(int yylineno, string type, string var) : Node(yylineno, type, var){};
};

// class NumVarNode : public Node {
//     public:
//     string var;
//     NumVarNode(int yylineno, string type, string var) : Node(yylineno, type), var(var){};
//     string print_Node() override{
//         return type;
//     };

// };

class NumNode : public Node {
    public:
    int num;
    NumNode(int yylineno, int num, string type, string var) : Node(yylineno, type, var), num(num) {};
    string print_Node() override{
        return type;
    };
};

class IdNode : public Node {
    public:
    string id;
    string truelab;
    string falselab;
    IdNode(int yylineno, string id, string type, string var) : Node(yylineno, type, var),  id(id){};
    string print_Node() override{
        return type;
    };
};

class StringNode : public Node {
    public:
    string str;
    StringNode(int yylineno, string str, string type, string var) : Node(yylineno, type, var), str(str){};
};




#define YYSTYPE Node*	// Tell Bison to use STYPE as the stack type