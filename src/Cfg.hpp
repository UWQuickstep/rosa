#ifndef ROSA_CFG_HPP_
#define ROSA_CFG_HPP_

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <vector>

#include "Ast.hpp"

namespace rosa {

class CfgNode;
class Cfg;

typedef std::shared_ptr<CfgNode> CfgNodePtr;
typedef std::shared_ptr<Cfg> CfgPtr;

class CfgNode {
public:
  inline CfgNode() : id(++idc) {}

  std::size_t id;
  SPtr sp;

  static std::size_t idc;
};

class Cfg {
 public:
  Cfg() {}

  inline CfgNodePtr& enterNode() {
    return enter_node_;
  }

  inline CfgNodePtr& exitNode() {
    return exit_node_;
  }

  inline const std::set<CfgNodePtr> &nodes() const {
    return nodes_;
  }

  inline const std::map<CfgNodePtr, std::set<CfgNodePtr>>& pred() const {
    return pred_;
  }
  inline const std::map<CfgNodePtr, std::set<CfgNodePtr>>& succ() const {
    return succ_;
  }

  inline const std::map<SPtr, CfgNodePtr>& mapping() const {
    return mapping_;
  }

  std::string toString();

 private:
  inline CfgNodePtr createNode() {
    CfgNodePtr node = std::make_shared<CfgNode>();
    nodes_.emplace(node);
    return node;
  }

  inline void addEdge(const CfgNodePtr &src, const CfgNodePtr &dst) {
    succ_[src].emplace(dst);
    pred_[dst].emplace(src);
  }

  inline void addMap(const SPtr &sxp, const CfgNodePtr &node) {
    mapping_[sxp] = node;
  }

  void contract();

  std::set<CfgNodePtr> nodes_;
  std::map<CfgNodePtr, std::set<CfgNodePtr>> pred_;
  std::map<CfgNodePtr, std::set<CfgNodePtr>> succ_;
  std::map<SPtr, CfgNodePtr> mapping_;

  // Enter node is an empty node
  CfgNodePtr enter_node_;
  CfgNodePtr exit_node_;

  friend class CfgBuilder;

  DISALLOW_COPY_AND_ASSIGN(Cfg);
};


class CfgBuilder {
 public:
  CfgBuilder(const CloPtr &input);

  inline const CfgPtr cfg() const {
    return cfg_;
  }

 private:
  struct SubexpCollector {
    SubexpCollector(const SPtr &sxp);
    void visit(const SPtr &sxp);
    std::set<SPtr> exps;
  };

  inline void pushLoopEnv() {
    loop_env_.push(cont_ptr_);
    loop_env_.push(exit_ptr_);
  }

  inline void popLoopEnv() {
    exit_ptr_ = loop_env_.top();
    loop_env_.pop();
    cont_ptr_ = loop_env_.top();
    loop_env_.pop();
  }

  void visit(const SPtr &sxp);
  void visit(const CloPtr &sxp);
  void visit(const LangPtr &sxp);
  void visit(const ListPtr &sxp);

  CfgPtr cfg_;
  CfgNodePtr node_ptr_;
  CfgNodePtr cont_ptr_;
  CfgNodePtr exit_ptr_;
  std::stack<CfgNodePtr> loop_env_;
};


class CfgCatalog {
 public:
  CfgCatalog() {}

  inline const std::map<CloPtr, CfgPtr>& mapping() const {
    return mapping_;
  }

  inline CfgPtr getCfg(CloPtr clo) {
    auto it = mapping_.find(clo);
    if (it == mapping_.end()) {
      it = mapping_.emplace(clo, CfgBuilder(clo).cfg()).first;
    }
    return it->second;
  }

  inline static CfgCatalog& Instance() {
    static CfgCatalog instance;
    return instance;
  }

 private:
  std::vector<CfgPtr> cfgs_;
  std::map<CloPtr, CfgPtr> mapping_;

  DISALLOW_COPY_AND_ASSIGN(CfgCatalog);
};

}  // namespace rosa

#endif  // ROSA_CFG_HPP_
