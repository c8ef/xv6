#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define R 0
#define W 1
int parent_fd[2], child_fd[2];

int main(int argc, char **argv) {
  pipe(parent_fd);
  pipe(child_fd);

  char buf = ' ';

  if (fork() == 0) {
    close(child_fd[R]);
    close(parent_fd[W]);
    read(parent_fd[R], &buf, 1);
    printf("%d: received ping\n", getpid());
    write(child_fd[W], &buf, 1);
    close(parent_fd[R]);
    close(child_fd[W]);
  } else {
    close(parent_fd[R]);
    close(child_fd[W]);
    write(parent_fd[W], &buf, 1);
    wait((int *)0);
    read(child_fd[R], &buf, 1);
    printf("%d: received pong\n", getpid());
    close(child_fd[R]);
    close(parent_fd[W]);
  }

  exit(0);
}
