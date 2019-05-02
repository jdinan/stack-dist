#ifndef _LIST_H_
#define _LIST_H_

#define MARKER_MAX_SZ 250

/* List elements */
typedef struct list_elem_s {
  union {
    void *data;  // When it's a data element
    int   count; // When it's a marker element
  };
  int   is_marker;
  struct list_elem_s *prev_marker;
  struct list_elem_s *next;
} list_elem_t;


/* The list structure */
typedef struct {
  list_elem_t *head;
  list_elem_t *marker_tail;
} list_t;


list_t *list_create();
int     list_is_empty(list_t *list);
list_elem_t *list_push(list_t *list, void *data);
int     list_move_to_head(list_t *list, list_elem_t *data);
void    list_print(list_t *list);

#endif /* _LIST_H_ */
