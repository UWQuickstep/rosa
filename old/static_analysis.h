#ifndef _STATIC_ANALYSIS_H_
#define _STATIC_ANALYSIS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <memory>

#define USE_RINTERNALS

#include "../src/include/Rinternals.h"
#include "../src/include/config.h"
#include "../src/include/Defn.h"

#define MAX_STMTS 8192
#define MAX_LABEL_LENGTH 1024
#define MAX_VARS_PER_FN 1024

using namespace std;

typedef set<int> alias_t;

struct stmt_t {
	int stmt_id;
	SEXP expr;
	bool is_discont;
	bool is_fn_call;
	bool is_conditioned;
	bool is_hoisted;
	bool is_loop_header;
	set<int> pred;
	set<int> succ;
	set<int> kill;
	set<int> gen;
	set<int> single_gen;
	set<int> live_in;
	set<int> live_out;
	set<alias_t> gen_alias;
	set<alias_t> alias_in;
	set<alias_t> alias_out;
	vector<SEXP> agg_expr;
	map<int,set<int> > reachdef_in;
	map<int,set<int> > reachdef_out;
	map<int,SEXP> single_gen_map;
	int fn_id;
	vector<int> loops;
	char start_tag[100];
	char end_tag[100];
};

#define ADD_USE(s,x,e,this_use_id) {				\
	if(force_def) {									\
		ADD_DEF(s,x,force_def_super,this_use_id)	\
	}												\
	else {											\
		int i;										\
		int cnt=s->gen.size();						\
		int id=find_var_id(x);						\
		s->gen.insert(id);							\
		if(s->gen.size()!=cnt) {					\
			s->single_gen.insert(id);				\
			s->single_gen_map[id]=e;				\
		}											\
		else {										\
			s->single_gen.erase(id);				\
			s->single_gen_map.erase(id);			\
		}											\
		this_use_id=id;								\
	}												\
}

#define ADD_DEF(s,x,super,this_def_id) {			\
	int id;											\
	if(!super) {									\
		id=add_var(x);								\
	}												\
	else {											\
		int t=current_fn_id;						\
		current_fn_id=function_list[current_fn_id].parent_id;	\
		id=find_var_id(x);							\
		current_fn_id=t;							\
	}												\
	s->kill.insert(id);								\
	this_def_id=id;									\
}

#define ADD_AGGEXPR(s,e) {							\
	s->agg_expr.push_back(e);						\
}

#define ADD_EDGE_FORCE(p,s) {						\
	stmt_list[p].succ.insert(s);					\
	stmt_list[s].pred.insert(p);					\
}

#define ADD_EDGE(p,s) {								\
	if(!stmt_list[p].is_discont) {					\
		ADD_EDGE_FORCE(p,s)							\
	}												\
}

#define REMOVE_EDGE_FORCE(p,s){						\
	stmt_list[p].succ.erase(s);						\
	stmt_list[s].pred.erase(p);						\
}

#define ADD_EXITS(x,y,z,w) {						\
	if(y) {											\
		int xi;										\
		for(xi=0;xi<y;xi++) {						\
			ADD_EDGE_FORCE(x[xi],z)					\
		}											\
	}												\
	else {											\
		ADD_EDGE(w,z);								\
	}												\
}

#define ADD_CONTS(x,y,z) {							\
	if(y) {											\
		int xi;										\
		for(xi=0;xi<y;xi++) {						\
			ADD_EDGE_FORCE(x[xi],z)					\
		}											\
	}												\
}

#define ADD_EDGE_ALL_SUCC_FORCE(p,n,f) {				\
	set<int>::iterator s_iter;						\
	for(s_iter=stmt_list[n].succ.begin();s_iter!=stmt_list[n].succ.end();s_iter++) {	\
		if(stmt_list[(*s_iter)].fn_id!=f) continue;	\
		ADD_EDGE_FORCE(p,(*s_iter))					\
	}												\
}

#define REMOVE_ALL_EDGE_SUCC(n,f) {					\
	set<int>::iterator s_iter,tmp;					\
	for(s_iter=stmt_list[n].succ.begin();s_iter!=stmt_list[n].succ.end();) {	\
		tmp=s_iter;									\
		tmp++;										\
		if(stmt_list[(*s_iter)].fn_id==f) {			\
			REMOVE_EDGE_FORCE(n,(*s_iter))			\
		}											\
		s_iter=tmp;									\
	}												\
}

#define ADD_ALIAS(s,x,y) {							\
	alias_t t;										\
	t.insert(find_var_id(x));						\
	t.insert(find_var_id(y));						\
	s->gen_alias=union_alias(s->gen_alias,t);		\
}

#define SET_CONDITIONED(start,end) {				\
	for(int i=start;i<=end;i++)	{					\
		stmt_list[i].is_conditioned=true;			\
	}												\
}

struct function_t {
	string name;
	string orig_name;
	SEXP formals;
	SEXP body;
	int entry_node;
	int exit_node;
	int parent_id;
	map<const char *,int> vars;	// maps from actual name to global ids
};

struct call_t {
	int node_id;
	int fn_id;
	int var_id;
};

struct loop_t{
	int start_node;
	int end_node;
	int ind_var_id;
	bool is_for_loop;
	set<int> hoisted_stmts;
	set<int> invariants;
};

bool is_known_function(const char *name);

#endif

