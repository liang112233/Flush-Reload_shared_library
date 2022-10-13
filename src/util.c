
#include "util.h"

/*
 * Loads from virtual address addr and measure the access time
 */
CYCLES measure_one_block_access_time(ADDR_PTR addr)
{
    CYCLES cycles;

    asm volatile("mov %1, %%r8\n\t"
            "lfence\n\t"
            "rdtsc\n\t"
            "mov %%eax, %%edi\n\t"
            "mov (%%r8), %%r8\n\t"
            "lfence\n\t"
            "rdtsc\n\t"
            "sub %%edi, %%eax\n\t"
    : "=a"(cycles) /*output*/
    : "r"(addr)
    : "r8", "edi");

    return cycles;
}

/* 
 * Returns Time Stamp Counter 
 */
extern inline __attribute__((always_inline))
CYCLES rdtscp(void) {
	CYCLES cycles;
	asm volatile ("rdtscp"
	: /* outputs */ "=a" (cycles));

	return cycles;
}

/* 
 * Gets the value Time Stamp Counter 
 */
inline CYCLES get_time() {
    return rdtscp();
}

/* Synchronizes at the overflow of a counter
 *
 * Counter is created by masking the lower bits of the Time Stamp Counter
 * Sync done by spinning until the counter is less than CHANNEL_SYNC_JITTER
 */
extern inline __attribute__((always_inline))
CYCLES cc_sync() {
    while((get_time() & CHANNEL_SYNC_TIMEMASK) > CHANNEL_SYNC_JITTER) {}
    return get_time();
}


/*
 * Flushes the cache block accessed by a virtual address out of the cache
 */
extern inline __attribute__((always_inline))
void clflush(ADDR_PTR addr)
{
    asm volatile ("clflush (%0)"::"r"(addr));
}

/*
 * Convert a given ASCII string to a binary string.
 * From:
 * https://stackoverflow.com/questions/41384262/convert-string-to-binary-in-c
 */
char *string_to_binary(char *s)
{
    if (s == NULL) return 0; /* no input string */

    size_t len = strlen(s) - 1;

    // Each char is one byte (8 bits) and + 1 at the end for null terminator
    char *binary = malloc(len * 8 + 1);
    binary[0] = '\0';

    for (size_t i = 0; i < len; ++i) {
        char ch = s[i];
        for (int j = 7; j >= 0; --j) {
            if (ch & (1 << j)) {
                strcat(binary, "1");
            } else {
                strcat(binary, "0");
            }
        }
    }

    return binary;
}

/*
 * Convert 8 bit data stream into character and return
 */
char *conv_char(char *data, int size, char *msg)
{
    for (int i = 0; i < size; i++) {
        char tmp[8];
        int k = 0;

        for (int j = i * 8; j < ((i + 1) * 8); j++) {
            tmp[k++] = data[j];
        }

        char tm = strtol(tmp, 0, 2);
        msg[i] = tm;
    }

    msg[size] = '\0';
    return msg;
}

void init_config(struct config *config)
{
// open a shared library LIBM_SO with dlopen
  void *handle;
  double (*cosine)(double);
  char *error;
  handle = dlopen(LIBM_SO, RTLD_LAZY);
  if(!handle){
      fprintf(stderr, "%s\n",dlerror());
      exit(EXIT_FAILURE);
  }
  dlerror();
  //obtain the virtual address of a function symble in a shared object or executable
  cosine = (double (*)) dlsym(handle, "cos");
  error = dlerror();
  if (error != NULL){
  fprintf(stderr, "%s\n", error);
  exit(EXIT_FAILURE);
  }

	// Initialize default config parameters
	config->interval = CHANNEL_DEFAULT_INTERVAL;


	//	config->addr = (ADDR_PTR) cosine;
    config->addr = (ADDR_PTR) cosine;
}

