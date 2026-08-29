// ocr.cpp
// Watches /ocr/capture/ for image files, runs PaddleOCR to extract Chinese
// text, and writes a JSON result file to /ocr/output/.
//
// For each image <name>.{png,jpg,jpeg,bmp} found in the capture directory:
//   1. Run PaddleOCR detection + recognition (Chinese model).
//   2. Filter result blocks to those whose text contains at least one
//      Chinese character (Unicode range U+4E00–U+9FFF, U+3400–U+4DBF,
//      U+20000–U+2A6DF).
//   3. Write /ocr/output/<name>.json with the structure:
//
//        {
//          "source": "<name>.png",
//          "blocks": [
//            {
//              "text": "你好",
//              "confidence": 0.9876,
//              "box": [[x0,y0],[x1,y1],[x2,y2],[x3,y3]]
//            },
//            ...
//          ]
//        }
//
//   4. Remove the processed image from /ocr/capture/ to avoid reprocessing.
//
// The program polls the capture directory every POLL_INTERVAL_MS milliseconds
// (default 500 ms, overridable via OCR_POLL_INTERVAL_MS).
//
// Environment variables:
//   OCR_CAPTURE_DIR       – capture directory  (default /ocr/capture)
//   OCR_OUTPUT_DIR        – output directory   (default /ocr/output)
//   OCR_DET_MODEL_DIR     – PaddleOCR detection  model dir
//   OCR_REC_MODEL_DIR     – PaddleOCR recognition model dir
//   OCR_CLS_MODEL_DIR     – PaddleOCR classifier model dir (optional)
//   OCR_REC_CHAR_DICT     – path to ppocr_keys_v1.txt character dictionary
//   OCR_POLL_INTERVAL_MS  – poll interval in milliseconds (default 500)
//   OCR_USE_GPU           – set to "1" to enable GPU inference (default 0)

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "paddle_api.h"
#include "ppocr/include/paddleocr.h"

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string env_or(const char *name, const char *fallback) {
    const char *val = std::getenv(name);
    return (val && val[0] != '\0') ? std::string(val) : std::string(fallback);
}

