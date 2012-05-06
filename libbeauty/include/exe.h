
#ifndef __EXE__
#define __EXE__

/* Memory and Registers are a list of accessed stores. */
/* A record is only valid when it has been accessed. */
/* Initially the search algorithm will be slow,
 * but if the method works, fast algorithms will be used. */

struct memory_s {
	/* Start address of multibyte access. */
	uint64_t start_address;
	/* Number of bytes accessed at one time */
	int length;
	/* 0 - Unknown, 1 - Known */
	int init_value_type;
	/* Initial value when first accessed */
	uint64_t init_value;
	/* init_value + offset_value = absolute value to be used */
	uint64_t offset_value;
	/* Indirect value */
	uint64_t indirect_init_value;
	/* Indirect offset */
	uint64_t indirect_offset_value;
	/* Special case of a value_id before dereferencing */
	uint64_t indirect_value_id;
	/* 0 - unknown,
	 * 1 - unsigned,
	 * 2 - signed,
	 * 3 - pointer,
	 * 4 - Instruction,
	 * 5 - Instruction pointer(EIP),
	 * 6 - Stack pointer.
	 */
	int	value_type;
	/* Moving to: */
	/* 0 - Unlikely
	 * 1 or above - more likely
	 */
	int	value_unsigned;
	int	value_signed;
	int	value_instruction;
	int	value_pointer;
	int	value_normal;
	/* Index into the various structure tables */
	int	value_struct;
	/* last_accessed_from_instruction_at_memory_location */
	uint32_t ref_memory;
	/* last_accessed_from_instruction_log_at_location */
	uint32_t ref_log;
	/* value_scope: 0 - unknown, 1 - Param, 2 - Local, 3 - Global */
	int value_scope;
	/* Each time a new value is assigned, this value_id increases */
	uint64_t value_id;
	/* valid: 0 - Entry Not used yet, 1 - Entry Used */
	uint64_t valid;
	/* The instruction that assigned the value within SSA scope */
	/* If size > 1 there is more than one path between there and here */
	int prev_size;
	int *prev;
	/* The instruction that uses the value within SSA scope */
	/* If size > 1 there is more than one path between there and here */
	int next_size;
	int *next;
} ;

struct entry_point_s {
	int used;
	/* FIXME: Is this enough, or will full register backup be required */
	uint64_t esp_init_value;
	uint64_t esp_offset_value;
	uint64_t ebp_init_value;
	uint64_t ebp_offset_value;
	uint64_t eip_init_value;
	uint64_t eip_offset_value;
	uint64_t previous_instuction;
} ;

struct inst_log_entry_s {
	struct instruction_s instruction;	/* The instruction */
	int prev_size;
	int *prev;
	int next_size;
	int *next;
	struct memory_s value1;		/* First input value */
	struct memory_s value2;		/* Second input value */
	struct memory_s value3;		/* Result */
	void *extension;		/* Instruction specific extention */
} ;

/* redirect is used for SSA correction, when one needs to rename a variable */
/* renaming the variable within the log entries would take too long. */
/* so use log entry value_id -> redirect -> label_s */
struct label_redirect_s {
	uint64_t redirect;
} ;

struct label_s {
	/* local = 1, param = 2, data = 3, mem = 4, sp_bp = 5 */
	uint64_t scope;
	/* For local or param: reg = 1, stack = 2 */
	/* For data: data = 1, &data = 2, value = 3 */
	uint64_t type;
	/* value */
	uint64_t value;
	/* size in bits */
	uint64_t size_bits;
	/* is it a pointer */
	uint64_t lab_pointer;
	/* is it a signed */
	uint64_t lab_signed;
	/* is it a unsigned */
	uint64_t lab_unsigned;
	/* human readable name */
	char *name;
} ;

extern struct memory_s *search_store(
        struct memory_s *memory, uint64_t index, int size);
extern struct memory_s *add_new_store(
	struct memory_s *memory, uint64_t index, int size);

//extern instructions_t instructions;
extern uint8_t *inst;
extern struct rev_eng *handle;
extern struct disassemble_info disasm_info;
extern char *dis_flags_table[];
extern uint64_t inst_log;      /* Pointer to the current free instruction log entry. */
extern char out_buf[1024];
extern int local_counter;
extern size_t inst_size;

#endif /* __EXE__ */
