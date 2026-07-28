#include "display_driver.h"

static const char* TAG = "display.driver";

static i2c_master_bus_handle_t bus_handler;
static i2c_master_dev_handle_t device_handler;
static uint8_t frame_buffer[1024] = {0};

void config_device_protocol(gpio_num_t sda_pin, gpio_num_t clk_pin) {
    i2c_master_bus_config_t config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .sda_io_num = sda_pin,
        .scl_io_num = clk_pin,
        .intr_priority = 2,
        .glitch_ignore_cnt = 7,
        .trans_queue_depth = 0,
        .flags = {.enable_internal_pullup = 1, .allow_pd = 0}};

    i2c_new_master_bus(&config, &bus_handler);

    i2c_device_config_t device_config = {.device_address = DISPLAY_ADDRESS,
                                         .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                         .scl_speed_hz = 400000,
                                         .scl_wait_us = 20000};

    i2c_master_bus_add_device(bus_handler, &device_config, &device_handler);
}

void send_display_command_with_params(uint8_t command, uint8_t* params,
                                      int params_size) {
    int buffer_size = 2 + params_size;
    uint8_t buffer[buffer_size];

    buffer[0] = COMMAND_CONTROL_BYTE;
    buffer[1] = command;

    for (int i = 0; i < params_size; i++) {
        buffer[i + 2] = params[i];
    }

    ESP_ERROR_CHECK(
        i2c_master_transmit(device_handler, buffer, buffer_size, 200));
}

void send_display_command(uint8_t command) {
    uint8_t buffer[2] = {COMMAND_CONTROL_BYTE, command};

    ESP_ERROR_CHECK(i2c_master_transmit(device_handler, buffer, 2, 200));
}

void send_display_data(uint8_t* data, int data_size) {
    int buffer_size = data_size + 1;

    uint8_t buffer[buffer_size];
    buffer[0] = DATA_CONTROL_BYTE;

    for (int i = 0; i < data_size; i++) {
        buffer[i + 1] = data[i];
    }

    ESP_ERROR_CHECK(
        i2c_master_transmit(device_handler, buffer, buffer_size, 200));
}

void config_display() {
    uint8_t mux_ratio = 0x3F;
    uint8_t display_offset = 0x00;
    uint8_t display_start_line = 0x00;
    uint8_t contrast_rate = 0x7F;
    uint8_t oscillator_config = 0x80;
    uint8_t charge_pump_enable_status = 0x14;

    send_display_command_with_params(MULTIPLEX_RATIO, &mux_ratio, 1);
    send_display_command_with_params(DISPLAY_OFFSET, &display_offset, 1);
    send_display_command_with_params(DISPLAY_START_LINE, &display_start_line,
                                     1);
    send_display_command(SEGMENT_REMAPPED_DISABLE);
    send_display_command(COM_OUTPUT_SCAN_REMAPPED_DISABLE);
    send_display_command_with_params(CONTRAST_CONTROL, &contrast_rate, 1);
    send_display_command(ENTIRE_DISPLAY_OFF);
    send_display_command(NORMAL_DISPLAY_MODE);
    send_display_command_with_params(DISPLAY_OSCILLATOR_FREQUENCY,
                                     &oscillator_config, 1);
    send_display_command_with_params(CHARGE_PUMP_REGULATOR,
                                     &charge_pump_enable_status, 1);
    send_display_command(DISPLAY_ON);
}

bool troubleshoot_display_connection_success() {
    esp_err_t probe_res = i2c_master_probe(bus_handler, DISPLAY_ADDRESS, 100);
    return probe_res == ESP_OK;
}

void set_address_mode(display_address_mode_t mode) {
    uint8_t address_mode_param = 0x00 + mode;
    send_display_command_with_params(MEMORY_ADDRESSING_MODE,
                                     &address_mode_param, 1);
}

// Page address mode
bool set_page_address(uint8_t page) {
    if (page > DISPLAY_PAGES_NUMBER) {
        ESP_LOGE(TAG, "Selected page %d is out of bounds(0 - %d)", page,
                 DISPLAY_PAGES_NUMBER);
        return false;
    }
    send_display_command(page + 0xB0);
    return true;
}

