

#include <BleKeyboard.h>

#define ARRAY_ELEMENTS(array) (sizeof(array) / sizeof((array)[0]))
#define DEBUG 1
#define DEBOUNCE_THRESHOLD  4

/*
 *              |         dc01         ||         dc00         |
 *              |3  6  5  4  7  2  1  0||0  6  5  4  3  2  1  7|
 *   o--x--o--x--o--o--o--o--o--o--o--o--o--o--o--o--o--o--o--o
 *   G     2    12 13 14 15 16 17 18 23 21 22 19 25 26 27 32 33
 *                                   
 *   G     2    12 13 14 15 16 17 18 19 21 22 22 24 25 26 32 33
 */
const unsigned char DC00_gpios[8] = { 21, 32, 27, 26, 25, 19, 22, 33 };
const unsigned char DC01_gpios[8] = { 23, 18, 17, 12, 15, 14, 13, 16 };
const unsigned char LED_gpios[2]  = { 0, 4 };
const unsigned char RESTORE_GPIO  = 2;
const int           RESTORE_id    = ARRAY_ELEMENTS(DC00_gpios) * ARRAY_ELEMENTS(DC01_gpios);

BleKeyboard bleKeyboard( "C64 Bluetooth Keyboard", "SvOlli", 100 );


void setup()
{
  int i;
  Serial.begin( 115200 );
  bleKeyboard.begin();
  setCpuFrequencyMhz( 240 ); // quick setup

  LED_setup();
  Keyboard_setup();

  while( !Serial )
  {
    /* wait for USB to connect */
  }
  Serial.print( F("\n    **** VICEboard Bluetooth Keyboard ****\r\n\r\n ESP32 based system    ") );
  Serial.print( ESP.getFreeHeap() );
  Serial.println( F(" heap bytes free\r\n\nREADY.\n") );

  EEPROM_setup();

  for ( i = 0; i < ARRAY_ELEMENTS(DC01_gpios); ++i )
  {
    pinMode( DC00_gpios[i], INPUT_PULLUP );
    pinMode( DC01_gpios[i], INPUT_PULLUP );
  }
  pinMode( RESTORE_GPIO, INPUT_PULLUP );

  MenuSystem_setup();
  Power_setup();
}


void loop()
{
  static int dc00 = 0, dc01 = 0;

  static bool last_connected = false;
  
  if( bleKeyboard.isConnected() != last_connected )
  {
    last_connected = bleKeyboard.isConnected();
    LED_set( 0, bleKeyboard.isConnected() ? 0 : 100 );
    LED_set( 1, bleKeyboard.isConnected() ? 100 : 0 );
    Power_timer();
  }

  for( dc00 = 0; dc00 < ARRAY_ELEMENTS(DC00_gpios); ++dc00 )
  {
    pinMode( DC00_gpios[dc00], OUTPUT );
    digitalWrite( DC00_gpios[dc00], LOW );
    for( dc01 = 0; dc01 < ARRAY_ELEMENTS(DC01_gpios); ++dc01 )
    {
      int id = dc00 * ARRAY_ELEMENTS(DC00_gpios) + dc01;
      Keyboard_checkkey( id, DC01_gpios[dc01] );
    }
    digitalWrite( DC00_gpios[dc00], HIGH );
    pinMode( DC00_gpios[dc00], INPUT_PULLUP );
  }
  Keyboard_checkkey( RESTORE_id, RESTORE_GPIO );

  LED_loop();
  Power_loop();
  MenuSystem_loop();
}
