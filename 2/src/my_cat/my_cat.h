#pragma once

//---------------------------------------------------------

#include <unistd.h>

//=========================================================

static const size_t Buffer_size = 4096;

//=========================================================

int my_cat(int argc, const char** argv);

ssize_t safe_write(int fd, const void* buf, size_t count);
