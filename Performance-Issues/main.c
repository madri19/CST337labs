#include <xc.h>
#include <math.h>

// PIC32MZ2048ECG144 or EFG144 based HMZ144 board Configuration Bit Settings
// DEVCFG2
#if defined(__32MZ2048EFG144__)
#pragma config FPLLIDIV = DIV_4         // System PLL Input Divider (4x Divider) for 24MHz clock (Rev D (C1) board) 24MHz/2 = 6MHz
#pragma config UPLLFSEL = FREQ_24MHZ    // USB PLL Input Frequency Selection (USB PLL input is 24 MHz)
#else
#pragma config FPLLIDIV = DIV_2         // System PLL Input Divider (2x Divider) for 12 MHz crystal (Rev B and C boards) 12MHz/2 = 6MHz
#pragma config UPLLEN = OFF             // USB PLL Enable (USB PLL is disabled)
#endif
#pragma config FPLLRNG = RANGE_5_10_MHZ // System PLL Input Range (5-10 MHz Input)
#pragma config FPLLICLK = PLL_POSC      // System PLL Input Clock Selection (POSC is input to the System PLL)
#pragma config FPLLMULT = MUL_112       // System PLL Multiplier (PLL Multiply by 112) 6MHz * 112 = 672MHz
#pragma config FPLLODIV = DIV_8         // System PLL Output Clock Divider (8x Divider) 672MHz / 8 = 84MHz

// DEVCFG1
#pragma config FNOSC = SPLL             // Oscillator Selection Bits (Primary Osc (HS,EC))
#pragma config FSOSCEN = OFF            // Secondary Oscillator Enable (Disable SOSC)
#if defined(__32MZ2048EFG144__)
#pragma config POSCMOD = EC             // Primary Oscillator Configuration EC - External clock osc
#else
#pragma config POSCMOD = HS             // Primary Oscillator Configuration HS - Crystal osc
#endif
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor Selection (Clock Switch Disabled, FSCM Disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (WDT Disabled)
#pragma config FDMTEN = OFF             // Deadman Timer Enable (Deadman Timer is disabled)
#pragma config DMTINTV = WIN_127_128    // Default DMT Count Window Interval (Window/Interval value is 127/128 counter value)
#pragma config DMTCNT = DMT31           // Max Deadman Timer count = 2^31

// DEVCFG0
#pragma config JTAGEN = OFF             // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx2        // ICE/ICD Comm Channel Select (Communicate on PGEC2/PGED2)


#include "fourier.h"
#define NSAMP   256                 //must be a power of 2
#define TPIN    (2 * M_PI / NSAMP)  //generates sinosoidial inputs for the FFT

float inputSineWave[NSAMP];
float realOutput[NSAMP];
float imaginaryOutput[NSAMP];
float mag[NSAMP];
float * imaginaryNULL = NULL;

int main(){
    
    //PBCLK3 Set up needs to divide by 1
    SYSKEY = 0; //Ensure lock
    SYSKEY = 0xAA996655; // Write Key 1
    SYSKEY = 0x556699AA; // Write Key 2
    PB3DIV = _PB3DIV_ON_MASK | 0 & _PB3DIV_PBDIV_MASK; // 0 = div by 1, 1 = div by 2 etc up to 128, so 3 = div by 4
    SYSKEY = 0; // Re lock
    
    //Flash wait states set up
    //PRECON = (1 & _PRECON_PFMWS_MASK);
    
    ////Flash prefetch set up
    PRECON = (1 & _PRECON_PFMWS_MASK) | ((2 << _PRECON_PREFEN_POSITION) & _PRECON_PREFEN_MASK);
    
    //setup the 32 bit timer
    T2CON = 0x0;
    T3CON = 0x0;
    T2CONSET = 0x0008;
    TMR2 = 0x0;
    PR2 = 0xFFFFFFFF;
    
    //declare some variables
    int n = 0;
    int f = 2;
    
    //generate test data
    for(n = 0; n < NSAMP; n++)
    {
        inputSineWave[n] = 200 * sin(f * TPIN * n);
    }
    
    //call fft_foat fuction and loop it twice for step 17
    for(n = 0; n < 2; n++)
    {
        //start the timer
        fft_float(NSAMP, 0, inputSineWave, imaginaryNULL, realOutput, imaginaryOutput);
        T2CONSET = 0x8000;
    }
    //stop it here after the second loop
    T2CONCLR = 0x8000;
    
    //generate magnitude from FFT data
    for(n = 0; n < NSAMP; n++)
    {
        mag[n] = sqrt((realOutput[n] * realOutput[n]) + (imaginaryOutput[n] * imaginaryOutput[n]));
    }
    
    
    return 0;
}
