#include "sstr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct s_str s_str(const char *str) {
    struct s_str s;
    s.len = strlen(str);
    s.str = malloc(s.len);
    memcpy(s.str, str, s.len);
    return s;
}

struct s_str s_str_n(const char *str, unsigned int len) {
    struct s_str s;
    s.len = len;
    s.str = malloc(len);
    memcpy(s.str, str, s.len);
    return s;
}

struct s_str s_str_copy(const struct s_str *sstr) {
    struct s_str s;
    s.len = sstr->len;
    s.str = malloc(s.len);
    memcpy(s.str, sstr->str, s.len);
    return s;
}

void s_str_free(struct s_str *sstr) {
    free(sstr->str);
    sstr->str = NULL;
    sstr->len = 0;
}

struct s_str s_str_concat(const struct s_str *s1, const struct s_str *s2) {
    struct s_str s;
    s.len = s1->len + s2->len;
    s.str = malloc(s.len);
    memcpy(s.str, s1->str, s1->len);
    memcpy(s.str + s1->len, s2->str, s2->len);
    return s;
}

struct s_str s_str_concat_c(const struct s_str *s1, const char *s2) {
    struct s_str s;
    s.len = s1->len + strlen(s2);
    s.str = malloc(s.len);
    memcpy(s.str, s1->str, s1->len);
    memcpy(s.str + s1->len, s2, strlen(s2));
    return s;
}

struct s_str s_str_concat_s(const char *s1, const struct s_str *s2) {
    struct s_str s;
    s.len = strlen(s1) + s2->len;
    s.str = malloc(s.len);
    memcpy(s.str, s1, strlen(s1));
    memcpy(s.str + strlen(s1), s2->str, s2->len);
    return s;
}

struct s_str s_str_concat_cc(const char *s1, const char *s2) {
    struct s_str s;
    s.len = strlen(s1) + strlen(s2);
    s.str = malloc(s.len);
    memcpy(s.str, s1, strlen(s1));
    memcpy(s.str + strlen(s1), s2, strlen(s2));
    return s;
}

struct s_str s_str_concat_d(const struct s_str *sstr1, double value) {
    char output[50];
    sprintf(output, "%f", value);

    size_t len = strlen(output);
    for (size_t i = len - 1; i >= 0; i--) {
        if (output[i] == '0')
            output[i] = '\0';
        else
            break;
    }

    len = strlen(output);
    if (output[len - 1] == '.')
        output[len - 1] = '\0';
    len = strlen(output);

    struct s_str s;
    s.len = sstr1->len + len;
    s.str = malloc(s.len * sizeof(char));
    memcpy(s.str, sstr1->str, sstr1->len);
    memcpy(s.str + sstr1->len, output, len);
    return s;
}

struct s_str s_str_concat_dd(double value, const struct s_str *sstr1) {
    char output[50];
    sprintf(output, "%f", value);

    size_t len = strlen(output);
    for (size_t i = len - 1; i >= 0; i--) {
        if (output[i] == '0')
            output[i] = '\0';
        else
            break;
    }

    len = strlen(output);
    if (output[len - 1] == '.')
        output[len - 1] = '\0';
    len = strlen(output);

    struct s_str s;
    s.len = len + sstr1->len;
    s.str = malloc(s.len * sizeof(char));
    memcpy(s.str, output, len);
    memcpy(s.str + len, sstr1->str, sstr1->len);
    return s;
}

int s_str_cmp(const struct s_str *s1, const struct s_str *s2) {
    if (s1->len != s2->len)
        return s1->len - s2->len;
    return memcmp(s1->str, s2->str, s1->len);
}

int s_str_cmp_c(const struct s_str *s1, const char *s2) {
    int len = strlen(s2);
    if (s1->len != len)
        return s1->len - len;
    return memcmp(s1->str, s2, len);
}