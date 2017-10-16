// Source code using the ViSi-Genie-RaspPi library

// 4D Systems


#include <stdio.h>

#include <fcntl.h>

#include <stdlib.h>

#include <string.h>

#include <ctype.h>

#include <errno.h>

#include <unistd.h>

#include <sys/types.h>

#include <pthread.h>

#include <math.h>



#include <geniePi.h>  //the ViSi-Genie-RaspPi library




int main()

{

 
  //print some information on the terminal window

  printf("\n\n");

  printf("Visi-Genie-Raspberry-Pi basic demo\n");

  printf("==================================\n");

  printf("Program is running. Press Ctrl + C to close.\n");


  //open the Raspberry Pi's onboard serial port, baud rate is 115200

  //make sure that the display module has the same baud rate

	genieSetup("/dev/ttyUSB0", 115200); 	

	int x,y;

	while (scanf("%d %d", &x, &y) == 2)

	{

		printf("Value is %d and %d\n", x, y);
        genieWriteObj(GENIE_OBJ_COOL_GAUGE, 0x00, x);
        genieWriteObj(GENIE_OBJ_COOL_GAUGE, 0x01, y);		

	}



  return 0;

}
