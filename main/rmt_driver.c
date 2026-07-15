#include "rmt_driver.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_private/periph_ctrl.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rmt_reg.h"
#include "soc/soc.h" /* REG_WRITE / REG_READ */
#include "soc/system_reg.h"
#include "soc/gpio_sig_map.h"
#include "esp_log.h"

#include "measurement_constants.h"

#define MAX_CLK_FREQ (40 * HZ_TO_MEGA)

static const char* TAG = "RMTDrivder";

void config_system_rmt_clock() {
 // Enable clock for rmt peripheral
    REG_WRITE(SYSTEM_PERIP_CLK_EN0_REG,
              REG_READ(SYSTEM_PERIP_CLK_EN0_REG) | SYSTEM_RMT_CLK_EN);
    uint32_t sys_rst = REG_READ(SYSTEM_PERIP_RST_EN0_REG);
    sys_rst &= ~SYSTEM_RMT_RST;
    REG_WRITE(SYSTEM_PERIP_RST_EN0_REG, sys_rst);
    ESP_LOGI(TAG, "System rmt clock config control 0x%" PRIX32, REG_READ(SYSTEM_PERIP_CLK_EN0_REG));
    ESP_LOGI(TAG, "System rmt clock rst config control 0x%" PRIX32, REG_READ(SYSTEM_PERIP_RST_EN0_REG));
}

void gpio_init(uint32_t gpio) {
    // config IO MUX pin function
    uint32_t mux_reg = IO_MUX_GPIO0_REG + (gpio * 4);
    uint32_t mux_config = REG_READ(mux_reg);

    // clear bits before settings new ones
    mux_config &= ~(0x2 << MCU_SEL_S);
    // config function(GPIO pin/direct peripheral wired)
    mux_config |= PIN_FUNC_GPIO << MCU_SEL_S;
    // config pull down
    mux_config |= FUN_PD;
    REG_WRITE(mux_reg, mux_config);
    ESP_LOGI(TAG, "IO MUX config for gpio 0x%08" PRIX32, mux_config);

    // config GPIO output to RMT peripheral GPIO_FUNC0_OUT_SEL_CFG_REG
    uint32_t gpio_periph_reg = GPIO_FUNC0_OUT_SEL_CFG_REG + (gpio * 4);
    uint32_t gpio_periph_config = REG_READ(gpio_periph_reg);

    // As this mask require 8 bits, first it's better to clear this bits from last data
    gpio_periph_config &= ~0xFF;
    // set peripheral out function num as gpio output GPIO_FUNCx_OUT_SEL
    gpio_periph_config |= RMT_SIG_OUT0_IDX;
    // set who is enabling gpio (peripheral or manually through GPIO_ENABLE_REG) GPIO_FUNCx_OEN_SEL
    gpio_periph_config |= BIT(10);
    REG_WRITE(gpio_periph_reg, gpio_periph_config);

     ESP_LOGI(TAG, "GPIO pin function config 0x%08" PRIX32, gpio_periph_config);
     ESP_LOGI(TAG, "gpio = %d", gpio);
    if (gpio <= 31) {
        REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(gpio));
    } else if (gpio <= 48) {
        REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(gpio - 32));
    }
}

void rmt_config(uint32_t resolution_hz) {
    uint32_t rmt_sys_config = REG_READ(RMT_SYS_CONF_REG);
    // Set RMT Ram FIFO access 
    rmt_sys_config &= ~RMT_APB_FIFO_MASK;

    // Set RMT clock source APB_CLK(max 80MHz)
    rmt_sys_config &= ~(0x3 << RMT_SCLK_SEL_S);
    rmt_sys_config |= 1 << RMT_SCLK_SEL_S;

    /* 
    Config RMT clock source divider (RMT_SCLK_DIV_NUM + 1 + RMT_SCLK_DIV_A/RMT_SCLK_DIV_B)
    We are trying to get max possible requency at 40MHz
    */
    rmt_sys_config &= ~(0x3F << RMT_SCLK_DIV_A_S);
    rmt_sys_config &= ~(0x3F << RMT_SCLK_DIV_B_S);
    rmt_sys_config &= ~(0xFF << RMT_SCLK_DIV_NUM_S);
    rmt_sys_config |= 1 << RMT_SCLK_DIV_NUM_S;
    // Enable RMT 
    rmt_sys_config |= RMT_SCLK_ACTIVE;
    // Enable internal RMT clock gate
    rmt_sys_config |= RMT_CLK_EN;

    ESP_LOGI(TAG, "RMT sys config 0x%" PRIX32, rmt_sys_config);

    uint32_t rmt_ch_data_register = REG_READ(RMT_CH0CONF0_REG);
    // Clear bits for rmt channel clock divider
    rmt_ch_data_register &= ~(0xFF << RMT_DIV_CNT_CH0_S); 
    // Set rmt channel clock divider
    uint32_t divider = MAX_CLK_FREQ / resolution_hz;
    if (divider > 255) {
        divider = 255;
    } else if (divider < 1) {
        divider = 1;
    }

    ESP_LOGI(TAG, "RMT chn divider %d", divider);
    rmt_ch_data_register |= divider << RMT_DIV_CNT_CH0_S;

    // Disable signal modulation
    rmt_ch_data_register &= ~RMT_CARRIER_EN_CH0;

    // Set this bit to reset to RMT_IDLE_OUT_LV_CH0 level after transmission finished
    rmt_ch_data_register |= RMT_IDLE_OUT_EN_CH0;
    // Value to reset to level after transmission finished
    rmt_ch_data_register &= ~RMT_IDLE_OUT_LV_CH0;

    // Reset and clear FIFO memory
    rmt_ch_data_register |= RMT_APB_MEM_RST_CH0;
    rmt_ch_data_register |= RMT_CONF_UPDATE_CH0;
    REG_WRITE(RMT_CH0CONF0_REG, rmt_ch_data_register);
}

