#include "src/macros.h"
#include "Wire.h"

#define debug Serial

const int   STRAIGHT        = 0 ;
const int   CURVED          = 1 ;
const int   OFF             = 2 ;

const int   IS_OUTPUT = 0x80 ; // flag to set output LEDs (needed to ignore reading in led states)

uint16_t    iodir ;
uint16_t    input ;

const uint8_t nPins = 15 ;
const uint8_t firstPin = 3 ;

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
        pinMode( pin, INPUT ) ; // turns of LEDs by default
    }
    else
    {
        pinMode( pin, INPUT_PULLUP ) ;
    }    
}

// CALLED BY I2C MESSAGE
Task( setNx, pin, state ) 
{
    if( state )
    {
        pinMode( pin, OUTPUT ) ;
        digitalWrite( pin, LOW ) ; // PULL DOWN LED WHEN ON
    }   
    else
    {
        pinMode( pin, INPUT_PULLUP ) ;
    }
}

// CALLED BY I2C MESSAGE
Task( setLed, pin, state )
{
    switch( state )
    {
    case OFF:
        pinMode( pin+firstPin, INPUT ) ;
        break ;

    case STRAIGHT:
        pinMode( pin+firstPin, OUTPUT ) ;
        digitalWrite( pin+firstPin, LOW ) ;
        break ;

    case CURVED:
        pinMode( pin+firstPin, OUTPUT ) ;
        digitalWrite( pin+firstPin, HIGH ) ;
        break ;
    }
}

 // I2C: input request
void requestEvent()
{
    Wire.write( highByte( input ) ) ; // transmitt my own IO when requested
    Wire.write(  lowByte( input ) ) ;
}



//  I2C: command

#define callTask(x,y,z) case x: x##F(firstPin+ y  , z ); break ;
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

void setup()
{
    for( int i = 0 ; i < nPins ; i ++ )
    {
        setIodirF( firstPin + i, 0 ) ;
    }

    const int addressPins = A7 ;
    int sample = analogRead( addressPins ) ;

    if(      sample >= 100 - 10 && sample <= 100 + 10 ) Wire.begin(1) ; // fix me values please
    else if( sample >= 200 - 10 && sample <= 200 + 10 ) Wire.begin(2) ;
    else if( sample >= 300 - 10 && sample <= 300 + 10 ) Wire.begin(3) ;
    else if( sample >= 400 - 10 && sample <= 400 + 10 ) Wire.begin(4) ;

    
    Wire.onRequest( requestEvent );
    Wire.onReceive( receiveEvent );
    
    debug.begin( 9600 ) ;
    debug.println("PanelX slave booted") ;
}


void loop()
{
    REPEAT_MS( 50 )
    {
        input = iodir ; // set all point LED pins in before adding the input status
        
        for( int i = 0 ; i < nPins ; i ++ )
        {
            input |= digitalRead( firstPin + i ) ;
        }
    }
    END_REPEAT
}

