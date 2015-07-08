#ifndef PLATFORM_H
#define PLATFORM_H

#define UNIT16
#define MUNIT16
//#define mp_setp		P_SETP
//#define mp_addc		P_ADDC
//#define mp_subb		P_SUBB
//#define mp_rotate_left	P_ROTL
//#define mp_smula	P_SMULA
//#define mp_quo_digit	P_QUO_DIGIT
//#define mp_set_recip	P_SETRECIP
#define SMITH
#define PLATFORM_SPECIFIED

#define memzero(object) memset(&(object), 0, sizeof(object))

#endif	PLATFORM_H
