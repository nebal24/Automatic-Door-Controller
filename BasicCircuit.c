// PIC18F4620 Configuration Bit Settings
#pragma config OSC = XT
#pragma config FCMEN = OFF
#pragma config IESO = OFF
#pragma config PWRT = OFF
#pragma config BOREN = SBORDIS
#pragma config BORV = 3
#pragma config WDT = ON
#pragma config WDTPS = 32768
#pragma config CCP2MX = PORTC
#pragma config PBADEN = OFF      // FIX 1: was ON ? kept RB0/RB1 analog, breaking buttons
#pragma config LPT1OSC = OFF
#pragma config MCLRE = ON
#pragma config STVREN = ON
#pragma config LVP = ON
#pragma config XINST = OFF
#pragma config CP0 = OFF
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CP3 = OFF
#pragma config CPB = OFF
#pragma config CPD = OFF
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF
#pragma config WRTC = OFF
#pragma config WRTB = OFF
#pragma config WRTD = OFF
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF
#pragma config EBTR3 = OFF
#pragma config EBTRB = OFF

#define _XTAL_FREQ 4000000UL

#include <xc.h>
#include <stdio.h>
#include "lcd_x8.h"
#include "my_adc.h"
#include "my_ser.h"

#define TRIG   LATCbits.LATC0
#define BUZZER LATCbits.LATC1
#define SERVO  LATCbits.LATC2
#define ECHO   PORTBbits.RB2


void initPorts(void);
void delay_ms(unsigned int n);
void delay_us(unsigned int n);
void buzzer_tone(unsigned int freq, unsigned int duration_ms);

void init_ultrasonic(void);
unsigned int measure_distance_cm(void);

void servo_open(void);
void servo_close(void);
void servo_mid(void);

void checkModeButton(void);
void executePart1(void);
void executePart2(void);

void door_beep(unsigned char times);

unsigned char buzzer_on = 0;         // tracks buzzer toggle state
unsigned char currentMode = 1;   // 1 = Part 1, 2 = Part 2

void delay_ms(unsigned int n) {
    unsigned int i;
    for (i = 0; i < n; i++) {
        __delaywdt_ms(1);
    }
}

// FIX 2: buzzer_tone was passing microsecond values to delay_ms ? completely wrong unit
void delay_us(unsigned int n) {
    unsigned int i;
    for (i = 0; i < n; i++) {
        __delaywdt_us(1);
    }
}

void buzzer_tone(unsigned int freq, unsigned int duration_ms) {
    unsigned long cycles;
    unsigned int delay_half_us;

    if (freq == 0) return;

    delay_half_us = 1000000UL / (2 * freq);  // half-period in microseconds
    cycles = ((unsigned long)duration_ms * 1000UL) / (2 * delay_half_us);

    while (cycles--) {
        BUZZER = 1;
        delay_us(delay_half_us);   // FIX 2: must use delay_us here, not delay_ms
        BUZZER = 0;
        delay_us(delay_half_us);
    }
}

void init_ultrasonic(void)
{
    T1CON = 0x00;
    PIR1bits.TMR1IF = 0;
    TMR1H = 0;
    TMR1L = 0;
}

unsigned int measure_distance_cm(void)
{
    unsigned int t = 0;
    unsigned int distance_cm;

    TRIG = 0;
    __delay_us(2);
    TRIG = 1;
    __delay_us(10);
    TRIG = 0;

    while (!ECHO && (t < 60000))
    {
        __delay_us(1);
        t++;
    }

    if (t >= 60000)
        return 0;

    TMR1H = 0;
    TMR1L = 0;
    PIR1bits.TMR1IF = 0;
    T1CONbits.TMR1ON = 1;

    while (ECHO && !PIR1bits.TMR1IF)
    {
    }

    T1CONbits.TMR1ON = 0;

    if (PIR1bits.TMR1IF)
        return 0;

    t = ((unsigned int)TMR1H << 8) | TMR1L;
    distance_cm = t / 58;

    return distance_cm;
}

void servo_open(void)
{
    unsigned char i;
    for (i = 0; i < 50; i++)
    {
        SERVO = 1;
        __delay_ms(1);     // 1 ms HIGH
        SERVO = 0;
        __delay_ms(19);    // total about 20 ms
    }
}

void servo_close(void)
{
    unsigned char i;
    for (i = 0; i < 50; i++)
    {
        SERVO = 1;
        __delay_ms(2);     // 2 ms HIGH
        SERVO = 0;
        __delay_ms(18);    // total about 20 ms
    }
}

void servo_mid(void)
{
    unsigned char i;
    for (i = 0; i < 50; i++)
    {
        SERVO = 1;
        __delay_ms(1);
        __delay_us(500);   // 1.5 ms HIGH
        SERVO = 0;
        __delay_ms(18);
        __delay_us(500);   // total about 20 ms
    }
}

