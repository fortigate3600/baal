import os
from flask import Flask, request, render_template_string

app = Flask(__name__)

# Cartella dove salveremo tutti i file dei vari client
DATI_DIR = "dati_client"

# Assicuriamoci che la cartella esista quando il server parte
if not os.path.exists(DATI_DIR):
    os.makedirs(DATI_DIR)

# La nuova interfaccia HTML aggiornata per mostrare più client
TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <title>Testi Ricevuti</title>
    <style>
        body { font-family: sans-serif; padding: 20px; background-color: #f4f4f9; }
        .client-box { background: white; border: 1px solid #ccc; padding: 15px; margin-bottom: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h2 { color: #2c3e50; margin-top: 0; font-size: 1.2em; border-bottom: 2px solid #3498db; padding-bottom: 5px; }
        ul { padding-left: 20px; color: #555; }
    </style>
</head>
<body>
    <h1>Messaggi Ricevuti dai Client:</h1>

    {% if clients %}
        {% for client, messaggi in clients.items() %}
            <div class="client-box">
                <h2>ID Client: {{ client }}</h2>
                <ul>
                    {% for msg in messaggi %}
                        <li>{{ msg }}</li>
                    {% endfor %}
                </ul>
            </div>
        {% endfor %}
    {% else %}
        <p>Nessun messaggio ricevuto finora.</p>
    {% endif %}
</body>
</html>
"""

# Rotta GET: Legge tutti i file nella cartella e li mostra sulla pagina
@app.route('/', methods=['GET'])
def mostra_testi():
    clients_data = {}

    # Scorre tutti i file presenti nella cartella dati_client
    for filename in os.listdir(DATI_DIR):
        if filename.endswith(".txt"):
            client_name = filename[:-4] # Rimuove l'estensione ".txt" dal nome
            filepath = os.path.join(DATI_DIR, filename)

            with open(filepath, "r") as f:
                # Salva le righe lette nel dizionario associato a quell'utente
                clients_data[client_name] = f.readlines()

    return render_template_string(TEMPLATE, clients=clients_data)

# Rotta POST: Riceve il testo, lo divide e lo salva nel file corretto
@app.route('/invia', methods=['POST'])
def ricevi_testo():
    nuovo_testo = request.form.get('testo')

    if nuovo_testo:
        # Il formato atteso è "username_mguid: TEXT"
        # Usiamo split(':', 1) per dividere la stringa solo al PRIMO due punti
        parti = nuovo_testo.split(':', 1)

        if len(parti) == 2:
            # Rimuoviamo eventuali spazi vuoti extra all'inizio e alla fine
            client_id = parti[0].strip()
            messaggio = parti[1].strip()

            # Per sicurezza, puliamo il nome del file permettendo solo lettere, numeri, _ e -
            client_id = "".join(c for c in client_id if c.isalnum() or c in ('_', '-'))
            if not client_id:
                client_id = "client_sconosciuto"

            # Costruisce il percorso, es: dati_client/mario_12345.txt
            nome_file = os.path.join(DATI_DIR, f"{client_id}.txt")

            # Apre il file in modalità "a" (append). Se non esiste, lo crea in automatico!
            with open(nome_file, "a") as f:
                f.write(messaggio + "\n")

            return f"Testo salvato con successo per {client_id}!", 200

        else:
            # Se il C invia un messaggio senza i due punti, lo mettiamo in un file generico
            nome_file = os.path.join(DATI_DIR, "non_formattati.txt")
            with open(nome_file, "a") as f:
                f.write(nuovo_testo + "\n")
            return "Testo ricevuto, ma mancava il formato 'ID: Testo'. Salvato in non_formattati.", 200

    return "Errore: nessun testo inviato", 400

if __name__ == '__main__':
    app.run(debug=True, port=5000)
