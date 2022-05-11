#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "Parser.h"

#ifdef _WIN32
  #define PARSER ".\\parser.exe"
  #define TEMP ".\\parser.tmp"
#else
  #define PARSER "./parser"
  #define TEMP "./parser.tmp"
#endif

void saveline(char *line, char *filename)
{
  FILE *fp = fopen(filename, "w");
  fprintf(fp, "%s", line);
  fclose(fp);
}

char **parseline(char *line)
{
  FILE *file = NULL;
  char cmd[1024];

  saveline(line, TEMP);
  sprintf(cmd, "%s %s", PARSER, TEMP);
  file = popen(cmd, "r");

  if (file == NULL) return NULL;

  char **parsed = (char **) malloc(sizeof(char *) * 5);
  char elem[256];
  int i = 0;

  while (fgets(elem, 256, file))
  {
    size_t length = strlen(elem);

    if (length > 0 && elem[length - 1] == '\n')
      elem[--length] = '\0';

    if (length == 0)
      parsed[i] = NULL;
    else
    {
      parsed[i] = (char *) malloc(sizeof(char) * (length + 1));
      strcpy(parsed[i], elem);
    }

    i++;
  }

  for (; i < 5; i++)
    parsed[i] = NULL;

  pclose(file);

  return parsed;
}

void freeline(char **parsed)
{
  for (int i = 0; i < 5; i++)
    free(parsed[i]);

  free(parsed);
}
