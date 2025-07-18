#include <stdio.h>

int main(void)
{
  char buf[BUFSIZ];
  FILE *file;

  if (! (file = fopen("goaway.msg", "r"))) {
    printf("\nSorry, we're not open right now.  Try again later.\n\n");
  } else {
    while (fgets(buf, sizeof buf, file))
      fputs(buf, stdout);
    fclose(file);
  }
  fflush(stdout);
  sleep(1);
}
