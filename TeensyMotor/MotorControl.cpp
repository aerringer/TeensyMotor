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
    m_maxPwmValue( 80 ),
    m_targetRate( 8 ),
    m_maxRate( 10 ),
    m_desiredPosition( 0 ),
    m_distRemaining( 0 ),
    m_rate( 0 ),
    m_lastCount( 0 ),
    m_rampRange( 10 ),
    m_direction( FORWARD_DIRECTION ),
    m_scheme( "Bouncer" ),
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
    if (0)
    {
        Serial.print("{\"Debug\":\"");
        Serial.print(" m_distRemaining ");
        Serial.print(m_distRemaining);
        Serial.print(" m_rampRange ");
        Serial.print(m_rampRange);
        Serial.print(" m_lastCount ");
        Serial.print(m_lastCount);
        Serial.print(" m_maxPwmValue ");
        Serial.print(m_maxPwmValue);
        Serial.print(" m_desiredPosition ");
        Serial.print(m_desiredPosition);
        Serial.print(" inramp ");
        Serial.print(m_distRemaining < m_rampRange);
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
    m_distRemaining = abs(m_desiredPosition - currentCount);
    
    m_direction = m_desiredPosition > currentCount ? FORWARD_DIRECTION : REVERSE_DIRECTION;
    
    bool inRamp = m_distRemaining < m_rampRange;
    
    if ( inRamp )
    {
        Damp( );
    }
    else
    {
        AdjustPwm( );
    }
 }


void MotorControl::AdjustPwm( )
{
    if ( micros( ) < m_nextAdjustUs )
    {
        return;
    }
    m_nextAdjustUs = micros( ) + PWM_ADJUST_US;
    
    if ( abs(m_rate) < m_targetRate && m_motorPwm < m_maxPwmValue )
    {
        m_motorPwm++;
    }
    else if ( abs(m_rate) > m_targetRate && m_motorPwm > 0 )
    {
        m_motorPwm--;
    }
}


void MotorControl::Damp( )
{
    float distRemaining = m_distRemaining;
    float rampRange     = m_rampRange;
    m_motorPwm = float(m_maxPwmValue) * ( distRemaining / rampRange );
}


// Update Frequently
void MotorControl::UpdateParameters( )
{
    CheckSpeed( );
    UpdateController( );
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
    m_maxPwmValue       = root["maxPwmValue"];
    m_direction         = root["direction"] == 0 ? FORWARD_DIRECTION : REVERSE_DIRECTION;
    m_maxRate           = root["maxRate"];
    m_targetRate        = root["targetRate"];
    m_desiredPosition   = root["desiredPosition"];
    m_rampRange         = root["rampRange"];
    const char* scheme  = root["scheme"];
    m_scheme            = scheme;
}
