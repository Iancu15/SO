/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <Windows.h>

#define DLL_EXPORTS
#include "loader.h"
#include "exec_parser.h"

#define page_size 65536

static so_exec_t *exec;
static HANDLE exec_file;
static HANDLE log_file;

#define PROT_NONE 0
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4

/* traduce o masca a permisiunilor folosita in linux cu masca echivalenta */
/* din windows */
static DWORD translate_perm(unsigned int linux_perm)
{
	switch (linux_perm) {
	case PROT_NONE:
		return PAGE_NOACCESS;

	case PROT_READ:
		return PAGE_READONLY;

	case PROT_WRITE:
		return PAGE_READWRITE;

	case PROT_WRITE | PROT_READ:
		return PAGE_READWRITE;

	case PROT_EXEC:
		return PAGE_EXECUTE;

	case PROT_EXEC | PROT_READ:
		return PAGE_EXECUTE_READ;

	default:
		return PAGE_EXECUTE_READWRITE;
	}
}

static LONG CALLBACK loader_handler(PEXCEPTION_POINTERS ExceptionInfo)
{
	unsigned int page_number_in_segment;
	so_seg_t segment;
	int *is_page_mapped;
	uintptr_t address, segment_address;
	uintptr_t offset_inside_segment, page_address;
	LPVOID valloc_return;
	unsigned int bytes_until_eof, bytes_to_read, bytes_read;
	unsigned int page_offset;
	char *buffer;
	int i;
	DWORD exception_code;
	LONG offset;
	DWORD r;
	BOOL bRet;
	DWORD segment_permissions;
	DWORD old_perm;

	/* ne intereseaza doar exceptiile cauzate de accese invalide */
	/* la memorie */
	exception_code = ExceptionInfo->ExceptionRecord->ExceptionCode;
	if (exception_code != EXCEPTION_ACCESS_VIOLATION)
		return EXCEPTION_CONTINUE_SEARCH;

	address = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
	for (i = 0; i < exec->segments_no; i++) {
		segment = exec->segments[i];
		segment_address = segment.vaddr;
		if (segment_address > address)
			continue;

		offset_inside_segment = address - segment_address;
		if (offset_inside_segment >= segment.mem_size)
			continue;

		page_number_in_segment = offset_inside_segment / page_size;
		is_page_mapped = segment.data;
		if (is_page_mapped[page_number_in_segment])
			return EXCEPTION_CONTINUE_SEARCH;

		/* cand specific adresa si folosesc MEM_COMMIT trebuie */
		/* sa folosesc si MEM_RESERVE altfel da fail */
		/* mapez pagina cu PAGE_READWRITE pentru a scrie datele */
		/* din executabil in ea */
		page_offset = page_number_in_segment * page_size;
		page_address = segment_address + page_offset;
		valloc_return = VirtualAlloc(
			(LPVOID) page_address,
			page_size,
			MEM_COMMIT | MEM_RESERVE,
			PAGE_READWRITE
		);

		if (!valloc_return)
			return EXCEPTION_CONTINUE_SEARCH;

		if (offset_inside_segment < segment.file_size) {
			buffer = malloc(sizeof(char) * page_size);

			/* daca e ultima pagina din file_size voi citii maxim */
			/* page_size din executabil */
			/* restul de pagini de pana in file_size sunt pline */
			bytes_until_eof = segment.file_size - page_offset;
			bytes_to_read = page_size;
			if (bytes_until_eof < page_size)
				bytes_to_read = bytes_until_eof;

			/* mut cursor-ul la offset-ul din fisier al paginii */
			offset = segment.offset + page_offset;
			r = SetFilePointer(
				exec_file,
				offset,
				NULL,
				FILE_BEGIN
			);

			if (r == INVALID_SET_FILE_POINTER)
				return EXCEPTION_CONTINUE_SEARCH;

			/* citesc datele din executabil */
			/* al patrulea camp trebuie sa fie non-NULL ca sa */
			/* nu dea fail */
			bRet = ReadFile(
				exec_file,
				buffer,
				bytes_to_read,
				&bytes_read,
				NULL
			);

			if (bRet == FALSE)
				return EXCEPTION_CONTINUE_SEARCH;

			/* copiez in pagina datele din executabil */
			memcpy((void *) page_address, buffer, bytes_to_read);
			free(buffer);
		}

		/* actualizez permisiunile paginii la cele adevarate */
		/* care le voi afla traducand permisiunile segmentului */
		/* de pe linux in cele de pe windows */
		segment_permissions = translate_perm(segment.perm);
		bRet = VirtualProtect(
			(LPVOID) page_address,
			page_size,
			segment_permissions,
			&old_perm
		);

		if (bRet == FALSE)
			return EXCEPTION_CONTINUE_SEARCH;

		is_page_mapped[page_number_in_segment] = 1;
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

int so_init_loader(void)
{
	PVOID r;

	/* initializez loader-ul printr-un vector de exceptii */
	/* primul camp l-am pus 1 ca e o valoarea diferita de 0 si */
	/* handler-ul va fi rulat primul */
	r = AddVectoredExceptionHandler(
		1,
		loader_handler
	);

	if (!r)
		return -1;

	return 0;
}

int so_execute(char *path, char *argv[])
{
	int i;
	unsigned int number_of_pages;
	so_seg_t *segment;
	BOOL r;

	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	/* aloc campul data pentru fiecare segment */
	for (i = 0; i < exec->segments_no; i++) {
		segment = &(exec->segments[i]);
		number_of_pages = segment->mem_size / page_size + 1;
		segment->data = malloc(number_of_pages * sizeof(int));
		memset(segment->data, 0, number_of_pages * sizeof(int));
	}

	exec_file = CreateFile(
		path,
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (exec_file == INVALID_HANDLE_VALUE)
		return -1;

	so_start_exec(exec, argv);

	r = CloseHandle(exec_file);
	if (r == FALSE)
		return -1;

	return -1;
}
