import random
from flask import Flask, jsonify, request

app = Flask(__name__)

# 生成模拟波长数组（假设1024个通道，波长从400到800nm线性分布）
def generate_wavelengths():
    channels = 1024
    start_wl = 400.0
    end_wl = 800.0
    step = (end_wl - start_wl) / (channels - 1)
    return [start_wl + i * step for i in range(channels)]

WAVELENGTHS = generate_wavelengths()

@app.route("/info")
def info():
    return jsonify({"status": "ok", "model_name": "Simulated_Spectrometer"})

@app.route("/wavelengths")
def wavelengths_route():
    return jsonify({"status": "ok", "wavelengths": WAVELENGTHS})

@app.route("/spectrum")
def spectrum_route():
    # 模拟积分时间和平均数参数，但不实际使用
    integration_time = int(request.args.get("integration_time_us", 100000))
    average = int(request.args.get("average", 1))
    # 生成模拟强度，随机波动叠加一个峰值
    intensities = []
    peak_center = 600  # nm
    peak_width = 20
    peak_height = 10000
    for wl in WAVELENGTHS:
        base = random.uniform(100, 200)
        # 简单高斯峰模拟
        gauss = peak_height * (2.71828 ** (-((wl - peak_center) ** 2) / (2 * peak_width ** 2)))
        val = base + gauss
        intensities.append(round(val, 2))
    data = [{wl: inten} for wl, inten in zip(WAVELENGTHS, intensities)]
    return jsonify({"status": "ok", "spectrum": data})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=9000)

