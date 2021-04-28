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

static void writeintpipe(int fd, int num)
{
  if (write(fd, &num, sizeof(num)) == -1)
  {
    fprintf(2, "failed writing to pipe\n");
    exit(-1);
  }
}

static int readintpipe(int fd)
{
  char buff[4] = {};
  int read_res = read(fd, &buff, sizeof(int));
  if (read_res == -1)
  {
    fprintf(2, "failed reading from pipe\n");
    exit(-1);
  }

  return read_res == 0 ? -1 : *((int*)buff);
}

static int isprime(int num)
{
  for (int i = 2; i < num; ++i)
  {
    if (num % i == 0)
    {
      return 0;
    }
  }

  return 1;
}

static void wait_for_pid(int pid)
{
  if (wait((int*)0) == -1)
  {
    fprintf(2, "failed waiting for pid %d\n", pid);
    exit(-1);
  }
}

static void fill_pipe(int pipe, int from, int to)
{
  /* Write numbers to the pipeline. */
  for (int i = from; i <= to; ++i)
  {
    writeintpipe(pipe, i);
  }
}

static int firstprimepipe(int pipe)
{
  int num = 0;

  while ((num = readintpipe(pipe)) != -1)
  {
    if (isprime(num))
    {
      break;
    }
  }

  return num;
}

static int is_first_process(int pipe_id)
{
  return pipe_id == -1;
}

static void filter_possible_primes(int from_pipe, int to_pipe, int prime)
{
  int num = 0;

  if (is_first_process(from_pipe))
  {
    return;
  }

  while ((num = readintpipe(from_pipe)) != -1)
  {
    if (num % prime != 0)
    {
      writeintpipe(to_pipe, num);
    }
  }
}

static void
child(int parent_pipe)
{
  int pipes_parent[2] = {};
  int prime = 0;
  int pid = 0;

  if (is_first_process(parent_pipe))
  {
    openpipe(pipes_parent);
    fill_pipe(pipes_parent[1], 2, 35);
  }
  else
  {
    if ((prime = firstprimepipe(parent_pipe)) == -1)
    {
      /* No more primes left to read */
      return;
    }

    openpipe(pipes_parent);
    fprintf(1, "prime %d\n", prime);
  }

  pid = startfork();

  if (pid > 0)
  {
    closepipe(pipes_parent[0]);
    filter_possible_primes(parent_pipe, pipes_parent[1], prime);
    closepipe(pipes_parent[1]);

    wait_for_pid(pid);
  }
  else
  {
    closepipe(pipes_parent[1]);
    child(pipes_parent[0]);
    closepipe(pipes_parent[0]);

    exit(0);
  }
}

int
main(int argc, char* argv[])
{
  child(-1);
  exit(0);
}
