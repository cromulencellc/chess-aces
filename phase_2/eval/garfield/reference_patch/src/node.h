#ifndef __NODE_H__
#define __NODE_H__

#include <iostream>
#include <vector>

class Context;
class NStatement;
class NExpression;
class NVariableDeclaration;
class Value;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

enum node_type {
    ninteger,
    ndouble,
    nstring,
    nidentifier,
    nmethodcall,
    nbinaryoperator,
    nassignment,
    nblock,
    nbuiltin,
    nreturn,
    nfor,
    nforeach,
    nlist,
    nwhile,
    ndowhile,
    nif,
    nifelse,
    nexpressionstmt,
    nvariable,
    nvardec,
    nfunc,
    nfuncdec,
};

//!  Base Node class 
/*!
  NExpression and NStatement inherit from this class
*/
class Node {
public:
    node_type nt;
    virtual ~Node() {}
    virtual Value* execute (Context* context) { return 0; }
};

//!  Base Expression class 
/*!
  NExpression and NStatement inherit from this class
*/
class NExpression : public Node {
};

class NStatement : public Node {
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { nt = ninteger; }
    virtual Value* execute(Context* context );
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { nt = ndouble; }
    virtual Value* execute(Context* context );
};

class NList : public NExpression {
public:
    ExpressionList elements;
    NList( ExpressionList& elements ) : elements(elements) { nt = nlist; }
    virtual Value* execute(Context* context );
};

class NString : public NExpression {
public:
    std::string value;
    NString(std::string value) : value(value) { nt = nstring; }
    virtual Value* execute(Context* context );
};

class NIdentifier : public NExpression {
public:
    std::string name;
    NIdentifier(const std::string& name) : name(name) { nt = nidentifier;}
    virtual Value* execute(Context* context );
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    NMethodCall(const NIdentifier& id) : id(id){ nt = nmethodcall; }
    virtual Value* execute(Context* context );
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
        op(op), lhs(lhs), rhs(rhs){ nt = nbinaryoperator; }
    virtual Value* execute(Context* context );
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) : 
        lhs(lhs), rhs(rhs) { nt = nassignment; }
    virtual Value* execute(Context* context );
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { nt = nblock; }
    virtual Value* execute(Context* context );
};

class NReturnStatement : public NStatement {
public:
    NExpression& expression;
    NReturnStatement(NExpression& expression) : 
        expression(expression) { nt = nreturn; }
    virtual Value* execute(Context* context );
};

class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) : 
        expression(expression) { nt = nexpressionstmt; }
    virtual Value* execute(Context* context );
};

class NVariable : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    Value *v;

    ~NVariable( );
    NVariable(const NIdentifier& type, const NIdentifier& id) :
        type(type), id(id), v(NULL) { nt = nvariable; }
    NVariable(const NIdentifier& type, NIdentifier& id, Value *v) :
        type(type), id(id), v(v) { nt = nvariable;  }
    virtual Value* execute(Context* context );
};

class NVariableDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(const NIdentifier& type, const NIdentifier& id) :
        type(type), id(id), assignmentExpr(NULL) { nt = nvardec; }
    NVariableDeclaration(const NIdentifier& type, const NIdentifier& id, NExpression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { nt = nvardec;  }
    virtual Value* execute(Context* context );
};

class NWhileStatement : public NStatement {
public:
    NExpression& condition;
    NBlock& whileBlock;
    NWhileStatement(NExpression& condition, NBlock& whileBlock) : condition(condition), whileBlock(whileBlock) { nt = nwhile; }
    virtual Value* execute(Context* context );
};

class NDoWhileStatement : public NStatement {
public:
    NExpression& condition;
    NBlock& whileBlock;
    NDoWhileStatement(NExpression& condition, NBlock& whileBlock) : condition(condition), whileBlock(whileBlock) { nt = ndowhile; }
    virtual Value* execute(Context* context );
};

class NForStatement : public NStatement {
public:
    NExpression& init;
    NExpression& condition;
    NExpression& increment;
    NBlock& forBlock;
    NForStatement(NExpression& init, NExpression& condition, NExpression& increment, NBlock& forBlock) : 
        init(init), condition(condition), increment(increment), forBlock(forBlock) { nt = nfor; }
    virtual Value* execute(Context* context );
};

class NForEachStatement : public NStatement {
public:
    NExpression& loopList;
    NBlock& forBlock;
    NForEachStatement(NExpression& loopList, NBlock& forBlock) : 
        loopList(loopList), forBlock(forBlock) { nt = nforeach; }
    virtual Value* execute(Context* context );
};

class NIfStatement : public NStatement {
public:
    NExpression& condition;
    NBlock& ifBlock;
    NIfStatement(NExpression& condition, NBlock& ifBlock) : condition(condition), ifBlock(ifBlock) { nt = nif; }
    virtual Value* execute(Context* context );
};

class NIfElseStatement : public NStatement {
public:
    NExpression& condition;
    NBlock& ifBlock;
    NBlock& elseBlock;
    NIfElseStatement(NExpression& condition, NBlock& ifBlock, NBlock& elseBlock) : condition(condition), ifBlock(ifBlock), elseBlock(elseBlock) { nt = nif; }
    virtual Value* execute(Context* context );
};

class NBuiltInFunction : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NBuiltInFunction(const NIdentifier& type, const NIdentifier& id, const VariableList& arguments) :
        type(type), id(id), arguments(arguments) { nt = nbuiltin; }
    virtual Value* execute(Context* context );
    virtual Value* execute_builtin( std::vector<Value *> args );
};

/*
 * Used to place the function into a given context
 */
class NFunction : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunction(const NIdentifier& type, const NIdentifier& id, 
            const VariableList& arguments, NBlock& block) :
        type(type), id(id), arguments(arguments), block(block) { nt = nfunc; }
    virtual Value* execute(Context* context );
};

class NFunctionDeclaration : public NStatement {
public:
    const NIdentifier& type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    NFunctionDeclaration(const NIdentifier& type, const NIdentifier& id, 
            const VariableList& arguments, NBlock& block) :
        type(type), id(id), arguments(arguments), block(block) { nt = nfuncdec; }
    virtual Value* execute(Context* context );
};

#endif