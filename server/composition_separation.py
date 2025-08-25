from flask import Flask, request, jsonify
import numpy as np
import joblib
import os
from scipy.optimize import nnls

app = Flask(__name__)

# -------------------------
# 加载模型和标准曲线
# -------------------------
model = joblib.load("rf_model.pkl")

# 自动加载所有 npy 文件 (约定: avg_curve_xxx.npy)
standard_curves = {}
for fname in os.listdir("curve"):
    if fname.startswith("avg_curve_") and fname.endswith(".npy"):
        key = fname.replace("avg_curve_", "").replace(".npy", "")
        standard_curves[key] = np.load(os.path.join("curve", fname))

print("[INFO] 加载的标准曲线:", list(standard_curves.keys()))


def predict_and_separate_curve(curve, model, standard_curves):
    """
    curve: 1D numpy array, 待预测的混合光谱
    model: 训练好的随机森林回归模型
    standard_curves: dict, {物质名: 1D np.array}
    """
    curve_np = np.array(curve, dtype=float)
    curve_1d = curve_np.reshape(1, -1)

    # -------------------------
    # 模型预测比例
    # -------------------------
    pred_ratio = model.predict(curve_1d)[0]   # shape: (n_substances,)
    pred_ratio_percent = {name: float(r * 100) 
                          for name, r in zip(standard_curves.keys(), pred_ratio)}

    print("[INFO] 预测比例:", pred_ratio_percent)

    # -------------------------
    # NNLS 分离曲线
    # -------------------------
    A = np.vstack(list(standard_curves.values())).T   # shape: (len(curve), n_substances)
    coeffs, _ = nnls(A, curve_np.flatten())

    s = coeffs.sum()
    if s < 1e-8 or np.isnan(s) or all(v < 1e-6 for v in pred_ratio):
        coeffs = np.zeros_like(coeffs)
    else:
        coeffs /= s

    separated_curves = {name: (coeff * spec).tolist() 
                        for (name, spec), coeff in zip(standard_curves.items(), coeffs)}

    return {
        "pred_ratio": pred_ratio_percent,
        "mix_curve": curve_np.flatten().tolist(),
        "separated_curves": separated_curves
    }


# -------------------------
# 接口 1：预测 + 分离
# -------------------------
@app.route("/predict", methods=["POST"])
def predict():
    data = request.json
    signal = data["signal"]

    result = predict_and_separate_curve(signal, model, standard_curves)
    return jsonify(result)


# -------------------------
# 接口 2：标准曲线
# -------------------------
@app.route("/standard", methods=["GET"])
def standard():
    return jsonify({name: curve.tolist() for name, curve in standard_curves.items()})


# -------------------------
# 启动服务
# -------------------------
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5015, debug=True)
