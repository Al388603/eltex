#include "ui.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INPUT_LEN 512
static int read_line(
    const char *prompt,
    char *buffer,
    size_t size
)
{
    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (fgets(buffer, (int)size, stdin) == NULL)
        return 0;
    size_t length = strlen(buffer);
    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    }
    else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
        printf("ввод был обрезан\n");
    }
    //удаление пробелов в начале
    char *start = buffer;
    while (isspace((unsigned char)*start))
        ++start;
    if (start != buffer) {
        memmove(buffer, start, strlen(start) + 1);
    }
    //удаление пробелов в конце
    length = strlen(buffer);
    while (length > 0 &&
           isspace((unsigned char)buffer[length - 1])) {
        buffer[--length] = '\0';
    }
    return 1;
}
//чтение целого числа
static int read_int(const char *prompt, int *value)
{
    char line[INPUT_LEN];
    char *end = NULL;
    long number;
    for (;;) {
        if (!read_line(prompt, line, sizeof(line)))
            return 0;
        errno = 0;
        number = strtol(line, &end, 10);
        if (line[0] != '\0' &&
            end != line &&
            *end == '\0' &&
            errno != ERANGE &&
            number >= INT_MIN &&
            number <= INT_MAX) {
            *value = (int)number;
            return 1;
        }
        printf("ошибка: введите целое число.\n");
    }
}
//чтение обязательного поля
static int read_required(
    const char *prompt,
    char *buffer,
    size_t size
)
{
    for (;;) {
        if (!read_line(prompt, buffer, size))
            return 0;
        if (buffer[0] != '\0')
            return 1;
        printf("поле обязательно. повторите ввод.\n");
    }
}
//чтение необязательного поля
static int read_optional(
    const char *prompt,
    char *buffer,
    size_t size
)
{
    if (!read_line(prompt, buffer, size))
        return 0;
    return 1;
}
//проверка электронной почты
static int valid_email(const char *email)
{
    const char *at = strchr(email, '@');
    if (at == NULL || at == email || at[1] == '\0')
        return 0;
    if (strchr(at + 1, '@') != NULL)
        return 0;
    const char *dot = strrchr(at + 1, '.');
    return dot != NULL &&
           dot > at + 1 &&
           dot[1] != '\0';
}
//проверка телефонного номера
static int valid_phone(const char *phone)
{
    int digits = 0;
    for (size_t i = 0; phone[i] != '\0'; ++i) {
        unsigned char ch = (unsigned char)phone[i];
        if (isdigit(ch)) {
            ++digits;
        }
        else if (ch != '+' &&
                 ch != '-' &&
                 ch != '(' &&
                 ch != ')' &&
                 ch != ' ') {

            return 0;
        }
    }
    return digits >= 5;
}
//добавление необязательного элемента в список
static int prompt_add_to_list(
    StringList *list,
    const char *label,
    int email_mode
)
{
    char value[INPUT_LEN];
    if (!read_optional(label, value, sizeof(value)))
        return 0;
    if (value[0] == '\0')
        return 1;
    if (email_mode && !valid_email(value)) {
        printf("некорректный адрес почты\n");
        return 1;
    }
    if (!email_mode &&
        strcmp(label, "Телефон: ") == 0 &&
        !valid_phone(value)) {
        printf("некорректный телефон\n");
        return 1;
    }

    if (!string_list_add(list, value)) {
        printf("не удалось выделить память\n");
        return 0;
    }
    return 1;
}

//вывод динамического списка
static void print_list(
    const StringList *list,
    const char *title
)
{
    printf("%s", title);
    if (list->count == 0) {
        printf("нет\n");
        return;
    }
    printf("\n");
    for (size_t i = 0; i < list->count; ++i) {
        printf("  %zu) %s\n",
               i + 1,
               list->items[i]);
    }
}

//вывод информации о контакте
static void print_contact(const Contact *c)
{
    printf("\n========== контакт ==========\n");

    printf("ID: %d\n", c->id);

    printf("фамилия: %s\n", c->surname);
    printf("имя: %s\n", c->name);

    printf("отчество: %s\n",
           c->patronymic[0] ?
           c->patronymic : "не указано");

    printf("место работы: %s\n",
           c->workplace[0] ?
           c->workplace : "не указано");

    printf("должность: %s\n",
           c->position[0] ?
           c->position : "не указана");

    print_list(&c->phones, "телефоны: ");
    print_list(&c->emails, "электронная почта: ");
    print_list(&c->social_links, "социальные сети: ");
}

