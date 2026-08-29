// tts.cpp
// Reads a line of Chinese text from STDIN, synthesises speech with
// sherpa-onnx, and writes a WAV file (or stdout when path is "-").
//
// Usage:
//   echo "你好世界" | tts [output.wav]
//
// If no output path is supplied the WAV bytes are written to stdout so that
// the caller can pipe them directly into an audio player:
//   echo "你好世界" | tts - | aplay -
//
// Environment variables (all required):
//   SHERPA_VITS_MODEL   – path to the VITS model file   (.onnx)
//   SHERPA_TOKENS       – path to the tokens file        (e.g. tokens.txt)
//
// Optional (set one depending on the model type):
//   SHERPA_LEXICON      – path to lexicon file for lexicon-based models (e.g. vits-zh-aishell3)
//   SHERPA_DATA_DIR     – path to espeak-ng-data/ dir for Piper models  (e.g. vits-piper-*)
//   SHERPA_SPEED        – speaking speed, float (default 1.0)
//   SHERPA_SPEAKER_ID   – speaker id, integer (default 0)

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "sherpa-onnx/c-api/c-api.h"

// ---------------------------------------------------------------------------
// Minimal WAV writer (PCM 16-bit mono)
// ---------------------------------------------------------------------------
static void write_wav(std::ostream &out,
                      const float *samples,
                      int32_t num_samples,
                      int32_t sample_rate) {
  // Convert float [-1, 1] → int16
  std::vector<int16_t> pcm(num_samples);
  for (int32_t i = 0; i < num_samples; ++i) {
    float s = samples[i];
    if (s > 1.0f)  s = 1.0f;
    if (s < -1.0f) s = -1.0f;
    pcm[i] = static_cast<int16_t>(s * 32767.0f);
  }

  uint32_t data_size   = static_cast<uint32_t>(num_samples * 2);
  uint32_t chunk_size  = 36 + data_size;
  uint16_t audio_fmt   = 1;       // PCM
  uint16_t num_channels = 1;      // mono
  uint32_t byte_rate   = static_cast<uint32_t>(sample_rate * 2);
  uint16_t block_align = 2;
  uint16_t bits_per_sample = 16;

  // RIFF header
  out.write("RIFF", 4);
  out.write(reinterpret_cast<const char *>(&chunk_size),     4);
  out.write("WAVE", 4);
  // fmt sub-chunk
  out.write("fmt ", 4);
  uint32_t fmt_size = 16;
  out.write(reinterpret_cast<const char *>(&fmt_size),       4);
  out.write(reinterpret_cast<const char *>(&audio_fmt),      2);
  out.write(reinterpret_cast<const char *>(&num_channels),   2);
  out.write(reinterpret_cast<const char *>(&sample_rate),    4);
  out.write(reinterpret_cast<const char *>(&byte_rate),      4);
  out.write(reinterpret_cast<const char *>(&block_align),    2);
  out.write(reinterpret_cast<const char *>(&bits_per_sample),2);
  // data sub-chunk
  out.write("data", 4);
  out.write(reinterpret_cast<const char *>(&data_size),      4);
  out.write(reinterpret_cast<const char *>(pcm.data()),
            static_cast<std::streamsize>(data_size));
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static const char *require_env(const char *name) {
  const char *val = std::getenv(name);
  if (!val || val[0] == '\0') {
    std::string msg = std::string("Required environment variable not set: ") + name;
    throw std::runtime_error(msg);
  }
  return val;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char *argv[]) {
  try {
    // --- Read Chinese text from STDIN ---
    std::cerr << "Enter Chinese text: ";
    std::string text;
    if (!std::getline(std::cin, text) || text.empty()) {
      throw std::runtime_error("No input text received on STDIN.");
    }

    // --- Build sherpa-onnx TTS config from environment ---
    SherpaOnnxOfflineTtsConfig config;
    std::memset(&config, 0, sizeof(config));

    SherpaOnnxOfflineTtsVitsModelConfig vits;
    std::memset(&vits, 0, sizeof(vits));
    vits.model  = require_env("SHERPA_VITS_MODEL");
    vits.tokens = require_env("SHERPA_TOKENS");

    const char *lexicon = std::getenv("SHERPA_LEXICON");
    vits.lexicon = (lexicon && lexicon[0] != '\0') ? lexicon : "";

    const char *data_dir = std::getenv("SHERPA_DATA_DIR");
    vits.data_dir = (data_dir && data_dir[0] != '\0') ? data_dir : "";

    const char *speed_str = std::getenv("SHERPA_SPEED");
    vits.length_scale = (speed_str && speed_str[0] != '\0')
                        ? std::stof(speed_str)
                        : 1.0f;

    config.model.vits = vits;
    config.model.num_threads = 2;
    config.model.debug = 0;
    config.model.provider = "cpu";

    // --- Create TTS handle ---
    const SherpaOnnxOfflineTts *tts = SherpaOnnxCreateOfflineTts(&config);
    if (!tts) {
      throw std::runtime_error("Failed to create sherpa-onnx TTS instance. "
                               "Check that model paths are correct.");
    }

    const char *speaker_str = std::getenv("SHERPA_SPEAKER_ID");
    int speaker_id = (speaker_str && speaker_str[0] != '\0')
                     ? std::stoi(speaker_str)
                     : 0;

    // --- Synthesise ---
    const SherpaOnnxGeneratedAudio *audio =
        SherpaOnnxOfflineTtsGenerate(tts, text.c_str(), speaker_id, 1.0f);
    if (!audio || audio->n == 0) {
      SherpaOnnxDestroyOfflineTts(tts);
      throw std::runtime_error("TTS synthesis produced no audio samples.");
    }

    // --- Write WAV ---
    std::string output_path = (argc > 1) ? argv[1] : "output.wav";
    if (output_path == "-") {
      write_wav(std::cout, audio->samples, audio->n, audio->sample_rate);
    } else {
      std::ofstream f(output_path, std::ios::binary);
      if (!f) {
        throw std::runtime_error("Cannot open output file: " + output_path);
      }
      write_wav(f, audio->samples, audio->n, audio->sample_rate);
      std::cerr << "WAV written to: " << output_path << "\n";
    }

    SherpaOnnxDestroyOfflineTtsGeneratedAudio(audio);
    SherpaOnnxDestroyOfflineTts(tts);
    return 0;

  } catch (const std::exception &ex) {
    std::cerr << "ERROR: " << ex.what() << "\n";
    return 1;
  }
}
