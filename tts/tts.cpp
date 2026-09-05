#include <cstdint>
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include "sherpa-onnx/c-api/cxx-api.h"

static int32_t ProgressCallback(const float *samples, int32_t num_samples,
                                float progress, void *arg) {
  fprintf(stderr, "Progress: %.3f%%\n", progress * 100);
  // return 1 to continue generating
  // return 0 to stop generating
  return 1;
}

int32_t main(int32_t argc, char *argv[]) {
  using namespace sherpa_onnx::cxx; // NOLINT
  OfflineTtsConfig config;
  config.model.vits.model = "model/vits-piper-zh_CN-chaowen-medium/zh_CN-chaowen-medium.onnx";
  config.model.vits.lexicon = "model/vits-piper-zh_CN-chaowen-medium/lexicon.txt";
  config.model.vits.tokens = "model/vits-piper-zh_CN-chaowen-medium/tokens.txt";
  config.model.num_threads = 1;

  // If you want to see debug messages, please set it to 1
  config.model.debug = 0;
  config.rule_fsts = "model/vits-piper-zh_CN-chaowen-medium/phone.fst,model/vits-piper-zh_CN-chaowen-medium/date.fst,model/vits-piper-zh_CN-chaowen-medium/number.fst";

  std::string filename = "./output/output.wav";

  std::ifstream input_file("./input/input.txt");
  // Check if the file was opened successfully
  if (!input_file.is_open()) {
      std::cerr << "Error opening the file!";
      return 1;
  }

  std::string text;

  // Read 1st line from the file
  std::getline(input_file, text);

  // Close the file
  input_file.close();

  auto tts = OfflineTts::Create(config);

  GenerationConfig gen_cfg;
  gen_cfg.sid = 0;
  gen_cfg.speed = 1.0; // larger -> faster in speech speed

#if 0
  // If you don't want to use a callback, then please enable this branch
  GeneratedAudio audio = tts.Generate(text, gen_cfg);
#else
  GeneratedAudio audio = tts.Generate(text, gen_cfg, ProgressCallback);
#endif

  WriteWave(filename, {audio.samples, audio.sample_rate});

  fprintf(stderr, "Input text is: %s\n", text.c_str());
  fprintf(stderr, "Speaker ID is: %d\n", gen_cfg.sid);
  fprintf(stderr, "Saved to: %s\n", filename.c_str());

  return 0;
}