#include <avr/io.h>
#include <util/delay.h>

uint16_t count = 0;
uint16_t compare = 0;

uint8_t shift = 1;
uint8_t edit_mode = 1;
uint8_t pas_mode=1;

/* ---- BLINK ---- */
uint16_t blink_counter = 0;
uint8_t blink_state = 0;

/* ---- BUTTON LOCKS ---- */
uint8_t shift_lock = 0;
uint8_t inc_lock = 0;
uint8_t dec_lock = 0;
uint8_t enter_lock = 0;

/* ---------- SEGMENT ---------- */

void seg_off() {
  PORTD |= 0xFC;
  PORTB |= (1<<PB0);
}

void show_underscore() {
  seg_off();
  PORTD &= ~(1<<PD5);
}

void show(uint8_t num) {

  seg_off();

  switch(num) {
    case 0: PORTD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7)); break;
    case 1: PORTD &= ~((1<<PD3)|(1<<PD4)); break;
    case 2: PORTD &= ~((1<<PD2)|(1<<PD3)|(1<<PD5)|(1<<PD6)); PORTB &= ~(1<<PB0); break;
    case 3: PORTD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)); PORTB &= ~(1<<PB0); break;
    case 4: PORTD &= ~((1<<PD3)|(1<<PD4)|(1<<PD7)); PORTB &= ~(1<<PB0); break;
    case 5: PORTD &= ~((1<<PD2)|(1<<PD4)|(1<<PD5)|(1<<PD7)); PORTB &= ~(1<<PB0); break;
    case 6: PORTD &= ~((1<<PD2)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7)); PORTB &= ~(1<<PB0); break;
    case 7: PORTD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)); break;
    case 8: PORTD &= ~(0xFC); PORTB &= ~(1<<PB0); break;
    case 9: PORTD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD7)); PORTB &= ~(1<<PB0); break;
  }
}

void digit_off() {
  PORTB &= ~((1<<PB1)|(1<<PB2)|(1<<PB3)|(1<<PB4));
}

/* ---------- DISPLAY WITH CURSOR ---------- */

void display_cursor(uint16_t num) {

  uint8_t d1 = num / 1000;
  uint8_t d2 = (num / 100) % 10;
  uint8_t d3 = (num / 10) % 10;
  uint8_t d4 = num % 10;

  digit_off();
  if(shift==1 && blink_state) show_underscore(); else show(d4);
  PORTB |= (1<<PB1); _delay_ms(2);

  digit_off();
  if(shift==2 && blink_state) show_underscore(); else show(d3);
  PORTB |= (1<<PB2); _delay_ms(2);

  digit_off();
  if(shift==3 && blink_state) show_underscore(); else show(d2);
  PORTB |= (1<<PB3); _delay_ms(2);

  digit_off();
  if(shift==4 && blink_state) show_underscore(); else show(d1);
  PORTB |= (1<<PB4); _delay_ms(2);
}

/* ---------- NORMAL DISPLAY ---------- */

void display(uint16_t num) {

  uint8_t d1 = num / 1000;
  uint8_t d2 = (num / 100) % 10;
  uint8_t d3 = (num / 10) % 10;
  uint8_t d4 = num % 10;

  digit_off(); show(d4); PORTB |= (1<<PB1); _delay_ms(2);
  digit_off(); show(d3); PORTB |= (1<<PB2); _delay_ms(2);
  digit_off(); show(d2); PORTB |= (1<<PB3); _delay_ms(2);
  digit_off(); show(d1); PORTB |= (1<<PB4); _delay_ms(2);
}

/* ---------- BLINK WHEN MATCH ---------- */

