#pragma once

struct s_str {
    char *str;
    int len;
};

#define S_NULL_STR                                                                                                     \
    { NULL, 0 }

#define SI_NULL_STR (struct s_str) S_NULL_STR

#define S_C_STR(c)                                                                                                     \
    { c, sizeof(c) - 1 }

#define SI_C_STR(c) (struct s_str) S_C_STR(c)

#define s_str(s) s_str_new(s)

struct s_str s_str(const char *str);
struct s_str s_str_n(const char *str, unsigned int len);
void s_str_free(struct s_str *sstr);
struct s_str s_str_concat(const struct s_str *sstr1, const struct s_str *sstr2);
struct s_str s_str_concat_c(const struct s_str *sstr, const char *str);
struct s_str s_str_concat_s(const char *str, const struct s_str *sstr2);
struct s_str s_str_concat_cc(const char *str, const char *str2);
int s_str_cmp(const struct s_str *sstr1, const struct s_str *sstr2);
int s_str_cmp_c(const struct s_str *sstr1, const char *str);