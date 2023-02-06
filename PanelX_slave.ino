#include "src/macros.h"
#include "Wire.h"

#define debug Serial

const int   STRAIGHT        = 0 ;
const int   CURVED          = 1 ;
const int   OFF             = 2 ;

const int   IS_OUTPUT = 0x80 ; // flag to set output LEDs (needed to ignore reading in led states)

uint16_t    iodir ;
uint16_t    input ;

const uint8_t GPIO[] =
{
    3,      // GPIO  1  
    4,      // GPIO  2
    5,      // GPIO  3
    6,      // GPIO  4
    7,      // GPIO  5
    8,      // GPIO  6
    9,      // GPIO  7
    10,     // GPIO  8
    11,     // GPIO  9
    12,     // GPIO 10
    13,     // GPIO 11
    A0,     // GPIO 12
    A1,     // GPIO 13
    A2,     // GPIO 14
    A3,     // GPIO 15
} ;

const uint8_t nPins = 15 ;

enum i2c_tasks
{
    setLed, // 0
    setNx,
    setIodir,
} ;


// CALLED BY I2C MESSAGE // state = 1 -> point LED, state = 0 -> Nx button

#define Task(x,y,z) void x##F( uint8_t y, uint8_t z ) 

Task( setIodir, pin, state )
{
    if( state )
    {
        iodir |= (1<<pin) ;
        pinMode( GPIO[pin], INPUT ) ; // turns of LEDs by default
    }
    else
    {
        pinMode( GPIO[pin], INPUT_PULLUP ) ;
    }    
}

// CALLED BY I2C MESSAGE
Task( setNx, pin, state ) 
{
    if( state )
    {
        pinMode( GPIO[pin], OUTPUT ) ;
        digitalWrite( GPIO[pin], LOW ) ; // PULL DOWN LED WHEN ON
    }   
    else
    {
        pinMode( GPIO[pin], INPUT_PULLUP ) ;
    }
}

// CALLED BY I2C MESSAGE
Task( setLed, pin, state )
{
    switch( state )
    {
    case OFF:
        pinMode( GPIO[pin], INPUT ) ;
        break ;

    case STRAIGHT:
        pinMode( GPIO[pin], OUTPUT ) ;
        digitalWrite( GPIO[pin], LOW ) ;
        break ;

    case CURVED:
        pinMode( GPIO[pin], OUTPUT ) ;
        digitalWrite( GPIO[pin], HIGH ) ;
        break ;
    }
}


//  I2C: command
#define callTask(x,y,z) case x: x##F(y  , z ); break ;
void receiveEvent( int nBytes )
{
    uint8_t  task = Wire.read() ;
    uint8_t   pin = Wire.read() ;
    uint8_t state = Wire.read() ;

    switch( task )
    {
        callTask( setLed,   pin, state ) ;
        callTask( setNx,    pin, state ) ;
        callTask( setIodir, pin, state ) ;
    }
}

 // I2C: input request
void requestEvent()
{
    Wire.write( highByte( input ) ) ; // transmitt my own IO when requested
    Wire.write(  lowByte( input ) ) ;
}



void setup()
{
    for( int i = 0 ; i < nPins ; i ++ )
    {
        pinMode( GPIO[i], INPUT_PULLUP ) ;
    }

    const int addressPins = A7 ;
    int sample = analogRead( addressPins ) ;
    uint8_t myAddress ;

    if(      sample >= 100 - 10 && sample <= 100 + 10 ) myAddress = 1 ; // fix me values please
    else if( sample >= 200 - 10 && sample <= 200 + 10 ) myAddress = 2 ;
    else if( sample >= 300 - 10 && sample <= 300 + 10 ) myAddress = 3 ;
    else if( sample >= 400 - 10 && sample <= 400 + 10 ) myAddress = 4 ;

    Wire.begin( myAddress ) ;
    
    Wire.onRequest( requestEvent );
    Wire.onReceive( receiveEvent );
    
    debug.begin( 9600 ) ;
    debug.println("PanelX slave booted") ;

    printNumberln("my address = ", myAddress ) ;
}


void loop()
{
    REPEAT_MS( 50 )
    {
        input = iodir ; // set all point LED pins in before adding the input status
        
        for( int i = 0 ; i < nPins ; i ++ )
        {
            input |= digitalRead( GPIO[i] ) ;
        }
    }
    END_REPEAT
}

