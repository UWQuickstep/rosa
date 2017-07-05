library(Rgraphviz)

cfg_create=function(labels) {
	num_nodes=length(labels)
	adj_matrix=matrix(seq(0,0,length.out=num_nodes*num_nodes),num_nodes,num_nodes)
	rownames(adj_matrix)=labels
	colnames(adj_matrix)=labels
	adj_matrix
}

cfg_add_edge=function(adj_matrix,node,succ_list) {
	num_elts=length(succ_list)
	for(i in 1:num_elts) {
		succ=succ_list[i]
		adj_matrix[node,succ]=1
	}
	adj_matrix
}

cfg_plot=function(adj_matrix,file_name=NULL) {
	cfg=new("graphAM", adjMat=adj_matrix, edgemode="directed")
#	plot(cfg,attrs = list(node = list(fillcolor = "lightblue",fontsize=14,width=1.6),edge = list(arrowsize=0.5)))
	plot(cfg,attrs = list(node = list(fillcolor = "lightblue",fontsize=44,width=1.6),edge = list(arrowsize=0.5)))
}

#nodes=c("a","b","c","d","e")
#adj_matrix=cfg_create(nodes)
#adj_matrix=cfg_add_edge(adj_matrix,1,c(2,3))
#adj_matrix=cfg_add_edge(adj_matrix,2,c(4))
#cfg_plot(adj_matrix)
