/*
 * Keyboard.ino
 * ============
 * 
 * have alternative keyset?
 * keyboard buffer when not connected
 */

#define       KEYBOARD_BUFFER_ENABLED 1
#define       BLUETOOTH_PIN_SUPPORTED 1

#define       KEYBOARD_KEYNUM   (65)
#define       KEYBOARD_RESTORE  (KEYBOARD_KEYNUM-1)
bool          Keyboard_pressed[KEYBOARD_KEYNUM];
signed int    Keyboard_debounce[KEYBOARD_KEYNUM];
const uint8_t Keymap_VICE_positional[KEYBOARD_KEYNUM] =
{
  KEY_BACKSPACE, KEY_RETURN, KEY_RIGHT_ARROW, KEY_F7,   KEY_F1,          KEY_F3,        KEY_F5,     KEY_DOWN_ARROW,
  '3',           'w',        'a',             '4',      'z',             's',           'e',        KEY_LEFT_SHIFT,
  '5',           'r',        'd',             '6',      'c',             'f',           't',        'x',
  '7',           'y',        'g',             '8',      'b',             'h',           'u',        'v',
  '9',           'i',        'j',             '0',      'm',             'k',           'o',        'n',
  '-',           'p',        'l',             '=',      '.',             ';',           '[',        ',',
  KEY_INSERT,    ']',        '\'',            KEY_HOME, KEY_RIGHT_SHIFT, '\\',          KEY_DELETE, '/',
  '1',           '`',        KEY_TAB,         '2',      ' ',             KEY_LEFT_CTRL, 'q',        KEY_ESC,
  KEY_PAGE_UP
};


void Keyboard_setup()
{
  int i;
  for( i = 0; i < KEYBOARD_KEYNUM; ++i )
  {
    Keyboard_pressed[i] = false;
    Keyboard_debounce[i] = 0;
  }
}


void Keyboard_start_Bluetooth()
{
  /*
   * if compiling fails at this point, you're using an old version
   * of the ESP32-BLE-Keyboard library. As a workaround you can set
   * BLUETOOTH_PIN_SUPPORTED to 0 at the top of this file.
   */
#if BLUETOOTH_PIN_SUPPORTED
  bleKeyboard.begin( sysconfig.bluetooth_pin );
#else
  bleKeyboard.begin();
#endif
}

#if KEYBOARD_BUFFER_ENABLED
bool Keyboard_send( uint8_t keycode, bool pressed )
{
  static int     buffered = 0;
  static uint8_t keycodes[16];
  static bool    presseds[16];

  if( bleKeyboard.isConnected() )
  {
    if( buffered )
    {
      /* send buffer first */
      for( int i = 0; i < buffered; ++i )
      {
        if( presseds[i] )
        {
          bleKeyboard.press( keycodes[i] );
        }
        else
        {
          bleKeyboard.release( keycodes[i] );
        }
        /* a short delay between buffered keyevents: a PAL frame */
        delay(20);
      }
    }
    buffered = 0;

    if( pressed )
    {
      bleKeyboard.press( keycode );
    }
    else
    {
      bleKeyboard.release( keycode );
    }
  }
  else
  {
    if( buffered < 16 )
    {
      keycodes[buffered] = keycode;
      presseds[buffered] = pressed;
    }
  }
}
#endif


int Keyboard_getcode( int id, bool pressed )
{
  static int combokey = 0;
  int key = 0;
  int modifier = 0x000;

  const uint8_t *Keymap = sysconfig.use_custom_keymap ?
                          sysconfig.custom_keymap :
                          Keymap_VICE_positional;

  if( !sysconfig.use_combokeys )
  {
    /* combokey disabled or no modifier pressed */
    return Keymap[id];
  }

  /* left shift */
  if( Keyboard_pressed[0x0f] )
  {
    modifier |= 0x100;
  }
  /* right shift */
  if( Keyboard_pressed[0x34] )
  {
    modifier |= 0x200;
  }
  /* commodore */
  if( Keyboard_pressed[0x3d] )
  {
    modifier |= 0x400;
  }
  /* control */
  if( Keyboard_pressed[0x3a] )
  {
    modifier |= 0x800;
  }
  
  switch( modifier | id )
  {
    /* shift + crsr right */
    case 0x102:
    case 0x202:
      key = KEY_LEFT_ARROW;
      break;
    /* shift + crsr down */
    case 0x107:
    case 0x207:
      key = KEY_UP_ARROW;
      break;
    /* control + f1 */
    case 0x804:
      key = sysconfig.ctrl_f1;
      break;
    /* control + f3 */
    case 0x805:
      key = sysconfig.ctrl_f3;
      break;
    /* control + f5 */
    case 0x806:
      key = sysconfig.ctrl_f5;
      break;
    /* control + f7 */
    case 0x803:
      key = sysconfig.ctrl_f7;
      break;
    default:
      break;
  }

  if( pressed )
  {
    if( combokey || !key )
    {
      /* a combokey is already pressed or not replaced */
      return Keymap[id];
    }
    combokey = modifier | key;
  }
  else
  {
    if( !combokey )
    {
      /* no combokey is pressed */
      return Keymap[id];
    }
    
    key = combokey & 0xFF;
    switch( id )
    {
      case 0x0f: /* left shift */
      case 0x34: /* right shift */
      case 0x3d: /* commodore */
      case 0x3a: /* control */
        bleKeyboard.release( Keymap[id] );
        break;
      default:
        break;
    }
    combokey = 0;
  }
  return key;
}


void Keyboard_checkkey( int id, int gpio )
{
  if( digitalRead( gpio ) == LOW )
  {
    /* press detected */
    if( Keyboard_debounce[id] == DEBOUNCE_THRESHOLD )
    {
      Keyboard_pressed[id] = true;
#if KEYBOARD_BUFFER_ENABLED
      Keyboard_send( Keyboard_getcode( id, true ), true );
#else
      bleKeyboard.press( Keyboard_getcode( id, false ) );
#endif
    }
    if( Keyboard_debounce[id] < DEBOUNCE_THRESHOLD + DEBOUNCE_THRESHOLD )
    {
      ++Keyboard_debounce[id];
    }
    Power_timer();
  }
  else
  {
    /* release detected */
    if( Keyboard_debounce[id] == -DEBOUNCE_THRESHOLD )
    {
      Keyboard_pressed[id] = false;
#if KEYBOARD_BUFFER_ENABLED
      Keyboard_send( Keyboard_getcode( id, false ), false );
#else
      bleKeyboard.release( Keyboard_getcode( id, false ) );
#endif
    }
    if( Keyboard_debounce[id] > -DEBOUNCE_THRESHOLD - DEBOUNCE_THRESHOLD )
    {
      --Keyboard_debounce[id];
    }
  }
}
