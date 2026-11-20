#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pico/cyw43_arch.h"

const uint16_t cap_program_instructions[] = {
    0x80a0, // 0: pull block
    0xe020, // 1: set x, 0
    0xe080, // 2: set pindirs, 0
    0x0044, // 3: jmp x--, 4
    0x00c6, // 4: jmp pin, 6
    0x0003, // 5: jmp 3
    0xe081, // 6: set pindirs, 1
    0xa0c1, // 7: mov isr, x
    0x8020, // 8: push
};

const struct pio_program cap_program = {
    .instructions = cap_program_instructions,
    .length = 9,
    .origin = 0,
};

uint64_t mittaa_kapasitanssi(PIO pio, uint sm, uint32_t kierrokset) {
    uint64_t summa = 0;
    for (uint32_t i = 0; i < kierrokset; i++) {
        pio_sm_put_blocking(pio, sm, 1);
        uint32_t tulos = pio_sm_get_blocking(pio, sm);
        summa += (0xFFFFFFFF - tulos);
        sleep_us(1);
    }
    return summa;
}

// Siirtofunktio: muuntaa raakalukeman (N) etäisyydeksi [cm]
float laske_etaisyys_cm(int64_t arvo) {
    const float lukemat[]    = {2500.0f, 15000.0f, 25000.0f, 100000.0f, 200000.0f, 400000.0f, 6700000.0f};
    const float etaisyydet[] = {20.0f,   17.5f,    15.0f,    10.0f,     5.0f,      3.0f,      0.0f};
    const int pisteita = 7;

    if (arvo <= lukemat[0]) return 20.0f;
    if (arvo >= lukemat[pisteita - 1]) return 0.0f;

    for (int i = 0; i < pisteita - 1; i++) {
        if (arvo >= lukemat[i] && arvo <= lukemat[i + 1]) {
            float osuus = ((float)arvo - lukemat[i]) / (lukemat[i + 1] - lukemat[i]);
            return etaisyydet[i] + osuus * (etaisyydet[i + 1] - etaisyydet[i]);
        }
    }
    return 0.0f;
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        return -1;
    }
    bool led_tila = false;

    PIO pio = pio1;
    uint sm = 0;
    uint offset = pio_add_program(pio, &cap_program);

    pio_gpio_init(pio, 15);
    gpio_pull_up(15);

    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset, offset + 8);
    sm_config_set_set_pins(&c, 15, 1);
    sm_config_set_jmp_pin(&c, 15);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_pins_with_mask(pio, sm, 0, (1u << 15));
    pio_sm_set_enabled(pio, sm, true);

    const uint32_t KIERROKSET = 100000;

    // Alkukalibrointi lepotilassa
    sleep_ms(1000);
    uint64_t kalibrointi_summa = 0;
    for (int i = 0; i < 10; i++) {
        kalibrointi_summa += mittaa_kapasitanssi(pio, sm, KIERROKSET);
    }
    uint64_t perustaso = (kalibrointi_summa / 10);

    while (true) {
        uint64_t raaka_arvo = mittaa_kapasitanssi(pio, sm, KIERROKSET);
        int64_t arvo = (int64_t)raaka_arvo - (int64_t)perustaso;
        if (arvo < 0) arvo = 0;

        float etaisyys = laske_etaisyys_cm(arvo);

        printf("Lukema: %7lld | Etaisyys: %4.1f cm\n", arvo, etaisyys);

        if (etaisyys > 5.0f) {
            led_tila = !led_tila;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_tila);
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        }
        sleep_ms(100);
    }
}