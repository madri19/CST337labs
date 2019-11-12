// Lab 1 Introduction to C on the PIC32MZ
// Juan Madrigal

// PIC32MZ2048ECG144 or EFG144 based HMZ144 board Configuration Bit Settings
// DEVCFG2
#if defined(__32MZ2048EFG144__)
#pragma config FPLLIDIV = DIV_4 // System PLL Input Divider (4x Divider) for 24MHz clock (Rev D (C1) board) 24MHz/2 = 6MHz
#pragma config UPLLFSEL = FREQ_24MHZ // USB PLL Input Frequency Selection (USB PLL input is 24 MHz)
#else

#pragma config FPLLIDIV = DIV_2 // System PLL Input Divider (2x Divider) for 12 MHz crystal (Rev B and C boards) 12MHz/2 = 6MHz
#pragma config UPLLEN = OFF // USB PLL Enable (USB PLL is disabled)
#endif

#pragma config FPLLRNG = RANGE_5_10_MHZ // System PLL Input Range (5-10 MHz Input)
#pragma config FPLLICLK = PLL_POSC // System PLL Input Clock Selection (POSC is input to the System PLL)
#pragma config FPLLMULT = MUL_112 // System PLL Multiplier (PLL Multiply by 112) 6MHz * 112 = 672MHz
#pragma config FPLLODIV = DIV_8 // System PLL Output Clock Divider (8x Divider) 672MHz / 8 = 84MHz

// DEVCFG1
#pragma config FNOSC = SPLL // Oscillator Selection Bits (Primary Osc (HS,EC))
#pragma config FSOSCEN = OFF // Secondary Oscillator Enable (Disable SOSC)
#if defined(__32MZ2048EFG144__)
#pragma config POSCMOD = EC // Primary Oscillator Configuration EC - External clock osc
#else
#pragma config POSCMOD = HS // Primary Oscillator Configuration HS - Crystal osc
#endif
#pragma config FCKSM = CSDCMD // Clock Switching and Monitor Selection (Clock Switch Disabled, FSCM Disabled)
#pragma config FWDTEN = OFF // Watchdog Timer Enable (WDT Disabled)
#pragma config FDMTEN = OFF // Deadman Timer Enable (Deadman Timer is disabled)
#pragma config DMTINTV = WIN_127_128 // Default DMT Count Window Interval (Window/Interval value is 127/128 counter value)
#pragma config DMTCNT = DMT31 // Max Deadman Timer count = 2^31

// DEVCFG0
#pragma config JTAGEN = OFF // JTAG Enable (JTAG Disabled)
#pragma config ICESEL = ICS_PGx2 // ICE/ICD Comm Channel Select (Communicate on PGEC2/PGED2)

#include <xc.h>

int main(){

  SYSKEY = 0; //Ensure lock
  SYSKEY = 0xAA996655; // Write Key 1
  SYSKEY = 0x556699AA; // Write Key 2
  PB3DIV = _PB3DIV_ON_MASK | 3 & _PB3DIV_PBDIV_MASK; // 0 = div by 1, 1 = div by 2 etc up to 128, so 3 = div by 4
  SYSKEY = 0; // Re lock

  TRISH = 0x0000;
  T2CON = 0x0;
  TMR2 = 0;
  PR2 = 41016; // = 41016
  T2CON = 0x8070;

while(1){

  if(IFS0 & _IFS0_T2IF_MASK){
    IFS0CLR = _IFS0_T2IF_MASK;
    LATHINV = _LATH_LATH2_MASK;
    }
  }

return 0;
}
