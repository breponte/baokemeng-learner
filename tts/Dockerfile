# ---------------------------------------------------------------------------
# Stage 1 — build the C++ TTS binary
# ---------------------------------------------------------------------------
FROM registry.access.redhat.com/ubi9/ubi-minimal:latest AS builder

# sherpa-onnx release to pin against
ARG SHERPA_VERSION=1.10.41

# Install build toolchain
RUN microdnf install -y \
        gcc-c++ \
        cmake \
        wget \
        tar \
        bzip2 \
        ca-certificates \
    && microdnf clean all

WORKDIR /build

# Download the pre-built sherpa-onnx shared-library release for Linux x86-64.
# The archive contains headers and .so files so we can compile against them
# without building the whole project from source.
RUN wget --progress=dot:giga --server-response \
    "https://github.com/k2-fsa/sherpa-onnx/releases/download/v${SHERPA_VERSION}/sherpa-onnx-v${SHERPA_VERSION}-linux-x64-shared.tar.bz2" \
    -O sherpa.tar.bz2 \
  && tar -xjf sherpa.tar.bz2 \
  && mv sherpa-onnx-v${SHERPA_VERSION}-linux-x64-shared sherpa-onnx-sdk \
  && rm sherpa.tar.bz2

# Copy application source
COPY tts/tts.cpp .

# Compile
RUN g++ -std=c++17 -O2 \
        -I sherpa-onnx-sdk/include \
        -L sherpa-onnx-sdk/lib \
        -o tts tts.cpp \
        -lsherpa-onnx-c-api \
        -Wl,-rpath,/usr/local/lib/sherpa-onnx

# ---------------------------------------------------------------------------
# Stage 2 — minimal runtime image
# ---------------------------------------------------------------------------
FROM registry.access.redhat.com/ubi9/ubi-minimal:latest

RUN microdnf install -y \
        libstdc++ \
    && microdnf clean all

# Copy compiled binary
COPY --from=builder /build/tts /usr/local/bin/tts

# Copy only the shared libraries needed at runtime
COPY --from=builder /build/sherpa-onnx-sdk/lib /usr/local/lib/sherpa-onnx

# Make shared libs discoverable
RUN echo "/usr/local/lib/sherpa-onnx" > /etc/ld.so.conf.d/sherpa-onnx.conf \
  && ldconfig

# Non-root user (UID 1001)
RUN useradd -m -u 1001 ttsuser
USER 1001

# Models are mounted at runtime — see docker-compose.yaml
ENV SHERPA_VITS_MODEL=/models/vits-piper-zh_CN-huayan-medium/zh_CN-huayan-medium.onnx \
    SHERPA_TOKENS=/models/vits-piper-zh_CN-huayan-medium/tokens.txt \
    SHERPA_LEXICON= \
    SHERPA_DATA_DIR=/models/vits-piper-zh_CN-huayan-medium/espeak-ng-data \
    SHERPA_SPEED=1.0 \
    SHERPA_SPEAKER_ID=0

WORKDIR /home/ttsuser

# Default: wait for one line of STDIN and produce /output/output.wav
CMD ["tts", "/output/output.wav"]
