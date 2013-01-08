/* Name: main.c
 * Author: Justin R. Cutler <justin.r.cutler@gmail.com>
 * Copyright: 2012 Justin R. Cutler
 * License: GPLv3
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * ATtiny25/45/85 connections:
 *
 *         +-\/-+
 *   Vcc  1|o   |8  Vcc
 *    NC  2|    |7  NC
 *    NC  3|    |6  IR_TX
 *   GND  4|    |5  IR_RX
 *         +----+
 *
 * The ATtiny85 (http://www.sparkfun.com/products/9378) is generally the easiest
 * to find 8-pin DIP AVR.
 *
 * IR_RX should be connected to the output pin of an IR Receiver Diode such as:
 *  - TSOP85338 IR Receiver Breakout (http://www.sparkfun.com/products/8554)
 *  - TSOP38238 IR Receiver Diode (http://www.sparkfun.com/products/10266)
 *
 * IR_TX should be connected to a current-limited IR LED or a high-power
 * switched LED, such as:
 *  - LED Infrared 950nm (https://www.sparkfun.com/products/9349) with a 330 Ohm
 *    resistor
 *  - Max Power IR LED Kit (http://www.sparkfun.com/products/10732)
 *
 * The IR Control Kit Retail (http://www.sparkfun.com/products/10783) from
 * SparkFun Electronics contains everything you need to get started, other than
 * the ATtiny25/45/85.
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>


/* IR modulation frequency (in Hz) */
#define IR_MODULATION 38000

/* IR receive pin (demodulated, negative logic) */
#define IR_RX PINB0
/* IR transmit pin */
#define IR_TX PINB1

/* enable modulated IR transmit (via Timer1) */
#define IR_TX_ON()  do { TCCR1 |= _BV(COM1A1); } while (0)
/* disable modulated IR transmit (via Timer1) */
#define IR_TX_OFF() do { TCCR1 &= ~_BV(COM1A1); } while (0)


/* IR receiver pin change interrupt */
ISR(PCINT0_vect)
{
    /* test IR_RX pin (negative logic) */
    if (PINB & _BV(IR_RX))
    {
        IR_TX_OFF();
    }
    else
    {
        IR_TX_ON();
    }
}


void setup(void)
{
    /*
     * IR receiver setup
     */

    /* set IR_RX as input */
    DDRB &= _BV(IR_RX);
    /* enable interrupt on change of IR_RX */
    PCMSK |= _BV(PCINT0);
    GIMSK |= _BV(PCIE);

    /*
     * IR transmitter setup
     */

    /* enable IR modulation (50% duty cycle) via Timer1 */
    TCCR1 = _BV(PWM1A) | _BV(CS12);
    OCR1C = (((F_CPU / 8 / IR_MODULATION)) - 1);
    OCR1A = OCR1C / 2;
    /* set IR_TX as output */
    DDRB |= _BV(IR_TX);
    /* turn IR_TX off when not enabled */
    PORTB &= ~_BV(IR_TX);

    /*
     * Power saving
     */

    /* disable unnecessary hardware */
    PRR |= _BV(PRADC) | _BV(PRUSI);
    /* enable idle sleep mode */
    set_sleep_mode(SLEEP_MODE_IDLE);
}


int main(void)
{
    /* initialize hardware */
    setup();

    /* enable interrupts */
    sei();

    for (;;)
    {
        /* go to sleep (all work done in PCINT0 ISR) */
        sleep_mode();
    }

    return 0;   /* never reached */
}
