int rmb(struct rev_eng *handle, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset, uint64_t size, uint8_t rex, uint8_t *return_reg);
int prefix_0f(struct rev_eng *handle, struct dis_instructions_s *dis_instructions, uint8_t *base_address, uint64_t offset, uint64_t size, uint8_t rex);
