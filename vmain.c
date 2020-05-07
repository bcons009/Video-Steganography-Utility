#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define FFMPEG_PATH "/usr/bin/ffmpeg"
#define IMG_NAME "img%d.bmp"

//Prints an error message that informs the user how to use the vsteg executable
void print_help(char *path){
    printf("*** Video Steganography ***\n\n"
           "Usage:  \n"
           "%s [-e] <text file to encode> <source video> <destination video>\n"
           "%s [-d] <encoded video> <decoded file>\n\n"
           "-e : Encode text in video\n"
           "-d : Decode text from video\n",
           path, path);
}

//Counts the number of files in the specified directory using directory pointer *dr
int file_count(char *dir){
	struct dirent *de;
	DIR *dr = opendir(dir);
	int count = 0;
	
	while((de = readdir(dr)) != NULL){
		++count;
	}
	
	return count;
}

void encode_vid(char *text_source, char *original_vid, char *destination_vid){
	
	//create temporary "frames" directory
	if (mkdir("frames", ACCESSPERMS) == -1){
		//if "frames" directory already exists, print error message and exit program
		printf("Error! Directory \"frames\" could not be created. \nAnother \"frames\" directory was found, "
		 "which was not created by vsteg. \nPlease remove the duplicate \"frames\" directory and try again.\n");
		exit(1);
	}
	
	chdir("./frames");	//changes working directory to "frames"
	
	char *look_above = "../";
	//Dynamically allocates memory for string containing video to encode as well as string "../" to look
	//for video in parent directory of "frames"
	char *vid_above = (char *) malloc(1 + strlen(look_above)+ strlen(original_vid));
	strcpy(vid_above, look_above);
	strcat(vid_above, original_vid);	//look for target video in directory above "frames"
	char *args[] = {FFMPEG_PATH, "-i", vid_above, IMG_NAME, NULL};	//arguments for ffmpeg's execv
	
	pid_t pid1 = fork();	//fork
	
	if(pid1 == (pid_t) 0){
		execv(args[0], args);	//child extracts bitmap frames from target video
	}
	else{					
		pid1 = wait(NULL);		//parent waits for child to finish
	}
	
	char file_name[150] = "../";			//creates string for name of text file to encode into video, looking
	strcat(file_name, text_source);			//for that file in the parent directory of "frames"
	
	int total_char_count = 0;
	char ch;
	FILE *fp;
	fp = fopen(file_name, "r+");
	
	fseek(fp, 0, SEEK_END);	
	fputc('\0', fp);				//Place a null character at the end of the file, meant to 
	fseek(fp, 0, SEEK_SET);			//signal end of decoding for stego.c
	
	while (1) {						//Count the number of characters inside the txt file,
        ch = fgetc(fp);				//store that value inside of int total_char_count
        if (ch == EOF)
            break;
        ++total_char_count;
    }
	
	fclose(fp);
	
	int char_count = 0;
	
	int img_count = file_count(".") - 2;	//counts the number of frames in current directory using function file_count()
	
	//Loop continues until all images have been encoded or entire message has been encoded
	for(int i = 1; i < img_count && char_count < total_char_count; i++){
		char *im = "img";
		char num[30];
		sprintf(num, "%d", i);
		//Dynamically allocate memory for name of bmp frame to encode
		char *img = (char *) malloc(1 + strlen(im)+ strlen(num));
		strcpy(img, im);
		strcat(img, num);
		strcat(img, ".bmp");
		
		char *args2[] = {"../isteg", "-e", file_name, img, img, NULL};	//arguments to run isteg's encode via execvp
		
		pid_t pid2 = fork();		//fork
		
		if(pid2 == (pid_t) 0){
			execvp(args2[0], args2);	//child runs isteg to encode some of the text into the image
		}
		else{
			pid2 = wait(NULL);			//parent waits for child to finish
		}
		
		FILE *count_f1;
		if((count_f1 = fopen("count1.txt", "r"))){	//checks if file "count1.txt" exists
			int n;
			fscanf(count_f1, "%d", &n);		//count1.txt stores the index the file stream was on before ending in isteg's
			char_count += n;				//encoding. That value is read from count1.txt and stored into n, which is
			fclose(count_f1);				//then added to char_count, or the index of encode_vid's file stream.
		}
		
		FILE *count_f2;
		count_f2 = fopen("count2.txt", "w");						//Similarly, count2.txt tells isteg what index to start
		fprintf(count_f2, "%d %d", char_count, total_char_count);	//it's filestream to continue reading the txt file to encode.
		fclose(count_f2);							//It also tells isteg the total number of characters in the txt file.
	}
	
	remove("count1.txt");				//Once encoding, is complete, remove count1.txt and count2.txt
	remove("count2.txt");
	
	char resolution[20];			//Strings to store resoution, frames per second, and pixel format of video to encode
	char fps[3];
	char pix_fmt[20];
	
	for(int k = 0; k < 3; k++){
		char *ff;
		//Depending on iteration of loop, ff string stores different arguments for ffprobe
		switch(k){
			case 0:
				ff = "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 ";
				break;
			case 1:
				ff = "ffprobe -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of csv=s=x:p=0 ";
				break;
			case 2:
				ff = "ffprobe -v error -select_streams v:0 -show_entries stream=pix_fmt -of csv=s=x:p=0 ";
				break;
		}
		
		//ffprobe_call grabs the arguments from ff and concatenates the file name of the video specified in the 
		//user's input, making sure to check in "frames"'s parent directory
		char *ffprobe_call = (char *) malloc(1 + strlen(ff)+ strlen(file_name));
		strcpy(ffprobe_call, ff);
		strcat(ffprobe_call, file_name);
		
		//Grabs the output of ffprobe, which will either be the video's resolution, fps, or pixel format
		FILE *ffprobe_f;
		ffprobe_f = popen(ffprobe_call, "r");
		
		//Depending on iteration of loop, stores ffprobe's output into their respective strings
		switch(k){
			case 0:
				fscanf(ffprobe_f, "%s", resolution);
				break;
			case 1:
				fps[0] = fgetc(ffprobe_f);
				fps[1] = fgetc(ffprobe_f);
				break;
			case 2:
				fscanf(ffprobe_f, "%s", pix_fmt);
				break;
		}
		
		pclose(ffprobe_f);		//closes file stream for ffprobe's output
	}
	
	char *dest_vid = (char *) malloc(1 + strlen(look_above)+ strlen(destination_vid));
	strcpy(dest_vid, look_above);
	strcat(dest_vid, destination_vid);
		
	char *args3[] = {FFMPEG_PATH, "-r", fps, "-f", "image2", "-s", resolution, "-i", "img%d.bmp", 
			"-vcodec", "ffv1", "-pix_fmt", pix_fmt, dest_vid, NULL};	//creates arguments to call ffmpeg via execv
	
	pid_t pid3 = fork();	//fork
	
	if(pid3 == (pid_t) 0){
		execv(args3[0], args3);	//child stitches bitmap frames back into a video
	}
	else{					
		pid3 = wait(NULL);		//parent waits for child to finish
	}
	
	//Deletes all the frames left inside the "frames" folder
	for(int i = 1; i <= img_count; i++){
		char *im = "img";
		char num[30];
		sprintf(num, "%d", i);
		char *img = (char *) malloc(1 + strlen(im)+ strlen(num));
		strcpy(img, im);
		strcat(img, num);
		strcat(img, ".bmp");
		
		remove(img);
	}
	
	chdir("..");				//moves working directory back to "vsteg"
	
	rmdir("./frames");			//delete "frames" directory
}

