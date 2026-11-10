#include "contact.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
//копирует строку в отдельную область памяти 
static char *copy_string(const char *source)
{
    size_t length = strlen(source) + 1;
    char *copy = malloc(length);

    if (copy != NULL) {
        memcpy(copy, source, length);
    }

    return copy;
}
void contact_init(Contact *contact)
{
    if (contact == NULL) {
        return;
    }

    memset(contact, 0, sizeof(*contact));
}
void string_list_destroy(StringList *list)
{
    if (list == NULL) {
        return;
    }

    for (size_t i = 0; i < list->count; ++i) {
        free(list->items[i]);
    }

    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}
void contact_destroy(Contact *contact)
{
    if (contact == NULL) {
        return;
    }

    string_list_destroy(&contact->phones);
    string_list_destroy(&contact->emails);
    string_list_destroy(&contact->social_links);
    memset(contact, 0, sizeof(*contact));
}
int string_list_add(StringList *list, const char *value)
{
    if (list == NULL || value == NULL || value[0] == '\0') {
        return 0;
    }

    if (list->count == list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 2 : list->capacity * 2;
        char **new_items = realloc(list->items, new_capacity * sizeof(*new_items));

        if (new_items == NULL) {
            return 0;
        }

        list->items = new_items;
        list->capacity = new_capacity;
    }
    char *copy = copy_string(value);
    if (copy == NULL) {
        return 0;
    }

    list->items[list->count++] = copy;
    return 1;
}
int string_list_remove(StringList *list, size_t index)
{
    if (list == NULL || index >= list->count) {
        return 0;
    }

    free(list->items[index]);

    for (size_t i = index; i + 1 < list->count; ++i) {
        list->items[i] = list->items[i + 1];
    }

    --list->count;
    return 1;
}
int contact_add_email_args(Contact *contact, size_t count, ...)
{
    if (contact == NULL) {
        return 0;
    }

    va_list args;
    va_start(args, count);

    for (size_t i = 0; i < count; ++i) {
        const char *email = va_arg(args, const char *);
        if (email == NULL || !string_list_add(&contact->emails, email)) {
            va_end(args);
            return 0;
        }
    }

    va_end(args);
    return 1;
}
