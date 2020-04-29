
/*
 * 
 * 
 * TODO: 
 * clock down
 * turn off one CPU?
 * deep sleep
 * 
 * NOTE:
 * Only RTC IO can be used as a source for external wake
 * source. They are pins: 0,2,4,12-15,25-27,32-39.
 */

#include "esp32-hal-cpu.h"

unsigned long Power_off_millis= 0xFFFFFFFF;



void Power_timer( unsigned short time )
{
  Power_off_millis = millis() + 1000 * time;
  /* overflow check */
  if( millis() > Power_off_millis )
  {
    Power_off_millis = 0xFFFFFFFF;
  }
}

void Power_timer()
{
  Power_timer( bleKeyboard.isConnected() ? systemconfig.timer_bt : systemconfig.timer_nobt );
}


void Power_setup()
{
  setCpuFrequencyMhz( systemconfig.cpu_freq ); //Set CPU clock to 80MHz fo example
  Power_timer();
}


#if 0
void Power_deepsleep()
{
  char c = 0;
  if( Serial.available() )
  {
    c = toupper(Serial.read());
  }
  switch( c )
  {
    case '9':
      Serial.println( "0" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_0, 0 );
      break;
    case '2':
      Serial.println( "2" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_2, 0 );
      break;
    case '4':
      Serial.println( "4" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_4, 0 );
      break;
    case '5':
      Serial.println( "5" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_5, 0 );
      break;
    case 'A':
      Serial.println( "12" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_12, 0 );
      break;
    case 'B':
      Serial.println( "13" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_13, 0 );
      break;
    case 'C':
      Serial.println( "14" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_14, 0 );
      break;
    case 'D':
      Serial.println( "15" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_15, 0 );
      break;
    case 'E':
      Serial.println( "16" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_16, 0 );
      break;
    case 'F':
      Serial.println( "17" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_17, 0 );
      break;
    case 'G':
      Serial.println( "18" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_18, 0 );
      break;
    case 'H':
      Serial.println( "23" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_23, 0 );
      break;
    case 'I':
      Serial.println( "21" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_21, 0 );
      break;
    case 'J':
      Serial.println( "22" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_22, 0 );
      break;
    case 'K':
      Serial.println( "19" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_19, 0 );
      break;
    case 'L':
      Serial.println( "25" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_25, 0 );
      break;
    case 'M':
      Serial.println( "26" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_26, 0 );
      break;
    case 'N':
      Serial.println( "27" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_27, 0 );
      break;
    case 'O':
      Serial.println( "32" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_32, 0 );
      break;
    case 'P':
      Serial.println( "33" );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_33, 0 );
      break;
    default:
      pinMode( 2, INPUT_PULLUP );
      esp_sleep_enable_ext0_wakeup( GPIO_NUM_2, 0 );
      break;
  }
  delay(1000);
  esp_deep_sleep_start();
}
#endif


  /* hard stuff to add value of RESTORE_GPIO to a label */
#define GPIO_NUM__IFY(x) GPIO_NUM_ ## x
#define GPIO_NUM_IFY(x) GPIO_NUM__IFY(x)

void Power_loop()
{
  if( millis() > Power_off_millis )
  {
    LED_off();
    esp_sleep_enable_ext0_wakeup( GPIO_NUM_IFY(RESTORE_GPIO), 0 );
    Serial.println( "Zzzzzzz...." );
    esp_deep_sleep_start();
  }
}

#undef GPIO_NUM_IFY
#undef GPIO_NUM__IFY
