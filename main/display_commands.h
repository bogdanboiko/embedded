#define COMMAND_CONTROL_BYTE 0x00 // control byte to indicate that next byte will be command
#define DATA_CONTROL_BYTE 0x40 // control byte to indicate that next byte will be command

#define DISPLAY_OFF 0xAE // sleep mode
#define DISPLAY_ON 0xAF // normal mode

#define ENTIRE_DISPLAY_OFF 0xA4 // return to RAM content
#define ENTIRE_DISPLAY_ON 0xA5 // lit all display

#define MULTIPLEX_RATIO 0xA8 // MUX ratio(16-64MUX) visible rows
#define DISPLAY_OFFSET 0xD3 // COM rows offset(0-64)
#define DISPLAY_START_LINE 0x40 // set offset for maps RAM rows to COM rows(1-64)

#define SEGMENT_REMAPPED_DISABLE 0xA0 // if column address 0 is mapped to SEG0 
#define SEGMENT_REMAPPED_ENABLE 0xA1 // if column address 127 is mapped to SEG0 

#define COM_OUTPUT_SCAN_REMAPPED_DISABLE 0xC0 // Enable or disable inverting COM rows scan for drawing upside down 
#define COM_OUTPUT_SCAN_REMAPPED_ENABLE 0xC8

#define CONTRAST_CONTROL 0x81 // Set param to 1-256 level of brightness

#define NORMAL_DISPLAY_MODE 0xA6
#define INVERSE_DISPLAY_MODE 0xA7 // show inversed image(bits set as 1 count as 0 and vice versa)

#define DISPLAY_OSCILLATOR_FREQUENCY 0xD5 // used to config display frame rate frequency(default 0x80)

#define CHARGE_PUMP_REGULATOR 0x8D // params ON - 0x14; OFF - 0x10

// Ram manipulation commands

#define MEMORY_ADDRESSING_MODE 0x20 // params: Horizontal - 0x00; Vertical - 0x01; Page - 0x02; Invalid - 0x03;

#define SET_PAGE_ADDRESS_RANGE_COMMAND 0x22 // require 2 bytes of params, page start and page end
#define SET_COLUMN_ADDRESS_RANGE_COMMAND 0x21 // require 2 bytes of params, column start and column end

typedef enum {
    HORIZONTAL_DISPLAY_ADDRESS_MODE = 0,
    VERTICAL_DISPLAY_ADDRESS_MODE,
    PAGE_DISPLAY_ADDRESS_MODE,
} display_address_mode_t;
/*
    In Page addressing mode, to set iterator point, you should specify page pointer(B0-B7 commands)
    and column pointer. Column pointer is setting up by 2 separate commands, lower(00-0F) 4 bits of
    pointer number and higher(10-1F).

    So to set Page 1 colum 1 - send commands B0 - 01 - 10
*/