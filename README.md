# LSB-Image-steganogrpahy

This project implements image steganography using the Least Significant Bit (LSB) technique to securely hide text data inside BMP images. It provides encoding and decoding functionality with reliable file extraction and magic-string validation.

# 🖼️ Image Steganography using LSB Technique (BMP)

This project implements **Image Steganography** using the **Least Significant Bit (LSB)** method.  
It allows you to **encode** secret data (text files) inside a BMP image and later **decode** it to retrieve the hidden message.

---

## 📁 Project Structure

├── src/
│ ├── encode.c
│ ├── decode.c
│ ├── common.h
│ ├── types.h
│ ├── encode.h
│ ├── decode.h
│ └── main.c
├── images/
│ ├── input.bmp
│ ├── output_stego.bmp
├── secret/
│ └── secret.txt
└── README.md

---

## 🔍 **Features**

✔️ Encode secret text inside a 24-bit BMP image  
✔️ Decode hidden text from the encoded image  
✔️ Uses **LSB (Least Significant Bit)** substitution  
✔️ Supports:  
- Secret file extension  
- Secret file size  
- Magic string checking (`#*`)  
✔️ Error-handling for invalid files  
✔️ Simple command-line interface

---

## 🚀 How It Works

### **Encoding Process**
1. Read the input BMP image  
2. Copy BMP header (54 bytes)  
3. Embed:
   - Magic string `#*`
   - Secret file extension
   - Size of secret file
   - File data (character-by-character)
4. Save as a new stego image

### **Decoding Process**
1. Read encoded BMP image  
2. Detect magic string  
3. Extract file extension  
4. Extract file size  
5. Reconstruct the secret file  

---

## 🛠️ Build & Run

### **Compile**
```bash
gcc *.c

Encoding
./stego -e input.bmp secret.txt output.bmp

Decoding
./stego -d output.bmp decoded_secret.txt

🧰 Requirements

GCC compiler
24-bit BMP image
C standard libraries

In a BMP file, each pixel is represented as R G B (3 bytes).
The least significant bit of each byte does not drastically affect the pixel color, so we store the secret bits there.

Example:

Pixel Byte:  11001010  
Secret Bit:                    1  
Modified Byte: 11001011  
