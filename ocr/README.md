# OCR Service

Extracts Chinese (and mixed) text from images locally using [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) with the default [PP-OCRv6](https://www.paddleocr.ai/main/en/version3.x/algorithm/PP-OCRv6/PP-OCRv6.html) models. The sample program in `ocr.py` runs inference through the [ONNX Runtime](https://onnxruntime.ai/) engine, prints recognized text, and writes visualization plus JSON under `output/`.

---

## Overview

This service is the optical character recognition stage of Bǎokěmèng Learner: an image (for example a screenshot or captured frame) is turned into Chinese text that later stages can speak or study. Models download on first use into `~/.paddlex/official_models/`; inference is intended to stay on-device.

The current prototype uses **PP-OCRv6 medium** detection and recognition with `lang="ch"`, document preprocessing helpers turned off, and `engine="onnxruntime"` so a full `paddlepaddle` install is not required. See the [OCR pipeline usage docs](https://www.paddleocr.ai/main/en/version3.x/pipeline_usage/OCR.html) for the official API shape this script follows.

---

## How it Works

1. `ocr.py` constructs a [PaddleOCR](https://www.paddleocr.ai/main/en/version3.x/pipeline_usage/OCR.html) pipeline with orientation / unwarping / textline orientation disabled, `device="gpu:0"`, `engine="onnxruntime"`, and `lang="ch"`.
2. `predict` runs detection and recognition on `./input/general_ocr_002.png` (paths are relative to the `ocr/` working directory).
3. For each result, `print()` dumps recognized lines; `save_to_img` / `save_to_json` write annotated images and structured results into `output/`.

On first run, PP-OCRv6 ONNX model archives are fetched automatically. If CUDA / cuDNN are unavailable, ONNX Runtime may fall back to CPU while still completing inference.

---

## Setup

Run all Docker commands from the project root (`baokemeng-learner/`).

Python 3.10+ is used by the OCR container. You do **not** need to create a local Python virtual environment or install the OCR dependencies on your host.

### 1. Build the OCR container

```bash
docker compose build ocr
```

This builds the OCR image using `ocr/Dockerfile`. The image installs the required Python dependencies, including `onnxruntime-gpu` and `paddleocr`, along with the Linux libraries required by OpenCV.

### 2. Add a sample input image

Create the input and output directories if they do not already exist:

```bash
mkdir -p ocr/input ocr/output
```

Download the sample image:

```bash
wget -O ocr/input/general_ocr_002.png \
  https://paddle-model-ecology.bj.bcebos.com/paddlex/imgs/demo_image/general_ocr_002.png
```

The Docker Compose configuration mounts these directories into the container:

```text
Host             Container
ocr/input/  →    /app/input/
ocr/output/ →    /app/output/
```

Therefore, files written to `/app/output/` by `ocr.py` are persisted in `ocr/output/` on the host.

### 3. Run OCR

```bash
docker compose up ocr
```

This starts the OCR container using the image built in the previous step.

`ocr.py` expects the input image at:

```text
/app/input/general_ocr_002.png
```

It loads (or downloads) the PP-OCRv6 ONNX models, runs OCR on the sample boarding-pass image, prints the results, and writes files to the mounted `ocr/output` directory.

### 4. Rebuild after changing the Dockerfile

If you modify `ocr/Dockerfile`, rebuild the image:

```bash
docker compose build ocr
```

Then start the service again:

```bash
docker compose up ocr
```

If you need to ensure Docker completely rebuilds the image without using cached layers:

```bash
docker compose build --no-cache ocr
```

### 5. Stopping the container

Press `Ctrl+C` while `docker compose up` is running to stop the service.

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
