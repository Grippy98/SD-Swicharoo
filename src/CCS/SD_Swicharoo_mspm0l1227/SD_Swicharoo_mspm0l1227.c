#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

// =========================================================================
// == USER CONFIGURATION ===================================================
// =========================================================================

#define F_CPU 32000000UL
#define PIXELS 3

// WS2812 LED configuration
#define PIXEL_PORT_BASE     ((GPIO_Regs *)GPIOA_BASE)
#define PIXEL_PIN           DL_GPIO_PIN_6
#define RES_US 300 // Reset time in microseconds

// TMUX131 Selector Pin Configuration
// Bank 1: SD Card -> DUT Port
#define SEL0_IN_PORT_BASE   ((GPIO_Regs *)GPIOA_BASE)
#define SEL0_IN_PIN         DL_GPIO_PIN_26
#define SEL1_IN_PORT_BASE   ((GPIO_Regs *)GPIOA_BASE)
#define SEL1_IN_PIN         DL_GPIO_PIN_25

// Bank 2: SD Card -> FLASH Port
#define SEL0_OUT_PORT_BASE  ((GPIO_Regs *)GPIOA_BASE)
#define SEL0_OUT_PIN        DL_GPIO_PIN_22
#define SEL1_OUT_PORT_BASE  ((GPIO_Regs *)GPIOA_BASE)
#define SEL1_OUT_PIN        DL_GPIO_PIN_21

// LED Colors
#define COLOR_GREEN {0, 20, 0}
#define COLOR_BLUE  {0, 0, 20}
#define COLOR_OFF   {0, 0, 0}

/*
 * Number of bytes to receive from Controller.
 * This is calculated as 3 bytes (R, G, B) for each pixel.
 */
#define I2C_RX_PACKET_SIZE (PIXELS * 3)

/*
 * Number of bytes to send from Target to Controller.
 * This can be used by the controller to verify the device identity.
 */
#define I2C_TX_PACKET_SIZE (5)

// =========================================================================
// == DATA STORAGE FOR PIXELS ==============================================
// =========================================================================

// Holds the color data for each pixel (in GRB format)
uint8_t pixel_colors[PIXELS][3];

// Flag to be set by I2C ISR when data is received
volatile bool gI2CDataReceived = false;
// State variable for LED1
bool gLed1On = false;

/* Data sent to Controller in response to a Read transfer */
uint8_t gTxPacket[I2C_TX_PACKET_SIZE] = {'L', 'E', 'D', 'O', 'K'};

/* Data received from Controller during a Write transfer */
uint8_t gRxPacket[I2C_RX_PACKET_SIZE];

// Holds the color data for each pixel (in GRB format for the show() function)
uint8_t pixel_colors[PIXELS][3];

// =========================================================================
// == LOW-LEVEL BIT-SENDING FUNCTION =======================================
// =========================================================================

static inline void sendBit(bool bitVal) {
    // These volatile pointers are used to directly manipulate GPIO registers for speed.
    volatile uint32_t *port_set_addr = &PIXEL_PORT_BASE->DOUTSET31_0;
    volatile uint32_t *port_clear_addr = &PIXEL_PORT_BASE->DOUTCLR31_0;
    uint32_t pin_mask = PIXEL_PIN;

    if (bitVal) { // Send a '1' Bit
        *port_set_addr = pin_mask;
        // T1H delay (~800ns) - NOPs are used for precise timing
        __asm__ volatile(
            ".rept 24 \n\t"
            "nop \n\t"
            ".endr \n\t"
        );
        *port_clear_addr = pin_mask;
        // T1L delay (~450ns)
        __asm__ volatile(
            ".rept 12 \n\t"
            "nop \n\t"
            ".endr \n\t"
        );
    } else { // Send a '0' Bit
        *port_set_addr = pin_mask;
        // T0H delay (~400ns)
        __asm__ volatile(
            ".rept 11 \n\t"
            "nop \n\t"
            ".endr \n\t"
        );
        *port_clear_addr = pin_mask;
        // T0L delay (~850ns)
        __asm__ volatile(
            ".rept 25 \n\t"
            "nop \n\t"
            ".endr \n\t"
        );
    }
}

void delay_us(uint32_t us) {
    uint32_t cycles = us * (F_CPU / 1000000);
    DL_Common_delayCycles(cycles);
}

