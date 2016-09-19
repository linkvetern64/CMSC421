/**
 * @Author Joshua Standiford (jstand1@umbc.edu)
 * This file accepts a single argument.  The program then attempts
 * to find the matching interrupt number in the /proc/interrupts/ file.
 * If the number or driver doesn't exist nothing is printed.  
 * Otherwise the driver name is printed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

/**
 * function - main():
 * Pre-conditions:  1 integer argument
 * Post-conditions: Driver information according to interrupt number will be printed
 *
 * Description: 
 * This function contains the logic for parsing the driver names of /proc/interrupts.
 * A command line argument is taken in.  A check is done to make sure its a number
 * and that there's exactly one argument.
 *
 * A file reader will tokenize each line and check the first token against the argument.
 * If the number matches, the program will then print out the corresponding driver names.  
 * Otherwise the program will not print out anything.
 */
int main(int argc, char *argv[]){
  int NUM_CPUS = sysconf(_SC_NPROCESSORS_ONLN);
  FILE *file;
  char buffer[255];
  char *token;
 
  
  if(argc == 2){
    int arg = atoi(argv[1]);
    file = fopen("/proc/interrupts", "r");
    
    //done to start reading from after format header of table
    token = fgets(buffer, 255, file); 

    while(fgets(buffer, 255, file) != NULL){
      //First token is analyzed here to see if interrupt number matches
      token = strtok (buffer, " ");

      int i = 0;
      //If the token is matched then line is parsed through for driver information.
      if(arg == strtol(token,NULL,0) && isdigit(* argv[1])){
	while(token != NULL){
	  i++;
	  //3 + NUM_CPUS is because the interrupts table is formatted dynamically based on CPU #
	  if(i > (3 + NUM_CPUS)){
	    printf("%s ", token);
	  }
	  token = strtok (NULL, " ");
	}
	//Breaks after the first occurence of interrupt number
	break; 
      }
    }
    fclose(file);
  } 
}
