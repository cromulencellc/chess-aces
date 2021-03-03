#include "opcode.hpp"

#include "opcodes/push_str.hpp"
#include "opcodes/deref.hpp"
#include "opcodes/eq.hpp"
#include "opcodes/like.hpp"
#include "opcodes/not_op.hpp"
#include "opcodes/and_op.hpp"
#include "opcodes/or_op.hpp"
#include "opcodes/xor_op.hpp"
#include "opcodes/push.hpp"

using namespace yaml;
using namespace yaml::opcodes;

#define mk(k) std::make_shared<k>(query_str);

OpcodePtr Opcode::next_opcode(std::string_view& query_str) {
  if (0 == query_str.size()) return nullptr;

  switch ((Opcode::Code) query_str.at(0)) {
  case Code::push_str:
    return mk(PushStr);
  case Code::deref:
    return mk(Deref);
  case Code::eq:
    return mk(Eq);
  case Code::like:
    return mk(Like);
  case Code::not_op:
    return mk(NotOp);
  case Code::and_op:
    return mk(AndOp);
  case Code::or_op:
    return mk(OrOp);
  case Code::xor_op:
    return mk(XorOp);
  case Code::push_true:
    return mk(PushTrue);
  case Code::push_false:
    return mk(PushFalse);
  default:
    throw UnknownOperationError(query_str.at(0));
  }
}
