#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "db.h"

// InputBuffer 输入输出 *******************************************
// 作为一个小的包装来和 getline() 进行交互

InputBuffer* new_input_buffer() {
	InputBuffer *input_buffer = (InputBuffer*) malloc(sizeof(InputBuffer));
	input_buffer->buffer = NULL;
	input_buffer->buffer_length = 0;
	input_buffer->input_length = 0;

	return input_buffer;
}
void close_input_buffer(InputBuffer *input_buffer) {
	free(input_buffer->buffer);
	free(input_buffer);
}

// 打印一个提示符给用户
void print_prompt() {
	printf("db > ");
}

void print_constants() {
	printf("ROW_SIZE: %d\n", ROW_SIZE);
	printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
	printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
	printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
	printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
	printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}
void read_input(InputBuffer *input_buffer) {
	ssize_t bytes_read = getline(&(input_buffer->buffer),
			&(input_buffer->buffer_length), stdin);

	if (bytes_read <= 0) {
		printf("Error reading input\n");
		exit (EXIT_FAILURE);
	}

	// Ignore trailing newline
	input_buffer->input_length = bytes_read - 1;
	input_buffer->buffer[bytes_read - 1] = 0;
}
// 元命令, 类似.exit *****************************************
MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table) {
	if (strcmp(input_buffer->buffer, ".exit") == 0) {
		db_close(table);
		exit (EXIT_SUCCESS);
	} else if (strcmp(input_buffer->buffer, ".constants") == 0) {
		printf("Constants:\n");
		print_constants();
		return META_COMMAND_SUCCESS;
	} else {
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

// sql语句 ************************************************************

// Row ********************************************

void print_row(Row *row) {
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
// Pager *****************************
//打开文件, 并将pages置为空
Pager* pager_open(const char *filename) {
	int fd = open(filename, O_RDWR |      // Read/Write mode
			O_CREAT,  // Create file if it does not exist
	S_IWUSR |     // User write permission
			S_IRUSR   // User read permission
			);

	if (fd == -1) {
		printf("Unable to open file\n");
		exit (EXIT_FAILURE);
	}

	off_t file_length = lseek(fd, 0, SEEK_END);

	Pager *pager = (Pager*) malloc(sizeof(Pager));
	pager->file_descriptor = fd;
	pager->file_length = file_length;
	pager->num_pages = (file_length / PAGE_SIZE);
	// 文件长度是页长度的整数倍
	if (file_length % PAGE_SIZE != 0) {
		printf("Db file is not a whole number of pages. Corrupt file.\n");
		exit (EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		pager->pages[i] = NULL;
	}

	return pager;
}

// 获取第page_num个page
void* get_page(Pager *pager, uint32_t page_num) {
	if (page_num > TABLE_MAX_PAGES) {
		printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
		TABLE_MAX_PAGES);
		exit (EXIT_FAILURE);
	}

	if (pager->pages[page_num] == NULL) {
		// 第page_num个page还未创建
		// Cache miss. Allocate memory and load from file.
		void *page = malloc(PAGE_SIZE);
		uint32_t num_pages = pager->file_length / PAGE_SIZE;

		// We might save a partial page at the end of the file
		if (pager->file_length % PAGE_SIZE) {
			num_pages += 1;
		}
		// num_pages为总的需要的page数

		if (page_num <= num_pages) {
			// 将该页读入page中
			lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
			ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
			if (bytes_read == -1) {
				printf("Error reading file: %d\n", errno);
				exit (EXIT_FAILURE);
			}
		}

		pager->pages[page_num] = page;

		// 应该是插入了新内容, 超出了原有的长度
		if (page_num >= pager->num_pages) {
			pager->num_pages = page_num + 1;
		}
	}

	return pager->pages[page_num];
}
// 将pager的第page_num页中size大小, 刷入磁盘
void pager_flush(Pager *pager, uint32_t page_num) {
	if (pager->pages[page_num] == NULL) {
		printf("Tried to flush null page\n");
		exit (EXIT_FAILURE);
	}

	off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE,
			SEEK_SET);

	if (offset == -1) {
		printf("Error seeking: %d\n", errno);
		exit (EXIT_FAILURE);
	}

	ssize_t bytes_written = write(pager->file_descriptor,
			pager->pages[page_num], PAGE_SIZE);

	if (bytes_written == -1) {
		printf("Error writing: %d\n", errno);
		exit (EXIT_FAILURE);
	}
}

// Table *********************************************
Table* db_open(const char *filename) {
	Pager *pager = pager_open(filename);

	Table *table = (Table*) malloc(sizeof(Table));
	table->pager = pager;
	table->root_page_num = 0;

	if (pager->num_pages == 0) {
		//全新的数据库 初始化一个叶节点
		// New database file. Initialize page 0 as leaf node.
		void *root_node = get_page(pager, 0);
		initialize_leaf_node(root_node);
	}
	return table;
}

// 关闭db时, 将内存中数据写入磁盘
void db_close(Table *table) {
	Pager *pager = table->pager;

	for (uint32_t i = 0; i < pager->num_pages; i++) {
		if (pager->pages[i] == NULL) {
			continue;
		}
		pager_flush(pager, i);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	int result = close(pager->file_descriptor);
	if (result == -1) {
		printf("Error closing db file.\n");
		exit (EXIT_FAILURE);
	}
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		void *page = pager->pages[i];
		if (page) {
			free(page);
			pager->pages[i] = NULL;
		}
	}
	free(pager);
	free(table);
}

void free_table(Table *table) {
	for (int i = 0; table->pager->pages[i]; i++) {
		free(table->pager->pages[i]);
	}
	free(table);
}

// 创建curosr指向table头
Cursor* table_start(Table *table) {
	Cursor *cursor = (Cursor*) malloc(sizeof(Cursor));
	cursor->table = table;
	cursor->page_num = table->root_page_num;
	cursor->cell_num = 0;

	void *root_node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = *leaf_node_num_cells(root_node);
	cursor->end_of_table = (num_cells == 0);

	return cursor;
}

/*
 Return the position of the given key.
 If the key is not present, return the position
 where it should be inserted
 */
// 根据\key, 查找在内存中的位置
Cursor* table_find(Table *table, uint32_t key) {
	uint32_t root_page_num = table->root_page_num;
	void *root_node = get_page(table->pager, root_page_num);

	if (get_node_type(root_node) == NODE_LEAF) {
		return leaf_node_find(table, root_page_num, key);
	} else {
		printf("Need to implement searching an internal node\n");
		exit (EXIT_FAILURE);
	}
}

// Cursor *****************************

// 获取cursor指向的row的地址
// in: cursor
// out: page_offset
void* cursor_value(Cursor *cursor) {
	uint32_t page_num = cursor->page_num;
	void *page = get_page(cursor->table->pager, page_num);

	return (void*) leaf_node_value(page, cursor->cell_num);
}

// cursor向后移动一行
void cursor_advance(Cursor *cursor) {
	uint32_t page_num = cursor->page_num;
	void *node = get_page(cursor->table->pager, page_num);
	cursor->cell_num += 1;
	if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
		cursor->end_of_table = true;
	}
}
// Statement *************************************
// 将sql字符串转换为内部表示
PrepareResult prepare_insert(InputBuffer *input_buffer, Statement *statement) {
	statement->type = STATEMENT_INSERT;

	char *keyword = strtok(input_buffer->buffer, " ");
	char *id_string = strtok(NULL, " ");
	char *username = strtok(NULL, " ");
	char *email = strtok(NULL, " ");

	if (id_string == NULL || username == NULL || email == NULL) {
		return PREPARE_SYNTAX_ERROR;
	}

	int id = atoi(id_string);
	if (id < 0) {
		return PREPARE_NEGATIVE_ID;
	}
	if (strlen(username) > COLUMN_USERNAME_SIZE) {
		return PREPARE_STRING_TOO_LONG;
	}
	if (strlen(email) > COLUMN_EMAIL_SIZE) {
		return PREPARE_STRING_TOO_LONG;
	}

	statement->row_to_insert.id = id;
	strcpy(statement->row_to_insert.username, username);
	strcpy(statement->row_to_insert.email, email);

	return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer *input_buffer,
		Statement *statement) {
	if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
		return prepare_insert(input_buffer, statement);

	}
	if (strcmp(input_buffer->buffer, "select") == 0) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement *statement, Table *table) {
	void *node = get_page(table->pager, table->root_page_num);
	uint32_t num_cells = (*leaf_node_num_cells(node));
	if (num_cells >= LEAF_NODE_MAX_CELLS) {
		return EXECUTE_TABLE_FULL;
	}

	Row *row_to_insert = &(statement->row_to_insert);
	uint32_t key_to_insert = row_to_insert->id;
	Cursor *cursor = table_find(table, key_to_insert);
	if (cursor->cell_num < num_cells) {
		uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
		if (key_at_index == key_to_insert) {
			return EXECUTE_DUPLICATE_KEY;
		}
	}

	leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

	free(cursor);

	return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table) {
	Cursor *cursor = table_start(table);

	Row row;
	while (!(cursor->end_of_table)) {
		deserialize_row(cursor_value(cursor), &row);
		print_row(&row);
		cursor_advance(cursor);
	}

	free(cursor);
	return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table) {
	switch (statement->type) {
	case (STATEMENT_INSERT):
		return execute_insert(statement, table);
	case (STATEMENT_SELECT):
		return execute_select(statement, table);
	}
}

