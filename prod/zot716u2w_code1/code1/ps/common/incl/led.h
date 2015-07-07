#ifndef LED_TIME_H
#define LED_TIME_H

//ZOT716u2 extern unsigned char WirelessLightToggle;
extern unsigned char StatusLightToggle;
//ZOT716u2 extern unsigned char LanLightToggle;
extern unsigned char USB11LightToggle;	//ZOT716u2
extern unsigned char USB20LightToggle;	//ZOT716u2

void LightOnForever(unsigned char LED);
void Light_On(unsigned char LED);
void Light_Off(unsigned char LED);
//ZOT716u2 void Light_Flash(int nOnLoop,int nOffLoop);
//ZOT716u2 void Light_ALL_Flash(int nOnLoop,int nOffLoop);

#define ErrLightOnForever(code) LightOnForever((code))

#endif  LED_TIME_H
