# ---------------------------------------------------------------------------
# Stage 1 — build
# Installs the PaddlePaddle C++ inference library, builds PaddleOCR's C++
# utilities, then compiles ocr.cpp against them.
# ---------------------------------------------------------------------------
FROM registry.access.redhat.com/ubi9/ubi-minimal:latest AS builder

# Versions to pin
ARG PADDLE_VERSION=2.6.2
ARG PADDLEOCR_VERSION=2.9.1
ARG OPENCV_VERSION=4.10.0

RUN microdnf install -y \
        gcc-c++ \
        cmake \
        make \
        wget \
        tar \
        bzip2 \
        gzip \
        ca-certificates \
        git \
        libgomp \
    && microdnf clean all

WORKDIR /build

# ---------------------------------------------------------------------------
# 1a. PaddlePaddle CPU-only inference library (pre-built for Linux x86-64)
# ---------------------------------------------------------------------------
RUN wget --progress=dot:giga \
      "https://paddle-inference-lib.bj.bcebos.com/${PADDLE_VERSION}/cxx_c/Linux/CPU/x86-64_gcc8.2_avx_mkl/paddle_inference.tgz" \
      -O paddle_inference.tgz \
    && tar -xzf paddle_inference.tgz \
    && mv paddle_inference paddle-sdk \
    && rm paddle_inference.tgz

# ---------------------------------------------------------------------------
# 1b. OpenCV — minimal build (imgcodecs + imgproc only, static libs)
# ---------------------------------------------------------------------------
RUN microdnf install -y \
        libjpeg-turbo-devel \
        libpng-devel \
        libtiff-devel \
        pkgconf-pkg-config \
    && microdnf clean all

RUN wget --progress=dot:giga \
      "https://github.com/opencv/opencv/archive/refs/tags/${OPENCV_VERSION}.tar.gz" \
      -O opencv.tar.gz \
    && tar -xzf opencv.tar.gz \
    && cmake -S opencv-${OPENCV_VERSION} -B opencv-build \
         -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/build/opencv-sdk \
         -DBUILD_LIST=core,imgcodecs,imgproc \
         -DBUILD_SHARED_LIBS=OFF \
         -DBUILD_TESTS=OFF \
         -DBUILD_PERF_TESTS=OFF \
         -DWITH_GTK=OFF \
         -DWITH_QT=OFF \
         -DWITH_FFMPEG=OFF \
    && cmake --build opencv-build --parallel "$(nproc)" \
    && cmake --install opencv-build \
    && rm -rf opencv.tar.gz opencv-${OPENCV_VERSION} opencv-build

# ---------------------------------------------------------------------------
# 1c. PaddleOCR C++ deploy utilities (det/rec/cls wrappers)
# ---------------------------------------------------------------------------
RUN git clone --depth=1 \
      --branch "release/${PADDLEOCR_VERSION}" \
      https://github.com/PaddlePaddle/PaddleOCR.git \
      paddleocr-src

RUN cmake -S paddleocr-src/deploy/cpp_infer -B paddleocr-build \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=/build/paddleocr-sdk \
          -DPADDLE_LIB=/build/paddle-sdk \
          -DOpenCV_DIR=/build/opencv-sdk/lib/cmake/opencv4 \
          -DWITH_GPU=OFF \
          -DWITH_MKL=ON \
    && cmake --build paddleocr-build --parallel "$(nproc)" \
    && cmake --install paddleocr-build

# ---------------------------------------------------------------------------
# 1d. Download PP-OCRv4 Chinese models + character dictionary
# ---------------------------------------------------------------------------
RUN mkdir -p /models \
    && wget --progress=dot:giga \
         "https://paddleocr.bj.bcebos.com/PP-OCRv4/chinese/ch_PP-OCRv4_det_infer.tar" \
         -O det.tar && tar -xf det.tar -C /models && rm det.tar \
    && wget --progress=dot:giga \
         "https://paddleocr.bj.bcebos.com/PP-OCRv4/chinese/ch_PP-OCRv4_rec_infer.tar" \
         -O rec.tar && tar -xf rec.tar -C /models && rm rec.tar \
    && wget --progress=dot:giga \
         "https://raw.githubusercontent.com/PaddlePaddle/PaddleOCR/release/${PADDLEOCR_VERSION}/ppocr/utils/ppocr_keys_v1.txt" \
         -O /models/ppocr_keys_v1.txt

# ---------------------------------------------------------------------------
# 1e. Compile ocr.cpp
# ---------------------------------------------------------------------------
COPY ocr/ocr.cpp .

RUN g++ -std=c++17 -O2 \
        -I paddleocr-src/deploy/cpp_infer/include \
        -I paddle-sdk/paddle/include \
        -I opencv-sdk/include/opencv4 \
        -L paddleocr-sdk/lib \
        -L paddle-sdk/paddle/lib \
        -L opencv-sdk/lib \
        -o ocr ocr.cpp \
        -lpaddleocr \
        -lpaddle_inference \
        $(pkg-config --libs opencv4) \
        -lgomp \
        -lpthread \
        -Wl,-rpath,/usr/local/lib/ocr

# ---------------------------------------------------------------------------
# Stage 2 — minimal runtime image
# ---------------------------------------------------------------------------
FROM registry.access.redhat.com/ubi9/ubi-minimal:latest

RUN microdnf install -y \
        libstdc++ \
        libgomp \
        libjpeg-turbo \
        libpng \
    && microdnf clean all

# Compiled binary
COPY --from=builder /build/ocr /usr/local/bin/ocr

# Shared libraries from PaddlePaddle and PaddleOCR
COPY --from=builder /build/paddle-sdk/paddle/lib  /usr/local/lib/ocr
COPY --from=builder /build/paddleocr-sdk/lib      /usr/local/lib/ocr

RUN echo "/usr/local/lib/ocr" > /etc/ld.so.conf.d/ocr.conf && ldconfig

# PP-OCRv4 models baked in at build time — no manual download required
COPY --from=builder /models /models

# Non-root user
RUN useradd -m -u 1001 ocruser
USER 1001

# Bind-mounted at runtime by docker-compose
VOLUME ["/ocr/capture", "/ocr/output"]

ENV OCR_CAPTURE_DIR=/ocr/capture \
    OCR_OUTPUT_DIR=/ocr/output \
    OCR_DET_MODEL_DIR=/models/ch_PP-OCRv4_det_infer \
    OCR_REC_MODEL_DIR=/models/ch_PP-OCRv4_rec_infer \
    OCR_CLS_MODEL_DIR= \
    OCR_REC_CHAR_DICT=/models/ppocr_keys_v1.txt \
    OCR_POLL_INTERVAL_MS=500 \
    OCR_USE_GPU=0

WORKDIR /home/ocruser

CMD ["ocr"]
