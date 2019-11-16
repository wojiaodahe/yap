#include "list.h"
#include "assert.h"

void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

void __list_add(struct list_head *new_lst, struct list_head *prev, struct list_head *next)
{
    next->prev = new_lst;
    new_lst->next = next;
    new_lst->prev = prev;
    prev->next = new_lst;
}

void list_add(struct list_head *new_lst, struct list_head *head)
{
    __list_add(new_lst, head, head->next);
}

void list_add_tail(struct list_head *new_lst, struct list_head *head)
{
    __list_add(new_lst, head->prev, head);
}

void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

void list_del(struct list_head * entry)
{
    check_addr(entry);

    __list_del(entry->prev,entry->next);

    entry->next = entry;
    entry->prev = entry;
}


void list_remove_chain(struct list_head *ch,struct list_head *ct)
{
    ch->prev->next=ct->next;
    ct->next->prev=ch->prev;
}

void list_add_chain(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
    ch->prev=head;
    ct->next=head->next;
    head->next->prev=ct;
    head->next=ch;
}

void list_add_chain_tail(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
    ch->prev=head->prev;
    head->prev->next=ch;
    head->prev=ct;
    ct->next=head;
}

int list_empty(const struct list_head *head)
{
    return head->next == head;
}

