#source("./static_analysis/cfg_plot.R")
dyn.load("./static_analysis/static_analysis.so");

analyze=function(qcode,code,env) {
	.Call("analyze",qcode,code,env)	
#	nodes=.Call("analyze",qcode,code,env)	
#	adj_matrix=cfg_create(nodes)
#	num_nodes=length(nodes)
#	for(i in 1:num_nodes) {
#		succ_list=.Call("get_successor_list",i)
#		adj_matrix=cfg_add_edge(adj_matrix,i,succ_list)
#	}
#	cfg_plot(adj_matrix)
}

