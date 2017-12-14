#include"helper.h"

int ifFileExists(FILE *fp, header *head,char *name){
	int list_offset = head->first_image_meta_address, count = 0;
	while (list_offset > 0){
		image_meta *list_image_meta = (image_meta *)read_memory(fp, list_offset, sizeof(image_meta));
		if (!strcmp(list_image_meta->image_name,name) && list_image_meta->is_valid){
			return 1;	
		}
		list_offset = list_image_meta->next_image_offset;
	}
	return 0;
}


void uploadImage(FILE *fp, header *head){
	int read_offset = 0, source_size = 0;
	image_meta *new_image_meta = createImageMeta(fp, head);
 	if (current_image_meta != NULL){
		if (ifFileExists(fp, head, new_image_meta->image_name)){
			printf("\nFile already exists.\n");
			return;
		}
	}
	image_meta *update_old_meta = (image_meta *)malloc(sizeof(image_meta));
	int offset = loadOffset(fp);
	if (current_image_meta != NULL){
		update_old_meta = (image_meta *)read_memory(fp, current_image_meta_offset, sizeof(image_meta));
		update_old_meta->next_image_offset = loadOffset(fp);
		write_memory(fp, update_old_meta, current_image_meta_offset, sizeof(image_meta));
		current_image_meta = update_old_meta;
	}
	FILE *source_file = fopen(new_image_meta->image_name, "rb");
	if (source_file == NULL){
		printf("\nFile Does not exist.\n");
		return;
	}
	fseek(source_file, 0, SEEK_END);
	source_size = ftell(source_file);
	new_image_meta->image_size = source_size;
	write_memory(fp, new_image_meta, offset, sizeof(image_meta));
	current_image_meta_offset = offset;
	offset += sizeof(image_meta);
	updateOffset(fp, offset, head);
	fseek(source_file, 0, SEEK_SET);
	if (source_size <= IMAGE_SIZE){
		image_data *new_image_data = (image_data *)malloc(sizeof(image_data));
		for (int i = 0; i < 512000; i++){
			new_image_data->data[i] = '\0';
		}
		//strcpy(new_image_data->data, (char *)read_memory(source_file, 0, source_size));
		fread(new_image_data->data, source_size, 1, source_file);
		new_image_data->next_block = -1;
		write_memory(fp, new_image_data, offset, sizeof(image_data));
		offset += sizeof(image_data);
		updateOffset(fp, offset, head);
	}
	else{
		while (source_size > IMAGE_SIZE){
			image_data *new_image_data = (image_data *)malloc(sizeof(image_data));
			for (int i = 0; i < IMAGE_SIZE; i++){
				new_image_data->data[i] = '\0';
			}
			//strcpy(new_image_data->data, (char *)read_memory(source_file, read_offset, IMAGE_SIZE));
			fread(new_image_data->data, IMAGE_SIZE, 1, source_file);
			new_image_data->next_block = loadOffset(fp) + sizeof(image_data);
			write_memory(fp, new_image_data, offset, sizeof(image_data));
			offset += sizeof(image_data);
			read_offset += IMAGE_SIZE;
			updateOffset(fp, offset, head);
			source_size -= IMAGE_SIZE;
		}
		if (source_size <= IMAGE_SIZE){
			image_data *new_image_data = (image_data *)malloc(sizeof(image_data));
			for (int i = 0; i < 512000; i++){
				new_image_data->data[i] = '\0';
			}
			//			strcpy(new_image_data->data, (char *)read_memory(source_file, 0, source_size));
			fread(new_image_data->data, source_size, 1, source_file);
			new_image_data->next_block = -1;
			write_memory(fp, new_image_data, offset, sizeof(image_data));
			offset += sizeof(image_data);
			updateOffset(fp, offset, head);
		}
	}
	current_image_meta = new_image_meta;
	fclose(source_file);
	printf("\nImage Uploaded Sucessfully!\n");
}


