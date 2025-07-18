/*
**  Converts the DATA/system/locations file into rooms.c
*/

#include "kernel.h"

static char sccsid[] = "@(#)convertloc.c	4.100.0 (IthilMUD)	6/02/90";

FILE *ifp, *ofp;
int b, j;
char x[4096], c;

LOCDATA tmp;

static char *get_string(FILE *file)
{
  char *y;

  for (y = x; (*y = fgetc(file)) != '^'; y++)
    ;
  *y = 0;
  fgetc(file);
  return x;
}

main()
  {
  if ((ifp = fopen("DATA/system/locations", "r")) == NULL)
    {
    fprintf(stderr, "Who snatched the locations file?\n");
    exit(1);
    }
  if ((ofp = fopen("rooms.c", "w")) == NULL)
    {
    fprintf(stderr, "Give me write access!\n");
    exit(1);
    }
  fprintf(ofp, "#include \"locdata.h\"\n\nconst LOCDATA room_data[] = {\n");
  while(fgetc(ifp) != '\n')
    ;
  while(fscanf(ifp, "%d %d %d %d %d %d %d\n", 
	       &tmp.r_code,
               &tmp.r_exit[0], &tmp.r_exit[1],
	       &tmp.r_exit[2], &tmp.r_exit[3],
	       &tmp.r_exit[4], &tmp.r_exit[5]) != EOF)
    {
    fscanf(ifp, "%s\n", x);
    for (b = 0, j = 0; (c = x[j]) && (c == '0' || c == '1'); j++)
      b = (b << 1) | (c == '1' ? 1 : 0);
    fprintf(ofp,"{ %d, { %d, %d, %d, %d, %d, %d }, %d,\n",
	    tmp.r_code,
	    tmp.r_exit[0], tmp.r_exit[1],
	    tmp.r_exit[2], tmp.r_exit[3],
	    tmp.r_exit[4], tmp.r_exit[5],
	    b);
    fprintf(ofp, "\"%s\",\n", get_string(ifp));
    tmp.r_long = get_string(ifp);
    fputc('"', ofp);
    for(j = 0; x[j]; j++)
      {
      switch (x[j])
	{
      case '\n' : fputs("\\n\\\n", ofp);
	break;
      case '"' :  fprintf(ofp, "%s", "\\\"");
	break;
      default :   fputc(x[j], ofp);
	}
      }
    fputs("\"},\n", ofp);
    }
  fputs("{0, {0, 0, 0, 0, 0, 0}, 0, \"\", \"\"} };\n", ofp);
  exit(0);
  }
