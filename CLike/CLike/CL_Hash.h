/*******************************************************************************
	FILE:		CL_Hash.h
	
	DESCRIPTTION:	
	
	CREATED BY: YangCao, 2015/05/12

	Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef __COCONAT_CL_HASH_H_
#define __COCONAT_CL_HASH_H_

struct Symbol
{

};

#define MAXKEY 1024

int elf_hash(const char* key)
{
	int h = 0, g = 0;
	while (*key)
	{
		h = (h << 4) + *key++;
		g = h & 0xf00000000;
		if (g)
			h ^= g >> 24;
		h &= ~g;
	}
	return h % MAXKEY;
}

struct TKWord 
{
	int tkcode;
	TKWord* next;
	char* words;
	Symbol* sym_struct;
	Symbol* sym_identifier;
};

struct TKHashtable
{
	TKHashtable()
	{
		for (int i = 0; i < MAXKEY; i++)
		{
			tk_hashtable[i] = NULL;
		}
	}

	void direct_insert(TKWord* tp)
	{
		int keyno = 0;
		tk_wordtable.push_back(tp);
		keyno = elf_hash(tp->words);
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

	void insert(const char* words)
	{
		TKWord* tp = NULL;
		int keyno = 0;
		char* s = NULL;
		char* end = NULL;
		int length = 0;
		
		keyno = elf_hash(words);
		tp = find(words, keyno);
		if (tp)
		{
			return;
		}
		length = strlen(words);
		tp = malloc(sizeof(TKWord) + length + 1);
		tp->next = tk_hashtable[keyno];
		tk_hashtable[keyno] = tp;
		tk_wordtable.push_back(tp);
		tp->tkcode = tk_wordtable.size() - 1;
		s = (char*)tp + sizeof(TKWord);
		tp->words = s;
		strcpy(s, words);
		s[length] = '\0';
	}
protected:
	TKWord* tk_hashtable[MAXKEY];
	std::vector<TKWord*> tk_wordtable;
};

#endif