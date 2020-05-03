
/*

   TODO:
   - map all data
*/

typedef enum
{
  END = 0,
  TEXT,
  MENU,
  FUNC,
  BOOL,
  DECI,
  HEX2,
  HEX4
} menuentrytype_t;

typedef void (*menucallback_t)();
typedef struct {
  unsigned short     *value;
  unsigned short     minimum;
  unsigned short     maximum;
} menuentry_value_t;


typedef struct menuentry_s menuentry_t;
typedef struct menuentry_s {
  menuentrytype_t         type;
  char                    letter;
  const char              *text;
  union {
    void                 *handle;
    menuentry_t          *submenu;
    void                 (*callback)();
    bool                 *toggle;
    menuentry_value_t    *value;
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
#define MENUENTRY_FUNC( a, b, c )   { FUNC, a, b, (void*)c }, /* broken :( */
#define MENUENTRY_BOOL( a, b, c )   { BOOL, a, b, &(c) },
#define MENUENTRY_DECI( a, b, c )   { DECI, a, b, &(c) },
#define MENUENTRY_HEX2( a, b, c )   { HEX2, a, b, &(c) },
#define MENUENTRY_HEX4( a, b, c )   { HEX4, a, b, &(c) },
#define MENUENTRY_END               { END, 0, 0, 0 }



void show_config()
{
  int i;
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

  Serial.print( "CPU freqency: " );
  Serial.println( systemconfig.cpu_freq );
  Serial.print( "Timeout (bluetooth): " );
  Serial.println( systemconfig.timer_bt );
  Serial.print( "Timeout (no bluetooth): " );
  Serial.println( systemconfig.timer_nobt );
  Serial.print( "LED delay: " );
  Serial.println( systemconfig.led_delay );
  Serial.print( "LED brightness: " );
  Serial.println( systemconfig.led_max );
  Serial.print( "Combokeys: " );
  Serial.println( systemconfig.combokeys );
  Serial.print( "CTRL + F1: " );
  Serial.println( systemconfig.ctrl_f1 );
  Serial.print( "CTRL + F3: " );
  Serial.println( systemconfig.ctrl_f3 );
  Serial.print( "CTRL + F5: " );
  Serial.println( systemconfig.ctrl_f5 );
  Serial.print( "CTRL + F7: " );
  Serial.println( systemconfig.ctrl_f7 );
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


menuentry_t menu_eeprom[] = {
  MENUENTRY_TEXT(      "EEPROM Menu" )
  MENUENTRY_TEXT(      "===========" )
  MENUENTRY_FUNC( 'D', "Raw dump EEPROM", EEPROM_dump )
  MENUENTRY_FUNC( 'N', "Nuke (clear) EEPROM", EEPROM_clean )
  MENUENTRY_FUNC( 'W', "Write system configuration to EEPROM", EEPROM_saveconfig )
  MENUENTRY_FUNC( 'G', "Write GPIO configuration to EEPROM", EEPROM_savegpios )
  MENUENTRY_END
};

menuentry_value_t   menu_led_max   = { &(systemconfig.led_max),   0x01, 0xff };
menuentry_value_t   menu_led_delay = { &(systemconfig.led_delay), 0x01, 0x63 };

menuentry_t menu_leds[] = {
  MENUENTRY_TEXT(      "LEDs Menu" )
  MENUENTRY_TEXT(      "=========" )
  MENUENTRY_DECI( 'M', "Maximum brightness", menu_led_max )
  MENUENTRY_DECI( 'D', "Delay for color change", menu_led_delay )
  MENUENTRY_FUNC( 'S', "Show settings on LEDs", LED_change )
  MENUENTRY_END
};

menuentry_value_t   menu_key_ctrl_f1 = { &(systemconfig.ctrl_f1), 0x20, 0xff };
menuentry_value_t   menu_key_ctrl_f3 = { &(systemconfig.ctrl_f3), 0x20, 0xff };
menuentry_value_t   menu_key_ctrl_f5 = { &(systemconfig.ctrl_f5), 0x20, 0xff };
menuentry_value_t   menu_key_ctrl_f7 = { &(systemconfig.ctrl_f7), 0x20, 0xff };

menuentry_t menu_keys[] = {
  MENUENTRY_TEXT(      "Keys Menu" )
  MENUENTRY_TEXT(      "=========" )
  MENUENTRY_BOOL( 'C', "Combo keys", systemconfig.combokeys )
  MENUENTRY_HEX2( '1', "Ctrl + F1", menu_key_ctrl_f1 )
  MENUENTRY_HEX2( '3', "Ctrl + F3", menu_key_ctrl_f3 )
  MENUENTRY_HEX2( '5', "Ctrl + F5", menu_key_ctrl_f5 )
  MENUENTRY_HEX2( '7', "Ctrl + F7", menu_key_ctrl_f7 )
  MENUENTRY_FUNC( 'S', "Show keycodes", show_keycodes )
  MENUENTRY_END
};

menuentry_value_t   menu_power_freq         = { &(systemconfig.led_delay),  80, 240 };
menuentry_value_t   menu_power_connected    = { &(systemconfig.timer_bt),   10, 3600 };
menuentry_value_t   menu_power_disconnected = { &(systemconfig.timer_nobt), 10, 3600 };

menuentry_t menu_power[] = {
  MENUENTRY_TEXT(      "Power Menu" )
  MENUENTRY_TEXT(      "==========" )
  MENUENTRY_DECI( 'F', "Set CPU frequency", menu_power_freq )
  MENUENTRY_DECI( 'C', "Set standbytime when connected", menu_power_connected )
  MENUENTRY_DECI( 'D', "Set standbytime when disconnected", menu_power_disconnected )
  MENUENTRY_FUNC( 'R', "Reboot", esp_restart )
  MENUENTRY_FUNC( 'Z', "Sleep now", Power_off )
  MENUENTRY_END
};

menuentry_t menu_main[] = {
  MENUENTRY_TEXT(      "VICEboard Main Menu" )
  MENUENTRY_TEXT(      "===================" )
  MENUENTRY_FUNC( 'S', "Show full configuration", show_config )
  MENUENTRY_MENU( 'L', "Configure LEDs", menu_leds )
  MENUENTRY_MENU( 'K', "Configure key combos", menu_keys )
  MENUENTRY_MENU( 'P', "Power configuration", menu_power )
  MENUENTRY_MENU( 'E', "EEPROM functions", menu_eeprom )
  MENUENTRY_END
};



/* f'ing arduino reorders source code */
bool MenuSystem_getValue( char c, char base, void *rawentry )
{
  /* c = 0: prepare, else handle input */
  static int maxdigits = 0;
  static int digits = 0;
  static int input = 0;
  bool valid = false;
  menuentry_t *entry = (menuentry_t*)rawentry;

  if( !c )
  {
    input = entry->value->maximum;
    for( maxdigits = 0; input /= base; ++maxdigits )
    {
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
      if( (input >= entry->value->minimum) && (input <= entry->value->maximum) )
      {
        *(entry->value->value) = input;
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

  if( input > entry->value->maximum )
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
        case DECI:
          Serial.printf( "%s\r\n(%d-%d) [%d]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value->minimum,
                         menustack[m].follow->value->maximum,
                         *(menustack[m].follow->value->value) );
          return;
        case HEX2:
          Serial.printf( "%s\r\n($%02X-$%02X) [$%02X]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value->minimum,
                         menustack[m].follow->value->maximum,
                         *(menustack[m].follow->value->value) );
          return;
        case HEX4:
          Serial.printf( "%s\r\n($%04X-$%04X) [$%04X]\r\n", menustack[m].follow->text,
                         menustack[m].follow->value->minimum,
                         menustack[m].follow->value->maximum,
                         *(menustack[m].follow->value->value) );
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
          case DECI:
            Serial.printf( "[%c] %s [%d]\r\n", entry->letter, entry->text, *(entry->value->value) );
            break;
          case HEX2:
            Serial.printf( "[%c] %s [$%02X]\r\n", entry->letter, entry->text, *(entry->value->value) );
            break;
          case HEX4:
            Serial.printf( "[%c] %s [$%04X]\r\n", entry->letter, entry->text, *(entry->value->value) );
            break;
          default:
            break;
        }
      }
      if( m > 0 )
      {
        Serial.println( F("[X] Go back") );
      }
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
        case DECI:
          if( MenuSystem_getValue( c, 10, menustack[m].follow ) )
          {
            menustack[m].follow = 0;
            menuprinted = false;
          }
          return;
        case HEX2:
        case HEX4:
          if( MenuSystem_getValue( c, 16, menustack[m].follow ) )
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
        case DECI:
          menustack[m].follow = entry;
          MenuSystem_getValue( 0, 10, entry );
          menuprinted = false;
          break;
        case HEX2:
        case HEX4:
          menustack[m].follow = entry;
          MenuSystem_getValue( 0, 16, entry );
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
    MenuSystem_eval( toupper( Serial.read() ) );
  }
}
