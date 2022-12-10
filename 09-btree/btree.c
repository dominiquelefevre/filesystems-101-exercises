#include "solution.h"

#include <stdio.h>
#include <stdlib.h>

struct btree
{
	struct btree *left, *right;
	int *content
	
};

struct btree* btree_alloc(unsigned int L)
{
	if (L==0)
	{
		return NULL;
	}
	struct btree *tr1 = (struct btree *)malloc(sizeof(struct btree));
	if (tr1 == NULL)
	{
		return errno;
	}
	if (L != 0)
	{
		tr1->right = btree_alloc((L - 1) / 2);
		tr1->left = btree_alloc((L - 1) / 2);
		
	}
	tr1 ->content = NULL:
	//(void) L;

	return tr1;
}

void btree_free(struct btree *t)
{
	//checking t
	if (t == NULL)
	{
		//exit();
		return;
	}
	if (t != NULL)
	{
	free(t->content);
	btree_free(t->right);
	btree_free(t->left);
	free(t);
	}
	//(void) t;
	return;
}

void btree_insert(struct btree *t, int x)
{
	//(void) t;
	//(void) x;
	if (x == NULL
}

void btree_delete(struct btree *t, int x)
{
	(void) t;
	(void) x;
}

bool btree_contains(struct btree *t, int x)
{
	(void) t;
	(void) x;

	return false;
}

struct btree_iter
{
};

struct btree_iter* btree_iter_start(struct btree *t)
{
	(void) t;

	return NULL;
}

void btree_iter_end(struct btree_iter *i)
{
	(void) i;
}

bool btree_iter_next(struct btree_iter *i, int *x)
{
	(void) i;
	(void) x;

	return false;
}
