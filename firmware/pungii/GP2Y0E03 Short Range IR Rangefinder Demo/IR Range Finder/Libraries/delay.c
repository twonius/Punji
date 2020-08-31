#include "delay.h"


void delay_us(unsigned int value)
{
    register unsigned int loops =  ((F_CPU * value) >> 4) ;

    while(loops)
    {
        _delay_cycles(1);
        loops--;
    };
}


void delay_ms(unsigned int value)
{
    while(value)
    {
        delay_us(1000);
        value--;
    };
}