//добавление нового контакта
static void add_contact(AddressBook *book)
{
    Contact c;
    char value[INPUT_LEN];
    contact_init(&c);
    printf("\n--- Добавление контакта ---\n");
    if (!read_required(
            "фамилия (обязательно): ",
            c.surname,
            sizeof(c.surname)
        ) ||

        !read_required(
            "имя (обязательно): ",
            c.name,
            sizeof(c.name)
        )) {

        contact_destroy(&c);
        return;
    }

    if (!read_optional(
            "отчество (можно пропустить): ",
            c.patronymic,
            sizeof(c.patronymic)
        ) ||

        !read_optional(
            "место работы: ",
            c.workplace,
            sizeof(c.workplace)
        ) ||

        !read_optional(
            "должность: ",
            c.position,
            sizeof(c.position)
        )) {

        contact_destroy(&c);
        return;
    }

    //добавление телефона
    if (read_optional(
            "телефон: ",
            value,
            sizeof(value)
        ) && value[0]) {

        if (valid_phone(value)) {

            if (!string_list_add(&c.phones, value)) {
                printf("ошибка памяти при добавлении телефона\n");
            }
        }
        else {
            printf("телефон не добавлен: некорректный формат\n");
        }
    }

    //добавление первой почты
    if (read_optional(
            "первая почта: ",
            value,
            sizeof(value)
        ) && value[0]) {

        if (valid_email(value)) {

            if (!contact_add_email_args(&c, 1, value)) {
                printf("не удалось добавить почту\n");
            }
        }
        else {
            printf("почта не добавлена: некорректный формат\n");
        }
    }

    //добавление ссылки на социальную сеть
    prompt_add_to_list(
        &c.social_links,
        "ссылка на соцсеть: ",
        0
    );

    //добавляем контакт в телефонную книгу
    if (!address_book_add(book, &c)) {

        printf("не удалось добавить контакт.\n");

        contact_destroy(&c);

        return;
    }

    printf(
        "контакт успешно добавлен. Его ID: %d\n",
        book->items[book->count - 1].id
    );
}

//редактирование текстового поля
static void edit_text_field(
    const char *label,
    char *field,
    size_t size
)
{
    char value[INPUT_LEN];

    printf(
        "%sтекущее значение: %s\n",
        label,
        field[0] ? field : "(пусто)"
    );

    if (!read_optional(
            "новое значение: ",
            value,
            sizeof(value)
        )) {

        return;
    }

    if (value[0] != '\0') {
        snprintf(field, size, "%s", value);
    }
}

//управление динамическим списком
static void manage_list(
    StringList *list,
    const char *title,
    int email_mode,
    int phone_mode
)
{
    int choice;

    for (;;) {

        printf("\n--- %s ---\n", title);

        print_list(list, "текущие значения: ");

        printf("1. добавить\n");
        printf("2. удалить\n");
        printf("0. назад\n");

        if (!read_int("ваш выбор: ", &choice))
            return;

        if (choice == 0)
            return;

        //добавление элемента
        if (choice == 1) {

            char value[INPUT_LEN];

            if (!read_required(
                    "введите значение: ",
                    value,
                    sizeof(value)
                )) {

                return;
            }

            if (email_mode && !valid_email(value)) {

                printf("некорректный email\n");
                continue;
            }

            if (phone_mode && !valid_phone(value)) {

                printf("некорректный телефон\n");
                continue;
            }

            if (string_list_add(list, value)) {
                printf("добавлено\n");
            }
            else {
                printf("ошибка выделения памяти\n");
            }
        }

        //удаление элемента
        else if (choice == 2) {

            int index;

            if (list->count == 0) {

                printf("список пуст\n");
                continue;
            }

            if (!read_int(
                    "номер элемента для удаления: ",
                    &index
                )) {

                return;
            }

            if (index < 1 ||
                (size_t)index > list->count) {

                printf("нет элемента с таким номером\n");
                continue;
            }

            string_list_remove(
                list,
                (size_t)(index - 1)
            );

            printf("удалено\n");
        }

        else {
            printf("неизвестный пункт меню\n");
        }
    }
}

