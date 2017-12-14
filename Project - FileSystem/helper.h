#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define IMAGE_SIZE 512000
#define COMMENT_SIZE 100	

typedef struct header_struct{
	int first_image_meta_address;
	int next_free_offset;
	int current_image_id;
	int current_comment_id;
}header;


typedef struct Image_meta_struct{
	int image_id;
	int is_valid;
	int image_address;
	char image_name[28];
	int image_size;
	int next_image_offset;
	int first_comment_offset;
	int last_comment_offset;
}image_meta;


typedef struct Image_data_struct{
	char data[IMAGE_SIZE];
	int next_block;
}image_data;


typedef struct comment_info{
	int comment_id;
	int is_valid;
	char comment_data[COMMENT_SIZE];
	int next_comment;
}comment;

image_meta *current_image_meta = NULL;
int current_image_meta_offset = 0;

void *read_memory(FILE *fp, int offset, int size){
	void * memory = malloc(size);
	fseek(fp, offset, SEEK_SET);
	fread(memory, size, 1, fp);
	return memory;
}


void write_memory(FILE *fp, void *memory, int offset, int size){
	fseek(fp, offset, SEEK_SET);
	fwrite(memory, size, 1, fp);
}


header * createHeader(){
	header *head = (header *)malloc(sizeof(header));
	head->first_image_meta_address = -1;
	head->next_free_offset = sizeof(header);
	head->current_comment_id = 0;
	head->current_image_id = 0;
	return head;
}


void updateOffset(FILE *fp, int offset, header *head){
	head->next_free_offset = offset;
	write_memory(fp, head, 0, sizeof(header));
}

int loadOffset(FILE *fp){
	header *head = (header *)read_memory(fp, 0, sizeof(header));
	if (head->next_free_offset <= 0)
		return 0;
	return head->next_free_offset;
}


image_meta * createImageMeta(FILE *fp,header *head){
	printf("Enter Image Name -");
	image_meta *new_image_meta = (image_meta *)malloc(sizeof(image_meta));
	fflush(stdin);
	gets(new_image_meta->image_name);
	new_image_meta->first_comment_offset = -1;
	new_image_meta->last_comment_offset = -1;
	new_image_meta->is_valid = 1;
	new_image_meta->next_image_offset = -1;
	head->current_image_id++;
	if (head->first_image_meta_address == -1){
		head->first_image_meta_address = sizeof(header);
	}
	write_memory(fp, head, 0, sizeof(header));
	new_image_meta->image_id = head->current_image_id;
	new_image_meta->image_size = 512000;
	new_image_meta->image_address = loadOffset(fp) + sizeof(image_meta);
	write_memory(fp, head, 0, sizeof(header));
	current_image_meta = new_image_meta;
	return new_image_meta;
}


comment *createComment(FILE *fp,header *head){
	comment *new_Comment = (comment *)malloc(sizeof(comment));
	printf("Enter Comment -");
	fflush(stdin);
	gets(new_Comment->comment_data);
	new_Comment->is_valid = 1;
	new_Comment->next_comment = -1;
	head->current_comment_id++;
	new_Comment->comment_id = head->current_comment_id;
	write_memory(fp, head, 0, sizeof(header));
	return new_Comment;
}