// налаштувати канал і пін НА РЕГІСТРАХ; самі підберіть дільники під розрішення
void rmt_raw_init(int gpio, uint32_t resolution_hz) {
    config_system_rmt_clock();
    gpio_init(gpio);
    rmt_config(resolution_hz);
}

// зібрати символи в 32-бітні слова, записати в CHnDATA (+ нуль-маркер) і
// запустити
void rmt_raw_send_items(const rmt_item_t* items, size_t count) {
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_APB_MEM_RST_CH0);

    rmt_item_t buffer;
    uint32_t data;
    
    for (int i = 0; i < count; i++) {
        data = 0;
        buffer = items[i];

        data |= buffer.dur0;
        data |= buffer.lvl0 << 15;
        data |= buffer.dur1 << 16;
        data |= buffer.lvl1 << 31;

        ESP_LOGI(TAG, "RMT signal number %d with data: 0x%08" PRIX32, i, data);

        REG_WRITE(RMT_CH0DATA_REG, data);
    }

    REG_WRITE(RMT_CH0DATA_REG, 0);
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_MEM_RD_RST_CH0);
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_TX_START_CH0);
    rmt_raw_wait();
}


uint32_t time_to_ticks(uint32_t n_time_to_tick_ratio, uint32_t n_time) {
    return n_time / n_time_to_tick_ratio;
}

uint32_t get_current_config_n_time_to_tick_ratio() {
    uint32_t channel_config = REG_READ(RMT_CH0CONF0_REG);
    uint32_t divider = (channel_config & RMT_DIV_CNT_CH0_M) >> RMT_DIV_CNT_CH0_S;
        ESP_LOGI(TAG, "divider readed %d", divider);

    uint32_t current_freq = MAX_CLK_FREQ / divider;
    return S_TO_NANO / current_freq;
}

// зручна обгортка: тривалості в МІКРОСЕКУНДАХ, рівень чергується від
// start_level
void rmt_raw_send_pulses(const uint32_t* durations_us, size_t count,
                         int start_level) {
    uint32_t data = 0;

    if (count % 2 > 0) {
        return;
    }

    // if (count > 1) {
    //     count = 1;
    // } else if (count < 0) {
    //     count = 0;
    // }

    uint32_t nano_time_to_tick_ratio = get_current_config_n_time_to_tick_ratio();
    ESP_LOGI(TAG, "nano_time_to_tick_ratio %d", nano_time_to_tick_ratio);
    uint32_t pairs = count / 2;

    for (int i = 0; i < pairs; i++) {
        int32_t duration_pointer = i * pairs;

        data |= time_to_ticks(nano_time_to_tick_ratio, durations_us[duration_pointer] * 1000);
        data |= start_level << 15;
        data |= time_to_ticks(nano_time_to_tick_ratio, durations_us[duration_pointer + 1] * 1000) << 16;
        data |= (~start_level) << 31;
        REG_WRITE(RMT_CH0DATA_REG, data);
    }

    REG_WRITE(RMT_CH0DATA_REG, 0);
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_MEM_RD_RST_CH0);
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_TX_START_CH0);
    rmt_raw_wait();
}

// меандр: частота (Гц), скважність (%), кількість періодів
void rmt_raw_square(uint32_t freq_hz, uint8_t duty_pct, uint32_t periods) {
    uint8_t normalized_duty = duty_pct;

    if (duty_pct > 100) {
        normalized_duty = 100;
    } else if (duty_pct < 1) {
        normalized_duty = 1;  
    }

    uint32_t n_time_of_period = S_TO_NANO / freq_hz;
    uint32_t n_time_of_level_on = n_time_of_period * duty_pct / 100;
    uint32_t n_time_of_level_off = n_time_of_period - n_time_of_level_on;
    uint32_t nano_time_to_tick_ratio = get_current_config_n_time_to_tick_ratio();

    uint32_t data = 0;
    
    for(int i = 0; i < periods; i++) {
        data |= time_to_ticks(nano_time_to_tick_ratio, n_time_of_level_on);
        data |= 1 << 15;
        data |= time_to_ticks(nano_time_to_tick_ratio, n_time_of_level_off) << 16;
        data |= 0 << 31;
        REG_WRITE(RMT_CH0DATA_REG, data);
    }

    REG_WRITE(RMT_CH0DATA_REG, 0);
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_MEM_RD_RST_CH0);
    REG_SET_BIT(RMT_CH0CONF0_REG, RMT_TX_START_CH0);
    rmt_raw_wait();
}

// дочекатися завершення (опитуванням TX_END)
void rmt_raw_wait(void) {
    while (!(REG_READ(RMT_INT_RAW_REG) & RMT_CH0_TX_END_INT_RAW)) {
        vTaskDelay(500);
    }
    
    REG_WRITE(RMT_INT_CLR_REG, 1 << RMT_CH0_TX_END_INT_CLR);
}
