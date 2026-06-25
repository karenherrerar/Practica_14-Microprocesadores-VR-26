#include <xc.h>

// CONFIGURACION
#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 8000000

// Servo en RC2
#define SERVO_PIN PORTCbits.RC2

volatile unsigned int servo_us = 1500;
volatile unsigned char estado_servo = 0;

//================================================
// CARGAR TIMER1 EN MICROSEGUNDOS
// Fosc = 8 MHz
// Ciclo instruccion = 0.5 us
// Timer1 prescaler 1:2 => 1 us por cuenta
//================================================
void Timer1_Load_us(unsigned int us)
{
    unsigned int preload;

    preload = 65536 - us;

    TMR1H = (preload >> 8) & 0xFF;
    TMR1L = preload & 0xFF;
}

//================================================
// INTERRUPCION TIMER1 PARA SERVO
//================================================
void __interrupt() ISR(void)
{
    if(PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;

        if(estado_servo == 0)
        {
            SERVO_PIN = 1;
            estado_servo = 1;
            Timer1_Load_us(servo_us);
        }
        else
        {
            SERVO_PIN = 0;
            estado_servo = 0;
            Timer1_Load_us(20000 - servo_us);
        }
    }
}

//================================================
// ADC
//================================================
void ADC_Init(void)
{
    ANSEL = 0x01;     // AN0 analogico
    ANSELH = 0x00;    // Resto digitales

    TRISAbits.TRISA0 = 1;

    ADCON0 = 0b10000001; // ADC ON, canal AN0, Fosc/32
    ADCON1 = 0b10000000; // Justificado derecha, Vref = VDD/VSS
}

unsigned int ADC_Read(void)
{
    __delay_us(20);

    ADCON0bits.GO_nDONE = 1;

    while(ADCON0bits.GO_nDONE);

    return ((unsigned int)ADRESH << 8) | ADRESL;
}

//================================================
// PROMEDIO ADC PARA EVITAR BRINCOS
//================================================
unsigned int ADC_Read_Avg(void)
{
    unsigned long suma = 0;
    unsigned char i;

    for(i = 0; i < 8; i++)
    {
        suma += ADC_Read();
        __delay_ms(1);
    }

    return (unsigned int)(suma / 8);
}

//================================================
// TIMER1
//================================================
void Timer1_Init(void)
{
    T1CON = 0b00010001; // Timer1 ON, reloj interno, prescaler 1:2

    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;

    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    Timer1_Load_us(20000);
}

//================================================
// MAIN
//================================================
void main(void)
{
    unsigned int adc;
    unsigned int pulso_objetivo;
    unsigned int pulso_suave = 1500;

    TRISCbits.TRISC2 = 0;
    SERVO_PIN = 0;

    ADC_Init();
    Timer1_Init();

    while(1)
    {
        adc = ADC_Read_Avg();

        // ADC 0-1023 a pulso 1000-2000 us
        pulso_objetivo = 500 + ((unsigned long)adc * 2000UL) / 1023UL;

        // Suavizado para que no brinque
        if(pulso_objetivo > pulso_suave)
        {
            pulso_suave += (pulso_objetivo - pulso_suave) / 4;
            if(pulso_suave < pulso_objetivo) pulso_suave++;
        }
        else if(pulso_objetivo < pulso_suave)
        {
            pulso_suave -= (pulso_suave - pulso_objetivo) / 4;
            if(pulso_suave > pulso_objetivo) pulso_suave--;
        }

        // Actualizar variable usada por interrupcion
        INTCONbits.GIE = 0;
        servo_us = pulso_suave;
        INTCONbits.GIE = 1;

        __delay_ms(5);
    }
}
