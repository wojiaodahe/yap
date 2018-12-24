#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

struct list_head
{
	struct list_head *next, *prev;
};

#define offsetof(type, member) ((unsigned int)&((type *)0)->member)

#define container_of(ptr, type, member) \
		(type *)((char *)(ptr) - offsetof(type, member));

#define list_entry(list_ptr, struct_type, member_name)\
        container_of(list_ptr, struct_type, member_name)

#define list_for_each(pos, head)\
	for (pos = (head)->next; pos != (head); pos = pos->next)

void INIT_LIST_HEAD(struct list_head *list);
void __list_add(struct list_head *new_lst, struct list_head *prev, struct list_head *next);
void list_add(struct list_head *new_lst, struct list_head *head);
void list_add_tail(struct list_head *new_lst, struct list_head *head);
void __list_del(struct list_head * prev, struct list_head * next);
void list_del(struct list_head * entry);
void list_remove_chain(struct list_head *ch,struct list_head *ct);
void list_add_chain(struct list_head *ch,struct list_head *ct,struct list_head *head);
void list_add_chain_tail(struct list_head *ch,struct list_head *ct,struct list_head *head);
int list_empty(const struct list_head *head);

#endif

