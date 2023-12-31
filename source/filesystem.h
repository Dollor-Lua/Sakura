#pragma once

#include "sstr.h"

struct s_str readfile(const char *path);
struct s_str readfile_s(const struct s_str *path);
int writefile(const char *path, const struct s_str *content);
int writefile_c(const char *path, const char *content);
int removefile(const char *path);