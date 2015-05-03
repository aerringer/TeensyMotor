//
//  MotorControl.h
//  TeensyMotor
//
//  Created by Alan Erringer on 02/05/15.
//  Copyright (c) 2015 Joe Bob. All rights reserved.
//

#pragma once

#include <Arduino.h>
#include <avr/io.h>

#include <ArduinoJson.h>

class MotorControl
{
public:
    enum Direction
    {
        FORWARD_DIRECTION = 0,
        REVERSE_DIRECTION = 1,
    };
    
    MotorControl( );
    void SendToClient(Print& serial);
    void UpdateRate( );
    void CheckDirection( );
    void CheckSpeed( );
    void UpdateParameters( );
    void UpdateController( );
    void ReceiveCommand( const JsonObject& root );
    const String& GetScheme( ) const;
    void GoToAbsolute( );
    void AdjustPwm( uint32_t targetRate );

private:
    uint32_t    m_motorPwm;
    uint32_t    m_minRate;
    uint32_t    m_targetRate;
    uint32_t    m_maxRate;
    uint32_t    m_maxPwmValue;
    int32_t     m_maxPosition;
    int32_t     m_desiredPosition;
    int32_t     m_minPosition;
    int32_t     m_rate;
    int32_t     m_lastCount;
    int32_t     m_rampRange;
    Direction   m_direction;
    String      m_scheme;
    uint32_t    m_slop;
    uint32_t    m_maxAbsRate;
    uint32_t    m_minAbsRate;
    uint32_t    m_nextAdjustUs;
    float       m_distRemaining;
};


