#include "DebugUtil.hpp"

#include <sstream>

#include "Ast.hpp"
#include "AstPrinter.hpp"
#include "Cfg.hpp"
#include "ReachingDefinition.hpp"
#include "TemplateUtil.hpp"
#include "VariableFacts.hpp"

namespace rosa {

std::string CfgPrinter::ToString(const SPtr &input, const VariableFacts &vf) {
  std::ostringstream out;
  Visit(input, vf, out);
  return out.str();
}

void CfgPrinter::Visit(
    const SPtr &sxp, const VariableFacts &vf, std::ostringstream &out) {
  switch (sxp->type()) {
    case CLOSXP: {
      Visit(Cast<CLOSXP>(sxp), vf, out);
      break;
    }
    case LANGSXP: {
      for (const auto &node : Cast<LANGSXP>(sxp)->arguments()->values()) {
        Visit(node, vf, out);
      }
      break;
    }
    case LISTSXP: {
      for (const auto &node : Cast<LISTSXP>(sxp)->values()) {
        Visit(node, vf, out);
      }
      break;
    }
    default:
      break;
  }
}

void CfgPrinter::Visit(
    const CloPtr &sxp, const VariableFacts &vf, std::ostringstream &out) {
  const CfgPtr cfg = CfgCatalog::Instance().getCfg(sxp);

  out << "******** CFG ********\n";
  out << cfg->toString() << "\n";

  out << "******** USE DEF ********\n";
  for (const auto &node : cfg->nodes()) {
    const SPtr sp = node->sp;
    if (sp) {
      out << "[" << node->id << "] use:{";
      if (vf.use().find(sp) != vf.use().end()) {
        for (const auto &var : vf.use().at(sp)) {
          out << " " << var;
        }
      }
      out << " } def:{";
      if (vf.def().find(sp) != vf.def().end()) {
        for (const auto &var : vf.def().at(sp)) {
          out << " " << var;
        }
      }
      out << " }\n";
    }
  }

  // Defined values
  out << "\n******** VALUES ********\n";
  for (const auto &node : cfg->nodes()) {
    const SPtr sp = node->sp;
    if (sp) {
      out << "[" << node->id << "]";
      if (vf.values().find(sp) != vf.values().end()) {
        for (const auto &it : vf.values().at(sp)) {
          out << "  " << it.first << "{";
          for (const auto &p : it.second) {
            out << " " << ToCompactString(p);
          }
          out << " }";
        }
      }
      out << "\n";
    }
  }

  // Reaching def
  out << "\n******** REACHING ********\n";
  ReachingDefinition rd(cfg, sxp, vf);
  for (const auto &it : rd.before()) {
    out << "[" << it.first->id << "]" << it.second->toString() << "\n";
  }
  out << "\n";

  Visit(sxp->body(), vf, out);
}


std::string TypePrinter::ToString(const TypeInference &ti) {
  std::ostringstream out;

  CfgCatalog &catalog = CfgCatalog::Instance();
  for (const auto &it : catalog.mapping()) {
    out << "---------------- CLOSURE ----------------- \n"
        << ToCompactString(it.first) << "\n";

    out << "******** SEXP TYPES ********\n";
    const CfgPtr &cfg = it.second;
    for (const auto &node : cfg->nodes()) {
      if (node->sp) {
        out << "[" << node->id << "] " << ToCompactString(node->sp) << "\n";
        Visit(node->sp, ti, out);
      }
    }
    out << "\n\n";

    out << "******** RETURN TYPES ********\n";
    DCHECK(ContainsKey(ti.rtn_types(), it.first));
    const auto &rtn_types = ti.rtn_types().at(it.first);
    for (const auto &it : rtn_types) {
      out << ">> signature: ";
      PrintSignature(it.first, out);
      out << "\n>> return type: " << it.second->getName() << "\n";
    }

    out << "\n******** DEF TYPES ********\n";
    for (const auto &node : cfg->nodes()) {
      out << "[" << node->id << "]";
      const auto &dfit = ti.def_types().find(node);
      if (dfit != ti.def_types().end()) {
        for (const auto &ts : dfit->second) {
          out << " " << ts.first;
          bool first = true;
          for (const auto &t : ts.second) {
            if (first) {
              first = false;
            } else {
              out << ", ";
            }
            if (ts.second.size() > 1) {
              out << " ";
              PrintSignature(t.first, out);
            }
            out << ": " << t.second->getName();
          }
        }
      }
      out << "\n";
    }

    out << "\n********  DECL TYPES ********\n";
    DCHECK(ContainsKey(ti.decl_types(), it.first));
    const auto &decl_types = ti.decl_types().at(it.first);
    for (const auto &ts : decl_types) {
      out << ts.first;
      for (const auto &t : ts.second) {
        if (ts.second.size() > 1) {
          out << " ";
          PrintSignature(t.first, out);
        }
        out << ": " << t.second->getName();
      }
      out << "\n";
    }
    out << "\n";
  }

  return out.str();
}

void TypePrinter::Visit(const SPtr &sxp,
                        const TypeInference &ti,
                        std::ostringstream &out) {
  out << ToCompactString(sxp) << "\n";

  const auto &it = ti.expr_types().find(sxp);
  if (it == ti.expr_types().end()) {
    out << "???\n";
  } else {
    for (const auto &t : it->second) {
      out << ">> ";
      if (it->second.size() > 1) {
        PrintSignature(t.first, out);
        out << " ";
      }
      out << "type: " << t.second->getName() << "\n";
    }
  }

  switch (sxp->type()) {
    case LANGSXP: {
      for (const auto &node : Cast<LANGSXP>(sxp)->arguments()->values()) {
        Visit(node, ti, out);
      }
      break;
    }
    case LISTSXP: {
      for (const auto &node : Cast<LISTSXP>(sxp)->values()) {
        Visit(node, ti, out);
      }
      break;
    }
    default:
      break;
  }
}

void TypePrinter::PrintSignature(const FnSignature &sig, std::ostringstream &out) {
  out << "(";
  bool first = true;
  for (const auto &it : sig) {
    if (first) {
      first = false;
    } else {
      out << ", ";
    }
    out << it.second->getName() << " " << it.first;
  }
  out << ")";
}

}  // namespace rosa
