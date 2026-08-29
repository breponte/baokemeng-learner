# OCR Service

Watches a shared capture directory for image files, runs [PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) (PP-OCRv4 Chinese models) to extract Chinese text, and writes a JSON result file per image to the output directory.

---

## How it works

1. The service polls `/ocr/capture/` every **500 ms** (configurable).
2. Any `.png`, `.jpg`, `.jpeg`, or `.bmp` file found is processed immediately.
3. OCR detection + recognition runs against the image using the PP-OCRv4 Chinese models.
4. Result blocks whose `text` field contains **at least one Chinese character** (CJK Unified Ideograph) are kept; Latin-only blocks are discarded.
5. A JSON file is written to `/ocr/output/<stem>.json`.
6. The original image is **removed** from `/ocr/capture/` to prevent reprocessing.

---

## Output JSON format

```json
{
  "source": "screenshot_001.png",
  "blocks": [
    {
      "text": "你好",
      "confidence": 0.9876,
      "box": [[120, 45], [210, 45], [210, 72], [120, 72]]
    }
  ]
}
```

| Field | Type | Description |
|---|---|---|
| `source` | string | Original image filename |
| `blocks` | array | Each detected Chinese text region |
| `blocks[].text` | string | Recognised Chinese text |
| `blocks[].confidence` | float (0–1) | Model confidence score |
| `blocks[].box` | `[[x,y],…]×4` | Bounding quadrilateral (top-left → clockwise) |

---

## Running via Docker Compose

```bash
docker compose up ocr
```

Drop any image into `ocr/capture/`. The JSON result will appear in `ocr/output/` within one poll interval.

---

## Environment variables

| Variable | Default | Description |
|---|---|---|
| `OCR_CAPTURE_DIR` | `/ocr/capture` | Directory polled for incoming images |
| `OCR_OUTPUT_DIR` | `/ocr/output` | Directory where JSON results are written |
| `OCR_DET_MODEL_DIR` | `/models/ch_PP-OCRv4_det_infer` | PaddleOCR detection model |
| `OCR_REC_MODEL_DIR` | `/models/ch_PP-OCRv4_rec_infer` | PaddleOCR recognition model |
| `OCR_CLS_MODEL_DIR` | *(empty — disabled)* | Optional angle classifier model |
| `OCR_REC_CHAR_DICT` | `/models/ppocr_keys_v1.txt` | Character dictionary for the rec model |
| `OCR_POLL_INTERVAL_MS` | `500` | Polling interval in milliseconds |
| `OCR_USE_GPU` | `0` | Set to `1` to enable GPU inference |

---

## Build

The Dockerfile is a two-stage build:

| Stage | Base image | What happens |
|---|---|---|
| `builder` | `ubi9/ubi-minimal` | Downloads PaddlePaddle inference lib, builds OpenCV (minimal static), clones & builds PaddleOCR C++ deploy utilities, downloads PP-OCRv4 models, compiles `ocr.cpp` |
| runtime | `ubi9/ubi-minimal` | Copies binary + shared libs + models; runs as non-root UID 1001 |

The PP-OCRv4 models (`ch_PP-OCRv4_det_infer`, `ch_PP-OCRv4_rec_infer`, `ppocr_keys_v1.txt`) are **baked into the image** during the build stage — no manual model download step is required.

To rebuild after source changes:

```bash
docker compose build ocr
```
