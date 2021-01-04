#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓰거나 삭제 레코드를 수정할 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉 페이지 단위로 읽거나 써야 합니다.

typedef struct _Header
{
	int total_pagenum;			//전체 page 수
	int total_recordnum;		//전체 record 수
	int last_delete_pagenum;	//가장 최근에 삭제된 page 번호
	int last_delete_recordnum;	//가장 최근에 삭제돈 record 번호
	char dummy[PAGE_SIZE - 16];
} Header;

typedef struct _Delete_record
{
	char delimiter;		//delete mark
	int RPN;			//가장 최근에 삭제된 page 번호
	int RRN;			//가장 최근에 삭제된 record 번호
} Delete_record;


//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	fseek(fp, pagenum*PAGE_SIZE, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}


//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, pagenum*PAGE_SIZE, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}


//
// 새로운 레코드를 저장할 때 터미널로부터 입력받은 정보를 Person 구조체에 먼저 저장하고, pack() 함수를 사용하여
// 레코드 파일에 저장할 레코드 형태를 recordbuf에 만든다. 그런 후 이 레코드를 저장할 페이지를 readPage()를 통해 프로그램 상에
// 읽어 온 후 pagebuf에 recordbuf에 저장되어 있는 레코드를 저장한다. 그 다음 writePage() 호출하여 pagebuf를 해당 페이지 번호에
// 저장한다. pack() 함수에서 readPage()와 writePage()를 호출하는 것이 아니라 pack()을 호출하는 측에서 pack() 함수 호출 후
// readPage()와 writePage()를 차례로 호출하여 레코드 쓰기를 완성한다는 의미이다.
// 
void pack(char *recordbuf, const Person *p)
{
	memset(recordbuf, (char)0xFF, RECORD_SIZE);
	strcpy(recordbuf, p->sn);
	strcat(recordbuf, "#");

	strcat(recordbuf, p->name);
	strcat(recordbuf, "#");

	strcat(recordbuf, p->age);
	strcat(recordbuf, "#");

	strcat(recordbuf, p->addr);
	strcat(recordbuf, "#");

	strcat(recordbuf, p->phone);
	strcat(recordbuf, "#");

	strcat(recordbuf, p->email);
	strcat(recordbuf, "#");

}


// 
// 아래의 unpack() 함수는 recordbuf에 저장되어 있는 레코드를 구조체로 변환할 때 사용한다. 이 함수가 언제 호출되는지는
// 위에서 설명한 pack()의 시나리오를 참조하면 된다.
//
void unpack(const char *recordbuf, Person *p)
{
	char cpy_recordbuf[RECORD_SIZE];
	char *ptr;

	strcpy(cpy_recordbuf, recordbuf);
	
    ptr = strtok(cpy_recordbuf, "#");
	strcpy(p->sn, ptr);

	strtok(NULL, "#");
	strcpy(p->name, ptr);

	strtok(NULL, "#");
	strcpy(p->age, ptr);
	
	strtok(NULL, "#");
	strcpy(p->addr, ptr);

	strtok(NULL, "#");
	strcpy(p->phone, ptr);

	strtok(NULL, "#");
	strcpy(p->email, ptr);
}