bool set_page_column_address(uint8_t column_address) {
    if (column_address > DISPLAY_COL_NUMBER) {
        ESP_LOGE(TAG, "Selected column %d is out of bounds(0 - %d)",
                 column_address, DISPLAY_COL_NUMBER);
        return false;
    }

    uint8_t lower_bit = column_address & 0x0F;
    uint8_t higher_bit = 0x10 | (column_address & 0xF0);
    send_display_command(lower_bit);
    send_display_command(higher_bit);
    return true;
}

// Horizontal/Vertical address mode
void set_page_address_range(uint8_t start, uint8_t end) {
    if (end >= DISPLAY_PAGES_NUMBER || start > end) {
        ESP_LOGE(TAG, "Selected page range %d - %d is out of bounds(0 - %d)",
                 start, end, DISPLAY_PAGES_NUMBER);
    }

    uint8_t params[2] = {start, end};

    send_display_command_with_params(SET_PAGE_ADDRESS_RANGE_COMMAND, params, 2);
}

void set_column_address_range(uint8_t start, uint8_t end) {
    if (end >= DISPLAY_COL_NUMBER || start > end) {
        ESP_LOGE(TAG, "Selected column range %d - %d is out of bounds(0 - %d)",
                 start, end, DISPLAY_COL_NUMBER);
    }

    uint8_t params[2] = {start, end};

    send_display_command_with_params(SET_COLUMN_ADDRESS_RANGE_COMMAND, params,
                                     2);
}

void flush_display_buffer() {
    set_address_mode(HORIZONTAL_DISPLAY_ADDRESS_MODE);
    set_page_address_range(0U, 7U);
    set_column_address_range(0U, 127U);
    send_display_data(frame_buffer, 1024);
}

void clear_display_buffer() {
    memset(frame_buffer, 0, 1024);
    flush_display_buffer();
}

void toggle_pixel(canvas_point_t point) {
    if (point.x >= DISPLAY_COL_NUMBER || point.y >= DISPLAY_ROW_NUMBER) {
        ESP_LOGE(TAG, "Invalid point x=%d, y=%d!", point.x, point.y);
        return;
    }

    int y_bitmask = point.y % 8;
    int point_page = point.y / DISPLAY_PAGES_NUMBER;
    int frame_col_index = DISPLAY_COL_NUMBER * point_page + point.x;

    frame_buffer[frame_col_index] |= 1u << y_bitmask;
}

void draw_line_to_buffer(canvas_point_t start, canvas_point_t end) {
    // Bresenham's line algorithm
    int dx = abs((int)end.x - (int)start.x);
    int dy = -abs((int)end.y - (int)start.y);
    int sx = start.x < end.x ? 1 : -1;
    int sy = start.y < end.y ? 1 : -1;
    int error = dx + dy;

    int x = start.x;
    int y = start.y;

    while (true) {
        toggle_pixel((canvas_point_t){.x = x, .y = y});

        if (x == (int)end.x && y == (int)end.y) break;

        int e2 = 2 * error;

        if (e2 >= dy) {
            error += dy;
            x += sx;
        }

        if (e2 <= dx) {
            error += dx;
            y += sy;
        }
    }
}

void draw_line(canvas_point_t start, canvas_point_t end) {
    if (end.x >= DISPLAY_COL_NUMBER || end.y >= DISPLAY_ROW_NUMBER) {
        ESP_LOGE(TAG, "Invalid points!");
        return;
    }

    draw_line_to_buffer(start, end);
    flush_display_buffer();
}

void draw_rect(canvas_point_t tl, canvas_point_t rb) {
    canvas_point_t start = {.x = tl.x, .y = tl.y};
    canvas_point_t end = {.x = rb.x, .y = tl.y};
    draw_line_to_buffer(start, end);

    end.x = start.x = rb.x;
    start.y = tl.y;
    end.y = rb.y;
    draw_line_to_buffer(start, end);

    start.y = end.y = rb.y;
    start.x = rb.x;
    end.x = tl.x;
    draw_line_to_buffer(start, end);

    end.x = start.x = tl.x;
    start.y = rb.y;
    end.y = tl.y;
    draw_line_to_buffer(start, end);
    flush_display_buffer();
}
