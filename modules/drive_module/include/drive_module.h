#ifndef __DRIVE_MODULE_H__
#define __DRIVE_MODULE_H__


void init_drive_module();

//Pinout used to control L9110 motor drivers
#define F_L_FORWARD 12
#define F_L_REVERSE 11

#define F_R_FORWARD 14
#define F_R_REVERSE 13

#define B_L_FORWARD 4
#define B_L_REVERSE 3

#define B_R_FORWARD 7
#define B_R_REVERSE 6

#endif //__DRIVE_MODULE_H__