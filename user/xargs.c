#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char **argv) {
  char *_argv[MAXARG + 1];
  for (int i = 1; i < argc; ++i) _argv[i - 1] = argv[i];
  int _argc = argc - 1;

  char buf;
  int param_len = 0;
  char param[MAXPATH];
  while (read(0, &buf, sizeof(char)) > 0) {
    if (buf == ' ' || buf == '\n') {
      param[param_len] = 0;
      _argv[_argc] = malloc(param_len + 1);
      strcpy(_argv[_argc], param);
      _argc++;
      param_len = 0;
      continue;
    }
    param[param_len++] = buf;
  }

  _argv[_argc] = 0;
  if (fork() == 0) {
    exec(_argv[0], _argv);
  } else {
    wait((int *)0);
  }
  exit(0);
}
