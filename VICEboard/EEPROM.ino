

/*
 * 
 * 
 * TODO: 
 */

#include "EEPROM.h"

struct header_s
{
  unsigned short  version;
  unsigned short  crc16;
};
struct config_s
{
  unsigned short  cpu_freq;
  unsigned short  led_delay;
  unsigned short  led_max;
  unsigned short  timer_bt;
  unsigned short  timer_nobt;
  bool            combokeys;
  int             ctrl_f1;
  int             ctrl_f3;
  int             ctrl_f5;
  int             ctrl_f7;
} systemconfig;

#define DATA_VERSION 1

unsigned short EEPROM_crc16()
{
  int address = sizeof(struct header_s);
  unsigned short crc = 0;
  char i;

  while( address < (sizeof(struct header_s) + sizeof(struct config_s)) )
  {
    crc ^= (unsigned short)(EEPROM.readByte( address++ )) << 8;
    for( i = 0; i < 8; ++i )
    {
      crc = crc << 1 ^ ((crc & 0x8000) ? 0x1021 : 0);
    }
  }
  return crc;
}

void EEPROM_setup()
{
  struct header_s header;
  unsigned short crc16;

  systemconfig.cpu_freq   = 80;
  systemconfig.led_delay  = 100;
  systemconfig.led_max    = 80;
  systemconfig.timer_bt   = 300;
  systemconfig.timer_nobt = 60;
  systemconfig.combokeys  = true;
  systemconfig.ctrl_f1    = KEY_F9;
  systemconfig.ctrl_f3    = KEY_F10;
  systemconfig.ctrl_f5    = KEY_F11;
  systemconfig.ctrl_f7    = KEY_F12;

  if( !EEPROM.begin( sizeof(struct header_s) + sizeof(struct config_s) ) )
  {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  crc16 = EEPROM_crc16();
  EEPROM.get( 0, header );
  Serial.printf( "EEPROM version: %u\nEEPROM crc save: %04x\nEEPROM crc calc: %04x\n",
                 header.version, header.crc16, crc16 );
  if( (header.version == DATA_VERSION) &&
      (header.crc16   == crc16) )
  {
    Serial.print( "Loading EEPROM..." );
    EEPROM.get( sizeof(struct header_s), systemconfig );
    Serial.println( " done.");
  }
  else
  {
    Serial.printf( "EEPROM seems invalid: %04:%04x %04x:%04x\n",
                   header.version, DATA_VERSION );
  }
}

void EEPROM_save()
{
  struct header_s header;
  EEPROM.put( sizeof(struct header_s), systemconfig );
  header.version = DATA_VERSION;
  header.crc16   = EEPROM_crc16();
  EEPROM.put( 0, header );
  EEPROM.commit();
}