// 序列化
void serialize_row(Row *source, void *destination) {
	memcpy((char*) destination + ID_OFFSET, &(source->id), ID_SIZE);
	memcpy((char*) destination + USERNAME_OFFSET, &(source->username),
			USERNAME_SIZE);
	memcpy((char*) destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
	memcpy(&(destination->id), (char*) source + ID_OFFSET, ID_SIZE);
	memcpy(&(destination->username), (char*) source + USERNAME_OFFSET,
			USERNAME_SIZE);
	memcpy(&(destination->email), (char*) source + EMAIL_OFFSET, EMAIL_SIZE);
}

// B+Tree *********************************
uint32_t* leaf_node_num_cells(void *node) {
	return (uint32_t*) node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void* leaf_node_cell(void *node, uint32_t cell_num) {
	return (uint32_t*) node + LEAF_NODE_HEADER_SIZE
			+ cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* leaf_node_key(void *node, uint32_t cell_num) {
	return (uint32_t*) leaf_node_cell(node, cell_num);
}

void* leaf_node_value(void *node, uint32_t cell_num) {
	return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void initialize_leaf_node(void *node) {
	*leaf_node_num_cells(node) = 0;
}

void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value) {
	//	获取Cursor指向内存中的页面, Page
	void *node = get_page(cursor->table->pager, cursor->page_num);

	uint32_t num_cells = *leaf_node_num_cells(node);
	if (num_cells >= LEAF_NODE_MAX_CELLS) {
		// Node full
		printf("Need to implement splitting a leaf node.\n");
		exit (EXIT_FAILURE);
	}

	if (cursor->cell_num < num_cells) {
		// Make room for new cell
		for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
			memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
					LEAF_NODE_CELL_SIZE);
		}
	}

	*(leaf_node_num_cells(node)) += 1;
	*(leaf_node_key(node, cursor->cell_num)) = key;
	serialize_row(value, leaf_node_value(node, cursor->cell_num));
}

Cursor* leaf_node_find(Table *table, uint32_t page_num, uint32_t key) {
	void *node = get_page(table->pager, page_num);
	uint32_t num_cells = *leaf_node_num_cells(node);

	Cursor *cursor = malloc(sizeof(Cursor));
	cursor->table = table;
	cursor->page_num = page_num;

	// Binary search
	uint32_t min_index = 0;
	uint32_t one_past_max_index = num_cells;
	while (one_past_max_index != min_index) {
		uint32_t index = (min_index + one_past_max_index) / 2;
		uint32_t key_at_index = *leaf_node_key(node, index);
		if (key == key_at_index) {
			cursor->cell_num = index;
			return cursor;
		}
		if (key < key_at_index) {
			one_past_max_index = index;
		} else {
			min_index = index + 1;
		}
	}

	cursor->cell_num = min_index;
	return cursor;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Must supply a database filename.\n");
		exit (EXIT_FAILURE);
	}

	char *filename = argv[1];
	Table *table = db_open(filename);
	InputBuffer *input_buffer = new_input_buffer();

	while (true) {
		// 打印
		print_prompt();
		// 读输入
		read_input(input_buffer);

		// 执行输入
		// 元命令, 类似.exit
		if (input_buffer->buffer[0] == '.') {
			switch (do_meta_command(input_buffer, table)) {
			case (META_COMMAND_SUCCESS):
				continue;
			case (META_COMMAND_UNRECOGNIZED_COMMAND):
				printf("Unrecognized command '%s'\n", input_buffer->buffer);
				continue;
			}
		}

		// sql语句转换为内部结构
		Statement statement;
		switch (prepare_statement(input_buffer, &statement)) {
		case (PREPARE_SUCCESS):
			break;
		case (PREPARE_SYNTAX_ERROR):
			printf("Syntax error. Could not parse statement.\n");
			continue;
		case (PREPARE_UNRECOGNIZED_STATEMENT):
			printf("Unrecognized keyword at start of '%s'.\n",
					input_buffer->buffer);
			continue;
		case (PREPARE_STRING_TOO_LONG):
			printf("String is too long.\n");
			continue;
		case (PREPARE_NEGATIVE_ID):
			printf("ID must be positive.\n");
			continue;
		}

		//执行sql语句
		switch (execute_statement(&statement, table)) {
		case (EXECUTE_SUCCESS):
			printf("Executed.\n");
			break;
		case (EXECUTE_TABLE_FULL):
			printf("Error: Table full.\n");
			break;
		}
	}
}
