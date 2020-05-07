//
// Created by Miguel Alonso Jr on 2019-02-28.
//

#include "stego.h"
#include "bitmap.h"
#include "util.h"
#include <stdio.h>


unsigned char _get_bit(char byte, int bit_number) {
    return (unsigned char) ((byte >> bit_number) & 1);
}

void _store_length(long size, unsigned char *data) {
    data[0] = (unsigned char) ((size >> 24) & 0xFF);
    data[1] = (unsigned char) ((size >> 16) & 0xFF);
    data[2] = (unsigned char) ((size >> 8) & 0xFF);
    data[3] = (unsigned char) (size & 0xFF);
}

long _get_length(const unsigned char *data) {
    long length = 0;
    length = (long) (data[3] & 0xFF);
    length = length | (long) ((data[2] & 0xFF) << 8);
    length = length | (long) ((data[1] & 0xFF) << 16);
    length = length | (long) ((data[0] & 0xFF) << 24);

    return length;
}

bool encode(char *text_source, char *original_image, char *destination_image) {
    char *error_message = NULL;
    char buff;
    int index = 4;
    unsigned char mask = 1;
    FILE *text_file = _open_file(text_source, "r");
    long text_size = _get_text_length(text_file);
    Bitmap *image = read_bitmap(original_image, &error_message);
    _store_length(text_size, image->data);
	
	int byte_width = image->header.width_px * 3;	//Number of bytes available to encode in one row of pixels
	int width_count = 4;			//counts how many bytes inside one row of the image have been used
	int char_count_out = 0;			//counts how many characters have been read from the txt file by encode()
	int char_count_in;				//stores the index value the file stream should start from
	int total_char_count;			//stores the total number of characters inside the txt file
	
	FILE *count_f1;
	if((count_f1 = fopen("count2.txt", "r"))){
		fscanf(count_f1, "%d %d", &char_count_in, &total_char_count);	//assigns values to char_count_in and total_char_count
		fclose(count_f1);												//from count2.txt
	}
	
	fseek(text_file, char_count_in, SEEK_SET);			//moves index of file stream up by the value of char_count_in
	//(Instead of starting to read the file from the beginning, the program begins reading the file "char_count_in"
	//characters away from the start of the file.)
	
    do {
		if((byte_width - width_count) < 8)	//If image cannot store another full character byte...
			width_count = byte_width;		//Set width_count = byte_width, exiting the loop
		else{
			
			buff = (char) fgetc(text_file);
			//Iterates through 8 bytes of image, replacing LSB of image with 1 bit of char
			//in event of padding, index can be increased and loop can continue.
			//8 bytes of image data can hold 1 encoded char byte
			for (int i = 0; i < 8; i++) {
				image->data[index] = (image->data[index] & ~mask) | (_get_bit(buff, i) & mask);
				index++;
				width_count++;		//increments width_count every time a byte of the image is encoded
			}
			char_count_out++;		//increments every time a character is encoded into the image
		}        
    } while (!feof(text_file) && width_count < byte_width);	//Loop continues until width_count >= byte_width or EOF is reached
	//NOTE: This code only encodes the first row of image data, unlike the original code which attempts to encode
	//the entire image.
	
	FILE *count_f2;
	count_f2 = fopen("count1.txt", "w");	//write how many characters were read by encode() into count1.txt
	fprintf(count_f2, "%d", char_count_out);
	fclose(count_f2);

    write_bitmap(destination_image, image, &error_message);
    bool is_valid = check_bitmap_header(image->header, "sw_poster_copy.bmp");
    return is_valid;
}

bool decode(char *image_source, char *text_destination) {
    char *error_message = NULL;
    unsigned char buff = 0;
    unsigned char mask = 1;
    long message_length;
    Bitmap *image = read_bitmap(image_source, &error_message);	
    FILE *text_file = _open_file(text_destination, "w");
	message_length = _get_length(image->data);
	
	int byte_width = image->header.width_px * 3;	//Number of bytes available to encode in one row of pixels
	int width_count = 4;			//counts how many bytes inside one row of the image have been used
	int stop_i = 0;
	
	//Every 8 bytes of image = 1 char
	//Right-most bit of image data is replaced with one bit of char byte
	
	for (int index = 4; index < message_length * 8 && width_count < byte_width && stop_i == 0; index++) {
		//First part: mask that only looks at right-most bit of image data
		//Second part: Shifting bits so that only char bit from image data remains (no image data left)
        buff = buff | ((image->data[index] & mask) << ((index - 4) % 8));
        if (((index - 4) % 8) > 6) {
			if(buff == '\0'){		//if decoded character is a null character...
				FILE *stop_f;
				stop_f = fopen("stop.txt", "w");
				fputc('n', stop_f);
				fclose(stop_f);
				stop_i = 1;			//set stop = 1, exiting the loop and stopping the decoding of the image
			}
			else{
				fputc(buff, text_file);		//else, write decoded character in txt file
				buff = 0;					//reset value of buff (which holds the decoded character)
			}
        }
		width_count++;				//increments width_count every time a byte of the image is decoded
    }

    bool is_valid = true;
    return is_valid;
}

//TODO: decode

