#include "assembler.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "container/aa_tree.h"
#include "container/bb.h"
#include "container/stack.h"
#include "instructions.h"
#include "lexer.h"
#include "rust.h"
struct parser {
  enum section section;
  struct aa_tree *label_locations;
  struct list *relocations;
  struct stack *stack;
  struct bb *code;
  struct bb *data;
  struct token *lookahead;
};
struct label_location *label_location_create(const char *label, uint16_t offset,
                                             enum section section) {
  struct label_location *ll =
      (struct label_location *)malloc(sizeof(struct label_location));
  ll->label = strdup(label);
  ll->offset = offset;
  ll->section = section;
  return ll;
}
void label_location_delete(struct label_location *ll) {
  free(ll->label);
  free(ll);
}
int label_location_cmp(const struct label_location *lhs,
                       const struct label_location *rhs) {
  return strcmp(lhs->label, rhs->label);
}
struct relocation *relocation_create(enum relocation_type relocation_type,
                                     uint16_t location, uint16_t addend,
                                     enum section location_section,
                                     const char *target) {
  struct relocation *rel =
      (struct relocation *)malloc(sizeof(struct relocation));
  rel->relocation_type = relocation_type;
  rel->location = location;
  rel->addend = addend;
  rel->location_section = location_section;
  rel->target = strdup(target);
  return rel;
}
void relocation_delete(struct relocation *relocation) {
  free(relocation->target);
  free(relocation);
}
enum ast_type {
  AST_INVALID,
  AST_TOKEN,
  AST_ARITHMETIC,
  AST_BRANCH,
  AST_MEM,
  AST_OPCODE,
  AST_OPERAND_R,
  AST_OPERAND_IMM,
  AST_OPERAND_R_R,
  AST_OPERAND_R_IMM,
  AST_OPERAND_MEM,
  AST_LABEL,
  AST_DATA8,
  AST_DATA16
};
const char *ast_type_descriptions[] = {"AST_INVALID",
                                       "AST_TOKEN",
                                       "AST_ARITHMETIC",
                                       "AST_BRANCH",
                                       "AST_MEM",
                                       "AST_OPCODE",
                                       "AST_OPERAND_R",
                                       "AST_OPERAND_IMM",
                                       "AST_OPERAND_R_R",
                                       "AST_OPERAND_R_IMM",
                                       "AST_OPERAND_MEM",
                                       "AST_LABEL",
                                       "AST_DATA8"
                                       "AST_DATA16"};
