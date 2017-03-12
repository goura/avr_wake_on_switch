# avr_wake_on_switch
```
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
```
