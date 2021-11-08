#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// compile with:
// accepts number of chirps as an argument. Default is 1.



static int32_t adc1_accumulator [100000];
//static int32_t adc2_accumulator [8012500];
int main(int argc, char **argv)
{
	int fd;
	void *cfg;
	int16_t *ram;
	uint8_t *rst;
	int i, j;
	FILE *adc1;//,*adc2;
	int page = sysconf(_SC_PAGESIZE);
	int fpga_memory_size = 50*pow(10,6);
	printf("Allocated fpga memory is %d\n",fpga_memory_size);
	uint32_t num_pages = fpga_memory_size/page;
	uint32_t num_bytes = num_pages*page;
	uint32_t delay = 1800; //see formula
	printf("Size of memory mapped for receiver: %zu\n", num_bytes);
	const uint32_t num_samples = 100000;
	printf("Number of samples to be stored: %d\n", num_samples);
	int num_chirps;
	//use Julia pinc calculator
	// uint32_t pinc_min = 5167; 
	// uint32_t pinc_max = 5570;
	if (argc == 2)
		num_chirps = atoi(argv[1]);
	else num_chirps = 1;
	
	
	if((fd = open("/dev/mem", O_RDWR)) < 0)
	{
		perror("open");
		return 1;
	}
	
	cfg = mmap(NULL, page, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40000000);
	//configure ram for 90 000 pages (can go up to about 92 285)
	ram = mmap(NULL, num_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x1e000000);
	
	printf("cfg address: %p\n", cfg);
	printf("ram address: %p\n", ram);

	rst = (uint8_t *)(cfg + 0);

	*(uint32_t *)(cfg+4) = num_samples;	// set number of samples to store. address 32-63
	printf("Number of samples sent to packetiser\n");
		
	*(uint32_t *)(cfg+8) = delay;//set delay -> stretches the chirp
	printf("Pinc delay set to %d (determines chirp duration)\n",delay);
	
	/* *(uint32_t *)(cfg+12) = pinc_min;
	printf("Pinc min = %d\n",pinc_min);
	
	*(uint32_t *)(cfg+16) = pinc_max;
	printf("Pinc max = %d\n",pinc_max); */

	// intialise pinc_FSM
	// populates pinc array and sets delay using assigned values (see above)
	*rst &= ~8;
	*rst |= 8;
	printf("initialised phase incrementor FSM.\n");
	//clear init pin (good practice) and ensure gen set to 0
	*rst &= ~8;
	*rst &= ~4;
	
	printf("Number of chirps to be accumulated: %d\n", num_chirps);
	
	//repeat experiment for accumulating data. improves SNR
	for (j = 0;j<num_chirps;j++){
		printf("Iteration %d\n",j);
		//DDS requires rst for min 2 clck cycles
		*rst &= ~16;
		usleep(1);//no need for precise sleep
		*rst |= 16;
		usleep(20); //give DDS more time to warm up
		printf("DDS reset \n");
		*rst &= ~7; //rst writer = rst packetiser = pinc_gen = 0;
		*rst |= 7;
		*rst &= ~4;//turn off gen for chirp
		printf("Reset writer, reset packetiser and started chirp\n");
		printf("Accumulating...\n");
		for(i = 0; i < num_samples; ++i)
		{
			adc1_accumulator[i] += ram[2 * i];
			//adc2_accumulator[i] += ram[2 * i + 1];
		}
	
	//------ACCUMULATION SEPARATE FILE TEST-------------------------
	/* if (j == 0)adc1 = fopen("1.txt","w+");
	else adc1 = fopen("2.txt","w+");
	printf("Accumulation complete. Storing data\n");
	
	for(i = 0; i < num_samples; ++i)
	{
		//divide by num samples!!
		//--------------------------------
		fprintf(adc1, "%d\n",ram[2 * i]);
	} */
	//------------------------------------------------------
	}
	
	
	adc1 = fopen("adc1.txt","w+");
	printf("Accumulation complete. Storing data\n");
	for(i = 0; i < num_samples; ++i)
	{
		//divide by num samples!!
		//--------------------------------
		fprintf(adc1, "%d\n",adc1_accumulator[i]);
	}
		
		
	printf("Data recorded. closing files...\n");
	fclose(adc1);
	//fclose(adc2);
	munmap(cfg, page);
	munmap(ram, num_bytes);
	return 0;
}
