#ifndef POSIX_SM_H
#define POSIX_SM_H

#include <stdio.h>
#include <sys/stat.h>

int scoria_sm_open(const char *name, int oflag, mode_t mode, const char *msg);

void scoria_sm_unlink(const char *name, const char *msg);

void scoria_sm_truncate(const int fd, const size_t length, const char *msg);

void *scoria_sm_map(void *addr, const size_t length, const int prot,
                    const int flags, const int fd, const off_t offset,
                    const char *msg);

void scoria_sm_unmap(void *ptr, const size_t length, const char *msg);

#endif /* POSIX_SM_H */
