# Monitoramento de Botão e Temperatura com Raspberry Pi Pico W

Este projeto demonstra o uso do microcontrolador **Raspberry Pi Pico W** para:
- Monitorar o estado de um **botão físico** com tratamento de debounce por interrupção.
- Medir a **temperatura interna** do chip usando o ADC.
- Enviar os dados via **HTTP GET** para um **servidor Flask** na rede local.
- Exibir os dados em tempo real em uma interface web moderna.

## 📦 Estrutura do Projeto

```
.
├── config/                  # Arquivos de configuração lwIP e mbedTLS
├── server/                 # Servidor Flask com dashboard em HTML/CSS/JS
│   └── server.py
├── src/                    # Código-fonte em C para o Raspberry Pi Pico W
│   ├── example_http_client_util.c/h
│   ├── picow_http_client.c
│   └── picow_http_verify.c
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── requirements.txt        # Dependências Python (Flask)
├── .gitignore
└── README.md
```

---

## 🛠 Requisitos

### Hardware:
- Raspberry Pi Pico W
- Botão físico conectado ao pino **GP5**
- (Opcional) Sensor de temperatura externo no ADC4

### Software:
- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- Python 3.10+
- Pacotes Python: `flask`, `flask-cors`

---

## 🚀 Como Usar

### 1. Compilação do Firmware

```bash
git clone https://github.com/seu-usuario/seu-repo.git
cd seu-repo
mkdir build && cd build
cmake ..
make
```

O firmware `picow_http_client.uf2` será gerado. Carregue-o no Pico W via arrastar/soltar no BOOTSEL.

### 2. Configuração de Wi-Fi

Edite o arquivo `picow_http_client.c`:

```c
#define WIFI_SSID "SeuWiFi"
#define WIFI_PASS "SenhaDoWiFi"
#define HOST "192.168.x.x" // IP do servidor Flask
```

### 3. Servidor Web

#### Criação do ambiente virtual e instalação de dependências:

```bash
python -m venv .venv
source .venv/bin/activate  # Linux/macOS
.venv\Scripts\activate     # Windows

pip install -r requirements.txt
```

#### Execução do servidor Flask:

```bash
python server/server.py
```

Acesse `http://<SEU_IP_LOCAL>:5000` para ver o dashboard com:
- Estado do botão
- Temperatura em °C
- Timestamp da última atualização

---

## 📡 Comunicação HTTP

O Pico W envia os dados via método `GET` no formato:

```
/data?button=1&temp=85.19
```

O servidor Flask atualiza o valor mais recente e serve-o para o frontend HTML dinamicamente usando `fetch()`.

---

## 🧠 Lógica Interna do Pico W

- Usa interrupção em borda de subida e descida para detectar pressões no botão.
- Aplica **debounce** via controle de tempo (`absolute_time_diff_us`).
- Lê a temperatura interna suavizada com média de 32 amostras.
- Envia os dados para o servidor apenas quando o estado do botão muda.