void decode_vid(char *video_source, char *text_destination){
	
	//create temporary "frames" directory
	if (mkdir("frames", ACCESSPERMS) == -1){
		//if "frames" directory already exists, print error message and exit program
		printf("Error! Directory \"frames\" could not be created. \nAnother \"frames\" directory was found, "
		 "which was not created by vsteg. \nPlease remove the duplicate \"frames\" directory and try again.\n");
		exit(1);
	}
	
	chdir("./frames");	//changes working directory to "frames"
	
	char vid_above[150] = "../";
	strcat(vid_above, video_source);	//look for target video in directory above "frames"
	char *args[] = {FFMPEG_PATH, "-i", vid_above, IMG_NAME, NULL};	//arguments for ffmpeg's execv
	
	pid_t pid = fork();	//fork
	
	if(pid == (pid_t) 0){
		execv(args[0], args);	//child extracts bitmap frames from target video
	}
	else{					
		pid = wait(NULL);		//parent waits for child to finish
	}
	
	int img_count = file_count(".") - 2;	//counts the number of frames in current directory using function file_count()
	int j = 1;
	
	FILE *fout;
	char out_text[150] = "../";
	strcat(out_text, text_destination);
	fout = fopen(out_text, "w");	//create output file for decoded video text
	
	for(int i = 1; i <= img_count && access("stop.txt", F_OK) == -1; i++){
		
		char img[20] = "img";
		char txt[20] = "txt";
		char num[15];
		sprintf(num, "%d", i);
		strcat(img, num);
		strcat(img, ".bmp");		//format name of bmp file being decoded ("img" + i + ".bmp")
		strcat(txt, num);
		strcat(txt, ".txt");		//format name of txt file to store decoded text ("txt" + i + ".txt")
		
		char *args2[] = {"../isteg", "-d", img, txt, NULL};		//arguments for isteg's execv
		
		pid_t pid = fork();		//fork
		
		if(pid == (pid_t) 0){
			execvp(args2[0], args2);		//child decodes "img[i]" using isteg executable
		}
		else{
			pid = wait(NULL);			//parent waits for child to finish
		}
		
		FILE *f_bmptxt;
		f_bmptxt = fopen(txt, "r");		//opens "txt[i]" txt file
		
		while(1){						//copies contents of "txt[i]" into output txt file
			char c = fgetc(f_bmptxt);
			if(feof(f_bmptxt))
				break;
			fputc(c, fout);
		}
		
		fclose(f_bmptxt);				//closes "txt[i]" file
		
		remove(img);			//delete img[i] and txt[i]
		remove(txt);

		j++;			//int j is incremented, used to track undeleted frames should decode_vid() early
						//(via stop == 0 condition)
	}
	
	remove("stop.txt");
	
	
	
	for(int i = j; i <= img_count; i++){
		char img[20] = "img";
		char txt[20] = "txt";
		char num[15];
		sprintf(num, "%d", i);
		strcat(img, num);
		strcat(img, ".bmp");		//format name of bmp file being decoded ("img" + i + ".bmp")
		strcat(txt, num);
		strcat(txt, ".txt");		//format name of txt file to store decoded text ("txt" + i + ".txt")
		
		remove(img);			//delete img[i] and txt[i]
		remove(txt);
	}
	
	fclose(fout);				//closes output file
	
	chdir("..");				//moves working directory back to "vsteg"
	
	rmdir("./frames");			//delete "frames" directory
}

//main() works identically to isteg's main
int main(int argc, char **argv) {
	
	if ( argc != 5 && argc != 4 ) {
        print_help(argv[0]);
        exit(1);
    }
	
	int mode;

    if (!strcmp(argv[1], "-e"))
        mode = 1;
    else if (!strcmp(argv[1], "-d"))
        mode = 0;
    else {
        print_help(argv[0]);
        exit(1);
    }

    if(mode){
        encode_vid(argv[2], argv[3], argv[4]);		
    } else{
		decode_vid(argv[2], argv[3]);
    }

    return EXIT_SUCCESS;
}