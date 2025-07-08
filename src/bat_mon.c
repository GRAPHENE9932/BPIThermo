#include "bat_mon.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t bat_mon_voltage = 0;

void bat_mon_init(void) {
    bat_mon_voltage = BAT_MON_VOLTAGE_UNKNOWN;

    // Here we initialize the Timer/Counter1 to be triggering the ADC every ~half a second.
    // Disable the Output Compare pins and use the normal mode of operation.
    TCCR1A = 0;
    // Do not enable the input capture noise canceler and set the /64 prescaler.
    TCCR1B = (1 << CS11) | (1 << CS10);
    
    // Select the internal 1.1V reference and ADC7 (PA1) channel.
    ADMUX = ADC7D;
    // Enable the ADC, trigger the conversion start, enable the auto trigger mode, enable interrupts,
    // and set the ADC clock prescaler to /128.
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // Set the ADC trigger source to Timer/Counter1 Overflow.
    ADCSRB = (1 << ADTS2) | (1 << ADTS1);
}

enum bat_state bat_get_state(void) {
    if (bat_mon_voltage > BAT_MON_VOLTAGE_LOW || bat_mon_voltage == BAT_MON_VOLTAGE_UNKNOWN)
        return BAT_NORMAL;
    else if (bat_mon_voltage > BAT_MON_VOLTAGE_CRITICAL)
        return BAT_LOW;
    else
        return BAT_CRIT;
}

ISR(ADC_vect) {
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    bat_mon_voltage = low | ((uint16_t)high << 8);
}
