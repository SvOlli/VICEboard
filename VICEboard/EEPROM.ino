

/*
 * 
 * 
 * TODO: 
 */

#include "EEPROM.h"

struct header_s
{
  unsigned short  version;
  unsigned short  crc16_gpio;
  unsigned short  crc16_config;
};
struct gpioconfig_s
{
  unsigned short  dc00[8];
  unsigned short  dc01[8];
  unsigned short  leds[2];
  unsigned short  restore;
  unsigned short  extra;
} gpios;
struct config_s
{
  unsigned short  cpu_freq;
  unsigned short  timer_bt;
  unsigned short  timer_nobt;
  unsigned short  led_delay;
  unsigned short  led_max;
  bool            combokeys;
  unsigned short  ctrl_f1;
  unsigned short  ctrl_f3;
  unsigned short  ctrl_f5;
  unsigned short  ctrl_f7;
} systemconfig;

#define DATA_VERSION 1

const int header_size = sizeof(struct header_s);
const int gpioconfig_size = sizeof(struct gpioconfig_s);
const int sysconfig_size = sizeof(struct config_s);
const int header_address = 0;
const int gpioconfig_address = 16;
const int sysconfig_address = gpioconfig_address + gpioconfig_size;
const int eeprom_end = sysconfig_address + sysconfig_size;

unsigned short EEPROM_crc16( int start, int size )
{
  int address = start;
  unsigned short crc = size;
  char i;

  while( address < size )
  {
    crc ^= (unsigned short)(EEPROM.readByte( address++ )) << 8;
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

  for( int address = 0; address < eeprom_end; ++address )
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
  for( int address = 0; address < eeprom_end; ++address )
  {
    EEPROM.write( address, 0 );
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
  unsigned short crc16_gpio;
  unsigned short crc16_config;

  systemconfig.cpu_freq   = 80;
  systemconfig.timer_bt   = 300;
  systemconfig.timer_nobt = 60;
  systemconfig.led_delay  = 50;
  systemconfig.led_max    = 80;
  systemconfig.combokeys  = true;
  systemconfig.ctrl_f1    = KEY_F9;
  systemconfig.ctrl_f3    = KEY_F10;
  systemconfig.ctrl_f5    = KEY_F11;
  systemconfig.ctrl_f7    = KEY_F12;

  if( !EEPROM.begin( eeprom_end ) )
  {
    Serial.println( F("Failed to initialise EEPROM") );
    Serial.println( F("Restarting...") );
    delay(1000);
    ESP.restart();
  }
  EEPROM_read( header_address, header_size, &header );

  crc16_gpio   = EEPROM_crc16( gpioconfig_address, gpioconfig_size );
  crc16_config = EEPROM_crc16( sysconfig_address,  sysconfig_size );
  Serial.printf(     "           EEPROM CALCULATED"
                 "\r\nVersion:    %04x   %04x"
                 "\r\nCRC GPIO:   %04x   %04x"
                 "\r\nCRC Config: %04x   %04x\r\n\n",
                 header.version, DATA_VERSION,
                 header.crc16_gpio, crc16_gpio,
                 header.crc16_config, crc16_config );
  if( header.crc16_gpio == crc16_gpio )
  {
    Serial.print( F("Loading GPIO Config EEPROM...") );
    EEPROM_read( gpioconfig_address, gpioconfig_size, &gpios );
    Serial.println( F(" done.") );
  }
  if( (header.version      == DATA_VERSION) &&
      (header.crc16_config == crc16_config ) )
  {
    Serial.print( F("Loading System Config EEPROM...") );
    EEPROM_read( sysconfig_address, sysconfig_size, &systemconfig );
    Serial.println( F(" done.") );
  }
}


void EEPROM_savegpios()
{
  int i;
  struct header_s header;
  EEPROM_read( header_address, header_size, &header );

  for( i = 0; i < 8; ++i )
  {
    gpios.dc00[i] = DC00_gpios[i];
    gpios.dc01[i] = DC01_gpios[i];
  }
  gpios.leds[0] = LED_gpios[0];
  gpios.leds[1] = LED_gpios[1];
  gpios.restore = RESTORE_GPIO;
  gpios.extra   = 5;

  EEPROM_write( gpioconfig_address, gpioconfig_size, &gpios );
  header.crc16_gpio = EEPROM_crc16( gpioconfig_address, gpioconfig_size );
  EEPROM_write( header_address, header_size, &header );
}


void EEPROM_saveconfig()
{
  struct header_s header;
  EEPROM_read( header_address, header_size, &header );
  EEPROM_write( sysconfig_address, sysconfig_size, &systemconfig );
  header.version      = DATA_VERSION;
  header.crc16_config = EEPROM_crc16( sysconfig_address, sysconfig_size );
  EEPROM_write( header_address, header_size, &header );
}
