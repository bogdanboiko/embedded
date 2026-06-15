#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "c4mcu";

/* глобальні дані в різних сегментах */
int        g_data = 42;     /* .data   -> DRAM (+копія у Flash) */
int        g_bss;           /* .bss    -> DRAM                  */
const int  g_ro   = 7;      /* .rodata -> Flash (DROM)          */


/* ===== ДЕМО 1: робота з вказівниками ===== */
void demo_pointers(void)
{
    int   x = 25;
    int  *p = &x;          /* p зберігає адресу x */

    ESP_LOGI(TAG, "== Демо 1: вказівники ==");
    ESP_LOGI(TAG, "x        = %d", x);
    ESP_LOGI(TAG, "&x       = %p", (void *)&x);    /* адреса x */
    ESP_LOGI(TAG, "p        = %p", (void *)p);     /* те саме */
    ESP_LOGI(TAG, "*p       = %d", *p);            /* значення за адресою */

    *p = 99;                                       /* запис через вказівник */
    ESP_LOGI(TAG, "після *p = 99  ->  x = %d", x); /* x теж змінилася */

    ESP_LOGI(TAG, "sizeof(x) = %u, sizeof(p) = %u",
             (unsigned)sizeof(x), (unsigned)sizeof(p));

    int arr[3] = {10, 20, 30};
    ESP_LOGI(TAG, "&arr[0]  = %p", (void *)&arr[0]);
    ESP_LOGI(TAG, "&arr[1]  = %p", (void *)&arr[1]); /* +4 байти */
    ESP_LOGI(TAG, "*(arr+2) = %d", *(arr + 2));      /* = arr[2] = 30 */

    int *nul = NULL;                               /* нікуди не вказує */
    ESP_LOGI(TAG, "nul      = %p", (void *)nul);
}


/* ===== ДЕМО 2: де живе кожна змінна ===== */
void demo_addresses(void)
{
    int   local = 1;                     /* стек */
    int  *heap  = malloc(sizeof *heap);  /* купа */

    ESP_LOGI(TAG, "== Демо 2: адреси сегментів ==");
    ESP_LOGI(TAG, ".text    func  : %p", (void *)&demo_addresses); /* IROM  ~0x42.. */
    ESP_LOGI(TAG, ".rodata  g_ro  : %p", (void *)&g_ro);           /* DROM  ~0x3C.. */
    ESP_LOGI(TAG, ".data    g_data: %p", (void *)&g_data);         /* DRAM  ~0x3FC. */
    ESP_LOGI(TAG, ".bss     g_bss : %p", (void *)&g_bss);          /* DRAM         */
    ESP_LOGI(TAG, "heap     *heap : %p", (void *)heap);            /* DRAM (купа)  */
    ESP_LOGI(TAG, "stack    local : %p", (void *)&local);          /* DRAM (стек задачі) */

    free(heap);
}


/* ===== ДЕМО 3: стек росте вниз ===== */
void recurse(int depth)
{
    int marker;
    // int big[32];
    // big[0] = depth;
    ESP_LOGI(TAG, "depth=%d  &marker=%p", depth, (void *)&marker);
    if (depth < 8)
        recurse(depth + 1);
}

void demo_stack(void)
{
    ESP_LOGI(TAG, "== Демо 3: стек росте вниз ==");
    recurse(0);
}


/* ===== ДЕМО 4: фрагментація на «арені» 16 байт ===== */
#define ARENA 16
static char arena[ARENA];

void show_arena(void)
{
    char buf[ARENA + 1];
    for (int i = 0; i < ARENA; i++)
        buf[i] = arena[i] ? arena[i] : '.';
    buf[ARENA] = '\0';
    ESP_LOGI(TAG, "%s", buf);
}

int my_alloc(char tag, int size)
{
    for (int i = 0; i + size <= ARENA; i++) {
        int free_here = 1;
        for (int j = 0; j < size; j++)
            if (arena[i + j]) { free_here = 0; break; }
        if (free_here) {
            for (int j = 0; j < size; j++) arena[i + j] = tag;
            return i;
        }
    }
    return -1;                 /* немає size байтів ПІДРЯД */
}

