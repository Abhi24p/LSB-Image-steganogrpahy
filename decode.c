
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Read and validate Decode args from argv */
DStatus read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //Check if argv[2] has a file or not 
    if (argv[2] == NULL)
    {
        printf("Error : provide a stego image\n");
        return d_failure;
    }

    // Validate that the provided file has a .bmp extension
    // Decoding works only on BMP images
    if (strstr(argv[2], ".bmp") == NULL)
        return d_failure;

    // Store the stego image file name in the structure
    decInfo->stego_image_fname = argv[2];

    // Check if the user provided an output filename (argv[3])
    // If yes, copy it to the decode structure
    if (argv[3])
    strcpy(decInfo->output_fname, argv[3]);
    else
    // If no output file name provided, use default name "decoded"
    strcpy(decInfo->output_fname, "decoded");

    return d_success; // validations successful
}

/* Skip BMP header */
DStatus skip_bmp_header(FILE *fptr)
{   // Skip the 54-byte BMP header to reach pixel data
    fseek(fptr, 54, SEEK_SET);
    return d_success;
}



/* Get File pointers for input stego and output decoded files*/
DStatus open_decode_files(DecodeInfo *decInfo)
{
    // open Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego_image == NULL)
    {
        // print system error
    	perror("fopen");
    	fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->stego_image_fname);
    	return d_failure; 
    }
    // Open/create the output file where decoded secret data will be written
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    // Error handling for opening output file
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open output file %s\n", decInfo->output_fname);
        return e_failure;
    }
    return d_success;
    
}

/* Decode LSBs from image buffer into 1 byte */
DStatus decode_byte_from_lsb(char *image_buffer)
{
    unsigned char data = 0;
    // Loop through 8 bytes → extract 8 bits
    for (int i = 0; i < 8; i++)
    {   // Shift left to make space for next bit
        data = data << 1;
        data |= (image_buffer[i] & 1);  // Extract LSB and insert into data
    }
    return data;
}

// Function definition decode size from lsb
DStatus decode_size_from_lsb(char *image_buffer)
{
    int size = 0;
    // Extract 32 bits → each bit comes from LSB of one image byte
    for (int i = 0; i < 32; i++)
    {   // Shift left to make space for next bit
        size = (size << 1);
        size |= (image_buffer[i] & 1);   // Add LSB to size
    }
    return size;
}



/* Deore Magic String from image */
DStatus decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic_string[10];
    // Read MAGIC_STRING length bytes
    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
       fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
       magic_string[i]= decode_byte_from_lsb(image_buffer);    // decode 1 byte
    }

    magic_string[strlen(MAGIC_STRING)] = '\0';  
    // Compare with expected MAGIC_STRING
    if (strcmp(magic_string, MAGIC_STRING) == 0)
    {
        printf("INFO: Magic string matched successfully.\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Magic string mismatch.\n");
        return e_failure;
    }

}


// Function definition for decode file extn size
DStatus decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char image_buffer[32];
    // Read 32 bytes → 32 bits of length
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);
    // Convert LSB encoded bits → integer
    decInfo->extn_size = decode_size_from_lsb(image_buffer);

    return d_success;

}


/* Decode secret file extenstion */
DStatus decode_secret_file_extn(DecodeInfo *decInfo)
{
    char image_buffer[8];
    int extn_size;
    // Read each extension character
    for (int i = 0; i < decInfo->extn_size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        decInfo->file_extn[i] = decode_byte_from_lsb(image_buffer);

    }
    // Null-terminate
    decInfo->file_extn[decInfo->extn_size] = '\0';  

    //Check if the output filename already ends with the extension 
    int name_len = strlen(decInfo->output_fname);
    int ext_len  = strlen(decInfo->file_extn);

    //Check if the output filename already ends with the extension 
    if (name_len >= ext_len &&
        strcmp(decInfo->output_fname + name_len - ext_len, decInfo->file_extn) == 0)
    {
        // extension already present --> no change
    }
    else
    {
        // extension not present, so append
        strcat(decInfo->output_fname, decInfo->file_extn);
    }

    return d_success;
}


/* Decode secret file size */
DStatus decode_secret_file_size(DecodeInfo *decInfo)
{

    char image_buffer[32];

    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    decode_size_from_lsb(image_buffer);

    // decode integer from 32 LSB bits
    decInfo->size_secret_file = decode_size_from_lsb(image_buffer);

    return d_success;
}


/* Decode secret file data*/
DStatus decode_secret_file_data(DecodeInfo *decInfo)
{

    char image_buffer[8];
    char ch;
    // Read every byte of secret file
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        ch= decode_byte_from_lsb(image_buffer);   // decode one byte
        fputc(ch, decInfo->fptr_output);         // write to output file

    }

    return d_success;
   
}

/* Main decoding */
DStatus do_decoding(DecodeInfo *decInfo)
{
    printf("INFO : ## Decoding Procedure Started ##\n");

    // Open stego image FIRST
    printf("INFO : Opening stego image\n");
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (!decInfo->fptr_stego_image)
    {
        perror("fopen");
        fprintf(stderr, "ERROR : Unable to open file %s\n", decInfo->stego_image_fname);
        return d_failure;
    }
    printf("INFO : Done\n");

    //Skip 54-byte BMP header
    skip_bmp_header(decInfo->fptr_stego_image);

    /* Decode Magic String */
    printf("INFO : Decoding Magic String Signature\n");
    if (decode_magic_string(decInfo) != d_success)
    {
        printf("ERROR : Magic String Mismatch\n");
        fclose(decInfo->fptr_stego_image);
        return d_failure;
    }
    printf("INFO : Done\n");

    /*  Decode extension size */
    printf("INFO : Decoding Extension Size\n");
    if (decode_secret_file_extn_size(decInfo) != d_success)
    {
        printf("ERROR : Failed decoding extension size\n");
        fclose(decInfo->fptr_stego_image);
        return d_failure;
    }
    printf("INFO : Done\n");

    /* Decode extension (this appends to output_fname) */
    printf("INFO : Decoding Extension\n");
    if (decode_secret_file_extn(decInfo) != d_success)
    {
        printf("ERROR : Failed decoding extension\n");
        fclose(decInfo->fptr_stego_image);
        return d_failure;
    }
    printf("INFO : Done\n");

    /* Now open output file with the FINAL name (after extension appended) */
    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if (!decInfo->fptr_output)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open output file %s\n", decInfo->output_fname);
        fclose(decInfo->fptr_stego_image);
        return d_failure;
    }

    /*  Decode secret file size */
    printf("INFO : Decoding Secret File Size\n");
    if (decode_secret_file_size(decInfo) != d_success)
    {
        printf("ERROR : Failed decoding secret file size\n");
        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_output);
        return d_failure;
    }
    printf("INFO : Done\n");

    /*  Decode secret file data */
    printf("INFO : Decoding Secret File Data\n");
    if (decode_secret_file_data(decInfo) != d_success)
    {
        printf("ERROR : Failed decoding secret file data\n");
        fclose(decInfo->fptr_stego_image);
        fclose(decInfo->fptr_output);
        return d_failure;
    }
    printf("INFO : Done\n");
    printf("INFO: Decoding completed successfully.\n");

    /* Close files */
    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    return d_success;
}
