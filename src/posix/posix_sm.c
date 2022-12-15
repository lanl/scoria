#include "posix_sm.h"
#include "config.h"
#include "utils.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int scoria_sm_open(const char *name, int oflag, mode_t mode, const char *msg) {
  int fd;

  if ((fd = shm_open(name, oflag, mode)) == -1)
    scoria_error(msg);

  return fd;
}

void scoria_sm_unlink(const char *name, const char *msg) {
  if (shm_unlink(name) == -1)
    scoria_error(msg);
}

void scoria_sm_truncate(const int fd, const size_t length, const char *msg) {
  if (ftruncate(fd, length) == -1)
    scoria_error(msg);
}

void *scoria_sm_map(void *addr, const size_t length, const int prot,
                    const int flags, const int fd, const off_t offset,
                    const char *msg) {
  void *ptr;

  if ((ptr = mmap(addr, length, prot, flags, fd, offset)) == MAP_FAILED)
    scoria_error(msg);

  return ptr;
}

void scoria_sm_unmap(void *ptr, const size_t length, const char *msg) {
  if (munmap(ptr, length) == -1)
    scoria_error(msg);
}
