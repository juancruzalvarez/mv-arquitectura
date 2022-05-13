#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

#define PARSED_LENGTH 9

#ifdef _WIN32
  #define PARSER ".\\parser.exe"
  #define TEMP ".\\parser.tmp"
#else
  #define PARSER "./parser"
  #define TEMP "./parser.tmp"
#endif

void saveline(const char *line, const char *filename)
{
  FILE *fp = fopen(filename, "w");
  fprintf(fp, "%s", line);
  fclose(fp);
}

char **parse_line(const char *line)
{
  FILE *file = NULL;
  char cmd[1024];

  saveline(line, TEMP);
  sprintf(cmd, "%s %s", PARSER, TEMP);
  file = popen(cmd, "r");

  if (file == NULL) return NULL;

  char **parsed = (char **) malloc(sizeof(char *) * PARSED_LENGTH);
  char elem[256];
  int i = 0;

  while (i < PARSED_LENGTH && fgets(elem, 256, file))
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

  for (; i < PARSED_LENGTH; i++)
    parsed[i] = NULL;

  pclose(file);

  return parsed;
}

void free_line(char **parsed)
{
  for (int i = 0; i < PARSED_LENGTH; i++)
    free(parsed[i]);

  free(parsed);
}