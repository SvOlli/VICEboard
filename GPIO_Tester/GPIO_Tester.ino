
/*
 * ==========
 *  1 pin  Restore
 * 16 pins DC00 / DC01
 *  2 pins LEDs (power/bt)
 * -------------
 * 19 pins Total
 *   G     5    12 13 14 15 16 17 18 23 21 22 19 25 26 27 32 33
 */

#define DEBOUNCE_COUNTER 100
#define NUM_GPIOS 20
const unsigned char GPIOs[NUM_GPIOS] = { 0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33 };
const unsigned char WAKEUP[11] = { 0, 2, 4, 12, 14, 15, 25, 26, 27, 32, 33 };
signed char pins[20];
signed char debounce[20];


void setup()
{
  Serial.begin( 115200 );
  for( int i = 0; i < NUM_GPIOS; ++i )
  {
    int gpio = GPIOs[i];
    pinMode( gpio, INPUT_PULLUP );
    debounce[i] = DEBOUNCE_COUNTER;
    pins[i]     = -1;
  }
  Serial.println( "GPIO tester started" );
}


void results()
{
  Serial.printf(
    "const unsigned char DC00_gpios[8] = { %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d };\n"
    "const unsigned char DC01_gpios[8] = { %2d, %2d, %2d, %2d, %2d, %2d, %2d, %2d };\n"
    "const unsigned char LED_gpios[2]  = { %2d, %2d };\n"
    "#define             RESTORE_GPIO    %2d\n",
    pins[9+0], pins[9+6], pins[9+5], pins[9+4], pins[9+3], pins[9+2], pins[9+1], pins[9+7],
    pins[1+7], pins[1+6], pins[1+5], pins[1+0], pins[1+3], pins[1+2], pins[1+1], pins[1+4],
    pins[17+0], pins[17+1],
    pins[0]
  );
}


int find_gpio()
{
  for(;;)
  {
    for( int i = 0; i < 20; ++i )
    {
      int gpio = GPIOs[i];
      bool state = (digitalRead( gpio ) == LOW);
      if ( !state )
      {
        if( debounce[i] < 0 )
        {
          //Serial.printf( "GPIO %d HIGH\n", gpio );
          debounce[i] = DEBOUNCE_COUNTER;
        }
        continue;
      }
      --debounce[i];
      if( debounce[i] == 0 )
      {
        //Serial.printf( "GPIO %d LOW\n", gpio );
        bool found = false;
        for( int j = 0; j < 19; ++j )
        {
          if( pins[j] == gpio )
          {
            found = true;
          }
        }
        if( !found )
        {
          return gpio;
        }
      }
      else if( debounce[i] < 0 )
      {
        debounce[i] = -1;
      }
    }
  }
}


void loop()
{
  for( int i = 0; i < 19; ++i )
  {
    switch( i )
    {
      case 0:
        Serial.println( F("GPIO for RESTORE KEY (the free standing one)") );
        break;
      case 1:
        Serial.println( F("GPIO for pin 1 (closest to RESTORE key)") );
        break;
      case 17:
        Serial.println( F("GPIO for power LED (GPIO 0 recommended)\n") );
        break;
      case 18:
        Serial.println( F("GPIO for connected LED") );
        break;
      default:
        Serial.print( F("GPIO for pin ") );
        Serial.println( i );
    }
    int gpio = find_gpio();
    if( i == 0 )
    {
      bool found = false;
      for( int j = 0; j < 11; ++j )
      {
        if( gpio == WAKEUP[j] )
        {
          found = true;
        }
      }
      if( found )
      {
        Serial.println( F("Wake up compatible. Good.") );
      }
      else
      {
        Serial.println( F("The GPIO is not known to support wake from deep sleep.\nKnown to work are:") );
        for( int j = 0; j < 11; ++j )
        {
          Serial.print( F(" ") );
          Serial.print( WAKEUP[j] );
        }
        Serial.println( F("") );
        esp_deep_sleep_start();
      }
    }
    Serial.print( F("  using GPIO ") );
    Serial.println( gpio );
    pins[i] = gpio;
  }
  Serial.println( F("") );
  results();
  Serial.println( F("") );
  Serial.println( F("All done. Press reset button to start again") );
  esp_deep_sleep_start();
}
