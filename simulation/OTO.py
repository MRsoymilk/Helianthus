import os
import pandas as pd
from flask import Flask, jsonify

app = Flask(__name__)

# 数据目录
DATA_DIR = "./oto"

# 生成模拟波长数组（假设1024个通道，波长从950到1700nm线性分布）
def generate_wavelengths():
    channels = 1024
    start_wl = 950.0
    end_wl = 1700.0
    step = (end_wl - start_wl) / (channels - 1)
    return [start_wl + i * step for i in range(channels)]

WAVELENGTHS = generate_wavelengths()

# -------- 预加载所有光谱列 --------
ALL_CURVES = []   # [(curve, filename, col), ...]
CURVE_INDEX = 0   # 当前发送到第几列

def preload_curves():
    global ALL_CURVES
    files = [f for f in os.listdir(DATA_DIR) if f.endswith(".csv")]
    if not files:
        raise RuntimeError("目录下没有CSV文件")

    for f in files:
        file_path = os.path.join(DATA_DIR, f)
        df = pd.read_csv(file_path)

        cols = [c for c in df.columns if c.startswith("vol24_")]
        for col in cols:
            curve = df[col].tolist()
            ALL_CURVES.append((curve, file_path, col))

    if not ALL_CURVES:
        raise RuntimeError("没有找到任何 vol24_x 列")

preload_curves()


@app.route("/info")
def info():
    return jsonify({"status": "ok", "model_name": "Simulated_Spectrometer"})


@app.route("/wavelengths")
def wavelengths_route():
    return jsonify({"status": "ok", "wavelengths": WAVELENGTHS})


@app.route("/spectrum")
def spectrum_route():
    """
    顺序返回完整的一组光谱列，每次调用依次取下一列
    """
    global CURVE_INDEX
    try:
        curve, filename, col = ALL_CURVES[CURVE_INDEX]
        data = [{wl: inten} for wl, inten in zip(WAVELENGTHS, curve)]

        # 更新索引，循环使用
        CURVE_INDEX = (CURVE_INDEX + 1) % len(ALL_CURVES)

        return jsonify({
            "status": "ok",
            "source_file": filename,
            "column": col,
            "spectrum": data
        })
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=9000)

