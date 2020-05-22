
/*
 * MenuSystem.ino
 * ==============
 *
 * TODO:
 * - map all data
 */


typedef enum
{
  END = 0,
  TEXT,
  MENU,
  FUNC,
  BOOL,
  DEC1,
  DEC2,
  DEC4,
  HEX1,
  HEX2,
  HEX4
} menuentrytype_t;


typedef void (*menucallback_t)();
typedef struct {
  uint8_t      *value;
  uint8_t      minimum;
  uint8_t      maximum;
} menuentry_uint8_t;
typedef struct {
  uint16_t     *value;
  uint16_t     minimum;
  uint16_t     maximum;
} menuentry_uint16_t;
typedef struct {
  uint32_t     *value;
  uint32_t     minimum;
  uint32_t     maximum;
} menuentry_uint32_t;


typedef struct menuentry_s menuentry_t;
typedef struct menuentry_s {
  menuentrytype_t         type;
  char                    letter;
  const char              *text;
  union {
    void                  *handle;
    menuentry_t           *submenu;
    void                  (*callback)();
    bool                  *toggle;
    menuentry_uint8_t     *value8;
    menuentry_uint16_t    *value16;
    menuentry_uint32_t    *value32;
  };
};


typedef struct menustack_s {
  menuentry_t    *follow;
  menuentry_t    *entries;
} menustack_t;
bool menuprinted = false;

#define MENUSTACKSIZE 8
menustack_t menustack[MENUSTACKSIZE];


#define MENUENTRY_TEXT( b )         { TEXT, 0, b, 0 },
#define MENUENTRY_MENU( a, b, c )   { MENU, a, b, c },
#define MENUENTRY_FUNC( a, b, c )   { FUNC, a, b, (void*)c },
#define MENUENTRY_BOOL( a, b, c )   { BOOL, a, b, &(c) },
#define MENUENTRY_DEC1( a, b, c )   { DEC1, a, b, &(c) },
#define MENUENTRY_DEC2( a, b, c )   { DEC2, a, b, &(c) },
#define MENUENTRY_DEC4( a, b, c )   { DEC4, a, b, &(c) },
#define MENUENTRY_HEX1( a, b, c )   { HEX1, a, b, &(c) },
#define MENUENTRY_HEX2( a, b, c )   { HEX2, a, b, &(c) },
#define MENUENTRY_HEX4( a, b, c )   { HEX4, a, b, &(c) },
#define MENUENTRY_END               { END,  0, 0, 0 }



void show_config()
{
  int i;
  const uint8_t *Keymap = sysconfig.use_custom_keymap ?
                                sysconfig.custom_keymap :
                                Keymap_VICE_positional;
  Serial.println( "Configuration" );
  Serial.println( "=============" );

  Serial.print( F("RESTORE PIN:") );
  Serial.print( RESTORE_GPIO );
  Serial.println( F(".") );

  Serial.print( F("OUTPUT PINS:") );
  for( i = 0; i < 8; ++i )
  {
    Serial.print( F(" ") );
    Serial.print( DC00_gpios[i] );
  }
  Serial.println( F(".") );
  Serial.print( F("INPUT  PINS:") );
  for( i = 0; i < 8; ++i )
  {
    Serial.print( F(" ") );
    Serial.print( DC01_gpios[i] );
  }
  Serial.println( F(".") );

  Serial.print( F("LED    PINS: ") );
  Serial.print( LED_gpios[0] );
  Serial.print( F(" ") );
  Serial.print( LED_gpios[1] );
  Serial.println( F(".") );

  Serial.print( F("CPU freqency: ") );
  Serial.println( sysconfig.cpu_freq );
  Serial.print( F("Timeout (bluetooth): ") );
  Serial.println( sysconfig.timer_bt );
  Serial.print( F("Timeout (no bluetooth): ") );
  Serial.println( sysconfig.timer_nobt );
  Serial.print( F("LED delay: ") );
  Serial.println( sysconfig.led_delay );
  Serial.print( F("LED brightness: ") );
  Serial.println( sysconfig.led_max );
  Serial.print( F("Combokeys: ") );
  Serial.println( sysconfig.use_combokeys );
  Serial.print( F("CTRL + F1: ") );
  Serial.printf( "$%02X\r\n", sysconfig.ctrl_f1 );
  Serial.print( F("CTRL + F3: ") );
  Serial.printf( "$%02X\r\n", sysconfig.ctrl_f3 );
  Serial.print( F("CTRL + F5: ") );
  Serial.printf( "$%02X\r\n", sysconfig.ctrl_f5 );
  Serial.print( F("CTRL + F7: ") );
  Serial.printf( "$%02X\r\n", sysconfig.ctrl_f7 );
  Serial.print( F("Custom keymap: ") );
  Serial.println( sysconfig.use_custom_keymap );
  for( i = 0; i < 65; ++i )
  {
    Serial.printf( "$%02X%s", Keymap[i], (i % 8) == 7 ? F("\r\n") : F(" ") );
  }
  Serial.println( F(".") );
}


