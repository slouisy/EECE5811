#ifndef LIST_HH
#define LIST_HH

typedef struct node_hh {
    int key;
    struct node_hh* next;
    pthread_mutex_t lock;  /*node lock*/
} node_hh_t;

typedef struct {
    node_hh_t* head;
    pthread_mutex_t lock;  /*global lock*/
} list_hh_t;

void List_HH_Init(list_hh_t* L);
void List_HH_Insert(list_hh_t *L, int key);
int List_HH_Lookup(list_hh_t *L, int key);
void List_HH_Destroy(list_hh_t *L);

#endif