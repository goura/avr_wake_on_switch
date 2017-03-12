#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#define LED_PORT PB0

/* ATTiny13 Wake on Switch Action Example
 *
 * Wake up and perform action when a switch connected on
 * PB1(pin 6) is on. Blinks LED connected on PB0 (pin 5).
 * INT0 is fixed on PB1, so it can't be changed.
 *
 * This program moves between the following states:
 *
 * UNKNOWN (initial state)
 * Moves to WAITING_FOR_LOW or HIGH, depending on the level of PB1
 *
 * WAITING_FOR_LOW
 * Waits for low level on PB1. On low, moves to ACTIVE.
 *
 * WAITING_FOR_HIGH
 * Waits for high level on PB1. On high, moves to WAITING_FOR_LOW
 *
 * ACTIVE
 * Performs action (blink 9 times on this demo)
 * After the action, moves to UNKNOWN
 *
 */

typedef enum State {
    UNKNOWN,
    WAITING_FOR_LOW,
    ACTIVE,
    WAITING_FOR_HIGH,
} State;

static State state;

void blink(int ms) {
    int i;
    PORTB |= (1<<LED_PORT);
    for (i = 0; i < (ms / 25); i++) _delay_ms(25);
    PORTB &= ~(1<<LED_PORT);
    for (i = 0; i < (ms / 25); i++) _delay_ms(25);
}

ISR(INT0_vect) {
    /* Disable all inturrupts */
    cli();
    GIMSK &= ~_BV(INT0);

    switch (state) {
    case WAITING_FOR_LOW:
        blink(50);
        state = ACTIVE;
        break;
    case WAITING_FOR_HIGH:
        blink(150); blink(150);
        state = WAITING_FOR_LOW;
        break;
    case ACTIVE:
        break;
    default:
        break;
    }
}

void setup(void) {
    /* Turn off ADC */
    ADCSRA &= ~(1 << ADSC);

    /* PB0 as output, PB1 as input */
    DDRB |= (1<<PB0) | (0<<PB1);

    /* Pull-up PB1 */
    PORTB |= (1<<PB1);

    state = UNKNOWN;
}

void setup_waiting_for_low(void) {
    state = WAITING_FOR_LOW;

    /* Set INT0 as low level trigger 
       (it's the only available trigger in SLEEP_MODE_PWR_DOWN) */
    MCUCR &= ~(_BV(ISC01) | _BV(ISC00));

    GIMSK |= _BV(INT0); // Enable INT0

    // We can use SLEEP_MODE_PWR_DOWN
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    sei(); // Enable all inturrupts
    sleep_mode();
}

void setup_waiting_for_high(void) {
    state = WAITING_FOR_HIGH;

    /* Set INT0 as rising edge trigger */
    MCUCR |= _BV(ISC01) | _BV(ISC00);

    GIMSK |= _BV(INT0); // Enable INT0

    /* We need SLEEP_MODE_IDLE to use rising edge trigger */
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    sei(); // Enable all inturrupts
    sleep_mode();
}

void do_action(void) {
    /* Blink nine times for now */
    blink(25); blink(25); blink(25);
    blink(25); blink(25); blink(25);
    blink(25); blink(25); blink(25);

    state = UNKNOWN;
}

/* Scan PB1 and decide which state should it be in */
void decide_state(void) {
    int i;
    int count;
    int scan_times = 10;

    while (1) {
        count = 0;
        /* Scan PB1 several times and count how many times it was high */
        for (i = 0; i < scan_times; i++) {
            if (PINB & _BV(PB1)) count++;
            _delay_ms(5);
        }

        if (count == scan_times) {
            /* PB1 is high, then state is WAITING_FOR_LOW */
            state = WAITING_FOR_LOW;
            break;
        } else if (count == 0) {
            /* PB1 is low, then state is WAITING_FOR_HIGH */
            state = WAITING_FOR_HIGH;
            break;
        } else {
            /* PB1 is not stabilized. Scan again */
            continue;
        }
    }
}

int main(void)
{
    setup();
    blink(50); blink(50);

    while(1) {
        /* Inturrupt should be cleared at this line */
        switch (state) {
        case UNKNOWN:
            decide_state();
            break;
        case WAITING_FOR_LOW:
            setup_waiting_for_low();
            break;
        case WAITING_FOR_HIGH:
            setup_waiting_for_high();
            break;
        case ACTIVE:
            do_action();
            break;
        default:
            break;
        }
    }
}
