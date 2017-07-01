#ifndef ROSA_CFG_ANALYSIS_HPP_
#define ROSA_CFG_ANALYSIS_HPP_

#include <map>
#include <memory>
#include <stack>
#include <string>

#include "Cfg.hpp"
#include "Macros.hpp"

namespace rosa {

class CfgFact;
typedef std::shared_ptr<CfgFact> CfgFactPtr;

class CfgFact {
 public:
  virtual ~CfgFact() {}
  virtual CfgFactPtr merge(const CfgFactPtr &other) = 0;
  virtual bool equals(const CfgFactPtr &other) = 0;
  virtual std::string toString() const = 0;
};

class CfgAnalysis {
 public:
  enum Direction {
    kForward,
    kBackward
  };

  CfgAnalysis(const CfgPtr &cfg, const Direction dir)
      : cfg_(cfg), dir_(dir) {}

  virtual ~CfgAnalysis() {}

  inline const std::map<CfgNodePtr, CfgFactPtr>& before() const {
    return before_;
  }

  inline const std::map<CfgNodePtr, CfgFactPtr>& after() const {
    return after_;
  }

  inline void execute() {
    if (dir_ == kForward) {
      analyze<kForward>(cfg_);
    } else {
      analyze<kBackward>(cfg_);
    }
  }

 protected:
  virtual CfgFactPtr initialFact() = 0;
  virtual CfgFactPtr updateFact(const CfgFactPtr &old, const CfgNodePtr &node) = 0;

  std::map<CfgNodePtr, CfgFactPtr> before_;
  std::map<CfgNodePtr, CfgFactPtr> after_;

  const CfgPtr cfg_;
  const Direction dir_;

 private:
  template <Direction dir>
  inline void analyze(const CfgPtr &cfg) {
    std::stack<CfgNodePtr> nodes;

    const CfgFactPtr init_fact = initialFact();
    for (const auto &node : cfg->nodes()) {
      nodes.push(node);
      if (dir == kForward) {
        before_[node] = init_fact;
        after_[node] = updateFact(init_fact, node);
      } else {
        after_[node] = init_fact;
        before_[node] = updateFact(init_fact, node);
      }
    }
    nodes.push(cfg->enterNode());

    while (!nodes.empty()) {
      const CfgNodePtr node = nodes.top();
      nodes.pop();

      if (dir == kForward) {
        CfgFactPtr fact_before = init_fact;
        const auto &it = cfg->pred().find(node);
        if (it != cfg->pred().end()) {
          for (const auto &node : it->second) {
            fact_before = fact_before->merge(after_[node]);
          }
        }
        if (!fact_before->equals(before_[node])) {
          before_[node] = fact_before;
          after_[node] = updateFact(fact_before, node);
          const auto &it = cfg->succ().find(node);
          if (it != cfg->succ().end()) {
            for (const auto &node : it->second) {
              nodes.push(node);
            }
          }
        }
      } else {
        CfgFactPtr fact_after = init_fact;
        const auto &it = cfg->succ().find(node);
        if (it != cfg->succ().end()) {
          for (const auto &node : it->second) {
            fact_after = fact_after->merge(before_[node]);
          }
        }
        if (!fact_after->equals(after_[node])) {
          after_[node] = fact_after;
          before_[node] = updateFact(fact_after, node);
          const auto &it = cfg->pred().find(node);
          if (it != cfg->pred().end()) {
            for (const auto &node : it->second) {
              nodes.push(node);
            }
          }
        }
      }
    }
  }

  DISALLOW_COPY_AND_ASSIGN(CfgAnalysis);
};

}  // namespace rosa

#endif  // ROSA_CFG_ANALYSIS_HPP_
