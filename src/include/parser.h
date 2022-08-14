#ifndef _LCP_PARSER_H_
#define _LCP_PARSER_H_

#include <vector>
#include <assert.h>
#include <map>
#include "token.h"
#include "op.h"
#include "program.h"
#include "error.h"

Op convert_token_to_op(Token tok, Program program);
std::vector<Op> link_ops(std::vector<Op> ops, std::map<std::string, int> labels);
Program parse_tokens(std::vector<Token> tokens);

#endif