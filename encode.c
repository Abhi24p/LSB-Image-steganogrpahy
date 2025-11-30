#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
// Find the size of secret file data
uint get_file_size(FILE *fptr)
{
    
    fseek(fptr, 0, SEEK_END);      // Move to end of file
    uint size = ftell(fptr);       // Get current file position (end = size)
    rewind(fptr);                  // Reset to start
    return size;
}



/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //Check if argv[2] has a .bmp file or not 
    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
    }
    else
    {
        printf("Invalid : source file must be a .bmp file\n");
        return e_failure;
    }

    // Validate secret file (.txt, .c, .sh, .pdf)
    if (strstr(argv[3], ".txt") != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else if (strstr(argv[3], ".c") != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else if (strstr(argv[3], ".sh") != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else if (strstr(argv[3], ".pdf") != NULL)
    {
        encInfo->secret_fname = argv[3];
    }
    else
    {
        printf("Invalid : secret file must be .txt / .c / .sh / .pdf\n");
        return e_failure;
    }

    // Validate output stego file (.bmp)
    if (argv[4] == NULL)
    {
        encInfo->stego_image_fname = "default_stego.bmp";
    }
    else if (strstr(argv[4], ".bmp") == NULL)
    {
        printf("Invalid : output file must be a .bmp file\n");
        return e_failure;
    }
    else
    {
        encInfo->stego_image_fname = argv[4];
    }

    return e_success;
}

/* Open all files used in encoding:
 * - Source image (.bmp)
 * - Secret file (text/binary)
 * - Destination stego image (.bmp)
*/
Status open_files(EncodeInfo *encInfo)
{
    // Open source image file (read in binary)
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if(encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "Invalid : Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Open stego image file (write in binary)
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

 /* Check if the source image has enough capacity to store:
 *  magic string        → 2 bytes
 *  extn size           → 4 bytes
 *  extn characters     → variable
 *  secret file size    → 4 bytes
 *  secret file data    → n bytes
 * Each byte requires 8 bits → 8 image bytes*/

Status check_capacity(EncodeInfo *encInfo)
{   
    //get total image capacity 
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    //get secret file size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    
    /* capacity = (magic string size(2*8) + secret extn size(4*8) + secret file extn(4*8) + secret file size(4*8) + secret file data size*8; */
    int capacity = 16 + 32 + 32 + 32 + (encInfo -> size_secret_file * 8);

    //check if image can store all the data
    if(encInfo->image_capacity > capacity)
    {
        return e_success;   
    }
    else
    {
        return e_failure;

    }
}
   
/* Copy  BMP header into the stego image*/
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // Setting pointer to point to 0th position
    rewind(fptr_src_image);
    char header[54];
    // Reading 54 bytes of header from source.bmp
    fread(header, 54, 1, fptr_src_image);
    // Writing the 54 bytes header to destination.bmp
    fwrite(header, 54, 1, fptr_dest_image);
    if(ftell(fptr_src_image) == ftell(fptr_dest_image))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }    
}

/* Encode  byte → into LSBs image bytes*/
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        image_buffer[i] &= 0xFE;                   // clear LSB
        image_buffer[i] |= ((data >> (7 - i)) & 1); // set bit
    }
    return e_success;
}
/** Encode 32-bit integer → into LSBs of 32 image bytes*/
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for (int i = 0; i < 32; i++)
    {
        imageBuffer[i] &= 0xFE;
        imageBuffer[i] |= ((size >> (31 - i)) & 1);
    }
    return e_success;
}
/*Encode magic string like "#*" for validation during decoding*/
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], imageBuffer);
        fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image);
    }
    return e_success;
}
/*Encode secret file extension size*/
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char imageBuffer[32];
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);
    encode_size_to_lsb(size, imageBuffer);
    fwrite(imageBuffer, 32, 1, encInfo->fptr_stego_image);
    return e_success;
}
/*Encode file extension characters*/
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char imageBuffer[8];
    for(int i=0; i<strlen(file_extn); i++)
    {
        fread(imageBuffer,8 ,1, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],imageBuffer);
        fwrite(imageBuffer,8 ,1 ,encInfo->fptr_stego_image);
    }
    return e_success;
}
/*Encode secret file size*/
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imageBuffer[32];
    fread(imageBuffer, 32, 1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, imageBuffer);
    fwrite(imageBuffer,32 ,1 ,encInfo->fptr_stego_image);
    return e_success;
}
/*Encode secret file data byte-by-byte*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    if(!encInfo || !encInfo->fptr_secret || !encInfo->fptr_src_image || !encInfo->fptr_stego_image)
    {
        return e_failure;
    }
   
    rewind(encInfo->fptr_secret);

    int byte_read;
    char imageBuffer[8];

    // Read secret file one byte at a time to avoid large memory use
    while(fread(&byte_read, 1, 1, encInfo->fptr_secret) == 1)
    {
        if(fread(imageBuffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            return e_failure; 
        }
        encode_byte_to_lsb((char)byte_read, imageBuffer);

        if (fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image) != 1)
        {
            return e_failure; 
        }
            
    }

    return e_success;
}
/*Copy leftover image data after encoding*/
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch, 1, 1, fptr_src) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

/*The main encoding controller function*/
Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");
    /* Open source image, secret file, and create stego output file */
    if (open_files(encInfo) == e_success)
    {
        // get the actual size of secret.txt
        encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
        printf("INFO : Opened SkeletonCode/extension file \n");
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR: Failed to Open files \n");
        return e_failure;
    }
    /* Check if image has enough capacity to encode all required data */
    printf("INFO : ##---Encoding Procedure Started---##\n");
    printf("INFO : Checking for SkeletonCode/.ext file capacity to handle secret.txt\n");
    if (check_capacity(encInfo) == e_success)
    {
        printf("INFO : Done. Found OK\n");
    }
    else
    {
        printf("ERROR : Image cannot hold secret data\n");
        return e_failure;
    }
    /* Copy the 54-byte BMP header unchanged to the stego image */
    printf("INFO : Copying Image Header\n"); 
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to copy bmp header\n");
        return e_failure;
    }
    /*Encode magic string ("#*") into image*/
    printf("INFO : Encoding Magic String Signature\n"); 
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode magic string\n");
        return e_failure;
    }
    /*Extract the extension from secret file*/
    printf("INFO : Encoding secret.txt File Extenstion Size\n"); 
    // Extract extension
    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file extn size\n");
        return e_failure;
    }
    /*Encode actual extension characters*/
    printf("INFO : Encoding secret.txt File Extenstion\n"); 
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file extn\n");
        return e_failure;
    }/*Encode secret file size (in bytes)  */
    printf("INFO : Encoding secret.txt File Size\n");
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file size\n");
        return e_failure;
    }
    /*Encode secret file data byte-by-byte */
    printf("INFO : Encoding secret.txt File Data\n"); 
    if (encode_secret_file_data(encInfo) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to encode secret file data\n");
        return e_failure;
    }
    /*Copy the remaining pixels of the image*/
    printf("INFO : Copying Left Over Data\n"); 
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        printf("INFO : Done\n");
    }
    else
    {
        printf("ERROR : Failed to copy remaining data successfully\n");
        return e_failure;
    }

    // close all the opened files
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}