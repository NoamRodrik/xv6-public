#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/syscall.h"
#include "user/user.h"

static void closepipe(int pipe)
{
    if (close(pipe) == -1)
    {
        fprintf(2, "failed to close pipe\n");
        exit(-1);
    }
}

static void openpipe(int* pipes)
{
    if (pipe(pipes) == -1)
    {
        fprintf(2, "failed opening anonymous pipes\n");
        exit(-1);
    }
}

static int startfork()
{
    int pid = fork();
    if (pid < 0)
    {
        fprintf(2, "fork failed\n");
        exit(-1);
    }

    return pid;
}

static void writepipe(int fd)
{
    static const char BUFF[] = "\0";
    if (write(fd, BUFF, sizeof(BUFF)) == -1)
    {
        fprintf(2, "failed writing to pipe\n");
        exit(-1);
    }
}

static int readpipe(int fd)
{
    char buff[1] = {};
    int ret = read(fd, buff, sizeof(buff));
    if (ret == -1)
    {
        fprintf(2, "failed reading from pipe\n");
        exit(-1);
    }

    return ret;
}

int
main(int argc, char* argv[])
{
  int pipes_parent[2] = {};
  int pipes_child[2] = {};
  int pid = 0;

  openpipe(pipes_parent);
  openpipe(pipes_child);

  pid = startfork();

  if (pid > 0)
  {
    /* We are the parent */
    writepipe(pipes_parent[1]);
    if (readpipe(pipes_child[0]) > 0)
    {
        fprintf(1, "%d: received pong\n", getpid());
    }
  }
  else
  {
      /* We are the child */
      if (readpipe(pipes_parent[0]) > 0)
      {
          fprintf(1, "%d: received ping\n", getpid());
          writepipe(pipes_child[1]);
      }
  }

  closepipe(pipes_parent[0]);
  closepipe(pipes_parent[1]);
  closepipe(pipes_child[0]);
  closepipe(pipes_child[1]);

  exit(0);
}