void listImages(FILE *fp, header *head){
	int list_offset = head->first_image_meta_address, count = 0;
	printf("------------------------------------------");
	printf("\n\tLIST OF IMAGES ");
	printf("\nImage ID\tImage Name\n");
	while (list_offset != -1){
		image_meta *list_image_meta = (image_meta *)read_memory(fp, list_offset, sizeof(image_meta));
		if (list_image_meta->is_valid != 0){
			printf("%d\t\t%s\n", list_image_meta->image_id, list_image_meta->image_name);
			count++;
		}
		list_offset = list_image_meta->next_image_offset;
	}
	if (!count)
		printf("No Images Found!\n");
}


void downloadImage(FILE *fp, header *head){
	int download_image_id = 0, download_image_meta_offset, found = 0, download_size = 0;
	char download_name[40] = "download - ";
	listImages(fp, head);
	image_meta *find_image_meta = (image_meta *)malloc(sizeof(image_meta));
	image_data *write_image_data = (image_data *)malloc(sizeof(image_data));
	printf("\nEnter Image ID to download - ");
	fflush(stdin);
	scanf("%d", &download_image_id);
	download_image_meta_offset = head->first_image_meta_address;
	while (!found && download_image_meta_offset != -1){
		find_image_meta = (image_meta *)read_memory(fp, download_image_meta_offset, sizeof(image_meta));
		if (find_image_meta->image_id == download_image_id){
			found = 1;
			break;
		}
		download_image_meta_offset = find_image_meta->next_image_offset;
	}
	if (found){
		strcat(download_name, find_image_meta->image_name);
		FILE *download = fopen(download_name, "wb");
		download_size = find_image_meta->image_size;
		int read_offset = find_image_meta->image_address;
		int write_address = 0;
		while (download_size >= IMAGE_SIZE){
			write_image_data = (image_data *)read_memory(fp, read_offset, IMAGE_SIZE);
			read_offset += (IMAGE_SIZE + 4);
			download_size -= (IMAGE_SIZE);
			write_memory(download, write_image_data, write_address, IMAGE_SIZE);
			write_address += IMAGE_SIZE;
		}
		if (download_size > 0){
			write_image_data = (image_data *)read_memory(fp, read_offset, download_size);
			//			read_offset += (IMAGE_SIZE + 4);
			write_memory(download, write_image_data, write_address, download_size);
			download_size -= (IMAGE_SIZE);
			//			write_address += IMAGE_SIZE;
		}
		fclose(download);
		printf("\nImage Downloaded Succesfully!\n");
	}
	else{
		printf("\nInvalid file chosen\n");
	}
}


void deleteImage(FILE *fp, header *head){
	int delete_image_id = 0, find_image_meta_offset, found = 0;
	listImages(fp, head);
	printf("\nEnter Image ID to delete - ");
	fflush(stdin);
	scanf("%d", &delete_image_id);
	find_image_meta_offset = head->first_image_meta_address;
	image_meta *find_image_meta = (image_meta *)malloc(sizeof(image_meta));
	while (!found && find_image_meta_offset != -1){
		find_image_meta = (image_meta *)read_memory(fp, find_image_meta_offset, sizeof(image_meta));
		if (find_image_meta->image_id == delete_image_id){
			found = 1;
			break;
		}
		find_image_meta_offset = find_image_meta->next_image_offset;
	}
	if (found){
		find_image_meta->is_valid = 0;
		write_memory(fp, find_image_meta, find_image_meta_offset, sizeof(image_meta));
		printf("\nImage Deleted Sucessfully!\n");
	}
	else{
		printf("Image Not Found\n");
	}
}


