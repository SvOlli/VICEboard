
/*
 * 
 * TODO:
 */

typedef enum {
  SERIALUI_MENU_MAIN = 0,
  SERIALUI_MENU_FREQ,
  SERIALUI_MENU_CTRL_F1,
  SERIALUI_MENU_CTRL_F3,
  SERIALUI_MENU_CTRL_F5,
  SERIALUI_MENU_CTRL_F7,
  SERIALUI_MENU_END
} SerialUI_menuid_t;

SerialUI_menuid_t SerialUI_menuid = SERIALUI_MENU_MAIN;
bool SerialUI_menu_printed = false;

void SerialUI_setup()
{
  
}

void SerialUI_menu_main()
{
  Serial.printf(
    "Main Menu\n"
    "=========\n"
    "[F] set cpu frequency: %d MHz (real: %d MHz)\n"
    "[L] change LED parameters: max=%d, delay=%d\n"
    "[K] toggle combo keys: %s\n"
    "[1] set ctrl+f1\n"
    "[3] set ctrl+f3\n"
    "[5] set ctrl+f5\n"
    "[7] set ctrl+f7\n"
    "[E] write settings to EEPROM\n"
    "[Z] deep sleep\n"
    "[R] reboot\n",
    systemconfig.cpu_freq, getCpuFrequencyMhz(),
    systemconfig.led_max, systemconfig.led_delay,
    systemconfig.combokeys ? "on" : "off"
  );
}

void SerialUI_menu_freq()
{
  Serial.printf(
    "Select CPU Freqency\n"
    "===================\n"
    "[1] 80 MHz\n"
    "[2] 160 MHz\n"
    "[3] 240 MHz\n"
  );
}

void SerialUI_menu_ctrl_f( int fkey )
{
  
}

void SerialUI_loop()
{
  if( !SerialUI_menu_printed )
  {
    Serial.println( "\n" );
    switch( SerialUI_menuid )
    {
      case SERIALUI_MENU_MAIN:
        SerialUI_menu_main();
        break;
      case SERIALUI_MENU_FREQ:
        SerialUI_menu_freq();
        break;
      case SERIALUI_MENU_CTRL_F1:
        SerialUI_menu_ctrl_f(1);
        break;
      case SERIALUI_MENU_CTRL_F3:
        SerialUI_menu_ctrl_f(3);
        break;
      case SERIALUI_MENU_CTRL_F5:
        SerialUI_menu_ctrl_f(5);
        break;
      case SERIALUI_MENU_CTRL_F7:
        SerialUI_menu_ctrl_f(7);
        break;
      default:
        break;
    }
    SerialUI_menu_printed = true;
  }
  if( !Serial.available() )
  {
    return;
  }
  char c = toupper(Serial.read());
  switch( SerialUI_menuid )
  {
    case SERIALUI_MENU_MAIN:
      switch( c )
      {
        case 'F':
          SerialUI_menuid = SERIALUI_MENU_FREQ;
          SerialUI_menu_printed = false;
          break;
        case 'K':
          systemconfig.combokeys = !systemconfig.combokeys;
          SerialUI_menu_printed = false;
          break;
        case '1':
          SerialUI_menuid = SERIALUI_MENU_CTRL_F1;
          SerialUI_menu_printed = false;
          break;
        case '3':
          SerialUI_menuid = SERIALUI_MENU_CTRL_F3;
          SerialUI_menu_printed = false;
          break;
        case '5':
          SerialUI_menuid = SERIALUI_MENU_CTRL_F5;
          SerialUI_menu_printed = false;
          break;
        case '7':
          SerialUI_menuid = SERIALUI_MENU_CTRL_F7;
          SerialUI_menu_printed = false;
          break;
        case 'E':
          EEPROM_save();
          Serial.println( "Configuration saved." );
          break;
        case 'L':
          LED_set( 1, 0 );
          break;
        case 'R':
          Serial.println( "Rebooting..." );
          ESP.restart();
          break;
        default:
          break;
      }
      break;
    default:
      SerialUI_menuid = SERIALUI_MENU_MAIN;
      SerialUI_menu_printed = false;
      break;
  }
}
