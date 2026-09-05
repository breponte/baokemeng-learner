# TTS Service

Converts Chinese text to speech locally using [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) and a [Piper VITS Chinese model](https://k2-fsa.github.io/sherpa/onnx/tts/all/Chinese/vits-piper-zh_CN-chaowen-medium.html). The sample program in `tts.cpp` loads the model via the [sherpa-onnx C++ API](https://k2-fsa.github.io/sherpa/onnx/tts/index.html), synthesizes audio, and writes a WAV file (`test.wav`).

---

## Overview

This service is the text-to-speech stage of Bǎokěmèng Learner: Chinese text (for example from OCR) is spoken with an offline neural TTS engine. Inference runs entirely on-device through [sherpa-onnx](https://k2-fsa.github.io/sherpa/onnx/) — no cloud TTS API is required.

The current demo uses the **vits-piper-zh_CN-chaowen-medium** model, which includes a lexicon and rule FSTs for phone numbers, dates, and numerals. See the [model sample page](https://k2-fsa.github.io/sherpa/onnx/tts/all/Chinese/vits-piper-zh_CN-chaowen-medium.html) for the official config and download link.

---

## How it Works

1. `tts.cpp` builds an [`OfflineTtsConfig`](https://k2-fsa.github.io/sherpa/onnx/tts/index.html) pointing at the ONNX model, lexicon, tokens, and rule FSTs under `model/`.
2. [`OfflineTts::Create`](https://github.com/k2-fsa/sherpa-onnx) loads the shared sherpa-onnx libraries and validates that every configured path exists.
3. [`Generate`](https://k2-fsa.github.io/sherpa/onnx/tts/index.html) turns the input Chinese string into PCM samples (with an optional progress callback).
4. [`WriteWave`](https://k2-fsa.github.io/sherpa/onnx/tts/index.html) saves the samples to `./test.wav` at the model’s native sample rate.

Paths in `tts.cpp` are relative to the `tts/` working directory. If create fails (for example a missing `.fst`), generation returns empty audio and the WAV may report an invalid sample rate.

---

## Setup

Run all commands from the `tts/` directory.

### 1. Install and extract the model

Download the official release archive and unpack it into `model/` so the layout matches the paths in `tts.cpp`:

```bash
mkdir -p model
cd model
wget https://github.com/k2-fsa/sherpa-onnx/releases/download/tts-models/vits-piper-zh_CN-chaowen-medium.tar.bz2
tar xf vits-piper-zh_CN-chaowen-medium.tar.bz2
cd ..
```

`wget` fetches the model tarball from the [sherpa-onnx TTS model releases](https://github.com/k2-fsa/sherpa-onnx/releases/tag/tts-models). `tar xf` extracts `vits-piper-zh_CN-chaowen-medium/` (ONNX weights, `lexicon.txt`, `tokens.txt`, and `phone.fst` / `date.fst` / `number.fst`) under `model/`.

After extraction you should have:

```text
model/vits-piper-zh_CN-chaowen-medium/
  zh_CN-chaowen-medium.onnx
  lexicon.txt
  tokens.txt
  phone.fst
  date.fst
  number.fst
  ...
```

### 2. Compile

Link against the installed sherpa-onnx shared libraries:

```bash
g++ -std=c++17 \
  -I ./sherpa-onnx/shared/include \
  -L ./sherpa-onnx/shared/lib \
  -o ./tts ./tts.cpp \
  -lsherpa-onnx-cxx-api \
  -lsherpa-onnx-c-api \
  -lonnxruntime \
  -Wl,-rpath,'$ORIGIN/sherpa-onnx/shared/lib'
```

`-I` / `-L` locate the [C++ API headers](https://github.com/k2-fsa/sherpa-onnx/blob/master/sherpa-onnx/c-api/cxx-api.h) and shared libraries. `-l…` links sherpa-onnx and ONNX Runtime. `-Wl,-rpath,'$ORIGIN/…'` embeds a runtime search path so `./tts` finds the `.so` files next to the binary without setting `LD_LIBRARY_PATH`.

### 3. Run

```bash
./tts
```

This loads the model, synthesizes the sample Chinese text in `tts.cpp`, and writes `./test.wav`. You can inspect the result with `ffprobe test.wav` or any audio player.

---

## Resources

### sherpa-onnx

[sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) is an open-source speech toolkit (ASR, TTS, VAD, and more) that runs ONNX models locally with [ONNX Runtime](https://onnxruntime.ai/). For this project it provides the offline TTS C/C++ API used by `tts.cpp`.

| Resource | Description |
|---|---|
| [sherpa-onnx GitHub](https://github.com/k2-fsa/sherpa-onnx) | Source code, issues, and releases |
| [sherpa-onnx documentation](https://k2-fsa.github.io/sherpa/onnx/) | Official docs for install, APIs, and examples |
| [TTS documentation](https://k2-fsa.github.io/sherpa/onnx/tts/index.html) | Offline TTS overview and usage |
| [vits-piper-zh_CN-chaowen-medium](https://k2-fsa.github.io/sherpa/onnx/tts/all/Chinese/vits-piper-zh_CN-chaowen-medium.html) | Model page with sample config and download |
| [TTS model releases](https://github.com/k2-fsa/sherpa-onnx/releases/tag/tts-models) | Prebuilt model archives hosted by the project |
