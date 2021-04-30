#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

static void
find(char* path, char* file)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  (void)de;
  (void)p;

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot find path %s\n", path);
    exit(1);
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    goto cleanup;
  }

  if (st.type != T_DIR) {
    fprintf(2, "find: path given isn't a directory: %s\n", path);
    goto cleanup;
  }

  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
    printf("find: path too long\n");
    goto cleanup;
  }

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
      continue;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    if (strcmp(de.name, file) == 0) {
      fprintf(1, "%s\n", buf);
    }

    if (stat(buf, &st) < 0) {
      fprintf(2, "find: cannot stat %s\n", buf);
      goto cleanup;
    }

    if (st.type == T_DIR) {
      find(buf, file);
    }
  }

  close(fd);
  return;

cleanup:
  close(fd);
  exit(1);
}

int
main(int argc, char *argv[])
{
  if (argc < 2 || argc > 3) {
    fprintf(2, "Usage: find (path) [file]\n");
    exit(1);
  } else if (argc < 3) {
    find(".", argv[1]);
  } else {
    find(argv[1], argv[2]);
  }

  exit(0);
}