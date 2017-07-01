#ifndef ROSA_REACHING_DEFINITION_HPP_
#define ROSA_REACHING_DEFINITION_HPP_

#include <map>
#include <memory>
#include <set>
#include <string>

#include "Cfg.hpp"
#include "CfgAnalysis.hpp"
#include "Macros.hpp"
#include "VariableFacts.hpp"

namespace rosa {

class ReachingDefinitionFact;
typedef std::shared_ptr<const ReachingDefinitionFact> ReachingDefinitionFactPtr;

class ReachingDefinitionFact : public CfgFact {
 public:
  ReachingDefinitionFact() {}

  CfgFactPtr merge(const CfgFactPtr &others) override;

  bool equals(const CfgFactPtr &other) override;

  std::string toString() const override;

  inline const std::map<std::string, std::set<CfgNodePtr>>& locations() const {
    return locations_;
  }

 private:
  std::map<std::string, std::set<CfgNodePtr>> locations_;

  friend class ReachingDefinition;

  DISALLOW_COPY_AND_ASSIGN(ReachingDefinitionFact);
};

class ReachingDefinition : public CfgAnalysis {
public:
  ReachingDefinition(const CfgPtr &cfg,
                     const CloPtr &clo,
                     const VariableFacts &vf);

  inline ReachingDefinitionFactPtr getBefore(const CfgNodePtr &node) const {
    const auto &it = before_.find(node);
    DCHECK(it != before_.end());
    return std::static_pointer_cast<const ReachingDefinitionFact>(it->second);
  }

  inline ReachingDefinitionFactPtr getAfter(const CfgNodePtr &node) const {
    const auto &it = after_.find(node);
    DCHECK(it != after_.end());
    return std::static_pointer_cast<const ReachingDefinitionFact>(it->second);
  }

protected:
  CfgFactPtr initialFact() override;
  CfgFactPtr updateFact(const CfgFactPtr &old, const CfgNodePtr &node) override;

private:
  const CloPtr clo_;
  const VariableFacts &vf_;

  DISALLOW_COPY_AND_ASSIGN(ReachingDefinition);
};

}  // namespace rosa

#endif  // ROSA_REACHING_DEFINITION_HPP_
