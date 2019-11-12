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

#include <xc.h>
#include <sys/attribs.h>

int k = 0;
int j = 0;

int main(){
    
    //Initial Set up
    //****************************************************************************************************************
    
    //Globally disable interrupts
    asm volatile("di");
    
    //PBCLK3 Set up needs to divide by 1 so we use 84MHz
    SYSKEY = 0;                                         // Ensure lock
    SYSKEY = 0xAA996655;                                // Write Key 1
    SYSKEY = 0x556699AA;                                // Write Key 2
    PB3DIV = _PB3DIV_ON_MASK | 0 & _PB3DIV_PBDIV_MASK;  // 0 = div by 1, 1 = div by 2 etc up to 128, so 3 = div by 4
    SYSKEY = 0;                                         // Re lock
    
    //Set flash wait states to 1 and enable flash prefetch set up
    PRECON = (1 & _PRECON_PFMWS_MASK) | ((2 << _PRECON_PREFEN_POSITION) & _PRECON_PREFEN_MASK);
    
    //Setup timer 2
    T2CON = 0x0000;
    TMR2 = 0;
    PR2 = 511;
    T2CONCLR = 0x8000;                                  // Stop the timer
    IFS0CLR = _IFS0_T2IF_MASK;                          // Clear the timer2 interrupt flag
    
    //Setup timer 3
    T3CON = 0x0000;
    TMR3 = 0;
    PR3 = 999;
    T3CONCLR = 0x8000;                                  // Stop the timer
    IFS0CLR = _IFS0_T3IF_MASK;                          // Clear the timer3 interrupt flag
    
    //Set up multi vector interrupt system mode
    //INTCONSET = _INTCON_MVEC_MASK;                      // Set MVEC bit
    
    // Set up priority 4 for timer 2
    IPC2CLR = _IPC2_T2IP_MASK | _IPC2_T2IS_MASK;        // clear T2 priority and sub priority
    IPC2SET = (5 << _IPC2_T2IP_POSITION) & _IPC2_T2IP_MASK; // Set Priority 3, Sub priority 0
    
    // Set up priority 2 for timer 3
    IPC3CLR = _IPC3_T3IP_MASK | _IPC3_T3IS_MASK;        // clear T3 priority and sub priority
    IPC3SET = (3 << _IPC3_T3IP_POSITION) & _IPC3_T3IP_MASK; // Set Priority 4, Sub priority 0
    
    //PRISS = 0x70000000;                                 //Priority 7 will go to shadow set 7

    // Enable timer 2 interrupts
    IEC0SET = _IEC0_T2IE_MASK;                          // enable T2 INTs
    
    // Enable timer 3 interrupts
    IEC0SET = _IEC0_T3IE_MASK;                          // enable T3 INTs
    
    // Globally enable interrupts
    asm volatile("ei");
    
    //****************************************************************************************************************
    
    //Turn on both timers
    T2CON = 0x8000;
    T3CON = 0x8000;
    IFS0SET = _IFS0_T3IF_MASK; //artificially trigger T3 interrupt
    
    //Run loop so it runs from cache the second time
    int i = 0;
    for(i = 0; i < 2; i++){
        TMR2 = 0;
        TMR3 = 5;
    }
    
    while(1);
    
    return 0;
}

void __ISR_AT_VECTOR(0, RIPL) MyMergedHandler(void)
{      
    if(IFS0 & _IFS0_T2IF_MASK){
        k++;
        IFS0CLR = _IFS0_T2IF_MASK;
    }
    if(IFS0 & _IFS0_T3IF_MASK){
        while(TMR3 < 40);
        j++;
        IFS0CLR = _IFS0_T3IF_MASK; 
    }
}
