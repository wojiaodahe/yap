#include "kmalloc.h"
#include "common.h"
//#include "head.h"

//#include "list.h"
extern const unsigned int KMALLOC_ADDR_START;
extern const unsigned int KMALLOC_MEM_SIZE;

static unsigned long _MEM_SIZE;
static unsigned long _MEM_START;
#define _MEM_END   (_MEM_START + _MEM_SIZE - 1)

#define PAGE_SHIFT                  (12)
#define PAGE_SIZE                   (1 << PAGE_SHIFT)
#define MAX_BUDDY_PAGE_NUM          (9)
#define PAGE_NUM_FOR_MAX_BUDDY      ((1 << MAX_BUDDY_PAGE_NUM))

#define KERNEL_PAGE_NUM             (_MEM_SIZE / ((sizeof (struct page) + PAGE_SIZE)))
/*
KERNEL_PAGE_NUM 鐞涖劎銇氭稉锟介崗杈ㄦ箒婢舵艾鐨稉顏堛��(缁狅拷閸楁洜娈戠粻妤佺《: 閸愬懎鐡ㄩ惃鍕亣鐏忥拷 / (妞ら潧銇囩亸锟� + 娑擄拷娑撶尰truct page閻ㄥ嫬銇囩亸锟�),閺堝顦跨亸鎴滈嚋妞ら潧姘ㄩ張澶婎樋鐏忔垳閲渟truct page))
娴ｅ棜绻栭弽铚傜窗鐎佃壈鍤х粭顑跨娑擃亪銆夐獮鏈电瑝閺勶拷4K鐎靛綊缍�!!!!!!!!!!!!!!!!!!!!!!!!!!
-------------------------------------------------------------
閿濇笩truct page|  ....., |struct page|4K|4k|4k| ...... |4K|4k|
-------------------------------------------------------------
閿濇窚ERNEL_PAGE_NUM 娑擄拷 struct page   |KERNEL_PAGE_NUM 娑擃亪銆�,濮ｅ繋閲滄い闈涖亣鐏忥拷4k
*/

#define HEAP_START_ADDR	            (_MEM_START + (sizeof (struct page) * KERNEL_PAGE_NUM))
/*
 *HEAP_START_ADDR 鐞涖劎銇氶惇鐔割劀閻ㄥ嫬鐖㈤崠鍝勭磻婵缍呯純锟�,娑旂喎姘ㄩ弰顖滎儑娑擄拷娑擄拷4k妞ら潧绱戞慨瀣畱娴ｅ秶鐤�
 */
#define	HEAP_END_ADDR	            (HEAP_START_ADDR + KERNEL_PAGE_NUM * PAGE_SIZE - 1)

#define BUDDY_STRUCT_ADDR           (_MEM_START)
/*
 *BUDDY_STRUCT_ADDR 鐞涖劎銇歴truct page瀵拷婵娈戞担宥囩枂濮ｅ繋閲渟truct page鐎电懓绨叉稉锟芥稉锟�4k閻ㄥ嫬褰查悽銊ょ艾閸掑棝鍘ら惃鍕敶鐎涳拷
 * */

/*page flags*/
#define PAGE_AVAILABLE				0x00
#define PAGE_DIRTY					0x01
#define PAGE_PROTECT				0x02
#define PAGE_BUDDY_BUSY				0x04
#define PAGE_IN_CACHE				0x08

/////////////////////////kmem_cache///////////////////////////////////////////////////////////////////////////////////
#define KMEM_CACHE_MAX_ORDER            (5)
#define KMEM_CACHE_SAVE_RATE            (90)
#define KMEM_CACHE_PERCENT              (100)
#define KMEM_CACHE_MAX_WAST             (PAGE_SIZE - (PAGE_SIZE * KMEM_CACHE_SAVE_RATE) / KMEM_CACHE_PERCENT)

struct kmem_cache
{
    unsigned int obj_size;
    unsigned int obj_nr;
    unsigned int page_order;
    unsigned int flags;
    struct page *head_page;
    struct page *end_page;
    
    void *nf_block;
};
struct page
{
	unsigned long vaddr;
	unsigned int flags;
	int order;
    struct kmem_cache *cache;
    struct page *prev;
    struct page *next;
};

struct page page_buddy[MAX_BUDDY_PAGE_NUM + 1];

void init_page_buddy()
{
    int i;

    for (i = 0; i <= MAX_BUDDY_PAGE_NUM; i++)
    {
        page_buddy[i].prev = &page_buddy[i];
        page_buddy[i].next = &page_buddy[i];
    }
}

