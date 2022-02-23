/** 
 * Luke Rogers 
 * Jason Colonna 
 * Lab 9 - Bluetooth 
 * 11/17/20 
 **/

#include "mcc_generated_files/system.h"
#include <stdio.h>
#include <string.h>
#include "bluetooth.h"

//#define RTS _RF13 // Output, For potential hardware handshaking.
//#define CTS _RF12 // Input, For potential hardware handshaking.
//
//#define ASTERISK 0x2A
//#define CARRIAGE 0x0D
//#define NEWLINE 0x0A
//#define NULL 0x00
#define CHAR_A 0x41
#define CHAR_Z 0x5a
#define CHAR_a 0x61
#define CHAR_z 0x7a

void msDelay(int ms) 
{
    T1CON = 0x8030; // Timer 1 On, prescale 1:256, Tcy as clock  
    TMR1 = 0; // Clear TMR1  
    while (TMR1 < ms * 62)
    {
    }
}

void us_delay(int n)
{
    T1CON = 0x8010; // Timer 1 on, Prescale 8:1 
    TMR1 = 0;
    while (TMR1 < n * 2); // (1/(16MHz/8))*2 = 1us, i is multiplier for ius. 
}

void InitPMP( void)
{
    // PMP initialization. See my notes in Sec 13 PMP of Fam. Ref. Manual
    PMCON = 0x8303; // Following Fig. 13-34. Text says 0x83BF (it works) *
    PMMODE = 0x03FF; // Master Mode 1. 8-bit data, long waits.
    PMAEN = 0x0001; // PMA0 enabled
}

//void InitU2(void)
//{
//    U2BRG = 34; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy= 16MHz.
//    U2MODE =0x8008; // See data sheet, pg148. Enable UART2, BRGH = 1,
//    // Idle state = 1, 8 data, No parity, 1 Stop bit
//    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
//    // Following lines pertain Hardware handshaking
//    TRISFbits.TRISF13 = 1; // enable RTS , output
//    RTS = 1; // default status , not ready to send
//}
//
//char putU2(char c)
//{
//    while (CTS); //wait for !CTS (active low)
//    while (U2STAbits.UTXBF ); // Wait if transmit buffer full.
//    U2TXREG = c; // Write value to transmit FIFO
//    return c;
//}
//
//char getU2( void )
//{
//    RTS = 0; // telling the other side !RTS
//    TMR1 = 0;
//    while (!U2STAbits. URXDA && TMR1 < 100 * 2);
//    if (!U2STAbits. URXDA)
//    {
//        RTS = 1;
//        return NEWLINE;
//    }
////    while (! U2STAbits . URXDA ); // wait
//    RTS =1; // telling the other side RTS
//    return U2RXREG ; // from receiving buffer
//} //getU2

void InitLCD( void)
{
    // PMP is in Master Mode 1, simply by writing to PMDIN1 the PMP takes care
    // of the 3 control signals so as to write to the LCD.
    PMADDR = 0; // PMA0 physically connected to RS, 0 select Control register
    PMDIN1 = 0b00111000; // 8-bit, 2 lines, 5X7. See Table 9.1 of text Function set
    msDelay(1); // 1ms > 40us
    PMDIN1 = 0b00001100; // ON, cursor off, blink off
    msDelay(1); // 1ms > 40us
    PMDIN1 = 0b00000001; // clear display
    msDelay(2); // 2ms > 1.64ms
    PMDIN1 = 0b00000110; // increment cursor, no shift
    msDelay(2); // 2ms > 1.64ms
} // InitLCD

