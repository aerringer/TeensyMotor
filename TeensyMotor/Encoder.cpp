#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "Encoder.h"


#define MAX_INTERVAL_MS 100000

Encoder*  Encoder::g_pEncoder = NULL;
uint32_t  Encoder::g_interruptPinA = 0;
uint32_t  Encoder::g_interruptPinB = 0;

Encoder::Encoder( uint32_t  interruptPinA,
                  uint32_t  interruptPinB,
                  uint32_t  aPhase,
                  uint32_t  bPhase )
: m_stateA( 0 ),
  m_stateB( 0 ),
  m_count( 0 ),
  m_rate( 0 ),
  m_lastCount( 0 ),
  m_lastAInterruptUs( 0 ),
  m_aPhase( aPhase ),
  m_bPhase( bPhase )
{
    cli();
    g_interruptPinA = interruptPinA;
    g_interruptPinB = interruptPinB;

    attachInterrupt( g_interruptPinA,
                    HandleInterruptA,
                    CHANGE );
    attachInterrupt( g_interruptPinB,
                    HandleInterruptB,
                    CHANGE );
    sei();
 }

Encoder::~Encoder( )
{
    cli();
    detachInterrupt( g_interruptPinA );
    detachInterrupt( g_interruptPinB );
}

Encoder* Encoder::GetEncoder( )
{
    if ( g_pEncoder == NULL )
    {
        Serial.println( "g_pEncoder == NULL" );
        delay(1000);
        exit( 1 );
    }
    return g_pEncoder;
}

Encoder* Encoder::CreateEncoder( uint32_t  interruptPinA,
                                 uint32_t  interruptPinB,
                                 uint32_t  aPhase,
                                 uint32_t  bPhase )
{
    if ( g_pEncoder != NULL )
    {
        Serial.println( "g_pEncoder != NULL" );
        delay(1000);
        exit( 1 );
    }
    g_pEncoder = new Encoder( interruptPinA,
                              interruptPinB,
                              aPhase,
                              bPhase );
}

void Encoder::SampleEvent1ms( )
{
    uint32_t countPerMs = g_pEncoder->GetCount( ) - g_pEncoder->m_lastCount;
    g_pEncoder->m_lastCount = g_pEncoder->GetCount( );
    if ( micros( ) - g_pEncoder->m_lastAInterruptUs > MAX_INTERVAL_MS )
    {
        countPerMs = 0;
    }
    g_pEncoder->m_rate = countPerMs;
}

void Encoder::HandleInterruptA( )
{
    cli();
    g_pEncoder->m_lastAInterruptUs = micros( );
    g_pEncoder->m_stateA = digitalRead( g_pEncoder->m_aPhase );

    if ( g_pEncoder->m_stateB == g_pEncoder->m_stateA )
    {
        g_pEncoder->m_count--;
    }
    else
    {
        g_pEncoder->m_count++;
    }
    sei();
}

void Encoder:: HandleInterruptB( )
{
    cli();
    g_pEncoder->m_stateB = digitalRead( g_pEncoder->m_bPhase );
    
    if ( g_pEncoder->m_stateB == g_pEncoder->m_stateA )
    {
        g_pEncoder->m_count++;
    }
    else
    {
        g_pEncoder->m_count--;
    }
    sei();
}
