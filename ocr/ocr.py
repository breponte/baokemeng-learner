from paddleocr import PaddleOCR

# Uses PP-OCRv6 models by default
ocr = PaddleOCR(
    use_doc_orientation_classify=False, # Disables document orientation classification model via this parameter
    use_doc_unwarping=False, # Disables text image rectification model via this parameter
    use_textline_orientation=False, # Disables text line orientation classification model via this parameter
    device="gpu:0",
    engine="onnxruntime",
    lang="ch"
)

result = ocr.predict("./input/general_ocr_002.png")  
for res in result:  
    res.print()  
    res.save_to_img("output")  
    res.save_to_json("output")