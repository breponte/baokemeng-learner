FROM ubuntu:latest AS build
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    git \
    libsndfile1-dev \
    libsndfile-dev \
    libsndfile1 \
    libsndfile-dev

WORKDIR /app

COPY tts/ ./

RUN g++ -std=c++17 \
    -I ./sherpa-onnx/shared/include \
    -L ./sherpa-onnx/shared/lib \
    -o ./tts ./tts.cpp \
    -lsherpa-onnx-cxx-api \
    -lsherpa-onnx-c-api \
    -lonnxruntime \
    -Wl,-rpath,'$ORIGIN/sherpa-onnx/shared/lib'

CMD ["./tts"]