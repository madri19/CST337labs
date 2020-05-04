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
#include <stdint.h>
#include <sys/attribs.h>

void setup();
int readStatusCommand();
void writeEnableCommand();
void readByte(int* byteRead, int nbytes, uint16_t address);
void writeByte(int nbytes, int* newByte, uint16_t address);


int main(){
    
    int status;
    int byteRead[5];
    int newByte[5] = {0, 1, 2, 3, 4};
    int nbytes = 5;
    
    setup();
    //read the 0's status
    status = readStatusCommand();
    // write enable
    writeEnableCommand();
    //read the write status and confirm its value is 2
    status = readStatusCommand();
    
    writeByte(nbytes, newByte, 0x8888);
    readByte(byteRead, nbytes, 0x8888);
    
    newByte[4] = 0;
    newByte[3] = 1;
    newByte[2] = 2;
    newByte[1] = 3;
    newByte[0] = 4;
    
    
    
    writeByte(nbytes, newByte, 0xBEEF);
    readByte(byteRead, nbytes, 0xBEEF);
    
    return 0;
}

void setup(){
    
    SYSKEY = 0; //Ensure lock
    SYSKEY = 0xAA996655; // Write Key 1
    SYSKEY = 0x556699AA; // Write Key 2
    PB2DIV = _PB2DIV_ON_MASK | 0 & _PB2DIV_PBDIV_MASK; // 0 = div by 1, 1 = div by 2 etc up to 128, 41 = divide by 42
    SYSKEY = 0; // Re lock
    
    //Flash wait states set up - set waitstates to 1
    PRECON = (1 & _PRECON_PFMWS_MASK);
    
    //Flash prefetch set up 
    PRECON = (1 & _PRECON_PFMWS_MASK) | ((2 << _PRECON_PREFEN_POSITION) & _PRECON_PREFEN_MASK);
    
    //clear the analog function of pin RPB3
    ANSELBbits.ANSB3 = 0;
    //SDI4 connection to RPB3 for input
    SDI4R = 0x08;
    // SDO4 connected to RPF2 for output
    RPF2R = 0x08;
    //disable CS feature(we will do CS in software not hardware)
    SPI4CONbits.MSSEN = 0;
    //enable latch, so when CS line is first driven, it is driven high which leaves 25LC256 unselected
    LATFbits.LATF8 = 1;
    // clear bit 8 in TRISF to control CS(RF8) using software, make output
    TRISFbits.TRISF8 = 0;
    //SPI4CON initializations for compatibility with the 25LC256
    SPI4CONbits.CKE = 0;
    SPI4CONbits.CKP = 1;
    SPI4CONbits.SMP = 0;
    
    // BRG is 20 so we can get a clock of 2 Mega Hz
    SPI4BRG = 20;
    
    // Enable master SPI
    SPI4CONbits.MSTEN = 1;
    // Enable SPI4 module
    SPI4CONbits.ON = 1;
}

int readStatusCommand(){
    
    // 1) assert CS
    PORTFbits.RF8 = 0;
    // 2) write a "read status" command to SPI4BUF
    SPI4BUF = 0b00000101;
    // 3) Wait for TBE(transmitter buffer empty)
    while(SPI4STATbits.SPITBE == 0);
    // 4) Write a dummy data byte to SPI4BUF
    //(we need to write a byte to get the SPI to clock the returned status byte in)
    SPI4BUF = 'a';
    // 5) Wait for RBF(recieve buffer full) which will be set after the read status
    // command is fully shifted out
    while(SPI4STATbits.SPIRBF != 1);
    // 6) Read SPI4BUF and discard the dummy data that was clocked in while the
    // read status command was sent out
    SPI4BUF;
    // 7) Wait for RBF which will be set after the dummy data byte(sent at step 3) is clocked out
    while(SPI4STATbits.SPIRBF != 1);
    // 8) Read the status byte which was clocked in from the 25LC256 while the dummy 
    // data byte(sent at step 3) was clocked out.
    int byte = SPI4BUF;
    // 9) Negate CS
    PORTFbits.RF8 = 1;
    
    return byte;
}

