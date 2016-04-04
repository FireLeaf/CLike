/*******************************************************************************
FILE:		RBTree.h

DESCRIPTTION: implement red black tree

CREATED BY: YangCao, 2016/03/30

Copyright (C) - All Rights Reserved with Coconat
*******************************************************************************/

#ifndef _RBTREE_H_
#define _RBTREE_H_

#include <assert.h>
#include <string>

// @Enum node color
enum NodeColor
{
	RED		= 0,
	BLACK	= 1,
};

// @Function operator string less operation
bool operator < (const std::string& l, const std::string& r);

// @Structure tree node
template<typename K, typename V>
struct RBNode 
{
	//using SelfType = RBNode < K, V > ;
	typedef RBNode < K, V > SelfType;
	K key;
	V value;
	NodeColor color;
	SelfType* parent;
	SelfType* left_child;
	SelfType* right_child;

	RBNode(const K& _key = K(), const V& _value = V()) : key(_key), value(_value), color(RED), parent(NULL)
														, left_child(NULL), right_child(NULL){}
};

// @Class red black tree
template<typename K, typename V>
class RBTree
{
	typedef RBNode<K, V> NodeType;
public:
// #define COLOR(node) node->color
// #define LEFT_CHILD(node) node->left_child
// #define RIGTHT_CHILD(node) node->right_child

	// @Function node color
	// @Return return the node color
	// @Param node the node pointer
	NodeColor color(const NodeType* node) const
	{
		return node == nullptr ? BLACK : node->color;
	}

	// @Function fix up tree when insert node 
	// @Param node the inserted node just now
	void fixup_insert(NodeType* node)
	{
		while (color(node->parent) == RED)
		{
			NodeType* parent = node->parent;
			NodeType* grand_parent = parent->parent;
			if (parent == grand_parent->left_child)
			{
				NodeType* uncle = grand_parent->right_child;
				//assert(uncle != nullptr);
				if (color(uncle) == RED)
				{
					parent->color = BLACK;
					uncle->color = BLACK;
					grand_parent->color = RED;
					node = grand_parent;
				}
				else if (node == parent->right_child)
				{
					node = parent;
					left_rotate(node);
				}
				parent = node->parent;
				grand_parent = node->parent;
				
				parent->color = BLACK;
				grand_parent->color = RED;
				right_rotate(grand_parent);
			}
			else
			{
				NodeType* uncle = grand_parent->left_child;
				//assert(uncle != nullptr);
				if (color(uncle) == RED)
				{
					parent->color = BLACK;
					uncle->color = BLACK;
					grand_parent->color = RED;
					node = grand_parent;
				}
				else if (node == parent->left_child)
				{
					node = parent;
					right_rotate(node);
				}
				else
				{
					parent = node->parent;
					grand_parent = node->parent;

					parent->color = BLACK;
					grand_parent->color = RED;
					left_rotate(grand_parent);
				}
				
			}
		}
	}

	// @Function insert a node
	// @Param key the node key
	// @Param value the node value
	void insert_node(const K& key, const V& value)
	{
		if (nullptr == root)
		{
			// Root must be black
			root = new NodeType(key, value);
			root->color = BLACK;
		}
		else
		{
			// If exist this node, just update value
			NodeType* node = find(key);
			if (nullptr != node)
			{
				node->value = value;
				return;
			}

			// Create a new node and insert into tree
			node = new NodeType(key, value);
			NodeType* p = root;
			NodeType* q = nullptr;
			while (p)
			{
				q = p;
				if (node->key > p->key)
					p = p->right_child;
				else if (node->key < p->key)
					p = p->left_child;
				else
				{
					assert(0);
					break;
				}
			}

			if (node->key < q->key)
				q->left_child = node;
			else
				q->right_child = node;

			node->parent = q;

			fixup_insert(node);
		}
	}

	// @Function erase a node
	// @Param key node key
	void erase_node(const K& key)
	{

	}

	// @Function find a node
	// @Return the node pointer
	// @Param key node key
	NodeType* find(const K& key)
	{
		NodeType* p = root;

		while (p)
		{
			if (p->key == key) // find the same key
				return p;
			else if (p->key < key) // if key less than current node, find in left child tree
				p = p->left_child;
			else
				p = p->right_child; // if key more than current node, find in right child tree
		}
		return nullptr;
	}

protected:
	// @Function left rotate node
	// @Param x the node what will be left rotated
	void left_rotate(NodeType *x)
	{
		NodeType* y = x->right_child;
		x->right_child = y->left_child;
		if (y->left_child != nullptr)
			y->left_child->parent = x;

		y->parent = x->parent;
		if (x->parent == nullptr)
			root = y;
		else if (x == x->parent->left_child)
			x->parent->left_child = y;
		else
			x->parent->right_child = y;
		y->left_child = x;
		x->parent = y;
	}

	// @Function right rotate node
	// @Param node the node what will be right rotated
	void right_rotate(NodeType *y)
	{
		NodeType* x = y->left_child;
		y->left_child = x->right_child;
		if (x->right_child)
			x->right_child->parent = y;

		x->parent = y->parent;
		if (y->parent == nullptr)
			root = x;
		else if (y == y->parent->left_child)
			y->parent->left_child = x;
		else
			y->parent->right_child = x;

		x->right_child = y;
		y->parent = x;
	}
protected:

	// @Property the red black root node
	NodeType* root{nullptr};

};

#endif