// =========================================================================
// == PIXEL API FUNCTIONS ==================================================
// =========================================================================

static inline void sendByte(unsigned char byte) {
    for (unsigned char bit = 0; bit < 8; bit++) {
        sendBit((byte & 0x80) != 0); // Send the most significant bit first
        byte <<= 1;
    }
}

void ledsetup() {
    DL_GPIO_initDigitalOutput(PIXEL_PIN);
    DL_GPIO_setPins(PIXEL_PORT_BASE, PIXEL_PIN);
    DL_GPIO_clearPins(PIXEL_PORT_BASE, PIXEL_PIN);
}

// Sends the color data stored in the pixel_colors array to the LED string
void show() {
    __disable_irq(); // Disable interrupts to ensure precise timing
    for (int i = 0; i < PIXELS; i++) {
        sendByte(pixel_colors[i][1]); // Green
        sendByte(pixel_colors[i][0]); // Red
        sendByte(pixel_colors[i][2]); // Blue
    }
    __enable_irq(); // Re-enable interrupts
    delay_us(RES_US); // Send reset pulse
}

// Sets the color of a specific pixel in the array
void setPixelColor(uint16_t pixel_index, uint8_t r, uint8_t g, uint8_t b) {
    if (pixel_index < PIXELS) {
        pixel_colors[pixel_index][0] = r;
        pixel_colors[pixel_index][1] = g;
        pixel_colors[pixel_index][2] = b;
    }
}

// =========================================================================
// == MULTIPLEXER CONTROL FUNCTIONS ========================================
// =========================================================================

// Initializes the GPIO pins used to control the TMUX131 multiplexers
void setup_mux() {
    DL_GPIO_initDigitalOutput(SEL0_IN_PIN);
    DL_GPIO_initDigitalOutput(SEL1_IN_PIN);
    DL_GPIO_initDigitalOutput(SEL0_OUT_PIN);
    DL_GPIO_initDigitalOutput(SEL1_OUT_PIN);
}

// Sets the multiplexer to connect an SD card to the DUT port
// card_number: 1, 2, or 3. Any other value disconnects.
void select_dut_card(uint8_t card_number) {
    switch (card_number) {
        case 0: // Disconnect (S1=1, S0=1)
            DL_GPIO_setPins(SEL1_IN_PORT_BASE, SEL1_IN_PIN);
            DL_GPIO_setPins(SEL0_IN_PORT_BASE, SEL0_IN_PIN);
            break;
        case 1: // S1=1, S0=0 -> D1/D2 connected to S1A/S2A
            DL_GPIO_setPins(SEL1_IN_PORT_BASE, SEL1_IN_PIN);
            DL_GPIO_clearPins(SEL0_IN_PORT_BASE, SEL0_IN_PIN);
            break;
        case 2: // S1=0, S0=0 -> D1/D2 connected to S1B/S2B
            DL_GPIO_clearPins(SEL1_IN_PORT_BASE, SEL1_IN_PIN);
            DL_GPIO_clearPins(SEL0_IN_PORT_BASE, SEL0_IN_PIN);
            break;
        case 3: // S1=0, S0=1 -> D1/D2 connected to S1C/S2C
            DL_GPIO_clearPins(SEL1_IN_PORT_BASE, SEL1_IN_PIN);
            DL_GPIO_setPins(SEL0_IN_PORT_BASE, SEL0_IN_PIN);
            break;
    }
}

// Sets the multiplexer to connect an SD card to the FLASH port
// card_number: 1, 2, or 3. Any other value disconnects.
void select_flash_card(uint8_t card_number) {
    switch (card_number) {
        case 0: // Disconnect (S1=1, S0=1)
            DL_GPIO_setPins(SEL1_OUT_PORT_BASE, SEL1_OUT_PIN);
            DL_GPIO_setPins(SEL0_OUT_PORT_BASE, SEL0_OUT_PIN);
            break;
        case 1: // S1=1, S0=0 -> D1/D2 connected to S1A/S2A
            DL_GPIO_setPins(SEL1_OUT_PORT_BASE, SEL1_OUT_PIN);
            DL_GPIO_clearPins(SEL0_OUT_PORT_BASE, SEL0_OUT_PIN);
            break;
        case 2: // S1=0, S0=0 -> D1/D2 connected to S1B/S2B
            DL_GPIO_clearPins(SEL1_OUT_PORT_BASE, SEL1_OUT_PIN);
            DL_GPIO_clearPins(SEL0_OUT_PORT_BASE, SEL0_OUT_PIN);
            break;
        case 3: // S1=0, S0=1 -> D1/D2 connected to S1C/S2C
            DL_GPIO_clearPins(SEL1_OUT_PORT_BASE, SEL1_OUT_PIN);
            DL_GPIO_setPins(SEL0_OUT_PORT_BASE, SEL0_OUT_PIN);
            break;
    }
}

