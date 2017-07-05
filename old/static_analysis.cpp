//#include <R.h>
//#include <Rinternals.h>

#include "static_analysis.h"

map<int,const char*> rvars;

int current_fn_id;
int current_fn_ctr;

struct stmt_t stmt_list[MAX_STMTS];
int global_stmt_cnt;

vector<function_t> function_list;
vector<call_t> call_list;
vector<loop_t> loop_list;

char labels[MAX_STMTS][MAX_LABEL_LENGTH];
bool force_def=false;
bool force_def_super=false;

map<int,SEXP> candidate_expr_map;

FILE *codegen_fp;

#define FOR_EACH(it, c) for (auto it = c.begin(); it != c.end(); it++)

inline int add_var(const char *v) {
	map<const char*,int>::iterator it=function_list[current_fn_id].vars.find(v);
	if(it==function_list[current_fn_id].vars.end()) {
		int id=function_list[current_fn_id].vars.size()+current_fn_id*MAX_VARS_PER_FN;
		function_list[current_fn_id].vars[v]=id;
		rvars[id]=v;
		return id;
	}
	return it->second;
}

inline int find_var_id(const char *v) {
	int fn_id=current_fn_id;
	while(fn_id!=-1) {
		map<const char*,int>::iterator it=function_list[fn_id].vars.find(v);
		if(it==function_list[fn_id].vars.end()) {
			fn_id=function_list[fn_id].parent_id;
			continue;
		}
		return it->second;
	}
	// assume a correct program (simplifies handling for formal args to functions)	
	if(!is_known_function(v))
		return add_var(v);
	return -1;
}

void print_alias_set(set<alias_t> s) {
	set<alias_t>::iterator it1;
	set<int>::iterator it;
	for(it1=s.begin();it1!=s.end();it1++) {
		if(!(*it1).size()) continue;
		printf("{");
		for(it=(*it1).begin();it!=(*it1).end();it++) {
			printf(" %s[%d] ",rvars[*it],*it);
		}
		printf(" }");
	}
}

set<alias_t> union_alias(set<alias_t> s,alias_t t) {
	set<alias_t> result;
	alias_t temp=t;
	alias_t::iterator it;
	set<alias_t>::iterator it1;
	for(it1=s.begin();it1!=s.end();it1++) {					// for each alias set in s check
		for(it=t.begin();it!=t.end();it++) { 				// in any entry in t matches
			if(it1->find(*it)!=it1->end()) {				// if they do,
				temp.insert(it1->begin(),it1->end()); 		// insert the whole set in temp
				break;										// no point in considering the rest of the elements in t
			}
		}
		if(it==t.end()) {									// no element in t matched, insert alias set in result
			result.insert(*it1);
		}
	}
	result.insert(temp);									// finally, insert temp
	return result;
}

set<alias_t> union_alias(set<alias_t> s,set<alias_t> t) {
	set<alias_t> result=s;
	set<alias_t>::iterator it;
	for(it=t.begin();it!=t.end();it++) {
		result=union_alias(result,*it);
	}
	return result;
}

set<alias_t> remove_alias(set<alias_t> s,set<int> k) {		// remove alias sets from s that have at least one var in common with k
	set<alias_t> result;
	set<int>::iterator it;
	set<alias_t>::iterator it1;
	for(it1=s.begin();it1!=s.end();it1++) {
		for(it=k.begin();it!=k.end();it++) {
			if(it1->find(*it)!=it1->end())					// element found
				break;										// no point in considering the rest of the elements in k
		}
		if(it==k.end()) {									// no element in k matched, select the alias set
			result.insert(*it1);
		}
	}
	return result;
}

void init_stmt_list() {
	int i;
	for(i=0;i<MAX_STMTS;i++) {
		stmt_list[i].expr=NULL;
		stmt_list[i].is_discont=false;
		stmt_list[i].is_fn_call=false;
		stmt_list[i].is_conditioned=false;
		stmt_list[i].is_hoisted=false;
		stmt_list[i].is_loop_header=false;
		stmt_list[i].pred.clear();
		stmt_list[i].succ.clear();
		stmt_list[i].kill.clear();
		stmt_list[i].gen.clear();
		stmt_list[i].single_gen.clear();
		stmt_list[i].single_gen_map.clear();
		stmt_list[i].gen_alias.clear();
		stmt_list[i].live_in.clear();
		stmt_list[i].live_out.clear();
		stmt_list[i].alias_in.clear();
		stmt_list[i].alias_out.clear();
		stmt_list[i].agg_expr.clear();
		stmt_list[i].loops.clear();
		stmt_list[i].fn_id=-1;
		stmt_list[i].start_tag[0]='\0';
		stmt_list[i].end_tag[0]='\0';
	}
	global_stmt_cnt=0;
	rvars.clear();
}

