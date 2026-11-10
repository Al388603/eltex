#include "address_book.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
//сравнивает контакты по фамилии, имени и отчеству 
static int compare_contacts(const Contact *a, const Contact *b)
{
    int result = strcmp(a->surname, b->surname);
    if (result != 0) {
        return result;
    }

    result = strcmp(a->name, b->name);
    if (result != 0) {
        return result;
    }

    result = strcmp(a->patronymic, b->patronymic);
    if (result != 0) {
        return result;
    }

    return (a->id > b->id) - (a->id < b->id);
}
//вставляет узел на место, которое подходит по алфавитному порядку 
static void insert_sorted(AddressBook *book, ContactNode *node)
{
    ContactNode *current = book->head;

    while (current != NULL &&
           compare_contacts(&current->contact, &node->contact) < 0) {
        current = current->next;
    }

    if (current == NULL) {
        node->prev = book->tail;
        node->next = NULL;

        if (book->tail != NULL) {
            book->tail->next = node;
        } else {
            book->head = node;
        }

        book->tail = node;
        return;
    }

    node->next = current;
    node->prev = current->prev;

    if (current->prev != NULL) {
        current->prev->next = node;
    } else {
        book->head = node;
    }

    current->prev = node;
}

//отсоединяет узел, не освобождая его память 
static void detach_node(AddressBook *book, ContactNode *node)
{
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        book->head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        book->tail = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

void address_book_init(AddressBook *book)
{
    if (book == NULL) {
        return;
    }

    book->head = NULL;
    book->tail = NULL;
    book->count = 0;
    book->next_id = 1;
}

void address_book_destroy(AddressBook *book)
{
    if (book == NULL) {
        return;
    }

    ContactNode *current = book->head;
    while (current != NULL) {
        ContactNode *next = current->next;
        contact_destroy(&current->contact);
        free(current);
        current = next;
    }

    book->head = NULL;
    book->tail = NULL;
    book->count = 0;
}

int address_book_add(AddressBook *book, Contact *contact)
{
    if (book == NULL || contact == NULL || book->next_id == INT_MAX) {
        return 0;
    }

    ContactNode *node = malloc(sizeof(*node));
    if (node == NULL) {
        return 0;
    }

    contact->id = book->next_id++;
    node->contact = *contact;
    node->prev = NULL;
    node->next = NULL;

    //после копирования динамическими данными владеет новый узел 
    contact_init(contact);

    insert_sorted(book, node);
    ++book->count;
    return 1;
}

Contact *address_book_find(AddressBook *book, int id)
{
    if (book == NULL || id <= 0) {
        return NULL;
    }

    ContactNode *current = book->head;
    while (current != NULL) {
        if (current->contact.id == id) {
            return &current->contact;
        }
        current = current->next;
    }

    return NULL;
}

int address_book_remove(AddressBook *book, int id)
{
    if (book == NULL) {
        return 0;
    }

    ContactNode *current = book->head;
    while (current != NULL && current->contact.id != id) {
        current = current->next;
    }

    if (current == NULL) {
        return 0;
    }

    detach_node(book, current);
    contact_destroy(&current->contact);
    free(current);
    --book->count;
    return 1;
}

int address_book_reorder(AddressBook *book, int id)
{
    if (book == NULL) {
        return 0;
    }

    ContactNode *current = book->head;
    while (current != NULL && current->contact.id != id) {
        current = current->next;
    }

    if (current == NULL) {
        return 0;
    }

    detach_node(book, current);
    insert_sorted(book, current);
    return 1;
}
