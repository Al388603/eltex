#ifndef ADDRESS_BOOK_H
#define ADDRESS_BOOK_H
#include "contact.h"
#include <stddef.h>
typedef struct {
    Contact *items;
    size_t count;
    size_t capacity;
    int next_id;
} AddressBook;
void address_book_init(AddressBook *book);
void address_book_destroy(AddressBook *book);
int address_book_add(AddressBook *book, Contact *contact);
Contact *address_book_find(AddressBook *book, int id);
int address_book_remove(AddressBook *book, int id);
#endif