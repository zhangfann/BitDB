#ifndef DB_H
#define DB_H
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
 #include <errno.h>

// InputBuffer 输入输出 *******************************************
// 作为一个小的包装来和 getline() 进行交互
typedef struct {
	char *buffer;
	size_t buffer_length;
	ssize_t input_length;
} InputBuffer;
InputBuffer* new_input_buffer();
void close_input_buffer(InputBuffer *input_buffer);
void print_prompt(); // 打印提示信息
void read_input(InputBuffer *input_buffer); //读取输入

// 元命令, 类似.exit *****************************************
typedef enum {
	META_COMMAND_SUCCESS, META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;
MetaCommandResult do_meta_command(InputBuffer *input_buffer); //执行元命令

// Row
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
	uint32_t id;
	char username[COLUMN_USERNAME_SIZE+1]; // +1:增加一个结束符
	char email[COLUMN_EMAIL_SIZE+1];
} Row;
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
void print_row(Row *row); // 打印行
// 序列化
void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);

// Pager 管理磁盘中的一个文件 *******************************
#define TABLE_MAX_PAGES 100
typedef struct {
  int file_descriptor;
  uint32_t file_length;
  void* pages[TABLE_MAX_PAGES];
} Pager;
Pager* pager_open(const char* filename);
void* get_page(Pager *pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);

// Table 管理内存中的一个页 *************************
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct {
	uint32_t num_rows;
	Pager* pager;
} Table;
Table* db_open();
void db_close(Table *table);
void free_table(Table *table);
void* row_slot(Table *table, uint32_t row_num); // 根据row_num(行数)查找row


// sql Statement **************
typedef enum {
	EXECUTE_SUCCESS, EXECUTE_TABLE_FULL
} ExecuteResult;
typedef enum {
	PREPARE_SUCCESS,
	PREPARE_SYNTAX_ERROR, // 语法错误
	PREPARE_UNRECOGNIZED_STATEMENT, // 未识别的语句
	PREPARE_STRING_TOO_LONG, // 输入文本参数过长
	PREPARE_NEGATIVE_ID, // id不能是负数
} PrepareResult;

typedef enum {
	STATEMENT_INSERT, STATEMENT_SELECT
} StatementType;
typedef struct {
	StatementType type; // sql语句类型
	Row row_to_insert; //待插入的行数据, only used by insert statement
} Statement;

PrepareResult prepare_statement(InputBuffer *input_buffer,
		Statement *statement); // 将sql字符串转换为内部表示
ExecuteResult execute_insert(Statement *statement, Table *table);
ExecuteResult execute_select(Statement *statement, Table *table);
ExecuteResult execute_statement(Statement *statement, Table *table);

#endif
