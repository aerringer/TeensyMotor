//
//  SerialReader.h
//  TeensyMotor
//
//  Created by Alan Erringer on 02/05/15.
//  Copyright (c) 2015 Joe Bob. All rights reserved.
//

#pragma once

#include <Arduino.h>
#include <avr/io.h>

class SerialReader
{
public:
    SerialReader( );
    bool IsBufferAvailable( ) const;
    void DoRead( );
    char* GetBuffer( );
    void Clear( );
    
private:
    char        m_buffer[4096];
    uint32_t    m_currentChar;
    bool        m_isBufferAvailable;
};