void writeEnableCommand(){
    
    PORTFbits.RF8 = 0;
    // write a "write enable" command to SPI4BUF
    SPI4BUF = 0b00000110;
    while(SPI4STATbits.SPIRBF != 1);
    //assignment so it takes more cycles
    int garbage = SPI4BUF;
    PORTFbits.RF8 = 1;
}

void readByte(int* byteRead, int nbytes, uint16_t address){
    
    uint8_t bytes[2];
    int i = 0;
    //split the 16 into 2 8-bits
    bytes[0] = address >> 8;
    bytes[1] = address & 0x00FF;
    
    //make sure there isn't a "write in progress"
    while(readStatusCommand() & 0b00000001);
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    //assert CS
    PORTFbits.RF8 = 0;
    //write "read data command" 
    SPI4BUF = 0b00000011;
    //wait for TBE
    while(SPI4STATbits.SPITBE == 0);
    //write most sig address to SPI4BUF
    SPI4BUF = bytes[0];
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);
    // read the dummy
    int garbage = SPI4BUF;
    // write least sig address to SPI4BUF
    SPI4BUF = bytes[1];
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);
    // read the dummy
    garbage = SPI4BUF;
    // write the dummy
    SPI4BUF = garbage;
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);
    // read the dummy
    garbage = SPI4BUF;
    // write the dummy
    SPI4BUF = garbage;
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);
    // read the data********************************************************** READ Data 1
    if(nbytes == 1){
        //only process one byte
        byteRead[0] = SPI4BUF;
    }
    else{
        for(i = 0; i < (nbytes-2); i++){
            // read the byte
            byteRead[i] = SPI4BUF;
            // write the dummy
            SPI4BUF = garbage;
            // wait for RBF
            while(SPI4STATbits.SPIRBF != 1);
        }
        // read the byte
        byteRead[nbytes-2] = SPI4BUF;
        // wait for RBF
        while(SPI4STATbits.SPIRBF != 1);
        // read the byte
        byteRead[nbytes-1] = SPI4BUF;
    }
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    //negate CS
    PORTFbits.RF8 = 1;
}

void writeByte(int nbytes, int* newByte, uint16_t address){
    
    int i = 0;
    
    //make sure there isn't a "write in progress"
    while(readStatusCommand() & 0b00000001);
    
    writeEnableCommand();
    
    uint8_t bytes[2];
    //split the 16 into 2 8-bits
    bytes[0] = address >> 8;
    bytes[1] = address & 0x00FF;
    
    //assert CS
    PORTFbits.RF8 = 0;
    //write "write data command" 
    SPI4BUF = 0b00000010;
    //wait for TBE
    while(SPI4STATbits.SPITBE == 0);
    //write most sig address to SPI4BUF
    SPI4BUF = bytes[0];
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);
    // read the dummy
    int garbage = SPI4BUF;
    // write least sig address to SPI4BUF
    SPI4BUF = bytes[1];
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);

    // write your new data**********Write Data 1
    if(nbytes == 1){
        // read the dummy
        garbage = SPI4BUF;
        //only process one byte
        SPI4BUF = newByte[0];
        // wait for RBF
        while(SPI4STATbits.SPIRBF != 1);
    }
    else{
        for(i = 0; i < (nbytes); i++){
            // read the dummy
            garbage = SPI4BUF;
            // write the byte
            SPI4BUF = newByte[i];
            // wait for RBF
            while(SPI4STATbits.SPIRBF != 1);
        }
    }
    // read the dummy
    garbage = SPI4BUF;
    // wait for RBF
    while(SPI4STATbits.SPIRBF != 1);
    // read the dummy
    garbage = SPI4BUF;
    //negate CS
    PORTFbits.RF8 = 1;   
}
