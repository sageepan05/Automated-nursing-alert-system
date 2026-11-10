// Automated Nursing Alert System
// MCU      : PIC16F877A | Clock: 8 MHz (set in Proteus / crystal)
// Compiler : mikroC PRO for PIC (uses sbit and interrupt() syntax)
// Pins     : Ward 0-7 = RB0-RB7, Ward 8 = RA0, Ward 9 = RA1, Reset = RA2
//            LED = RC0, Buzzer = RC1, 7-segment display = PORTD
// Timing   : Timer1 interrupt every ~65 ms (8 MHz, 1:2 prescaler, no preload)
//            Buzzer sounds for 6 ticks = ~0.4 s on each new call

//Define pins
sbit W8       at RA0_bit;   // Ward 8 switch is connected to pin RA0
sbit W9       at RA1_bit;   // Ward 9 switch is connected to pin RA1
sbit RESET_SW at RA2_bit;   // Reset push-button is connected to pin RA2
sbit LED      at RC0_bit;   // Alerting LED is connected to pin RC0
sbit BUZZER   at RC1_bit;   // Buzzer is connected to pin RC1

//Global state
unsigned int  pending_wards   = 0;   // 16 bit variable for 0-9 wards, 1 = call active, 0 = no call
unsigned int  prev_pending    = 0;   // Stores last checked pending_wards
unsigned char reset_prev      = 0;   // Stores last state of reset button
unsigned char buzzer_timer    = 0;   // Counts down for buzzer on period
unsigned char current_ward    = 0;   // Currently shown ward number on display
unsigned char display_valid   = 0;   // 1 = display on, 0 = display off

// 7-segment lookup table
unsigned char seg_table[10] =
{
    0x3F, // 0 - segments a,b,c,d,e,f ON
    0x06, // 1 - segments b,c ON
    0x5B, // 2 - segments a,b,g,e,d ON
    0x4F, // 3 - segments a,b,g,c,d ON
    0x66, // 4 - segments f,g,b,c ON
    0x6D, // 5 - segments a,f,g,c,d ON
    0x7D, // 6 - segments a,f,g,e,d,c ON
    0x07, // 7 - segments a,b,c ON
    0x7F, // 8 - all segments ON
    0x6F  // 9 - segments a,b,c,d,f,g ON
};

// Read all 10 ward switches into a single 10-bit mask
unsigned int read_switches()
{
    unsigned int mask = 0;

    if (RB0_bit) mask |= (1 << 0);   // If ward 0 switch is HIGH, set bit 0 in mask
    if (RB1_bit) mask |= (1 << 1);   // If ward 1 switch is HIGH, set bit 1 in mask
    if (RB2_bit) mask |= (1 << 2);   // If ward 2 switch is HIGH, set bit 2 in mask
    if (RB3_bit) mask |= (1 << 3);   // If ward 3 switch is HIGH, set bit 3 in mask
    if (RB4_bit) mask |= (1 << 4);   // If ward 4 switch is HIGH, set bit 4 in mask
    if (RB5_bit) mask |= (1 << 5);   // If ward 5 switch is HIGH, set bit 5 in mask
    if (RB6_bit) mask |= (1 << 6);   // If ward 6 switch is HIGH, set bit 6 in mask
    if (RB7_bit) mask |= (1 << 7);   // If ward 7 switch is HIGH, set bit 7 in mask
    if (W8)      mask |= (1 << 8);   // If ward 8 switch is HIGH, set bit 8 in mask
    if (W9)      mask |= (1 << 9);   // If ward 9 switch is HIGH, set bit 9 in mask

    return mask;   // send back the full 10-bit picture of all switches
}

// Find highest-priority ward among currently pending
// *ward - used as pointer, to hold memory address
unsigned char get_highest_priority(unsigned int pending, unsigned char *ward)
{
    signed char i;   //Signed instead of unsigned beacuse need to count upto 0, to take i = -1

    for (i = 9; i >= 0; i--)            // check ward 9 first,then 8, then 7... down to 0
    {
        if (pending & (1 << i)) 
        {
            *ward = (unsigned char)i;  // save this ward number into the caller's variable
            return 1;                  // Found a pending ward
        }
    }
    return 0;   // No ward is pending
}

// Update the 7-segment display + LED + buzzer
void update_outputs()
{
    if (display_valid)
    {
        PORTD = seg_table[current_ward];   // send the correct segment pattern to the display
        LED   = 1;                         // turn the status LED ON
    } 
    else
    {
        PORTD = 0x00;                     // turn off all segments (blank display)
        LED   = 0;                        // turn the status LED OFF
    }

    if (buzzer_timer > 0)
    {
        BUZZER = 1;           // keep buzzer on
        buzzer_timer--;       // count down by 1 tick (~65ms) closer to silence
    } 
    else
    {
        BUZZER = 0;           //  buzzer off
    }
}

// Timer1 interrupt executes about every 65 ms (8 MHz, 1:2 prescaler)
void interrupt()
{
    if (PIR1.TMR1IF)         // TMR1 IF = 1 means Timer1 has just overflowed (time's up)
    {
        PIR1.TMR1IF = 0;    // Clear the flag so we don't process the same tick twice

        pending_wards |= read_switches();    // Read switch pressed and add to que

        //Instead of using delay_ms(300);
        if (pending_wards & (~prev_pending))  // Detect new calls and trigger buzzer
        {
            buzzer_timer = 6;    // start buzzer countdown: 6 ticks x ~65ms = ~0.4 s
        }

        prev_pending = pending_wards;   // remember this tick's state for next time's comparison


        if (RESET_SW && !reset_prev)         // Handle reset switch
        {
            // RESET_SW is HIGH now, but was LOW last tick, then this is a fresh button press
            if (display_valid)     // only clear if a ward is actually shown
            {
                pending_wards &= ~(1 << current_ward);    // clear just that one ward's pending bit
            }
        }

        reset_prev = RESET_SW;   // remember reset button's state for next tick's check

        display_valid = get_highest_priority(pending_wards, &current_ward);   // Determine which ward to show now

        update_outputs();   // Update display + LED + buzzer
    }
}

void main()
{
    ADCON1 = 0x06;      // Configure PORTA pins as DIGITAL I/O

    TRISB  = 0xFF;       // Set all PORTB pins (W0-W7 switches) as INPUTS
    TRISA  = 0xFF;       // Set all PORTA pins (W8, W9, RESET) as INPUTS
    TRISD  = 0x00;       // Set all PORTD pins (7-seg segments) as OUTPUTS
    TRISC0_bit = 0;      // Set RC0 (LED pin) as OUTPUT
    TRISC1_bit = 0;      // Set RC1 (BUZZER pin) as OUTPUT

    PORTD  = 0x00;      // Start with display blank
    LED    = 0;         // Start with LED off
    BUZZER = 0;         // Start with buzzer off

    // Timer1 setup: prescaler 1:2, internal clock, overflow ~65 ms @ 8MHz
    T1CON  = 0x11;       // TMR1 ON = 1 (timer running), prescaler=1:2, clock source=internal
    TMR1H  = 0;          // Clear Timer1's high byte (start counting from 0)
    TMR1L  = 0;          // Clear Timer1's low byte (start counting from 0)
    PIR1.TMR1IF = 0;     // Clear any leftover Timer1 interrupt flag before we start
    PIE1.TMR1IE = 1;     // Enable (unmask) the Timer1 interrupt specifically
    INTCON.PEIE = 1;     // Enable peripheral interrupts in general (required for Timer1)
    INTCON.GIE  = 1;     // Enable global interrupts (master switch - must be on)

    while (1) {}
}