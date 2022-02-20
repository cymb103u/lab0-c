#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    /**
     * we want to free the queue, so list nodes would be deleted.
     * we can use this marco "list_for_each_safe" in list.h to
     * make sure that this node would be removed safely.
     */
    struct list_head *head = l;
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        list_del(node);
        element_t *ele = container_of(node, element_t, list);
        q_release_element(ele);
    }
    free(head);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
#if DEBUG
        printf("The add node is NULL.\n");
#endif
        return false;
    }
    element_t *ele = malloc(sizeof(element_t));
    if (!ele) {
#if DEBUG
        printf("locate element mem fail.\n");
#endif
        return false;
    }
    struct list_head *ele_list = &ele->list;

    ele->value = strdup(s);
    if (!ele->value) {
#if DEBUG
        printf("allocate the element value mem fail.");
#endif
        free(ele);
        return false;
    }
    list_add(ele_list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
#if DEBUG
        printf("The add node is NULL.\n");
#endif
        return false;
    }
    element_t *ele = malloc(sizeof(element_t));
    if (!ele) {
#if DEBUG
        printf("allocate element mem fail.\n");
#endif
        return false;
    }
    struct list_head *ele_list = &ele->list;

    ele->value = strdup(s);
    if (!ele->value) {
#if DEBUG
        printf("allocate the element value mem fail.");
#endif
        free(ele);
        return false;
    }
    list_add_tail(ele_list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head))
        return NULL;
    struct list_head *next = head->next;
    element_t *ele = container_of(next, element_t, list);
    list_del(next);
    // if sp is non-NULL (handling)
    if (sp) {
        int len;
        for (len = 0; *(ele->value + len); len++)
            ;
        if (len > bufsize) {
            strncpy(sp, ele->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        } else {
            strncpy(sp, ele->value, len);
            sp[len] = '\0';
        }
    }
    return ele;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head))
        return NULL;
    struct list_head *prev = head->prev;
    element_t *ele = container_of(prev, element_t, list);
    list_del(prev);
    if (sp) {
        int len;
        for (len = 0; *(ele->value + len); len++)
            ;
        if (len > bufsize) {
            strncpy(sp, ele->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        } else {
            strncpy(sp, ele->value, len);
            sp[len] = '\0';
        }
    }
    return ele;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    struct list_head *node;

    list_for_each (node, head)
        len++;
    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return NULL if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head)
        return false;
    struct list_head *slow, *fast;
    for (slow = head->next, fast = head->next->next;
         fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;
    element_t *ele_mid = container_of(slow, element_t, list);
#if DEBUG
    printf("The mid element value is %s\n", ele_mid->value);
#endif
    list_del(slow);
    q_release_element(ele_mid);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head) {}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
