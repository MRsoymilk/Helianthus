#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import argparse
import numpy as np
import pandas as pd
from sklearn.decomposition import PCA
from sklearn.neighbors import KNeighborsClassifier
import joblib
import plotly.express as px
import shutil
import subprocess

# -----------------------------
# 参数解析
# -----------------------------
parser = argparse.ArgumentParser(description="1D信号KNN训练脚本")
parser.add_argument("--data_dir", type=str, required=True, help="CSV数据所在目录")
parser.add_argument("--output", type=str, default="knn_model.pkl", help="输出KNN模型文件名")
parser.add_argument("--target_length", type=int, default=468, help="每个信号截取长度")
parser.add_argument("--plot", action="store_true", help="是否绘制PCA三维图")
args = parser.parse_args()


# -----------------------------
# 数据预处理函数
# -----------------------------
def preprocess(df, target_length=args.target_length):
    df_cut = df.iloc[:target_length]
    no_nan_mask = df_cut.notnull().all(axis=0)
    df_filtered = df_cut.loc[:, no_nan_mask]
    return df_filtered.reset_index(drop=True)


# -----------------------------
# 特征提取函数
# -----------------------------
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


def extract_feature_matrix(df, pattern_str=r"curve_\d+"):
    pattern = re.compile(pattern_str)
    feature_matrix = []
    for col in df.columns:
        if pattern.fullmatch(col):
            signal = df[col].values
            features = extract_features(signal)
            feature_matrix.append(features)
    return np.array(feature_matrix)



output_dir = os.path.join("output", args.output)
os.makedirs(output_dir, exist_ok=True) 
# -----------------------------
# 遍历数据目录
# -----------------------------
feature_matrices = []
labels = []

for file_name in os.listdir(args.data_dir):
    if not file_name.endswith(".csv"):
        print("skip invalid file: " + file_name)
        continue

    file_path = os.path.join(args.data_dir, file_name)
    df = preprocess(pd.read_csv(file_path))
    feature_matrix = extract_feature_matrix(df)

    if feature_matrix.size == 0:
        print(f"[WARN] 文件 {file_name} 没有有效列，跳过")
        continue

    feature_matrices.append(feature_matrix)
    # 标签用文件名（去掉扩展名）表示
    label = os.path.splitext(file_name)[0]
    labels.extend([label] * len(feature_matrix))
    print("handle file: " + file_name)

# 合并特征矩阵
if not feature_matrices:
    raise ValueError("没有有效数据用于训练")

X = np.vstack(feature_matrices)

# -----------------------------
# PCA 可视化
# -----------------------------
if args.plot:
    print("do PCA")
    pca = PCA(n_components=3)
    X_pca = pca.fit_transform(X)
    df_pca = pd.DataFrame({
        "PC1": X_pca[:, 0],
        "PC2": X_pca[:, 1],
        "PC3": X_pca[:, 2],
        "label": labels
    })
    fig = px.scatter_3d(
        df_pca,
        x="PC1", y="PC2", z="PC3",
        color="label",
        title="PCA 三维降维结果",
        labels={"PC1": "主成分1", "PC2": "主成分2", "PC3": "主成分3"},
        width=800, height=800
    )
    fig.write_html(output_dir + "/pca.html")

# -----------------------------
# 训练 KNN
# -----------------------------
print("start train KNN model")
knn = KNeighborsClassifier(n_neighbors=len(labels))
knn.fit(X, labels)
print("finish train KNN model")

joblib.dump(knn, output_dir + "/" + args.output)
print(f"[INFO] KNN 模型训练完成并保存为 {args.output}")

# -----------------------------
# 服务相关
# -----------------------------
src_path = os.path.join(output_dir, args.output)
dst_dir = "D:\work\server\knn_predict"
os.makedirs(dst_dir, exist_ok=True)

base = os.path.splitext(os.path.basename(filename))[0]  # 去掉 .pkl
tag = base.split("_")[-1]  # 拿最后一段
print(f"文件类型: {tag}")

dst_path = os.path.join(dst_dir, "knn_model_" + tag + ".pkl")

# 移动文件
shutil.copy(src_path, dst_path)
print(f"[INFO] 模型已移动到 {dst_path}")

# 执行 pm2 重启命令
try:
    subprocess.run("pm2 restart knn_predict", shell=True, check=True, encoding="utf-8", errors="replace")
    print("[INFO] pm2 服务已重启 knn_predict")
except subprocess.CalledProcessError as e:
    print(f"[ERROR] pm2 重启失败: {e}")

#  python .\train_knn.py --data_dir .\uploads\0822 --output knn_model_0822_r.pkl --plot