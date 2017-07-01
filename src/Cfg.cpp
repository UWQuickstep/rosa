#include "Cfg.hpp"

#include <cstddef>
#include <memory>
#include <set>
#include <sstream>
#include <string>

#include "Ast.hpp"
#include "AstPrinter.hpp"

#include <Rcpp.h>

namespace rosa {

std::size_t CfgNode::idc = 0;

void Cfg::contract() {
  std::set<CfgNodePtr> nodes;
  for (const auto &node : nodes_) {
    bool erase = false;
    if (node != enter_node_ && node != exit_node_ && !node->sp) {
      const auto &pred = pred_[node];
      const auto &succ = succ_[node];
      if (pred.size() <= 1 || succ.size() <= 1) {
        // re-link
        for (const auto &pn: pred) {
          for (const auto &sn : succ) {
            succ_[pn].erase(node);
            succ_[pn].emplace(sn);
            pred_[sn].erase(node);
            pred_[sn].emplace(pn);
          }
        }
        erase = true;
      }
    }
    if (erase) {
      pred_.erase(node);
      succ_.erase(node);
    } else {
      nodes.emplace(node);
    }
  }
  nodes_ = std::move(nodes);
}

std::string Cfg::toString() {
  std::ostringstream out;
  out << "-- Nodes --\n";
  for (const auto &node : nodes_) {
    out << "[" << node->id << "]\n";
    if (node->sp) {
      out << ToCompactString(node->sp) << "\n";
    } else {
      out << "<empty>\n";
    }
  }
  out << "\n-- Edges --\n";
  for (const auto &it : succ_) {
    out << it.first->id << " ->";
    for (const auto &node : it.second) {
      out << " " << node->id;
    }
    out << "\n";
  }
  return out.str();
}


CfgBuilder::CfgBuilder(const CloPtr &input) {
  visit(input);
}

void CfgBuilder::visit(const SPtr &sxp) {
  switch (sxp->type()) {
    case CLOSXP: {
      visit(Cast<CLOSXP>(sxp));
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
    default:
      break;
  }
}

void CfgBuilder::visit(const CloPtr &sxp) {
  cfg_ = std::make_shared<Cfg>();

  cfg_->enterNode() = cfg_->createNode();
  cfg_->exitNode() = cfg_->createNode();

  node_ptr_ = cfg_->createNode();
  cfg_->addEdge(cfg_->enterNode(), node_ptr_);
  visit(sxp->body());
  cfg_->addEdge(node_ptr_, cfg_->exitNode());

  cfg_->contract();

  for (const auto &node : cfg_->nodes()) {
    if (node->sp) {
      SubexpCollector sc(node->sp);
      for (const auto &exp : sc.exps) {
        cfg_->addMap(exp, node);
      }
    }
  }
}

void CfgBuilder::visit(const LangPtr &sxp) {
  if (sxp->op()->type() != SYMSXP) {
    FATAL_ERROR("Cannot handle high order function calls!\n");
  }

  std::string op_name = Cast<SYMSXP>(sxp->op())->name();
  const std::vector<SPtr> &args = sxp->arguments()->values();
  // Deal with control functors
  if (op_name == "if") {
    // condition
    node_ptr_->sp = args[0];
    CfgNodePtr cond_exit = node_ptr_;
    // then-branch
    node_ptr_ = cfg_->createNode();
    cfg_->addEdge(cond_exit, node_ptr_);
    visit(args[1]);
    CfgNodePtr t_br_exit = node_ptr_;
    // else-branch
    if (args.size() == 2) {
      cfg_->addEdge(cond_exit, t_br_exit);
    } else {
      node_ptr_ = cfg_->createNode();
      cfg_->addEdge(cond_exit, node_ptr_);
      visit(args[2]);
      cfg_->addEdge(t_br_exit, node_ptr_);
    }
  } else if (op_name == "while" || op_name == "for") {
    pushLoopEnv();
    // condition
    node_ptr_->sp = args[0];
    cont_ptr_ = node_ptr_;
    exit_ptr_ = cfg_->createNode();
    cfg_->addEdge(cont_ptr_, exit_ptr_);
    // body
    node_ptr_ = cfg_->createNode();
    cfg_->addEdge(cont_ptr_, node_ptr_);
    visit(args[1]);
    cfg_->addEdge(node_ptr_, cont_ptr_);
    // exit
    node_ptr_ = exit_ptr_;
    popLoopEnv();
  } else if (op_name == "return") {
    node_ptr_->sp = sxp;
    cfg_->addEdge(node_ptr_, cfg_->exitNode());
    node_ptr_ = cfg_->createNode();
  } else if (op_name == "break") {
    cfg_->addEdge(node_ptr_, exit_ptr_);
    node_ptr_ = cfg_->createNode();
  } else if (op_name == "next") {
    cfg_->addEdge(node_ptr_, cont_ptr_);
    node_ptr_ = cfg_->createNode();
  } else if (op_name == "{") {
    visit(sxp->arguments());
  } else {
    node_ptr_->sp = sxp;
    CfgNodePtr new_node = cfg_->createNode();
    cfg_->addEdge(node_ptr_, new_node);
    node_ptr_ = new_node;
  }
}

void CfgBuilder::visit(const ListPtr &sxp) {
  for (const auto &child : sxp->values()) {
    if (child->type() == LANGSXP) {
      visit(child);
    } else {
      node_ptr_->sp = child;
      CfgNodePtr new_node = cfg_->createNode();
      cfg_->addEdge(node_ptr_, new_node);
      node_ptr_ = new_node;
    }
  }
}

CfgBuilder::SubexpCollector::SubexpCollector(const SPtr &sxp) {
  visit(sxp);
}

void CfgBuilder::SubexpCollector::visit(const SPtr &sxp) {
  exps.emplace(sxp);

  switch (sxp->type()) {
    case CLOSXP: {
      visit(Cast<CLOSXP>(sxp)->body());
      break;
    }
    case LANGSXP: {
      for (const auto &node : Cast<LANGSXP>(sxp)->arguments()->values()) {
        visit(node);
      }
      break;
    }
    case LISTSXP: {
      for (const auto &node : Cast<LISTSXP>(sxp)->values()) {
        visit(node);
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace rosa;