//редактирование контакта
static void edit_contact(AddressBook *book)
{
    int id;

    if (!read_int(
            "введите ID контакта для редактирования: ",
            &id
        )) {

        return;
    }

    Contact *c = address_book_find(book, id);

    if (c == NULL) {

        printf("контакт с таким ID не найден\n");

        return;
    }

    int choice;

    for (;;) {

        printf(
            "\n--- редактирование контакта ID=%d ---\n",
            c->id
        );

        printf("1. фамилия\n");
        printf("2. имя\n");
        printf("3. отчество\n");
        printf("4. место работы\n");
        printf("5. должность\n");
        printf("6. телефоны\n");
        printf("7. электронная почта\n");
        printf("8. социальные сети\n");
        printf("9. показать контакт\n");
        printf("0. назад\n");

        if (!read_int("ваш выбор: ", &choice))
            return;

        switch (choice) {

            case 0:
                return;

            case 1:
                edit_text_field(
                    "фамилия ",
                    c->surname,
                    sizeof(c->surname)
                );
                break;

            case 2:
                edit_text_field(
                    "имя ",
                    c->name,
                    sizeof(c->name)
                );
                break;

            case 3:
                edit_text_field(
                    "отчество ",
                    c->patronymic,
                    sizeof(c->patronymic)
                );
                break;

            case 4:
                edit_text_field(
                    "место работы ",
                    c->workplace,
                    sizeof(c->workplace)
                );
                break;

            case 5:
                edit_text_field(
                    "должность ",
                    c->position,
                    sizeof(c->position)
                );
                break;

            case 6:
                manage_list(
                    &c->phones,
                    "телефоны",
                    0,
                    1
                );
                break;

            case 7:
                manage_list(
                    &c->emails,
                    "электронная почта",
                    1,
                    0
                );
                break;

            case 8:
                manage_list(
                    &c->social_links,
                    "социальные сети",
                    0,
                    0
                );
                break;

            case 9:
                print_contact(c);
                break;

            default:
                printf("неизвестный пункт меню\n");
        }
    }
}

//удаление контакта
static void delete_contact(AddressBook *book)
{
    int id;

    if (!read_int(
            "введите ID контакта для удаления: ",
            &id
        )) {

        return;
    }

    Contact *c = address_book_find(book, id);

    if (c == NULL) {

        printf("контакт с таким ID не найден\n");

        return;
    }

    print_contact(c);

    char answer[INPUT_LEN];

    if (!read_optional(
            "точно удалить контакт? (да/нет): ",
            answer,
            sizeof(answer)
        )) {

        return;
    }

    if (strcmp(answer, "да") == 0 ||
        strcmp(answer, "ДА") == 0 ||
        strcmp(answer, "д") == 0 ||
        strcmp(answer, "Д") == 0) {

        if (address_book_remove(book, id)) {
            printf("Контакт удалён.\n");
        }
    }
    else {
        printf("удаление отменено\n");
    }
}

//вывод всех контактов
static void show_all(const AddressBook *book)
{
    if (book->count == 0) {

        printf("телефонная книга пуста\n");

        return;
    }

    for (size_t i = 0; i < book->count; ++i) {
        print_contact(&book->items[i]);
    }
}

//поиск и вывод одного контакта
static void show_one(AddressBook *book)
{
    int id;

    if (!read_int("введите ID контакта: ", &id))
        return;

    Contact *c = address_book_find(book, id);

    if (c == NULL) {
        printf("контакт не найден\n");
    }
    else {
        print_contact(c);
    }
}

//главное меню
void run_menu(AddressBook *book)
{
    int choice;

    for (;;) {

        printf("\n========== телефонная книга ==========\n");

        printf("1. добавить контакт\n");
        printf("2. показать все контакты\n");
        printf("3. найти контакт по ID\n");
        printf("4. редактировать контакт\n");
        printf("5. удалить контакт\n");
        printf("0. выход\n");

        if (!read_int("выберите действие: ", &choice)) {

            printf("\nзавершение программы.\n");

            return;
        }

        switch (choice) {

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

            case 0:
                printf("до свидания\n");
                return;

            default:
                printf(
                    "такого пункта меню нет. выберите ещё раз\n"
                );
        }
    }
}