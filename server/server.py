from flask import Flask, request, jsonify, render_template_string
from flask_cors import CORS
import time

app = Flask(__name__)
CORS(app)

# Dados mais recentes
latest_data = {
    "button": False,
    "temp": 0.0,
    "timestamp": time.time()
}

HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>Monitor Pico W</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600&display=swap" rel="stylesheet">
    <style>
        body {
            font-family: 'Inter', sans-serif;
            background: #f9f9f9;
            margin: 0;
            padding: 30px;
        }
        .container {
            max-width: 500px;
            background: white;
            margin: 0 auto;
            padding: 25px 30px;
            border-radius: 12px;
            box-shadow: 0 0 12px rgba(0,0,0,0.08);
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 25px;
        }
        .status {
            padding: 20px;
            margin-bottom: 15px;
            border-radius: 8px;
            font-size: 1.1em;
            font-weight: 500;
        }
        .button-on {
            background-color: #e1fbe1;
            color: #267326;
        }
        .button-off {
            background-color: #ffe6e6;
            color: #cc0000;
        }
        .temp-high {
            color: #d80000;
        }
        .temp-normal {
            color: #0055cc;
        }
        .footer {
            font-size: 0.9em;
            color: #666;
            text-align: center;
            margin-top: 15px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Monitor de Hardware (Pico W)</h1>

        <div id="button-status" class="status button-off">
            <strong>Botão:</strong> <span id="button-state">DESLIGADO</span>
        </div>

        <div id="temp-status" class="status">
            <strong>Temperatura:</strong> <span id="temp-value">0.0</span>°C
        </div>

        <div class="status">
            <strong>Última atualização:</strong> <span id="last-update">Nunca</span>
        </div>
    </div>

    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    const buttonState = document.getElementById('button-state');
                    const buttonDiv = document.getElementById('button-status');
                    if (data.button) {
                        buttonState.textContent = "LIGADO";
                        buttonDiv.className = "status button-on";
                    } else {
                        buttonState.textContent = "DESLIGADO";
                        buttonDiv.className = "status button-off";
                    }

                    const tempElement = document.getElementById('temp-value');
                    tempElement.textContent = data.temp.toFixed(2);
                    tempElement.className = data.temp > 30 ? "temp-high" : "temp-normal";

                    const now = new Date(data.timestamp * 1000);
                    document.getElementById('last-update').textContent = now.toLocaleTimeString();
                });
        }

        setInterval(updateData, 1000);
        updateData();
    </script>
</body>
</html>
"""


@app.route("/data", methods=["GET"])
def get_data():
    # Se receber novos dados via GET, atualiza
    if 'button' in request.args:
        latest_data['button'] = request.args.get('button', '0') == '1'
    if 'temp' in request.args:
        latest_data['temp'] = float(request.args.get('temp', '0.0'))
    latest_data['timestamp'] = time.time()
    
    print(f"Recebido - Botão: {latest_data['button']}, Temp: {latest_data['temp']}°C")
    return jsonify(latest_data)

@app.route("/", methods=["GET"])
def index():
    return render_template_string(HTML_TEMPLATE)

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)