void my_free(char tag)
{
    for (int i = 0; i < ARENA; i++)
        if (arena[i] == tag) arena[i] = 0;
}

void demo_fragmentation(void)
{
    ESP_LOGI(TAG, "== Демо 4: фрагментація (модель) ==");
    memset(arena, 0, ARENA);

    my_alloc('A', 4);
    my_alloc('B', 4);
    my_alloc('C', 4);
    show_arena();                       /* AAAABBBBCCCC.... */

    my_free('C');
    show_arena();                       /* AAAA....CCCC.... */

    int r = my_alloc('D', 6);
    show_arena();
    ESP_LOGI(TAG, "my_alloc(6) -> %s", r < 0 ? "FAIL" : "ok");
}


/* ===== ДЕМО 5: реальна купа ESP32-S3 ===== */
void demo_heap_real(void)
{
    ESP_LOGI(TAG, "== Демо 5: реальна купа ESP32 ==");
    ESP_LOGI(TAG, "вільно (сума)   : %u", (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT));
    ESP_LOGI(TAG, "найбільший блок : %u", (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    void *p[8];
    for (int i = 0; i < 8; i++) p[i] = malloc(4096);
    for (int i = 0; i < 8; i += 1) free(p[i]);     /* звільняємо через один */

    ESP_LOGI(TAG, "після шахматних free:");
    ESP_LOGI(TAG, "вільно (сума)   : %u", (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT));
    ESP_LOGI(TAG, "найбільший блок : %u", (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

    // for (int i = 1; i < 8; i += 2) free(p[i]);     /* прибираємо за собою */
}


/* ----------------------------------------------------------
 * ПСЕВДОКОД: шлях до app_main() на ESP32-S3
 *   ROM-завантажувач -> 2-й завантажувач (bootloader)
 *   -> startup: copy .data, zero .bss, кеш Flash
 *   -> старт FreeRTOS -> задача main -> app_main();
 * ---------------------------------------------------------- */

/* ===== ДЕМО 1: кільцевий буфер (контейнер без malloc) ===== */
#define RB_SIZE 4
static uint8_t  rb_buf[RB_SIZE];     /* фіксований буфер, .bss -> RAM */
static uint16_t rb_head, rb_tail;

static bool rb_push(uint8_t v)
{
    uint16_t next = (rb_head + 1u) % RB_SIZE;
    if (next == rb_tail) return false;      /* буфер повний */
    rb_buf[rb_head] = v;
    rb_head = next;
    return true;
}

static bool rb_pop(uint8_t *v)
{
    if (rb_tail == rb_head) return false;   /* буфер порожній */
    *v = rb_buf[rb_tail];
    rb_tail = (rb_tail + 1u) % RB_SIZE;
    return true;
}

void demo_ringbuf(void)
{
    ESP_LOGI(TAG, "== Демо 1: кільцевий буфер (без malloc) ==");
    rb_head = rb_tail = 0;
    int pushCounter = 0;

    for (uint8_t v = 1; v <= 12; v++) {
        if (pushCounter < 3) {
            ESP_LOGI(TAG, "push %3d -> %s", v * 10, rb_push(v * 10) ? "ok" : "FULL");
            pushCounter++;
        } else {
            uint8_t out;
            rb_pop(&out);
            ESP_LOGI(TAG, "pop  %3d", out);
            pushCounter = 0;
        }
    }
}


/* ===== ДЕМО 2: superloop + state machine (без блокувань) ===== */
typedef enum { ST_WAIT, ST_LED_ON, ST_PAUSE, ST_LED_OFF, ST_DONE } st_t;

void demo_state_machine(void)
{
    ESP_LOGI(TAG, "== Демо 2: superloop + state machine ==");
    st_t st = ST_WAIT;
    int blinks = 0;
    for (int tick = 0; tick < 10; tick++) {     /* імітація 10 проходів суперлупа */
        switch (st) {
            case ST_WAIT:    ESP_LOGI(TAG, "tick %d: WAIT",   tick); st = ST_LED_ON;  break;
            case ST_LED_ON:  ESP_LOGI(TAG, "tick %d: LED ON", tick); st = ST_PAUSE; break;
            case ST_PAUSE:    ESP_LOGI(TAG, "tick %d: PAUSE",   tick); st = ST_LED_OFF;  break;
            case ST_LED_OFF: ESP_LOGI(TAG, "tick %d: LED OFF",tick);
                             st = (++blinks >= 3) ? ST_DONE : ST_LED_ON; break;
            case ST_DONE:    ESP_LOGI(TAG, "tick %d: DONE",   tick); break;
        }
    }
}

/* ===== ДЕМО 3: volatile — прапорець від таймера (як ISR) ===== */
static volatile sig_atomic_t tick_flag = 0;

static void on_timer(void *arg)
{
    (void)arg;
    tick_flag = 1;            /* ставимо прапорець «ззовні» основного циклу */
}

void demo_volatile(void)
{
    ESP_LOGI(TAG, "== Демо 3: volatile (прапорець від таймера) ==");
    const esp_timer_create_args_t args = { .callback = on_timer, .name = "tick" };
    esp_timer_handle_t timer;
    esp_timer_create(&args, &timer);
    esp_timer_start_periodic(timer, 100000);    /* 100 мс */

    int ticks = 0;
    while (ticks < 5) {                          /* суперлуп, що реагує на прапорець */
        if (tick_flag) {
            tick_flag = 0;
            ESP_LOGI(TAG, "tick %d (прапорець помічено)", ++ticks);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    esp_timer_stop(timer);
    esp_timer_delete(timer);
}


/* ===== ДЕМО 4: константи й розміщення у Flash ===== */
#define LUT_SIZE 4
enum { CH_R = 0, CH_G = 1, CH_B = 2 };
// const int N = 4;
// static uint8_t test[N]; 
static uint8_t gamma_lut[LUT_SIZE] = { 0, 3, 9, 27 };  /* .rodata -> Flash */
static uint8_t       work_buf[LUT_SIZE];                     /* .bss    -> RAM   */

void demo_constants(void)
{

    ESP_LOGI(TAG, "== Демо 4: константи й Flash ==");
    ESP_LOGI(TAG, "enum CH_G       = %d", CH_G);
    ESP_LOGI(TAG, "#define LUT_SIZE = %d", LUT_SIZE);
    ESP_LOGI(TAG, "&gamma_lut (Flash ~0x3C/0x42): %p", (void *)gamma_lut);
    ESP_LOGI(TAG, "&work_buf  (RAM   ~0x3FC):     %p", (void *)work_buf);
}


/* ===== ДЕМО 5: ініціалізація (designated init + lazy static) ===== */
struct gpio_config { uint32_t dir; uint32_t out; uint8_t pin; uint32_t speed;};

static const struct gpio_config led_cfg = {     /* лежить у Flash */
    .dir = 1u << 5,
    .out = 0,
    .pin = 5,
};

static void apply_config(const struct gpio_config *c)
{
    ESP_LOGI(TAG, "apply: speed=%d pin=%u dir=0x%08x out=0x%08x",
             c->speed, c->pin, (unsigned)c->dir, (unsigned)c->out);
}

static void lazy_feature(void)
{
    static bool inited = false;     /* УВАГА: у C цей патерн НЕ потокобезпечний */
    if (!inited) {
        ESP_LOGI(TAG, "lazy: ініціалізація (один раз)");
        inited = true;
    }
    ESP_LOGI(TAG, "lazy: використання");
}

void demo_init(void)
{
    ESP_LOGI(TAG, "== Демо 5: ініціалізація ==");
    apply_config(&led_cfg);
    lazy_feature();
    lazy_feature();      /* друге — вже без повторної ініціалізації */
}

void demo_hwB2(void) {
    int a = 5;
    int b = 9;

    ESP_LOGI(TAG, "before: a: %d, b: %d", a, b);
    int *p1 = &a;
    int *p2 = &b;
    int buf;
    buf = *p2;
    *p2 = *p1;
    *p1 = buf;

    ESP_LOGI(TAG, "after: a: %d, b: %d", a, b);
}

void demo_hwB3(void) {
    int arr[5] = {10, 20, 30, 40, 50};

    for (int i = 0; i < 5; i++) {
        ESP_LOGI(TAG, "array item %d address : %p", i, &arr[i]);
    }
}

void demo_hwB4(void) {
    int arr[5] = {10, 20, 30, 40, 50};

    ESP_LOGI(TAG, "arr %p &arr: %p", arr, &arr[0]);
    ESP_LOGI(TAG, "*(arr + 2) %p &arr[2]: %p", *(arr + 2), &arr[2]);
    ESP_LOGI(TAG, "(arr + 4) - (arr + 1): %p", (arr + 4) - (arr + 1));
}

void demo_hwB5(void) {
    int arr[5] = {10, 20, 30, 40, 50};

    int sum1 = 0;
    for(int i = 0; i < 5; i++) {
        sum1 += arr[i];
    }

    ESP_LOGI(TAG, "sum1 = %d", sum1);

    int sum2 = 0;
    for(int *q = arr; q < arr + 5; q++) {
        sum2 += *q;
    }

    ESP_LOGI(TAG, "sum2 = %d", sum2);
}

void demo_hwB6(void) {
    char s[] = "ESP32-S3";
    
    int length = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        length++;
    }
    
    ESP_LOGI(TAG, "string length = %d; s: %p; &s[0]: %p", length, s, &s[0]);

}

void fill(int *buf, int n, int value) {
    for (int i = 0; i < n; i++) {
        *(buf + i) = value;
    }
}

void demo_hwB7(void) {
    int arr[5] = {10, 20, 30, 40, 50};
    
    for(int i = 0; i < 5; i++) {
        ESP_LOGI(TAG, "array item %d before: %d", i, arr[i]);
    }

    fill(arr, 5, 12);

    for(int i = 0; i < 5; i++) {
        ESP_LOGI(TAG, "array item %d after: %d", i, arr[i]);
    }
}

void get_min_max(const int *arr, int n, int *min, int *max) {
    for (int i = 0; i < n; i++) {
        if (arr[i] < *min) {
            *min = arr[i];
        }

        if (arr[i] > *max) {
            *max = arr[i];
        }
    }
}

void demo_hwB8(void) {
    int arr[7] = {-10, 2, 7, -40, 35, 11, 0};

    int min = arr[0];
    int max = arr[0];
    get_min_max(arr, 7, &min, &max);
    ESP_LOGI(TAG, "array min: %d max: %d", min, max);
}

struct point {
    short x;
    short y;
};

typedef struct point {
    short x;
    short y;
} point;

void demo_hwB9(void) {
    struct point pts[3];
    for (struct point *p = pts; p < pts + 3; p++) {
        p->x = 1;
        p->y = 1;
    }

    ESP_LOGI(TAG, "&pts[0]: %p &pts[1]: %p, size of point: %d", &pts[0], &pts[1], sizeof(struct point));
}

void app_main(void)
{
    // demo_hwB2();
    // demo_hwB3();
    // demo_hwB4();
    // demo_hwB5();
    // demo_hwB6();
    // demo_hwB7();
    // demo_hwB8();
    // demo_hwB9();
    // demo_pointers();
    // demo_addresses();
    // demo_stack();
    // demo_fragmentation();
    // demo_heap_real();
    // demo_ringbuf();
    // demo_state_machine();
    // demo_volatile();
    // demo_constants();
    // demo_init();
}