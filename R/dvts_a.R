#' @export
dvts_a <- function() {

preda <- function(x,k) {
  n <- length(x)
  k2 <- k/2
  pred <- vector(length=n-k)
  for(i in 1:(n-k)) {
    if(sum(x[i:(i+(k-1))]) >= k2) pred[i] <- 1 else pred[i] <- 0
  }
  return(mean(abs(pred-x[(k+1):n])))
}

n <- 1e6
y <- sample(0:1,n,replace=T)
preda(y,1000)

}