void checkModeButton(void)
{
    static unsigned char lastButtonState = 1;

    if (PORTBbits.RB0 == 0 && lastButtonState == 1)
    {
        delay_ms(30);   // debounce
        if (PORTBbits.RB0 == 0)
        {
            if (currentMode == 1)
                currentMode = 2;
            else
                currentMode = 1;

            lcd_putc('\f');   // clear LCD on mode switch
        }
    }

    lastButtonState = PORTBbits.RB0;
}


void executePart1(void)
{
    char buffer1[17];
    char x;
    static unsigned char pos = 0;

    int adc_value0 = read_adc_raw_no_lib(0);
    float vol = (adc_value0 / 1023.0) * 5.0;

    int adc_value1 = read_adc_raw_no_lib(1);
    float temp = (adc_value1 / 1023.0) * 500.0;

    // Line 1: voltage + temperature
    sprintf(buffer1, "V=%4.2f T=%4.1f", vol, temp);
    lcd_gotoxy(1, 1);
    lcd_puts("                ");
    lcd_gotoxy(1, 1);
    lcd_puts(buffer1);

    // UART handling
    if (is_byte_available())
    {
        x = read_byte_no_lib();

        // If B/b received: toggle buzzer and DO NOT print it
        if (x == 'B' || x == 'b')
        {
            buzzer_on = !buzzer_on;

            if (buzzer_on)
                buzzer_tone(1000, 150);   // short confirmation beep
            else
                BUZZER = 0;
        }
        else
        {
            // Print normal characters on line 2
            lcd_gotoxy(pos + 1, 2);
            lcd_putc(x);
            pos++;

            // If line full, clear it
            if (pos >= 16)
            {
                lcd_gotoxy(1, 2);
                lcd_puts("                ");
                pos = 0;
            }
        }
    }

    CLRWDT();
    delay_ms(100);
}
void door_beep(unsigned char times)
{
    unsigned char i;
    for (i = 0; i < times; i++)
    {
        BUZZER = 1;
        __delay_ms(120);
        BUZZER = 0;
        __delay_ms(120);
    }
}
void executePart2(void)
{
    char buffer1[17];
    unsigned int distance;
    static unsigned char doorOpen = 0;

    distance = measure_distance_cm();

    if (distance == 0)
        sprintf(buffer1, "AUTO D=--- cm  ");
    else
        sprintf(buffer1, "AUTO D=%3u cm  ", distance);

    lcd_gotoxy(1, 1);
    lcd_puts("                ");
    lcd_gotoxy(1, 1);
    lcd_puts(buffer1);

    if (distance > 0 && distance <= 15)
    {
        lcd_gotoxy(1, 2);
        lcd_puts("Door Opened     ");

        if (!doorOpen)
        {
            servo_open();
            door_beep(3);
            doorOpen = 1;
        }
    }
    else
    {
        lcd_gotoxy(1, 2);
        lcd_puts("Door Closed     ");

        if (doorOpen)
        {
            servo_close();
            doorOpen = 0;
        }
    }

    delay_ms(100);
    CLRWDT();
}



void initPorts(void) {
    // FIX 6: 0x0D configures AN0?AN12 as analog ? too many!
    // Use 0x0D only if your my_adc library sets this. If not, use:
    // ADCON1 = 0x0D means AN0 and AN1 are analog, rest digital ? check your lib
    ADCON1 = 0x0D;   // AN0 and AN1 analog, rest digital

    LATA = 0;
    LATB = 0;
    LATC = 0;
    LATD = 0;
    LATE = 0;

    TRISA = 0xFF;    // all inputs (AN0, AN1 are analog)

     TRISBbits.TRISB0 = 1;   // mode button
TRISBbits.TRISB1 = 1;
TRISBbits.TRISB2 = 1;   // ultrasonic ECHO
TRISBbits.TRISB3 = 1;
TRISBbits.TRISB4 = 1;
TRISBbits.TRISB5 = 1;
TRISBbits.TRISB6 = 1;
TRISBbits.TRISB7 = 1;

TRISCbits.TRISC0 = 0;   // ultrasonic TRIG
TRISCbits.TRISC1 = 0;   // buzzer
TRISCbits.TRISC2 = 0;   // servo
TRISCbits.TRISC3 = 0;
TRISCbits.TRISC4 = 0;
TRISCbits.TRISC5 = 0;
TRISCbits.TRISC6 = 0;   // UART TX
TRISCbits.TRISC7 = 1;   // UART RX

    TRISD = 0x00;        // LCD data bus ? all output
    TRISE = 0x00;
}


void main(void)
{
    INTCON = 0;
    initPorts();
    setupSerial();
    init_adc_no_lib();
    init_ultrasonic();
    lcd_init();
    lcd_putc('\f');

    TRIG = 0;
    BUZZER = 0;
    SERVO = 0;

    servo_close();   // start with door closed

    while (1)
    {
        checkModeButton();

        if (currentMode == 1)
            executePart1();
        else
            executePart2();

        CLRWDT();
    }
}