void disconnectCards(){
    delay_us(5000000);
    select_dut_card(0);
    select_flash_card(0);
    setPixelColor(0, 0, 00, 0);  
    setPixelColor(1, 0, 00, 0);  
    setPixelColor(2, 0, 00, 0);
    show();
    }

int main(void) {
    SYSCFG_DL_init();
    //ledsetup();
    //setup_mux();
    //delay_us(1000000);
    // --- Set Initial State ---
    //select_dut_card(0); //Disconnect DUT
    //select_flash_card(0); //Disconnect FLASH
    // 1. Connect SD Card 1 to the DUT port
    //select_dut_card(1);

    // 2. Connect SD Card 3 to the FLASH port
    //select_flash_card(3);

    // 3. Update LEDs to reflect the connections
    //    LED 1 (index 0) = Green (DUT)
    //    LED 2 (index 1) = Off
    //    LED 3 (index 2) = Blue (FLASH)
    
    setPixelColor(0, 20, 00, 0);  // LED 1 Green
    setPixelColor(1, 0, 20, 0);   // LED 2 Off
    setPixelColor(2, 0, 0, 20);  // LED 3 Blue
    show();
    //delay_us(5000000);

    select_dut_card(2);
    select_flash_card(3);
    setPixelColor(0, 0, 00, 20);  
    setPixelColor(1, 0, 00, 0);  
    setPixelColor(2, 0, 00, 0);
    show();

    // --- System is now configured. Enter an infinite loop. ---
    while (1) {
        // The processor will idle here, maintaining the configured state.
        //__WFI(); // Wait For Interrupt - a low power mode

        /*disconnectCards();


        delay_us(5000000); //5 second delay
        select_dut_card(1);
        setPixelColor(0, 0, 20, 0);  
        setPixelColor(1, 0, 00, 0);  
        setPixelColor(2, 0, 00, 0);
        show();

        disconnectCards();

        delay_us(5000000); //5 second delay
        select_dut_card(2);
        setPixelColor(0, 0, 00, 0);  
        setPixelColor(1, 0, 20, 0);  
        setPixelColor(2, 0, 00, 0);
        show();

        disconnectCards();

        delay_us(5000000); //5 second delay
        select_dut_card(3);
        setPixelColor(0, 0, 0, 0);  
        setPixelColor(1, 0, 00, 0);  
        setPixelColor(2, 0, 20, 0);
        show();

        disconnectCards();*/
        /*

                // CORRECTED LINE: Use '...Counter' instead of the non-existent '...Status'
        if (DL_I2C_getTargetRXFIFOCounter(I2C_0_INST) >= I2C_RX_PACKET_SIZE) {
            
            // A full packet is available, so now we read it.
            for (uint8_t i = 0; i < I2C_RX_PACKET_SIZE; i++) {
                gRxPacket[i] = DL_I2C_receiveTargetData(I2C_0_INST);
            }

            // Once all bytes are received, update the pixel_colors array.
            for (int i = 0; i < PIXELS; i++) {
                uint8_t r = gRxPacket[i * 3 + 0];
                uint8_t g = gRxPacket[i * 3 + 1];
                uint8_t b = gRxPacket[i * 3 + 2];
                setPixelColor(i, r, g, b);
            }

            // Push the new colors to the LED strip
            show();
        }

        // Refill the TX buffer if it's empty
        if (DL_I2C_isTargetTXFIFOEmpty(I2C_0_INST)) {
            DL_I2C_fillTargetTXFIFO(I2C_0_INST, &gTxPacket[0], I2C_TX_PACKET_SIZE);
        }
        */
    }
}
