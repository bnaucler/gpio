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

#define PLEN 128

int usage(int ret) {

	puts("Usage: gpio [r/w/d] [gpio #] ([on/off][in/out])");
	puts("");
	puts("Read example: gpio r 22 (returns status of gpio pin #22)");
	puts("Write example: gpio w 17 on (enables current on gpio pin #17)");
	puts("Direction example: gpio d 4 in (sets pinmode on gpio pin 4 to input)");

	exit(ret);
}

// Turning output pins on/off (including direction validation)
int write(int pin, int value) {

	char vpath[PLEN];
	char dpath[PLEN];
	char direction[4];

	snprintf(dpath, PLEN, "/sys/class/gpio/gpio%d/direction", pin);
	snprintf(vpath, PLEN, "/sys/class/gpio/gpio%d/value", pin);

	FILE* fpv = fopen(vpath, "w");
	FILE* fpd = fopen(dpath, "r");

	if(!fpd) {
		printf("Cannot open file %s for reading direction.\n", dpath);
		return 1;
	}

	if(!fpv) {
		printf("Cannot open file %s for writing.\n", vpath);
		return 1;
	}

	fread(direction, 3, 1, fpd);

	if (strcmp(direction, "out")) {
		printf("GPIO pin %d is not configured for output. Try 'gpio d %d out'.\n",
			pin, pin);
		return 1;
	}

	fprintf(fpv, "%d", value);

	fclose(fpd);
	fclose(fpv);

	return 0;
}

// Output pin direction and value
int read(int pin) {

	int value;

	char vpath[PLEN];
	char dpath[PLEN];

	sprintf(vpath, "/sys/class/gpio/gpio%d/value", pin);
	sprintf(dpath, "/sys/class/gpio/gpio%d/direction", pin);

	FILE* fpd = fopen(dpath, "r");
	FILE* fpv = fopen(vpath, "r");

	if(!fpv) {
		printf("Cannot open file %s for reading.\n", vpath);
		return 1;
	}
	if(!fpd) {
		printf("Cannot open file %s for reading.\n", dpath);
		return 1;
	}

	printf("Direction: ");
	while(fscanf(fpd, "%d", &value)) printf("%c\n", value);

	printf("Value: ");
	while(fscanf(fpv, "%d", &value)) printf("%d\n", value);

	fclose(fpv);
	fclose(fpd);

	return 0;
}

// Set in/out direction per pin
int direction(int pin, int value) {

	char dpath[PLEN];

	sprintf(dpath, "/sys/class/gpio/gpio%d/direction", pin);

	FILE* fpd = fopen(dpath, "w");
	if(!fpd) {
		printf("Cannot open file %s for writing.\n", dpath);
		return 1;
	}

	if(value == 1) fprintf(fpd, "out");
	else if(!value) fprintf(fpd, "in");

	fclose(fpd);
	return 0;
}

int main(int argc, char *argv[]) {

	// Initial syntax check
	if(argc < 2 || argc > 4 ||
	(!strcmp(argv[1], "w") && argc != 4) ||
	(!strcmp(argv[1], "r") && argc != 3) ||
	(!strcmp(argv[1], "d") && argc != 4) ||
	((strcmp(argv[1], "w")) &&
	(strcmp(argv[1], "r")) &&
	(strcmp(argv[1], "d"))))
		usage(1);

	int a = 0, cpin = 0;
	int pnum = (int) strtol(argv[2], NULL, 10);
	int apins[] = {
	2, 3, 4, 7, 8, 9, 10, 11, 14,
	15, 17, 18, 22, 23, 24, 25, 27
	};

	// Validating pin number (adjust 16 for array size)
	do {
		if(pnum == apins[a]) {
			cpin++;
			break;
		}
	} while(apins[++a]);

	// Function selector and secondary syntax check
	if(!cpin) {
		puts("Incorrect pin number.");
		return 1;

	} else if(!strcmp(argv[1], "w") && argc == 4 &&
		(!strcmp(argv[3], "1") || !strcmp(argv[3], "on"))) {
		return write(pnum, 1);

	} else if(!strcmp(argv[1], "w") && argc == 4 &&
		(!strcmp(argv[3], "0") || !strcmp(argv[3], "off"))) {
		return write(pnum, 0);

	} else if(!strcmp(argv[1], "r") && argc == 3) {
		return read(pnum);

	} else if(!strcmp(argv[1], "d") && argc == 4 &&
		(!strcmp(argv[3], "0") || !strcmp(argv[3], "in"))) {
		return direction(pnum, 0);

	} else if(!strcmp(argv[1], "d") && argc == 4 &&
		(!strcmp(argv[3], "1") || !strcmp(argv[3], "out"))) {
		return direction(pnum, 1);
	}

	return usage(1);
}
