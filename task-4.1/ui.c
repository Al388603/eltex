#include "ui.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INPUT_LEN 512
static int read_line(const char *prompt, char *buffer, size_t size)
{
    if (prompt != NULL) {
        printf("%s", prompt);
    }

    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }

    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[--length] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }

    size_t start = 0;
    while (isspace((unsigned char)buffer[start])) {
        ++start;
    }

    while (length > start && isspace((unsigned char)buffer[length - 1])) {
        buffer[--length] = '\0';
    }

    if (start > 0) {
        memmove(buffer, buffer + start, length - start + 1);
    }

    return 1;
}

static int read_int(const char *prompt, int *value)
{
    char buffer[INPUT_LEN];
    char *end = NULL;
    long number;

    for (;;) {
        if (!read_line(prompt, buffer, sizeof(buffer))) {
            return 0;
        }

        errno = 0;
        number = strtol(buffer, &end, 10);

        if (buffer[0] != '\0' && *end == '\0' && errno != ERANGE &&
            number >= INT_MIN && number <= INT_MAX) {
            *value = (int)number;
            return 1;
        }

        printf("ошибка: введите целое число\n");
    }
}

static int read_required(const char *prompt, char *buffer, size_t size)
{
    for (;;) {
        if (!read_line(prompt, buffer, size)) {
            return 0;
        }
        if (buffer[0] != '\0') {
            return 1;
        }
        printf("поле не должно быть пустым\n");
    }
}

static int read_optional(const char *prompt, char *buffer, size_t size)
{
    return read_line(prompt, buffer, size);
}

static int valid_email(const char *email)
{
    const char *at = strchr(email, '@');
    return at != NULL && at != email && strchr(at + 1, '.') != NULL;
}

static int valid_phone(const char *phone)
{
    int digits = 0;

    for (size_t i = 0; phone[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)phone[i];
        if (isdigit(ch)) {
            ++digits;
        } else if (ch != '+' && ch != '-' && ch != '(' && ch != ')' && ch != ' ') {
            return 0;
        }
    }

    return digits >= 5;
}

static void print_list(const char *title, const StringList *list)
{
    printf("%s", title);
    if (list->count == 0) {
        printf("нет\n");
        return;
    }

    printf("\n");
    for (size_t i = 0; i < list->count; ++i) {
        printf("  %zu. %s\n", i + 1, list->items[i]);
    }
}

static void print_contact(const Contact *contact)
{
    printf("\nid: %d\n", contact->id);
    printf("фио: %s %s %s\n", contact->surname, contact->name,
           contact->patronymic);
    printf("место работы: %s\n", contact->workplace[0] ? contact->workplace : "не указано");
    printf("должность: %s\n", contact->position[0] ? contact->position : "не указана");
    print_list("телефоны: ", &contact->phones);
    print_list("почта: ", &contact->emails);
    print_list("ссылки: ", &contact->social_links);
}

static int add_value(StringList *list, const char *label, int type)
{
    char value[INPUT_LEN];
    if (!read_optional(label, value, sizeof(value))) {
        return 0;
    }

    if (value[0] == '\0') {
        return 1;
    }

    if ((type == 1 && !valid_phone(value)) ||
        (type == 2 && !valid_email(value))) {
        printf("неверный формат. значение не добавлено\n");
        return 1;
    }

    if (!string_list_add(list, value)) {
        printf("не удалось выделить память\n");
        return 0;
    }

    return 1;
}

static void add_contact(AddressBook *book)
{
    Contact contact;
    char buffer[INPUT_LEN];
    contact_init(&contact);

    if (!read_required("фамилия: ", contact.surname, sizeof(contact.surname)) ||
        !read_required("имя: ", contact.name, sizeof(contact.name))) {
        contact_destroy(&contact);
        return;
    }

    if (!read_optional("отчество: ", contact.patronymic, sizeof(contact.patronymic)) ||
        !read_optional("место работы: ", contact.workplace, sizeof(contact.workplace)) ||
        !read_optional("должность: ", contact.position, sizeof(contact.position))) {
        contact_destroy(&contact);
        return;
    }

    if (!add_value(&contact.phones, "телефон (можно оставить пустым): ", 1) ||
        !add_value(&contact.emails, "почта (можно оставить пустым): ", 2) ||
        !add_value(&contact.social_links, "ссылка (можно оставить пустым): ", 0)) {
        contact_destroy(&contact);
        return;
    }

    if (address_book_add(book, &contact)) {
        Contact *saved = address_book_find(book, book->next_id - 1);
        printf("контакт добавлен id: %d\n", saved != NULL ? saved->id : book->next_id - 1);
    } else {
        printf("не удалось добавить контакт\n");
        contact_destroy(&contact);
    }

    (void)buffer;
}

static void show_all(const AddressBook *book)
{
    if (book->head == NULL) {
        printf("список контактов пуст\n");
        return;
    }

    printf("\nконтакты в алфавитном порядке:\n");
    const ContactNode *current = book->head;
    while (current != NULL) {
        printf("id: %d | %s %s %s\n", current->contact.id,
               current->contact.surname, current->contact.name,
               current->contact.patronymic);
        current = current->next;
    }
}

