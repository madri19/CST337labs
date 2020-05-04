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
#include <stdio.h>
#include <string.h>

void setup();
int readStatusCommand();
void writeEnableCommand();
void ReadEEProm(int nbytes, unsigned int eeprom_address, unsigned char* readbuffer);
void WriteEEProm(int nbytes, unsigned int eeprom_address, unsigned char* writebuffer);

unsigned char _BUFFER_[100];
unsigned int _EEPROM_ADDRESS_;
int _NBYTES_;
unsigned int _STATE_;
int _EEPROM_SYS_BUSY_;
int _READ_OR_WRITE_;// 0 = read, 1 = write
int _INDEX_;


int main(){
    
    
    unsigned int eeprom_address = 0x4000;
    //unsigned char writebuffer[100] = "Z";
    //unsigned char writebuffer[100] = "QW";
    unsigned char writebuffer[100] = "IOP";
    //unsigned char writebuffer[100] = "8888888888888888888888888888888888888888888888888888888888888888";
    unsigned char readbuffer[100];
    int nbytes = strlen(writebuffer);
  
    setup();
    WriteEEProm(nbytes, eeprom_address, writebuffer);
    ReadEEProm(nbytes, eeprom_address, readbuffer);
    
    return 0;
}

void __ISR_AT_VECTOR(_SPI4_RX_VECTOR, IPL7SRS) SpiServ(void){
    
    int garbageCollector;
    int statusRead;
    
    // set up the address split
    uint8_t bytes[2];
    //split the 16 into 2 8-bits
    bytes[0] = _EEPROM_ADDRESS_ >> 8;
    bytes[1] = _EEPROM_ADDRESS_ & 0x00FF;
    
    
    switch(_STATE_) {
      case 0:
         // start CHECK STATUS
            // assert CS
            PORTFbits.RF8 = 0;
            //Send READ STATUS Command to the chip
            SPI4BUF = 0b00000101;
            // wait for TBE
            while(SPI4STATbits.SPITBE == 0);
            // send 0Dummy
            SPI4BUF = 'a';
            // set the next state
            _STATE_ = 1;
         break;
      case 1:
            // mid CHECK STATUS
            // read the iDummy
            garbageCollector = SPI4BUF;
            // set the next state
            _STATE_ = 2;
         break;
      case 2:
            // end Check Status, start Read or Write Enable or repeat Check Status
            // read the status
            statusRead = SPI4BUF;
            // negate CS
            PORTFbits.RF8 = 1;
            // make some cycles pass
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            // Assert CS
            PORTFbits.RF8 = 0;
            // if write is still in progress we need to start check status again
            if(statusRead & 0b00000001){
                //Send READ STATUS Command to the chip
                SPI4BUF = 0b00000101;
                // wait for TBE
                while(SPI4STATbits.SPITBE == 0);
                // send 0Dummy
                SPI4BUF = 'a';
                // set the next state
                _STATE_ = 1;
            }
            else if(_READ_OR_WRITE_){// do we want to write or read?, 0 = read, 1 = write
                // write a "write enable" command to SPI4BUF
                SPI4BUF = 0b00000110;
                //set the next state
                _STATE_ = 3;
            }
            else{// go here if we want to read
                //write "read data command" 
                SPI4BUF = 0b00000011;
                //wait for TBE
                while(SPI4STATbits.SPITBE == 0);
                //write most sig address to SPI4BUF
                SPI4BUF = bytes[0];
                // set the next state
                _STATE_ = 4;
            }
          
         break;
      case 3: // end write enable, start writing
            // read iDummy
            garbageCollector = SPI4BUF;
            // negate CS
            PORTFbits.RF8 = 1;
            // make some cycles pass
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            // Assert CS
            PORTFbits.RF8 = 0;
            //write "write data command" 
            SPI4BUF = 0b00000010;
            //wait for TBE
            while(SPI4STATbits.SPITBE == 0);
            //write most sig address to SPI4BUF
            SPI4BUF = bytes[0];
            _STATE_ = 6;
         break;
       case 4: // continue reading
           // read the dummy
            garbageCollector = SPI4BUF;
            // write least sig address to SPI4BUF
            SPI4BUF = bytes[1];
            _STATE_ = 5;
            break;
       case 5:
            // read the dummy
            garbageCollector = SPI4BUF;
            // write the dummy
            SPI4BUF = garbageCollector;
            _STATE_ = 9;
           break;
       case 6:// continue writing
            // read the dummy
            garbageCollector = SPI4BUF;
            // write least sig address to SPI4BUF
            SPI4BUF = bytes[1];
            _STATE_ = 7;
            _INDEX_ = 0;
           break;
       case 7:
            if(_INDEX_ < _NBYTES_){
                // read the dummy
                garbageCollector = SPI4BUF;
                // write the byte
                SPI4BUF = _BUFFER_[_INDEX_];
                _INDEX_++;
            }
            else {
                // read the dummy
                garbageCollector = SPI4BUF;
                _INDEX_ = 0;
                _STATE_ = 8;
            }
           break;
        case 8:
            // read the dummy
            garbageCollector = SPI4BUF;
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            //negate CS
            PORTFbits.RF8 = 1;
            // last state of action(either read or write)
            _EEPROM_SYS_BUSY_ = 0;
            _STATE_ = 0;
           break;
        case 9:
            // read the dummy
            garbageCollector = SPI4BUF;
            // write the dummy
            SPI4BUF = garbageCollector;
            _STATE_ = 10;
            _INDEX_ = 0;
           break;
        case 10:
            if(_NBYTES_ == 1){
                //only process one byte
                _BUFFER_[0] = SPI4BUF;
                asm volatile("nop");
                asm volatile("nop");
                asm volatile("nop");
                asm volatile("nop");
                asm volatile("nop");
                asm volatile("nop");
                //negate CS
                PORTFbits.RF8 = 1;
                _EEPROM_SYS_BUSY_ = 0;
                _STATE_ = 0;
                _INDEX_ = 0;
            }
            else if(_INDEX_ < (_NBYTES_-2)){
                // read the byte
                _BUFFER_[_INDEX_] = SPI4BUF;
                // write the dummy
                SPI4BUF = garbageCollector;
                _INDEX_++;
            }
            else{
                // read the byte
                _BUFFER_[_NBYTES_-2] = SPI4BUF;
                _STATE_ = 11;
            }
           break;
        case 11:
            // read the byte
            _BUFFER_[_NBYTES_-1] = SPI4BUF;
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            asm volatile("nop");
            //negate CS
            PORTFbits.RF8 = 1;
            _EEPROM_SYS_BUSY_ = 0;
            _STATE_ = 0;
            _INDEX_ = 0;
           break;
      default:
            // ERROR
            asm volatile("nop");
   }
    
    // clear flag
    IFS5CLR = _IFS5_SPI4RXIF_MASK;
}