void init_list_head(struct page *page)
{
    if (!page)
        return;
    page->prev = page;
    page->next = page;
}

void list_add_chain_tail(struct page *ch, struct page *ct, struct page *head)
{
    ch->prev = head->prev;
    head->prev->next = ch;
    head->prev = ct;
    ct->next = head;
}

void list_add_chain(struct page *ch, struct page *ct,struct page *head)
{
    ch->prev=head;
    ct->next=head->next;
    head->next->prev=ct;
    head->next=ch;
}

void __list_add(struct page *new_lst, struct page *prev, struct page *next)
{
    next->prev = new_lst;
    new_lst->next = next;
    new_lst->prev = prev;
    prev->next = new_lst;
}

void list_add_tail(struct page *new_lst, struct page *head)
{
    __list_add(new_lst, head->prev, head);
}

void list_remove_chain(struct page *ch,struct page *ct)
{
	ch->prev->next=ct->next;
	ct->next->prev=ch->prev;
}

int list_empty(struct page *head)
{
    return head->next == head;
}

struct page *virt_to_page(unsigned long addr)
{
    unsigned long i;
    i = ((addr) - HEAP_START_ADDR) >> PAGE_SHIFT;

    if (i > KERNEL_PAGE_NUM)
        return NULL;

    return (struct page *)BUDDY_STRUCT_ADDR + i;
}

void init_page_map()
{
    int i;
    struct page *pg;

    init_page_buddy();
    
    pg = (struct page *)BUDDY_STRUCT_ADDR;
    for (i = 0; i < (KERNEL_PAGE_NUM); i++)
    {
        pg->vaddr = HEAP_START_ADDR + (i * PAGE_SIZE); 
        pg->flags = PAGE_AVAILABLE;
        init_list_head(pg);
    
        if (i < (KERNEL_PAGE_NUM & (~(PAGE_NUM_FOR_MAX_BUDDY - 1))))
        {
            if ((i & (PAGE_NUM_FOR_MAX_BUDDY - 1)) == 0)
                pg->order = MAX_BUDDY_PAGE_NUM;
            else
                pg->order = -1;
            list_add_tail(pg, &page_buddy[MAX_BUDDY_PAGE_NUM]);
        }
        else
        {
            pg->order = 0;
            list_add_tail(pg, &page_buddy[0]);
        }
        pg++;
    }
}


#define BUDDY_END(x, order)         ((x) + (1 << (order)) - 1)
#define NEXT_BUDDY_START(x, order)  ((x) + (1 << (order)))
#define PREV_BUDDY_START(x, order)  ((x) - (1 << (order)))

struct page *get_page_from_list(int order)
{
    struct page *pg;
    int neworder = order;
    struct page *tlst0;
    struct page *tlst1;

    for (; neworder <= MAX_BUDDY_PAGE_NUM; neworder++)
    {
        if (list_empty(&page_buddy[neworder]))
            continue;
        else
        {
            pg = page_buddy[neworder].next;
            tlst0 = BUDDY_END(pg, neworder);
            tlst0->next->prev = &page_buddy[neworder];
            page_buddy[neworder].next = tlst0->next;

            for (neworder--; neworder >= order; neworder--)
            {
                tlst1 = BUDDY_END(pg, neworder);
                tlst0 = pg;

                pg = NEXT_BUDDY_START(pg, neworder);
                tlst0->order = neworder;
                
                list_add_chain_tail(tlst0, tlst1, &page_buddy[neworder]);
            }
            pg->flags |= PAGE_BUDDY_BUSY;
            pg->order = order;
            return pg;
        }
    }

    return NULL;
}

void *page_address(struct page *pg)
{
    return (void *)(pg->vaddr);
}

struct page *alloc_pages(unsigned int flag, int order)
{
    struct page *pg;
    int i;

    pg = get_page_from_list(order);
    if (pg == NULL)
        return NULL;

    for (i = 0; i < (1 << order); i++)
        (pg + i)->flags |= PAGE_DIRTY;
    
    return pg;
}

