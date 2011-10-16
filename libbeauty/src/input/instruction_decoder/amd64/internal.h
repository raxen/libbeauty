int rmb(struct rev_eng *handle, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset, uint64_t size, uint8_t rex, uint8_t *return_reg, int *half);
int prefix_0f(struct rev_eng *handle, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset, uint64_t size, uint8_t rex);

int dis_Gx_Ex(struct rev_eng *handle, int opcode, uint8_t rex, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset, uint8_t *reg, int size);
int dis_Ex_Gx(struct rev_eng *handle, int opcode, uint8_t rex, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset, uint8_t *reg, int size);
