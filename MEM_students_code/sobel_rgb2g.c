#include <stdint.h>

#include "sobel_rgb2g.h"

void rgb2grayscale (int width, int height, uint8_t const *rgb_source, 
                    uint8_t *gs_dest )
{
    int loop, j;
    uint8_t temp;
    unsigned int greyscale;
    int max_loop = width * height * 3;

    for (loop = 0, j = 0; loop < max_loop; loop +=3, j++) {
        temp = rgb_source[loop];
        greyscale= (temp*30)/100;
    
        temp = (rgb_source[loop + 1]);
        greyscale += (temp*59)/100;
    
        temp = (rgb_source[loop + 2]);
        greyscale += (temp*11)/100;
          
        gs_dest[j] = (uint8_t)greyscale;   
    }      
}