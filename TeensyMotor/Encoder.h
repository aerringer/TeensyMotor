#include <Arduino.h>
#include <avr/io.h>

class Encoder
{
public:
    ~Encoder( );
    
    static Encoder* CreateEncoder( uint32_t  interruptPinA,
                                   uint32_t  interruptPinB,
                                   uint32_t  aPhase,
                                   uint32_t  bPhase );
    static Encoder* GetEncoder( );
    
    static void SampleEvent1ms( );
    
    int32_t GetCount( ) const { return m_count; }
    int32_t GetRate ( ) const { return m_rate; }
    
private:
    static void HandleInterruptA( );
    static void HandleInterruptB( );
    
    Encoder( uint32_t  interruptPinA,
             uint32_t  interruptPinB,
             uint32_t  aPhase,
             uint32_t  bPhase );

    static Encoder*  g_pEncoder;
    static uint32_t  g_interruptPinA;
    static uint32_t  g_interruptPinB;
    
    volatile uint32_t m_stateA;
    volatile uint32_t m_stateB;
    volatile uint32_t m_count;
    uint32_t          m_rate;
    uint32_t          m_lastCount;
    volatile uint32_t m_lastAInterruptUs;
    uint32_t          m_aPhase;
    uint32_t          m_bPhase;
};