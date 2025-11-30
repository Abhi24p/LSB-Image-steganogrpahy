/*
*Name:Abhishek Patil.
*Date:11/11/2025
*Project Title: LSB IMAAGE STEGANOGRAPHY
*Description:
This project is a console-based LSB Image Steganography application built for Linux terminal environments. 
It enables users to hide and retrieve secret data—such as text or files—inside BMP images using the Least 
Significant Bit (LSB) technique, a widely used and effective method of steganography.

*------Encode------*
The encoding module hides a secret file inside a BMP image using the Least Significant Bit (LSB) technique. 
It validates inputs, checks image capacity, copies the BMP header, and then embeds the magic string, 
file extension, extension size, file size, and the secret file data into the image’s pixel bytes. 
Each byte of secret data is encoded into 8 image bytes using LSB substitution, producing a valid stego BMP image.

*------Decode------*
The decoding module extracts the hidden secret file from the stego BMP image by reversing the LSB 
encoding process. It skips the BMP header, reads and verifies the magic string, decodes the file 
extension size, extension, secret file size, and then retrieves the original file data byte-by-byte 
from the image’s LSBs. Finally, it reconstructs and saves the hidden file exactly as it was before encoding.

This project demonstrates practical knowledge of bit-level manipulation, BMP image structure, 
and file I/O operations in C. It serves as a strong learning experience in image processing, binary operations, 
and foundational information security techniques.
*/

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

     if(argc < 3)
    {
        printf("##Error: Insufficient arguments##\n");
        //return 1;
        printf("Usage:\n");
        printf("  To encode : ./a.out -e <.bmp file> <.txt file> [output file(optional)]\n");
        printf("  To decode : ./a.out -d <.bmp file> [output file(optional)]\n");
        return e_failure;
    }
    if(check_operation_type(argv[1]) == e_encode)
    {
        if(argc < 4)
        {
            printf("##Error: Insufficient arguments##\n");
            //return 1;
            printf("Usage:\n");
            printf("  To encode : ./a.out -e <.bmp file> <.txt file> [output file(optional)]\n");
            printf("  To decode : ./a.out -d <.bmp file> [output file(optional)]\n");
            return e_failure;
         printf("Start Encoding operation...\n");
        }
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
           printf("Validation Successful\n");
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding Completed Successfully\n");
            }
            else
            {
                printf("ERROR: Encoding Failed\n");
                return e_failure;
            }
        }
        else
        {
           printf("ERROR: Validation Failed\n"); 
           return e_failure;
        }
    }
    else if (check_operation_type(argv[1]) == e_decode)
    {
         if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            printf("Validation Successful\n");
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding Completed Successfully\n");
            }
            else
            {
                printf("ERROR: Decoding Failed\n");
                return e_failure;
            }
        }
        else
        {
            printf("ERROR: Validation Failed\n");
            return e_failure;
        }
    }
    else
    {
        printf("ERROR: Unsupported operation type '%s'\n", argv[1]);
        printf("Usage:\n");
        printf("  To encode : ./a.out -e <.bmp file> <.txt file> [output file(optional)]\n");
        printf("  To decode : ./a.out -d <.bmp file> [output file(optional)]\n");
        return e_failure;
    }

    return e_success;     
    
}

OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol, "-e") == 0)           
    {
        return e_encode ;
    }
    else if(strcmp(symbol, "-d") == 0)
    {
        return e_decode ;
    }
    else
    {
        return e_unsupported;
    }
}