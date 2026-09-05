# OCR Service

Extracts Chinese (and mixed) text from images locally using [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) with the default [PP-OCRv6](https://www.paddleocr.ai/main/en/version3.x/algorithm/PP-OCRv6/PP-OCRv6.html) models. The sample program in `ocr.py` runs inference through the [ONNX Runtime](https://onnxruntime.ai/) engine, prints recognized text, and writes visualization plus JSON under `output/`.

---

## Overview

This service is the optical character recognition stage of Bǎokěmèng Learner: an image (for example a screenshot or captured frame) is turned into Chinese text that later stages can speak or study. Models download on first use into `~/.paddlex/official_models/`; inference is intended to stay on-device.

The current prototype uses **PP-OCRv6 medium** detection and recognition with `lang="ch"`, document preprocessing helpers turned off, and `engine="onnxruntime"` so a full `paddlepaddle` install is not required. See the [OCR pipeline usage docs](https://www.paddleocr.ai/main/en/version3.x/pipeline_usage/OCR.html) for the official API shape this script follows.

---

## How it Works

1. `ocr.py` constructs a [`PaddleOCR`](https://www.paddleocr.ai/main/en/version3.x/pipeline_usage/OCR.html) pipeline with orientation / unwarping / textline orientation disabled, `device="gpu:0"`, `engine="onnxruntime"`, and `lang="ch"`.
2. [`predict`](https://www.paddleocr.ai/main/en/version3.x/pipeline_usage/OCR.html) runs detection and recognition on `./input/general_ocr_002.png` (paths are relative to the `ocr/` working directory).
3. For each result, `print()` dumps recognized lines; `save_to_img` / `save_to_json` write annotated images and structured results into `output/`.

On first run, PP-OCRv6 ONNX model archives are fetched automatically. If CUDA / cuDNN are unavailable, ONNX Runtime may fall back to CPU while still completing inference.

---

## Setup

Run all commands from the `ocr/` directory. Python 3.10+ is assumed (the prototype was developed on 3.10).

### 1. Create and activate a virtualenv

```bash
python3 -m venv venv
source venv/bin/activate
```

`python3 -m venv venv` creates an isolated environment under `ocr/venv/` (gitignored). `source venv/bin/activate` puts that environment’s `python` / `pip` on your `PATH` for the rest of the session.

### 2. Install dependencies

```bash
python3 -m pip install onnxruntime-gpu
pip install paddleocr
```

`onnxruntime-gpu` provides the ONNX Runtime backend used by `engine="onnxruntime"` (CPU-only `onnxruntime` also works if you do not need a GPU build). `paddleocr` pulls in PaddleX OCR core packages used by `ocr.py`. This prototype does **not** install `paddlepaddle`; keep `engine="onnxruntime"` so the default Paddle static engine is not required.

### 3. Add a sample input image

```bash
mkdir -p input output
wget -O input/general_ocr_002.png \
  https://paddle-model-ecology.bj.bcebos.com/paddlex/imgs/demo_image/general_ocr_002.png
```

`ocr.py` currently expects `./input/general_ocr_002.png`. Create `output/` as well so save helpers have a destination directory.

### 4. Run

```bash
python3 ocr.py
```

This loads (or downloads) the PP-OCRv6 ONNX models, runs OCR on the sample boarding-pass image, prints results, and writes files such as `output/general_ocr_002_ocr_res_img.png` and `output/general_ocr_002_res.json`.

---

## Resources

### PaddleOCR

[PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) is an open-source OCR toolkit (detection, recognition, and document pipelines) with a Python API. This prototype uses the v3.x OCR pipeline over ONNX Runtime rather than the Paddle inference engine.

| Resource | Description |
|---|---|
| [PaddleOCR GitHub](https://github.com/PaddlePaddle/PaddleOCR) | Source code, issues, and releases |
| [PaddleOCR documentation](https://www.paddleocr.ai/) | Official docs for install, pipelines, and examples |
| [OCR pipeline usage](https://www.paddleocr.ai/main/en/version3.x/pipeline_usage/OCR.html) | `PaddleOCR` / `predict` API used by `ocr.py` |
| [PP-OCRv6 introduction](https://www.paddleocr.ai/main/en/version3.x/algorithm/PP-OCRv6/PP-OCRv6.html) | Default model family and quick-start samples |
| [ONNX Runtime](https://onnxruntime.ai/) | Inference engine selected via `engine="onnxruntime"` |
