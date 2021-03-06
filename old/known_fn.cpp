bool is_known_function(const char *name) {
	if(!strcmp(name,"if")) return true;
	if(!strcmp(name,"while")) return true;
	if(!strcmp(name,"for")) return true;
	if(!strcmp(name,"repeat")) return true;
	if(!strcmp(name,"break")) return true;
	if(!strcmp(name,"next")) return true;
	if(!strcmp(name,"return")) return true;
	if(!strcmp(name,"function")) return true;
	if(!strcmp(name,"<-")) return true;
	if(!strcmp(name,"=")) return true;
	if(!strcmp(name,"<<-")) return true;
	if(!strcmp(name,"")) return true;
	if(!strcmp(name,"(")) return true;
	if(!strcmp(name,".subset")) return true;
	if(!strcmp(name,".subset2")) return true;
	if(!strcmp(name,"[")) return true;
	if(!strcmp(name,"[[")) return true;
	if(!strcmp(name,"$")) return true;
	if(!strcmp(name,"@")) return true;
	if(!strcmp(name,"[<-")) return true;
	if(!strcmp(name,"[[<-")) return true;
	if(!strcmp(name,"$<-")) return true;
	if(!strcmp(name,"switch")) return true;
	if(!strcmp(name,"browser")) return true;
	if(!strcmp(name,".primTrace")) return true;
	if(!strcmp(name,".primUntrace")) return true;
	if(!strcmp(name,".Internal")) return true;
	if(!strcmp(name,".Primitive")) return true;
	if(!strcmp(name,"call")) return true;
	if(!strcmp(name,"quote")) return true;
	if(!strcmp(name,"substitute")) return true;
	if(!strcmp(name,"missing")) return true;
	if(!strcmp(name,"nargs")) return true;
	if(!strcmp(name,"on.exit")) return true;
	if(!strcmp(name,"stop")) return true;
	if(!strcmp(name,"warning")) return true;
	if(!strcmp(name,"gettext")) return true;
	if(!strcmp(name,"ngettext")) return true;
	if(!strcmp(name,"bindtextdomain")) return true;
	if(!strcmp(name,".addCondHands")) return true;
	if(!strcmp(name,".resetCondHands")) return true;
	if(!strcmp(name,".signalCondition")) return true;
	if(!strcmp(name,".dfltStop")) return true;
	if(!strcmp(name,".dfltWarn")) return true;
	if(!strcmp(name,".addRestart")) return true;
	if(!strcmp(name,".getRestart")) return true;
	if(!strcmp(name,".invokeRestart")) return true;
	if(!strcmp(name,".addTryHandlers")) return true;
	if(!strcmp(name,"geterrmessage")) return true;
	if(!strcmp(name,"seterrmessage")) return true;
	if(!strcmp(name,"printDeferredWarnings")) return true;
	if(!strcmp(name,"interruptsSuspended")) return true;
	if(!strcmp(name,"restart")) return true;
	if(!strcmp(name,"as.function.default")) return true;
	if(!strcmp(name,"debug")) return true;
	if(!strcmp(name,"undebug")) return true;
	if(!strcmp(name,"isdebugged")) return true;
	if(!strcmp(name,"debugonce")) return true;
	if(!strcmp(name,"Recall")) return true;
	if(!strcmp(name,"delayedAssign")) return true;
	if(!strcmp(name,"makeLazy")) return true;
	if(!strcmp(name,"identical")) return true;
	if(!strcmp(name,"+")) return true;
	if(!strcmp(name,"-")) return true;
	if(!strcmp(name,"*")) return true;
	if(!strcmp(name,"/")) return true;
	if(!strcmp(name,"^")) return true;
	if(!strcmp(name,"%%")) return true;
	if(!strcmp(name,"%/%")) return true;
	if(!strcmp(name,"%*%")) return true;
	if(!strcmp(name,"==")) return true;
	if(!strcmp(name,"!=")) return true;
	if(!strcmp(name,"<")) return true;
	if(!strcmp(name,"<=")) return true;
	if(!strcmp(name,">=")) return true;
	if(!strcmp(name,">")) return true;
	if(!strcmp(name,"&")) return true;
	if(!strcmp(name,"|")) return true;
	if(!strcmp(name,"!")) return true;
	if(!strcmp(name,"&&")) return true;
	if(!strcmp(name,"||")) return true;
	if(!strcmp(name,":")) return true;
	if(!strcmp(name,"~")) return true;
	if(!strcmp(name,"all")) return true;
	if(!strcmp(name,"any")) return true;
	if(!strcmp(name,"length")) return true;
	if(!strcmp(name,"length<-")) return true;
	if(!strcmp(name,"c")) return true;
	if(!strcmp(name,"oldClass")) return true;
	if(!strcmp(name,"oldClass<-")) return true;
	if(!strcmp(name,"class")) return true;
	if(!strcmp(name,".cache_class")) return true;
	if(!strcmp(name,"class<-")) return true;
	if(!strcmp(name,"unclass")) return true;
	if(!strcmp(name,"names")) return true;
	if(!strcmp(name,"names<-")) return true;
	if(!strcmp(name,"dimnames")) return true;
	if(!strcmp(name,"dimnames<-")) return true;
	if(!strcmp(name,"dim")) return true;
	if(!strcmp(name,"dim<-")) return true;
	if(!strcmp(name,"attributes")) return true;
	if(!strcmp(name,"attributes<-")) return true;
	if(!strcmp(name,"attr")) return true;
	if(!strcmp(name,"attr<-")) return true;
	if(!strcmp(name,"@<-")) return true;
	if(!strcmp(name,"levels<-")) return true;
	if(!strcmp(name,"vector")) return true;
	if(!strcmp(name,"complex")) return true;
	if(!strcmp(name,"matrix")) return true;
	if(!strcmp(name,"array")) return true;
	if(!strcmp(name,"diag")) return true;
	if(!strcmp(name,"backsolve")) return true;
	if(!strcmp(name,"max.col")) return true;
	if(!strcmp(name,"row")) return true;
	if(!strcmp(name,"col")) return true;
	if(!strcmp(name,"unlist")) return true;
	if(!strcmp(name,"cbind")) return true;
	if(!strcmp(name,"rbind")) return true;
	if(!strcmp(name,"drop")) return true;
	if(!strcmp(name,"all.names")) return true;
	if(!strcmp(name,"comment")) return true;
	if(!strcmp(name,"comment<-")) return true;
	if(!strcmp(name,"get")) return true;
	if(!strcmp(name,"mget")) return true;
	if(!strcmp(name,"exists")) return true;
	if(!strcmp(name,"assign")) return true;
	if(!strcmp(name,"list2env")) return true;
	if(!strcmp(name,"remove")) return true;
	if(!strcmp(name,"duplicated")) return true;
	if(!strcmp(name,"unique")) return true;
	if(!strcmp(name,"anyDuplicated")) return true;
	if(!strcmp(name,"anyNA")) return true;
	if(!strcmp(name,"which")) return true;
	if(!strcmp(name,"which.min")) return true;
	if(!strcmp(name,"pmin")) return true;
	if(!strcmp(name,"pmax")) return true;
	if(!strcmp(name,"which.max")) return true;
	if(!strcmp(name,"match")) return true;
	if(!strcmp(name,"pmatch")) return true;
	if(!strcmp(name,"charmatch")) return true;
	if(!strcmp(name,"match.call")) return true;
	if(!strcmp(name,"crossprod")) return true;
	if(!strcmp(name,"tcrossprod")) return true;
	if(!strcmp(name,"attach")) return true;
	if(!strcmp(name,"detach")) return true;
	if(!strcmp(name,"search")) return true;
	if(!strcmp(name,"setFileTime")) return true;
	if(!strcmp(name,"round")) return true;
	if(!strcmp(name,"signif")) return true;
	if(!strcmp(name,"log")) return true;
	if(!strcmp(name,"log10")) return true;
	if(!strcmp(name,"log2")) return true;
	if(!strcmp(name,"abs")) return true;
	if(!strcmp(name,"floor")) return true;
	if(!strcmp(name,"ceiling")) return true;
	if(!strcmp(name,"sqrt")) return true;
	if(!strcmp(name,"sign")) return true;
	if(!strcmp(name,"trunc")) return true;
	if(!strcmp(name,"exp")) return true;
	if(!strcmp(name,"expm1")) return true;
	if(!strcmp(name,"log1p")) return true;
	if(!strcmp(name,"cos")) return true;
	if(!strcmp(name,"sin")) return true;
	if(!strcmp(name,"tan")) return true;
	if(!strcmp(name,"acos")) return true;
	if(!strcmp(name,"asin")) return true;
	if(!strcmp(name,"atan")) return true;
	if(!strcmp(name,"cosh")) return true;
	if(!strcmp(name,"sinh")) return true;
	if(!strcmp(name,"tanh")) return true;
	if(!strcmp(name,"acosh")) return true;
	if(!strcmp(name,"asinh")) return true;
	if(!strcmp(name,"atanh")) return true;
	if(!strcmp(name,"lgamma")) return true;
	if(!strcmp(name,"gamma")) return true;
	if(!strcmp(name,"digamma")) return true;
	if(!strcmp(name,"trigamma")) return true;
	if(!strcmp(name,"cospi")) return true;
	if(!strcmp(name,"sinpi")) return true;
	if(!strcmp(name,"tanpi")) return true;
	if(!strcmp(name,"atan2")) return true;
	if(!strcmp(name,"lbeta")) return true;
	if(!strcmp(name,"beta")) return true;
	if(!strcmp(name,"lchoose")) return true;
	if(!strcmp(name,"choose")) return true;
	if(!strcmp(name,"dchisq")) return true;
	if(!strcmp(name,"pchisq")) return true;
	if(!strcmp(name,"qchisq")) return true;
	if(!strcmp(name,"dexp")) return true;
	if(!strcmp(name,"pexp")) return true;
	if(!strcmp(name,"qexp")) return true;
	if(!strcmp(name,"dgeom")) return true;
	if(!strcmp(name,"pgeom")) return true;
	if(!strcmp(name,"qgeom")) return true;
	if(!strcmp(name,"dpois")) return true;
	if(!strcmp(name,"ppois")) return true;
	if(!strcmp(name,"qpois")) return true;
	if(!strcmp(name,"dt")) return true;
	if(!strcmp(name,"pt")) return true;
	if(!strcmp(name,"qt")) return true;
	if(!strcmp(name,"dsignrank")) return true;
	if(!strcmp(name,"psignrank")) return true;
	if(!strcmp(name,"qsignrank")) return true;
	if(!strcmp(name,"besselJ")) return true;
	if(!strcmp(name,"besselY")) return true;
	if(!strcmp(name,"psigamma")) return true;
	if(!strcmp(name,"Re")) return true;
	if(!strcmp(name,"Im")) return true;
	if(!strcmp(name,"Mod")) return true;
	if(!strcmp(name,"Arg")) return true;
	if(!strcmp(name,"Conj")) return true;
	if(!strcmp(name,"dbeta")) return true;
	if(!strcmp(name,"pbeta")) return true;
	if(!strcmp(name,"qbeta")) return true;
	if(!strcmp(name,"dbinom")) return true;
	if(!strcmp(name,"pbinom")) return true;
	if(!strcmp(name,"qbinom")) return true;
	if(!strcmp(name,"dcauchy")) return true;
	if(!strcmp(name,"pcauchy")) return true;
	if(!strcmp(name,"qcauchy")) return true;
	if(!strcmp(name,"df")) return true;
	if(!strcmp(name,"pf")) return true;
	if(!strcmp(name,"qf")) return true;
	if(!strcmp(name,"dgamma")) return true;
	if(!strcmp(name,"pgamma")) return true;
	if(!strcmp(name,"qgamma")) return true;
	if(!strcmp(name,"dlnorm")) return true;
	if(!strcmp(name,"plnorm")) return true;
	if(!strcmp(name,"qlnorm")) return true;
	if(!strcmp(name,"dlogis")) return true;
	if(!strcmp(name,"plogis")) return true;
	if(!strcmp(name,"qlogis")) return true;
	if(!strcmp(name,"dnbinom")) return true;
	if(!strcmp(name,"pnbinom")) return true;
	if(!strcmp(name,"qnbinom")) return true;
	if(!strcmp(name,"dnorm")) return true;
	if(!strcmp(name,"pnorm")) return true;
	if(!strcmp(name,"qnorm")) return true;
	if(!strcmp(name,"dunif")) return true;
	if(!strcmp(name,"punif")) return true;
	if(!strcmp(name,"qunif")) return true;
	if(!strcmp(name,"dweibull")) return true;
	if(!strcmp(name,"pweibull")) return true;
	if(!strcmp(name,"qweibull")) return true;
	if(!strcmp(name,"dnchisq")) return true;
	if(!strcmp(name,"pnchisq")) return true;
	if(!strcmp(name,"qnchisq")) return true;
	if(!strcmp(name,"dnt")) return true;
	if(!strcmp(name,"pnt")) return true;
	if(!strcmp(name,"qnt")) return true;
	if(!strcmp(name,"dwilcox")) return true;
	if(!strcmp(name,"pwilcox")) return true;
	if(!strcmp(name,"qwilcox")) return true;
	if(!strcmp(name,"besselI")) return true;
	if(!strcmp(name,"besselK")) return true;
	if(!strcmp(name,"dnbinom_mu")) return true;
	if(!strcmp(name,"pnbinom_mu")) return true;
	if(!strcmp(name,"qnbinom_mu")) return true;
	if(!strcmp(name,"dhyper")) return true;
	if(!strcmp(name,"phyper")) return true;
	if(!strcmp(name,"qhyper")) return true;
	if(!strcmp(name,"dnbeta")) return true;
	if(!strcmp(name,"pnbeta")) return true;
	if(!strcmp(name,"qnbeta")) return true;
	if(!strcmp(name,"dnf")) return true;
	if(!strcmp(name,"pnf")) return true;
	if(!strcmp(name,"qnf")) return true;
	if(!strcmp(name,"dtukey")) return true;
	if(!strcmp(name,"ptukey")) return true;
	if(!strcmp(name,"qtukey")) return true;
	if(!strcmp(name,"rchisq")) return true;
	if(!strcmp(name,"rexp")) return true;
	if(!strcmp(name,"rgeom")) return true;
	if(!strcmp(name,"rpois")) return true;
	if(!strcmp(name,"rt")) return true;
	if(!strcmp(name,"rsignrank")) return true;
	if(!strcmp(name,"rbeta")) return true;
	if(!strcmp(name,"rbinom")) return true;
	if(!strcmp(name,"rcauchy")) return true;
	if(!strcmp(name,"rf")) return true;
	if(!strcmp(name,"rgamma")) return true;
	if(!strcmp(name,"rlnorm")) return true;
	if(!strcmp(name,"rlogis")) return true;
	if(!strcmp(name,"rnbinom")) return true;
	if(!strcmp(name,"rnbinom_mu")) return true;
	if(!strcmp(name,"rnchisq")) return true;
	if(!strcmp(name,"rnorm")) return true;
	if(!strcmp(name,"runif")) return true;
	if(!strcmp(name,"rweibull")) return true;
	if(!strcmp(name,"rwilcox")) return true;
	if(!strcmp(name,"rhyper")) return true;
	if(!strcmp(name,"sample")) return true;
	if(!strcmp(name,"sample2")) return true;
	if(!strcmp(name,"RNGkind")) return true;
	if(!strcmp(name,"set.seed")) return true;
	if(!strcmp(name,"sum")) return true;
	if(!strcmp(name,"min")) return true;
	if(!strcmp(name,"max")) return true;
	if(!strcmp(name,"prod")) return true;
	if(!strcmp(name,"mean")) return true;
	if(!strcmp(name,"range")) return true;
	if(!strcmp(name,"cumsum")) return true;
	if(!strcmp(name,"cumprod")) return true;
	if(!strcmp(name,"cummax")) return true;
	if(!strcmp(name,"cummin")) return true;
	if(!strcmp(name,"as.character")) return true;
	if(!strcmp(name,"as.integer")) return true;
	if(!strcmp(name,"as.double")) return true;
	if(!strcmp(name,"as.complex")) return true;
	if(!strcmp(name,"as.logical")) return true;
	if(!strcmp(name,"as.raw")) return true;
	if(!strcmp(name,"as.call")) return true;
	if(!strcmp(name,"as.environment")) return true;
	if(!strcmp(name,"storage.mode<-")) return true;
	if(!strcmp(name,"as.vector")) return true;
	if(!strcmp(name,"paste")) return true;
	if(!strcmp(name,"paste0")) return true;
	if(!strcmp(name,"file.path")) return true;
	if(!strcmp(name,"format")) return true;
	if(!strcmp(name,"format.info")) return true;
	if(!strcmp(name,"cat")) return true;
	if(!strcmp(name,"do.call")) return true;
	if(!strcmp(name,"nchar")) return true;
	if(!strcmp(name,"nzchar")) return true;
	if(!strcmp(name,"substr")) return true;
	if(!strcmp(name,"substr<-")) return true;
	if(!strcmp(name,"strsplit")) return true;
	if(!strcmp(name,"abbreviate")) return true;
	if(!strcmp(name,"make.names")) return true;
	if(!strcmp(name,"grep")) return true;
	if(!strcmp(name,"grepl")) return true;
	if(!strcmp(name,"grepRaw")) return true;
	if(!strcmp(name,"sub")) return true;
	if(!strcmp(name,"gsub")) return true;
	if(!strcmp(name,"regexpr")) return true;
	if(!strcmp(name,"gregexpr")) return true;
	if(!strcmp(name,"regexec")) return true;
	if(!strcmp(name,"agrep")) return true;
	if(!strcmp(name,"agrepl")) return true;
	if(!strcmp(name,"adist")) return true;
	if(!strcmp(name,"aregexec")) return true;
	if(!strcmp(name,"tolower")) return true;
	if(!strcmp(name,"toupper")) return true;
	if(!strcmp(name,"chartr")) return true;
	if(!strcmp(name,"sprintf")) return true;
	if(!strcmp(name,"make.unique")) return true;
	if(!strcmp(name,"charToRaw")) return true;
	if(!strcmp(name,"rawToChar")) return true;
	if(!strcmp(name,"rawShift")) return true;
	if(!strcmp(name,"intToBits")) return true;
	if(!strcmp(name,"rawToBits")) return true;
	if(!strcmp(name,"packBits")) return true;
	if(!strcmp(name,"utf8ToInt")) return true;
	if(!strcmp(name,"intToUtf8")) return true;
	if(!strcmp(name,"encodeString")) return true;
	if(!strcmp(name,"iconv")) return true;
	if(!strcmp(name,"strtrim")) return true;
	if(!strcmp(name,"strtoi")) return true;
	if(!strcmp(name,"is.null")) return true;
	if(!strcmp(name,"is.logical")) return true;
	if(!strcmp(name,"is.integer")) return true;
	if(!strcmp(name,"is.double")) return true;
	if(!strcmp(name,"is.complex")) return true;
	if(!strcmp(name,"is.character")) return true;
	if(!strcmp(name,"is.symbol")) return true;
	if(!strcmp(name,"is.environment")) return true;
	if(!strcmp(name,"is.list")) return true;
	if(!strcmp(name,"is.pairlist")) return true;
	if(!strcmp(name,"is.expression")) return true;
	if(!strcmp(name,"is.raw")) return true;
	if(!strcmp(name,"is.object")) return true;
	if(!strcmp(name,"isS4")) return true;
	if(!strcmp(name,"is.numeric")) return true;
	if(!strcmp(name,"is.matrix")) return true;
	if(!strcmp(name,"is.array")) return true;
	if(!strcmp(name,"is.atomic")) return true;
	if(!strcmp(name,"is.recursive")) return true;
	if(!strcmp(name,"is.call")) return true;
	if(!strcmp(name,"is.language")) return true;
	if(!strcmp(name,"is.function")) return true;
	if(!strcmp(name,"is.single")) return true;
	if(!strcmp(name,"is.na")) return true;
	if(!strcmp(name,"is.nan")) return true;
	if(!strcmp(name,"is.finite")) return true;
	if(!strcmp(name,"is.infinite")) return true;
	if(!strcmp(name,"is.vector")) return true;
	if(!strcmp(name,"proc.time")) return true;
	if(!strcmp(name,"gc.time")) return true;
	if(!strcmp(name,"withVisible")) return true;
	if(!strcmp(name,"expression")) return true;
	if(!strcmp(name,"interactive")) return true;
	if(!strcmp(name,"invisible")) return true;
	if(!strcmp(name,"rep")) return true;
	if(!strcmp(name,"rep.int")) return true;
	if(!strcmp(name,"rep_len")) return true;
	if(!strcmp(name,"seq.int")) return true;
	if(!strcmp(name,"seq_len")) return true;
	if(!strcmp(name,"seq_along")) return true;
	if(!strcmp(name,"list")) return true;
	if(!strcmp(name,"xtfrm")) return true;
	if(!strcmp(name,"enc2native")) return true;
	if(!strcmp(name,"enc2utf8")) return true;
	if(!strcmp(name,"emptyenv")) return true;
	if(!strcmp(name,"baseenv")) return true;
	if(!strcmp(name,"globalenv")) return true;
	if(!strcmp(name,"environment<-")) return true;
	if(!strcmp(name,"pos.to.env")) return true;
	if(!strcmp(name,"eapply")) return true;
	if(!strcmp(name,"lapply")) return true;
	if(!strcmp(name,"vapply")) return true;
	if(!strcmp(name,"mapply")) return true;
	if(!strcmp(name,".C")) return true;
	if(!strcmp(name,".Fortran")) return true;
	if(!strcmp(name,".External")) return true;
	if(!strcmp(name,".External2")) return true;
	if(!strcmp(name,".Call")) return true;
	if(!strcmp(name,".External.graphics")) return true;
	if(!strcmp(name,".Call.graphics")) return true;
	if(!strcmp(name,"Version")) return true;
	if(!strcmp(name,"machine")) return true;
	if(!strcmp(name,"commandArgs")) return true;
	if(!strcmp(name,"system")) return true;
	if(!strcmp(name,"parse")) return true;
	if(!strcmp(name,"save")) return true;
	if(!strcmp(name,"saveToConn")) return true;
	if(!strcmp(name,"load")) return true;
	if(!strcmp(name,"loadFromConn2")) return true;
	if(!strcmp(name,"serializeToConn")) return true;
	if(!strcmp(name,"unserializeFromConn")) return true;
	if(!strcmp(name,"deparse")) return true;
	if(!strcmp(name,"dput")) return true;
	if(!strcmp(name,"dump")) return true;
	if(!strcmp(name,"quit")) return true;
	if(!strcmp(name,"readline")) return true;
	if(!strcmp(name,"print.default")) return true;
	if(!strcmp(name,"print.function")) return true;
	if(!strcmp(name,"prmatrix")) return true;
	if(!strcmp(name,"gc")) return true;
	if(!strcmp(name,"gcinfo")) return true;
	if(!strcmp(name,"gctorture")) return true;
	if(!strcmp(name,"gctorture2")) return true;
	if(!strcmp(name,"memory.profile")) return true;
	if(!strcmp(name,"split")) return true;
	if(!strcmp(name,"is.loaded")) return true;
	if(!strcmp(name,"recordGraphics")) return true;
	if(!strcmp(name,"dyn.load")) return true;
	if(!strcmp(name,"dyn.unload")) return true;
	if(!strcmp(name,"ls")) return true;
	if(!strcmp(name,"typeof")) return true;
	if(!strcmp(name,"eval")) return true;
	if(!strcmp(name,"sys.parent")) return true;
	if(!strcmp(name,"sys.call")) return true;
	if(!strcmp(name,"sys.frame")) return true;
	if(!strcmp(name,"sys.nframe")) return true;
	if(!strcmp(name,"sys.calls")) return true;
	if(!strcmp(name,"sys.frames")) return true;
	if(!strcmp(name,"sys.on.exit")) return true;
	if(!strcmp(name,"sys.parents")) return true;
	if(!strcmp(name,"sys.function")) return true;
	if(!strcmp(name,"traceback")) return true;
	if(!strcmp(name,"browserText")) return true;
	if(!strcmp(name,"browserCondition")) return true;
	if(!strcmp(name,"browserSetDebug")) return true;
	if(!strcmp(name,"parent.frame")) return true;
	if(!strcmp(name,"sort")) return true;
	if(!strcmp(name,"is.unsorted")) return true;
	if(!strcmp(name,"psort")) return true;
	if(!strcmp(name,"qsort")) return true;
	if(!strcmp(name,"radixsort")) return true;
	if(!strcmp(name,"order")) return true;
	if(!strcmp(name,"rank")) return true;
	if(!strcmp(name,"scan")) return true;
	if(!strcmp(name,"t.default")) return true;
	if(!strcmp(name,"aperm")) return true;
	if(!strcmp(name,"builtins")) return true;
	if(!strcmp(name,"args")) return true;
	if(!strcmp(name,"formals")) return true;
	if(!strcmp(name,"body")) return true;
	if(!strcmp(name,"bodyCode")) return true;
	if(!strcmp(name,"environment")) return true;
	if(!strcmp(name,"environmentName")) return true;
	if(!strcmp(name,"env2list")) return true;
	if(!strcmp(name,"reg.finalizer")) return true;
	if(!strcmp(name,"options")) return true;
	if(!strcmp(name,"sink")) return true;
	if(!strcmp(name,"sink.number")) return true;
	if(!strcmp(name,"rapply")) return true;
	if(!strcmp(name,"islistfactor")) return true;
	if(!strcmp(name,"colSums")) return true;
	if(!strcmp(name,"colMeans")) return true;
	if(!strcmp(name,"rowSums")) return true;
	if(!strcmp(name,"rowMeans")) return true;
	if(!strcmp(name,"tracemem")) return true;
	if(!strcmp(name,"retracemem")) return true;
	if(!strcmp(name,"untracemem")) return true;
	if(!strcmp(name,"inspect")) return true;
	if(!strcmp(name,"mem.limits")) return true;
	if(!strcmp(name,"merge")) return true;
	if(!strcmp(name,"capabilities")) return true;
	if(!strcmp(name,"capabilitiesX11")) return true;
	if(!strcmp(name,"new.env")) return true;
	if(!strcmp(name,"parent.env")) return true;
	if(!strcmp(name,"parent.env<-")) return true;
	if(!strcmp(name,"l10n_info")) return true;
	if(!strcmp(name,"Cstack_info")) return true;
	if(!strcmp(name,"file.show")) return true;
	if(!strcmp(name,"file.create")) return true;
	if(!strcmp(name,"file.remove")) return true;
	if(!strcmp(name,"file.rename")) return true;
	if(!strcmp(name,"file.append")) return true;
	if(!strcmp(name,"file.symlink")) return true;
	if(!strcmp(name,"file.link")) return true;
	if(!strcmp(name,"file.copy")) return true;
	if(!strcmp(name,"list.files")) return true;
	if(!strcmp(name,"list.dirs")) return true;
	if(!strcmp(name,"file.exists")) return true;
	if(!strcmp(name,"file.choose")) return true;
	if(!strcmp(name,"file.info")) return true;
	if(!strcmp(name,"file.access")) return true;
	if(!strcmp(name,"dir.create")) return true;
	if(!strcmp(name,"tempfile")) return true;
	if(!strcmp(name,"tempdir")) return true;
	if(!strcmp(name,"R.home")) return true;
	if(!strcmp(name,"date")) return true;
	if(!strcmp(name,"Sys.getenv")) return true;
	if(!strcmp(name,"Sys.setenv")) return true;
	if(!strcmp(name,"Sys.unsetenv")) return true;
	if(!strcmp(name,"getwd")) return true;
	if(!strcmp(name,"setwd")) return true;
	if(!strcmp(name,"basename")) return true;
	if(!strcmp(name,"dirname")) return true;
	if(!strcmp(name,"Sys.chmod")) return true;
	if(!strcmp(name,"Sys.umask")) return true;
	if(!strcmp(name,"Sys.readlink")) return true;
	if(!strcmp(name,"Sys.info")) return true;
	if(!strcmp(name,"Sys.sleep")) return true;
	if(!strcmp(name,"Sys.getlocale")) return true;
	if(!strcmp(name,"Sys.setlocale")) return true;
	if(!strcmp(name,"Sys.localeconv")) return true;
	if(!strcmp(name,"path.expand")) return true;
	if(!strcmp(name,"Sys.getpid")) return true;
	if(!strcmp(name,"normalizePath")) return true;
	if(!strcmp(name,"Sys.glob")) return true;
	if(!strcmp(name,"unlink")) return true;
	if(!strcmp(name,"polyroot")) return true;
	if(!strcmp(name,"inherits")) return true;
	if(!strcmp(name,"UseMethod")) return true;
	if(!strcmp(name,"NextMethod")) return true;
	if(!strcmp(name,"standardGeneric")) return true;
	if(!strcmp(name,"Sys.time")) return true;
	if(!strcmp(name,"as.POSIXct")) return true;
	if(!strcmp(name,"as.POSIXlt")) return true;
	if(!strcmp(name,"format.POSIXlt")) return true;
	if(!strcmp(name,"strptime")) return true;
	if(!strcmp(name,"Date2POSIXlt")) return true;
	if(!strcmp(name,"POSIXlt2Date")) return true;
	if(!strcmp(name,"mkCode")) return true;
	if(!strcmp(name,"bcClose")) return true;
	if(!strcmp(name,"is.builtin.internal")) return true;
	if(!strcmp(name,"disassemble")) return true;
	if(!strcmp(name,"bcVersion")) return true;
	if(!strcmp(name,"load.from.file")) return true;
	if(!strcmp(name,"save.to.file")) return true;
	if(!strcmp(name,"growconst")) return true;
	if(!strcmp(name,"putconst")) return true;
	if(!strcmp(name,"getconst")) return true;
	if(!strcmp(name,"enableJIT")) return true;
	if(!strcmp(name,"compilePKGS")) return true;
	if(!strcmp(name,"setNumMathThreads")) return true;
	if(!strcmp(name,"setMaxNumMathThreads")) return true;
	if(!strcmp(name,"stdin")) return true;
	if(!strcmp(name,"stdout")) return true;
	if(!strcmp(name,"stderr")) return true;
	if(!strcmp(name,"isatty")) return true;
	if(!strcmp(name,"readLines")) return true;
	if(!strcmp(name,"writeLines")) return true;
	if(!strcmp(name,"readBin")) return true;
	if(!strcmp(name,"writeBin")) return true;
	if(!strcmp(name,"readChar")) return true;
	if(!strcmp(name,"writeChar")) return true;
	if(!strcmp(name,"open")) return true;
	if(!strcmp(name,"isOpen")) return true;
	if(!strcmp(name,"isIncomplete")) return true;
	if(!strcmp(name,"isSeekable")) return true;
	if(!strcmp(name,"close")) return true;
	if(!strcmp(name,"flush")) return true;
	if(!strcmp(name,"file")) return true;
	if(!strcmp(name,"url")) return true;
	if(!strcmp(name,"pipe")) return true;
	if(!strcmp(name,"fifo")) return true;
	if(!strcmp(name,"gzfile")) return true;
	if(!strcmp(name,"bzfile")) return true;
	if(!strcmp(name,"xzfile")) return true;
	if(!strcmp(name,"unz")) return true;
	if(!strcmp(name,"seek")) return true;
	if(!strcmp(name,"truncate")) return true;
	if(!strcmp(name,"pushBack")) return true;
	if(!strcmp(name,"clearPushBack")) return true;
	if(!strcmp(name,"pushBackLength")) return true;
	if(!strcmp(name,"rawConnection")) return true;
	if(!strcmp(name,"rawConnectionValue")) return true;
	if(!strcmp(name,"textConnection")) return true;
	if(!strcmp(name,"textConnectionValue")) return true;
	if(!strcmp(name,"socketConnection")) return true;
	if(!strcmp(name,"sockSelect")) return true;
	if(!strcmp(name,"getConnection")) return true;
	if(!strcmp(name,"getAllConnections")) return true;
	if(!strcmp(name,"summary.connection")) return true;
	if(!strcmp(name,"gzcon")) return true;
	if(!strcmp(name,"memCompress")) return true;
	if(!strcmp(name,"memDecompress")) return true;
	if(!strcmp(name,"readDCF")) return true;
	if(!strcmp(name,"lockEnvironment")) return true;
	if(!strcmp(name,"environmentIsLocked")) return true;
	if(!strcmp(name,"lockBinding")) return true;
	if(!strcmp(name,"unlockBinding")) return true;
	if(!strcmp(name,"bindingIsLocked")) return true;
	if(!strcmp(name,"makeActiveBinding")) return true;
	if(!strcmp(name,"bindingIsActive")) return true;
	if(!strcmp(name,"mkUnbound")) return true;
	if(!strcmp(name,"isNamespaceEnv")) return true;
	if(!strcmp(name,"registerNamespace")) return true;
	if(!strcmp(name,"unregisterNamespace")) return true;
	if(!strcmp(name,"getRegisteredNamespace")) return true;
	if(!strcmp(name,"getNamespaceRegistry")) return true;
	if(!strcmp(name,"importIntoEnv")) return true;
	if(!strcmp(name,"env.profile")) return true;
	if(!strcmp(name,"Encoding")) return true;
	if(!strcmp(name,"setEncoding")) return true;
	if(!strcmp(name,"setTimeLimit")) return true;
	if(!strcmp(name,"setSessionTimeLimit")) return true;
	if(!strcmp(name,"icuSetCollate")) return true;
	if(!strcmp(name,"readRenviron")) return true;
	if(!strcmp(name,"shortRowNames")) return true;
	if(!strcmp(name,"copyDFattr")) return true;
	if(!strcmp(name,"getRegisteredRoutines")) return true;
	if(!strcmp(name,"getLoadedDLLs")) return true;
	if(!strcmp(name,"getSymbolInfo")) return true;
	if(!strcmp(name,".isMethodsDispatchOn")) return true;
	if(!strcmp(name,"lazyLoadDBfetch")) return true;
	if(!strcmp(name,"lazyLoadDBflush")) return true;
	if(!strcmp(name,"getVarsFromFrame")) return true;
	if(!strcmp(name,"lazyLoadDBinsertValue")) return true;
	if(!strcmp(name,"bincode")) return true;
	if(!strcmp(name,"tabulate")) return true;
	if(!strcmp(name,"findInterval")) return true;
	if(!strcmp(name,"pretty")) return true;
	if(!strcmp(name,"formatC")) return true;
	if(!strcmp(name,"crc64")) return true;
	if(!strcmp(name,"bitwiseAnd")) return true;
	if(!strcmp(name,"bitwiseNot")) return true;
	if(!strcmp(name,"bitwiseOr")) return true;
	if(!strcmp(name,"bitwiseXor")) return true;
	if(!strcmp(name,"bitwiseShiftL")) return true;
	if(!strcmp(name,"bitwiseShiftR")) return true;
	if(!strcmp(name,"serialize")) return true;
	if(!strcmp(name,"serializeb")) return true;
	if(!strcmp(name,"unserialize")) return true;
	if(!strcmp(name,"rowsum_matrix")) return true;
	if(!strcmp(name,"rowsum_df")) return true;
	if(!strcmp(name,"setS4Object")) return true;
	if(!strcmp(name,"traceOnOff")) return true;
	if(!strcmp(name,"La_qr_cmplx")) return true;
	if(!strcmp(name,"La_rs")) return true;
	if(!strcmp(name,"La_rs_cmplx")) return true;
	if(!strcmp(name,"La_rg")) return true;
	if(!strcmp(name,"La_rg_cmplx")) return true;
	if(!strcmp(name,"La_rs")) return true;
	if(!strcmp(name,"La_rs_cmplx")) return true;
	if(!strcmp(name,"La_dlange")) return true;
	if(!strcmp(name,"La_dgecon")) return true;
	if(!strcmp(name,"La_dtrcon")) return true;
	if(!strcmp(name,"La_zgecon")) return true;
	if(!strcmp(name,"La_ztrcon")) return true;
	if(!strcmp(name,"La_solve_cmplx")) return true;
	if(!strcmp(name,"La_solve")) return true;
	if(!strcmp(name,"La_qr")) return true;
	if(!strcmp(name,"La_chol")) return true;
	if(!strcmp(name,"La_chol2inv")) return true;
	if(!strcmp(name,"qr_coef_real")) return true;
	if(!strcmp(name,"qr_qy_real")) return true;
	if(!strcmp(name,"det_ge_real")) return true;
	if(!strcmp(name,"qr_coef_cmplx")) return true;
	if(!strcmp(name,"qr_qy_cmpl")) return true;
	if(!strcmp(name,"La_svd")) return true;
	if(!strcmp(name,"La_svd_cmplx")) return true;
	if(!strcmp(name,"La_version")) return true;
	return false;
}
