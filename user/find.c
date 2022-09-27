#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *dirname, char *filename, char *target) {
  int fd;
  struct stat st;
  char buf[512], *p;
  struct dirent de;

  if ((fd = open(dirname, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", dirname);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "ls: cannot stat %s\n", dirname);
    close(fd);
    return;
  }

  switch (st.type) {
    case T_FILE:
      if (strcmp(filename, target) == 0) {
        printf("%s\n", dirname);
      }
      break;
    case T_DIR:
      if (strlen(dirname) + 1 + DIRSIZ + 1 > sizeof(buf)) {
        printf("find: path too long\n");
        break;
      }
      strcpy(buf, dirname);
      p = buf + strlen(buf);
      *p++ = '/';
      while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = '\0';
        find(buf, filename, de.name);
      }
      break;
    default:
      break;
  }
  close(fd);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(2, "Usage: find <dirname> <filename>\n");
    exit(1);
  }

  char *dirname = argv[1];
  char *filename = argv[2];
  find(dirname, filename, "");

  exit(0);
}