struct operand_r {
  uint8_t reg;
};
struct operand_imm {
  int16_t imm;
};
struct operand_r_imm {
  uint8_t reg;
  int16_t imm;
};
struct operand_r_r {
  uint8_t lhs;
  uint8_t rhs;
};
struct operand_mem {
  uint8_t base;
  uint8_t index;
  int8_t imm;
};
struct ast {
  enum ast_type type;
  union {
    struct token *token;
    struct operand_r operand_r;
    struct operand_imm operand_imm;
    struct operand_r_r operand_r_r;
    struct operand_r_imm operand_r_imm;
    struct operand_mem operand_mem;
    uint8_t data8;
    uint16_t data16;
  };
};
struct ast *ast_create() {
  struct ast *ast = (struct ast *)malloc(sizeof(struct ast));
  memset(ast, 0, sizeof(struct ast));
  ast->type = AST_INVALID;
  return ast;
}
void ast_delete(struct ast *ast) {
  switch (ast->type) {
  case AST_TOKEN:
  case AST_ARITHMETIC:
  case AST_BRANCH:
  case AST_MEM:
  case AST_OPCODE:
  case AST_LABEL:
    token_delete(ast->token);
    break;
  case AST_OPERAND_R:
  case AST_OPERAND_IMM:
  case AST_OPERAND_R_R:
  case AST_OPERAND_R_IMM:
  case AST_OPERAND_MEM:
  case AST_DATA8:
  case AST_DATA16:
  case AST_INVALID:
    break;
  }
  free(ast);
}
struct ast *ast_from_token(const struct token *token) {
  struct ast *ast = (struct ast *)malloc(sizeof(struct ast));
  ast->type = AST_TOKEN;
  ast->token = token_copy(token);
  return ast;
}
int ast_snprintf(const struct ast *ast, char *buf, uint32_t buf_len) {
  if (ast->type == AST_TOKEN) {
    return snprintf(buf, buf_len, "AST_TOKEN(%s)",
                    token_descriptions[ast->token->type]);
  } else {
    return snprintf(buf, buf_len, "%s", ast_type_descriptions[ast->type]);
  }
}
enum rule_type { RT_AST, RT_TOKEN, RT_END };
struct rule_it {
  enum rule_type rule_type;
  union {
    enum ast_type ast;
    enum token_type token;
  };
};
enum rule {
  R_INVALID,
  R_R0,
  R_R1,
  R_R2,
  R_R3,
  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_SP,
  R_TOK_IMMEDIATE,
  R_OPERAND_R_R,
  R_OPERAND_R_IMM,
  R_OPERAND_R_LABEL,
  R_OPERAND_MEM,
  R_ADD,
  R_SUB,
  R_MUL,
  R_UDIV,
  R_UMOD,
  R_SDIV,
  R_SMOD,
  R_AND,
  R_OR,
  R_XOR,
  R_SHL,
  R_SHR,
  R_ASR,
  R_CMP,
  R_MOV,
  R_CALL,
  R_RET,
  R_JMP,
  R_JE,
  R_JL,
  R_JLE,
  R_JB,
  R_JBE,
  R_STORE,
  R_STOREB,
  R_LOAD,
  R_LOADB,
  R_SYSCALL,
  R_ARITHMETIC_R_R,
  R_ARITHMETIC_R_IMM,
  R_ARITHMETIC_R_LABEL,
  R_BRANCH_R,
  R_BRANCH_LABEL,
  R_MEM_R_R,
  R_MEM_R_IMM,
  R_MEM_R_MEM,
  R_MEM_R_LABEL,
  R_PUSH_R,
  R_POP_R,
  R_OPCODE,
  R_SECTION_CODE,
  R_SECTION_DATA,
  R_LABEL,
  R_NEWLINE_NEWLINE,
  R_DB,
  R_DW,
  R_STRING
};
const char *rule_descriptions[] = {"R_INVALID",
                                   "R_R0",
                                   "R_R1",
                                   "R_R2",
                                   "R_R3",
                                   "R_R4",
                                   "R_R5",
                                   "R_R6",
                                   "R_R7",
                                   "R_SP",
                                   "R_TOK_IMMEDIATE",
                                   "R_OPERAND_R_R",
                                   "R_OPERAND_R_IMM",
                                   "R_OPERAND_R_LABEL",
                                   "R_OPERAND_MEM",
                                   "R_ADD",
                                   "R_SUB",
                                   "R_MUL",
                                   "R_UDIV",
                                   "R_UMOD",
                                   "R_SDIV",
                                   "R_SMOD",
                                   "R_AND",
                                   "R_OR",
                                   "R_XOR",
                                   "R_SHL",
                                   "R_SHR",
                                   "R_ASR",
                                   "R_CMP",
                                   "R_MOV",
                                   "R_CALL",
                                   "R_RET",
                                   "R_JMP",
                                   "R_JE",
                                   "R_JL",
                                   "R_JLE",
                                   "R_JB",
                                   "R_JBE",
                                   "R_STORE",
                                   "R_STOREB",
                                   "R_LOAD",
                                   "R_LOADB",
                                   "R_SYCALL",
                                   "R_ARITHMETIC_R_R",
                                   "R_ARITHMETIC_R_IMM",
                                   "R_ARITHMETIC_R_LABEL",
                                   "R_BRANCH_R",
                                   "R_BRANCH_LABEL",
                                   "R_MEM_R_R",
                                   "R_MEM_R_IMM",
                                   "R_MEM_R_MEM",
                                   "R_MEM_R_LABEL",
                                   "R_PUSH_R",
                                   "R_POP_R",
                                   "R_OPCODE",
                                   "R_SECTION_CODE",
                                   "R_SECTION_DATA",
                                   "R_LABEL",
                                   "R_NEWLINE_NEWLINE",
                                   "R_DB",
                                   "R_DW",
                                   "R_STRING"};
