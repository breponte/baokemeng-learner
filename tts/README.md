# Text-to-Speech (TTS)

Chinese text-to-speech powered by [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) with a VITS model.
The service reads one line of Chinese text from STDIN and writes a 16-bit mono WAV file.

---

## How it works

1. A line of Chinese text is written to the service's STDIN.
2. `tts` synthesises the audio using a VITS ONNX model.
3. The resulting WAV file is written to `/output/output.wav` inside the container, which is bind-mounted to `tts/output/` on the host.

---

## Setup

### Prerequisites

- [Docker](https://docs.docker.com/get-docker/) with the Compose plugin

### Build the image

Run from the repo root:

```
docker compose build tts
```

### Download the model

The service does **not** bundle a model — you must download one into `tts/models/` before the first run.

Run from the repo root:

```
docker compose run --rm --entrypoint="" tts sh -c "
  mkdir -p /models &&
  wget -O /models/vits-piper-zh_CN-huayan-medium.tar.bz2
    'https://github.com/k2-fsa/sherpa-onnx/releases/download/tts-models/vits-piper-zh_CN-huayan-medium.tar.bz2' &&
  tar -xjf /models/vits-piper-zh_CN-huayan-medium.tar.bz2 -C /models/ &&
  rm /models/vits-piper-zh_CN-huayan-medium.tar.bz2
"
```

This runs the download and extraction entirely inside the container, so no host-side tools (`wget`, `bzip2`, `tar`) are required.

After extraction `tts/models/` on the host should contain:

```
tts/models/
└── vits-piper-zh_CN-huayan-medium/
    ├── zh_CN-huayan-medium.onnx
    ├── zh_CN-huayan-medium.onnx.json
    ├── tokens.txt
    ├── MODEL_CARD
    └── espeak-ng-data/
```

---

## Usage

```
# Pipe Chinese text in; WAV is written to tts/output/output.wav on the host
echo "你好，世界！" | docker compose run --rm -T tts

# Pipe WAV bytes directly to stdout (e.g. to play immediately)
echo "今天天气很好。" | docker compose run --rm -T tts -
```

---

## Environment variables

| Variable            | Default                                   | Description                                                        |
|---------------------|-------------------------------------------|--------------------------------------------------------------------|
| `SHERPA_VITS_MODEL` | `.../zh_CN-huayan-medium.onnx`            | Path to the VITS `.onnx` model file (required)                     |
| `SHERPA_TOKENS`     | `.../tokens.txt`                          | Path to the tokens file (required)                                 |
| `SHERPA_LEXICON`    | _(empty)_                                 | Lexicon path — set for lexicon-based models, leave empty for Piper |
| `SHERPA_DATA_DIR`   | `.../espeak-ng-data`                      | espeak-ng data dir — set for Piper models, leave empty otherwise   |
| `SHERPA_SPEED`      | `1.0`                                     | Speaking speed (lower = slower)                                    |
| `SHERPA_SPEAKER_ID` | `0`                                       | Speaker ID for multi-speaker models                                |

Defaults are configured in `docker-compose.yaml` and match the `vits-piper-zh_CN-huayan-medium` model layout above.

---

## Future integration with the `translate` service

The `tts` service is on the same Docker network (`app-net`) as `translate`.
When the pipeline is connected, the `translate` service will pipe its Chinese
output directly to this service's STDIN — no network port changes needed.

Planned flow:

```
User STDIN → translate (EN→ZH) → tts STDIN → output.wav
```

---

## TODO

- Connect STDIN of `tts` to STDOUT of `translate` to form an end-to-end pipeline
