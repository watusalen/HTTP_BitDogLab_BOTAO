#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "lwip/altcp_tls.h"
#include "example_http_client_util.h"

#define WIFI_SSID "Lapec_Professores" // O Wi-Fi SSID deve ser definido aqui
#define WIFI_PASS "w1q2e3r4" // A senha do Wi-Fi deve ser definida aqui
#define HOST "192.168.31.116" // O host deve ser definido aqui. O server.py vai rodar na mesma rede local que o Raspberry Pi Pico W
#define PORT 5000
#define BUTTON_PIN 5
#define DEBOUNCE_DELAY_MS 200

// Variáveis globais utilizadas para controle do botão
volatile bool button_toggle_flag = false;  // Indica mudança de estado
volatile bool button_state = false;        // Armazena estado atual do botão (1: pressionado, 0: solto)
absolute_time_t last_interrupt_time;       // Armazena o tempo da última interrupção para debouncing

// Envia os dados do botão e temperatura via GET para o servidor
void send_data(bool state, float temperature) {
    char url[256];
    snprintf(url, sizeof(url), "/data?button=%d&temp=%.2f", state, temperature); // Monta URL com parâmetros

    EXAMPLE_HTTP_REQUEST_T req = {0};  // Estrutura de requisição HTTP
    req.hostname = HOST;
    req.url = url;
    req.port = PORT;
    req.headers_fn = http_client_header_print_fn;
    req.recv_fn = http_client_receive_print_fn;

    printf("Enviando: %s\n", url);  // Log de debug
    http_client_request_sync(cyw43_arch_async_context(), &req);  // Envia a requisição síncrona
}

// Faz leitura da temperatura interna com filtragem (média de 32 amostras)
float read_onboard_temperature() {
    const float conv = 3.3f / (1 << 12);  // Conversão de 12 bits para tensão (0-3.3V)
    const int samples = 32;               // Número de amostras para média
    uint32_t sum = 0;

    adc_select_input(4);  // Canal 4 corresponde ao sensor de temperatura interno

    for (int i = 0; i < samples; ++i) {
        sum += adc_read();
        sleep_us(10);  // Delay curto entre leituras
    }

    float avg = (float)sum / samples;
    float voltage = avg * conv;
    return 27.0f - (voltage - 0.706f) / 0.001721f;  // Fórmula do datasheet do RP2040
}

// Interrupção disparada na borda do botão (subida ou descida)
void gpio_callback(uint gpio, uint32_t events) {
    absolute_time_t now = get_absolute_time();
    if (absolute_time_diff_us(last_interrupt_time, now) > DEBOUNCE_DELAY_MS * 1000) {
        last_interrupt_time = now;
        button_state = !gpio_get(BUTTON_PIN);  // Pressionado = 0 (pull-up ativado)
        button_toggle_flag = true;
    }
}

// Configura interrupção no pino do botão
void setup_button_interrupt() {
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
    last_interrupt_time = get_absolute_time();
    gpio_set_irq_enabled_with_callback(BUTTON_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

// Inicializa o ADC do sensor de temperatura
void setup_adc() {
    adc_init();
    adc_select_input(4);  // Sensor interno de temperatura
}

// Conecta à rede Wi-Fi
bool connect_wifi() {
    if (cyw43_arch_init()) return false;  // Inicializa chip Wi-Fi
    cyw43_arch_enable_sta_mode();         // Habilita modo Station
    printf("Conectando ao Wi-Fi...\n");
    return cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
        CYW43_AUTH_WPA2_AES_PSK, 10000) == 0;  // Conecta à rede
}

// Função principal
int main() {
    stdio_init_all();            // Inicializa comunicação padrão via USB
    setup_adc();                 // Configura o ADC
    setup_button_interrupt();   // Ativa interrupção para botão

    if (!connect_wifi()) {
        printf("Falha na conexão Wi-Fi\n");
        return 1;
    }

    // Mostra IP após conexão
    printf("Conectado! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

    // Loop principal
    while (true) {
        if (button_toggle_flag) {
            button_toggle_flag = false;
            float temp = read_onboard_temperature();
            printf("Botão estado: %s, Temp: %.2f°C\n", button_state ? "LIGADO" : "DESLIGADO", temp);
            send_data(button_state, temp);  // Envia dados ao servidor
        }
        sleep_ms(10);  // Polling leve
    }

    cyw43_arch_deinit();  // Nunca será chamado, mas é boa prática
    return 0;
}