void ReadEEProm(int nbytes, unsigned int eeprom_address, unsigned char* readbuffer){
    
    // make sure system is not busy
    while(_EEPROM_SYS_BUSY_ == 1);
    
    // make sure the state is 0
    if(_STATE_ != 0){
        //ERROR
    }
    
    //manage the parameter data
    _EEPROM_ADDRESS_ = eeprom_address;
    _NBYTES_ = nbytes;
    
    //let the system know we are reading
    _READ_OR_WRITE_ = 0;
    
    // the system is busy
    _EEPROM_SYS_BUSY_ = 1;
    
    // raise the flag
    IFS5SET = _IFS5_SPI4RXIF_MASK;
    
    while(_EEPROM_SYS_BUSY_ == 1);
    
    strncpy(readbuffer, _BUFFER_, _NBYTES_);
    
}

void WriteEEProm(int nbytes, unsigned int eeprom_address, unsigned char* writebuffer){
    
    int i; 
    
    // make sure system is not busy
    while(_EEPROM_SYS_BUSY_ == 1);
    
    // make sure the state is 0
    if(_STATE_ != 0){
        //ERROR
    }
    
    
    _NBYTES_ = nbytes;
    strncpy(_BUFFER_, writebuffer, _NBYTES_);
    _EEPROM_ADDRESS_ = eeprom_address;
    
    
    //let the system know we are writing
    _READ_OR_WRITE_ = 1;
    
    
    // the system is busy
    _EEPROM_SYS_BUSY_ = 1;
    
    // raise the flag
    IFS5SET = _IFS5_SPI4RXIF_MASK;
}

void setup(){
    
    //Globally disable interrupts
    asm volatile("di");
    
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
    
    // BRG is 20 so we can get a clock of 2 MHz
    SPI4BRG = 9;
    
    //Set up multi vector interrupt system mode
    INTCONSET = _INTCON_MVEC_MASK;
    
    // Set up priority 7 for SPI4_RX
    IPC41CLR = _IPC41_SPI4RXIP_MASK | _IPC41_SPI4RXIS_MASK;        // clear SPI4_RX priority and sub priority
    IPC41SET = (7 << _IPC41_SPI4RXIP_POSITION) & _IPC41_SPI4RXIP_MASK; // Set Priority 7, Sub priority 0
    
    // Set up shadow set
    PRISS = 0x70000000;                                 //Priority 7 will go to shadow set 7
    
    // clear flag
    IFS5CLR = _IFS5_SPI4RXIF_MASK;
    
    // Enable master SPI
    SPI4CONbits.MSTEN = 1;
    // Enable SPI4 module
    SPI4CONbits.ON = 1;
    
    // Enable SPI4 RX interrupt
    IEC5SET = _IEC5_SPI4RXIE_MASK;
    
    // Globally enable interrupts
    asm volatile("ei");
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