void Rf_BuildCFG(SEXP expr,SEXP rho,int &stmt_ctr,vector<int> &exit_points, vector<int> &break_points, vector<int> &cont_points, vector<int> &return_points) {
	SEXP op,t;
	struct stmt_t *stmt_ptr;
	if(candidate_expr_map.find(stmt_ctr)==candidate_expr_map.end()) {
		candidate_expr_map[stmt_ctr]=expr;
	}
    switch (TYPEOF(expr)) {
    case NILSXP:
		return;
    case LISTSXP:
		Rf_BuildCFG(CAR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
		Rf_BuildCFG(CDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
		return;
    case REALSXP:
		return;
	case SYMSXP: {
			int use_id;
			stmt_ptr=&stmt_list[stmt_ctr];
			ADD_USE(stmt_ptr,CHAR(PRINTNAME(expr)),expr,use_id);
			return;
		}
	case CLOSXP:
		Rf_BuildCFG(CDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
		return;
	case LANGSXP:
		stmt_ptr=&stmt_list[stmt_ctr];
		if (TYPEOF(CAR(expr)) == SYMSXP) {
			const char *fn_name=CHAR(PRINTNAME(CAR(expr)));
			if(!strcmp(fn_name,"<-")||!strcmp(fn_name,"=")||!strcmp(fn_name,"<<-")||!strcmp(fn_name,"for")||!strcmp(fn_name,"while")||!strcmp(fn_name,"if")||!strcmp(fn_name,"{")||!strcmp(fn_name,"break")||!strcmp(fn_name,"next"))
			  {
				if (!strcmp(fn_name,"<-")||!strcmp(fn_name,"=")||!strcmp(fn_name,"<<-")) {
					vector<int> local_exits;
					if(TYPEOF(CADDR(expr))==SYMSXP) {
						stmt_ptr=&stmt_list[stmt_ctr];
						ADD_ALIAS(stmt_ptr,CHAR(PRINTNAME(CADR(expr))),CHAR(PRINTNAME(CADDR(expr))));
					}
					stmt_ptr=&stmt_list[stmt_ctr];
					int def_id;
					// evaluate the LHS
					if((TYPEOF(CADR(expr)))==LANGSXP) {
						SEXP temp=CADR(expr);
						if(TYPEOF(CAR(temp)) == SYMSXP) {
							SEXP local_op;
							PROTECT(local_op = findFun(CAR(temp), rho));
							if(TYPEOF(local_op)==CLOSXP) {	// assignment to function
								const char *temp_fn_name=CHAR(PRINTNAME(CAR(temp)));
								if(!strcmp(temp_fn_name,"formals")) {
									printf("Redefining FORMALS\n");
								}
								else if(!strcmp(temp_fn_name,"body")) {
									printf("Redefining BODY\n");
								}
								force_def=true;
								force_def_super=(!strcmp(fn_name,"<<-"));
								Rf_BuildCFG(CDR(temp),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
								force_def=false;
								force_def_super=false;
							}
							else {
								const char *temp_fn_name=PRIMNAME(local_op);
								if (!strcmp(temp_fn_name,"[")) {
									ADD_DEF(stmt_ptr,CHAR(PRINTNAME(CADR(temp))),!strcmp(fn_name,"<<-"),def_id);
									ADD_AGGEXPR(stmt_ptr,CDDR(temp));
									stmt_ptr->expr=expr;
									expr->stmt_id=stmt_ctr;
									Rf_BuildCFG(CDDR(temp),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
								}
								else {
									ADD_DEF(stmt_ptr,"agg",!strcmp(fn_name,"<<-"),def_id);
									stmt_ptr->expr=expr;
									expr->stmt_id=stmt_ctr;
								}
							}
							UNPROTECT(1);
						}
						else {
							ADD_DEF(stmt_ptr,"agg",!strcmp(fn_name,"<<-"),def_id);
							stmt_ptr->expr=expr;
							expr->stmt_id=stmt_ctr;
						}
					}
					else {
						ADD_DEF(stmt_ptr,CHAR(PRINTNAME(CADR(expr))),!strcmp(fn_name,"<<-"),def_id);
						stmt_ptr->expr=expr;
						expr->stmt_id=stmt_ctr;
					}
					stmt_ptr=&stmt_list[stmt_ctr];
					// evaluate the RHS
					Rf_BuildCFG(CDDR(expr),rho,stmt_ctr,local_exits,break_points,cont_points,return_points);
					if(local_exits.size()) {
						stmt_ptr=&stmt_list[stmt_ctr];
						if(!stmt_ptr->expr) {
							stmt_ptr->expr=expr;
							expr->stmt_id=stmt_ctr;
						}
						stmt_ctr++;
						int c1=stmt_ctr;
						ADD_EXITS(local_exits,local_exits.size(),c1,c1);
					}
				}
				else if(!strcmp(fn_name,"for")) {
					int c1,c2;
					int b;
					int def_id;
					int local_break_stmt=-1;
					int local_cont_stmt=-1;
					vector<int> local_exits,local_breaks,local_conts;
					stmt_ptr=&stmt_list[stmt_ctr];
					ADD_DEF(stmt_ptr,CHAR(PRINTNAME(CADR(expr))),false,def_id);
					stmt_ptr->expr=expr;
					expr->stmt_id=stmt_ctr;
					c1=stmt_ctr;
					ADD_EDGE(c1,c1+1);
					exit_points.push_back(c1);
					Rf_BuildCFG(CADDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);	// condition
					stmt_ptr=&stmt_list[stmt_ctr];
					if(!stmt_ptr->expr) {
						stmt_ptr->expr=expr;
						expr->stmt_id=stmt_ctr;
					}
					stmt_ctr++;
					Rf_BuildCFG(CDR(CDDR(expr)),rho,stmt_ctr,local_exits,local_breaks,local_conts,return_points);
					c2=stmt_ctr;
					ADD_CONTS(local_conts,local_conts.size(),c1);							// continue edge
					ADD_EXITS(local_exits,local_exits.size(),c1,c2);						// back edge
					for(b=0;b<local_breaks.size();b++) {									// breaks
						exit_points.push_back(local_breaks[b]);
					}
					loop_t l;
					l.start_node=c1;
					l.end_node=c2;
					l.ind_var_id=def_id;
					l.is_for_loop=true;
					loop_list.push_back(l);
				}
				else if(!strcmp(fn_name,"while")) {
					int c1,c2;
					int b;
					vector<int> local_exits,local_breaks,local_conts;
					c1=stmt_ctr;
					ADD_EDGE(c1,c1+1);
					exit_points.push_back(c1);
					Rf_BuildCFG(CADR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);		// condition
					stmt_ptr=&stmt_list[stmt_ctr];
					if(!stmt_ptr->expr) {
						stmt_ptr->expr=expr;
						expr->stmt_id=stmt_ctr;
					}					
					stmt_ctr++;
					Rf_BuildCFG(CDDR(expr),rho,stmt_ctr,local_exits,local_breaks,local_conts,return_points);
					c2=stmt_ctr;
					ADD_CONTS(local_conts,local_conts.size(),c1);							// continue edge
					ADD_EXITS(local_exits,local_exits.size(),c1,c2);						// back edge
					for(b=0;b<local_breaks.size();b++) {									// breaks
						exit_points.push_back(local_breaks[b]);
					}					
					loop_t l;
					l.start_node=c1;
					l.end_node=c2;
					l.ind_var_id=-1;
					l.is_for_loop=false;
					loop_list.push_back(l);
				}
				else if(!strcmp(fn_name,"if")) {
					int c1,c2,c3;
					c1=stmt_ctr;
					ADD_EDGE(c1,c1+1);
					Rf_BuildCFG(CADR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);		// condition
					stmt_ptr=&stmt_list[stmt_ctr];
					if(!stmt_ptr->expr) {
						stmt_ptr->expr=expr;
						expr->stmt_id=stmt_ctr;
					}				
					stmt_ctr++;
					Rf_BuildCFG(CADDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);	// if true
					c2=stmt_ctr;
					if(!stmt_list[c2].is_discont) {
						exit_points.push_back(c2);
					}
					if(TYPEOF(CDR(CDDR(expr)))!=NILSXP) {	// only if else clause exists
						ADD_EDGE(c1,c2+1);
						stmt_ptr=&stmt_list[stmt_ctr];
						if(!stmt_ptr->expr) {
							stmt_ptr->expr=expr;
							expr->stmt_id=stmt_ctr;
						}	
						stmt_ctr++;
						Rf_BuildCFG(CDR(CDDR(expr)),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
						c3=stmt_ctr;
						if(!stmt_list[c3].is_discont) {
							exit_points.push_back(c3);
						}
						SET_CONDITIONED(c1,max(c2,c3));
					}
					else {	// no else clause
						exit_points.push_back(c1);
						SET_CONDITIONED(c1,c2);
					}
				}
				else if(!strcmp(fn_name,"{")) {
					t=CDR(expr);
					int c1;
					vector<int> local_exits;
					int i;
					if(TYPEOF(t)==LISTSXP) {
						do {
						    local_exits.clear();
							Rf_BuildCFG(CAR(t),rho,stmt_ctr,local_exits,break_points,cont_points,return_points);
							t=CDR(t);
							if(TYPEOF(t)==NILSXP)
								break;
							// add edges only if one more statement exists in the list, otherwise exits/breaks/continues will be handled by upper levels
							c1=stmt_ctr;
							ADD_EXITS(local_exits,local_exits.size(),c1+1,c1);
							stmt_ptr=&stmt_list[stmt_ctr];
							if(!stmt_ptr->expr) {
								stmt_ptr->expr=expr;
								expr->stmt_id=stmt_ctr;
							}
							stmt_ctr++;
						}while(1);
						for(i=0;i<local_exits.size();i++) { // carry over exits
							exit_points.push_back(local_exits[i]);
						}
					}
				}
				else if(!strcmp(fn_name,"break")||!strcmp(fn_name,"next")) {
					const char *temp=CHAR(PRINTNAME(CAR(expr)));
					if(!strcmp(temp,"break")) {
						break_points.push_back(stmt_ctr);
						stmt_ptr=&stmt_list[stmt_ctr];
						stmt_ptr->is_discont=true;
					}
					else if(!strcmp(temp,"next")) {
						cont_points.push_back(stmt_ctr);
						stmt_ptr=&stmt_list[stmt_ctr];
						stmt_ptr->is_discont=true;
					}
					else
						Rf_BuildCFG(CDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
				}
				else {
					Rf_BuildCFG(CDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
				}
			}
			else {
				if(!strcmp(fn_name,"function")) {
					function_t fn;
					current_fn_ctr++;
					stringstream ss;
					ss << current_fn_ctr;
					fn.name=function_list[current_fn_id].name+string("_")+ss.str();
					fn.formals=CADR(expr);
					fn.body=CDDR(expr);
					fn.parent_id=current_fn_id;
					function_list.push_back(fn);
					call_t c;
					int def_id;
					c.node_id=stmt_ctr;
					c.fn_id=function_list.size()-1;
					ADD_DEF(stmt_ptr,fn.name.c_str(),false,def_id);
					c.var_id=find_var_id(fn.name.c_str());
					call_list.push_back(c);
				}
				else if(!strcmp(fn_name,"return")) {
					stmt_ptr->is_discont=true;
					return_points.push_back(stmt_ctr);
					Rf_BuildCFG(CDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
				}
				else {
					int k1=stmt_ctr;
					SEXP old_expr=stmt_ptr->expr;
					Rf_BuildCFG(CDR(expr),rho,stmt_ctr,exit_points,break_points,cont_points,return_points);
					if(k1!=stmt_ctr) {
						assert(!old_expr);	// currently we don't handle this kind of compound statement; it needs to be broken up
						sprintf(stmt_list[k1].start_tag,"%s(",fn_name);
						sprintf(stmt_list[stmt_ctr].end_tag,")\n");
					}
					if(find_var_id(fn_name)>=0) {
						int use_id;
						ADD_USE(stmt_ptr,fn_name,expr,use_id);
						stmt_ptr->is_fn_call=true;
					}
					if(!strcmp(fn_name,"[")) {
						ADD_AGGEXPR(stmt_ptr,CDDR(expr));
					}
				}
			}
			return;
		}
    default: break;
    }
}

void Rf_ReplIteration_process(SEXP rho) {
	SEXP value;
	int i,j;
	init_stmt_list();
	vector<int> exit_points,break_points,cont_points,return_points;
	i=0;
	function_list[i].parent_id=-1;	// for the main function
	vector<function_t>::iterator it;
	while(i!=function_list.size()) {
		function_list[i].entry_node=global_stmt_cnt;
		ADD_EDGE(global_stmt_cnt,global_stmt_cnt+1);
		SEXP fn_body=function_list[i].body;
		current_fn_id=i;
		global_stmt_cnt++;
		current_fn_ctr=0;
		exit_points.clear();break_points.clear();cont_points.clear();return_points.clear();
		Rf_BuildCFG(fn_body, rho, global_stmt_cnt, exit_points, break_points, cont_points, return_points);
		ADD_EXITS(exit_points,exit_points.size(),global_stmt_cnt+1,global_stmt_cnt);
		ADD_EXITS(return_points,return_points.size(),global_stmt_cnt+1,global_stmt_cnt);
		function_list[i].exit_node=global_stmt_cnt+1;
		global_stmt_cnt+=2;
		i++;
	}
	// set all stmt ids
	printf("======================================================\n");
	for(i=0;i<global_stmt_cnt;i++) {
		stmt_list[i].stmt_id=i;
		if(!stmt_list[i].expr) {
//			printf("no expr at %d: ",i);
			if(stmt_list[i].gen.size()) {	// at least one use
//				printf("ASSIGNING ");
//				Rf_PrintValue(candidate_expr_map[i]);
				SEXP e=candidate_expr_map[i];
				stmt_list[i].expr=e;
				e->stmt_id=i;
			}
			else
				printf("\n");
		}
		if(stmt_list[i].expr) {
			printf("%d[%d] ",stmt_list[i].stmt_id,stmt_list[i].is_conditioned);Rf_PrintValue(stmt_list[i].expr);
		}
	}
	printf("======================================================\n");
	// tag nodes with loop ids
	for(i=0;i<loop_list.size();i++) {
		int start=loop_list[i].start_node;
		int end=loop_list[i].end_node;
		stmt_list[start].is_loop_header=true;
		for(j=start;j<=end;j++)
			stmt_list[j].loops.push_back(i);
	}
	// tag nodes with function ids
	for(i=0;i<function_list.size();i++) {
		printf("Function %s: Entry %d: Exit %d\n",function_list[i].name.c_str(),function_list[i].entry_node,function_list[i].exit_node);
		int entry=function_list[i].entry_node;
		int exit=function_list[i].exit_node;
		printf("TAG %d...%d %d\n",i,entry,exit);
		for(j=entry;j<=exit;j++)
			stmt_list[j].fn_id=i;
	}
	for(i=0;i<call_list.size();i++) {
		printf("Call %d -> %d\n",call_list[i].node_id,call_list[i].fn_id);
	}
	// add aliases for function definitions
	for(int c=0;c<call_list.size();c++) {
		int s=call_list[c].node_id;
		int fn=call_list[c].fn_id;
		if(stmt_list[s].kill.size()) {
			set<int>::iterator it;
			for(it=stmt_list[s].kill.begin();it!=stmt_list[s].kill.end();it++) {
				ADD_ALIAS((&stmt_list[s]),rvars[*it],function_list[fn].name.c_str());
				printf("added %s to %s at %d\n",function_list[fn].name.c_str(),rvars[*it],s);
			}
		}
	}
	fflush(stdout);
}

bool is_nequal(set<int> v1,set<int> v2) {
	int s1=v1.size();
	int s2=v2.size();
	if(s1!=s2) return true;
	set<int>::iterator it;
	for(it=v1.begin();it!=v1.end();it++) {
		if(v2.find(*it)==v2.end())
			return true;
	}
	return false;
}

bool is_nequal(set<alias_t> v1,set<alias_t> v2) {
	int s1=v1.size();
	int s2=v2.size();
	if(s1!=s2) return true;
	set<alias_t>::iterator it, it1;
	for(it=v1.begin();it!=v1.end();it++) {
		for(it1=v2.begin();it1!=v2.end();it1++) {
			if(!is_nequal(*it,*it1))		// match found
				break;
		}
		if(it1==v2.end()) return true;		// no match
	}
	return false;
}

set<int> set_union(set<int> v1,set<int> v2) {
	set<int> result=v1;
	result.insert(v2.begin(),v2.end());
	return result;
}

set<int> set_diff(set<int> v1,set<int> v2) {
	set<int> result=v1;
	set<int>::iterator it;
	for(it=v2.begin();it!=v2.end();it++) {
		if(result.find(*it)!=result.end())
			result.erase(*it);
	}
	return result;
}

set<int> set_intersect(set<int> v1,set<int> v2) {
	return set_diff(v1,set_diff(v1,v2));
}

void PrintGenKill() {
	printf("Gen-Kill Info\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		int pos=0;
		printf("%d: ",i);
		labels[i][0]='\0';
		pos+=sprintf(labels[i]+pos,"%d:: ",i);
		set<int>::iterator it;
		for(it=stmt_list[i].kill.begin();it!=stmt_list[i].kill.end();it++) {
			printf("%s[%d] ",rvars[*it],*it);
			pos+=sprintf(labels[i]+pos,"%s ",rvars[*it]);
		}
		printf(":: ");
		pos+=sprintf(labels[i]+pos,"<- ");
		for(it=stmt_list[i].gen.begin();it!=stmt_list[i].gen.end();it++) {
			printf("%s[%d] ",rvars[*it],*it);
			pos+=sprintf(labels[i]+pos,"%s ",rvars[*it]);
		}
		printf("\n");
	}
}

void PrintCFG() {
	printf("CFG\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		set<int>::iterator it;
		for(it=stmt_list[i].pred.begin();it!=stmt_list[i].pred.end();it++) {
			printf("%d ",*it);
		}
		printf(":: ");
		for(it=stmt_list[i].succ.begin();it!=stmt_list[i].succ.end();it++) {
			printf("%d ",*it);
		}
		printf("\n");
	}	
}

void LiveVarAnalysis() {
	int num_iter=0;
	bool changed;
	do {
		changed=false;
		for(int i=0;i<global_stmt_cnt;i++) {
			stmt_list[i].live_in=set_union(stmt_list[i].gen,set_diff(stmt_list[i].live_out,stmt_list[i].kill));
		}
		for(int i=0;i<global_stmt_cnt;i++) {
			set<int> old_live_out=stmt_list[i].live_out;
			stmt_list[i].live_out.clear();
			set<int>::iterator it;
			for(it=stmt_list[i].succ.begin();it!=stmt_list[i].succ.end();it++) {
				stmt_list[i].live_out=set_union(stmt_list[i].live_out,stmt_list[*it].live_in);
			}
			changed|=is_nequal(old_live_out,stmt_list[i].live_out);
		}
		num_iter++;
	}while(changed);
	printf("Live Variable Analysis\n");
	printf("Number of iterations=%d\n",num_iter);
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		set<int>::iterator it;
		for(it=stmt_list[i].live_out.begin();it!=stmt_list[i].live_out.end();it++)
			printf("%s[%d] ",rvars[*it],*it);
		printf("\n");
	}
/*	printf("=============================\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		set<int>::iterator it;
		for(it=stmt_list[i].live_in.begin();it!=stmt_list[i].live_in.end();it++)
			printf("%s ",rvars[*it]);
		printf("\n");
	}*/
}

void AliasAnalysis() {
	printf("Alias Gen\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		print_alias_set(stmt_list[i].gen_alias);
		printf("\n");
	}
	// first add any remaining aliases (e.g.,multiple assignments for a single statement)
	for(int i=0;i<global_stmt_cnt;i++) {
		if(stmt_list[i].kill.size()>1) {
			set<int>::iterator it;
			int first_var=*(stmt_list[i].kill.begin());
			for(it=stmt_list[i].kill.begin();it!=stmt_list[i].kill.end();it++) {
				ADD_ALIAS((&stmt_list[i]),rvars[first_var],rvars[*it]);
			}
		}
	}	
	int num_iter=0;
	bool changed;
	do {
		changed=false;
		for(int i=0;i<global_stmt_cnt;i++) {
			stmt_list[i].alias_out=union_alias(stmt_list[i].gen_alias,remove_alias(stmt_list[i].alias_in,stmt_list[i].kill));
		}
		for(int i=0;i<global_stmt_cnt;i++) {
			set<alias_t> old_alias_in=stmt_list[i].alias_in;
			stmt_list[i].alias_in.clear();
			set<int>::iterator it;
			for(it=stmt_list[i].pred.begin();it!=stmt_list[i].pred.end();it++) {
				stmt_list[i].alias_in=union_alias(stmt_list[i].alias_in,stmt_list[*it].alias_out);
			}
			changed|=is_nequal(old_alias_in,stmt_list[i].alias_in);
		}
		num_iter++;
	}while(changed);
	printf("Alias Analysis\n");
	printf("Number of iterations=%d\n",num_iter);
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		print_alias_set(stmt_list[i].alias_in);
		printf("\n");
	}
}

void PrintUseNotLive() {
	printf("Used but not Live Out\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		set<int> t=set_diff(stmt_list[i].gen,stmt_list[i].live_out);
		set<int>::iterator it;
		for(it=t.begin();it!=t.end();it++)
			printf("%s[%d] ",rvars[*it],*it);
		printf("\n");
	}
}

void CalcAndPrintDefNoLiveAlias() {
	printf("(Re-)defined, but either (1) not already aliased or (2) aliased variable is not live out\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		assert(stmt_list[i].expr);
		if(!stmt_list[i].kill.size()) continue;
		bool flag=false;
		printf("%d: ",i);
		set<int>::iterator it,it2;
		set<alias_t>::iterator it1;
		for(it=stmt_list[i].kill.begin();it!=stmt_list[i].kill.end();it++) {
			for(it1=stmt_list[i].alias_in.begin();it1!=stmt_list[i].alias_in.end();it1++) {
				it2=(*it1).find(*it);
				if((it2!=(*it1).end())&&((*it1).size()>1)) {	// aliased with a different variable
					// first construct a set with the other variables
					set<int> temp=(*it1);
					temp.erase(*it);
					if(!set_intersect(stmt_list[i].live_out,temp).size())
						continue;						// not live out
					else
						break;							// alias is live
				}
			}
			if(it1==stmt_list[i].alias_in.end()) {
				printf("%s[%d] ",rvars[*it],*it);
				flag=true;
			}
		}
		printf("\n");
		stmt_list[i].expr->ignore_shared=(char)(flag);
	}
}

void PrintSingleGen() {
	printf("Single Gen\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		set<int>::iterator it;
		for(it=stmt_list[i].single_gen.begin();it!=stmt_list[i].single_gen.end();it++) {
			printf("%s[%d] ",rvars[*it],*it);
		}
		printf("\n");
	}
}

void CalcAndPrintSingleGenNotLiveOut() {
	printf("Single Gen Not Live Out and either (1) no alias or (2) no alias is live out\n");
	for(int i=0;i<global_stmt_cnt;i++) {
		printf("%d: ",i);
		if(stmt_list[i].expr==NULL) continue;
		set<int>::iterator it;
		set<int> result=set_diff(stmt_list[i].single_gen,stmt_list[i].live_out);	// computes variables that are single gen but not live out
		set<int> result_final;
		// check if any of these are aliased with alias-in
		for(it=result.begin();it!=result.end();it++) {
			set<alias_t>::iterator it1;
			int this_var=(*it);
			bool flag=false;
			for(it1=stmt_list[i].alias_in.begin();it1!=stmt_list[i].alias_in.end();it1++) {
				set<int>::iterator it2=(*it1).find(this_var);
				if(it2==(*it1).end()) continue;		// this alias set does not have a match, search the next alias set
				set<int>::iterator it3;				// found a match, check if any of the other variables are live-in
				for(it3=(*it1).begin();it3!=(*it1).end();it3++) {
					if((*it3)==this_var)	continue;	
					int aliased_var=(*it3);			// check if this is live-in
					if(stmt_list[i].live_in.find(aliased_var)==stmt_list[i].live_in.end()) continue;	// not live-in
					flag=true;	// live-in
					break;
				}
				if(flag==true) break;	// no point in searching other alias sets
			}
			if(!flag) {
				printf("%s[%d] ",rvars[this_var],this_var);
				result_final.insert(this_var);
			}
		}
		printf("\n");
		assert(result_final.size()<16);
		int j;
                int nd=0;
		for(j=0,it=result_final.begin();it!=result_final.end();it++,j++) {
			dead_vars_info[i].dead_vars[j]=stmt_list[i].single_gen_map[*it];
                        nd++;
                        if(nd==16)  // max. 16 dead vars tracked per statement
			   break;
		}
		dead_vars_info[i].num_dead_vars=(char)(nd);
	}
}

void AddFnEdges() {
	for(int i=0;i<global_stmt_cnt;i++) {
		if(!stmt_list[i].is_fn_call) continue;
		set<alias_t>::iterator it;
		for(it=stmt_list[i].alias_in.begin();it!=stmt_list[i].alias_in.end();it++) {
			for(int c=0;c<call_list.size();c++) {
				int id=call_list[c].var_id;
				if((*it).find(id)!=(*it).end()&&( set_intersect((*it),stmt_list[i].gen ).size() )) {	// check that the fn is aliased AND that alias set is also in gen
					printf("Found call for function %s at node %d\n",function_list[call_list[c].fn_id].name.c_str(),i);
					ADD_EDGE_FORCE(i,function_list[call_list[c].fn_id].entry_node);
					ADD_EDGE_ALL_SUCC_FORCE(function_list[call_list[c].fn_id].exit_node,i,stmt_list[i].fn_id);
					REMOVE_ALL_EDGE_SUCC(i,stmt_list[i].fn_id);
				}
			}
		}
	}
}

int RemoveSubstr(char *src, char *substr, char *dst) {
	char *src_ptr=src;
	char *dst_ptr=dst;
	int num_matches=0;
	while(1)
	{
		char *ptr=strstr(src_ptr,substr);
		if(!ptr)
			break;
		num_matches++;
		while(src_ptr!=ptr) {
			*dst_ptr=*src_ptr;
			dst_ptr++;
			src_ptr++;
		}
		src_ptr+=strlen(substr);
		if(*src_ptr=='\0')
			return num_matches;
	}
	strcpy(dst_ptr,src_ptr);	// copy the remaining part
	return num_matches;
}

// find the number of occurrences of each symbol. This is needed to decide if hoisting is possible
void FindNumSymbols(SEXP expr,map<const char*,int> &num_sym_map) {
	int t=TYPEOF(expr);
	switch(t) {
		case LISTSXP:
		case LANGSXP:
			FindNumSymbols(CAR(expr),num_sym_map);
			FindNumSymbols(CDR(expr),num_sym_map);
			break;
		case SYMSXP: {
			const char *name=CHAR(PRINTNAME(expr));
			if(num_sym_map.find(name)==num_sym_map.end()) {
				num_sym_map[name]=1;
			}
			else {
				int old_val=num_sym_map[name];
				num_sym_map[name]=old_val+1;
			}
		}
		default: return;
	};
}

// returns true if successfully removed all ind. variables
bool RemoveIndVarsPrint(SEXP expr,set<int> ind_vars) {
	int t=TYPEOF(expr);
	long fpos=ftell(codegen_fp);
	Rf_PrintValue(expr);
	fseek(codegen_fp,fpos,SEEK_SET);
	char buffer[10000],buffer1[10000];
	fgets(buffer,sizeof(buffer),codegen_fp);
	set<int>::iterator it;
	int num_matches;
	map<const char*,int> num_sym_map;
	FindNumSymbols(expr,num_sym_map);
	bool cancel_hoisting=false;
	for(it=ind_vars.begin();it!=ind_vars.end();it++) {
		char subscript[10000];
		sprintf(subscript,"[%s]",rvars[*it]);
		num_matches=RemoveSubstr(buffer,subscript,buffer1);
		if((num_matches && (num_matches!=num_sym_map[rvars[*it]])) ||
		   ((!num_matches) && num_sym_map.find(rvars[*it])!=num_sym_map.end())
		  )
		{	// other occurrences of the ind. var exist in the expr
			cancel_hoisting=true;
			break;
		}
		strcpy(buffer,buffer1);
	}
	fseek(codegen_fp,fpos,SEEK_SET);
	if(!cancel_hoisting) {
		fprintf(codegen_fp,"%s",buffer);
		return true;
	}
	return false;
}

bool IsVectorizable(SEXP expr,int var_id) {
	bool result;
	set<int> var_set;
	var_set.insert(var_id);
	codegen_fp=freopen("codegen.txt","r+",stdout);
	Rf_PrintValue(expr);
	result=RemoveIndVarsPrint(expr,var_set);
	freopen("/dev/tty","w",codegen_fp);
	fclose(codegen_fp);
	return result;
}

void FindLoopInvariants() {
	vector<loop_t>::reverse_iterator it;	// check from outer to inner loops for maximum hoist
	// clear set of invariants initially
	for(it=loop_list.rbegin();it!=loop_list.rend();it++) {
		(*it).invariants.clear();
	}
	bool changed=true;
	while(changed){
	changed=false;
	for(it=loop_list.rbegin();it!=loop_list.rend();it++) {
		int loop_start_node=(*it).start_node;
		int loop_end_node=(*it).end_node;
		int loop_ind_var_id=(*it).ind_var_id;
		set<int> orig_invariants=(*it).invariants;
		for(int k=loop_start_node;k<=loop_end_node;k++) {
			stmt_t *stmt_ptr=&stmt_list[k];
			if(stmt_ptr->is_loop_header)
				continue;				// loop header not considered
			if(!stmt_ptr->expr)
				continue;
			set<int> all_vars;
			all_vars.insert(stmt_ptr->gen.begin(),stmt_ptr->gen.end());
			all_vars.insert(stmt_ptr->kill.begin(),stmt_ptr->kill.end());
			set<int>::iterator it1,it2;
			bool def_in_loop=false;
			bool is_vectorizable=false;
			if(!stmt_ptr->is_conditioned)
				is_vectorizable=IsVectorizable(stmt_ptr->expr,loop_ind_var_id);
			if(is_vectorizable)
				all_vars.erase(loop_ind_var_id);
			if(set_intersect(stmt_ptr->kill,stmt_ptr->gen).size())	// x = x op ...
				def_in_loop=true;
			else
			for(it1=all_vars.begin();it1!=all_vars.end();it1++) {	// for each gen
				int var_id=*it1;
				set<int> defs=stmt_ptr->reachdef_in[var_id];
				for(it2=defs.begin();it2!=defs.end();it2++) {
					int def_node=*it2;
					// non-invariant definition within the loop
					if((def_node>=loop_start_node)&&(def_node<=loop_end_node)&&(def_node!=stmt_ptr->stmt_id)&&((*it).invariants.find(def_node)==(*it).invariants.end())) {
						def_in_loop=true;
						break;										// no need to check other definitions
					}
				}
				if(def_in_loop) {
					break;											// no need to check other gens
				}
			}
			if(stmt_ptr->is_conditioned) 
				continue;						// since this may not be evaluated on all paths
			if(!def_in_loop) {
				(*it).invariants.insert(k);
			}
		}
		int orig_size=orig_invariants.size();
		orig_invariants.insert((*it).invariants.begin(),(*it).invariants.end());
		if(orig_size!=orig_invariants.size())
			changed=true;
		}
	}
}

bool can_hoist(struct stmt_t *stmt_ptr) {
	vector<int>::reverse_iterator it;	// check from outer to inner loops for maximum hoist
	set<int>::iterator it1,it2;
	for(it=stmt_ptr->loops.rbegin();it!=stmt_ptr->loops.rend();it++) {
		int loop_id=*it;
		if(loop_list[loop_id].invariants.find(stmt_ptr->stmt_id)!=loop_list[loop_id].invariants.end()) {
			fprintf(stderr,"HOIST %d above loop [%d,%d]\n",stmt_ptr->stmt_id,loop_list[loop_id].start_node,loop_list[loop_id].end_node);
			stmt_ptr->is_hoisted=true;
			loop_list[loop_id].hoisted_stmts.insert(stmt_ptr->stmt_id);
			return true;
		}
	}
	return false;
}

void CodeMotionAnalysis() {
	FindLoopInvariants();
	for(int i=0;i<global_stmt_cnt;i++) {
		stmt_t *stmt_ptr=&stmt_list[i];
		if(!stmt_ptr) continue;
		if(!stmt_ptr->loops.size()) continue;				// not in a loop
		vector<int>::iterator it;
		bool is_header=false;
		for(it=stmt_ptr->loops.begin();it!=stmt_ptr->loops.end();it++) {
			if(loop_list[*it].start_node==i) {
				is_header=true;								// loop header
				break;
			}
		}
		if(is_header) continue;
		if(can_hoist(stmt_ptr)) {
			stmt_ptr->is_hoisted=true;
		}
	}
}

void PrintHoisted(int loop_id,int stmt_id) {
	// find which index variables to remove
	int l_start=loop_list[loop_id].start_node;
	int l_end=loop_list[loop_id].end_node;
	set<int> loop_vars;
	vector<int>::iterator it;
	for(it=stmt_list[stmt_id].loops.begin();it!=stmt_list[stmt_id].loops.end();it++) {
		loop_t *loop_ptr=&loop_list[*it];
		if((loop_ptr->start_node>=l_start)&&(loop_ptr->end_node>=l_end)) {
			loop_vars.insert(loop_ptr->ind_var_id);
		}
	}
	bool is_successful=RemoveIndVarsPrint(stmt_list[stmt_id].expr,loop_vars);
	if(!is_successful) {
		stmt_list[stmt_id].is_hoisted=false;
		fprintf(stderr,"hoisting failed\n");
	}
}

void PrintTransformedFn(SEXP expr);

void FnGen(int fn_id) {
	int entry=function_list[fn_id].entry_node;
	int exit=function_list[fn_id].exit_node;
	for(int i=entry;i<=exit;i++) {
		stmt_t *stmt_ptr=&stmt_list[i];
		bool is_loop_header=false;
		int loop_header_id;
		bool is_for_loop=false;
		int loop_end_ctr=0;
		printf("%s",stmt_ptr->start_tag);
		//if (stmt_ptr->expr) 
		{
			vector<int>::iterator it;
			for(it=stmt_ptr->loops.begin();it!=stmt_ptr->loops.end();it++) {
				int loop_id=*it;
				if(stmt_ptr->stmt_id==loop_list[loop_id].start_node) {
					is_loop_header=true;
					loop_header_id=loop_id;
					is_for_loop=loop_list[(*it)].is_for_loop;
				}
			}
			if(is_loop_header) {
				printf("{\n");		// additional bracket needed for loops in function call arguments
				// first print hoisted statements
				set<int>::iterator it;
				for(it=loop_list[loop_header_id].hoisted_stmts.begin();it!=loop_list[loop_header_id].hoisted_stmts.end();it++) {
					PrintHoisted(loop_header_id,*it);
				}
				if(is_for_loop) {
					printf("for(");
					Rf_PrintValue(CAR(CDR(stmt_ptr->expr)));
					fseek(codegen_fp,-1,SEEK_CUR);	// to remove the newline
					printf(" in ");
					Rf_PrintValue(CAR(CDR(CDR(stmt_ptr->expr))));
					fseek(codegen_fp,-1,SEEK_CUR);	// to remove the newline
				}
				else {
					printf("while(");
					Rf_PrintValue(CAR(CDR(stmt_ptr->expr)));
					fseek(codegen_fp,-1,SEEK_CUR);	// to remove the newline
				}
				printf("){\n");
			}
			else if((stmt_ptr->expr)&&(!stmt_ptr->is_hoisted)) {
				PrintTransformedFn(stmt_ptr->expr);
				/*Rf_PrintValue(stmt_ptr->expr);
				if(stmt_ptr->is_conditioned) {
					while(stmt_ptr->is_conditioned) {	// nothing in the loop can be hoisted
						i++;
						stmt_ptr=&stmt_list[i];
					}
					i--;								// to correct for the i++ at the end of the loop
					stmt_ptr=&stmt_list[i];
				}*/
			}
			for(it=stmt_ptr->loops.begin();it!=stmt_ptr->loops.end();it++) {
				int loop_id=*it;
				if(stmt_ptr->stmt_id==loop_list[loop_id].end_node) {
					loop_end_ctr++;
				}
			}
			for(int j=0;j<loop_end_ctr;j++){
				printf("}\n");
			}
			if(loop_end_ctr)
				printf("}\n");
		}
		printf("%s",stmt_ptr->end_tag);
	}
}

int SearchFnId(SEXP fn_body) {
	int n=function_list.size();
	for(int i=0;i<n;i++) {
		if(function_list[i].body == fn_body)
			return i;
	}
	return -1;
}

bool HasFnDefn(SEXP expr) {
	int t=TYPEOF(expr);
	if(t==NILSXP) return false;
	bool result=false;
	if((t==LISTSXP)||(t==LANGSXP)) {
		SEXP e=CAR(expr);
		if(TYPEOF(e)!=NILSXP) {
			result |= HasFnDefn(e);
		}
		if(result) return true;
		e=CDR(expr);
		if(TYPEOF(e)!=NILSXP) {
			result |= HasFnDefn(e);
		}
		if(result) return true;
	}
	if(t!=SYMSXP) return false;
	const char *name=CHAR(PRINTNAME(expr));
	if(!strcmp(name,"function")) {
		result=true;
	}
	return result;
}

void PrintFormals(SEXP expr) {
	int t=TYPEOF(expr);
	switch(t) {
		case NILSXP: return;
		case LISTSXP:
			Rf_PrintValue(TAG(expr));
			fseek(codegen_fp,-1,SEEK_CUR);	// to remove the newline
			PrintFormals(CAR(expr));
			if(TYPEOF(CDR(expr))!=NILSXP) {
				printf(",");
				PrintFormals(CDR(expr));
			}
			break;
		case REALSXP: printf("=%g",*REAL(expr)); break;
		case INTSXP: printf("=%g",*INTEGER(expr)); break;
		case LGLSXP: printf("=%g",*LOGICAL(expr)); break;
		case STRSXP: printf("=\"%s\"",CHAR(STRING_ELT(expr,0))); break;
		case SYMSXP: {
			const char *tmp=CHAR(PRINTNAME(expr));
			if(strlen(tmp))
				printf("=%s",tmp);
		}
		break;
		default:
			printf("=");Rf_PrintValue(expr);
			fseek(codegen_fp,-1,SEEK_CUR);	// to remove the newline
			break;
	};
}

void PrintTransformedFn(SEXP expr) {
//	printf("TYPE=%d\n",TYPEOF(expr));
//	Rf_PrintValue(expr);
    switch (TYPEOF(expr)) {
    case NILSXP:
		return;
	case INTSXP:
		printf("%d\n",*INTEGER(expr));
		return;
	case REALSXP:
		printf("%g\n",*REAL(expr));
		return;
	case LGLSXP:
		printf("%d\n",*LOGICAL(expr));
		return;
	case STRSXP:
		printf("%s\n",CHAR(expr));
		return;
    case LISTSXP:
		PrintTransformedFn(CAR(expr));
		PrintTransformedFn(CDR(expr));
		return;
	case CLOSXP:
		PrintTransformedFn(CDR(expr));
		return;
	case LANGSXP:
		if (TYPEOF(CAR(expr)) == SYMSXP) {
			const char *fn_name=CHAR(PRINTNAME(CAR(expr)));
			if(!strcmp(fn_name,"{")) {
				printf("{\n");
				PrintTransformedFn(CDR(expr));
			}
//			else if(!HasFnDefn(expr)){
//				Rf_PrintValue(expr);
//			}
			else if(!strcmp(fn_name,"<-")||!strcmp(fn_name,"=")||!strcmp(fn_name,"<<-")) {
				PrintTransformedFn(CADR(expr));
				fseek(codegen_fp,-1,SEEK_CUR);	// to remove the newline
				printf(" %s ",fn_name);
				PrintTransformedFn(CDDR(expr));
			}
			else if(!strcmp(fn_name,"function")) {
				function_t fn;
				fn.formals=CADR(expr);
				fn.body=CDDR(expr);
				printf("function(");
				PrintFormals(fn.formals);
				printf(")\n");
		//		Rf_PrintValue(fn.formals);
		//		PrintTransformedFn(fn.body);
				int fn_id=SearchFnId(fn.body);
				assert(fn_id>=0);
				printf("{\n");
				FnGen(fn_id);
				printf("}\n");
			}
			else {
				Rf_PrintValue(expr);
			}
		}
		else {
			Rf_PrintValue(expr);
		}
		break;
    default: 
		Rf_PrintValue(expr);
		break;
    };
}

void CodeGen(SEXP expr) {
	codegen_fp=freopen("codegen.txt","w+",stdout);
	printf("main = function() {\n");
	//PrintTransformedFn(expr);
	FnGen(0);
	printf("}\n\n");
	printf("main()\n");
	fflush(stdout);
	freopen("/dev/tty","w",stdout);
}

/******************************************************************************/
/***************************** reaching definitions ***************************/
/******************************************************************************/

void ReachDefAnalysis() {
	int num_iter = 0;
	bool changed;
	// forward analysis
	do {
		changed = false;
		for(int i = 0; i < global_stmt_cnt; i++) {
			auto& rdin = stmt_list[i].reachdef_in;
			rdin.clear();
			FOR_EACH(it, stmt_list[i].pred) {
				auto const& defs = stmt_list[*it].reachdef_out;
				FOR_EACH(pd, defs) {
					rdin[pd->first] = set_union(rdin[pd->first], pd->second);
				}
			}
		}
		for(int i = 0; i < global_stmt_cnt; i++) {
			map<int, set<int> > rdout = stmt_list[i].reachdef_in;
			FOR_EACH(pv, stmt_list[i].kill) {
				rdout[*pv] = {i};
			}
			if (rdout != stmt_list[i].reachdef_out) {
				stmt_list[i].reachdef_out = rdout;
				changed = true;
			}
		}
		num_iter++;
	} while(changed);
	printf("Reaching Definition Analysis\n");
	printf("Number of iterations=%d\n", num_iter);
	for(int i = 0; i < global_stmt_cnt; i++) {
		printf("%d: ",i);
		FOR_EACH(it, stmt_list[i].reachdef_in) {
//			printf("%s[%d]{",rvars[it->first],it->first);
			printf("%s{",rvars[it->first]);
			bool first = true;
			FOR_EACH(pv, it->second) {
				if (first)
					first = false;
				else
					printf(",");
				printf("%d", *pv);
			}
			printf("}  ");
		}
		printf("\n");
	}
}

/******************************************************************************/
/******************************* type inferencing *****************************/
/******************************************************************************/

namespace typeinfer {
class Type;

class Type {
public:
	enum TypeID {
		kNull			= 0,
		kLogical		= 1,
		kInteger		= 2,
		kDouble			= 3,
		kString			= 4,
		kMatrix			= 101,
		kVector			= 151,
		kList			= 201,
		kAny			= 1001      // SEXP
	};
	virtual ~Type() {}
	virtual int id() = 0;
	virtual shared_ptr<Type> copy() = 0;
	virtual string toString() = 0;
	static shared_ptr<Type> join(shared_ptr<Type> t1, shared_ptr<Type> t2);
	static int compare(shared_ptr<Type> t1, shared_ptr<Type> t2);
protected:
	// Internal polymorphic method, pre-condition: this->id() > t->id()
	virtual shared_ptr<Type> join(shared_ptr<Type> t) = 0;
	// Internal polymorphic method, pre-condition: this->id() == t->id()
	virtual int compareTo(shared_ptr<Type> t) = 0;
};

/**
 * Null type: "bottom" of the type lattice
 */
class NilType: public Type {
public:
	int id() { return kNull; }
	shared_ptr<Type> copy() { return make_shared<NilType>(); }
	shared_ptr<Type> join(shared_ptr<Type> t) { return make_shared<NilType>(); }
	int compareTo(shared_ptr<Type> t) { return 0; }
	string toString() { return "NULL"; }
};

/**
 * Any type: "top" of the type lattice
 */
class AnyType: public Type {
public:
	int id() { return kAny; }
	shared_ptr<Type> join(shared_ptr<Type> t) { return make_shared<AnyType>(); }
	shared_ptr<Type> copy() { return make_shared<AnyType>(); }
	int compareTo(shared_ptr<Type> t) { return 0; }
	string toString() { return "any"; }
};

/**
 * Scalar type: an abstract type that involves bool / int / double / string
 */
class SclType: public Type {
public:
	shared_ptr<Type> join(shared_ptr<Type> t) { return this->copy(); }
	int compareTo(shared_ptr<Type> t) { return 0; }
};

/**
 * Logical type: corresponds to C bool
 */
class LglType: public SclType {
public:
	int id() { return kLogical; }
	shared_ptr<Type> copy() { return make_shared<LglType>(); }
	string toString() { return "bool"; }
};

/**
 * Integer type: corresponds to C int
 */
class IntType: public SclType {
public:
	int id() { return kInteger; }
	shared_ptr<Type> copy() { return make_shared<IntType>(); }
	string toString() { return "int"; }
};

/**
 * Double type: corresponds to C double
 */
class DblType: public SclType {
public:
	int id() { return kDouble; }
	shared_ptr<Type> copy() { return make_shared<DblType>(); }
	string toString() { return "double"; }
};

/**
 * String type: corresponds to C++ string
 */
class StrType: public SclType {
public:
	int id() { return kString; }
	shared_ptr<Type> copy() { return make_shared<StrType>(); }
	string toString() { return "string"; }
};

/**
 * Constructed type: an abstract type that involves vector / list / matrix
 */
class ConType: public Type {
public:
	virtual shared_ptr<Type> elemType() = 0;
	virtual shared_ptr<Type> withElemType(shared_ptr<Type> elemType)  = 0;
};

/**
 * Vector type: corresponds to C++ vector
 */
class VecType: public ConType {
protected:
	shared_ptr<Type> _elemType;
public:
	VecType() : _elemType(make_shared<NilType>()) {}
	VecType(shared_ptr<Type> elemType) : _elemType(elemType) {}
	int id() { return kVector; }
	shared_ptr<Type> elemType() { return _elemType; }
	shared_ptr<Type> withElemType(shared_ptr<Type> elemType) {
		return make_shared<VecType>(elemType);
	}
	shared_ptr<Type> copy() { return make_shared<VecType>(_elemType); }
	shared_ptr<Type> join(shared_ptr<Type> o) {
		shared_ptr<VecType> t = make_shared<VecType>();
		if (o->id() <= kString) {
			t->_elemType = Type::join(_elemType, o);
		} else if (o->id() <= kVector) {
			t->_elemType = Type::join(
					_elemType, dynamic_pointer_cast<VecType>(o)->elemType());
		}
		return t;
	}
	int compareTo(shared_ptr<Type> o) {
		return Type::compare(_elemType,
				dynamic_pointer_cast<VecType>(o)->elemType());
	}
	string toString() { return "vector(" + _elemType->toString() + ")"; }
};

/**
 * List type: corresponds to C++ vector
 */
class LstType: public ConType {
protected:
	shared_ptr<Type> _elemType;
public:
	LstType() : _elemType(make_shared<NilType>()) {}
	LstType(shared_ptr<Type> elemType) : _elemType(elemType) {}
	int id() { return kList; }
	shared_ptr<Type> elemType() { return _elemType; }
	shared_ptr<Type> withElemType(shared_ptr<Type> elemType) {
		return make_shared<LstType>(elemType);
	}
	shared_ptr<Type> copy() { return make_shared<LstType>(_elemType); }
	shared_ptr<Type> join(shared_ptr<Type> o) {
		shared_ptr<LstType> t = make_shared<LstType>();
		if (o->id() <= kString) {
			t->_elemType = Type::join(_elemType, o);
		} else if (o->id() <= kList) {
			t->_elemType = Type::join(
					_elemType, dynamic_pointer_cast<ConType>(o)->elemType());
		}
		return t;
	}
	int compareTo(shared_ptr<Type> o) {
		return Type::compare(_elemType,
				dynamic_pointer_cast<LstType>(o)->elemType());
	}
	string toString() { return "list(" + _elemType->toString() + ")"; }
};

/**
 * Matrix type: special vector / list type
 */
class MatType : public ConType {
protected:
	shared_ptr<Type> _baseType;
public:
	MatType() : _baseType(make_shared<VecType>()) {}
	MatType(shared_ptr<Type> baseType) : _baseType(baseType) {}
	int id() { return kMatrix; }
	shared_ptr<Type> elemType() {
		if (_baseType->id() == kVector || _baseType->id() == kList) {
			return dynamic_pointer_cast<ConType>(_baseType)->elemType();
		} else {
			return make_shared<AnyType>();
		}
	}
	shared_ptr<Type> baseType() { return _baseType; }
	shared_ptr<Type> withElemType(shared_ptr<Type> elemType) {
		if (_baseType->id() == kVector || _baseType->id() == kList) {
			return make_shared<MatType>(
					dynamic_pointer_cast<ConType>(_baseType)->withElemType(
							elemType));
		} else {
			return make_shared<MatType>();
		}
	}
	shared_ptr<Type> copy() { return make_shared<MatType>(_baseType); }
	shared_ptr<Type> join(shared_ptr<Type> o) {
		if (o->id() <= kString) {
			return make_shared<MatType>(Type::join(_baseType, o));
		} else if (o->id() == kMatrix){
			return make_shared<MatType>(
					Type::join(_baseType,
							dynamic_pointer_cast<MatType>(o)->_baseType));
		}
		return make_shared<AnyType>();
	}
	int compareTo(shared_ptr<Type> o) {
		return Type::compare(_baseType,
				dynamic_pointer_cast<MatType>(o)->baseType());
	}
	string toString() { return "matrix(base: " + _baseType->toString() + ")"; }
};

shared_ptr<Type> Type::join(shared_ptr<Type> t1, shared_ptr<Type> t2) {
	if (t1->id() < t2->id()) {
		swap(t1, t2);
	}
	return t1->join(t2);
}

int Type::compare(shared_ptr<Type> t1, shared_ptr<Type> t2) {
	if (!t1) t1 = make_shared<NilType>();
	if (!t2) t2 = make_shared<NilType>();
	return t1->id() == t2->id() ? t1->compareTo(t2) : t1->id() - t2->id();
}

class TypeInference {
private:
	class FunctionApply {
	public:
		virtual ~FunctionApply() {};
		virtual shared_ptr<Type> infer(
				vector<shared_ptr<Type> > const& args) = 0;
	};
	class StaticRtnApply : public FunctionApply {
	protected:
		shared_ptr<Type> _rtntype;
	public:
		StaticRtnApply(shared_ptr<Type> rtntype) : _rtntype(rtntype) {}
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			return _rtntype;
		}
	};
	class AdaptiveRtnApply : public FunctionApply {
	protected:
		shared_ptr<Type> _rtntype;
	public:
		AdaptiveRtnApply(shared_ptr<Type> rtntype) : _rtntype(rtntype) {}
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t =
					args.size() > 0 ?
							args.front() :
							dynamic_pointer_cast<Type>(make_shared<NilType>());
			if (t->id() <= Type::kString) {
				return _rtntype;
			} else if (t->id() == Type::kVector || t->id() == Type::kList
					|| t->id() == Type::kMatrix) {
				return dynamic_pointer_cast<ConType>(t)->withElemType(_rtntype);
			}
			return make_shared<AnyType>();
		}
	};
	class JoinApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t = make_shared<NilType>();
			FOR_EACH(it, args) {
				t = Type::join(t, (*it));
			}
			return t;
		}
	};
	class IfApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t = make_shared<NilType>();
			bool first = true;
			FOR_EACH(it, args) {
				if (first) {
					first = false;
				} else {
					t = Type::join(t, (*it));
				}
			}
			return t;
		}
	};
	class GenSeriesApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> et = make_shared<NilType>();
			FOR_EACH(it, args) {
				et = Type::join(et, (*it));
			}
			if (et->id() == Type::kInteger) {
				return make_shared<VecType>(make_shared<IntType>());
			} else if (et->id() == Type::kDouble) {
				return make_shared<VecType>(make_shared<DblType>());
			}
			// Should be type error
			return make_shared<AnyType>();
		}
	};
	class ConcatApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t = make_shared<NilType>();
			FOR_EACH(it, args) {
				t = Type::join(t, (*it));
			}
			if (t->id() <= Type::kString) {
				t = make_shared<VecType>(t);
			} else if (t->id() == Type::kMatrix) {
				t = dynamic_pointer_cast<MatType>(t)->baseType();
			}
			return t;
		}
	};
	class ListApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t = make_shared<NilType>();
			bool homo = true, first = true;
			FOR_EACH(it, args) {
				if (first) {
					t = *it;
					first = false;
				} else {
					if (Type::compare(t, *it) != 0) {
						homo = false;
						break;
					}
				}
			}
			return homo ?
					make_shared<LstType>(t) :
					make_shared<LstType>(make_shared<AnyType>());
		}
	};
	class MatrixApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t =
					args.size() > 0 ?
							args.front() :
							dynamic_pointer_cast<Type>(make_shared<NilType>());
			return make_shared<MatType>(t);
		}
	};
	class AssignApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			return args.at(1);
		}
	};
	class AsScalarApply : public FunctionApply {
	protected:
		shared_ptr<Type> _astype;
	public:
		AsScalarApply(shared_ptr<Type> astype) : _astype(astype) {}
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t =
					args.size() > 0 ?
							args.front() :
							dynamic_pointer_cast<Type>(make_shared<NilType>());
			if (t->id() <= Type::kString) {
				return _astype;
			} else if (t->id() == Type::kVector || t->id() == Type::kList
					|| t->id() == Type::kMatrix) {
				return make_shared<VecType>(_astype);
			}
			return make_shared<AnyType>();
		}
	};
	class SingleArgApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			return args.size() > 0 ?
					args.front() :
					dynamic_pointer_cast<Type>(make_shared<NilType>());
		}
	};
	class SubsettingApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			// TODO(jianqiao): currently only deal with 1-dimensional subsetting
			if (args.size() == 2) {
				shared_ptr<Type> h = args[0];
				shared_ptr<Type> a = args[1];
				if (h->id() == Type::kMatrix) {
					vector<shared_ptr<Type> > nargs = args;
					nargs[0] =
							dynamic_pointer_cast<MatType>(args[0])->baseType();
					return infer(nargs);
				} else if (a->id() == Type::kNull) {
					return h;
				} else if (a->id() == Type::kInteger
						|| a->id() == Type::kDouble) {
					if (h->id() <= Type::kString || h->id() == Type::kList) {
						return h;
					} else if (h->id() == Type::kVector) {
						return dynamic_pointer_cast<VecType>(h)->elemType();
					}
				} else if (a->id() == Type::kVector) {
					return h;
				}
			}
			return make_shared<AnyType>();

		}
	};
	class SingleAccessApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			// Should report type error if != 2 args
			if (args.size() == 2) {
				shared_ptr<Type> h = args[0];
				shared_ptr<Type> a = args[1];
				if (a->id() == Type::kInteger || a->id() == Type::kDouble) {
					if (h->id() <= Type::kString) {
						return h;
					} else if (h->id() == Type::kVector
							|| h->id() == Type::kList) {
						return dynamic_pointer_cast<ConType>(h)->elemType();
					}
				}
			}
			return make_shared<AnyType>();
		}
	};
	class MakeVecApply : public FunctionApply {
	public:
		shared_ptr<Type> infer(vector<shared_ptr<Type> > const& args) {
			shared_ptr<Type> t =
					args.size() > 0 ?
							args.front() :
							dynamic_pointer_cast<Type>(make_shared<NilType>());
			if (t->id() <= Type::kString) {
				return make_shared<VecType>(t);
			} else {
				return t;
			}
		}
	};

	int curstmt;
	map<int, map<int, shared_ptr<Type> > > symtypes;
	map<int, map<SEXP, shared_ptr<Type> > > exprtypes;
	map<string, shared_ptr<FunctionApply> > fns;
	int level;

	shared_ptr<Type> infer(SEXP expr) {
		shared_ptr<Type> t;
		switch (TYPEOF(expr)) {
		case NILSXP:
			t = make_shared<NilType>();
			break;
		case CHARSXP:
			t = make_shared<StrType>();
			break;
		case LGLSXP:
			if (XLENGTH(expr) == 1) {
				t = make_shared<LglType>();
			} else {
				t = make_shared<VecType>(make_shared<LglType>());
			}
			break;
		case INTSXP:
			if (XLENGTH(expr) == 1) {
				t = make_shared<IntType>();
			} else {
				t = make_shared<VecType>(make_shared<IntType>());
			}
			break;
		case REALSXP: {
			bool isInt = true;
			int len = XLENGTH(expr);
			for (int i = 0; i < len; i++) {
				double value = REAL(expr)[i], intpart;
				if (modf(value, &intpart) != 0.0) {
					isInt = false;
				}
			}
			if (len == 1) {
				if (isInt) {
					t = make_shared<IntType>();
				} else {
					t = make_shared<DblType>();
				}
			} else {
				if (isInt) {
					t = make_shared<VecType>(make_shared<IntType>());
				} else {
					t = make_shared<VecType>(make_shared<DblType>());
				}
			}
			break;
		}
		case SYMSXP:
			t = inferSymbol(expr);
			break;
		case LANGSXP:
			t = inferLang(expr);
			break;
		case STRSXP:
			if (XLENGTH(expr) == 1) {
				t = make_shared<StrType>();
			} else {
				t = make_shared<VecType>(make_shared<StrType>());
			}
			break;

		case CLOSXP:
		case ENVSXP:
		case PROMSXP:
		case SPECIALSXP:
		case BUILTINSXP:
		case CPLXSXP:
		case DOTSXP:
		case ANYSXP:
		case VECSXP:
		case EXPRSXP:
		case BCODESXP:
		case EXTPTRSXP:
		case WEAKREFSXP:
		case RAWSXP:
		case S4SXP:
		case NEWSXP:
		case FREESXP:
		//case FUNSXP:
		default:
			t = make_shared<AnyType>();
			break;
		}

		exprtypes[curstmt][expr] = t;
		return t;
	}

	void print(SEXP expr) {
		shared_ptr<Type> t = exprtypes[curstmt][expr];
		level++;
		for (int i = 0; i < level-1; i++) {
			printf("  ");
		}
		printf("[");
		switch (TYPEOF(expr)) {
		case NILSXP:	printf("NILSXP"); break;
		case CHARSXP:	printf("CHARSXP"); break;
		case LGLSXP:	printf("LGLSXP"); break;
		case INTSXP:	printf("INTSXP"); break;
		case REALSXP:	printf("REALSXP"); break;
		case SYMSXP:	printf("SYMSXP"); break;
		case LANGSXP:	printf("LANGSXP"); break;
		case STRSXP:	printf("STRSXP"); break;
		default:		printf("Other SXP"); break;
		}
		printf(", %s] ", t->toString().c_str());
		Rf_PrintValue(expr);

		if (TYPEOF(expr) == LANGSXP) {
			string fn = CHAR(PRINTNAME(CAR(expr)));
			if (fn == "for") {
				print(CADR(expr));
				print(CADDR(expr));
			} else if (fns.find(fn) != fns.end()) {
				SEXP lc = CDR(expr);
				while (lc != R_NilValue) {
					print(CAR(lc));
					lc = CDR(lc);
				}
			}
		}

		level--;
	}

	shared_ptr<Type> inferSymbol(SEXP v) {
		shared_ptr<Type> t = make_shared<NilType>();
		const char* vname = CHAR(PRINTNAME(v));
		if (strcmp(vname, "") != 0) {
			int var = find_var_id(vname);
			if (var >= 0) {
				auto const& symdefs = stmt_list[curstmt].reachdef_in[var];
				FOR_EACH(pstmtid, symdefs)
				{
					shared_ptr<Type> st = symtypes[*pstmtid][var];
					if (!st) {
						st = make_shared<NilType>();
						symtypes[*pstmtid][var] = st;
					}
					t = Type::join(t, st);
				}
			}
		}
		return t;
	}

	shared_ptr<Type> inferLang(SEXP v) {
		string fn = CHAR(PRINTNAME(CAR(v)));

		// handle for loop
		if (fn == "for") {
			int var = find_var_id(CHAR(PRINTNAME(CADR(v))));
			shared_ptr<Type> t = infer(CADDR(v));
			shared_ptr<Type> et = make_shared<AnyType>();
			if (t->id() == Type::kVector || t->id() == Type::kList) {
				et = dynamic_pointer_cast<ConType>(t)->elemType();
			}
			symtypes[curstmt][var] = et;
			exprtypes[curstmt][CADR(v)] = et;
			return make_shared<AnyType>();
		}

		// handle pre-defined functions
		auto const& pf = fns.find(fn);
		if (pf == fns.end()) {
			return make_shared<AnyType>();
		}
		vector<shared_ptr<Type>> argtypes;
		SEXP lc = CDR(v);
		while (lc != R_NilValue) {
			argtypes.push_back(infer(CAR(lc)));
		    lc = CDR(lc);
		}
		shared_ptr<Type> t = pf->second->infer(argtypes);
		// apply side effect: update symbol table
		if (fn == "<-" || fn == "=") {
			// TODO(jianqiao): should extend to deal with access uplifting
			// i.e. s <- c(1,2)
			//      s[1] <- "a"
			SEXP lv = CADR(v);
			if (TYPEOF(lv) == SYMSXP) {
				int var = find_var_id(CHAR(PRINTNAME(lv)));
				symtypes[curstmt][var] = t;
				printf("%d %d %s %s\n", curstmt, var, rvars[var],
						t->toString().c_str());
			} else {
				constructSymType(lv, Type::join(argtypes.front(), t));
			}
		}
		return t;
	}

	void constructSymType(SEXP v, shared_ptr<Type> t) {
		if (TYPEOF(v) == SYMSXP) {
			int var = find_var_id(CHAR(PRINTNAME(v)));
			symtypes[curstmt][var] = t;
		} else if (TYPEOF(v) == LANGSXP) {
			string fn = CHAR(PRINTNAME(CAR(v)));
			if (fn == "[" || fn == "[[") {
				SEXP symexpr = CADR(v);
				shared_ptr<Type> symt = infer(symexpr);
				int arglen = 0;
				for (SEXP lc = CDDR(v); lc != R_NilValue; lc = CDR(lc)) {
					arglen++;
				}

				// Currently only deal with 1-dimensional accessing
				if (arglen != 1) {
					return;
				}
				shared_ptr<Type> argt = infer(CADDR(v));
				// a[] <- ...
				if (argt->id() == Type::kNull) {
					if (t->id() <= Type::kString
							&& (symt->id() == Type::kVector
									|| symt->id() == Type::kList
									|| symt->id() == Type::kMatrix)) {
						t = dynamic_pointer_cast<ConType>(symt)->withElemType(
								t);
					} else if (symt->id() == Type::kMatrix) {
						t = make_shared<MatType>(t);
					}
					constructSymType(symexpr, t);
					return;
				}
				if (symt->id() <= Type::kString) {
					constructSymType(symexpr, make_shared<VecType>(t));
				} else if (argt->id() <= Type::kString
						&& (symt->id() == Type::kVector
								|| symt->id() == Type::kList
								|| symt->id() == Type::kMatrix)) {
					constructSymType(symexpr,
							dynamic_pointer_cast<ConType>(symt)->withElemType(
									t));
				} else if (argt->id() == Type::kVector) {
					constructSymType(symexpr, t);
				}
			}
		}
	}

