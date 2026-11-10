#ifndef ADDRESS_BOOK_H
#define ADDRESS_BOOK_H
#include "contact.h"
#include <stddef.h>
typedef struct ContactNode {
    Contact contact;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;
typedef struct {
    ContactNode *head;
    ContactNode *tail;
    size_t count;
    int next_id;
} AddressBook;
void address_book_init(AddressBook *book);
void address_book_destroy(AddressBook *book);
int address_book_add(AddressBook *book, Contact *contact);
Contact *address_book_find(AddressBook *book, int id);
int address_book_remove(AddressBook *book, int id);
int address_book_reorder(AddressBook *book, int id);
#endif
