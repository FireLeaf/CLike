/*******************************************************************************
FILE:		CL_Syntax.cpp

DESCRIPTTION:

CREATED BY: YangCao, 2015/05/13

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#include "CL_Syntax.h"
#include "CL_Lex.h"
#include "CL_Util.h"
#include <assert.h>
using namespace Util;

bool Syntax::type_specifier(Type* type)
{
	bool is_specifier = true;
	e_TypeCode t;
	Type type_struct;

	switch (token)
	{
	case KW_CHAR:
		t = T_CHAR;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_SHORT:
		t = T_SHORT;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_VOID:
		t = T_VOID;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_INT:
		t = T_INT;
		syntax_state = SNTX_SP;
		lex.get_token();
		break;
	case KW_STRUCT:
		t = T_STRUCT;
		syntax_state = SNTX_SP;
		struct_specifier(&type_struct);
		type->ref = type_struct.ref;
		break;
	default:
		is_specifier = false;
		break;
	}
	
	type->t = t;
	return is_specifier;
}

bool Syntax::is_type_specifier(int _token)
{

	bool is_specifier = true;
	switch (_token)
	{
	case KW_CHAR:
	case KW_SHORT:
	case KW_VOID:
	case KW_INT:
	case KW_STRUCT:
		break;
	default:
		is_specifier = false;
		break;
	}
	return is_specifier;
}

int Syntax::type_size(Type *t, int *a)
{
	Symbol *s;
	int bt;
	//pointer type length is 4
	int PTR_SIZE = 4;

	bt = t->t & T_BTYPE;

	switch (bt)
	{
	case T_STRUCT:
		s = t->ref;
		*a = s->r;
		return s->c;
	case T_PTR:
		if ( t->t & T_ARRAY )
		{
			s = t->ref;
			return type_size(&s->type, a) * s->c;
		}
		else
		{
			*a = PTR_SIZE;
			return PTR_SIZE;
		}
	case T_INT:
		*a = 4;
		return 4;
	case T_SHORT:
		*a = 2;
		return 2;
	default://char,void, function
		*a = 1;
		return 1;
	}
}

int Syntax::init_syntax(const char* src_path)
{
	if ( -1 == lex.init_lex(src_path))
	{ 
		return -1;
	}
	lex.get_char();
	lex.get_token();
	return 1;
}

void Syntax::translation_unit()
{
	while ( token != TK_EOF )
	{
		// in global space translation
		external_declatation(SC_GLOBAL);
	}
}

Symbol* Syntax::sym_direct_push(SymbolStack& ss, int v, Type* type, int c)
{
	Symbol *s = new Symbol;
	s->v = v;
	s->c = c;
	s->type = type;
	s->next = NULL;
	s->prev_tok = NULL;
	ss.push(s);
	return s;
}

Symbol* Syntax::sym_push(int v, Type* type, int r, int c)
{
	Symbol *ps = NULL, **pps = NULL;
	TKWord *ts = NULL;
	
	if (!local_sym_stack.empty())
		ps = sym_direct_push(local_sym_stack, v, type, c);
	else
		ps = sym_direct_push(global_sym_stack, v, type, c);

	ps->r = r;

	if ( (v & SC_STRUCT) || v < SC_ANOM)
	{
		ts = lex.get_tkword(v & ~SC_STRUCT);
		if (v & SC_STRUCT)
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_identifier;
		ps->prev_tok = *pps;
		*pps = ps;
	}
}

Symbol* Syntax::func_sym_push(int v, Type* type)
{
	Symbol *s = NULL, **ps = NULL;
	s = sym_direct_push(global_sym_stack, v, type, 0);

	ps = &(lex.get_tkword(v)->sym_identifier);
	
	// insert the symbol to the same name symbol list end
	while (*ps)
		ps = &((*ps)->prev_tok);

	s->prev_tok = NULL;
	*ps = s;
	return s;
}

Symbol* Syntax::var_sym_put(Type* type, int r, int v, int addr)
{
	Symbol* sym = NULL;

	if ( (r & SC_VALMASK) == SC_LOCAL )
	{
		sym = sym_push(v, type, r, addr);
	}
	else if ( (r & SC_VALMASK) == SC_GLOBAL )
	{
		sym = sym_search(v);
		if (sym)
			error("%s redefinition\n", lex.get_tkword(v)->words);
		else
		{
			sym = sym_push(v, type, r | SC_SYM, 0);
		}
	}

	return sym;
}

Symbol* Syntax::sec_sym_put(const char* sec, int c)
{
	TKWord* tp;
	Symbol* s;
	Type type;
	type.t = T_INT;
	tp = lex.word_table.insert(sec);
	token = tp->tkcode;
	s = sym_push(token, &type, SC_GLOBAL, c);
	return s;
}

void Syntax::sym_pop(SymbolStack& ss, Symbol* b)
{
	Symbol *s, **ps;
	TKWord *ts;
	int v;

	s = ss.top();
	while ( s != b)
	{
		v = s->v;

		// if the pop symbol is struct or identifier, let symbol pointer the prev same name symbol
		if ( (v & SC_STRUCT) || v < SC_ANOM )
		{
			ts = lex.get_tkword(v & ~SC_STRUCT);
			if (v & SC_STRUCT)
				ps = &ts->sym_struct;
			else
				ps = &ts->sym_identifier;

			*ps = s->prev_tok;
		}

		ss.pop();
		delete s;// release symbol
		if (ss.empty())
			break;

		s = ss.top();
	}
}

Symbol* Syntax::struct_search(int v)
{
	if ( v >= lex.word_table.tk_wordtable.size())
	{
		return NULL;
	}
	return lex.get_tkword(v)->sym_struct;
}

Symbol* Syntax::sym_search(int v)
{
	if (v >= lex.word_table.tk_wordtable.size())
	{
		return NULL;
	}
	return lex.get_tkword(v)->sym_identifier;
}

void Syntax::external_declatation(e_StorageClass space)
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol* sym;

	if (!type_specifier(&btype))
	{
		expect("<specifer>");
	}

	// null statement
	if ( btype.t == T_STRUCT && token == TK_SEMICOLON)
	{
		lex.get_token();
		return;
	}

	while (true)
	{
		type = btype;
		declarator(&type, &v, NULL);

		if ( token == TK_BEGIN ) // parser the function
		{
			if (space == SC_LOCAL)
				error("nonsupport function nest function");
			
			if ((type.t & T_BTYPE) != T_FUNC)
				expect("<function definition>");

			sym = sym_search(v);
			if (sym)//函数前面声明过，现在是函数定义
			{
				if ((sym->type.t & T_BTYPE) != T_FUNC)
					error("'%s' redefine", get_tkstr(v));
				sym->type = type;
			}
			else
				sym = func_sym_push(v, &type);
			funcbody(sym);
			break;
		}
		else
		{
			//函数声明
			if ( (type.t & T_BTYPE) == T_FUNC )
			{
				if (sym_search(v) == NULL)
				{
					sym = sym_push(v, &type, SC_GLOBAL | SC_SYM, 0);
				}
			}
			else //变量声明
			{
				r = 0;
				if ( !(type.t & T_ARRAY) )
					r |= SC_LVAL;
				
				r |= 1;
				has_init = (token == TK_ASSIGN);

				if (has_init)//初值表达式
				{
					lex.get_token();
					initializer(&type);
				}
				sym = var_sym_put(&type, r, v, addr);
			}

			if (token == TK_COMMA)
			{
				lex.get_token();
			}
			else
			{
				syntax_state = SNTX_LF_HT;
				lex.skip(TK_SEMICOLON);
				break;
			}
		}
	}
}

void Syntax::struct_specifier(Type* type)
{
	lex.get_token();
	int v = token;
	Symbol *s;
	Type type_struct;
	
	syntax_state = SNTX_DELAY;
	lex.get_token();
	
	if (token == TK_BEGIN) // struct definition
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA) // sizeof (struct name)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP;

	lex.syntax_indent();

	if (v < TK_IDENT)
		expect("struct name is specifier");

	s = struct_search(v);
	if (!s)
	{
		type_struct.t = T_STRUCT;//....
		// -1赋值给s->c，表示该结构体尚未定义
		s = sym_push(v | SC_STRUCT, &type_struct, 0, -1);
		s->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = s;

	if (token == TK_BEGIN)
		struct_declaration_list(type);
}

// @Function calculate struct align 
// @Param n unalign value
// @Param align
int calc_align(int n, int align)
{
	return ( (n + align -1) & (align - 1) );
}

void Syntax::struct_declaration_list(Type *type)
{
	int maxalign = 0, offset = 0;
	Symbol *s, **ps;
	s = type->ref;

	syntax_state = SNTX_LF_HT;	// the first struct member is different line with '{'
	syntax_level++;				// indent increase one

	lex.get_token();

	if (s->c != -1) //s->c记录结构体尺寸
		error("struct already definition");
	maxalign = 1;
	ps = &s->next;
	offset = 0;

	while (token != TK_END)
	{
		struct_declaration(&maxalign, &offset, &ps);
	}

	lex.skip(TK_END);// 

	s->c = calc_align(offset, maxalign);//calculate struct size
	s->r = maxalign;
	syntax_state = SNTX_LF_HT;
}

void Syntax::struct_declaration(int *maxalign, int *offset, Symbol***ps)
{
	int v, size, align;
	Symbol *ss;
	Type type_sub, btype;
	int force_align;

	//
	if (!type_specifier(&btype))
	{
		// To do here...
	}

	while (true)
	{
		v = 0;
		type_sub = btype;
		declarator(&type_sub, &v, &force_align);

		size = type_size(&type_sub, &align);

		if (force_align & ALIGN_SET)
			align = force_align & ~ALIGN_SET;

		*offset = calc_align(*offset, align);

		if (align > *maxalign)
			*maxalign = align;

		ss = sym_push(v | SC_MEMBER, &type_sub, 0, *offset);
		*offset += size;
		**ps = ss;
		*ps = &&ss->next;

		if (token == TK_SEMICOLON)
			break;
		lex.skip(TK_COMMA);
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::function_calling_convention(int* fc)
{
	assert(fc);

	if (token == KW_CDECL || token == KW_STDCALL)
	{
		*fc = token;
		syntax_state = SNTX_SP;
		lex.get_token();
	}
	else
		*fc = KW_CDECL;
}

void Syntax::struct_member_alignment(int * force_align)
{
	int align = 1;
	if (token == KW_ALIGN)
	{
		lex.get_token();
		lex.skip(TK_OPENPA);// "{"

		if (token == TK_CINT)
		{
			lex.get_token();
			align = tkvalue;
		}
		else
			expect("integer number");
		lex.skip(TK_CLOSEPA);//"}"

		if (align != 1 && align != 2 && align != 4)
			align = 1;
		align |= ALIGN_SET;
		*force_align = align;
	}
	else
		*force_align = 1;
}

void Syntax::declarator(Type *type, int *v, int *force_align)
{
	int fc;
	while ( token == TK_STAR)
	{
		mk_pointer(type);
		lex.get_token();
	}

	// check function call convention
	function_calling_convention(&fc);

	// check member alignment
	if (force_align)
		struct_member_alignment(force_align);

	direct_declarator(type, v, fc);
}

void Syntax::direct_declarator(Type* type, int *v, int func_call)
{
	if (token >= TK_IDENT)
	{
		*v = token;
		lex.get_token();
	}
	else
	{
		expect("identifier");
	}

	direct_declarator_postfix(type, func_call);
}

void Syntax::direct_declarator_postfix(Type* type, int func_call)
{
	int n;
	Symbol *s;

	if (token == TK_OPENPA)
	{
		parameter_type_list(type, func_call);
	}
	else if (token == TK_OPENBR)
	{
		lex.get_token();
		n = -1;
		if (token == TK_CINT)
		{
			lex.get_token();
			n = tkvalue;
		}
		lex.skip(TK_CLOSEBR);
		direct_declarator_postfix(type, func_call);
		s = sym_push(SC_ANOM, type, 0, n);
		type->t = T_ARRAY | T_PTR;
		type->ref = s;
	}
}

void Syntax::parameter_type_list(Type* type, int func_call)
{
	int n;
	Symbol **plast, *s, *first;
	Type pt;
	
	lex.get_token();
	first = NULL;
	plast = &first;

	lex.get_token();
	while (token != TK_CLOSEPA)
	{
		if (!type_specifier(&pt))
			error("unknown type specifier");
		
		declarator(&pt, &n, NULL);
		s = sym_push(n | SC_PARAMS, &pt, 0, 0);
		*plast = s;
		plast = &s->next;
		if (token == TK_CLOSEPA)
			break;

		lex.skip(TK_COMMA);

		if (token == TK_ELLIPSIS)
		{
			func_call = KW_CDECL;
			lex.get_token();
			break;
		}
	}

	syntax_state = SNTX_DELAY;
	lex.skip(TK_CLOSEPA);

	//此处将函数返回类型存储，然后指向参数，最后type设为函数类型引用的相关信息放在ref中
	s = sym_push(SC_ANOM, type, func_call, 0);
	s->next = first;
	type->t = T_FUNC;
	type->ref = s;

	if (token == TK_BEGIN)
		syntax_state = SNTX_LF_HT;
	else
		syntax_state = SNTX_NUL;

	lex.syntax_indent();
}

void Syntax::funcbody(Symbol* sym)
{
	//放一匿名符号在局部符号表中
	sym_direct_push(local_sym_stack, SC_ANOM, &int_type, 0);
	compound_statement(NULL, NULL);//
	//清空局部符号栈
	sym_pop(local_sym_stack, NULL);
}

void Syntax::initializer(Type* type)
{
	if ( type->t & T_ARRAY)
	{
		lex.get_token();
	}
	else
		assignment_expression();
}

void Syntax::statement(int* bsym, int* csym)
{
	switch (token)
	{
	case TK_BEGIN:
		compound_statement(/*bsym, csym*/);
		break;
	case KW_IF:
		if_statement(/*bsym*/);
		break;
	case KW_RETURN:
		return_statement();
		break;
	case KW_BREAK:
		break_statement(/*bsym*/);
		break;
	case KW_CONTINUE:
		continue_statement(/*csym*/);
		break;
	case KW_FOR:
		for_statement(/*bsym, csym*/);
		break;
	default:
		expression_statement();
		break;
	}
}