// Returns true if the UTF-8 string contains at least one CJK Unified Ideograph.
// Covers:
//   U+4E00–U+9FFF   CJK Unified Ideographs
//   U+3400–U+4DBF   CJK Unified Ideographs Extension A
//   U+20000–U+2A6DF CJK Unified Ideographs Extension B
static bool has_chinese(const std::string &utf8) {
    const unsigned char *p   = reinterpret_cast<const unsigned char *>(utf8.c_str());
    const unsigned char *end = p + utf8.size();
    while (p < end) {
        uint32_t cp = 0;
        unsigned char b = *p;
        int extra = 0;
        if      (b < 0x80)             { cp = b;        extra = 0; }
        else if ((b & 0xE0) == 0xC0)   { cp = b & 0x1F; extra = 1; }
        else if ((b & 0xF0) == 0xE0)   { cp = b & 0x0F; extra = 2; }
        else if ((b & 0xF8) == 0xF0)   { cp = b & 0x07; extra = 3; }
        else                           { ++p; continue; }
        ++p;
        for (int i = 0; i < extra && p < end; ++i, ++p)
            cp = (cp << 6) | ((*p) & 0x3F);
        if ((cp >= 0x4E00 && cp <= 0x9FFF)   ||
            (cp >= 0x3400 && cp <= 0x4DBF)   ||
            (cp >= 0x20000 && cp <= 0x2A6DF))
            return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Minimal JSON serialisation (no external dependency)
// ---------------------------------------------------------------------------

static std::string json_escape(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else if (c < 0x20)  { /* skip control chars */ }
        else                out += c;
    }
    return out;
}

static std::string build_json(const std::string &source_name,
                               const std::vector<PaddleOCR::OCRPredictResult> &results) {
    std::ostringstream js;
    js << "{\n  \"source\": \"" << json_escape(source_name) << "\",\n";
    js << "  \"blocks\": [";

    bool first = true;
    for (const auto &r : results) {
        if (!has_chinese(r.text)) continue;
        if (!first) js << ",";
        first = false;

        js << "\n    {\n";
        js << "      \"text\": \"" << json_escape(r.text) << "\",\n";
        js << "      \"confidence\": " << r.score << ",\n";
        js << "      \"box\": [";
        for (size_t i = 0; i < r.box.size(); ++i) {
            if (i > 0) js << ",";
            js << "[" << r.box[i][0] << "," << r.box[i][1] << "]";
        }
        js << "]\n    }";
    }

    js << "\n  ]\n}\n";
    return js.str();
}

// ---------------------------------------------------------------------------
// OCR processor
// ---------------------------------------------------------------------------

class OcrProcessor {
public:
    explicit OcrProcessor(const std::string &det_dir,
                          const std::string &rec_dir,
                          const std::string &cls_dir,
                          const std::string &char_dict,
                          bool use_gpu)
    {
        PaddleOCR::PaddleOCR::Config cfg;
        cfg.det_model_dir  = det_dir;
        cfg.rec_model_dir  = rec_dir;
        cfg.cls_model_dir  = cls_dir;
        cfg.char_list_file = char_dict;
        cfg.use_gpu        = use_gpu ? 1 : 0;
        cfg.use_angle_cls  = !cls_dir.empty() ? 1 : 0;
        cfg.lang           = "ch";
        cfg.det            = 1;
        cfg.rec            = 1;
        cfg.type           = "ocr";

        ocr_ = std::make_unique<PaddleOCR::PaddleOCR>(cfg);
    }

    // Process one image file; returns the JSON string.
    std::string process(const fs::path &image_path) {
        cv::Mat img = cv::imread(image_path.string());
        if (img.empty())
            throw std::runtime_error("Failed to load image: " + image_path.string());

        std::vector<cv::Mat> images = {img};
        auto results = ocr_->ocr(images, /*det=*/true, /*rec=*/true, /*cls=*/false);
        return build_json(image_path.filename().string(), results[0]);
    }

private:
    std::unique_ptr<PaddleOCR::PaddleOCR> ocr_;
};

// ---------------------------------------------------------------------------
// File-extension filter
// ---------------------------------------------------------------------------

static const std::set<std::string> IMAGE_EXTS = {".png", ".jpg", ".jpeg", ".bmp"};

static bool is_image(const fs::path &p) {
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return IMAGE_EXTS.count(ext) > 0;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
    try {
        const std::string capture_dir = env_or("OCR_CAPTURE_DIR", "/ocr/capture");
        const std::string output_dir  = env_or("OCR_OUTPUT_DIR",  "/ocr/output");
        const std::string det_dir     = env_or("OCR_DET_MODEL_DIR",
                                               "/models/ch_PP-OCRv4_det_infer");
        const std::string rec_dir     = env_or("OCR_REC_MODEL_DIR",
                                               "/models/ch_PP-OCRv4_rec_infer");
        const std::string cls_dir     = env_or("OCR_CLS_MODEL_DIR", "");
        const std::string char_dict   = env_or("OCR_REC_CHAR_DICT",
                                               "/models/ppocr_keys_v1.txt");
        const bool use_gpu = (env_or("OCR_USE_GPU", "0") == "1");

        const int poll_ms = []() -> int {
            const char *v = std::getenv("OCR_POLL_INTERVAL_MS");
            if (v && v[0] != '\0') {
                try { return std::stoi(v); } catch (...) {}
            }
            return 500;
        }();

        if (!fs::is_directory(capture_dir))
            throw std::runtime_error("Capture directory does not exist: " + capture_dir);
        if (!fs::is_directory(output_dir))
            throw std::runtime_error("Output directory does not exist: " + output_dir);

        std::cerr << "[ocr] Starting OCR service\n"
                  << "[ocr]   capture : " << capture_dir << "\n"
                  << "[ocr]   output  : " << output_dir  << "\n"
                  << "[ocr]   det     : " << det_dir     << "\n"
                  << "[ocr]   rec     : " << rec_dir     << "\n"
                  << "[ocr]   cls     : " << (cls_dir.empty() ? "(disabled)" : cls_dir) << "\n"
                  << "[ocr]   gpu     : " << (use_gpu ? "yes" : "no") << "\n"
                  << "[ocr]   poll ms : " << poll_ms << "\n";

        OcrProcessor processor(det_dir, rec_dir, cls_dir, char_dict, use_gpu);
        std::cerr << "[ocr] Models loaded. Watching " << capture_dir << " ...\n";

        while (true) {
            for (const auto &entry : fs::directory_iterator(capture_dir)) {
                if (!entry.is_regular_file()) continue;
                const fs::path &img_path = entry.path();
                if (!is_image(img_path)) continue;

                fs::path out_path = fs::path(output_dir) / (img_path.stem().string() + ".json");

                std::cerr << "[ocr] Processing: " << img_path.filename().string() << "\n";
                try {
                    std::string json = processor.process(img_path);

                    std::ofstream out(out_path);
                    if (!out)
                        throw std::runtime_error("Cannot open output file: " + out_path.string());
                    out << json;
                    out.close();

                    std::cerr << "[ocr] Written:    " << out_path.filename().string() << "\n";
                    fs::remove(img_path);

                } catch (const std::exception &ex) {
                    std::cerr << "[ocr] ERROR processing "
                              << img_path.filename().string()
                              << ": " << ex.what() << "\n";
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(poll_ms));
        }

    } catch (const std::exception &ex) {
        std::cerr << "[ocr] FATAL: " << ex.what() << "\n";
        return 1;
    }
}
