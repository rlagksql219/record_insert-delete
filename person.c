#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "person.h"
//�ʿ��� ��� ��� ���ϰ� �Լ��� �߰��� �� ����

// ���� ������� �����ϴ� ����� ���� �ٸ� �� ������ �ణ�� ������ �Ӵϴ�.
// ���ڵ� ������ ������ ������ ���� �����Ǳ� ������ ����� ���α׷����� ���ڵ� ���Ϸκ��� �����͸� �а� �� ����
// ������ ������ ����մϴ�. ���� �Ʒ��� �� �Լ��� �ʿ��մϴ�.
// 1. readPage(): �־��� ������ ��ȣ�� ������ �����͸� ���α׷� ������ �о�ͼ� pagebuf�� �����Ѵ�
// 2. writePage(): ���α׷� ���� pagebuf�� �����͸� �־��� ������ ��ȣ�� �����Ѵ�
// ���ڵ� ���Ͽ��� ������ ���ڵ带 �аų� ���ο� ���ڵ带 ���ų� ���� ���ڵ带 ������ ����
// ��� I/O�� ���� �� �Լ��� ���� ȣ���ؾ� �մϴ�. �� ������ ������ �аų� ��� �մϴ�.

typedef struct _Header
{
	int total_pagenum;			//��ü page ��
	int total_recordnum;		//��ü record ��
	int last_delete_pagenum;	//���� �ֱٿ� ������ page ��ȣ
	int last_delete_recordnum;	//���� �ֱٿ� ������ record ��ȣ
	char dummy[PAGE_SIZE - 16];
} Header;

typedef struct _Delete_record
{
	char delimiter;		//delete mark
	int RPN;			//���� �ֱٿ� ������ page ��ȣ
	int RRN;			//���� �ֱٿ� ������ record ��ȣ
} Delete_record;


//
// ������ ��ȣ�� �ش��ϴ� �������� �־��� ������ ���ۿ� �о �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	fseek(fp, pagenum*PAGE_SIZE, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);
}


//
// ������ ������ �����͸� �־��� ������ ��ȣ�� �ش��ϴ� ��ġ�� �����Ѵ�. ������ ���۴� �ݵ�� ������ ũ��� ��ġ�ؾ� �Ѵ�.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, pagenum*PAGE_SIZE, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);
}


//
// ���ο� ���ڵ带 ������ �� �͹̳ηκ��� �Է¹��� ������ Person ����ü�� ���� �����ϰ�, pack() �Լ��� ����Ͽ�
// ���ڵ� ���Ͽ� ������ ���ڵ� ���¸� recordbuf�� �����. �׷� �� �� ���ڵ带 ������ �������� readPage()�� ���� ���α׷� ��
// �о� �� �� pagebuf�� recordbuf�� ����Ǿ� �ִ� ���ڵ带 �����Ѵ�. �� ���� writePage() ȣ���Ͽ� pagebuf�� �ش� ������ ��ȣ��
// �����Ѵ�. pack() �Լ����� readPage()�� writePage()�� ȣ���ϴ� ���� �ƴ϶� pack()�� ȣ���ϴ� ������ pack() �Լ� ȣ�� ��
// readPage()�� writePage()�� ���ʷ� ȣ���Ͽ� ���ڵ� ���⸦ �ϼ��Ѵٴ� �ǹ��̴�.
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
// �Ʒ��� unpack() �Լ��� recordbuf�� ����Ǿ� �ִ� ���ڵ带 ����ü�� ��ȯ�� �� ����Ѵ�. �� �Լ��� ���� ȣ��Ǵ�����
// ������ ������ pack()�� �ó������� �����ϸ� �ȴ�.
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
// ���ο� ���ڵ带 �����ϴ� ����� �����ϸ�, �͹̳ηκ��� �Է¹��� �ʵ尪�� ����ü�� ������ �� �Ʒ��� insert() �Լ��� ȣ���Ѵ�.
//
void insert(FILE *fp, const Person *p)
{
	char pagebuf[PAGE_SIZE];
	char recordbuf[RECORD_SIZE];
	Header header;
	Delete_record delete_record;
	
	pack(recordbuf, p);

	readPage(fp, (char*)&header, 0);


	/* ������ ���ڵ� ������ ���� ��� */
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
			
			/* ������ page�� ������� �ִ� ��� */
			if(cnt != 6*total_recordnum) {
				memcpy(pagebuf + RECORD_SIZE*recordnum, recordbuf, RECORD_SIZE);
				writePage(fp, pagebuf, i);

				header.total_recordnum++;
				already_insert = 1;
				break;
			}
		}


		/* ���ο� page�� �߰��ؾ��ϴ� ��� */
		if(already_insert == 0) {
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			memcpy(pagebuf, recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, header.total_pagenum);

			header.total_pagenum++;
			header.total_recordnum++;
		}
	}


	/* ������ ���ڵ� ������ �ִ� ��� */
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
// �ֹι�ȣ�� ��ġ�ϴ� ���ڵ带 ã�Ƽ� �����ϴ� ����� �����Ѵ�.
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
	FILE *fp;  // ���ڵ� ������ ���� ������
	char option = argv[1][0];
	char *filename = argv[2];
	Header header;
	Person p;
	
	if(access(argv[2], F_OK) != 0) { //������ �������� �ʴ� ���
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