void show_keycode( const char *name, int code )
{
  const char spaces[16] = "               ";
  int l = strlen( name );
  Serial.printf( "%s: $%02X%s", name, code, &spaces[l] );
}

void show_fkeycode( int num, int code )
{
  const char spaces[16] = "               ";
  Serial.printf( "F%d: $%02X%s", num, code, &spaces[(num>9) ? 3 : 2] );
}


void show_keycodes()
{
  int i;

  Serial.println( "List of keycodes" );
  Serial.println( "================" );
  
  for( i = 0; i < 12; ++i )
  {
    if( i == 6 )
    {
      Serial.println( F("") );
    }
    show_fkeycode( i+1, i+0xC2 );
  }
  Serial.println( F("") );
  for( i = 0; i < 12; ++i )
  {
    if( i == 6 )
    {
      Serial.println( F("") );
    }
    show_fkeycode( i+13, i+0xF0 );
  }
  Serial.println( F("") );

  show_keycode( "Left GUI", KEY_LEFT_GUI );
  show_keycode( "Right GUI", KEY_RIGHT_GUI );
  show_keycode( "Tab", KEY_TAB );
  show_keycode( "Backspace", KEY_BACKSPACE );
  show_keycode( "Page Up", KEY_PAGE_UP );
  show_keycode( "Page Down", KEY_PAGE_DOWN );
  Serial.println( F("") );

  show_keycode( "End", KEY_END );
  Serial.println( F("") );
  
}


void check_keymap()
{
  int fails = 0;
  for( int i = 0; i < sizeof(sysconfig.custom_keymap); ++i )
  {
    if( sysconfig.custom_keymap[i] == 0 )
    {
      ++fails;
    }
  }
  if( fails > 0 )
  {
    Serial.print( F("Keymap invalid, number of $00 value(s) found:") );
    Serial.println( fails );
    sysconfig.use_custom_keymap = false;
  }
  else
  {
    Serial.println( F("Keymap valid") );
  }
}


void set_bluetooth_pin()
{
  Serial.println( F("This pin is used when pairing") );
}


void enter_keymap()
{
  int count = 0;
  bool valid = true;
  uint8_t input = 0;

  Serial.println( F("Keyboard configuration must be entered as single line,") );
  Serial.println( F("consisting 65 2-digit hex values, followed by a newline.") );
  Serial.println( F("So it's 130 characters in total, not counting the newline.") );
  Serial.println( F("Using copy'n'paste is strongly recommended.") );

  while( valid && (count < 130) )
  {
    if( Serial.available() )
    {
      char c = toupper( Serial.read() );
      valid = false;

      if( (c >= '0') && (c <= '9') )
      {
        input = input * 16 + (c - '0');
        valid = true;
      }
      if( (c >= 'A') && (c <= 'F') )
      {
        input = input * 16 + (c - 'A' + 10);
        valid = true;
      }
      if( valid && (count++ & 1) )
      {
        sysconfig.custom_keymap[count/2] = input;
        input = 0;
        Serial.write( c );
        if( !(count & 0x0f) )
        {
          Serial.println( F("") );
        }
        else
        {
          Serial.print( F(" ") );
        }
      }
      else
      {
        Serial.write( c );
      }
    }
  }

  Serial.println( F("") );
  Serial.println( valid ? F("Successful set keyboard configuration") 
                        : F("Setting keyboard configuration failed!") );
}


menuentry_t menu_eeprom[] = {
  MENUENTRY_TEXT(      "EEPROM Menu" )
  MENUENTRY_TEXT(      "===========" )
  MENUENTRY_FUNC( 'D', "Raw dump EEPROM", EEPROM_dump )
  MENUENTRY_FUNC( 'N', "Nuke (clear) EEPROM", EEPROM_clean )
  MENUENTRY_FUNC( 'W', "Write system configuration to EEPROM", EEPROM_saveconfig )
  MENUENTRY_FUNC( 'G', "Write GPIO configuration to EEPROM", EEPROM_savegpios )
  MENUENTRY_END
};

