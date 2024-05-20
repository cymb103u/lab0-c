#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    /* malloc element. & dup string */
    element_t *ele = malloc(sizeof(element_t));
    char *str = strdup(s);
    if (!ele || !str) {
        free(ele);
        free(str);
        return false;
    }

    /* modify element */
    ele->value = str;
    list_add(&ele->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    /* malloc element. & dup string */
    element_t *ele = malloc(sizeof(element_t));
    char *str = strdup(s);
    if (!ele || !str) {
        free(ele);
        free(str);
        return false;
    }

    /* modify element */
    ele->value = str;
    list_add_tail(&ele->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *next = head->next;
    element_t *ele = list_entry(next, element_t, list);
    list_del_init(next);
    if (sp && ele->value) {
        memcpy(sp, ele->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return ele;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *prev = head->prev;
    element_t *ele = list_entry(prev, element_t, list);
    list_del_init(prev);
    if (sp && ele->value) {
        memcpy(sp, ele->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return ele;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    unsigned int sz = 0;
    struct list_head *node;
    list_for_each (node, head)
        sz++;
    return sz;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head *slow, *fast;
    for (slow = fast = head->next; fast != head && fast->next != head;
         slow = slow->next, fast = fast->next->next)
        ;

    list_del_init(slow);

    element_t *ele = list_entry(slow, element_t, list);
    q_release_element(ele);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    if (list_empty(head) || list_is_singular(head))
        return true;
    bool dup = false;
    struct list_head *cur, *next;
    cur = head->next;
    next = cur->next;

    /** check cur != head */
    while (cur != head) {
        element_t *e_cur = list_entry(cur, element_t, list);
        element_t *e_next = list_entry(next, element_t, list);

        /** check next != head avoid accessing invaild addr,
         *   cmp cur,next val qual?
         *  equl: set flag
         */
        if (next != head && !strcmp(e_cur->value, e_next->value)) {
            dup = true;
            list_del_init(cur);
            q_release_element(e_cur);
            cur = next;
            next = next->next;
            continue;
        }

        if (dup) {
            list_del_init(cur);
            q_release_element(e_cur);
            dup = false;
        }
        cur = next;
        next = next->next;
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *one = head->next, *two = head->next->next;
    LIST_HEAD(tmp);
    while (!list_empty(head) && !list_is_singular(head)) {
        list_move_tail(two, &tmp);
        list_move_tail(one, &tmp);
        one = head->next;
        two = head->next->next;
    }
    list_splice_init(&tmp, head);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *cur = head;
    do {
        struct list_head *next = cur->next;
        cur->next = cur->prev;
        cur->prev = next;
        cur = next;
    } while (cur != head);
}

/* Reverse the nodes of the list k qat a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;

    int i = 0;
    struct list_head *cur = head->next;
    LIST_HEAD(reverse);
    LIST_HEAD(pre_reverse);
    while (cur != head) {
        if (i != (k - 1)) {
            cur = cur->next;
            i++;
        } else {
            list_cut_position(&pre_reverse, head, cur);
            q_reverse(&pre_reverse);
            list_splice_tail_init(&pre_reverse, &reverse);
            i = 0;
            cur = head->next;
        }
    }
    list_splice_init(&reverse, head);
}

void merge_list(struct list_head *merge,
                struct list_head *left,
                struct list_head *right,
                bool descend)
{
    struct list_head *l_travel = left->next, *r_travel = right->next;
    struct list_head *l_safe = left->next->next, *r_safe = right->next->next;
    struct list_head **node;

    for (; l_travel != left && r_travel != right;) {
        element_t *n_left = list_entry(l_travel, element_t, list);
        element_t *n_right = list_entry(r_travel, element_t, list);
        /** check equal condition => stable */
        int ret = (strcmp(n_left->value, n_right->value) <= 0) ^ descend;
        // printf("left: %s, right: %s\n", n_left->value,n_right->value);

        node = ret ? &l_travel : &r_travel;
        list_move_tail(*node, merge);

        *node = ret ? l_safe : r_safe;
        node = ret ? &l_safe : &r_safe;

        *node = (*node)->next;
        // if (ret) {
        //     list_move_tail(l_travel, merge);
        //     l_travel = l_safe;
        //     l_safe = l_safe->next;
        // } else {
        //     list_move_tail(r_travel, merge);
        //     r_travel = r_safe;
        //     r_safe = r_safe->next;
        // }
    }

    node = (l_travel == left) ? &right : &left;
    list_splice_tail_init(*node, merge);

    // if (l_travel == left) {
    //     printf("left node NULL\n");
    //     list_splice_tail_init(right, merge);
    // }
    // if (r_travel == right) {
    //     printf("right node NULL\n");
    //     list_splice_tail_init(left, merge);
    // }
    return;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    /** split list from mid node */
    struct list_head *slow, *fast;
    slow = head->next;
    fast = head->next->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    LIST_HEAD(left);
    list_cut_position(&left, head, slow);
    LIST_HEAD(right);
    list_splice_init(head, &right);

    q_sort(&left, descend);
    q_sort(&right, descend);
    merge_list(head, &left, &right, descend);
}

void quick_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;
    /** select first node as povit & remove from head */
    struct list_head *pivot = head->next;
    list_del_init(pivot);

    element_t *entry, *safe, *e_piovt = list_entry(pivot, element_t, list);
    LIST_HEAD(left);
    LIST_HEAD(right);
    list_for_each_entry_safe (entry, safe, head, list) {
        /** cmp result XOR with bool descend */
        int ret = (strcmp(entry->value, e_piovt->value) < 0) ^ descend;
        struct list_head *add_list = ret ? &left : &right;
        list_move(&entry->list, add_list);
    }

    if (!list_empty(&left) && !list_is_singular(&left))
        q_sort(&left, descend);
    if (!list_empty(&right) && !list_is_singular(&right))
        q_sort(&right, descend);

    list_splice_tail(&left, head);
    list_add_tail(pivot, head);
    list_splice_tail(&right, head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *entry = head->prev->prev, *safe = entry->prev,
                     *cur = head->prev;
    int node_cnt = 0;

    while (entry != head) {
        element_t *e_entry, *e_cur;
        e_entry = list_entry(entry, element_t, list);
        e_cur = list_entry(cur, element_t, list);

        /** a negative value if s1 is less than s2;*/
        if (strcmp(e_cur->value, e_entry->value) <= 0) {
            list_del_init(entry);
            free(e_entry->value);
            free(e_entry);
            entry = safe;
            safe = safe->prev;
            continue;
        }
        node_cnt++;
        cur = entry;
        entry = cur->prev;
        safe = entry->prev;
    }
    /** last add */
    node_cnt++;
    return node_cnt;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    // Remove every node which has a node with a greater value anywhere to the
    // right side of it. org: 5 -> 2 -> 13 -> 3 -> 8 mod: 13 -> 8
    if (!head || list_empty(head))
        return 0;
    struct list_head *entry = head->prev->prev, *safe = entry->prev,
                     *cur = head->prev;
    int node_cnt = 0;

    while (entry != head) {
        element_t *e_entry, *e_cur;
        e_entry = list_entry(entry, element_t, list);
        e_cur = list_entry(cur, element_t, list);

        /** a negative value if s1 is less than s2;*/
        if (strcmp(e_cur->value, e_entry->value) >= 0) {
            list_del_init(entry);
            free(e_entry->value);
            free(e_entry);
            entry = safe;
            safe = safe->prev;
            continue;
        }
        node_cnt++;
        cur = entry;
        entry = cur->prev;
        safe = entry->prev;
    }
    /** last add */
    node_cnt++;
    return node_cnt;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    struct list_head *cur = head->next;
    int node_cnt = 0, q_cnt = 0;

    while (cur != head) {
        queue_contex_t *q_cur = list_entry(cur, queue_contex_t, chain);

        node_cnt += q_cur->size;
        q_cnt++;
        cur = cur->next;
    }
    /**  refer to
     * https://hackmd.io/@lambert-wu/list-merge-sort#%E5%90%88%E4%BD%B5
     * */

    struct list_head *l1 = head->next, *l2 = head->next->next;
    cur = head->next;
    int iter_cnt = q_cnt;
    int end_cnt = q_cnt;
    while (end_cnt != 1) {
        queue_contex_t *q_l1 = list_entry(l1, queue_contex_t, chain);
        queue_contex_t *q_l2 = list_entry(l2, queue_contex_t, chain);
        LIST_HEAD(tmp);
        merge_list(&tmp, q_l1->q, q_l2->q, descend);

        queue_contex_t *q_cur = list_entry(cur, queue_contex_t, chain);

        list_splice_init(&tmp, q_cur->q);

        l1 = l2->next;
        l2 = l2->next->next;
        cur = cur->next;
        iter_cnt -= 2;
        if (iter_cnt <= 1) {
            /** odd q ,even q */
            if (iter_cnt == 1) {
                q_l1 = list_entry(l1, queue_contex_t, chain);
                q_cur = list_entry(cur, queue_contex_t, chain);
                list_splice_init(q_l1->q, q_cur->q);
                end_cnt += 1;
            }

            end_cnt /= 2;
            iter_cnt = end_cnt;
            l1 = head->next;
            l2 = head->next->next;
            cur = head->next;
        }
    }
    return node_cnt;
}