struct rules {
  enum rule rule;
  struct rule_it items[16];
  enum token_type lookaheads[8];
};
struct rules RULES[] = {
    {R_SECTION_CODE,
     {{RT_TOKEN, .token = TOK_DOT_CODE}, {RT_END}},
     {TOK_INVALID}},
    {R_SECTION_DATA,
     {{RT_TOKEN, .token = TOK_DOT_DATA}, {RT_END}},
     {TOK_INVALID}},
    {R_R0, {{RT_TOKEN, .token = TOK_R0}, {RT_END}}, {TOK_INVALID}},
    {R_R1, {{RT_TOKEN, .token = TOK_R1}, {RT_END}}, {TOK_INVALID}},
    {R_R2, {{RT_TOKEN, .token = TOK_R2}, {RT_END}}, {TOK_INVALID}},
    {R_R3, {{RT_TOKEN, .token = TOK_R3}, {RT_END}}, {TOK_INVALID}},
    {R_R4, {{RT_TOKEN, .token = TOK_R4}, {RT_END}}, {TOK_INVALID}},
    {R_R5, {{RT_TOKEN, .token = TOK_R5}, {RT_END}}, {TOK_INVALID}},
    {R_R6, {{RT_TOKEN, .token = TOK_R6}, {RT_END}}, {TOK_INVALID}},
    {R_R7, {{RT_TOKEN, .token = TOK_R7}, {RT_END}}, {TOK_INVALID}},
    {R_SP, {{RT_TOKEN, .token = TOK_SP}, {RT_END}}, {TOK_INVALID}},
    {R_TOK_IMMEDIATE,
     {{RT_TOKEN, .token = TOK_IMMEDIATE}, {RT_END}},
     {TOK_INVALID}},
    {R_OPERAND_R_R,
     {{RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_END}},
     {TOK_COMMA, TOK_INVALID}},
    {R_OPERAND_R_IMM,
     {{RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_AST, .ast = AST_OPERAND_IMM},
      {RT_END}},
     {TOK_BRACKET_CLOSE, TOK_INVALID}},
    {R_OPERAND_MEM,
     {{RT_TOKEN, .token = TOK_BRACKET_OPEN},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_AST, .ast = AST_OPERAND_IMM},
      {RT_TOKEN, .token = TOK_BRACKET_CLOSE},
      {RT_END}},
     {TOK_INVALID}},
    {R_ADD, {{RT_TOKEN, .token = TOK_ADD}, {RT_END}}, {TOK_INVALID}},
    {R_SUB, {{RT_TOKEN, .token = TOK_SUB}, {RT_END}}, {TOK_INVALID}},
    {R_MUL, {{RT_TOKEN, .token = TOK_MUL}, {RT_END}}, {TOK_INVALID}},
    {R_UDIV, {{RT_TOKEN, .token = TOK_UDIV}, {RT_END}}, {TOK_INVALID}},
    {R_UMOD, {{RT_TOKEN, .token = TOK_UMOD}, {RT_END}}, {TOK_INVALID}},
    {R_SDIV, {{RT_TOKEN, .token = TOK_SDIV}, {RT_END}}, {TOK_INVALID}},
    {R_SMOD, {{RT_TOKEN, .token = TOK_SMOD}, {RT_END}}, {TOK_INVALID}},
    {R_AND, {{RT_TOKEN, .token = TOK_AND}, {RT_END}}, {TOK_INVALID}},
    {R_OR, {{RT_TOKEN, .token = TOK_OR}, {RT_END}}, {TOK_INVALID}},
    {R_XOR, {{RT_TOKEN, .token = TOK_XOR}, {RT_END}}, {TOK_INVALID}},
    {R_SHL, {{RT_TOKEN, .token = TOK_SHL}, {RT_END}}, {TOK_INVALID}},
    {R_SHR, {{RT_TOKEN, .token = TOK_SHR}, {RT_END}}, {TOK_INVALID}},
    {R_ASR, {{RT_TOKEN, .token = TOK_ASR}, {RT_END}}, {TOK_INVALID}},
    {R_CMP, {{RT_TOKEN, .token = TOK_CMP}, {RT_END}}, {TOK_INVALID}},
    {R_MOV, {{RT_TOKEN, .token = TOK_MOV}, {RT_END}}, {TOK_INVALID}},
    {R_CALL, {{RT_TOKEN, .token = TOK_CALL}, {RT_END}}, {TOK_INVALID}},
    {R_RET, {{RT_TOKEN, .token = TOK_RET}, {RT_END}}, {TOK_INVALID}},
    {R_JMP, {{RT_TOKEN, .token = TOK_JMP}, {RT_END}}, {TOK_INVALID}},
    {R_JE, {{RT_TOKEN, .token = TOK_JE}, {RT_END}}, {TOK_INVALID}},
    {R_JL, {{RT_TOKEN, .token = TOK_JL}, {RT_END}}, {TOK_INVALID}},
    {R_JLE, {{RT_TOKEN, .token = TOK_JLE}, {RT_END}}, {TOK_INVALID}},
    {R_JB, {{RT_TOKEN, .token = TOK_JB}, {RT_END}}, {TOK_INVALID}},
    {R_JBE, {{RT_TOKEN, .token = TOK_JBE}, {RT_END}}, {TOK_INVALID}},
    {R_LOAD, {{RT_TOKEN, .token = TOK_LOAD}, {RT_END}}, {TOK_INVALID}},
    {R_LOADB, {{RT_TOKEN, .token = TOK_LOADB}, {RT_END}}, {TOK_INVALID}},
    {R_STORE, {{RT_TOKEN, .token = TOK_STORE}, {RT_END}}, {TOK_INVALID}},
    {R_STOREB, {{RT_TOKEN, .token = TOK_STOREB}, {RT_END}}, {TOK_INVALID}},
    {R_SYSCALL, {{RT_TOKEN, .token = TOK_SYSCALL}, {RT_END}}, {TOK_INVALID}},
    {R_PUSH_R,
     {{RT_TOKEN, .token = TOK_PUSH}, {RT_AST, .ast = AST_OPERAND_R}, {RT_END}},
     {TOK_INVALID}},
    {R_POP_R,
     {{RT_TOKEN, .token = TOK_POP}, {RT_AST, .ast = AST_OPERAND_R}, {RT_END}},
     {TOK_INVALID}},
    {R_ARITHMETIC_R_R,
     {{RT_AST, .ast = AST_ARITHMETIC},
      {RT_AST, .ast = AST_OPERAND_R_R},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_ARITHMETIC_R_IMM,
     {{RT_AST, .ast = AST_ARITHMETIC},
      {RT_AST, .ast = AST_OPERAND_R_IMM},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_ARITHMETIC_R_LABEL,
     {{RT_AST, .ast = AST_ARITHMETIC},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_TOKEN, .token = TOK_IDENTIFIER},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_BRANCH_LABEL,
     {{RT_AST, .ast = AST_BRANCH},
      {RT_TOKEN, .token = TOK_IDENTIFIER},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_BRANCH_R,
     {{RT_AST, .ast = AST_BRANCH},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_MEM_R_IMM,
     {{RT_AST, .ast = AST_BRANCH},
      {RT_AST, .ast = AST_OPERAND_R_IMM},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_MEM_R_R,
     {{RT_AST, .ast = AST_MEM},
      {RT_AST, .ast = AST_OPERAND_R_R},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_MEM_R_MEM,
     {{RT_AST, .ast = AST_MEM},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_AST, .ast = AST_OPERAND_MEM},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_MEM_R_LABEL,
     {{RT_AST, .ast = AST_BRANCH},
      {RT_AST, .ast = AST_OPERAND_R},
      {RT_TOKEN, .token = TOK_COMMA},
      {RT_TOKEN, .token = TOK_IDENTIFIER},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_OPCODE,
     {{RT_AST, .ast = AST_OPCODE}, {RT_TOKEN, .token = TOK_NEWLINE}, {RT_END}},
     {TOK_INVALID}},
    {R_NEWLINE_NEWLINE,
     {{RT_TOKEN, .token = TOK_NEWLINE},
      {RT_TOKEN, .token = TOK_NEWLINE},
      {RT_END}},
     {TOK_INVALID}},
    {R_LABEL,
     {{RT_TOKEN, .token = TOK_IDENTIFIER},
      {RT_TOKEN, .token = TOK_COLON},
      {RT_END}},
     {TOK_INVALID}},
    {R_DB,
     {{RT_TOKEN, .token = TOK_DB}, {RT_AST, .ast = AST_OPERAND_IMM}, {RT_END}},
     {TOK_INVALID}},
    {R_DW,
     {{RT_TOKEN, .token = TOK_DW}, {RT_AST, .ast = AST_OPERAND_IMM}, {RT_END}},
     {TOK_INVALID}},
    {R_STRING, {{RT_TOKEN, .token = TOK_STRING}, {RT_END}}, {TOK_INVALID}},
};
int rules_size(const struct rules *rules) {
  unsigned int i;
  for (i = 0; i < 16; i++) {
    if (rules->items[i].rule_type == RT_END) {
      return i;
    }
  }
  return -1;
}
enum rule parser_match_rule(struct parser *parser) {
  int i, j;
  for (i = 0; i < sizeof(RULES) / sizeof(struct rules); i++) {
    int rs = rules_size(&RULES[i]);
    if (rs == 0) {
      continue;
    }
    for (j = 0; j < rs; j++) {
      const struct rule_it *item = &RULES[i].items[rs - j - 1];
      if (item->rule_type == RT_AST) {
        struct ast *ast = (struct ast *)stack_peek(parser->stack, j);
        if ((ast == NULL) || (item->ast != ast->type)) {
          break;
        }
      } else if (item->rule_type == RT_TOKEN) {
        struct ast *ast = (struct ast *)stack_peek(parser->stack, j);
        if ((ast == NULL) || (ast->type != AST_TOKEN) ||
            (item->token != ast->token->type)) {
          break;
        }
      } else {
        break;
      }
    }
    if (j != rs) {
      continue;
    }
    if (parser->lookahead == NULL) {
      return RULES[i].rule;
    }
    int lookahead_pass = 1;
    for (j = 0; RULES[i].lookaheads[j] != TOK_INVALID; j++) {
      if (parser->lookahead->type == RULES[i].lookaheads[j]) {
        lookahead_pass = 0;
        break;
      }
    }
    if (lookahead_pass)
      return RULES[i].rule;
  }
  return R_INVALID;
}
int token_type_to_opcode(enum token_type token) {
  switch (token) {
  case TOK_ADD:
    return OP_ADD;
  case TOK_SUB:
    return OP_SUB;
  case TOK_MUL:
    return OP_MUL;
  case TOK_UDIV:
    return OP_UDIV;
  case TOK_UMOD:
    return OP_UMOD;
  case TOK_SDIV:
    return OP_SDIV;
  case TOK_SMOD:
    return OP_SMOD;
  case TOK_AND:
    return OP_AND;
  case TOK_OR:
    return OP_OR;
  case TOK_XOR:
    return OP_XOR;
  case TOK_SHL:
    return OP_SHL;
  case TOK_SHR:
    return OP_SHR;
  case TOK_ASR:
    return OP_ASR;
  case TOK_CMP:
    return OP_CMP;
  case TOK_MOV:
    return OP_MOV;
  case TOK_CALL:
    return OP_CALL;
  case TOK_RET:
    return OP_RET;
  case TOK_JMP:
    return OP_JMP;
  case TOK_JE:
    return OP_JE;
  case TOK_JL:
    return OP_JL;
  case TOK_JLE:
    return OP_JLE;
  case TOK_JB:
    return OP_JB;
  case TOK_JBE:
    return OP_JBE;
  case TOK_PUSH:
    return OP_PUSH;
  case TOK_POP:
    return OP_POP;
  case TOK_STORE:
    return OP_STORE;
  case TOK_STOREB:
    return OP_STOREB;
  case TOK_LOAD:
    return OP_LOAD;
  case TOK_LOADB:
    return OP_LOADB;
  case TOK_SYSCALL:
    return OP_SYSCALL;
  default:
    snprintf(error_description, ERROR_DESCRIPTION_SIZE,
             "Invalid opcode token: %u\n", token);
    return -1;
  }
}
int parser_buf_push(struct parser *parser, uint8_t byte) {
  switch (parser->section) {
  case SECTION_INVALID:
    return ERROR_PARSER_INVALID_SECTION;
  case SECTION_CODE:
    return bb_push(parser->code, byte);
  case SECTION_DATA:
    return bb_push(parser->data, byte);
  default:
    return ERROR_PARSER_INVALID_SECTION;
  }
}
struct parser *parser_create() {
  struct parser *parser = malloc(sizeof(struct parser));
  parser->section = SECTION_INVALID;
  parser->label_locations = aa_tree_create((aa_tree_cmp)label_location_cmp);
  parser->relocations = list_create();
  parser->stack = stack_create();
  parser->code = bb_create(0x10000);
  parser->data = bb_create(0x10000);
  return parser;
}
void parser_delete(struct parser *parser) {
  aa_tree_delete(parser->label_locations, (aa_tree_del)label_location_delete);
  list_delete(parser->relocations, (void (*)(void *))relocation_delete);
  stack_delete(parser->stack, (void (*)(void *))ast_delete);
  bb_delete(parser->code);
  bb_delete(parser->data);
  free(parser);
}
int parser_reduce(struct parser *parser) {
  enum rule rule;
  while (1) {
    rule = parser_match_rule(parser);
    if (rule == R_INVALID) {
      break;
    }
    switch (rule) {
    case R_SECTION_CODE:
      ast_delete(stack_pop(parser->stack));
      parser->section = SECTION_CODE;
      break;
    case R_SECTION_DATA:
      ast_delete(stack_pop(parser->stack));
      parser->section = SECTION_DATA;
      break;
    case R_R0:
    case R_R1:
    case R_R2:
    case R_R3:
    case R_R4:
    case R_R5:
    case R_R6:
    case R_R7:
    case R_SP: {
      struct ast *ast = stack_pop(parser->stack);
      ast_delete(ast);
      ast = ast_create();
      ast->type = AST_OPERAND_R;
      switch (rule) {
      case R_R0:
        ast->operand_r.reg = R0;
        break;
      case R_R1:
        ast->operand_r.reg = R1;
        break;
      case R_R2:
        ast->operand_r.reg = R2;
        break;
      case R_R3:
        ast->operand_r.reg = R3;
        break;
      case R_R4:
        ast->operand_r.reg = R4;
        break;
      case R_R5:
        ast->operand_r.reg = R5;
        break;
      case R_R6:
        ast->operand_r.reg = R6;
        break;
      case R_R7:
        ast->operand_r.reg = R7;
        break;
      case R_SP:
        ast->operand_r.reg = R7;
        break;
      default:
        snprintf(error_description, ERROR_DESCRIPTION_SIZE, "invalid register");
        ast_delete(ast);
        return ERROR_PARSER_INTERNAL;
      }
      stack_push(parser->stack, ast);
      break;
    }
    case R_TOK_IMMEDIATE: {
      struct ast *ast = stack_pop(parser->stack);
      struct ast *new_ast = ast_create();
      new_ast->type = AST_OPERAND_IMM;
      new_ast->operand_imm.imm = ast->token->imm;
      ast_delete(ast);
      stack_push(parser->stack, new_ast);
      break;
    }
    case R_OPERAND_MEM: {
      ast_delete(stack_pop(parser->stack));
      struct ast *imm = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *index = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *base = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *ast = ast_create();
      ast->type = AST_OPERAND_MEM;
      ast->operand_mem.base = base->operand_r.reg;
      ast->operand_mem.index = index->operand_r.reg;
      ast->operand_mem.imm = imm->operand_imm.imm;
      ast_delete(base);
      ast_delete(index);
      ast_delete(imm);
      stack_push(parser->stack, ast);
      break;
    }
    case R_OPERAND_R_R: {
      struct ast *rhs = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *lhs = stack_pop(parser->stack);
      struct ast *new_ast = ast_create();
      new_ast->type = AST_OPERAND_R_R;
      new_ast->operand_r_r.lhs = lhs->operand_r.reg;
      new_ast->operand_r_r.rhs = rhs->operand_r.reg;
      ast_delete(lhs);
      ast_delete(rhs);
      stack_push(parser->stack, new_ast);
      break;
    }
    case R_OPERAND_R_IMM: {
      struct ast *imm = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *reg = stack_pop(parser->stack);
      struct ast *new_ast = ast_create();
      new_ast->type = AST_OPERAND_R_IMM;
      new_ast->operand_r_imm.reg = reg->operand_r.reg;
      new_ast->operand_r_imm.imm = imm->operand_imm.imm;
      ast_delete(imm);
      ast_delete(reg);
      stack_push(parser->stack, new_ast);
      break;
    }
    case R_ADD:
    case R_SUB:
    case R_MUL:
    case R_UDIV:
    case R_UMOD:
    case R_SDIV:
    case R_SMOD:
    case R_AND:
    case R_OR:
    case R_XOR:
    case R_SHL:
    case R_SHR:
    case R_ASR:
    case R_CMP:
    case R_MOV: {
      struct ast *ast = stack_peek(parser->stack, 0);
      ast->type = AST_ARITHMETIC;
      break;
    }
    case R_CALL:
    case R_JMP:
    case R_JE:
    case R_JL:
    case R_JLE:
    case R_JB:
    case R_JBE: {
      struct ast *ast = stack_peek(parser->stack, 0);
      ast->type = AST_BRANCH;
      break;
    }
    case R_LOAD:
    case R_LOADB:
    case R_STORE:
    case R_STOREB: {
      struct ast *ast = stack_peek(parser->stack, 0);
      ast->type = AST_MEM;
      break;
    }
    case R_RET:
    case R_SYSCALL: {
      struct ast *ast = stack_peek(parser->stack, 0);
      ast->type = AST_OPCODE;
      break;
    }
    case R_PUSH_R:
    case R_POP_R: {
      struct ast *r = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      uint8_t op = 0;
      if (rule == R_PUSH_R) {
        op = OP_PUSH << 3;
      } else {
        op = OP_POP << 3;
      }
      op = op | OPERAND_R;
      try
        (parser_buf_push(parser, op)) try
          (parser_buf_push(parser, r->operand_r.reg)) ast_delete(r);
      break;
    }
    case R_ARITHMETIC_R_R:
    case R_MEM_R_R: {
      ast_delete(stack_pop(parser->stack));
      struct ast *rr = stack_pop(parser->stack);
      struct ast *opcode = stack_pop(parser->stack);
      uint8_t op = 0;
      int op_from_token = token_type_to_opcode(opcode->token->type);
      if (op_from_token < 0) {
        ast_delete(opcode);
        ast_delete(rr);
        return ERROR_PARSER_INTERNAL;
      }
      op = op_from_token;
      op = op << 3;
      op |= OPERAND_R_R;
      try
        (parser_buf_push(parser, op)) op = rr->operand_r_r.lhs << 4;
      op |= rr->operand_r_r.rhs;
      try
        (parser_buf_push(parser, op)) ast_delete(opcode);
      ast_delete(rr);
      break;
    }
    case R_MEM_R_IMM:
    case R_ARITHMETIC_R_IMM: {
      ast_delete(stack_pop(parser->stack));
      struct ast *r_imm = stack_pop(parser->stack);
      struct ast *opcode = stack_pop(parser->stack);
      uint8_t op = 0;
      int op_from_token = token_type_to_opcode(opcode->token->type);
      if (op_from_token < 0) {
        ast_delete(opcode);
        ast_delete(r_imm);
        return ERROR_PARSER_INTERNAL;
      }
      op = op_from_token;
      op = op << 3;
      if ((r_imm->operand_r_imm.imm < -128) ||
          (r_imm->operand_r_imm.imm > 127)) {
        op |= OPERAND_R_IMM16;
        try
          (parser_buf_push(parser, op)) try
            (parser_buf_push(parser, r_imm->operand_r_imm.reg)) try
              (parser_buf_push(parser,
                               (r_imm->operand_r_imm.imm >> 8) & 0xff)) try
                (parser_buf_push(parser, r_imm->operand_r_imm.imm & 0xff))
      } else {
        op |= OPERAND_R_IMM8;
        try
          (parser_buf_push(parser, op)) try
            (parser_buf_push(parser, r_imm->operand_r_imm.reg)) try
              (parser_buf_push(parser, r_imm->operand_r_imm.imm & 0xff))
      }
      ast_delete(opcode);
      ast_delete(r_imm);
      break;
    }
    case R_MEM_R_MEM: {
      ast_delete(stack_pop(parser->stack));
      struct ast *mem = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *r = stack_pop(parser->stack);
      struct ast *opcode = stack_pop(parser->stack);
      int op_from_token = token_type_to_opcode(opcode->token->type);
      if (op_from_token < 0) {
        ast_delete(opcode);
        ast_delete(r);
        ast_delete(mem);
        return ERROR_PARSER_INTERNAL;
      }
      uint8_t op = 0;
      op = op_from_token;
      op = op << 3;
      op |= OPERAND_R_MEM;
      try
        (parser_buf_push(parser, op)) try
          (parser_buf_push(parser, r->operand_r.reg)) op = mem->operand_mem.base
                                                           << 4;
      op |= mem->operand_mem.index;
      try
        (parser_buf_push(parser, op)) try
          (parser_buf_push(parser, mem->operand_mem.imm)) ast_delete(opcode);
      ast_delete(r);
      ast_delete(mem);
      break;
    }
    case R_ARITHMETIC_R_LABEL: {
      ast_delete(stack_pop(parser->stack));
      struct ast *identifier = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      struct ast *r = stack_pop(parser->stack);
      struct ast *opcode = stack_pop(parser->stack);
      uint8_t reg = r->operand_r.reg;
      ast_delete(r);
      int op_from_token = token_type_to_opcode(opcode->token->type);
      if (op_from_token < 0) {
        ast_delete(opcode);
        ast_delete(identifier);
        return ERROR_PARSER_INTERNAL;
      }
      if (parser->section != SECTION_CODE) {
        ast_delete(opcode);
        ast_delete(identifier);
        return ERROR_PARSER_INTERNAL;
      }
      struct relocation *rel =
          relocation_create(RELOCATION_ABSOLUTE, bb_length(parser->code) + 2, 0,
                            SECTION_CODE, identifier->token->text);
      list_append(parser->relocations, rel);
      uint8_t op = op_from_token;
      op = op << 3;
      op |= OPERAND_R_IMM16;
      try
        (parser_buf_push(parser, op)) try
          (parser_buf_push(parser, reg)) try
            (parser_buf_push(parser, 0xde)) try
              (parser_buf_push(parser, 0xad)) ast_delete(opcode);
      ast_delete(identifier);
      break;
    }
    case R_BRANCH_R: {
      ast_delete(stack_pop(parser->stack));
      struct ast *r = stack_pop(parser->stack);
      struct ast *opcode = stack_pop(parser->stack);
      uint8_t op = 0;
      int op_from_token = token_type_to_opcode(opcode->token->type);
      if (op_from_token < 0) {
        ast_delete(opcode);
        ast_delete(r);
        return ERROR_PARSER_INTERNAL;
      }
      op = op_from_token;
      op = op << 3;
      op |= OPERAND_R;
      try
        (parser_buf_push(parser, op)) try
          (parser_buf_push(parser, r->operand_r.reg)) ast_delete(opcode);
      ast_delete(r);
      break;
    }
    case R_BRANCH_LABEL: {
      ast_delete(stack_pop(parser->stack));
      struct ast *label = stack_pop(parser->stack);
      struct ast *opcode = stack_pop(parser->stack);
      uint8_t op = 0;
      int op_from_token = token_type_to_opcode(opcode->token->type);
      if (op_from_token < 0) {
        ast_delete(opcode);
        ast_delete(label);
        return ERROR_PARSER_INTERNAL;
      }
      if (parser->section != SECTION_CODE) {
        ast_delete(opcode);
        ast_delete(label);
        return ERROR_PARSER_INVALID_SECTION;
      }
      struct relocation *rel =
          relocation_create(RELOCATION_PCREL, bb_length(parser->code) + 1, 1,
                            SECTION_CODE, label->token->text);
      list_append(parser->relocations, rel);
      op = op_from_token;
      op = op << 3;
      op |= OPERAND_IMM16;
      try
        (parser_buf_push(parser, op)) try
          (parser_buf_push(parser, 0xde)) try
            (parser_buf_push(parser, 0xad)) ast_delete(opcode);
      ast_delete(label);
      break;
    }
    case R_OPCODE: {
      ast_delete(stack_pop(parser->stack));
      struct ast *opcode = stack_pop(parser->stack);
      int op_from_token = token_type_to_opcode(opcode->token->type);
      ast_delete(opcode);
      if (op_from_token < 0) {
        return ERROR_PARSER_INTERNAL;
      }
      uint8_t op = op_from_token;
      op = op << 3;
      op |= OPERAND_NONE;
      try
        (parser_buf_push(parser, op)) break;
    }
    case R_LABEL: {
      ast_delete(stack_pop(parser->stack));
      struct ast *label = stack_pop(parser->stack);
      enum section section = SECTION_INVALID;
      uint16_t offset = 0;
      switch (parser->section) {
      case SECTION_CODE:
        section = SECTION_CODE;
        offset = bb_length(parser->code);
        break;
      case SECTION_DATA:
        section = SECTION_DATA;
        offset = bb_length(parser->data);
        break;
      default:
        ast_delete(label);
        return ERROR_PARSER_INTERNAL;
      }
      struct label_location *ll =
          label_location_create(label->token->text, offset, section);
      struct label_location *existing_ll =
          aa_tree_fetch(parser->label_locations, ll);
      ast_delete(label);
      if (existing_ll != NULL) {
        snprintf(error_description, ERROR_DESCRIPTION_SIZE,
                 "%s is a duplicate label", ll->label);
        label_location_delete(ll);
        return ERROR_PARSER_DUPLICATE_LABEL;
      }
      try
        (aa_tree_insert(parser->label_locations, ll)) break;
    }
    case R_NEWLINE_NEWLINE: {
      ast_delete(stack_pop(parser->stack));
      break;
    }
    case R_DB:
    case R_DW: {
      struct ast *imm = stack_pop(parser->stack);
      ast_delete(stack_pop(parser->stack));
      if (parser->section != SECTION_DATA) {
        ast_delete(imm);
        return ERROR_PARSER_INVALID_SECTION;
      }
      if (rule == R_DW) {
        try
          (bb_push(parser->data, (imm->operand_imm.imm >> 8) & 0xff))
      }
      try
        (bb_push(parser->data, imm->operand_imm.imm & 0xff)) ast_delete(imm);
      break;
    }
    case R_STRING: {
      struct ast *string = stack_pop(parser->stack);
      if (parser->section != SECTION_DATA) {
        ast_delete(string);
        return ERROR_PARSER_INVALID_SECTION;
      }
      unsigned int string_len = strlen(string->token->text);
      try
        (bb_append(parser->data, string->token->text, string_len))
            ast_delete(string);
      break;
    }
    default:
      return ERROR_PARSER_INTERNAL;
    }
  }
  return SUCCESS;
}
int parser_relocations(struct parser *parser) {
  struct list_it *rel_it;
  for (rel_it = parser->relocations->head; rel_it != NULL;
       rel_it = rel_it->next) {
    struct relocation *rel = rel_it->data;
    struct label_location *ll_seek =
        label_location_create(rel->target, 0, SECTION_INVALID);
    struct label_location *ll = aa_tree_fetch(parser->label_locations, ll_seek);
    label_location_delete(ll_seek);
    if (ll == NULL) {
      return ERROR_PARSER_LABEL_RESOLVE;
    }
    uint16_t offset = ll->offset + rel->addend;
    if (rel->relocation_type == RELOCATION_PCREL) {
      offset = offset - rel->location;
    }
    ((uint8_t *)bb_data(parser->code))[rel->location] = (uint8_t)(offset >> 8);
    ((uint8_t *)bb_data(parser->code))[rel->location + 1] =
        (uint8_t)(offset & 0xff);
  }
  return SUCCESS;
}
int parse(const struct list *tokens, struct binary **binary) {
  struct parser *parser = parser_create();
  struct list_it *it;
  for (it = tokens->head; it != NULL; it = it->next) {
    try_cleanup_start(stack_push(parser->stack, ast_from_token(it->data)))
        parser_delete(parser);
    try_cleanup_end if (it->next == NULL) parser->lookahead = NULL;
    else parser->lookahead = it->next->data;
    try_cleanup_start(parser_reduce(parser)) parser_delete(parser);
    try_cleanup_end
  }
  struct ast *top = stack_peek(parser->stack, 0);
  if ((top != NULL) &&
      ((top->type != AST_TOKEN) || (top->token->type != TOK_NEWLINE) ||
       (stack_peek(parser->stack, 1) != NULL))) {
    parser_delete(parser);
    snprintf(error_description, ERROR_DESCRIPTION_SIZE,
             "Parser failed to parse all input");
    return ERROR_PARSER_INTERNAL;
  }
  try_cleanup_start(parser_relocations(parser)) parser_delete(parser);
  try_cleanup_end *binary =
      binary_create(bb_data(parser->code), bb_length(parser->code),
                    bb_data(parser->data), bb_length(parser->data));
  parser_delete(parser);
  return SUCCESS;
}