char ReadLCD( int addr)
{
    // As for dummy read, see 13.4.2, the first read has previous value in PMDIN1
    int dummy;
    while( PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr; // select the command address
    dummy = PMDIN1; // initiate a read cycle, dummy
    while( PMMODEbits.BUSY); // wait for PMP to be available
    return( PMDIN1); // read the status register
} // ReadLCD

// In the following, addr = 0 -> access Control, addr = 1 -> access Data
#define BusyLCD() ReadLCD( 0) & 0x80 // D<7> = Busy Flag
#define AddrLCD() ReadLCD( 0) & 0x7F // Not actually used here
#define getLCD() ReadLCD( 1) // Not actually used here.

void WriteLCD( int addr, char c)
{
    while (BusyLCD());
    while( PMMODEbits.BUSY); // wait for PMP to be available
    PMADDR = addr;
    PMDIN1 = c;
} // WriteLCD

// In the following, addr = 0 -> access Control, addr = 1 -> access Data
#define putLCD( d) WriteLCD( 1, (d))
#define CmdLCD( c) WriteLCD( 0, (c))
#define HomeLCD() WriteLCD( 0, 2) // See HD44780 instruction set in
#define ClrLCD() WriteLCD( 0, 1) // Table 9.1 of text book

void putsLCD( char *s)
{
    while( *s) putLCD( *s++); // See paragraph starting at bottom, pg. 87 text
} //putsLCD

void SetCursorAtLine(int i)
{
    int k;
    if (i == 1)
        CmdLCD(0x80); // Set DDRAM (i.e., LCD) address to upper left (0x80 | 0x00)
    else if(i == 2)
        CmdLCD(0xC0); // Set DDRAM (i.e., LCD) address to lower left (0x80 | 0x40)
    else
    {
        TRISA = 0x00; // Set PORTA<7:0> for output.
        for (k = 1; k < 20; k++) // Flash all 7 LED's @ 5Hz. for 4 seconds.
        {
            PORTA = 0xFF;
            msDelay(100); // 100 ms for ON then OFF yields 5Hz
            PORTA = 0x00;
            msDelay(100);
        }
    }
}

void initPortA(void)
{
    PORTA = 0x0000; //clear port A
    TRISA = 0xFF00; // set PORTA <6:0> to output
}
void initButtons(unsigned int mask)
{
    if(mask&0x0008) TRISDbits.TRISD6 = 1; // S3 TRISA
    if(mask&0x0004) TRISDbits.TRISD7 = 1; // S6 TRISA
    if(mask&0x0002) TRISAbits.TRISA7 = 1; //S5 TRISA
    if(mask&0x0001) TRISDbits.TRISD13 = 1; // S4 TRISA
}

unsigned int getButton(unsigned int mask) {
    unsigned int button;
    initButtons(0x000F);
    switch (mask) {
        case 0x0008: button = !PORTDbits.RD6; // S3
            break;
        case 0x0004: button = !PORTDbits.RD7; // S6
            break;
        case 0x0002: TRISAbits.TRISA7 = 1; // S5
            button = !PORTAbits.RA7;
            break;
        case 0x0001: button = !PORTDbits.RD13; // S4
            break;
        default:
            button = 0;
    }
    return (button);
}

void setPortA(unsigned int display)
{
    TRISAbits.TRISA7 = 0;
    PORTA = 0x00FF&display; // transfer to PORTA (LED?s)
}

int isLetter(char c)
{
    return ((c >= CHAR_A && c <= CHAR_Z) || (c >= CHAR_a && c <= CHAR_z));
}

int main(void) 
{   
    // initialize the device 
    SYSTEM_Initialize();
    
    initPortA();
    
    msDelay(32);
    InitPMP();
    InitLCD();
    InitU2();

    SetCursorAtLine(1);
    
    putsLCD("Start LCD");
   
//    int isCommand;
//    char str[25];
//    int size = 0;

    int command;
    while (1)
    {
        U2STAbits.OERR = 0;

        command = getCommand();
        msDelay(50);
        SetCursorAtLine(2);
//        putsLCD("                ");
//        msDelay(50);
        if (command == 1)
        {
            putsLCD("Start           ");
            msDelay(50);
        }
        else if (command == 2)
        {
            putsLCD("End             ");
            msDelay(50);
        }
        else
        {
            putsLCD("Not command     ");
            msDelay(50);
        }
//        char tmp;
//        tmp = getU2();
//        PORTA = 0x01 << 7;
//        msDelay(50);
//        
//        if (isLetter(tmp))
//        {
//            char t[4];
//            sprintf(t, "%c", tmp);
//            putsLCD(t);
//        }
//        
//        //checks if command starts w/ an asterisk
//        if (tmp == ASTERISK)
//        {
//            PORTA = 0x01 << 2;
//            isCommand = !isCommand;     //sets command flag
//            PORTA |= 0x01 << 3;
//                       
//            // checks if it is a command
//            if (isCommand)
//            {
//                //beginning of a command
//                PORTA = 0x01 << 6;
//                continue;
//            }
//            else
//            {
//                //end of a command
//                SetCursorAtLine(2);
//                // Only 16 characters fit on the LCD
//                str[16] = NULL;
//                int i;
//                for (i = 0; i < 25; i++)
//                {
//                    str[i] = NULL;
//                }
//                size = 0;
//                isCommand = 0;
//                tmp = NULL;
//                putU2(0x0D);
//                msDelay(50);
//                putU2(0x0A);
//                msDelay(50);
//                U2STAbits.OERR = 0;
//                PORTA = 0x01 << 7;
//            }
//        }
//        else if (isCommand && tmp != NEWLINE)   //middle of a command
//        {
//            char temp_str[2];
//            sprintf(temp_str, "%c", tmp);
//            putsLCD(temp_str);
//
//            putU2(tmp);
//            size++;
//            PORTA = 0x01;
//        }
    }
    return 1;
}

/** 
End of File 
 */


