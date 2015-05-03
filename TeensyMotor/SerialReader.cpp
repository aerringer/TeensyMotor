//
//  SerialReader.cpp
//  TeensyMotor
//
//  Created by Alan Erringer on 02/05/15.
//  Copyright (c) 2015 Joe Bob. All rights reserved.
//

#include "SerialReader.h"

SerialReader::SerialReader( )
: m_buffer( ),
  m_currentChar( 0 ),
  m_isBufferAvailable( false )
{
}

bool SerialReader::IsBufferAvailable( ) const
{
    return m_isBufferAvailable;
}

void SerialReader::DoRead( )
{
    while (Serial.available())
    {
        char c = Serial.read();
        if ( c == '\n' || c == '\r' )
        {
            m_buffer[m_currentChar++] = '\0';
            m_isBufferAvailable = true;
        }
        else
        {
            m_buffer[m_currentChar++] = c;
        }
    }
}

char* SerialReader::GetBuffer( )
{
    return m_buffer;
}

void SerialReader::Clear( )
{
    m_currentChar=0;
    m_isBufferAvailable = false;
}

