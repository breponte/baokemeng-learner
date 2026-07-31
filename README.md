# Bǎokěmèng Learner
A screen reader tool used to scan dialogue/text in games (specifically Pokemon) and produce audio output as well as translations.

## Toolchain

### Screen/Window Capture

__Bǎokěmèng Learner__ uses [XCap](https://github.com/nashaofu/xcap) to _implement screen capture for the optical character recognition tool_. XCap is a _cross-platform screen capture library_ with support for Linux (X11/Wayland), MacOS, and Windows.

The project will periodically capture a user-specified resizable region to upload to the OCR tool. XCap captures at regular intervals and NOT on input because dialogue can progress automatically without user input.

Additionally, there will be a second window that can capture on-demand in order to process text outside of the dialogue region. For example, text that appears in yes/no prompts, level up windows, or any text that deviates from the typical dialogue area.

### Optical Character Recognition

__Bǎokěmèng Learner__ uses [PaddleOCR](https://github.com/PADDLEPADDLE/PADDLEOCR) for the _optical character recognition tool to help with converting text data in images into parsable text_. This tool is run _locally_. This tool natively supports Chinese character recognition, which is the primary character recognition target.

PaddleOCR processes the image from the window capture and extracts the readable text as a JSON. The extracted Chinese characters are then sent to the text-to-speech and translation tool. 

### Text-to-Speech

__Bǎokěmèng Learner__ uses [sherpa-onnx](https://github.com/k2-fsa/sherpa-onnx) for the _text-to-speech output from the OCR extracted text_. This tool is run _locally_. The text from the OCR tool will serve as input to the sherpa-onnx model for text-to-speech via API call.

### Translation

__Bǎokěmèng Learner__ uses [Baidu Translation](https://fanyi-api.baidu.com) for the _Chinese to English translation_. This tool is run via API requests with a limited rate of ~50,000 word. The text from the OCR tool will serve as input to the Baidu Translation for translation via API call.

### Screen Overlay

__Bǎokěmèng Learner__ uses [SDL3](https://github.com/libsdl-org/SDL) for the _screen overlay in order to display buttons and highlights of current spoken text and translated text_. The screen overlay will portray all these tools in a user-friendly way, allowing access and interfacing with all the tools.
