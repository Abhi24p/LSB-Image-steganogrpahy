#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"

/* Structure to store decoding information */
typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    /* Secret File Info */
    char output_fname[100];
    FILE *fptr_output;
    int size_secret_file;
    char *extn_secret_file;
    char file_extn[10];
    int extn_size;

} DecodeInfo;

/* Function Prototypes */

/* Validate command line arguments for decoding */
DStatus read_and_validate_decode_args(char *argv[], DecodeInfo *decodeInfo);

/* Perform Decoding */
DStatus do_decoding(DecodeInfo *decodeInfo);

/* Skip BMP Header */
DStatus skip_bmp_header(FILE *fptr_src_image);

/* Decode Magic String */
DStatus decode_magic_string(DecodeInfo *decodeInfo);

/* Decode secret file extension size */
DStatus decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extension */
DStatus decode_secret_file_extn(DecodeInfo *decodeInfo);

/* Open secret file to store decoded data */
DStatus open_decode_files(DecodeInfo *decInfo);

/* Decode secret file size */
DStatus decode_secret_file_size(DecodeInfo *decodeInfo);

/* Decode secret file data */
DStatus decode_secret_file_data(DecodeInfo *decodeInfo);

/* Decode 1 byte from LSB */
DStatus decode_byte_from_lsb(char *image_buffer);

/* Decode integer (4 bytes) from LSB */
DStatus decode_size_from_lsb(char *image_buffer);

#endif