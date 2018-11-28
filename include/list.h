#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

struct list_head
{
	struct list_head *next, *prev;
};


static void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static  void __list_add(struct list_head *new_lst, struct list_head *prev, struct list_head *next)
{
	next->prev = new_lst;
	new_lst->next = next;
	new_lst->prev = prev;
	prev->next = new_lst;
}

static  void list_add(struct list_head *new_lst, struct list_head *head)
{
	__list_add(new_lst, head, head->next);
}

static  void list_add_tail(struct list_head *new_lst, struct list_head *head)
{
	__list_add(new_lst, head->prev, head);
}

static  void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static void list_del(struct list_head * entry)
{
	__list_del(entry->prev,entry->next);
}


static void list_remove_chain(struct list_head *ch,struct list_head *ct)
{
	ch->prev->next=ct->next;
	ct->next->prev=ch->prev;
}

static void list_add_chain(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
    ch->prev=head;
    ct->next=head->next;
    head->next->prev=ct;
    head->next=ch;
}

static void list_add_chain_tail(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
    ch->prev=head->prev;
    head->prev->next=ch;
    head->prev=ct;
    ct->next=head;
}

static int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#define offsetof(type, member) ((unsigned int)&((type *)0)->member)

#define container_of(ptr, type, member) \
		(type *)((char *)(ptr) - offsetof(type, member));

#define list_entry(list_ptr, struct_type, member_name)\
        container_of(list_ptr, struct_type, member_name)

#define list_for_each(pos, head)\
	for (pos = (head)->next; pos != (head); pos = pos->next)

#endif