//
// 새로운 레코드를 저장하는 기능을 수행하며, 터미널로부터 입력받은 필드값을 구조체에 저장한 후 아래의 insert() 함수를 호출한다.
//
void insert(FILE *fp, const Person *p)
{
	char pagebuf[PAGE_SIZE];
	char recordbuf[RECORD_SIZE];
	Header header;
	Delete_record delete_record;
	
	pack(recordbuf, p);

	readPage(fp, (char*)&header, 0);


	/* 삭제된 레코드 공간이 없는 경우 */
	if(header.last_delete_pagenum == -1 && header.last_delete_recordnum == -1) {
		int already_insert = 0;

		for(int i=1; i<header.total_pagenum; i++) {
			int cnt=0;
			int recordnum;
			int total_recordnum;

			total_recordnum = PAGE_SIZE/RECORD_SIZE;
			recordnum = total_recordnum - (((header.total_pagenum-1)*total_recordnum)-header.total_recordnum);

			readPage(fp, pagebuf, i);
			
			for(int j=0; j<PAGE_SIZE; j++) {
				if(pagebuf[j] == '#')
					cnt++;
			}
			
			/* 기존의 page에 빈공간이 있는 경우 */
			if(cnt != 6*total_recordnum) {
				memcpy(pagebuf + RECORD_SIZE*recordnum, recordbuf, RECORD_SIZE);
				writePage(fp, pagebuf, i);

				header.total_recordnum++;
				already_insert = 1;
				break;
			}
		}


		/* 새로운 page를 추가해야하는 경우 */
		if(already_insert == 0) {
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			memcpy(pagebuf, recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, header.total_pagenum);

			header.total_pagenum++;
			header.total_recordnum++;
		}
	}


	/* 삭제된 레코드 공간이 있는 경우 */
	else {
		readPage(fp, pagebuf, header.last_delete_pagenum);
		memcpy(&delete_record, pagebuf + RECORD_SIZE*header.last_delete_recordnum, sizeof(Delete_record));
		memcpy(pagebuf + RECORD_SIZE*header.last_delete_recordnum, recordbuf, RECORD_SIZE);
		writePage(fp, pagebuf, header.last_delete_pagenum);

		header.last_delete_pagenum = delete_record.RPN;
		header.last_delete_recordnum = delete_record.RRN;
	}

	writePage(fp, (char*)&header, 0);
}


//
// 주민번호와 일치하는 레코드를 찾아서 삭제하는 기능을 수행한다.
//
void delete(FILE *fp, const char *sn)
{
	char pagebuf[PAGE_SIZE];
	char recordbuf[RECORD_SIZE];
	Header header;
	Person p;
	Delete_record delete_record;

	readPage(fp, (char*)&header, 0);
	
	for(int i=1; i<header.total_pagenum; i++) {
		int recordnum;
		int total_recordnum;
		int already_delete = 0;

		total_recordnum = PAGE_SIZE/RECORD_SIZE;
		recordnum = total_recordnum - (((header.total_pagenum-1)*total_recordnum)-header.total_recordnum);

		readPage(fp, pagebuf, i);
		
		for(int j=0; j<total_recordnum; j++) {
			memcpy(recordbuf, pagebuf + RECORD_SIZE*j, RECORD_SIZE);
			
			unpack(recordbuf, &p);

			if(strcmp(sn, p.sn) == 0) {
				delete_record.delimiter = '*';
				delete_record.RPN = header.last_delete_pagenum;
				delete_record.RRN = header.last_delete_recordnum;

				memcpy(pagebuf + RECORD_SIZE*j, &delete_record, sizeof(Delete_record));
				writePage(fp, pagebuf, i);

				header.last_delete_pagenum = i;
				header.last_delete_recordnum = j;
				already_delete = 1;
				writePage(fp, (char*)&header, 0);
				break;
			}
		}

		if(already_delete == 1) 
			break;
	}
}


int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	char option = argv[1][0];
	char *filename = argv[2];
	Header header;
	Person p;
	
	if(access(argv[2], F_OK) != 0) { //파일이 존재하지 않는 경우
		if ((fp = fopen(filename, "w+")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", filename);
			exit(1);
		}

		header.total_pagenum = 1;
		header.total_recordnum = 0;
		header.last_delete_pagenum = -1;
		header.last_delete_recordnum = -1;
		memset(header.dummy, (char)0xFF, PAGE_SIZE - 16);

		writePage(fp, (char*)&header, 0);
		
		fclose(fp);
	}

	if ((fp = fopen(filename, "r+")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", filename);
			exit(1);
		}

	if(option == 'i') {
		strcpy(p.sn, argv[3]);
		strcpy(p.name, argv[4]);
		strcpy(p.age, argv[5]);
		strcpy(p.addr, argv[6]);
		strcpy(p.phone, argv[7]);
		strcpy(p.email, argv[8]);

		insert(fp, &p);
	}

	if(option == 'd')
		delete(fp, argv[3]);


	fclose(fp);

	return 1;
}