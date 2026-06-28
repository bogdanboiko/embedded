#include "traffic_config.h"

static const traffic_phase_config_t state_configs[] = {
    [TRAFFIC_NS_RED_YELLOW] = {.duration_ms = 4000,
                               .lights_config = ((RED | YELLOW) << NS_LIGHTS_BASE) |
                                   (RED << EW_LIGHTS_BASE)},
    [TRAFFIC_NS_GREEN] = {.duration_ms = 4000,
                          .lights_config = ((GREEN) << NS_LIGHTS_BASE) |
                                           (RED << EW_LIGHTS_BASE)},
    [TRAFFIC_NS_GREEN_BLINKING] = {.duration_ms = 4000,
                                   .lights_config = ((BLINKING | GREEN) << NS_LIGHTS_BASE) |
                                       (RED << EW_LIGHTS_BASE)},
    [TRAFFIC_NS_YELLOW] = {.duration_ms = 4000,
                           .lights_config = (YELLOW << NS_LIGHTS_BASE) |
                                            (RED << EW_LIGHTS_BASE)},
    [TRAFFIC_ALL_RED_NS_TO_EW] = {.duration_ms = 4000,
                                  .lights_config = (RED << NS_LIGHTS_BASE) |
                                                   (RED << EW_LIGHTS_BASE)},
    [TRAFFIC_EW_RED_YELLOW] = {.duration_ms = 4000,
                               .lights_config = (RED << NS_LIGHTS_BASE) |
                                   ((RED | YELLOW) << EW_LIGHTS_BASE)},
    [TRAFFIC_EW_GREEN] = {.duration_ms = 4000,
                          .lights_config = (RED << NS_LIGHTS_BASE) |
                                           (GREEN << EW_LIGHTS_BASE)},
    [TRAFFIC_EW_GREEN_BLINKING] = {.duration_ms = 4000,
                                   .lights_config = (RED << NS_LIGHTS_BASE) |
                                       ((BLINKING | GREEN) << EW_LIGHTS_BASE)},
    [TRAFFIC_EW_YELLOW] = {.duration_ms = 4000,
                           .lights_config = (RED << NS_LIGHTS_BASE) |
                                            (YELLOW << EW_LIGHTS_BASE)},
    [TRAFFIC_ALL_RED_EW_TO_NS] = {.duration_ms = 4000,
                                  .lights_config = (RED << NS_LIGHTS_BASE) |
                                                   (RED << EW_LIGHTS_BASE)},
};

traffic_phase_config_t get_state_config(traffic_state_t state) {
    return state_configs[state];
}