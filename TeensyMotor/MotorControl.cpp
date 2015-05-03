//
//  MotorControl.cpp
//  TeensyMotor
//
//  Created by Alan Erringer on 02/05/15.
//  Copyright (c) 2015 Joe Bob. All rights reserved.
//

#include "MotorControl.h"
#include "Encoder.h"

// Motor
#define     PWM_HIGH        3
#define     PWM_DIRECTION   4

#define     PWM_ADJUST_US   100


MotorControl::MotorControl( )
  : m_motorPwm( 0 ),
    m_minRate( 0 ),
    m_targetRate( 8 ),
    m_maxRate( 10 ),
    m_maxPwmValue( 80 ),
    m_maxPosition( 10000 ),
    m_desiredPosition( 0 ),
    m_minPosition( 0 ),
    m_rate( 0 ),
    m_lastCount( 0 ),
    m_rampRange( 10 ),
    m_direction( FORWARD_DIRECTION ),
    m_scheme( "Bouncer" ),
    m_slop( 0 ),
    m_maxAbsRate( 0 ),
    m_minAbsRate( 0 ),
    m_nextAdjustUs( 0 )
{
    pinMode( PWM_DIRECTION, OUTPUT );
    pinMode( PWM_HIGH,      OUTPUT );
    digitalWrite( PWM_DIRECTION,  m_direction );
    digitalWrite( PWM_HIGH,       0 );
}

void MotorControl::SendToClient(Print& serial)
{
    StaticJsonBuffer<2000> jsonBuffer;
    
    JsonObject& root = jsonBuffer.createObject();
    root["E"] = m_motorPwm;
    root["D"] = m_direction;
    root["C"] = Encoder::GetEncoder()->GetCount( );
    root["R"] = m_rate;
    
    root.printTo( serial );
    serial.println( );
    if (1)
    {
        Serial.print("{\"Debug\":\"");
        Serial.print(" m_distRemaining ");
        Serial.print(m_distRemaining);
        Serial.print(" m_rampRange ");
        Serial.print(m_rampRange);
        Serial.print(" m_lastCount ");
        Serial.print(m_lastCount);
        Serial.print(" m_desiredPosition ");
        Serial.print(m_desiredPosition);
        Serial.print(" m_maxRate ");
        Serial.print(m_maxRate);
        Serial.print(" m_motorPwm ");
        Serial.print(m_motorPwm);
        Serial.println("\"}");
    }
}

void MotorControl::UpdateRate( )
{
    const int32_t currentCount = Encoder::GetEncoder()->GetCount( );
    m_rate = currentCount - m_lastCount;
    m_lastCount = currentCount;
}

// Update frequently
void MotorControl::CheckSpeed( )
{
    const int32_t currentCount = Encoder::GetEncoder()->GetCount( );
    bool nearMax = (currentCount < m_desiredPosition) && currentCount > (m_desiredPosition - m_rampRange);
    bool nearMin = (currentCount > m_desiredPosition) && currentCount < (m_desiredPosition + m_rampRange);
    
    if ( ! nearMax && ! nearMin )
    {
        AdjustPwm( m_targetRate );
        
        return;
    }
    
    if ( m_rampRange > 0 )
    {
        if ( nearMax )
        {
            m_distRemaining = float(m_desiredPosition - currentCount) / m_rampRange;
        }
        else if ( nearMin )
        {
            m_distRemaining = float(currentCount - m_desiredPosition) / m_rampRange;
        }
    }
    bool rampDown = ( ( nearMax && m_direction == FORWARD_DIRECTION )
                     || ( nearMin && m_direction == REVERSE_DIRECTION ) );

    bool rampUp = ( ( nearMax && m_direction == REVERSE_DIRECTION  )
                   || ( nearMin && m_direction == FORWARD_DIRECTION ) );
    
    
    uint32_t rate = (float(m_maxRate - m_minRate) * m_distRemaining) + m_minRate;
    AdjustPwm( rate );
}

void MotorControl::AdjustPwm( uint32_t targetRate )
{
    if (micros() < m_nextAdjustUs)
    {
        return;
    }
    m_nextAdjustUs = micros() + PWM_ADJUST_US;
    
    if ( abs(m_rate) < targetRate && m_motorPwm < m_maxPwmValue )
    {
        m_motorPwm++;
    }
    else if ( abs(m_rate) > targetRate && m_motorPwm > 0 )
    {
        m_motorPwm--;
    }
    
}


// Update Frequently
void MotorControl::UpdateParameters( )
{
    CheckDirection( );
    CheckSpeed( );
    UpdateController( );
}

void MotorControl::CheckDirection( )
{
    const int32_t currentCount = Encoder::GetEncoder()->GetCount( );
    if ( currentCount > m_desiredPosition )
    {
        m_direction = REVERSE_DIRECTION;
    }
    else if ( currentCount < m_desiredPosition )
    {
        m_direction = FORWARD_DIRECTION;
    }
}

void MotorControl::UpdateController( )
{
    digitalWrite( PWM_DIRECTION, m_direction );
    analogWrite(  PWM_HIGH,      m_motorPwm);
}

const String& MotorControl::GetScheme( ) const
{
    return m_scheme;
}

void MotorControl::ReceiveCommand( const JsonObject& root )
{
    m_maxPwmValue = root["maxPwmValue"];
    m_direction = root["direction"] == 0 ? FORWARD_DIRECTION : REVERSE_DIRECTION;
    m_maxRate = root["maxRate"];
    m_targetRate = root["targetRate"];
    m_desiredPosition = root["desiredPosition"];
    m_rampRange = root["rampRange"];
    const char* scheme = root["scheme"];
    m_scheme = scheme;
}
