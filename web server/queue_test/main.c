#include <stdio.h>
#include <assert.h>
#include "queue.h"

#define PRINT_TEST(val1, val2)    \
    {                             \
        assert((val1) == (val2)); \
    }

#define PRINT_SUCCESS(val1)                \
    {                                      \
        printf("TEST %d PASSED!\n", val1); \
    }

// printf("expected %d got %d\n", val1, val2);
//  }

#define PRINT_BARRIER                       \
    {                                       \
        printf("----------------------\n"); \
    }
#define PRINT_BARRIER_LONG                                                              \
    {                                                                                   \
        printf("------------------------------------------------------------------\n"); \
    }

void test1(Node *first, Node *last)
{
    PRINT_TEST(0, get_size(first));
    for (int i = 0; i < 15; i++)
    {
        PRINT_TEST(ZEVEL, pop_from(first, last, 0));
    }
    PRINT_TEST(1, pop_last(first, last) == NULL);

    for (int i = 0; i < 15; i++)
    {
        push_data(first, last, i);
    }
    PRINT_TEST(15, get_size(first));

    // printf("%d\n", pop_from(first, last, 0));
    // printf("%d\n", pop_last(first, last));
    PRINT_TEST(-10, pop_from(first, last, 25));
    PRINT_TEST(15, get_size(first));
    for (int i = 0; i < 15; i++)
    {
        PRINT_TEST(i, pop_from(first, last, 0));
    }
    PRINT_TEST(-10, pop_from(first, last, -500));
    PRINT_TEST(0, get_size(first));
    PRINT_SUCCESS(1);
    PRINT_BARRIER
}

void test2(Node *first, Node *last)
{
    for (int i = 0; i < 15; i++)
    {
        push_data(first, last, i);
    }
    for (int i = 0; i < 15; i++)
    {
        PRINT_TEST(i, pop_first(first, last)->data);
    }
    for (int i = 0; i < 15; i++)
    {
        push_data(first, last, i);
    }
    for (int i = 0; i < 15; i++)
    {
        PRINT_TEST(14 - i, pop_last(first, last)->data);
    }

    PRINT_SUCCESS(2);
    PRINT_BARRIER
}

void test3(Node *first, Node *last)
{
    Node *cur;
    for (int i = 0; i < 15; i++)
    {
        push_data(first, last, i);
    }
    for (int i = 0; i < 15; i++)
    {
        cur = pop_first(first, last);
        push_node(first, last, cur);
    }
    PRINT_TEST(15, get_size(first));
    for (int i = 0; i < 15; i++)
    {
        PRINT_TEST(i, pop_first(first, last)->data);
    }
    PRINT_TEST(0, get_size(first));
    for (int i = 0; i < 15; i++)
    {
        push_data(first, last, i);
    }
    for (int i = 0; i < 15; i++)
    {
        cur = pop_first(first, last);
        push_node(first, last, cur);
        PRINT_TEST(cur, pop_node(first, last, cur));
    }
    PRINT_TEST(0, get_size(first));
    PRINT_SUCCESS(3);
    PRINT_BARRIER
}

int main()
{
    // get q specs
    Node *first = init_queue();
    Node *last = first->next;
    PRINT_BARRIER_LONG
    test1(first, last);
    test2(first, last);
    test3(first, last);
    PRINT_BARRIER_LONG

    return 0;
}