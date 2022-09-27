#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define R 0
#define W 1

int main() {
  int arr[34];
  int len = 34;
  for (int i = 0; i < len; ++i) arr[i] = i + 2;

  int fd[2];
  while (len > 0) {
    pipe(fd);
    if (fork() == 0) {
      close(fd[W]);
      len = 0;
      int buf = -1;
      while (read(fd[R], &buf, sizeof(int)) > 0) {
        arr[len++] = buf;
      }
      close(fd[R]);
    } else {
      close(fd[R]);
      int curr = -1;
      for (int i = 0; i < len; ++i) {
        if (curr == -1) {
          curr = arr[i];
          printf("prime %d\n", curr);
        } else {
          if (arr[i] % curr != 0) write(fd[W], &arr[i], sizeof(int));
        }
      }
      close(fd[W]);
      wait((int*)0);
      exit(0);
    }
  }
  exit(0);
}
