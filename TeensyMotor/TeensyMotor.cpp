// Core library for code-sense
#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>


// Include application, user and local libraries
#include "Encoder.h"
#include "MotorControl.h"
#include "SerialReader.h"

#include <TimerOne.h>
#include <ArduinoJson.h>

// Encoder
#define     PHASE_A         5
#define     PHASE_B         6
#define     INTERRUPT_A     5
#define     INTERRUPT_B     6

#define     LED_PIN         13

#define     RATE_SAMPLE_MS      1
#define     CLIENT_POLL_RATE_MS 200


uint32_t millisecondClock = 0;
void OneMsClock( )
{
    millisecondClock++;
    Encoder::SampleEvent1ms( );
}

//
// Add setup code
//
void setup( )
{
    Serial.begin( 115200 );
    
    analogWriteFrequency(3, 10000);
    
    pinMode( PHASE_A,       INPUT );
    pinMode( PHASE_B,       INPUT );
    pinMode( LED_PIN,       OUTPUT );
    
    Encoder::CreateEncoder( INTERRUPT_A,   // interrupt A
                            INTERRUPT_B,   // interrupt B
                            PHASE_A,
                            PHASE_B );
}

SerialReader serialReader;
MotorControl motorControl;

uint32_t nextRateSampleMs = 0;
uint32_t nextClientPollMs = 0;
void loop( )
{
    // Once per ms update rate
    if ( millis( ) > nextRateSampleMs )
    {
        motorControl.UpdateRate( );
        
        nextRateSampleMs  = millis( ) + RATE_SAMPLE_MS;
    }
    
    // Once per 100 ms Send to client
    if ( millis( ) > nextClientPollMs )
    {
        motorControl.SendToClient( Serial );
        
        nextClientPollMs  = millis( ) + CLIENT_POLL_RATE_MS;
        digitalWrite( LED_PIN, 0 );
    }
    
    // Update Frequently
    motorControl.UpdateParameters( );
    
    // Check for client data
    serialReader.DoRead( );
    if ( serialReader.IsBufferAvailable( ) )
    {
        digitalWrite(LED_PIN, 1);
        StaticJsonBuffer<2000> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject( serialReader.GetBuffer( ) );
        if ( !root.success( ) )
        {
            Serial.print("{\"Error\":\"");
            Serial.println("\"}");
        }
        else
        {
            motorControl.ReceiveCommand( root );
        }
        serialReader.Clear( );
    }
}