void blink() {
  while(1) {
    for(int i=0;i<100;i++) display(compare);
    digit_off();
    _delay_ms(200);
  }
}
void pass()
{

  while(1) {

    /* -------- DISPLAY BASED ON MODE -------- */
    if(pas_mode) {
      display_cursor(compare);
    } else {
      display(compare);
    }

    /* -------- BLINK ONLY IN EDIT MODE -------- */
    if(edit_mode) {
      blink_counter++;
      if(blink_counter > 60) {
        blink_counter = 0;
        blink_state ^= 1;
      }
    } else {
      blink_state = 0;
    }

    /* SPLIT DIGITS */
    uint8_t d1 = compare / 1000;
    uint8_t d2 = (compare/ 100) % 10;
    uint8_t d3 = (compare / 10) % 10;
    uint8_t d4 = compare % 10;

    /* -------- SHIFT -------- */
    if(!(PINC & (1<<PC0))) {
      if(!shift_lock && edit_mode) {
        shift++;
        if(shift > 4) shift = 1;
        shift_lock = 1;
      }
    } else {
      shift_lock = 0;
    }

    /* -------- INCREMENT -------- */
    if(!(PINC & (1<<PC1))) {
      if(!inc_lock && edit_mode) {
        if(shift==1) d4 = (d4+1)%10;
        if(shift==2) d3 = (d3+1)%10;
        if(shift==3) d2 = (d2+1)%10;
        if(shift==4) d1 = (d1+1)%10;

        compare = d1*1000 + d2*100 + d3*10 + d4;
        inc_lock = 1;
      }
    } else {
      inc_lock = 0;
    }

    /* -------- DECREMENT -------- */
    if(!(PINC & (1<<PC2))) {
      if(!dec_lock && edit_mode) {
        if(shift==1) d4 = (d4==0)?9:d4-1;
        if(shift==2) d3 = (d3==0)?9:d3-1;
        if(shift==3) d2 = (d2==0)?9:d2-1;
        if(shift==4) d1 = (d1==0)?9:d1-1;

        compare = d1*1000 + d2*100 + d3*10 + d4;
        dec_lock = 1;
      }
    } else {
      dec_lock = 0;
    }

    /* -------- ENTER -------- */
    if(!(PINC & (1<<PC3))) {
      if(!enter_lock) {
        enter_lock = 1;
        break;
      }
    } else {
      enter_lock = 0;
    }

    
  }
}

/* ---------- MAIN ---------- */

int main(void) {

  DDRD |= 0xFC;
  DDRB |= 0x1F;

  DDRC &= ~((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3));
  PORTC |= (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);
  pass();
 shift = 1;
  while(1) {

    /* -------- DISPLAY BASED ON MODE -------- */
    if(edit_mode) {
      display_cursor(count);
    } else {
      display(count);
    }

    /* -------- BLINK ONLY IN EDIT MODE -------- */
    if(edit_mode) {
      blink_counter++;
      if(blink_counter > 60) {
        blink_counter = 0;
        blink_state ^= 1;
      }
    } else {
      blink_state = 0;
    }

    /* SPLIT DIGITS */
    uint8_t d1 = count / 1000;
    uint8_t d2 = (count / 100) % 10;
    uint8_t d3 = (count / 10) % 10;
    uint8_t d4 = count % 10;

    /* -------- SHIFT -------- */
    if(!(PINC & (1<<PC0))) {
      if(!shift_lock && edit_mode) {
        shift++;
        if(shift > 4) shift = 1;
        shift_lock = 1;
      }
    } else {
      shift_lock = 0;
    }

    /* -------- INCREMENT -------- */
    if(!(PINC & (1<<PC1))) {
      if(!inc_lock && edit_mode) {
        if(shift==1) d4 = (d4+1)%10;
        if(shift==2) d3 = (d3+1)%10;
        if(shift==3) d2 = (d2+1)%10;
        if(shift==4) d1 = (d1+1)%10;

        count = d1*1000 + d2*100 + d3*10 + d4;
        inc_lock = 1;
      }
    } else {
      inc_lock = 0;
    }

    /* -------- DECREMENT -------- */
    if(!(PINC & (1<<PC2))) {
      if(!dec_lock && edit_mode) {
        if(shift==1) d4 = (d4==0)?9:d4-1;
        if(shift==2) d3 = (d3==0)?9:d3-1;
        if(shift==3) d2 = (d2==0)?9:d2-1;
        if(shift==4) d1 = (d1==0)?9:d1-1;

        count = d1*1000 + d2*100 + d3*10 + d4;
        dec_lock = 1;
      }
    } else {
      dec_lock = 0;
    }

    /* -------- ENTER -------- */
    if(!(PINC & (1<<PC3))) {
      if(!enter_lock) {
        edit_mode = 0;
        enter_lock = 1;
      }
    } else {
      enter_lock = 0;
    }

    /* -------- RUN MODE -------- */
    if(!edit_mode) {

      for(int i=0;i<200;i++) display(count);

      if(count == compare) {
        blink();
      }

      count++;
      if(count > 9999) count = 0;
    }
  }
}
