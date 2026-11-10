#ifndef CONTACT_H
#define CONTACT_H
#include <stddef.h>
#define NAME_LEN 64
#define WORK_LEN 100
#define POSITION_LEN 100
typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} StringList;
typedef struct {
    int id;
    char surname[NAME_LEN];
    char name[NAME_LEN];
    char patronymic[NAME_LEN];
    char workplace[WORK_LEN];
    char position[POSITION_LEN];
    StringList phones;
    StringList emails;
    StringList social_links;
} Contact;
void contact_init(Contact *contact);
void contact_destroy(Contact *contact);
int contact_add_email_args(Contact *contact, size_t count, ...);
int string_list_add(StringList *list, const char *value);
int string_list_remove(StringList *list, size_t index);
void string_list_destroy(StringList *list);
#endif
