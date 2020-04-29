
/*
 * LEDs
 * ====
 * 
 * will not switch color immediately, but fade to desired brightness
 * maximum brightness will also be limited
 */


typedef struct LED_state_s
{
  int current;
  int target;
  int next;
  
} LED_state_t;

LED_state_t LED_states[2];


void LED_setup()
{
  ledcSetup( 0, 5000, 8 );
  ledcSetup( 1, 5000, 8 );
  ledcAttachPin( LED_gpios[0], 0 );
  ledcAttachPin( LED_gpios[1], 1 );
  LED_states[0].current = 0;
  LED_states[1].current = 0;
  LED_states[0].target  = 100;
  LED_states[1].current = 0;
  LED_states[0].next    = 100;
  LED_states[1].next    = 0;
}


void LED_off()
{
  for( int i = 0; i < 2; ++i )
  {
    ledcDetachPin( LED_gpios[i] );
    pinMode( i, INPUT_PULLUP );
  }
}


void LED_set( int id, int value )
{
  LED_states[id].next = value;
}


void LED_loop()
{
  static unsigned short counter = 0;
  int i;
  for( i = 0; i < 2; ++i )
  {
    if( LED_states[i].current == LED_states[i].target )
    {
      LED_states[i].target = LED_states[i].next;
    }
  }
  if( ++counter < systemconfig.led_delay )
  {
    return;
  }

  counter = 0;
  for( i = 0; i < 2; ++i )
  {
    if( LED_states[i].current < LED_states[i].target )
    {
      ++(LED_states[i].current);
      ledcWrite( i, (unsigned short)LED_states[i].current * systemconfig.led_max / 100 );
    }
    if( LED_states[i].current > LED_states[i].target )
    {
      --(LED_states[i].current);
      ledcWrite( i, (unsigned short)LED_states[i].current * systemconfig.led_max / 100 );
    }
  }
}
