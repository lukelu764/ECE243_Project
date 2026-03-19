// CHANGE GAME STATE AFTER CERTAIN AMOUNTS OF TIME
// this version counts for 5 seconds
// #include "address_map_niosv.h"
// last saved 3/18 9:00

#define clock_rate 100000000
#define quarter_clock clock_rate / 4

static void handler(void) __attribute__((interrupt("machine")));
void set_mtimer(void);
void set_itimer(void);
void set_KEY(void);
void SWI_ISR(void);
void mtimer_ISR(void);
void itimer_ISR(void);
void KEY_ISR(void);

/* Global variables are written by interrupt service routines; we declare
16 * as volatile to avoid the compiler caching their values in registers */
volatile int counter = 0;  // binary counter to be displayed
volatile int digit = 0;    // decimal digit to be displayed
volatile int KEY_dir = 1;  // digit counter direction
volatile int counter2 = 0; 
volatile int game_state = 0; // game running = 0, game pause = 1

// 7-segment codes for digits 0, 1, ..., 9
char bit_codes[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67};

int main(void) {
  /* Declare volatile pointers to I/O registers (volatile means that the
 44 * accesses will always go to the memory (I/O) address */
  volatile int* mtime_ptr = (int*)0xFF202100;
  volatile int* LEDR_ptr = (int*)0xFF200000;
  volatile int* HEX3_HEX0_ptr = (int*)0xFF200020;

  set_mtimer();
  set_itimer();
  set_KEY();

  int mstatus_value, mtvec_value, mie_value;
  mstatus_value = 0b1000;  // interrupt bit mask
  // disable interrupts
  __asm__ volatile("csrc mstatus, %0" ::"r"(mstatus_value));
  mtvec_value = (int)&handler;  // set trap address
  __asm__ volatile("csrw mtvec, %0" ::"r"(mtvec_value));
  // disable all interrupts that are currently enabled
  __asm__ volatile("csrr %0, mie" : "=r"(mie_value));
  __asm__ volatile("csrc mie, %0" ::"r"(mie_value));
  mie_value = 0x50088;  // KEY, itimer, mtimer, SW interrupts
  // set interrupt enables
  __asm__ volatile("csrs mie, %0" ::"r"(mie_value));
  // enable Nios V interrupts
  __asm__ volatile("csrs mstatus, %0" ::"r"(mstatus_value));

  //*(mtime_ptr + 4) = 1;  // cause a software interrupt

  *HEX3_HEX0_ptr = 0x3f;  // show 0 on HEX0

  while (1) {
    *HEX3_HEX0_ptr = bit_codes[digit];  // display in decimal

    // after a certain count turn all the LEDs off
    //
    if (counter2 > 5) {
      game_state = 1;
    }
	  
	*LEDR_ptr = game_state; 
  }
}

/*******************************************************************
79 * Trap handler: determine what caused the interrupt and calls the
80 * appropriate subroutine.
81 ******************************************************************/
void handler(void) {
  int mcause_value;
  __asm__ volatile("csrr %0, mcause" : "=r"(mcause_value));
  if (mcause_value == 0x80000003)  // software interrupt
    SWI_ISR();
  else if (mcause_value == 0x80000007)  // machine timer
    mtimer_ISR();
  else if (mcause_value == 0x80000010)  // interval timer
    itimer_ISR();
  else if (mcause_value == 0x80000012)  // KEY port
    KEY_ISR();
  // else, ignore the trap
}

// Software interrupt service routine
void SWI_ISR(void) {
  volatile int* mtime_ptr = (int*)0xFF202100;
  counter = 0b1111111100;  // set global variable
  *(mtime_ptr + 4) = 0;    // clear interrupt
}

// Nios V machine timer interrupt service routine
typedef long long int64;

void mtimer_ISR(void) {
  volatile unsigned int* mtime_ptr = (unsigned int*)0xFF202100;
  int64 mtimecmp64;

  mtimecmp64 = *(mtime_ptr + 3);  // read high word of 64-bit register

  mtimecmp64 = (mtimecmp64 << 32) | *(mtime_ptr + 2);  // read low word

  mtimecmp64 = mtimecmp64 + (int64)quarter_clock;  // adjus timeout

  *(mtime_ptr + 2) = (unsigned int)mtimecmp64;  // store low word

  *(mtime_ptr + 3) = (unsigned int)(mtimecmp64 >> 32);  // store high word

  counter = counter + 1;
}

// FPGA interval timer interrupt service routine
void itimer_ISR(void) {
  int new_digit;
  volatile int* timer_ptr = (int*)0xFF202000;
  *timer_ptr = 0;               // clear the interrupt
  new_digit = digit + KEY_dir;  // inc/dec the digit
  if (new_digit < 10 && new_digit > -1) digit = new_digit;  // decimal (0 to 9)

  counter2 = counter2 + 1; 
}

// KEY port interrupt service routine
void KEY_ISR(void) {
  int pressed;
  volatile int* KEY_ptr = (int*)0xFF200050;
  pressed = *(KEY_ptr + 3);  // read EdgeCapture
  *(KEY_ptr + 3) = pressed;  // clear EdgeCapture register
  KEY_dir = -KEY_dir;        // reverse counting direction
}

// Configure the Nios V machine timer
void set_mtimer(void) {
  volatile int* mtime_ptr = (int*)0xFF202100;
  unsigned int mtime_h, mtime_l, carry, mtimecmp_l;
  do {
    mtime_h = *(mtime_ptr + 1);  // read mtime high word
    mtime_l = *(mtime_ptr);      // read mtime low word
  } while (*(mtime_ptr + 1) != mtime_h);
  mtimecmp_l = mtime_l + quarter_clock;  // add to current time
  carry = mtimecmp_l < mtime_l ? 1 : 0;  // check for carry-out
  *(mtime_ptr + 2) = mtimecmp_l;         // set mtimecmp low word
  *(mtime_ptr + 3) = mtime_h + carry;    // set mtimecmp high word
}

// Configure the FPGA interval timer
void set_itimer(void) {
  volatile int* timer_ptr = (int*)0xFF202000;
  // set the interval timer period
  int load_val = clock_rate;
  *(timer_ptr + 0x2) = (load_val & 0xFFFF);
  *(timer_ptr + 0x3) = (load_val >> 16) & 0xFFFF;

  // start interval timer, enable its interrupts
  *(timer_ptr + 1) = 0x7;  // STOP = 1, START = 1, CONT = 1, ITO = 1
}

// Configure the KEY port
void set_KEY(void) {
  volatile int* KEY_ptr = (int*)0xFF200050;
  *(KEY_ptr + 3) = 0xF;  // clear EdgeCapture register
  *(KEY_ptr + 2) = 0xF;  // enable interrupts for all KEYs
}
