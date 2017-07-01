#' @export
binsearch = function() {

bsearch <- function(x,y) {
  n <- length(x)
  lo <- 1
  hi <- n
  while(lo+1 < hi) {
    mid <- floor((lo+hi)/2)
    if (y == x[mid]) return(mid)
    if (y < x[mid]) hi <- mid else lo <- mid
  }
  if (y <= x[lo]) return(lo)
  if (y < x[hi]) return(hi)
  return(hi+1)
}

nn=1e5          # set this to 1e6 for a large run
x <- sample (1:1000,nn,replace=TRUE)
y <- sample (1:1000,nn,replace=TRUE)
z <- numeric(nn)
for(i in 1:length(y)) z[i] <- bsearch(x,y[i])

}
