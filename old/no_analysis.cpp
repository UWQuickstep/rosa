//#include <R.h>
//#include <Rinternals.h>

#include "static_analysis.h"

/* public functions */
extern "C" {
SEXP analyze(SEXP qexpr, SEXP expr, SEXP rho) {
	printf("evaluating...\n");
	fflush(stdout);
	struct timeval t1,t2;
	struct rusage u1,u2;
	getrusage(RUSAGE_SELF, &u1);
	gettimeofday(&t1,NULL);
	Rf_PrintValue(Rf_eval(qexpr,rho));
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
	SEXP result=R_NilValue;
	return result;
}
}

