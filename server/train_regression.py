import argparse
import pandas as pd
import numpy as np
import joblib
import matplotlib.pyplot as plt
plt.rcParams['font.family'] = 'Microsoft YaHei'
plt.rcParams['axes.unicode_minus'] = False   # 正确显示负号
from sklearn.ensemble import RandomForestRegressor
import os
import re
from collections import defaultdict
import shutil
import subprocess


# -------------------------
# 解析文件名中的物质比例
# -------------------------
def parse_ratios_from_filename(filename: str):
    """
    从文件名解析出各个物质的比例
    支持不定数量的物质，例如: '糖3_盐2_洗衣粉1.csv'
    返回 dict: {物质: 比例}
    """
    name = os.path.splitext(os.path.basename(filename))[0]

    if "空" in name:
        return {}

    pattern = re.compile(r"([\u4e00-\u9fff]+)(\d+)")
    matches = pattern.findall(name)

    if not matches:
        return {}

    ratios = defaultdict(int)
    total = 0
    for substance, num in matches:
        val = int(num)
        ratios[substance] += val
        total += val

    if total == 0:
        return {}

    return {sub: val / total for sub, val in ratios.items()}


# -------------------------
# 读取 CSV 并提取 vol24_* 列
# -------------------------
def load_vol_columns(path, target_len=None):
    df = pd.read_csv(path)
    vol_cols = [c for c in df.columns if c.startswith("vol24_")]
    matrix = df[vol_cols].to_numpy()
    print(f"[INFO] {path} matrix shape:", matrix.shape)

    if target_len is not None:
        matrix = matrix[:target_len, :]

    return matrix

def copy_dir_overwrite(src_dir, dst_dir, pattern=r"regression_model_.*\.pkl", new_name="regression_model.pkl"):
    """
    递归复制 src_dir 到 dst_dir，如果目标存在则覆盖。
    符合 pattern 的文件会被重命名为 new_name。
    """
    os.makedirs(dst_dir, exist_ok=True)

    for root, dirs, files in os.walk(src_dir):
        rel_path = os.path.relpath(root, src_dir)
        target_root = os.path.join(dst_dir, rel_path) if rel_path != "." else dst_dir
        os.makedirs(target_root, exist_ok=True)

        for file in files:
            src_file = os.path.join(root, file)

            # 判断是否匹配 pattern
            if re.fullmatch(pattern, file):
                dst_file = os.path.join(target_root, new_name)
            else:
                dst_file = os.path.join(target_root, file)

            shutil.copy2(src_file, dst_file)

# -------------------------
# 主程序
# -------------------------
def main(args):
    # 扫描目录下所有 CSV 文件
    csv_files = [os.path.join(args.data_dir, f)
                 for f in os.listdir(args.data_dir) if f.endswith(".csv")]
    if not csv_files:
        raise RuntimeError(f"[ERROR] 目录 {args.data_dir} 下没有找到 CSV 文件")

    # 收集所有物质名
    all_materials = set()
    file_ratios = {}
    for f in csv_files:
        ratios = parse_ratios_from_filename(f)
        file_ratios[f] = ratios
        all_materials.update(ratios.keys())

    all_materials = sorted(all_materials)
    print("[INFO] 检测到物质:", all_materials)

    target_len = args.target_len
    X_list, Y_list = [], []

    # 构建训练集
    for file, ratios in file_ratios.items():
        matrix = load_vol_columns(file, target_len)
        n_samples = matrix.shape[1]
        X_list.append(matrix.T)

        # 将 dict 转为定长向量
        vec = [ratios.get(m, 0) for m in all_materials]
        Y_list.append(np.tile(vec, (n_samples, 1)))

    X_train = np.vstack(X_list)
    Y_train = np.vstack(Y_list)

    print("[INFO] 训练样本形状:", X_train.shape)
    print("[INFO] 标签形状:", Y_train.shape)

    # -------------------------
    # 训练随机森林回归
    # -------------------------
    model = RandomForestRegressor(n_estimators=200, random_state=42, n_jobs=-1)
    model.fit(X_train, Y_train)
    print("[INFO] 训练完成")

    dir_model = os.path.join("output", args.output_model)
    os.makedirs(dir_model, exist_ok=True)
    output_model = os.path.join(dir_model, args.output_model)
    joblib.dump((model, all_materials), output_model)  # 保存物质顺序
    print(f"[INFO] 模型已保存为 {output_model}")

    # -------------------------
    # 保存平均曲线 (每种物质单独样本)
    # -------------------------
    output_curve_dir = os.path.join("output", args.output_curve_dir, "curves")
    os.makedirs(output_curve_dir, exist_ok=True)

    for i, mat in enumerate(all_materials):
        single_files = [f for f, r in file_ratios.items() if r.get(mat, 0) == 1.0]
        if not single_files:
            continue
        curve = load_vol_columns(single_files[0], target_len)
        avg_curve = curve.mean(axis=1)
        np.save(os.path.join(output_curve_dir, f"avg_curve_{mat}.npy"), avg_curve)

    if args.plot:
        plt.figure(figsize=(10, 5))
        colors = ["blue", "green", "red", "orange", "purple"]
        for i, mat in enumerate(all_materials):
            single_files = [f for f, r in file_ratios.items() if r.get(mat, 0) == 1.0]
            if not single_files:
                continue
            curve = load_vol_columns(single_files[0], target_len)
            avg_curve = curve.mean(axis=1)
            plt.plot(avg_curve, label=mat, color=colors[i % len(colors)])
        plt.legend()
        plt.title("平均曲线")
        plt.savefig(os.path.join(output_curve_dir, "avg_curves.png"))
        plt.close()
    print(f"[INFO] 平均曲线已保存到 {output_curve_dir}")

    # -----------------------------
    # 服务相关
    # -----------------------------
    dst_dir = "D:/work/server/composition_separation"
    os.makedirs(dst_dir, exist_ok=True)  # 确保目标目录存在

    # 遍历源目录下所有文件
    copy_dir_overwrite(dir_model, dst_dir)

    try:
        subprocess.run("pm2 restart composition_separation", shell=True, check=True, encoding="utf-8", errors="replace")
        print("[INFO] pm2 服务已重启 composition_separation")
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] pm2 重启失败: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="随机森林回归训练程序")
    parser.add_argument("--data_dir", type=str, required=True,
                        help="包含 CSV 数据文件的目录")
    parser.add_argument("--target_len", type=int, default=468,
                        help="裁切曲线长度")
    parser.add_argument("--output_model", type=str, required=True,
                        help="保存模型路径 (pkl)")
    parser.add_argument("--output_curve_dir", type=str, required=True,
                        help="保存曲线路径")
    parser.add_argument("--plot", action="store_true", help="是否绘制图像")

    args = parser.parse_args()
    main(args)
# python .\train_regression.py --data_dir .\uploads\0825_1\ --output_model .\regression_0825_1.pkl\regression_0825_1.pkl --output_curve_dir .\regression_0825_1.pkl --plot