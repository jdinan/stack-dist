/* LIST.C -- James Dinan <dinan@cse.ohio-state.edu> -- July, 2010
 *
 * Linked list with markers interspersed for fast backward traversal.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "list.h"

/*
 * Create a linked list
 */
list_t *list_create() {
  list_t *list;

  list = (list_t*) malloc(sizeof(list_t));
  assert(list != NULL);

  list->head = NULL;
  list->marker_tail = NULL;

  return list;
}


/*
 * Is the list empty?
 */
int list_is_empty(list_t *list) {
  return (list->head == NULL) ? 1 : 0;
}


/*
 * Push onto the head of the list.
 */
list_elem_t *list_push(list_t *list, void *data) {
  list_elem_t *new_elem;
  
  new_elem = (list_elem_t*) malloc(sizeof(list_elem_t));
  assert(new_elem != NULL);

  new_elem->data = data;
  new_elem->next = list->head;
  new_elem->is_marker   = 0;
  new_elem->prev_marker = NULL;

  // Push marker first if empty or add a new one if the current marker is full
  // Invariant: Always a marker to our right
  if (list_is_empty(list) || list->marker_tail->count >= MARKER_MAX_SZ) {
    list_elem_t *marker;
    marker = (list_elem_t*) malloc(sizeof(list_elem_t));
    assert(marker != NULL);

    marker->count       = 0;
    marker->is_marker   = 1;
    marker->next        = list->head;
    marker->prev_marker = NULL;

    if (list->marker_tail)
      list->marker_tail->prev_marker = marker;
    list->marker_tail = marker;

    new_elem->next    = marker;
  }

  list->marker_tail->count++;

  list->head = new_elem;

  return new_elem;
}


/*
 * Move an element to the head of the list.
 *
 * @return Distance to the element from the head or -1 if not found.
 */
int list_move_to_head(list_t *list, list_elem_t *elem) {
  int distance, dist_to_marker;
  list_elem_t *cur, *marker, *parent;

  // Special case: Element is already on the head
  if (list->head == elem) {
    return 0;
  }

  // Find next marker
  cur = elem;
  dist_to_marker = 0;
  while (!cur->is_marker) {
    cur = cur->next;
    dist_to_marker++;
  }
  marker = cur;

  distance = marker->count-dist_to_marker;

  // Walk markers to find distance back to the head
  cur = marker->prev_marker;
  while (cur != NULL) {
    distance += cur->count;
    cur = cur->prev_marker;
  }

  // Find my parent, scan from last marker or list head
  if (marker->prev_marker == NULL)
    cur = list->head;
  else
    cur = marker->prev_marker;
  while(cur->next != elem) cur = cur->next;
  parent = cur;

  // Remove the element from the list
  parent->next = elem->next;
  marker->count--;
 
  // Did we make an empty marker?  If so, remove it.
  if (marker->count == 0) {
    assert(marker == elem->next);
    assert(parent->is_marker);

    parent->next = marker->next;

    // Update back-links
    cur = marker->next;
    while (cur != NULL && !cur->is_marker) cur = cur->next;

    // End of the list
    if (cur == NULL) {
      ; // Do nothing

    // Head of the list
    } else if (list->marker_tail == marker) {
      list->marker_tail = cur;
      cur->prev_marker = NULL;

    // Middle of the list
    } else
      cur->prev_marker = parent;

    free(marker);
  }

  // If the current marker is full, create and push a new one
  if (list->marker_tail->count >= MARKER_MAX_SZ) {
    list_elem_t *new_marker;
    new_marker = (list_elem_t*) malloc(sizeof(list_elem_t));
    assert(new_marker != NULL);

    new_marker->count       = 0;
    new_marker->is_marker   = 1;
    new_marker->next        = list->head;
    new_marker->prev_marker = NULL;

    elem->next        = new_marker;
    list->marker_tail->prev_marker = new_marker;
    list->marker_tail = new_marker;
  } else {
    elem->next = list->head;
  }

  // Push element onto the head
  list->head = elem;
  list->marker_tail->count++;

  return distance;
}


void list_print(list_t *list) {
  list_elem_t *cur = list->head;

  printf("HEAD=%p MARKER_TAIL=%p\n", list->head, list->marker_tail);

  while (cur != NULL) {
    if (cur->is_marker)
      printf("mark=%p count=%9d prev=%p\n", cur, cur->count, cur->prev_marker);
    else
      printf("elem=%p  data=%p next=%p\n", cur, cur->data, cur->next);
    cur = cur->next;
  }
}
