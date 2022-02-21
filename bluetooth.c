/*
 * File:   bluetooth.c
 * Author: ljjr8
 *
 * Created on February 21, 2022, 5:21 PM
 */


#include <xc.h>
#include "bluetooth.h"

#define RTS _RF13 // Output, For potential hardware handshaking.
#define CTS _RF12 // Input, For potential hardware handshaking.

#define ASTERISK 0x2A
#define CARRIAGE 0x0D
#define NEWLINE 0x0A
#define NULL 0x00

// UART 
void InitU2(void)
{
    U2BRG = 34; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy= 16MHz.
    U2MODE =0x8008; // See data sheet, pg148. Enable UART2, BRGH = 1,
    // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
    // Following lines pertain Hardware handshaking
    TRISFbits.TRISF13 = 1; // enable RTS , output
    RTS = 1; // default status , not ready to send
}

char putU2(char c)
{
    while (CTS); //wait for !CTS (active low)
    while (U2STAbits.UTXBF ); // Wait if transmit buffer full.
    U2TXREG = c; // Write value to transmit FIFO
    return c;
}

char getU2( void )
{
    RTS = 0; // telling the other side !RTS
    TMR1 = 0;
    while (!U2STAbits. URXDA && TMR1 < 100 * 2);
    if (!U2STAbits. URXDA)
    {
        RTS = 1;
        return NEWLINE;
    }
//    while (! U2STAbits . URXDA ); // wait
    RTS =1; // telling the other side RTS
    return U2RXREG ; // from receiving buffer
} //getU2

// Bluetooth Communication
int checkCommand(char c)
{
    if (c == 's')           // checks if command was START
    {
        return 1;
    }
    else if (c == 'e')      // checks if command was END
    {
        return 2;
    }
    else
    {
        return 0;
    }
}
int getCommand ()
{
    int command = -1;
    
    int isCommand;
    char str[3];
    int size = 0;
    
    while(command == -1)
    {
        char temp;
        temp = getU2();
        msDelay(50);
        
        // Checks if char is asterisk
        if (temp == ASTERISK)
        {
            isCommand = !isCommand;     //sets command flag
            
            if (isCommand)
            {
                // Start of a command
                continue;
            }
            else
            {
                // End of a command
                putU2(CARRIAGE);
                msDelay(50);
                putU2(NEWLINE);
                msDelay(50);
                U2STAbits.OERR = 0;
                command = checkCommand(str[0]);
            }
        }
        else if (isCommand && temp != NEWLINE)  // Middle of a command
        {
            str[size] = temp;
            putU2(temp);
            size++;
        }
    }
   
    return command;
}

