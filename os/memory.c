/* ------
 * Includes
 * ------ */
#include "defines.h"
#include "kozos.h"
#include "lib.h"
#include "memory.h"


/* ------
 * Data types
 * ------ */
/* Memory block struct
   - Header of the memory area */
typedef struct _kzmem_block {
    struct _kzmem_block *next;
    int     size;
} kzmem_block;

/* Memory pool */
typedef struct _kzmem_pool {
    int size;
    int num;
    kzmem_block *free;
} kzmem_pool;


/* ------
 * Variables
 * ------ */
/* Memory pool definitions */
static kzmem_pool pool[] = {
    /* {bytes includes header, number of memory pool, start address} */
    {16, 8, NULL},
    {32, 8, NULL},
    {64, 4, NULL},
};

#define MEMORY_AREA_NUM (sizeof(pool) / sizeof(*pool))


/* ------
 * Function prototypes
 * ------ */
static int kzmem_init_pool(kzmem_pool *p);


/* ------
 * Functions
 * ------ */

/* ----------------------------------------------------------
 * Description:
 *  Initialize memory pool
 * ---------------------------------------------------------- */
static int kzmem_init_pool(kzmem_pool *p)
{
    int i;
    kzmem_block *mp;
    kzmem_block **mpp;
    extern char freearea;   /* reference to the variable in the linker script */
    static char *area = &freearea;

    mp = (kzmem_block *)area;

    /* Connect every individual memory area to the resolved link list */
    mpp = &p->free;
    for (i = 0; i < p->num; i++) {
        *mpp = mp;
        memset(mp, 0, sizeof(*mp));
        mp->size = p->size;
        mpp = &(mp->next);
        mp = (kzmem_block *)((char *)mp + p->size);
        area += p->size;
    }

    return 0;
}


/* ----------------------------------------------------------
 * Description:
 *  Initilize the dynamic memory
 * ---------------------------------------------------------- */
int kzmem_init(void)
{
    int i;
    for (i = 0; i < MEMORY_AREA_NUM; i++) {
        kzmem_init_pool(&pool[i]);
    }
    return 0;
}


/* ----------------------------------------------------------
 * Description:
 *  To acquire memory area
 * ---------------------------------------------------------- */
void *kzmem_alloc(int size)
{
    int i;
    kzmem_block *mp;
    kzmem_pool *p;

    for (i = 0; i < MEMORY_AREA_NUM; i++) {
        p = &pool[i];

        if (size <= p->size - sizeof(kzmem_block)) {
            /* if there is not available area, then shutdown */
            if (p->free == NULL) {
                kz_sysdown();
                return NULL;
            }
            mp = p->free;
            p->free = p->free->next;
            mp->next = NULL;

            /* actually available memory area is next to the memory block area */
            return mp + 1;
        }
    }

    /* There is not an available area that stores the requested memory size */
    kz_sysdown();
    return NULL;
}


/* ----------------------------------------------------------
 * Description:
 *  Free memory area
 * ---------------------------------------------------------- */
void kzmem_free(void *mem)
{
    int i;
    kzmem_block *mp;
    kzmem_pool *p;

    /* Acquire the memory block */
    mp = ((kzmem_block *)mem - 1);

    for (i = 0; i < MEMORY_AREA_NUM; i++) {
        p = &pool[i];
        if (mp->size == p->size) {
            mp->next = p->free;
            p->free = mp;
            return;
        }
    }

    kz_sysdown();
}

/* end of the file */