public:
	TypeInference() : curstmt(0), level(0) {
		// Preparing for type inferencing
		fns["c"] = make_shared<ConcatApply>();
		fns["list"] = make_shared<ListApply>();
		fns["<-"] = make_shared<AssignApply>();
		fns["="] = make_shared<AssignApply>();
		fns["["] = make_shared<SubsettingApply>();
		fns["[["] = make_shared<SingleAccessApply>();
		fns["{"] = make_shared<SingleArgApply>();
		fns["return"] = make_shared<SingleArgApply>();
		fns[":"] = make_shared<GenSeriesApply>();
		fns["matrix"] = make_shared<MatrixApply>();
		fns["as.logical"] = make_shared<AsScalarApply>(make_shared<LglType>());
		fns["as.integer"] = make_shared<AsScalarApply>(make_shared<IntType>());
		fns["as.double"] = make_shared<AsScalarApply>(make_shared<DblType>());
		fns["if"] = make_shared<IfApply>();

		// Simplified function-apply logic here, no type-error checking
		vector<string> joinfns = {"+", "-", "*", "^", "("};
		FOR_EACH(ps, joinfns) {
			fns[*ps] = make_shared<JoinApply>();
		}
		vector<string> makevecfns = { "sample", "rep" };
		FOR_EACH(ps, makevecfns) {
			fns[*ps] = make_shared<MakeVecApply>();
		}
		vector<string> rtnintfns = { "length", "floor", "nrow", "ncol" };
		FOR_EACH(ps, rtnintfns) {
			fns[*ps] = make_shared<StaticRtnApply>(make_shared<IntType>());
		}
		vector<string> rtndblvecfns = { "numeric", "rnorm", "rbeta", "runif" };
		FOR_EACH(ps, rtndblvecfns) {
			fns[*ps] = make_shared<StaticRtnApply>(
					make_shared<VecType>(make_shared<DblType>()));
		}
		vector<string> rtndblfns = { "sqrt", "/" };
		FOR_EACH(ps, rtndblfns) {
			fns[*ps] = make_shared<AdaptiveRtnApply>(make_shared<DblType>());
		}
		fns["paste"] = make_shared<StaticRtnApply>(make_shared<StrType>());
	}

	void inferTypes() {
		printf("Type Inferencing\n");

		int num_iter = 0;
		bool changed = true;
		// iterate until a fixpoint is reached
		while (changed && num_iter < 100) {
			map<int, map<int, shared_ptr<Type> > > oldtypes = symtypes;
			for (int i = 0; i < global_stmt_cnt; i++) {
				if (stmt_list[i].expr != NULL) {
					curstmt = i;
					level = 0;
					infer(stmt_list[i].expr);
				} else {
				}
			}
			num_iter++;

			// Check if there is an update
			changed = false;
			FOR_EACH(psym, symtypes) {
				auto const& poldsym = oldtypes.find(psym->first);
				if (poldsym == oldtypes.end()) {
					changed = true;
				} else {
					FOR_EACH(pvar, psym->second) {
						auto const &poldvar = poldsym->second.find(pvar->first);
						if (poldvar == poldsym->second.end()) {
							changed = true;
						} else {
							changed = (Type::compare(pvar->second,
									poldvar->second) != 0);
						}
					}
				}
				if (changed) break;
			}
		}
		printf("Number of iterations = %d\n\n", num_iter);
	}

	void printAST() {
		printf("Type inferencing AST: \n");
		for (int i = 0; i < global_stmt_cnt; i++) {
			printf("-- Stmt %d --\n", i);
			if (stmt_list[i].expr != NULL) {
				curstmt = i;
				level = 0;
				print(stmt_list[i].expr);
			} else {
			}
		}
		printf("\n");
	}

	void printVarTypes() {
		printf("Inferred variable types: \n");
		for(int i = 0; i < global_stmt_cnt; i++) {
			printf("%d: ", i);
			FOR_EACH(it, stmt_list[i].kill) {
				shared_ptr<Type> t = symtypes[i][*it];
				if (!t) t = make_shared<NilType>();
				printf("%s:%s ", rvars[*it], t->toString().c_str());
			}
			printf("\n");
		}
	}
};

}

