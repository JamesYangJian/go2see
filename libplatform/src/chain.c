/**************************************************************************************/
// File Name: chain.c
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

#include <stdio.h>

#include "chain.h"


/*Initialize a specified chain*/
void _init_chain(CHAIN_HEAD *phead)
{
    if (phead == NULL)
    {
        return;
    }

    phead->first = NULL;
    phead->last  = NULL;
}

/*Tail insert a node to the spcified list*/
int _insert_node_tail(CHAIN_HEAD *head, CHAIN_NODE *node)
{
    CHAIN_NODE *pLastNode = NULL;

    if (head == NULL || node == NULL)
    {
        return -1;
    }

    pLastNode = CHAIN_LAST(head);

    if (pLastNode == NULL)
    {
        head->first = node;
        head->last  = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        pLastNode->next = node;
        node->next = NULL;
        node->prev = pLastNode;
        head->last = node;
    }

    return 0;
}

/*Tail insert a node to the spcified list*/
int _dequeue_node(CHAIN_HEAD *pHead, CHAIN_NODE *pNode)
{
    CHAIN_NODE *pTmpNode = NULL;
    int        iRet      = -1;

    if (pHead == NULL || pNode == NULL)
    {
        return -1;
    }

    if (pNode == pHead->first)
    {
        //pNode is the first node
        if (pNode == pHead->last)
        {
            //pNode is the only node in the chain
            _init_chain((CHAIN_HEAD *)pHead);
        }
        else
        {
            pHead->first = pNode->next;
            pHead->first->prev = NULL;
            //pHead->first->next->prev = pHead->first;
        }
        iRet = 0;
    }
    else if (pNode == pHead->last)
    {
        //pNode is the last node
        pHead->last = pNode->prev;
        pHead->last->next = NULL;
        iRet = 0;
    }
    else
    {
        //pNode is the within node in the list
        pTmpNode = CHAIN_FIRST(pHead);
        while(pTmpNode != NULL && pTmpNode != pNode )
        {
            pTmpNode = pTmpNode->next;
        }
        if (pTmpNode == pNode)
        {
            //Find the specified node
            pNode->prev->next = pNode->next;
            pNode->next->prev = pNode->prev;
            iRet = 0;
        }
    }
    
    return iRet;
}

#if 0
/*free a common chain*/
void _free_chain(CHAIN_HEAD *phead)
{
    CHAIN_NODE *pNode = NULL;
    CHAIN_NODE *pTmpNode;

    if (phead == NULL)
    {
        return;
    }

    pNode = CHAIN_FIRST(phead);

    while(pNode != NULL)
    {
        pTmpNode = pNode;
        pNode = pNode->next;
        free(pTmpNode);
    }
}
#endif

/*pop head from chain*/
CHAIN_NODE* _pop_chain(CHAIN_HEAD *phead)
{
    CHAIN_NODE *pnode = NULL;

    if (phead == NULL)
    {
        return NULL;
    }

    pnode = CHAIN_FIRST(phead);
    if (pnode == NULL)
    {
        return NULL;
    }

    if (phead->first == phead->last)
    {
        phead->first    = NULL;
        phead->last     = NULL;
    }
    else
    {
        phead->first        = pnode->next;
        phead->first->prev  = NULL;
    }

    return pnode;
}

/*peek head from chain*/
CHAIN_NODE* peek_chain(CHAIN_HEAD *phead)
{
    if (phead == NULL)
    {
        return NULL;
    }

    return (CHAIN_NODE *)CHAIN_FIRST(phead);
}