menuentry_uint16_t   menu_led_max   = { &(sysconfig.led_max),   0x01, 0xff };
menuentry_uint16_t   menu_led_delay = { &(sysconfig.led_delay), 0x01, 0x63 };

menuentry_t menu_leds[] = {
  MENUENTRY_TEXT(      "LEDs Menu" )
  MENUENTRY_TEXT(      "=========" )
  MENUENTRY_DEC2( 'M', "Maximum brightness", menu_led_max )
  MENUENTRY_DEC2( 'D', "Delay for color change", menu_led_delay )
  MENUENTRY_FUNC( 'S', "Show settings on LEDs", LED_change )
  MENUENTRY_END
};

menuentry_uint16_t   menu_key_ctrl_f1        = { &(sysconfig.ctrl_f1), 0x20, 0xff };
menuentry_uint16_t   menu_key_ctrl_f3        = { &(sysconfig.ctrl_f3), 0x20, 0xff };
menuentry_uint16_t   menu_key_ctrl_f5        = { &(sysconfig.ctrl_f5), 0x20, 0xff };
menuentry_uint16_t   menu_key_ctrl_f7        = { &(sysconfig.ctrl_f7), 0x20, 0xff };
menuentry_uint16_t   menu_debounce_threshold = { &(sysconfig.debounce_threshold), 1, 19 };

menuentry_t menu_keys[] = {
  MENUENTRY_TEXT(      "Keys Menu" )
  MENUENTRY_TEXT(      "=========" )
  MENUENTRY_DEC2( 'T', "Debounce threshold", menu_debounce_threshold )
  MENUENTRY_FUNC( 'S', "Show keycodes", show_keycodes )
  MENUENTRY_BOOL( 'C', "Combo keys", sysconfig.use_combokeys )
  MENUENTRY_HEX1( '1', "Ctrl + F1", menu_key_ctrl_f1 )
  MENUENTRY_HEX1( '3', "Ctrl + F3", menu_key_ctrl_f3 )
  MENUENTRY_HEX1( '5', "Ctrl + F5", menu_key_ctrl_f5 )
  MENUENTRY_HEX1( '7', "Ctrl + F7", menu_key_ctrl_f7 )
  MENUENTRY_BOOL( 'M', "Use custom keymap", sysconfig.use_custom_keymap )
  MENUENTRY_FUNC( 'V', "Verify custom keymap", check_keymap )
  MENUENTRY_FUNC( 'E', "Enter custom keymap codes", enter_keymap )
  MENUENTRY_END
};

menuentry_uint16_t   menu_power_freq         = { &(sysconfig.led_delay),  80, 240 };
menuentry_uint16_t   menu_power_connected    = { &(sysconfig.timer_bt),   10, 3600 };
menuentry_uint16_t   menu_power_disconnected = { &(sysconfig.timer_nobt), 10, 3600 };

menuentry_t menu_power[] = {
  MENUENTRY_TEXT(      "Power Menu" )
  MENUENTRY_TEXT(      "==========" )
  MENUENTRY_DEC2( 'F', "Set CPU frequency", menu_power_freq )
  MENUENTRY_DEC2( 'C', "Set standbytime when connected", menu_power_connected )
  MENUENTRY_DEC2( 'D', "Set standbytime when disconnected", menu_power_disconnected )
  MENUENTRY_FUNC( 'R', "Reboot", esp_restart )
  MENUENTRY_FUNC( 'Z', "Sleep now", Power_off )
  MENUENTRY_END
};

menuentry_uint32_t   menu_bluetooth_pin = { &(sysconfig.bluetooth_pin), 0, 0xFFFFFFFF };

menuentry_t menu_main[] = {
  MENUENTRY_TEXT(      "VICEboard Main Menu" )
  MENUENTRY_TEXT(      "===================" )
  MENUENTRY_FUNC( 'S', "Show full configuration", show_config )
  MENUENTRY_DEC4( 'B', "Set Bluetooth PIN (0=off)", menu_bluetooth_pin )
  MENUENTRY_MENU( 'L', "Configure LEDs", menu_leds )
  MENUENTRY_MENU( 'K', "Configure keyboard", menu_keys )
  MENUENTRY_MENU( 'P', "Power configuration", menu_power )
  MENUENTRY_MENU( 'E', "EEPROM functions", menu_eeprom )
  MENUENTRY_END
};