/* public functions */
extern "C" {
SEXP analyze(SEXP qexpr, SEXP expr, SEXP rho) {
	int i;
	R_start_sa();
//struct timeval s1,s2,s3;
//gettimeofday(&s1,NULL);
	function_t fn;
	fn.name="main";
	fn.formals=NILSXP;
	fn.body=expr;
	function_list.push_back(fn);
	Rf_ReplIteration_process(rho);
//gettimeofday(&s2,NULL);
//printf("CFG construction Time taken = %g secs\n",(s2.tv_sec-s1.tv_sec)+(s2.tv_usec-s1.tv_usec)/1e6);
	PrintCFG();	
	PrintGenKill();
	PrintSingleGen();
	LiveVarAnalysis();
	PrintUseNotLive();
	for(int i=1;i<=2;i++) {
		AliasAnalysis();
		AddFnEdges();
	}
	CalcAndPrintSingleGenNotLiveOut();
	CalcAndPrintDefNoLiveAlias();

	ReachDefAnalysis();
	typeinfer::TypeInference ti;
	ti.inferTypes();
	ti.printAST();
	ti.printVarTypes();

	// touch "codegen.txt" if you uncomment these lines
	//CodeMotionAnalysis();
	//CodeGen(expr);

//gettimeofday(&s3,NULL);
//printf("Analysis Time taken = %g secs\n",(s3.tv_sec-s2.tv_sec)+(s3.tv_usec-s2.tv_usec)/1e6);
	printf("evaluating...\n");
	fflush(stdout);
	struct timeval t1,t2;
	struct rusage u1,u2;
	getrusage(RUSAGE_SELF, &u1);
	gettimeofday(&t1,NULL);
	//Rf_PrintValue(Rf_eval(qexpr,rho));
	SEXP result=(Rf_eval(qexpr,rho));
	gettimeofday(&t2,NULL);
	getrusage(RUSAGE_SELF, &u2);
	printf("Time taken = %g secs\n",(t2.tv_sec-t1.tv_sec)+(t2.tv_usec-t1.tv_usec)/1e6);
	double delta=u2.ru_maxrss-u1.ru_maxrss;
	printf("RSS: %g %g MB\n",u1.ru_maxrss/1024.0,u2.ru_maxrss/1024.0);
	if(delta>1048576)
		printf("Delta Max RSS = %.1f GB\n",delta/1048576.0);
	else if(delta>1024)
		printf("Delta Max RSS = %.1f MB\n",delta/1024.0);
	else
		printf("Delta Max RSS = %.1f KB\n",delta/1.0);
	R_stop_sa();
/*	SEXP result=R_NilValue;
	PROTECT(result=allocVector(STRSXP,global_stmt_cnt));
	for(i=0;i<global_stmt_cnt;i++) {
		SET_STRING_ELT(result,i,mkChar(labels[i]));
	}
	UNPROTECT(1);*/

//	fclose(codegen_fp);
//	char buffer[10000];
//	sprintf(buffer,"bin/R -f codegen.txt > codegen_results.txt");
//	system(buffer);
	return result;
}

SEXP get_successor_list(SEXP node_id) {
	int j,num_succ;
	int i=INTEGER(node_id)[0];
	SEXP result;
	i--;
	assert(i>=0);
	assert(i<global_stmt_cnt);
	num_succ=stmt_list[i].succ.size();
	result=allocVector(INTSXP,num_succ);
	set<int>::iterator it;
	for(j=0,it=stmt_list[i].succ.begin();it!=stmt_list[i].succ.end();it++,j++) {
		INTEGER(result)[j]=((*it)+1);
	}
	return result;
}
}

#include "known_fn.cpp"
