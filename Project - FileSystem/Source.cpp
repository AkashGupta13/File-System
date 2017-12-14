#include "functions.h"



int main(){
	int choice = 0,offset=0;
	FILE *fp = fopen("space", "rb+");
	if (fp == NULL){
		system("fsutil file createnew space 104857600");
		fp = fopen("space", "rb+");
	}
	header *head = (header *)read_memory(fp, 0, sizeof(header));
//	printf("%d\n%d\n", head->current_image_id, head->current_comment_id);
	if (head->first_image_meta_address == 0){
		head = createHeader();
		write_memory(fp, head, 0, sizeof(header));
		offset = sizeof(header);
	}
	else{
		offset = head->next_free_offset;
	}
	while (choice != 8){
		printf("\n-------------------------------------------");
		printf("\n1.Upload Image\n2.Download Image\n3.List Images\n4.Delete Image\n");
		printf("5.Add Comment\n6.View Comments\n7.Delete Comment\n8.Exit\n");
		printf("-------------------------------------------");
		printf("\nEnter your choice - ");
		scanf("%d", &choice);
		switch (choice){
		case 1:
			uploadImage(fp, head);
			break;
		case 2:
			downloadImage(fp, head);
			break;
		case 3:listImages(fp, head);
			break;
		case 4:
			deleteImage(fp, head);
			break;
		case 5:
			addComment(fp, head);
			break;
		case 6:
			viewComments(fp, head);
			break;
		case 7:
			deleteComment(fp, head);
			break;
		case 8:
			break;
		}
	}
	fclose(fp);
}