#include "fruits.h"

#define FRUITS_NUM 7
#define MIN_CALIBRATION_SAMPLE_RATE 32
#define MIN_TOUCH_DELTA 1500
#define TOUCH_POWER_RES 4095.0
#define MAX_TOUCH_RAW 10000.0

typedef struct {
    touch_pad_t pad;
    const char* name;
    uint32_t baseline;
    float ema;
    int delta;
    bool is_active;
} touch_pad_entry_t;

static touch_pad_entry_t touch_pad_entries[FRUITS_NUM] = {
    {TOUCH_PAD_NUM7, "1", 0, 0.0f, 0, false},
    {TOUCH_PAD_NUM8, "2", 0, 0.0f, 0, false},
    {TOUCH_PAD_NUM10, "3", 0, 0.0f, 0, false},
    {TOUCH_PAD_NUM11, "4", 0, 0.0f, 0, false},
    {TOUCH_PAD_NUM12, "5", 0, 0.0f, 0, false},
    {TOUCH_PAD_NUM13, "6", 0, 0.0f, 0, false},
    {TOUCH_PAD_NUM14, "7", 0, 0.0f, 0, false},
};

static TaskHandle_t sensors_task_handler = NULL;

void calibrate_fruits_baseline() {
    vTaskDelay(pdMS_TO_TICKS(150));

    for (int i = 0; i < FRUITS_NUM; i++) {
        touch_pad_entry_t* entry = &touch_pad_entries[i];

        int buffer = 0;
        uint32_t touch_value = 0;
        for (int n = 0; n < MIN_CALIBRATION_SAMPLE_RATE; n++) {
            touch_value = 0;
            touch_pad_read_raw_data(entry->pad, &touch_value);
            buffer += touch_value;
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        entry->baseline = (float)buffer / MIN_CALIBRATION_SAMPLE_RATE;
        entry->ema = entry->baseline;
        ESP_LOGI("Test",
                 "Touch pad with name: %s, has baseline: %d, init ema: %f",
                 entry->name, entry->baseline, entry->ema);
    }
}

void fruits_monitor_task(void* args) {
    while (1) {
        for (int i = 0; i < FRUITS_NUM; i++) {
            touch_pad_entry_t* entry = &touch_pad_entries[i];
            ESP_LOGI("Test", "Touch pad with name: %s, current delta is: %d",
                     entry->name, entry->delta);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int get_touch_delta_for_entry(touch_pad_entry_t* entry) {
    int delta = entry->ema - entry->baseline;
    return delta > 0 ? delta : 0;
}

void fruits_polling_task(void* args) {
    calibrate_fruits_baseline();

    while (1) {
        for (int i = 0; i < FRUITS_NUM; i++) {
            touch_pad_entry_t* entry = &touch_pad_entries[i];

            uint32_t raw = 0;
            touch_pad_read_raw_data(entry->pad, &raw);

            entry->ema = (entry->ema * 0.7) + (raw * 0.3);

            int delta = get_touch_delta_for_entry(entry);
            entry->delta = delta;
            entry->is_active = delta > MIN_TOUCH_DELTA;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void fruits_init(bool enable_debug_monitoring) {
    touch_pad_init();

    for (int i = 0; i < FRUITS_NUM; i++) {
        touch_pad_config(touch_pad_entries[i].pad);
        touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    }

    touch_pad_fsm_start();
    xTaskCreate(fruits_polling_task, "fruits_polling_task", 4096, NULL, 10,
                &sensors_task_handler);

    if (enable_debug_monitoring) {
        xTaskCreate(fruits_monitor_task, "fruits_monitoring_task", 2048, NULL,
                    10, NULL);
    }
}

uint16_t map_delta_to_touch_power(int delta) { return 0; }

bool fruits_is_touching(void) {
    for (int i = 0; i < FRUITS_NUM; i++) {
        touch_pad_entry_t* entry = &touch_pad_entries[i];

        if (entry->is_active) {
            return true;
        }
    }

    return false;
}

uint16_t fruits_read_pot(void) {
    uint16_t max_touch_power = 0;

    for (int i = 0; i < FRUITS_NUM; i++) {
        touch_pad_entry_t* entry = &touch_pad_entries[i];

        if (entry->is_active) {
            int buffer = (float)entry->delta * TOUCH_POWER_RES / MAX_TOUCH_RAW;
            uint16_t entry_touch_power =
                buffer < TOUCH_POWER_RES ? buffer : TOUCH_POWER_RES;

            if (entry_touch_power > max_touch_power) {
                max_touch_power = entry_touch_power;
            }
        }
    }

    return max_touch_power;
}

int fruits_active_index(void) {
    int active_fruit_index = -1;
    int max_delta = -1;

    for (int i = 0; i < FRUITS_NUM; i++) {
        touch_pad_entry_t* entry = &touch_pad_entries[i];

        if (entry->is_active && entry->delta >= max_delta) {
            active_fruit_index = i;
        }
    }

    return active_fruit_index;
}

char* fruits_name(int i) { return touch_pad_entries[i].name; }

int fruits_count(void) { return FRUITS_NUM; }