void put_pages_to_list(struct page *pg, int order)
{
    struct page *tprev;
    struct page *tnext;

    if (!(pg->flags & PAGE_BUDDY_BUSY))
    {
		printk("something must be wrong when you see this message,that probably means you are forcing to release a page that was not alloc at all\n");
		return;
    }

    pg->flags &= !(PAGE_BUDDY_BUSY);

    for (; order <= MAX_BUDDY_PAGE_NUM; order++)
    {
		tnext = NEXT_BUDDY_START(pg, order);
		tprev = PREV_BUDDY_START(pg, order);

        if ((!(tnext->flags & PAGE_BUDDY_BUSY)) && (tnext->order == order))
        {
            pg->order++;
            tnext->order = -1;
            list_remove_chain(tnext, BUDDY_END(tnext, order));

            BUDDY_END(pg, order)->next = tnext;
            tnext->prev = BUDDY_END(pg, order);
            continue;
        }
        else if ((!(tprev->flags & PAGE_BUDDY_BUSY)) && (tprev->order == order))
        {
            pg->order = -1; 

            list_remove_chain(tprev, BUDDY_END(tprev, order));
            BUDDY_END(tprev, order)->next = pg;
            pg->prev = BUDDY_END(tprev, order);

            pg = tprev;
            pg->order++;
            continue; 
        }
    }

    list_add_chain(pg, tnext - 1, &page_buddy[pg->order]);
}

int find_right_order(unsigned int size)
{
    int order;

    for (order = 0; order <= KMEM_CACHE_MAX_ORDER; order++)
    {
        if (size <= (KMEM_CACHE_MAX_WAST * (1 << order)))
            return order;
    }

    if (size > (1 << order))
        return order;

    return -1;
}

int kmem_cache_line_object(void *head, unsigned int size, int order)
{
    void **pl;
    char *p;
    int i, s;

    pl = (void **)head;
    p = (char *)head + size;

    s = PAGE_SIZE * (1 << order);

    for (i = 0; s > size; i++)
    {
        *pl = (void *)p;
        pl = (void **)p;
        p = p + size;
        s -= size;
    }
    if (s == size)
        i++;

    return i;
}

struct kmem_cache *kmem_cache_create(struct kmem_cache *cache, unsigned int size, unsigned int flags)
{
    int order;
    void  **nf_block = &(cache->nf_block);

    order = find_right_order(size);

    if (order == -1)
    {
        return NULL;
    }

    if ((cache->head_page = alloc_pages(0, order)) == NULL)
        return NULL;

    *nf_block = page_address(cache->head_page);
    cache->obj_nr = kmem_cache_line_object(*nf_block, size, order);
    cache->obj_size = size;
    cache->page_order = order;
    cache->flags = flags;
    cache->end_page = BUDDY_END(cache->head_page, order);

	return cache;
}

#define KMALLOC_BIAS_SHIFT          (4)
#define KMALLOC_MAX_SIZE            (1024 * 1024)
#define KMALLOC_MINIMAL_SIZE_BIAS   (1 << (KMALLOC_BIAS_SHIFT))
//KMALLOC_MINIMAL_SIZE_BIAS: malloc閻ㄥ嫭娓剁亸蹇撳瀻闁板秴宕熸担锟�,閸嬪洤顩х�瑰啰娈戞径褍鐨稉锟�(1 << 4 = 16)闁絼绠為悽瀹狀嚞鐏忓繋绨�16鐎涙濡惃鍕敶鐎涙﹫绱漦malloc娑旂喍绱版潻鏂挎礀婢堆冪毈16鐎涙濡惃鍕敶鐎涳拷
#define KMALLOC_CACHE_SIZE          (KMALLOC_MAX_SIZE / KMALLOC_MINIMAL_SIZE_BIAS)
struct kmem_cache kmalloc_cache[KMALLOC_CACHE_SIZE];
#define kmalloc_cache_size_to_index(size) ((size + 4) >> (KMALLOC_BIAS_SHIFT))

int kmem_cache_init()
{
    int i = 0;

    for (i = 0; i < KMALLOC_CACHE_SIZE; i++)
    {
        kmalloc_cache[i].obj_nr     = 0;
        kmalloc_cache[i].obj_size   = 0;       
        kmalloc_cache[i].head_page  = NULL;       
        kmalloc_cache[i].end_page   = NULL;       
    }

#if 0
    for (i = 0; i < KMALLOC_CACHE_SIZE; i++)
    {
        if (kmem_cache_create(&kmalloc_cache[i], (i + 1) * KMALLOC_MINIMAL_SIZE_BIAS, 0) == NULL)
            break;
    }

    for (; i < KMALLOC_CACHE_SIZE; i++)
        kmalloc_cache[i].page_order = find_right_order((i + 1) * KMALLOC_MINIMAL_SIZE_BIAS);
#endif

    return 0;
}

