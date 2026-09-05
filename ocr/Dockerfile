FROM python:3.11-slim
WORKDIR /app

RUN apt-get update && apt-get install -y \
    libgl1 \
    libglib2.0-0
RUN python3 -m pip install onnxruntime-gpu
RUN python3 -m pip install paddleocr

COPY ocr/ ./

CMD ["python", "ocr.py"]