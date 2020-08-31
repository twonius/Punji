#include <msp430.h> 
#include "delay.h"
#include "lcd.h"
#include "lcd_print.h"


void setup(void);



void main(void)
{
    float d = 0.0;
    unsigned char s = 0;
    signed long ADC_val = 0;

    setup();

    while(1)
    {

        P6OUT |= BIT2;

        for(s = 0; s < 16; s++)
        {
            ADC12CTL0 |= (ADC12SC | ENC);
            while((ADC12IFG & BIT1) == 0);
            ADC_val += ADC12MEM1;
        }

        P6OUT &= BIT2;

        ADC_val >>= 4;

        d = (69.0 - ((float)ADC_val * 0.0226));

        if(d > 64)
        {
            d = 64;
        }

        if(d <= 0)
        {
            d = 0;
        }

        print_F(10, 1, d, 2);

        delay_ms(100);
    };
}


void setup(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    BCSCTL2 = SELM_2 | DIVM_0 | SELS | DIVS_0;
    BCSCTL1 |= DIVA_0;
    BCSCTL1 &= ~XT2OFF;

    P1OUT = 0;
    P1DIR = 0;
    P1IES = 0;
    P1IFG = 0;
    P2OUT = 0;
    P2DIR = 0;
    P2IES = 0;
    P2IFG = 0;
    P3OUT = 0;
    P3DIR = 0;
    P4OUT = 0;
    P4DIR = BIT0 | BIT1 | BIT2;
    P5OUT = 0;
    P5DIR = BIT4 | BIT5 | BIT6 | BIT7;
    P6OUT = 0;
    P6SEL = BIT1;
    P6DIR = BIT2;

    ADC12CTL0 &= ~ENC;
    ADC12CTL0 = SHT1_2 | SHT0_2 | MSC | REFON | ADC12ON;
    ADC12CTL1 = CSTARTADD_1 | SHS_0 | SHP | ADC12DIV_1 | ADC12SSEL_0 | CONSEQ_2;
    ADC12MCTL0 = SREF_0 | INCH_0;
    ADC12MCTL1 = SREF_0 | INCH_1;
    ADC12MCTL2 = SREF_0 | INCH_2;
    ADC12MCTL3 = SREF_0 | INCH_3;
    ADC12MCTL4 = SREF_0 | INCH_4;
    ADC12MCTL5 = SREF_0 | INCH_5;
    ADC12MCTL6 = SREF_0 | INCH_6;
    ADC12MCTL7 = SREF_0 | INCH_7;
    ADC12MCTL8 = SREF_0 | INCH_8;
    ADC12MCTL9 = SREF_1 | INCH_9;
    ADC12MCTL10 = SREF_0 | INCH_10;
    ADC12MCTL11 = SREF_0 | INCH_11;
    ADC12MCTL12 = SREF_0 | INCH_12;
    ADC12MCTL13 = SREF_0 | INCH_12;
    ADC12MCTL14 = SREF_0 | INCH_12;
    ADC12MCTL15 = SREF_0 | INCH_15;
    __delay_cycles(30000);
    ADC12CTL0 |= ENC;

    LCD_init();
    LCD_clear_home();

    LCD_goto(1, 0);
    LCD_putstr("GP2Y0E02 Demo.");

    LCD_goto(0, 1);
    LCD_putstr("D/cm:");
}
