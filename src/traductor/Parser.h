#ifndef PARSER_H
#define PARSER_H

enum PARSED{
   LABEL = 0,
   MNEMO,
   OP1,
   OP2,
   COMENT,
   SEG,
   SEG_SIZE,
   CONST,
   CONST_VAL
};
char **parse_line(const char *line);
void free_line(char **parsed);

#endif
