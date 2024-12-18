#ifndef __QUEUEh__
#define __QUEUEh__
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#define ZEVEL -10

typedef struct Node
{
    int data;
    struct Node *next;
    struct Node *back;
    struct timeval arrival;
} Node;

Node *init_queue()
{
    Node *first = (Node *)malloc(sizeof(Node)); // first to enter
    Node *last = (Node *)malloc(sizeof(Node));  // last to enter
    first->data = ZEVEL;
    first->next = last;
    first->back = NULL;
    last->data = ZEVEL;
    last->next = NULL;
    last->back = first;
    return first;
}

void push_data(Node *first, Node *last, int data, struct timeval arrival)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->data = data;
    new_node->arrival = arrival;

    last->back->next = new_node;
    new_node->back = last->back;

    last->back = new_node;
    new_node->next = last;
}

void push_node(Node *first, Node *last, Node *new_node)
{
    last->back->next = new_node;
    new_node->back = last->back;

    last->back = new_node;
    new_node->next = last;
}

// will be used only for rand so nevermind the ret val
int pop_from(Node *first, Node *last, int pos)
{
    if (first->next == last)
        return ZEVEL;
    int i = 0;
    Node *cur = first->next;
    while (cur->next && cur->next != last && i < pos)
    {
        i++;
        cur = cur->next;
    }
    if (i != pos)
    {
        return ZEVEL; // pos illegal
    }

    cur->back->next = cur->next;
    cur->next->back = cur->back;

    return cur->data;
}

Node *pop_last(Node *first, Node *last)
{
    Node *cur = last->back;
    if (cur == first)
    {
        return NULL;
    }

    cur->back->next = last;
    last->back = cur->back;

    return cur;
}

Node *pop_first(Node *first, Node *last)
{
    Node *cur = first->next;
    if (cur == last)
    {
        return NULL;
    }

    first->next = cur->next;
    cur->next->back = first;

    return cur;
}

Node *pop_node(Node *first, Node *last, Node *node)
{
    if (first->next == last)
        return NULL;
    Node *cur = first->next;
    while (cur->next && cur->next != last && cur != node)
    {
        cur = cur->next;
    }
    if (cur != node)
    {
        return NULL; // node not found
    }

    cur->back->next = cur->next;
    cur->next->back = cur->back;

    return cur;
}

int get_size(Node *first)
{
    int i = -1;
    Node *cur = first;
    while (cur->next != NULL)
    {
        i++;
        cur = cur->next;
    }
    return i;
}

void print_queue(Node *first)
{
    Node *cur = first->next;
    int i = 1;
    while (cur->next && cur->next->next)
    {
        printf("node no. %d with data %d", i, cur->data);
        i++;
    }
    return;
}

#endif