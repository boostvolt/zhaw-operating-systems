/* ------------------------------------------------------------
                    ____     ________
                   /  _/__  / __/ __/    
                  _/ // _ \/ _/_\ \        
                 /___/_//_/___/___/            

    Institute of Embedded Systems
    Zurich University of Applied Sciences
    8401 Winterthur, Switzerland


    Project:  BSY Labs
    File:     $RCSfile$

    Purpose:  example answer for the BSY lab MEM
              
    Remarks:  main routine 

    Author:   donn

    Date:     K20 2021
    Version:  0.9 FS21

    $Id$

   ------------------------------------------------------------ */ 
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

#include <sys/sysinfo.h>
#include <sys/syscall.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

// image library stuff
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "sobel_rgb2g.h"

extern int errno;

//***************************************************************************
// Function: main(), parameter: none
//***************************************************************************

int main(void) {

    int result;
    int section = 1;
    
    (void) printf( "Hello MEM Lab\n" );
 
 
// ---------- section 
    
    (void) printf( "------------ Part %i:  Simple program check memory of process\n", section++ );

    // ToDO 1: insert function to get and printout the process pid. then put program in an endless loop 
    (void) printf("----- the PID of this process is %i\n", getpid());

    // Removed endless loop for ToDo 2
    // (void) printf( "\nPress enter to continue\n" );
    // (void) getchar();
 
// ---------- section  
        
    (void) printf( "------------ Part %i:  whats the page size\n", section++ );

    // ToDo 2: insert function to get and read the page size  
    int pagesize = getpagesize();
    (void) printf("Page size is %d bytes\n", pagesize);

    (void) printf( "Press return to continue\n" );
    (void) getchar();
 
// ---------- section 
 
    (void) printf( "------------ Part %i:  generate some memory area\n", section++ );

    // ToDo 3: insert code to reserve memory  
    int num_pages = 4;
    void *memory_ptr = malloc(num_pages * pagesize);
    
    if (memory_ptr == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    (void) printf("Reserved %d bytes (%d pages) of memory at address %p\n", 
                  num_pages * pagesize, num_pages, memory_ptr);
  
    
    (void) printf( "\nPress return to continue\n" );
    (void) getchar();

// ---------- section 
  
    (void) printf( "------------ Part %i: Are these pages in memory?\n", section++ );  

    // ToDO 4: reserve memory with aligned_alloc this is necessary for the mincore function to function properly
    


    // ToDo 5: write something into the reserved buffer      

 
    (void) printf( "Press return to continue\n" );
    (void) getchar();

// ---------- section 
 
    (void) printf( "------------ Part %i: Lets limit the available memory\n", section++ );  
 
    
    // prepare variables and load a large file
    int width=0, height=0, bpp=0;
    uint8_t *rgb_image = stbi_load("Hopper.png", &width, &height, &bpp, 3);

    // in case of issues
    if (rgb_image == NULL) {
        perror("File read failed\n"); 
        exit(EXIT_FAILURE);
    }
    
	// reserve memory for the greyscale image
	// the rgb png comes in 8-bit format
	uint8_t *gry_image = malloc ( width * height * sizeof(uint8_t) );    
    
    // lets exercise
    rgb2grayscale ( width, height, rgb_image, gry_image );    
    
    (void) printf( "Check the number of page faults\n" );
    (void) printf( "Press return to continue\n" );
    (void) getchar();

    
    (void) printf( "Carry out the instructions in the lab guide to limit the available memory and repeat \n" );

    
    (void) printf( "Bye MEM Lab\n" );
    
    exit( 0 );
 

}


//*************************************************************************** 