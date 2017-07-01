#include <memory>
#include <string>
#include <vector>

#include "Ast.hpp"
#include "AstTransforms.hpp"
#include "TemplateUtil.hpp"

namespace rosa {

SPtr AstTransform::apply(const SPtr &sxp) {
  return applyInternal(sxp);
}

SPtr AstTransform::applyInternal(const SPtr &sxp) {
  switch (sxp->type()) {
    case CLOSXP:
      return applyInternal(Cast<CLOSXP>(sxp));
    case LANGSXP:
      return applyInternal(Cast<LANGSXP>(sxp));
    case LISTSXP:
      return applyInternal(Cast<LISTSXP>(sxp));
    default:
      break;
  }
  return sxp;
}

SPtr AstTransform::applyInternal(const CloPtr &sxp) {
  return CloSxp::Create(
      sxp->formals(),
      applyInternal(sxp->body()),
      sxp->environment());
}

SPtr AstTransform::applyInternal(const LangPtr &sxp) {
  return LangSxp::Create(
      sxp->op(),
      Cast<LISTSXP>(applyInternal(sxp->arguments())));
}

SPtr AstTransform::applyInternal(const ListPtr &sxp) {
  std::vector<SPtr> new_values;
  new_values.reserve(sxp->size());
  for (const SPtr &value : sxp->values()) {
    new_values.emplace_back(applyInternal(value));
  }
  return ListSxp::Create(sxp->tags(), std::move(new_values));
}


SPtr LanguageToClosureTransform::applyInternal(const LangPtr &sxp) {
  const ListPtr new_args =
      Cast<LISTSXP>(AstTransform::applyInternal(sxp->arguments()));

  const SPtr op = sxp->op();
  if (op->type() == SYMSXP) {
    if (static_cast<const SymSxp*>(op.get())->name() == "function") {
      // TODO: figure out the proper environment.
      return CloSxp::Create(
          new_args->value(0), new_args->value(1), NilSxp::Create());
    }
  }

  return LangSxp::Create(op, new_args);
}


SPtr ControlTransform::applyInternal(const LangPtr &sxp) {
  DCHECK(sxp->op()->type() == SYMSXP);
  const SPtr op = sxp->op();
  const std::string &op_name = static_cast<const SymSxp*>(op.get())->name();
  const ListPtr new_args =
      Cast<LISTSXP>(AstTransform::applyInternal(sxp->arguments()));

  if (op_name == "for") {
    const ListPtr cond_args = ListSxp::CreateWithoutTags(
        std::vector<SPtr>({ new_args->value(0), new_args->value(1) }));
    const LangPtr cond = LangSxp::Create(
        SymSxp::Create("_$for_cond"), cond_args);

    const ListPtr for_args = ListSxp::CreateWithoutTags(
        std::vector<SPtr>({ cond, addBraces(new_args->value(2)) }));

    return LangSxp::Create(op, for_args);
  } else if (op_name == "if") {
    const ListPtr cond_args = ListSxp::CreateWithoutTags(
        std::vector<SPtr>({ new_args->value(0) }));
    const LangPtr cond = LangSxp::Create(
        SymSxp::Create("_$if_cond"), cond_args);

    std::vector<SPtr> if_args;
    if_args.emplace_back(cond);
    for (std::size_t i = 1; i < new_args->size(); ++i) {
      if_args.emplace_back(addBraces(new_args->value(i)));
    }

    return LangSxp::Create(op, ListSxp::CreateWithoutTags(std::move(if_args)));
  } else if (op_name == "while") {
    const ListPtr cond_args = ListSxp::CreateWithoutTags(
        std::vector<SPtr>({ new_args->value(0) }));
    const LangPtr cond = LangSxp::Create(
        SymSxp::Create("_$while_cond"), cond_args);

    const ListPtr while_args = ListSxp::CreateWithoutTags(
        std::vector<SPtr>({ cond, addBraces(new_args->value(1)) }));

    return LangSxp::Create(op, while_args);
  }

  return LangSxp::Create(op, new_args);
}

SPtr ControlTransform::addBraces(const SPtr &sxp) {
  if (sxp->type() == LANGSXP) {
    const SPtr op = static_cast<const LangSxp*>(sxp.get())->op();
    if (op->type() == SYMSXP &&
        static_cast<const SymSxp*>(op.get())->name() == "{") {
      return sxp;
    }
  }

  return LangSxp::Create(
      SymSxp::Create("{"),
      ListSxp::CreateWithoutTags(std::vector<SPtr>({ sxp })));
}


SPtr IfAssignTransform::applyInternal(const LangPtr &sxp) {
  DCHECK(sxp->op()->type() == SYMSXP);
  const SPtr op = sxp->op();
  const std::string &op_name = static_cast<const SymSxp*>(op.get())->name();
  const ListPtr new_args =
      Cast<LISTSXP>(AstTransform::applyInternal(sxp->arguments()));

  if (op_name == "<-" || op_name == "=") {
    const SPtr rhs = new_args->value(1);
    if (rhs->type() == LANGSXP) {
      const LangPtr lang = Cast<LANGSXP>(rhs);
      const SPtr op = lang->op();
      if (op->type() == SYMSXP && Cast<SYMSXP>(op)->name() == "if") {
        const ListPtr blocks = lang->arguments();
        const std::size_t m = blocks->size();

        std::vector<SPtr> trans_blocks;
        trans_blocks.emplace_back(blocks->value(0));

        for (std::size_t i = 1; i < m; ++i) {
          const LangPtr block_lang = Cast<LANGSXP>(blocks->value(i));
          const ListPtr block_stmts = block_lang->arguments();
          std::vector<SPtr> new_stmts = block_stmts->values();
          new_stmts.back() =
              makeAssignment(new_args->value(0), block_stmts->values().back());
          trans_blocks.emplace_back(
              LangSxp::Create(
                  block_lang->op(),
                  ListSxp::CreateWithoutTags(std::move(new_stmts))));
        }

        if (m < 3) {
          DCHECK(m == 2);
          const ListPtr block_stmts =
              ListSxp::CreateWithoutTags(std::vector<SPtr>(
                  { makeAssignment(new_args->value(0), NilSxp::Create()) }));
          trans_blocks.emplace_back(
              LangSxp::Create(SymSxp::Create("{"), block_stmts));
        }

        return LangSxp::Create(op, ListSxp::CreateWithoutTags(trans_blocks));
      }
    }
  }
  return LangSxp::Create(op, new_args);
}

SPtr IfAssignTransform::makeAssignment(const SPtr &sym, const SPtr &value) {
  return LangSxp::Create(
      SymSxp::Create("<-"),
      ListSxp::CreateWithoutTags(std::vector<SPtr>({ sym, value})));
}

}  // namespace rosa