bool MenuSystem_getValue( char c, char base, uint32_t minimum, uint32_t maximum, uint32_t *value )
{
  /* c = 0: prepare, else handle input */
  static int maxdigits = 0;
  static int digits = 0;
  static uint32_t input = 0;
  bool valid = false;

  if( !c )
  {
    /* c = 0: reset input */
    input = maximum;
    for( maxdigits = 0; input /= base; ++maxdigits )
    {
      /* counting maxdigits */
    }
    digits = 0;
    input = 0;
  }

  if( (c == 0x08) || (c == 0x14) || (c == 0x7f) )
  {
    if( digits > 0 )
    {
      input = input / base;
      --digits;
      Serial.print( "\b \b" );
    }
  }

  if( (c == 0x0a) || (c == 0x0d) )
  {
    if( digits > 0 )
    {
      if( (input >= minimum) && (input <= maximum) )
      {
        *value = input;
      }
      else
      {
        return false;
      }
    }
    Serial.write( '\n' );
    return true;
  }

  if( digits > maxdigits )
  {
    return false;
  }

  if( (c >= '0') && (c <= '9') )
  {
    input = input * base + (c - '0');
    valid = true;
  }

  if( base == 16 )
  {
    if( (c >= 'A') && (c <= 'F') )
    {
      input = input * base + (c - 'A' + 10);
      valid = true;
    }
    if( (c >= 'a') && (c <= 'f') )
    {
      input = input * base + (c - 'a' + 10);
      valid = true;
    }
  }

  if( input > maximum )
  {
    input /= base;
    valid = false;
  }

  if( valid )
  {
    Serial.write( c );
    ++digits;
  }

  return false;
}


bool MenuSystem_getUInt8( char c, char base, void *rawentry )
{
  bool retval;
  menuentry_t *entry = (menuentry_t*)rawentry;
  uint32_t input = *(entry->value8->value);

  retval = MenuSystem_getValue( c, base, entry->value8->minimum, entry->value8->maximum, &input );

  if( retval )
  {
    *(entry->value8->value) = input & 0xFF;
  }
  return retval;
}


bool MenuSystem_getUInt16( char c, char base, void *rawentry )
{
  bool retval;
  menuentry_t *entry = (menuentry_t*)rawentry;
  uint32_t input = *(entry->value16->value);

  retval = MenuSystem_getValue( c, base, entry->value16->minimum, entry->value16->maximum, &input );

  if( retval )
  {
    *(entry->value16->value) = input & 0xFFFF;
  }
  return retval;
}


bool MenuSystem_getUInt32( char c, char base, void *rawentry )
{
  bool retval;
  menuentry_t *entry = (menuentry_t*)rawentry;
  uint32_t input = *(entry->value32->value);

  retval = MenuSystem_getValue( c, base, entry->value32->minimum, entry->value32->maximum, &input );

  if( retval )
  {
    *(entry->value32->value) = input;
  }
  return retval;
}


void MenuSystem_print()
{
  int m, i;
  menuentry_t *entry;

  if( menuprinted )
  {
    return;
  }
  menuprinted = true;
  Serial.println( F("") );
  for ( m = 0; m < MENUSTACKSIZE; ++m )
  {
    if( menustack[m].follow )
    {
      switch ( menustack[m].follow->type )
      {
        case DEC1:
          Serial.printf( "%s\r\n(%u-%u) [%u]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value8->minimum,
                         menustack[m].follow->value8->maximum,
                         *(menustack[m].follow->value8->value) );
          return;
        case DEC2:
          Serial.printf( "%s\r\n(%u-%u) [%u]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value16->minimum,
                         menustack[m].follow->value16->maximum,
                         *(menustack[m].follow->value16->value) );
          return;
        case DEC4:
          Serial.printf( "%s\r\n(%u-%u) [%u]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value32->minimum,
                         menustack[m].follow->value32->maximum,
                         *(menustack[m].follow->value32->value) );
          return;
        case HEX1:
          Serial.printf( "%s\r\n($%02X-$%02X) [$%02X]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value8->minimum,
                         menustack[m].follow->value8->maximum,
                         *(menustack[m].follow->value8->value) );
          return;
        case HEX2:
          Serial.printf( "%s\r\n($%04X-$%04X) [$%04X]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value16->minimum,
                         menustack[m].follow->value16->maximum,
                         *(menustack[m].follow->value16->value) );
          return;
        case HEX4:
          Serial.printf( "%s\r\n($%08X-$%08X) [$%08X]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value32->minimum,
                         menustack[m].follow->value32->maximum,
                         *(menustack[m].follow->value32->value) );
          return;
        default:
          break;
      }
    }
    else
    {
      for ( entry = menustack[m].entries; entry->type != END; ++entry )
      {
        switch ( entry->type )
        {
          case END:
            return;
          case TEXT:
            Serial.printf( "    %s\r\n", entry->text );
            break;
          case MENU:
          case FUNC:
            Serial.printf( "[%c] %s\r\n", entry->letter, entry->text );
            break;
          case BOOL:
            Serial.printf( "[%c] %s [%s]\r\n", entry->letter, entry->text, *(entry->toggle) ? "on" : "off" );
            break;
          case DEC1:
            Serial.printf( "[%c] %s [%u]\r\n", entry->letter, entry->text, *(entry->value8->value) );
            break;
          case DEC2:
            Serial.printf( "[%c] %s [%u]\r\n", entry->letter, entry->text, *(entry->value16->value) );
            break;
          case DEC4:
            Serial.printf( "[%c] %s [%u]\r\n", entry->letter, entry->text, *(entry->value32->value) );
            break;
          case HEX1:
            Serial.printf( "[%c] %s [$%02X]\r\n", entry->letter, entry->text, *(entry->value8->value) );
            break;
          case HEX2:
            Serial.printf( "[%c] %s [$%04X]\r\n", entry->letter, entry->text, *(entry->value16->value) );
            break;
          case HEX4:
            Serial.printf( "[%c] %s [$%08X]\r\n", entry->letter, entry->text, *(entry->value32->value) );
            break;
          default:
            break;
        }
      }
      if( m > 0 )
      {
        Serial.println( F("[X] Go back") );
      }
      Serial.println( F("READY.") );
      return;
    }
  }
}


