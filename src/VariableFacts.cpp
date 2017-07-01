#include "AstPrinter.hpp"
#include "VariableFacts.hpp"

#include "Macros.hpp"
#include "TemplateUtil.hpp"

namespace rosa {

VariableFacts::VariableFacts(const SPtr &input) {
  lvalue_mode_.push(false);
  visit(input);
}

void VariableFacts::visit(const SPtr &sxp) {
  switch (sxp->type()) {
    case CLOSXP: {
      visit(Cast<CLOSXP>(sxp)->body());
      break;
    }
    case LANGSXP: {
      visit(Cast<LANGSXP>(sxp));
      break;
    }
    case LISTSXP: {
      visit(Cast<LISTSXP>(sxp));
      break;
    }
    case SYMSXP: {
      visit(Cast<SYMSXP>(sxp));
      break;
    }
    default:
      break;
  }
}

void VariableFacts::visit(const LangPtr &sxp) {
  if (sxp->op()->type() != SYMSXP) {
    FATAL_ERROR("Cannot handle high order function calls!\n");
  }

  const std::string &op_name = Cast<SYMSXP>(sxp->op())->name();
  const std::vector<SPtr> &args = sxp->arguments()->values();

  if (op_name == "<-" || op_name == "=" || op_name == "_$for_cond") {
    lvalue_mode_.push(true);
    visit(args[0]);
    lvalue_mode_.pop();

    if (args[0]->type() == SYMSXP) {
      const std::string &var_name = Cast<SYMSXP>(args[0])->name();
      values_[sxp][var_name] = { args[1] };
    }

    lvalue_mode_.push(false);
    for (std::size_t i = 1; i < args.size(); ++i) {
      visit(args[i]);
    }
    lvalue_mode_.pop();

    merge(sxp, args);
  } else if (lvalue_mode_.top()) {
    if (op_name == "[" || op_name == "[[" || op_name == "$") {
      visit(args[0]);

      lvalue_mode_.push(false);
      visit(args[1]);
      lvalue_mode_.pop();

      merge(sxp, args);
    } else {
      FATAL_ERROR("Unhandled l-value case: " + ToString(sxp) + "\n");
    }
  } else {
    const ListPtr arg_list = sxp->arguments();
    visit(arg_list);
    use_[sxp] = use_[arg_list];
    def_[sxp] = def_[arg_list];
    values_[sxp] = values_[arg_list];
  }
}

void VariableFacts::visit(const ListPtr &sxp) {
  for (const auto &child : sxp->values()) {
    visit(child);
  }
  merge(sxp, sxp->values());
}

void VariableFacts::visit(const SymPtr &sxp) {
  if (lvalue_mode_.top()) {
    def_[sxp].emplace(sxp->name());
  } else {
    use_[sxp].emplace(sxp->name());
  }
}

void VariableFacts::merge(const SPtr &node, const std::vector<SPtr> &children) {
  for (const auto &c : children) {
    const auto &cu = use_[c];
    use_[node].insert(cu.begin(), cu.end());
    const auto &cd = def_[c];
    def_[node].insert(cd.begin(), cd.end());
    const auto &cv = values_[c];
    for (const auto &it : cv) {
      values_[node][it.first].insert(it.second.begin(), it.second.end());
    }
  }
}

}  // namespace rosa