void addComment(FILE *fp, header *head){
	int selected_image_id = 0, find_image_meta_offset = 0, found = 0,comment_offset = 0;
	listImages(fp, head);
	printf("\nEnter Image ID - ");
	fflush(stdin);
	scanf("%d", &selected_image_id);
	comment *new_comment = createComment(fp, head);
	comment *read_comment = (comment *)malloc(sizeof(comment));
	image_meta *find_image_meta = (image_meta *)malloc(sizeof(image_meta));
	find_image_meta_offset = head->first_image_meta_address;
	while (!found && find_image_meta_offset != -1){
		find_image_meta = (image_meta *)read_memory(fp, find_image_meta_offset, sizeof(image_meta));
		if (find_image_meta->image_id == selected_image_id){
			found = 1;
			break;
		}
		find_image_meta_offset = find_image_meta->next_image_offset;
	}
	if (found){
		comment_offset = find_image_meta->last_comment_offset;
		read_comment = (comment *)read_memory(fp, comment_offset, sizeof(comment));
		read_comment->next_comment = loadOffset(fp);
		write_memory(fp, read_comment, comment_offset, sizeof(comment));
		find_image_meta->last_comment_offset = loadOffset(fp);
		if (find_image_meta->first_comment_offset == -1)
			find_image_meta->first_comment_offset = loadOffset(fp);
		write_memory(fp, find_image_meta, find_image_meta_offset, sizeof(image_meta));
	}
	write_memory(fp, new_comment, loadOffset(fp), sizeof(comment));
	updateOffset(fp, loadOffset(fp) + sizeof(comment), head);
	printf("\nComment added Succesfully!\n");
}


int viewComments(FILE *fp, header *head){
	int selected_image_id = 0, find_image_meta_offset = 0, found = 0;
	listImages(fp, head);
	printf("\nEnter Image ID - ");
	fflush(stdin);
	scanf("%d", &selected_image_id);
	comment *read_comment = (comment *)malloc(sizeof(comment));
	image_meta *find_image_meta = (image_meta *)malloc(sizeof(image_meta));
	find_image_meta_offset = head->first_image_meta_address;
	while (!found && find_image_meta_offset != -1){
		find_image_meta = (image_meta *)read_memory(fp, find_image_meta_offset, sizeof(image_meta));
		if (find_image_meta->image_id == selected_image_id){
			found = 1;
			break;
		}
		find_image_meta_offset = find_image_meta->next_image_offset;
	}
	if (found){
		printf("----------------------------");
		printf("\n\tCOMMENTS\n");
		int comment_offset = find_image_meta->first_comment_offset;
		if (comment_offset == -1){
			printf("\nNo Comments\n");
			return 0;
		}
		printf("Comment ID \t Comment\n");
		while (comment_offset != -1){
			read_comment = (comment *)read_memory(fp, comment_offset, sizeof(comment));
			if (read_comment->is_valid)
				printf("%d\t\t%s\n", read_comment->comment_id, read_comment->comment_data);
			comment_offset = read_comment->next_comment;
		}
	}
	printf("\n");
	return selected_image_id;
}


void deleteComment(FILE *fp, header *head){
	int selected_comment_id = 0, found = 0, find_image_meta_offset;
	int selected_image_id = viewComments(fp, head);
	printf("\nEnter Comment ID to delete - ");
	fflush(stdin);
	scanf("%d", &selected_comment_id);
	comment *read_comment = (comment *)malloc(sizeof(comment));
	image_meta *find_image_meta = (image_meta *)malloc(sizeof(image_meta));
	find_image_meta_offset = head->first_image_meta_address;
	while (!found && find_image_meta_offset != -1){
		find_image_meta = (image_meta *)read_memory(fp, find_image_meta_offset, sizeof(image_meta));
		if (find_image_meta->image_id == selected_image_id){
			found = 1;
			break;
		}
		find_image_meta_offset = find_image_meta->next_image_offset;
	}
	if (found){
		int comment_offset = find_image_meta->first_comment_offset;
		if (comment_offset == -1){
			printf("\nNo Comments\n");
			return;
		}
		while (comment_offset != -1){
			read_comment = (comment *)read_memory(fp, comment_offset, sizeof(comment));
			if (read_comment->is_valid){
				if (read_comment->comment_id == selected_comment_id){
					read_comment->is_valid = 0;
					write_memory(fp, read_comment, comment_offset, sizeof(comment));
				}
			}
			comment_offset = read_comment->next_comment;
		}
	}
	printf("\nComment deleted Succesfully!\n");
}