#include "midi_usb.h"
#include <avr/io.h>

//this example simply echos midi back, for now

void fallthrough_callback(MidiDevice * device, uint16_t cnt, uint8_t byte0, uint8_t byte1, uint8_t byte2){
   midi_send_data(device, cnt, byte0, byte1, byte2);
}

int main(void) {
   MidiDevice usb_midi;

   midi_usb_init(&usb_midi);
   midi_register_fallthrough_callback(&usb_midi, fallthrough_callback);

   //left adjust, use AVCC as ref
   ADMUX = _BV(ADLAR) | _BV(REFS0);
   ADCSRA = _BV(ADEN);
   ADCSRA |= _BV(ADSC);
   ADCSRA |= _BV(ADPS2) | _BV(ADPS1);
   uint16_t filtered = 0;
   uint8_t last = 0;
   while(1){
      midi_device_process(&usb_midi);
      //new adc val
      if (ADCSRA & _BV(ADIF)) {
         uint8_t val = ADCH;
         filtered = (filtered + val) >> 1;
         val = filtered >> 1;
         if (last != val) {
            last = val;
            midi_send_cc(&usb_midi, 0, 0, val);
         }
         //start a new conversion
         ADCSRA |= _BV(ADIF);
         ADCSRA |= _BV(ADSC);
      }
   }

   return 0;
}
