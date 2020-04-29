/*
 * Keyboard
 * ========
 * 
 * have alternative keyset?
 * keyboard buffer when not connected
 */

#define             KEYBOARD_BUFFER_ENABLED 1

#define             KEYBOARD_KEYNUM   (65)
#define             KEYBOARD_RESTORE  (KEYBOARD_KEYNUM-1)
bool                Keyboard_pressed[KEYBOARD_KEYNUM];
signed int          Keyboard_debounce[KEYBOARD_KEYNUM];
const unsigned char Keyboard_positional64[KEYBOARD_KEYNUM] =
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

  if( !systemconfig.combokeys )
  {
    /* combokey disabled or no modifier pressed */
    return Keyboard_positional64[id];
  }
  
  if( Keyboard_pressed[0x0f] )
  {
    modifier |= 0x100;
  }
  if( Keyboard_pressed[0x34] )
  {
    modifier |= 0x200;
  }
  if( Keyboard_pressed[0x3d] )
  {
    modifier |= 0x400;
  }
  if( Keyboard_pressed[0x3a] )
  {
    modifier |= 0x800;
  }
  
  switch( modifier | id )
  {
    case 0x102:
    case 0x202:
      key = KEY_LEFT_ARROW;
      break;
    case 0x107:
    case 0x207:
      key = KEY_UP_ARROW;
      break;
    case 0x804:
      key = systemconfig.ctrl_f1;
      break;
    case 0x805:
      key = systemconfig.ctrl_f3;
      break;
    case 0x806:
      key = systemconfig.ctrl_f5;
      break;
    case 0x803:
      key = systemconfig.ctrl_f7;
      break;
    default:
      break;
  }

  if( pressed )
  {
    if( combokey || !key )
    {
      /* a combokey is already pressed or not replaced */
      return Keyboard_positional64[id];
    }
    combokey = modifier | key;
  }
  else
  {
    if( !combokey )
    {
      /* no combokey is pressed */
      return Keyboard_positional64[id];
    }
    
    key = combokey & 0xFF;
    switch( id )
    {
      case 0x0f:
      case 0x34:
      case 0x3d:
      case 0x3a:
        bleKeyboard.release( Keyboard_positional64[id] );
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
