
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

#include <esp32-hal-cpu.h>

unsigned long Power_off_millis = 0xFFFFFFFF;



void Power_timer( unsigned short time )
{
  Power_off_millis = millis() + 1000 * time;
  /* overflow check */
  if( millis() > Power_off_millis )
  {
    /* known problem: pressing a key at the right time after 50 days non-stop usage, the auto-off
       feature will not work until the timer has wrapped over. very unlikely. */
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


void Power_off()
{
  LED_off();
  esp_sleep_enable_ext0_wakeup( (gpio_num_t)RESTORE_GPIO, 0 );
  Serial.println( F("Zzzzzzz....") );
  esp_deep_sleep_start();
}


void Power_loop()
{
  if( millis() > Power_off_millis )
  {
    Power_off();
  }
}
