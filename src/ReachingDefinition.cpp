#include "ReachingDefinition.hpp"

#include <memory>
#include <sstream>
#include <string>

#include "Cfg.hpp"
#include "Macros.hpp"
#include "TemplateUtil.hpp"

namespace rosa {

CfgFactPtr ReachingDefinitionFact::merge(const CfgFactPtr &other) {
  std::unique_ptr<ReachingDefinitionFact> new_fact =
      std::make_unique<ReachingDefinitionFact>();
  auto &locations = new_fact->locations_;

  for (const auto &it : locations_) {
    locations[it.first].insert(it.second.begin(), it.second.end());
  }
  if (other) {
    const ReachingDefinitionFact *other_fact =
        static_cast<const ReachingDefinitionFact*>(other.get());
    for (const auto &it : other_fact->locations()) {
      locations[it.first].insert(it.second.begin(), it.second.end());
    }
  }

  return CfgFactPtr(new_fact.release());
}

bool ReachingDefinitionFact::equals(const CfgFactPtr &other) {
  const ReachingDefinitionFact *other_fact =
      static_cast<const ReachingDefinitionFact*>(other.get());
  return locations_ == other_fact->locations_;
}

std::string ReachingDefinitionFact::toString() const {
  std::ostringstream out;
  for (const auto &dl : locations_) {
    out << " " << dl.first << "{";
    bool first = true;
    for (const auto &lc : dl.second) {
      out << (first ? "" : ",") << lc->id;
      first = false;
    }
    out << "}";
  }
  return out.str();
}

ReachingDefinition::ReachingDefinition(const CfgPtr &cfg,
                                       const CloPtr &clo,
                                       const VariableFacts &vf)
    : CfgAnalysis(cfg, CfgAnalysis::kForward),
      clo_(clo), vf_(vf) {
  CfgAnalysis::execute();
}

CfgFactPtr ReachingDefinition::initialFact() {
  return std::make_shared<ReachingDefinitionFact>();
}

CfgFactPtr ReachingDefinition::updateFact(
    const CfgFactPtr &old, const CfgNodePtr &node) {
  std::unique_ptr<ReachingDefinitionFact> new_fact =
      std::make_unique<ReachingDefinitionFact>();
  new_fact->locations_ =
      static_cast<const ReachingDefinitionFact*>(old.get())->locations_;

  if (node == cfg_->enterNode()) {
    for (const auto &arg_name : clo_->formals()->tags()) {
      DCHECK(arg_name->type() == SYMSXP);
      new_fact->locations_[Cast<SYMSXP>(arg_name)->name()] = { node };
    }
  } else if (node->sp) {
    const auto it = vf_.def().find(node->sp);
    if (it != vf_.def().end()) {
      for (const auto &var : it->second) {
        new_fact->locations_[var] = { node };
      }
    }
  }
  return CfgFactPtr(new_fact.release());
}

}  // namespace rosa
