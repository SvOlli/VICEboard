
/*
 * EEPROM.ino
 * ==========
 * 
 * TODO: 
 */

#include "EEPROM.h"

struct header_s
{
  uint16_t  version;
  uint16_t  crc16_gpio;
  uint16_t  crc16_config;
  uint16_t  size_config;
};
struct gpioconfig_s
{
  uint8_t   dc00[8];
  uint8_t   dc01[8];
  uint8_t   leds[2];
  uint8_t   restore;
  uint8_t   extra;
} gpios;
struct config_s
{
  uint16_t  cpu_freq;
  uint16_t  timer_bt;
  uint16_t  timer_nobt;
  uint16_t  led_delay;
  uint16_t  led_max;
  bool      use_combokeys;
  uint16_t  ctrl_f1;
  uint16_t  ctrl_f3;
  uint16_t  ctrl_f5;
  uint16_t  ctrl_f7;
  uint16_t  debounce_threshold;
  uint32_t  bluetooth_pin;
  bool      use_custom_keymap;
  uint8_t   custom_keymap[65];
  /*
     no elements will be changed or removed,
     new elements will always be appended
     this allows for partial config load on updates
  */
} sysconfig;

#define DATA_VERSION 1

#define HEADER_SIZE        (sizeof(struct header_s))
#define GPIOCONFIG_SIZE    (sizeof(struct gpioconfig_s))
#define SYSCONFIG_SIZE     (sizeof(struct config_s))
#define HEADER_ADDRESS     (0)
#define GPIOCONFIG_ADDRESS (0x10)
#define SYSCONFIG_ADDRESS  (GPIOCONFIG_ADDRESS + GPIOCONFIG_SIZE)
#define EEPROM_END         (SYSCONFIG_ADDRESS + SYSCONFIG_SIZE)

uint16_t EEPROM_crc16( int start, int size )
{
  int address = start;
  uint16_t crc = size;
  char i;

  while( address < size )
  {
    crc ^= (uint16_t)(EEPROM.readByte( address++ )) << 8;
    for( i = 0; i < 8; ++i )
    {
      crc = crc << 1 ^ ((crc & 0x8000) ? 0x1021 : 0);
    }
  }
  return crc;
}


void EEPROM_dump()
{
  const int width = 16;
  int address = 0;

  for( int address = 0; address < EEPROM_END; ++address )
  {
    if( address % width == 0 )
    {
      Serial.printf( "%04X: ", address );
    }
    Serial.printf( "%02X", EEPROM.read( address ) );
    if( address % width == width - 1 )
    {
      Serial.println( "." );
    }
    else
    {
      Serial.print( " " );
    }
  }
  Serial.println( "" );
}


void EEPROM_clean()
{
  for( int address = 0; address < EEPROM_END; ++address )
  {
    EEPROM.write( address, 0xFF );
  }
  EEPROM.commit();
}


void EEPROM_read( int address, int size, void *data )
{
  uint8_t *d = (uint8_t*)data;
  while( size-- > 0 )
  {
    *(d++) = EEPROM.read( address++ );
  }
}


void EEPROM_write( int address, int size, void *data )
{
  uint8_t *d = (uint8_t*)data;
  while( size-- > 0 )
  {
    EEPROM.write( address++, *(d++) );
  }
  EEPROM.commit();
}


void EEPROM_setup()
{
  struct header_s header;
  uint16_t crc16_gpio;
  uint16_t crc16_config;

  memset( &sysconfig, 0, sizeof(sysconfig) );
  sysconfig.cpu_freq           = 80;
  sysconfig.timer_bt           = 300;
  sysconfig.timer_nobt         = 60;
  sysconfig.led_delay          = 20;
  sysconfig.led_max            = 80;
  sysconfig.use_combokeys      = true;
  sysconfig.ctrl_f1            = KEY_F9;
  sysconfig.ctrl_f3            = KEY_F10;
  sysconfig.ctrl_f5            = KEY_F11;
  sysconfig.ctrl_f7            = KEY_F12;
  sysconfig.debounce_threshold = 5;
  sysconfig.bluetooth_pin      = 64738;
  sysconfig.use_custom_keymap  = false;

  if( !EEPROM.begin( EEPROM_END ) )
  {
    Serial.println( F("Failed to initialise EEPROM") );
    Serial.println( F("Restarting...") );
    delay(1000);
    ESP.restart();
  }
  EEPROM_read( HEADER_ADDRESS, HEADER_SIZE, &header );

  crc16_gpio   = EEPROM_crc16( GPIOCONFIG_ADDRESS, GPIOCONFIG_SIZE );
  crc16_config = EEPROM_crc16( SYSCONFIG_ADDRESS,  SYSCONFIG_SIZE );
  Serial.printf(     "           EEPROM CALCULATED"
                 "\r\nVersion:    %04X   %04X"
                 "\r\nCRC GPIO:   %04X   %04X"
                 "\r\nCRC Config: %04X   %04X\r\n\n",
                 header.version, DATA_VERSION,
                 header.crc16_gpio, crc16_gpio,
                 header.crc16_config, crc16_config );
  if( header.crc16_gpio == crc16_gpio )
  {
    Serial.print( F("Loading GPIO Config EEPROM...") );
    EEPROM_read( GPIOCONFIG_ADDRESS, GPIOCONFIG_SIZE, &gpios );
    Serial.println( F(" done.") );
  }
  if( (header.version      == DATA_VERSION) &&
      (header.crc16_config == crc16_config ) )
  {
    Serial.print( F("Loading System Config EEPROM...") );
    EEPROM_read( SYSCONFIG_ADDRESS, SYSCONFIG_SIZE, &sysconfig );
    Serial.println( F(" done.") );
  }
}


void EEPROM_savegpios()
{
  int i;
  struct header_s header;
  EEPROM_read( HEADER_ADDRESS, HEADER_SIZE, &header );

  for( i = 0; i < 8; ++i )
  {
    gpios.dc00[i] = DC00_gpios[i];
    gpios.dc01[i] = DC01_gpios[i];
  }
  gpios.leds[0] = LED_gpios[0];
  gpios.leds[1] = LED_gpios[1];
  gpios.restore = RESTORE_GPIO;
  gpios.extra   = 5;

  EEPROM_write( GPIOCONFIG_ADDRESS, GPIOCONFIG_SIZE, &gpios );
  header.crc16_gpio = EEPROM_crc16( GPIOCONFIG_ADDRESS, GPIOCONFIG_SIZE );
  EEPROM_write( HEADER_ADDRESS, HEADER_SIZE, &header );
}


void EEPROM_saveconfig()
{
  struct header_s header;
  EEPROM_read( HEADER_ADDRESS, HEADER_SIZE, &header );
  EEPROM_write( SYSCONFIG_ADDRESS, SYSCONFIG_SIZE, &sysconfig );
  header.version      = DATA_VERSION;
  header.crc16_config = EEPROM_crc16( SYSCONFIG_ADDRESS, SYSCONFIG_SIZE );
  header.size_config  = sizeof( sysconfig );
  EEPROM_write( HEADER_ADDRESS, HEADER_SIZE, &header );
}
