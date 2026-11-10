#include "address_book.h"
#include <stdlib.h>
#include <string.h>
//инициализация телефонной книги
void address_book_init(AddressBook *book)
{
    if (book == NULL) return;
    book->items = NULL;
    book->count = 0;
    book->capacity = 0;
    book->next_id = 1;
}
//освобождение всей телефонной книги
void address_book_destroy(AddressBook *book)
{
    if (book == NULL) return;
    for (size_t i = 0; i < book->count; ++i) {
        contact_destroy(&book->items[i]);
    }
    free(book->items);
    book->items = NULL;
    book->count = 0;
    book->capacity = 0;
}
//добавление контакта
int address_book_add(AddressBook *book, Contact *contact)
{
    if (book == NULL || contact == NULL)
        return 0;
    //увеличение массива контактов
    if (book->count == book->capacity) {
        size_t new_capacity =
            (book->capacity == 0) ? 4 : book->capacity * 2;
        Contact *new_items = realloc(
            book->items,
            new_capacity * sizeof(Contact)
        );
        if (new_items == NULL)
            return 0;
        book->items = new_items;
        book->capacity = new_capacity;
    }
    //присваиваем уникальный ID
    contact->id = book->next_id++;
    book->items[book->count++] = *contact;
    //передаём владение динамическими массивами 
    contact_init(contact);
    return 1;
}

//поиск контакта по ID
Contact *address_book_find(AddressBook *book, int id)
{
    if (book == NULL || id <= 0)
        return NULL;
    for (size_t i = 0; i < book->count; ++i) {
        if (book->items[i].id == id) {
            return &book->items[i];
        }
    }
    return NULL;
}
//удаление контакта
int address_book_remove(AddressBook *book, int id)
{
    if (book == NULL)
        return 0;
    for (size_t i = 0; i < book->count; ++i) {
        if (book->items[i].id == id) {
            contact_destroy(&book->items[i]);
            //сдвигаем элементы массива
            for (size_t j = i; j + 1 < book->count; ++j) {
                book->items[j] = book->items[j + 1];
            }
            --book->count;
            return 1;
        }
    }
    return 0;
}