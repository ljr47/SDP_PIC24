/* 
 * File:   bluetooth.h
 * Author: ljjr8
 *
 * Created on February 21, 2022, 5:20 PM
 */

#ifndef BLUETOOTH_H
#define	BLUETOOTH_H

#ifdef	__cplusplus
extern "C" {
#endif

    // UART
    void InitU2(void);
    char putU2(char c);
    char getU2(void);
    
    // Bluetooth Communication
    int checkCommand (char c);
    int getCommand ();

#ifdef	__cplusplus
}
#endif

#endif	/* BLUETOOTH_H */

