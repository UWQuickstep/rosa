#' @export
oddcount <- function() {

oddc <- function(x) {
  k <- 0
  for (n in x) {
    if (n %% 2 == 1) k <- k+1
  }
  return(k)
}

n <- 1e7        # set to 1e8 for a large run
x <- sample(1:1000,n,replace=TRUE)
b <- oddc(x)
return(b)

}
