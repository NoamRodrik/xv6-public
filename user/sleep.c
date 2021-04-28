#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/syscall.h"
#include "user/user.h"

int
main(int argc, char* argv[])
{
  if (argc < 2) {
    fprintf(2, "Usage: sleep <time>...\n");
    exit(1);
  }

  exit(sleep(atoi(argv[1])));
}
