#' @export
exps <- function() {

exps_f <- function(x, alpha) {
  s <- numeric(length(x) + 1)
  for (i in seq_along(s)) {
    if (i==1) {
      s[i] <- x[i]
    } else {
      s[i] <- alpha * x[i-1] + (1-alpha) * s[i-1]
    }
  }
  s
}

n <- 1e7
x <- runif(n)
exps_f(x,0.5)

}
