/* 

	Raspberry Pi GPIO control
	Björn W Nauclér 2014
	mail@bnaucler.se

	NB: Pins need to be exported with /sys/class/gpio/export
	    before use

	Todo:
	* Allow "gpio" in command line reference to pin
	* Function for 'cannot open file'
	* Add export and unexport of pins (need reference)
	* Add pull-up and pull-down functions (need reference)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[])
{
	// Initial syntax check
	if (argc < 2 || argc > 4 ||
	(!strcmp(argv[1], "w") && argc != 4) || 
	(!strcmp(argv[1], "r") && argc != 3) ||
	(!strcmp(argv[1], "d") && argc != 4) ||
	((strcmp(argv[1], "w")) && 
	(strcmp(argv[1], "r")) && 
	(strcmp(argv[1], "d"))) )
	{ usage(); }

	int a;
	int correctPin = 0;
	int pinNumber = atoi(argv[2]);
	int availablePins[] = {
	2, 3, 4, 7, 8, 9, 10, 11, 14, 
	15, 17, 18, 22, 23, 24, 25, 27
	};

	// Validating pin number (adjust 16 for array size)
	for (a=0;a<=16;a++)
	{ if (pinNumber == availablePins[a]) { correctPin = 1; } }

	// Function selector and secondary syntax check
	if (correctPin != 1)
	{ puts("Incorrect pin number."); exit(1); }
	else if (!strcmp(argv[1], "w") && argc == 4 &&
	(!strcmp(argv[3], "1") || !strcmp(argv[3], "on"))) 
	{ write(pinNumber, 1); }
	else if (!strcmp(argv[1], "w") && argc == 4 &&
	(!strcmp(argv[3], "0") || !strcmp(argv[3], "off")))
	{ write(pinNumber, 0); } 
	else if (!strcmp(argv[1], "r") && argc == 3)
	{ read(pinNumber); }
	else if (!strcmp(argv[1], "d") && argc == 4 &&
	(!strcmp(argv[3], "0") || !strcmp(argv[3], "in")))
	{ direction(pinNumber, 0); }
	else if (!strcmp(argv[1], "d") && argc == 4 &&
	(!strcmp(argv[3], "1") || !strcmp(argv[3], "out")))
	{ direction(pinNumber, 1); }
	else { usage(); }
}

int usage() {
	puts("Usage: gpio [r/w/d] [gpio #] ([on/off][in/out])");
	puts("");
	puts("Read example: gpio r 22 (returns status of gpio pin #22)");
	puts("Write example: gpio w 17 on (enables current on gpio pin #17)");
	puts("Direction example: gpio d 4 in (sets pinmode on gpio pin 4 to input)");
	exit(0);
}

// Turning output pins on/off (including direction validation)
int write (int pin, int value) {

	char vpath[60];
	char dpath[60];
	char direction[1];

	sprintf(dpath, "/sys/class/gpio/gpio%d/direction", pin);
	sprintf(vpath, "/sys/class/gpio/gpio%d/value", pin);

	FILE* fpv = fopen(vpath, "w");
	FILE* fpd = fopen(dpath, "r");

	if (fpd == NULL) {
		printf("Cannot open file %s for reading direction.", dpath);
		puts("");
		exit(1);
	}

	if (fpv == NULL) {
		printf("Cannot open file %s for writing.", vpath);
		puts("");
		exit(1);
	}

	fseek(fpd, SEEK_SET, 0);
	fread(direction, 3, 1, fpd);

	if (strcmp(direction, "out")) {
		printf("GPIO pin %d is not configured for output. Try 'gpio d %d out'.", pin, pin);
		puts("");
		exit(1);
	}

	fprintf(fpv, "%d", value);
	fclose(fpd);
	fclose(fpv);
	exit(0);
}

// Output pin direction and value
int read (int pin) {

	int value;

	char vpath[60];
	char dpath[60];

	sprintf(vpath, "/sys/class/gpio/gpio%d/value", pin);
	sprintf(dpath, "/sys/class/gpio/gpio%d/direction", pin);

	FILE* fpd = fopen(dpath, "r");
	FILE* fpv = fopen(vpath, "r");
	if (fpv == NULL) {
		printf("Cannot open file %s for reading.", vpath);
		puts("");
		exit(1);
	}
	if (fpd == NULL) {
		printf("Cannot open file %s for reading.", dpath);
		puts("");
		exit(1);
	}

	printf("Direction: ");
        while(fscanf(fpd, "%c", &value) == 1)
                printf("%c", value);

	printf("Value: ");
	while(fscanf(fpv, "%d", &value) == 1)
		printf("%d", value);

	puts("");
	fclose(fpv);
	fclose(fpd);
	exit(0);
}

// Set in/out direction per pin
int direction (pin, value) {
	
	char dpath[60];

	sprintf(dpath, "/sys/class/gpio/gpio%d/direction", pin);

        FILE* fpd = fopen(dpath, "w");
        if (fpd == NULL) {
                printf("Cannot open file %s for writing.", dpath);
                puts("");
                exit(1);
        }

	if (value == 1) { fprintf(fpd, "out"); }
	if (value == 0) { fprintf(fpd, "in"); }

	fclose(fpd);
	exit(0);
}
