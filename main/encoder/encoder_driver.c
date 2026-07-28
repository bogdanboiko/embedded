#include "encoder_driver.h"

#define DEBOUNCE_US 200000

static const char* TAG = "endoder.driver";
static gpio_num_t cn_a_gpio = GPIO_NUM_NC;
static gpio_num_t cn_b_gpio = GPIO_NUM_NC;
static gpio_num_t cn_sw_gpio = GPIO_NUM_NC;

static volatile uint8_t last_encoder_state = 0;
static volatile int counter = 0;
static volatile int isr_counter = 0;

// static const int8_t QUAD_TABLE[16] = {
//   0, -1, +1, 0,
//   +1, 0, 0, -1,
//   -1, 0, 0, +1,
//   0, +1, -1, 0
// };


static volatile int64_t last_us = 0;

bool IRAM_ATTR check_debounce(void) {
    int64_t now = esp_timer_get_time();

    if (now - last_us < DEBOUNCE_US) {
        return false;
    } else {
        last_us = now;
        return true;
    }
}

void IRAM_ATTR encoder_cn_a_handler(void* args) {
    bool current_cn_a_state = gpio_get_level(cn_a_gpio);
    bool current_cn_b_state = gpio_get_level(cn_b_gpio);

    uint8_t current_state = (current_cn_a_state << 1) | current_cn_b_state;
    uint8_t transition = (last_encoder_state << 2) | current_state;

    // counter += QUAD_TABLE[transition];
    switch (transition) {
        // counter-clockwise
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            counter--;
            break;

        // clockwise
        case 0b0010:
        case 0b1011:
        case 0b1101:
        case 0b0100:
            counter++;
            break;

        default:
            break;
    }

    last_encoder_state = current_state;
}

void config_encoder(gpio_num_t cn_a_gpio_arg, gpio_num_t cn_b_gpio_arg,
                    gpio_num_t sw_gpio_arg) {
    if (cn_a_gpio_arg == GPIO_NUM_NC || cn_b_gpio_arg == GPIO_NUM_NC ||
        sw_gpio_arg == GPIO_NUM_NC) {
        ESP_LOGE(TAG, "Provided wrong gpio numbers");
    }

    cn_a_gpio = cn_a_gpio_arg;
    cn_b_gpio = cn_b_gpio_arg;
    cn_sw_gpio = sw_gpio_arg;

    gpio_config_t cn_a_config = {.intr_type = GPIO_INTR_ANYEDGE,
                                 .mode = GPIO_MODE_INPUT,
                                 .pull_down_en = 0,
                                 .pull_up_en = 1,
                                 .pin_bit_mask = 1ULL << cn_a_gpio_arg};

    gpio_config_t cn_b_config = {.intr_type = GPIO_INTR_ANYEDGE,
                                 .mode = GPIO_MODE_INPUT,
                                 .pull_down_en = 0,
                                 .pull_up_en = 1,
                                 .pin_bit_mask = 1ULL << cn_b_gpio_arg};

    gpio_config(&cn_a_config);
    gpio_config(&cn_b_config);

     bool current_cn_a_state = gpio_get_level(cn_a_gpio);
    bool current_cn_b_state = gpio_get_level(cn_b_gpio);

    last_encoder_state = (current_cn_a_state << 1) | current_cn_b_state;

    gpio_install_isr_service(0);

    gpio_isr_handler_add(cn_a_gpio_arg, &encoder_cn_a_handler, NULL);
    gpio_isr_handler_add(cn_b_gpio_arg, &encoder_cn_a_handler, NULL);
}

int get_current_count() { return counter / 4; }

int get_current_angle() { return (counter * 360 / 80) % 360; }