void MenuSystem_eval( char c )
{
  int m, i;
  menuentry_t *entry;

  c = toupper(c);
  if( c == 0 )
  {
    Serial.printf( "PROBLEM: no way to handle NULL byte input\n" );
    return;
  }
  for ( m = 0; m < MENUSTACKSIZE; ++m )
  {

    if( menustack[m].follow )
    {
      switch ( menustack[m].follow->type )
      {
        case MENU:
          break;
        case DEC1:
          if( MenuSystem_getUInt8( c, 10, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        case DEC2:
          if( MenuSystem_getUInt16( c, 10, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        case DEC4:
          if( MenuSystem_getUInt32( c, 10, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        case HEX1:
          if( MenuSystem_getUInt8( c, 16, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        case HEX2:
          if( MenuSystem_getUInt16( c, 16, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        case HEX4:
          if( MenuSystem_getUInt32( c, 16, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        default:
          return;
      }
    }
    else
    {
      break;
    }
  }
  /* special case: X to go back */
  if( (m > 0) && (c == 'X') )
  {
    menustack[m - 1].follow = 0;
    menustack[m].follow = 0;
    menustack[m].entries = 0;
    menuprinted = false;
    return;
  }
  if( c == ' ' )
  {
    menuprinted = false;
  }

  for( entry = menustack[m].entries; entry->type != END; ++entry )
  {
    if( entry->letter == c )
    {
      switch( entry->type )
      {
        case MENU:
          if( m < MENUSTACKSIZE - 1 )
          {
            menustack[m].follow = entry;
            menustack[m + 1].follow = 0;
            menustack[m + 1].entries = entry->submenu;
            menuprinted = false;
          }
          break;
        case FUNC:
          Serial.println( entry->text );
          entry->callback();
          break;
        case BOOL:
          *(entry->toggle) = !*(entry->toggle);
          menuprinted = false;
          return;
          break;
        case DEC1:
          menustack[m].follow = entry;
          MenuSystem_getUInt8( 0, 10, entry );
          menuprinted = false;
          break;
        case DEC2:
          menustack[m].follow = entry;
          MenuSystem_getUInt16( 0, 10, entry );
          menuprinted = false;
          break;
        case DEC4:
          menustack[m].follow = entry;
          MenuSystem_getUInt32( 0, 10, entry );
          menuprinted = false;
          break;
        case HEX1:
          menustack[m].follow = entry;
          MenuSystem_getUInt8( 0, 16, entry );
          menuprinted = false;
          break;
        case HEX2:
          menustack[m].follow = entry;
          MenuSystem_getUInt16( 0, 16, entry );
          menuprinted = false;
          break;
        case HEX4:
          menustack[m].follow = entry;
          MenuSystem_getUInt32( 0, 16, entry );
          menuprinted = false;
          break;
        default:
          break;
      }
    }
  }
}


void MenuSystem_setup()
{
  menustack[0].follow = 0;
  menustack[0].entries = menu_main;
}


void MenuSystem_loop()
{
  MenuSystem_print();
  if( Serial.available() )
  {
    Power_timer();
    MenuSystem_eval( toupper( Serial.read() ) );
  }
}
