#include "address_book.h"
#include "ui.h"
#include <stdio.h>
int main(void)
{
    AddressBook book;
    address_book_init(&book);
    printf("телефонная книга запущена\n");
    run_menu(&book);
    address_book_destroy(&book);
    return 0;
}
