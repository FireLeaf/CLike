/*******************************************************************************
FILE:		CL_Syntax.h

DESCRIPTTION:

CREATED BY: YangCao, 2015/05/13

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef _CL_SYNTAX_H
#define _CL_SYNTAX_H

#include "CL_Lex.h"

#include <stack>
#include "CL_Coff.h"
#include "CL_CodeGen.h"

// @Class analyze source code syntax 
class Syntax
{
public:
	typedef std::stack<Symbol*> SymbolStack;

public:
	// @Function init syntax
	int init_syntax(const char* src_path);

	// @Function start to translation source code
	void translation_unit();

protected:
	// @Function insert symbol to stack
	// @Return inserted symbol
	// @Param ss want insert which symbol stack
	// @Param v word index
	// @Param type the pointer to data type
	// @Param c symbol related value
	Symbol* sym_direct_push(SymbolStack& ss, int v, Type* type, int c);

	// @Function insert symbol
	// @Return inserted symbol pointer
	// @Param v word index
	// @Param type the pointer to data type
	// @Param r the related register
	// @Param c symbol related value
	Symbol* sym_push(int v, Type* type, int r, int c);

	// @Function push function symbol
	// @Param v word index
	// @Param type the pointer to data type
	Symbol* func_sym_push(int v, Type* type);

	// @Function push variable symbol
	// @Param type the pointer to data type
	// @Param r the related register
	// @Param v word index
	// @Param addr variable address
	Symbol* var_sym_put(Type* type, int r, int v, int addr);

	// @Function put section name to global stack
	// @Param sec the section name
	// @Param c related value
	Symbol* sec_sym_put(const char* sec, int c);

	// @Function popup symbol until symbol b
	// @Param ss the pop statck
	// @Param b the stop pop symbol
	void sym_pop(SymbolStack& ss, Symbol* b);

	// @Function find the struct symbol by word index
	// @Param v the word index
	Symbol* struct_search(int v);

	// @Function find the identifier symbol by word index
	// @Param v the word index
	Symbol* sym_search(int v);

protected:
	// @Function translation external declaration
	// @Param space current code is in global space or function space (like express what in function body )
	void external_declatation(e_StorageClass space);

	// @Function translation struct specifier
	// @Param type:out data type
	void struct_specifier(Type *type);

	// @Function translation struct declaration list
	// @Param type:out data type
	void struct_declaration_list(Type *type);

	// @Function struct declaration
	// @Param maxalign 
	// @Param offset
	// @Param ps
	void struct_declaration(int *maxalign, int *offset, Symbol***ps);

	// @Function translation function calling convention
	// @Param fc [out] call convention type
	void function_calling_convention(int* fc);

	// @Function translation struct member alignment
	// @Param force_align force align
	void struct_member_alignment(int *force_align);

	// @Function translation declarator
	// @Param type:out
	// @Param v
	// @Param force_align
	void declarator(Type *type, int *v, int *force_align);

	// @Function direct declarator
	// @Param type:out
	// @Param v:out
	// @Param func_call
	void direct_declarator(Type* type, int *v, int func_call);

	// @Function direct declarator postfix
	// @Param type:out
	// @Param func_call
	void direct_declarator_postfix(Type* type, int func_call);

	// @Function parameter type list
	// @Param type:out
	// @Param func_call function call convention
	void parameter_type_list(Type* type, int func_call);

	// @Function function body
	// @Param sym symbol
	void funcbody(Symbol* sym);

	// @Function generate function begin code
	// @Param func_type function type
	void gen_prolog(Type * func_type);

	// @Function generate function end code
	void gen_epilog();

	// @Function initializer
	// @Param type
	// @Param c the variable related value
	// @Param sec the section where the variable in
	void initializer(Type* type, int c, Section* sec);

	// @Function initialize variable
	// @Param type variable type
	// @Param sec section where the variable in
	// @Param c the variable related value
	// @Param v the word index
	void init_variable(Type* type, Section* sec, int c, int v);

	// @Function translation statement
	// @Param bsym
	// @Param csym
	void statement(int* bsym, int* csym);

	// @Function compound statement
	// @Param bysm 
	// @Param csym
	void compound_statement(int *bsym, int *csym);

	// @Function expression statement
	void expression_statement();

	// @Function if statement
	// @Param bsym break jump position
	// @Param csym continue jump position
	void if_statement(int* bsym, int *csym);

	// @Function for statement
	void for_statement(int* bsym, int *csym);

	// @Function continue statement
	// @Param csym continue jump position
	void continue_statement(int *csym);

	// @Function break statement
	// @Param bsym break jump position
	void break_statement(int * bsym);

	// @Function return statement
	void return_statement();

	// @Function expression
	void expression();

	// @Function assignment expression
	void assignment_expression();

	// @Function equality expression
	void equality_expression();

	// @Function relational expression
	void relational_expression();

	// @Function additive expression
	void additive_expression();

	// @Function multiplication expression
	void mutiplicative_expression();

	// @Function unary expression
	void unary_expression();

	// @Function sizeof expression
	void sizeof_expression();

	// @Function postfix expression
	void postfix_expression();

	// @Function primary expression
	void primary_expression();

	// @Function argument expression list
	void argument_expression_list();
protected:
	// @Function allocate memory space
	// @Return the section which store this variable
	// @Param type variable type
	// @Param r variable storage class
	// @Param has_init is init?
	// @Param v word index
	// @Param addr variable store address
	Section* allocate_storage(Type* type, int r, int has_init, int v, int *addr);

	// @Function fill back, put in head of t every pending address fill determine address
	// @Param t list head
	// @Param a instruction jump position
	void backpatch(int t, int a);

	// @Function generate pointer type
	// @Param t source data type
	void mk_pointer(Type* t);
public:
	// @Function current token is specifier and get a token
	// @Param type:out data type
	bool type_specifier(Type* type);

	// @Function sense is specifier
	// @Param _token token
	bool is_type_specifier(int _token);
public:
	// @Property Coff
	Coff coff;
protected:
	// @Property lex parser
	Lexer lex;

	// @property codegen pointer
	CodeGen codegen;

	// @Property global symbol stack
	SymbolStack global_sym_stack;

	// @Property local symbol stack
	SymbolStack local_sym_stack;

	// @Property read only section symbol
	Symbol * sym_sec_rdata;
};

#endif