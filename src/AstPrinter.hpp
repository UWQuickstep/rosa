#ifndef ROSA_AST_PRINTER_HPP_
#define ROSA_AST_PRINTER_HPP_

#include <memory>
#include <sstream>
#include <stack>
#include <string>

#include "Ast.hpp"
#include "Macros.hpp"
#include "TemplateUtil.hpp"

namespace rosa {

enum class AstPrintFormat {
  kCompact,
  kNormal
};

/**
 * @brief Customized pretty printing of a SEXP.
 */
template <AstPrintFormat format = AstPrintFormat::kNormal>
class AstPrinter {
 public:
  static std::string ToString(const SPtr &sxp) {
    State state;
    ToStringDispatch(sxp, &state);
    return state.out.str();
  }

 private:
  struct State {
    State() : level(-1) {
      separator.emplace('\n');
    }
    inline void indent() {
//      out << '-' << level / 10 << level % 10 << '-';
      out << std::string(level, ' ');
    }

    int level;
    std::stack<char> separator;
    std::ostringstream out;
  };

  // Dispatcher.
  inline static void ToStringDispatch(const SPtr &sxp, State *state) {
    ++state->level;    
    InvokeOnSPtr(sxp, [state](const auto &sxp) {
        AstPrinter::ToStringInternal(sxp, state);
    });
    --state->level;
  }

  inline static void ToStringInternal(const NilPtr &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "]\n";
    } else {
      state->out << "nil";
    }
  }

  inline static void ToStringInternal(const SymPtr &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "] " << sxp->name() << "\n";
    } else {
      state->out << sxp->name();
    }
  }

  inline static void ToStringInternal(const ListPtr &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      for (std::size_t i = 0; i < sxp->size(); ++i) {
        SPtr tag = sxp->tag(i);
        if (tag->type() == SYMSXP) {
          state->indent();
          state->out << "--TAG-- "
                     << static_cast<const SymSxp*>(tag.get())->name() << "\n";
        }
        ToStringDispatch(sxp->value(i), state);
      }
    } else {
      for (int i = 0; i < sxp->size(); i++) {
        if (state->separator.top() == '\n') {
          state->indent();
        }

        SPtr tag = sxp->tag(i);
        if (tag->type() == SYMSXP) {
          state->out << static_cast<const SymSxp*>(tag.get())->name() << ":";
        }
        ToStringDispatch(sxp->value(i), state);
        if (state->separator.top() == '\n' || i != sxp->size() - 1) {
          state->out << state->separator.top();
        }
      }
    }
  }

  inline static void ToStringInternal(const CloPtr &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "]\n";
      state->indent();
      state->out << "--FORMALS--\n";
      ToStringDispatch(sxp->formals(), state);
      state->indent();
      state->out << "--BODY--\n";
      ToStringDispatch(sxp->body(), state);
      state->indent();
      state->out << "--ENV--\n";
      ToStringDispatch(sxp->environment(), state);
    } else {
      state->indent();
      state->out << "function(";
      state->separator.push(' ');
      ToStringDispatch(sxp->formals(), state);
      state->separator.pop();
      state->out << ")\n";
      state->indent();
      ToStringDispatch(sxp->body(), state);
    }
  }

  inline static void ToStringInternal(const LangPtr &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "]\n";
      state->indent();
      state->out << "--OP--\n";
      ToStringDispatch(sxp->op(), state);
      state->indent();
      state->out << "--ARGS--\n";
      ToStringDispatch(sxp->arguments(), state);
    } else {
      bool sp = false;
      if (sxp->op()->type() == SYMSXP) {
        const std::string &op_name =
            static_cast<const SymSxp*>(sxp->op().get())->name();
        if (op_name == "{") {
          state->out << "\n";
          state->separator.push('\n');
          ToStringDispatch(sxp->arguments(), state);
          sp = true;
        } else if (op_name == "while" || op_name == "if" || op_name == "for") {
          ToStringDispatch(sxp->op(), state);
          state->out << " ";
          state->separator.push(' ');
          ToStringDispatch(sxp->arguments(), state);
          sp = true;
        }
      }
      if (!sp) {
        ToStringDispatch(sxp->op(), state);
        state->out << "(";
        state->separator.push(' ');
        ToStringDispatch(sxp->arguments(), state);
        state->out << ")";
      }
      state->separator.pop();
    }
  }

  template <typename ValueT, typename VectorT>
  inline static void VectorToString(const VectorT &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "]";
      for (const ValueT &v : sxp->values()) {
        state->out << " " << v;
      }
      state->out << "\n";
    } else {
      int c = 0;
      for (const ValueT &v : sxp->values()) {
        state->out << ((c++ == 0) ? "" : " ") << v;
      }
    }
  }

  inline static void ToStringInternal(const LglPtr &sxp, State *state) {
    VectorToString<bool>(sxp, state);
  }

  inline static void ToStringInternal(const IntPtr &sxp, State *state) {
    VectorToString<int>(sxp, state);
  }

  inline static void ToStringInternal(const RealPtr &sxp, State *state) {
    VectorToString<double>(sxp, state);
  }

  inline static void ToStringInternal(const StrPtr &sxp, State *state) {
    VectorToString<std::string>(sxp, state);
  }

  inline static void ToStringInternal(const VecPtr &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "]";
    }
    for (const auto &v : sxp->values()) {
      ToStringDispatch(v, state);
    }
  }

  // Generic cases.
  template <typename T>
  inline static void ToStringInternal(const T &sxp, State *state) {
    if (format == AstPrintFormat::kNormal) {
      state->indent();
      state->out << "[" << sxp->type_name() << "] ...\n";
    } else {
      state->out << "[" << sxp->type_name() << "] ...";
    }
  }

  AstPrinter() {}

  DISALLOW_COPY_AND_ASSIGN(AstPrinter);
};

inline std::string ToString(const SPtr &sxp) {
  return AstPrinter<AstPrintFormat::kNormal>::ToString(sxp);
}

inline std::string ToCompactString(const SPtr &sxp) {
  return AstPrinter<AstPrintFormat::kCompact>::ToString(sxp);
}

}  // namespace rosa

#endif  // ROSA_AST_PRINTER_HPP_
