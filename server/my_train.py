from flask import Flask, request, jsonify
import os
import shutil
app = Flask(__name__)

UPLOAD_FOLDER = "./uploads"
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

import threading
import subprocess

# 存储训练状态
train_status = {}  # key: folder_name, value: dict(status, stdout, stderr)

def run_training(folder, cmd):
    train_status[folder] = {"status": "running", "stdout": "", "stderr": ""}
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1, encoding="gbk", errors="replace")

    def read_stream(stream, key):
        for line in iter(stream.readline, ''):
            train_status[folder][key] += line
        stream.close()

    t_out = threading.Thread(target=read_stream, args=(proc.stdout, "stdout"))
    t_err = threading.Thread(target=read_stream, args=(proc.stderr, "stderr"))
    t_out.start()
    t_err.start()

    proc.wait()
    t_out.join()
    t_err.join()

    train_status[folder]["status"] = "finished"


def get_unique_folder(folder_name):
    """检查文件夹是否存在，存在则追加 _1, _2 ..."""
    folder_path = os.path.join(UPLOAD_FOLDER, folder_name)
    if not os.path.exists(folder_path):
        return folder_path

    count = 1
    while True:
        new_name = f"{folder_name}_{count}"
        new_path = os.path.join(UPLOAD_FOLDER, new_name)
        if not os.path.exists(new_path):
            return new_path
        count += 1

@app.route('/train_status/<folder>', methods=['GET'])
def get_train_status(folder):
    status = train_status.get(folder)
    print(f"status: {status}")
    if not status:
        print("No such training task")
        return jsonify({"success": False, "msg": "No such training task"}), 404
    result = jsonify(status)
    train_status[folder]["stdout"] = ""
    train_status[folder]["stderr"] = ""
    return result

@app.route('/upload', methods=['POST'])
def upload_files():
    if not request.files:
        print("No files uploaded")
        return jsonify({"success": False, "msg": "No files uploaded"}), 400

    method = request.form.get("method")
    print("method =", method)
    folder = request.form.get("folder")
    print("folder = ", folder)
    if not folder:
        folder = "default"
    save_folder = get_unique_folder(folder)
    folder = os.path.basename(save_folder)
    os.makedirs(save_folder, exist_ok=True)

    uploaded_files = request.files.getlist("file")
    for file in uploaded_files:
        file_path = os.path.join(save_folder, file.filename)
        file.save(file_path)
    saved_files = ""
    if method == "knn_reflection":
        saved_files = f'knn_model_{folder}_r.pkl'
        cmd = ["python", "train_knn.py", "--data_dir", save_folder, "--output", saved_files, "--plot"]
    elif method == "knn_transmission":
        saved_files = f'knn_model_{folder}_t.pkl'
        cmd = ["python", "train_knn.py", "--data_dir", save_folder, "--output", saved_files, "--plot"]
    elif method == "regression":
        saved_files = f'regression_model_{folder}.pkl'
        cmd = ["python", "train_regression.py", "--data_dir", save_folder, "--output_model", saved_files, "--output_curve", saved_files,"--plot"]
    else:
        return jsonify({"success": False, "msg": f"Unknown method: {method}"}), 400

    thread = threading.Thread(target=run_training, args=(folder, cmd))
    thread.start()

    return jsonify({
        "success": True,
        "saved_folder": folder,
        "files": saved_files,
        "method": method,
        "message": "Traning started"
    })


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5050, debug=True)
