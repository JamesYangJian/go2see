/**************************************************************************************/
// File Name: chain.h
//
// Description:
//     This module provides some common chain management
//
// Author and Date:
//     yangjian 2008-12-04
//
// Version:
//     1.0 Raw edition
//
/**************************************************************************************/

#ifndef _CHAIN_H__
#define _CHAIN_H__

#include <semaphore.h>

/*common chain element*/
typedef struct _chain_node
{
    struct _chain_node *prev;
    struct _chain_node *next;
}CHAIN_NODE;

/*common chain management*/
typedef struct
{
    struct _chain_node *first;
    struct _chain_node *last;
    sem_t pop_sem;
    //sem_t pop_sem;
}CHAIN_HEAD;

typedef struct
{
    CHAIN_HEAD head;
    int count;
}S_COMMON_LIST;

#define CHAIN_FIRST(a) (a->first) 
#define CHAIN_LAST(a) (a->last)
#define CHAIN_NEXT(a) (a->next)

void _init_chain(CHAIN_HEAD *phead);
int _insert_node_tail(CHAIN_HEAD *head, CHAIN_NODE *node);
CHAIN_NODE* _pop_chain(CHAIN_HEAD *phead);
CHAIN_NODE* peek_chain(CHAIN_HEAD *phead);
int _dequeue_node(CHAIN_HEAD *pHead, CHAIN_NODE *pNode);
//void _free_chain(CHAIN_HEAD *phead);
#endif
