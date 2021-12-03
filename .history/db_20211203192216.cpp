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
MetaCommandResult do_meta_command(InputBuffer *input_buffer) {
	if (strcmp(input_buffer->buffer, ".exit") == 0) {
		exit (EXIT_SUCCESS);
	} else {
		return META_COMMAND_UNRECOGNIZED_COMMAND;
	}
}

// sql语句 ************************************************************

// Row ********************************************
// 从table中查询一个row的位置
// in: Table, row_num
// out: page_offset
void* row_slot(Table *table, uint32_t row_num) {
	uint32_t page_num = row_num / ROWS_PER_PAGE;
	void *page = table->pages[page_num];
	if (page == NULL) {
		// Allocate memory only when we try to access page
		page = table->pages[page_num] = malloc(PAGE_SIZE);
	}
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	return page + byte_offset;
}

void print_row(Row* row) {
  printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

Table* new_table() {
	Table *table = (Table *)malloc(sizeof(Table));
	table->num_rows = 0;
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		table->pages[i] = NULL;
	}
	return table;
}

void free_table(Table *table) {
	for (int i = 0; table->pages[i]; i++) {
		free(table->pages[i]);
	}
	free(table);
}



// 将sql字符串转换为内部表示
PrepareResult prepare_statement(InputBuffer *input_buffer,
		Statement *statement) {
	if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
		statement->type = STATEMENT_INSERT;
		int args_assigned = sscanf(input_buffer->buffer, "insert %d %s %s",
				&(statement->row_to_insert.id),
				statement->row_to_insert.username,
				statement->row_to_insert.email);
		if (args_assigned < 3) {
			return PREPARE_SYNTAX_ERROR;
		}
		return PREPARE_SUCCESS;
	}
	if (strcmp(input_buffer->buffer, "select") == 0) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement *statement, Table *table) {
	if (table->num_rows >= TABLE_MAX_ROWS) {
		return EXECUTE_TABLE_FULL;
	}

	Row *row_to_insert = &(statement->row_to_insert);

	serialize_row(row_to_insert, row_slot(table, table->num_rows));
	table->num_rows += 1;

	return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement *statement, Table *table) {
	Row row;
	for (uint32_t i = 0; i < table->num_rows; i++) {
		deserialize_row(row_slot(table, i), &row);
		print_row(&row);
	}
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
	memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
	memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
	memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
	memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
	memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
	memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

int main(int argc, char *argv[]) {
	InputBuffer *input_buffer = new_input_buffer();
	Table *table = new_table();

	while (true) {
		// 打印
		print_prompt();
		// 读输入
		read_input(input_buffer);

		// 执行输入
		// 元命令, 类似.exit
		if (input_buffer->buffer[0] == '.') {
			switch (do_meta_command(input_buffer)) {
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