void *kmem_cache_alloc(struct kmem_cache *cache, int index, unsigned int flag)
{
    void *p;
    struct page *pg;
    void **nf_block;
    unsigned int *nr;
    int order;

    if (cache == NULL)
        return NULL;

    if (cache->obj_size == 0)
    {
        if (kmem_cache_create(cache, (index + 1) * KMALLOC_MINIMAL_SIZE_BIAS, 0) == NULL)
            return NULL;
    }

    nf_block = &(cache->nf_block);
    nr = &(cache->obj_nr);
    order = cache->page_order;

    if (!*nr)
    {
        if ((pg = alloc_pages(0, order)) == NULL)
        {
            printk("alloc_pages error order: %d\n", order);
            return NULL;
        }

        *nf_block = page_address(pg);
        cache->end_page->next = pg;
        cache->end_page = BUDDY_END(pg, order);
        cache->end_page->next = NULL;

        *nr += kmem_cache_line_object(*nf_block, cache->obj_size, order);
    }

    (*nr)--;
    p = *nf_block;
    pg = virt_to_page((unsigned int)p);
    pg->cache = cache;
    *nf_block = *(void **)p;
    return (void *)((unsigned int)p + sizeof (long));
}

void kmem_cache_free(struct kmem_cache *cache, void *objp)
{
    *(void **)objp = cache->nf_block;
    cache->nf_block = objp;
    cache->obj_nr++;
}

void kfree(void *addr)
{
    struct page *pg;

    addr = (void *)((unsigned int)addr - 4);
    pg = virt_to_page((unsigned int)addr);
    kmem_cache_free(pg->cache, addr);
}

void *kmalloc(unsigned int size)
{
    void *p;
    int index = kmalloc_cache_size_to_index(size);

    if (index >= KMALLOC_CACHE_SIZE)
        return NULL;

    p = kmem_cache_alloc(&kmalloc_cache[index], index, 0);
    if (p)
        memset(p, 0, size);
    return p;
}

int system_mm_init(void)
{

    _MEM_START = KMALLOC_ADDR_START;
	_MEM_SIZE = KMALLOC_MEM_SIZE;
    init_page_map();
    kmem_cache_init();

    printk("_MEM_SIZE:                      %#lx\n", _MEM_SIZE);
    printk("_MEM_START:                     %#lx\n", _MEM_START);
    printk("_MEM_END:                       %#lx\n", _MEM_END);
    printk("PAGE_SHIFT:                     %d\n", PAGE_SHIFT);
    printk("MAX_BUDDY_PAGE_NUM:             %d\n", MAX_BUDDY_PAGE_NUM);
    printk("PAGE_NUM_FOR_MAX_BUDDY:         %d\n", PAGE_NUM_FOR_MAX_BUDDY);
    printk("KERNEL_PAGE_NUM:                %ld\n", KERNEL_PAGE_NUM);
    printk("HEAP_START_ADDR:                %#lx\n", HEAP_START_ADDR);
    printk("HEAP_END_ADDR:                  %#lx\n", HEAP_END_ADDR);
    printk("BUDDY_STRUCT_ADDR:              %#lx\n", BUDDY_STRUCT_ADDR);

    printk("KMEM_CACHE_MAX_ORDER:           %x\n", KMEM_CACHE_MAX_ORDER);
    printk("KMEM_CACHE_SAVE_RATE:           %d\n", KMEM_CACHE_SAVE_RATE);
    printk("KMEM_CACHE_PERCENT:             %d\n", KMEM_CACHE_PERCENT);
    printk("KMEM_CACHE_MAX_WAST:            %d\n", KMEM_CACHE_MAX_WAST);
    
    printk("KMALLOC_BIAS_SHIFT:             %d\n", KMALLOC_BIAS_SHIFT);
    printk("KMALLOC_MAX_SIZE:               %d\n", KMALLOC_MAX_SIZE);
    printk("KMALLOC_MINIMAL_SIZE_BIAS:      %d\n", KMALLOC_MINIMAL_SIZE_BIAS);
    printk("KMALLOC_CACHE_SIZE:             %d\n", KMALLOC_CACHE_SIZE);

    printk("sizeof (struct page):           %d\n", sizeof (struct page));
    printk("buddy struct area size:         %ld:%#lx\n", sizeof (struct page) * KERNEL_PAGE_NUM, sizeof (struct page) * KERNEL_PAGE_NUM);

	return 0;
}
void system_mm_destroy()
{

}