//复合语句
void Syntax::compound_statement(int *bsym, int *csym)
{
	Symbol *s;
	s = local_sym_stack.top();
	syntax_state = SNTX_LF_HT;
	syntax_level++;

	lex.get_token();
	while (is_type_specifier(token))
	{
		external_declatation(SC_LOCAL);
	}

	while ( token != TK_END)
	{
		statement(NULL, NULL);
	}

	syntax_state = SNTX_LF_HT;
	
	sym_pop(local_sym_stack, s);
	lex.get_token();
}


void Syntax::expression_statement()
{
	if (token != TK_SEMICOLON)
	{
		expression();
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::if_statement()
{
	syntax_state = SNTX_SP;
	lex.get_token();
	lex.skip(TK_OPENPA);
	expression();
	syntax_state = SNTX_LF_HT;
	lex.skip(TK_CLOSEPA);
	statement(NULL, NULL);
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		lex.get_token();
		statement(NULL, NULL);
	}
}

void Syntax::for_statement()
{
	lex.get_token();
	lex.skip(TK_OPENPA);
	if (token != TK_SEMICOLON)
	{
		expression();
	}

	lex.skip(TK_SEMICOLON);
	if (token != TK_SEMICOLON)
	{
		expression();
	}

	lex.skip(TK_SEMICOLON);
	if (token != TK_CLOSEPA)
	{
		expression();
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_CLOSEPA);
	statement(NULL, NULL);
}

void Syntax::continue_statement()
{
	lex.get_token();
	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::break_statement()
{
	lex.get_token();
	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::return_statement()
{
	syntax_state = SNTX_DELAY;
	lex.get_token();
	if ( token == TK_SEMICOLON)
	{
		syntax_state = SNTX_NUL;
	}
	else
	{
		syntax_state = SNTX_SP;
	}

	lex.syntax_indent();

	if (token != TK_SEMICOLON)
	{
		expression();
	}

	syntax_state = SNTX_LF_HT;
	lex.skip(TK_SEMICOLON);
}

void Syntax::expression()
{
	while (true)
	{
		assignment_expression();
		if (token != TK_COMMA)
		{
			break;
		}
		lex.get_token();
	}
}

void Syntax::assignment_expression()
{
	equality_expression();
	if (token == TK_ASSIGN)
	{
		lex.get_token();
		assignment_expression();
	}
}

void Syntax::equality_expression()
{
	relational_expression();
	while (token == TK_EQ || token == TK_NEQ)
	{
		lex.get_token();
		relational_expression();
	}
}

void Syntax::relational_expression()
{
	additive_expression();
	while (token == TK_LT || token == TK_LEQ || token == TK_GT || token == TK_GEQ)
	{
		lex.get_token();
		additive_expression();
	}
}

void Syntax::additive_expression()
{
	mutiplicative_expression();
	while (token == TK_PLUS || token == TK_MINUS)
	{
		lex.get_token();
		mutiplicative_expression();
	}
}

void Syntax::mutiplicative_expression()
{
	unary_expression();
	while ( token == TK_STAR || token == TK_DIVIDE || token == TK_MOD)
	{
		lex.get_token();
		unary_expression();
	}
}

void Syntax::unary_expression()
{
	switch (token)
	{
	case TK_AND:
	case TK_STAR:
	case TK_PLUS:
	case TK_MINUS:
		lex.get_token();
		unary_expression();
		break;
	case KW_SIZEOF:
		sizeof_expression();
		break;
	default:
		postfix_expression();
		break;
	}
}

void Syntax::sizeof_expression()
{
	int align, size;
	Type type;

	lex.get_token();
	lex.skip(TK_OPENPA);
	type_specifier(&type);
	lex.skip(TK_CLOSEPA);

	size = type_size(&type, &align);
	if (size < 0)
		error("calculate type size failed!");
}

void Syntax::postfix_expression()
{
	primary_expression();
	while (true)
	{
		if (token == TK_DOT || token == TK_POINTSTO)
		{
			lex.get_token();
			token |= SC_MEMBER;
			lex.get_token();
		}
		else if (token == TK_OPENBR)
		{
			lex.get_token();
			expression();
			lex.skip(TK_CLOSEBR);
		}
		else if (token == TK_OPENPA)
		{
			argument_expression_list();
		}
		else
			break;
	}
}

void Syntax::primary_expression()
{
	int t,addr;
	Type type;
	Symbol *s;

	switch (token)
	{
	case TK_CINT:
	case TK_CCHAR:
		lex.get_token();
		break;
	case TK_CSTR:
		//lex.get_token();
		t = T_CHAR;
		type.t = t;
		mk_pointer(&type);
		type.t |= T_ARRAY;
		var_sym_put(&type, SC_GLOBAL, 0, addr);
		initializer(&type);
		break;
	case TK_OPENPA:
		lex.get_token();
		expression();
		lex.skip(TK_CLOSEPA);
		break;
	default:
		t = token;
		lex.get_token();
		if (t < TK_IDENT)
			expect("identify or constant");
		
		s = sym_search(t);
		if (!s)
		{
			if (token != TK_OPENPA)
				error("'%s' undeclare", get_tkstr(t));

			s = func_sym_push(t, &default_func_type);//允许函数不声明，直接引用
			s->r = SC_GLOBAL | SC_SYM; 
		}

		break;
	}
}

void Syntax::argument_expression_list()
{
	lex.get_token();
	if (token != TK_CLOSEPA)
	{
		for (;;)
		{
			assignment_expression();
			if (token == TK_CLOSEPA)
			{
				break;
			}
			lex.skip(TK_COMMA);
		}
	}

	lex.skip(TK_CLOSEPA);
}