static void show_one(AddressBook *book)
{
    int id;
    if (!read_int("введите id: ", &id)) {
        return;
    }

    Contact *contact = address_book_find(book, id);
    if (contact == NULL) {
        printf("контакт не найден\n");
        return;
    }

    print_contact(contact);
}

static void edit_text_field(const char *prompt, char *field, size_t size)
{
    char value[INPUT_LEN];
    if (!read_optional(prompt, value, sizeof(value))) {
        return;
    }

    if (value[0] != '\0') {
        snprintf(field, size, "%s", value);
    }
}

static void manage_list(StringList *list, const char *title, int type)
{
    int choice;

    for (;;) {
        printf("\n%s\n1. добавить\n2. удалить\n3. показать\n0. назад\n", title);
        if (!read_int("выбор: ", &choice)) {
            return;
        }

        if (choice == 0) {
            return;
        }

        if (choice == 1) {
            char value[INPUT_LEN];
            if (!read_required("значение: ", value, sizeof(value))) {
                return;
            }

            if ((type == 1 && !valid_phone(value)) ||
                (type == 2 && !valid_email(value))) {
                printf("неверный формат\n");
            } else if (!string_list_add(list, value)) {
                printf("не удалось выделить память\n");
            }
        } else if (choice == 2) {
            int index;
            if (list->count == 0) {
                printf("список пуст\n");
                continue;
            }

            for (size_t i = 0; i < list->count; ++i) {
                printf("%zu. %s\n", i + 1, list->items[i]);
            }

            if (!read_int("номер для удаления: ", &index)) {
                return;
            }

            if (index < 1 || (size_t)index > list->count ||
                !string_list_remove(list, (size_t)index - 1)) {
                printf("неверный номер\n");
            }
        } else if (choice == 3) {
            print_list("", list);
        } else {
            printf("неизвестный пункт меню\n");
        }
    }
}

static void edit_contact(AddressBook *book)
{
    int id;
    if (!read_int("введите id контакта: ", &id)) {
        return;
    }

    Contact *contact = address_book_find(book, id);
    if (contact == NULL) {
        printf("контакт не найден\n");
        return;
    }

    int choice;
    int name_changed = 0;

    for (;;) {
        printf("\nредактирование контакта %d\n", id);
        printf("1. фамилия\n2. имя\n3. отчество\n4. место работы\n");
        printf("5. должность\n6. телефоны\n7. почта\n8. ссылки\n9. показать контакт\n0. назад\n");

        if (!read_int("выбор: ", &choice)) {
            break;
        }

        switch (choice) {
            case 0:
                if (name_changed) {
                    address_book_reorder(book, id);
                }
                return;
            case 1:
                edit_text_field("новая фамилия: ", contact->surname, sizeof(contact->surname));
                name_changed = 1;
                break;
            case 2:
                edit_text_field("новое имя: ", contact->name, sizeof(contact->name));
                name_changed = 1;
                break;
            case 3:
                edit_text_field("новое отчество: ", contact->patronymic, sizeof(contact->patronymic));
                name_changed = 1;
                break;
            case 4:
                edit_text_field("новое место работы: ", contact->workplace, sizeof(contact->workplace));
                break;
            case 5:
                edit_text_field("новая должность: ", contact->position, sizeof(contact->position));
                break;
            case 6:
                manage_list(&contact->phones, "телефоны", 1);
                break;
            case 7:
                manage_list(&contact->emails, "почта", 2);
                break;
            case 8:
                manage_list(&contact->social_links, "ссылки", 0);
                break;
            case 9:
                print_contact(contact);
                break;
            default:
                printf("неизвестный пункт меню\n");
                break;
        }
    }

    if (name_changed) {
        address_book_reorder(book, id);
    }
}

static void delete_contact(AddressBook *book)
{
    int id;
    if (!read_int("введите id контакта для удаления: ", &id)) {
        return;
    }

    Contact *contact = address_book_find(book, id);
    if (contact == NULL) {
        printf("контакт не найден\n");
        return;
    }

    print_contact(contact);

    char answer[INPUT_LEN];
    if (!read_required("удалить этот контакт? (да/нет): ", answer, sizeof(answer))) {
        return;
    }

    if (strcmp(answer, "да") == 0 || strcmp(answer, "д") == 0) {
        if (address_book_remove(book, id)) {
            printf("контакт удалён\n");
        }
    } else {
        printf("удаление отменено\n");
    }
}

void run_menu(AddressBook *book)
{
    int choice;

    for (;;) {
        printf("\n--- телефонная книга ---\n");
        printf("1. добавить контакт\n");
        printf("2. показать все контакты\n");
        printf("3. найти контакт по id\n");
        printf("4. редактировать контакт\n");
        printf("5. удалить контакт\n");
        printf("0. выход\n");

        if (!read_int("выбор: ", &choice)) {
            break;
        }

        switch (choice) {
            case 0:
                return;
            case 1:
                add_contact(book);
                break;
            case 2:
                show_all(book);
                break;
            case 3:
                show_one(book);
                break;
            case 4:
                edit_contact(book);
                break;
            case 5:
                delete_contact(book);
                break;
            default:
                printf("неизвестный пункт меню\n");
                break;
        }
    }
}
