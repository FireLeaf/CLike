/*******************************************************************************
	FILE:		CL_Hash.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/12

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_CL_HASH_H_
#define __COCONAT_CL_HASH_H_

#include <vector>
#include <string>

#include "CL_CompDef.h"
#include "CL_Util.h"

#define MAXKEY 1024

struct TKWord 
{
	int tkcode;
	TKWord* next;
	char* words;
	Symbol* sym_struct;
	Symbol* sym_identifier;
	//TKWord(){ memset(this, 0, sizeof(TKWord)); }
};

struct TKHashtable
{
	TKHashtable()
	{
		for (int i = 0; i < MAXKEY; i++)
		{
			tk_hashtable[i] = nullptr;
		}
	}

	~TKHashtable()
	{
		for (int i = 0; i < MAXKEY; i++)
		{
			TKWord* p = tk_hashtable[i];
			while (p != nullptr)
			{
				TKWord* q = p->next;
				if (p->tkcode >= TK_IDENT)
					free(p);
				p = q;
			}
			tk_hashtable[i] = NULL;
// 			if (tk_hashtable[i])
// 			{
// 				free(tk_hashtable[i]);
// 				tk_hashtable[i] = NULL;
// 			}
		}
	}

	void direct_insert(TKWord* tp)
	{
		int keyno = 0;
		tk_wordtable.push_back(tp);
		keyno = Util::elf_hash(tp->words);
		tp->next = tk_hashtable[keyno];
		tk_hashtable[keyno] = tp;
	}

	TKWord* find(const char* words, int keyno)
	{
		TKWord* p = NULL;
		for (p = tk_hashtable[keyno]; p; p = p->next)
		{
			if (0 == strcmp(words, p->words))
			{
				return p;
			}
		}
		return p;
	}

	TKWord* insert(const char* words)
	{
		TKWord* tp = NULL;
		int keyno = 0;
		char* s = NULL;
		char* end = NULL;
		int length = 0;
		
		keyno = Util::elf_hash(words);
		tp = find(words, keyno);
		if (tp)
		{
			return tp;
		}
		length = strlen(words);
		tp = reinterpret_cast<TKWord*>(Util::mallocz(sizeof(TKWord) + length + 1));
		tp->next = tk_hashtable[keyno];
		tk_hashtable[keyno] = tp;
		tk_wordtable.push_back(tp);
		tp->tkcode = tk_wordtable.size() - 1;
		s = (char*)tp + sizeof(TKWord);
		tp->words = s;
		strcpy(s, words);
		s[length] = '\0';
		
		return tp;
	}

	const char* get_word_string(int idx)
	{
		return tk_wordtable[idx] ? tk_wordtable[idx]->words : NULL;
	}
public:
	TKWord* tk_hashtable[MAXKEY];
	std::vector<TKWord*> tk_wordtable;
};

#endif