#include <xc.h>

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

void Servo_0Grados(void)
{
    unsigned char i;

    for(i=0; i<50; i++)
    {
        RC2 = 1;
        __delay_us(500);      // Extremo mínimo
        RC2 = 0;
        __delay_us(19500);
    }
}

void Servo_180Grados(void)
{
    unsigned char i;

    for(i=0; i<50; i++)
    {
        RC2 = 1;
        __delay_us(2500);     // Extremo máximo
        RC2 = 0;
        __delay_us(17500);
    }
}

void main(void)
{
    TRISC2 = 0;
    RC2 = 0;

    while(1)
    {
        Servo_0Grados();
        __delay_ms(1000);

        Servo_180Grados();
        __delay_ms(1000);
    }
}
