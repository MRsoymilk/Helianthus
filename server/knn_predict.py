from flask import Flask, request, jsonify
import json
import os
import csv
import uuid
import joblib
import numpy as np
import time
import warnings
from sklearn.exceptions import InconsistentVersionWarning

# 忽略版本警告
warnings.filterwarnings("ignore", category=InconsistentVersionWarning)

base_dir = os.path.dirname(os.path.abspath(__file__))

app = Flask(__name__)

MODEL_FILE = os.path.join(base_dir, 'knn_model_0708.pkl')
CSV_FILE = os.path.join(base_dir, "predict_history.csv")

# 预加载模型，避免每次都重新加载
if not os.path.exists(MODEL_FILE):
    raise FileNotFoundError(f"Model file '{MODEL_FILE}' not found")
model = joblib.load(MODEL_FILE)


def extract_features(signal):
    fft_result = np.fft.fft(signal)
    fft_magnitude = np.abs(fft_result)
    freq_features = fft_magnitude[:10]

    mean = np.mean(signal)
    std = np.std(signal)
    max_val = np.max(signal)
    min_val = np.min(signal)

    features = np.concatenate([freq_features, [mean, std, max_val, min_val]])

    norm = np.linalg.norm(features)
    if norm != 0:
        features = features / norm

    return features


def append_to_csv(uuid_str, signal, result):
    is_new = not os.path.exists(CSV_FILE)
    with open(CSV_FILE, 'a', newline='') as f:
        writer = csv.writer(f)
        if is_new:
            writer.writerow(['uuid', 'signal', 'result'])
        writer.writerow([uuid_str, json.dumps(signal), result])


@app.route('/knn_predict', methods=['POST'])
def predict():
    start = time.perf_counter()

    try:
        data = request.get_json()
        signal = data.get("signal", [])

        if not isinstance(signal, list) or not signal:
            return jsonify({"error": "Missing or invalid 'signal'"}), 400

        features = extract_features(signal)
        X = [features]
        result = model.predict(X)[0]

        uuid_str = str(uuid.uuid4())
        append_to_csv(uuid_str, signal, result)

        elapsed_ms = (time.perf_counter() - start) * 1000

        return jsonify({
            "uuid": uuid_str,
            "result": result,
            "elapsed": elapsed_ms
        })

    except Exception as e:
        return jsonify({"error": str(e)}), 500


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5010, debug=False)
