#include <avr/sleep.h>

const int LED = 13;                          //LED on pin 13
const unsigned long KEEP_RUNNING = 10000;    //milliseconds

void setup(void)
{
    //to minimize power consumption while sleeping, output pins must not source
    //or sink any current. input pins must have a defined level; a good way to
    //ensure this is to enable the internal pullup resistors.

    for (byte i=0; i<20; i++) {    //make all pins inputs with pullups enabled
        pinMode(i, INPUT_PULLUP);
    }

    pinMode(LED, OUTPUT);          //make the led pin an output
    digitalWrite(LED, LOW);        //drive it low so it doesn't source current
}

void loop(void)
{
    for (byte i=0; i<10; i++) {     //flash the LED
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
    delay(KEEP_RUNNING);           //opportunity to measure active supply current 
    digitalWrite(LED, HIGH);       //one blink before sleeping
    delay(100);
    digitalWrite(LED, LOW);
    goToSleep();
}

void goToSleep(void)
{
    byte adcsra = ADCSRA;          //save the ADC Control and Status Register A
    ADCSRA = 0;                    //disable the ADC
    EICRA = _BV(ISC01);            //configure INT0 to trigger on falling edge
    EIMSK = _BV(INT0);             //enable INT0
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();                         //stop interrupts to ensure the BOD timed sequence executes as required
    sleep_enable();
    //disable brown-out detection while sleeping (20-25µA)
    uint8_t mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);
    uint8_t mcucr2 = mcucr1 & ~_BV(BODSE);
    MCUCR = mcucr1;
    MCUCR = mcucr2;
    //sleep_bod_disable();           //for AVR-GCC 4.3.3 and later, this is equivalent to the previous 4 lines of code
    sei();                         //ensure interrupts enabled so we can wake up again
    sleep_cpu();                   //go to sleep
    sleep_disable();               //wake up here
    ADCSRA = adcsra;               //restore ADCSRA
}

//external interrupt 0 wakes the MCU
ISR(INT0_vect)
{
    EIMSK = 0;                     //disable external interrupts (only need one to wake up)
}
