# Hadley Wickham, Advanced R
# Dirk Eddelbuettel, "MCMC and faster Gibbs Sampling using Rcpp", http://dirk.eddelbuettel.com/blog/2011/07/14/
# Darren Wilkinson, "Gibbs sampler in various languages (revisited)", https://darrenjw.wordpress.com/2011/07/16/gibbs-sampler-in-various-languages-revisited/

#' @export
gibbs <- function() {

gibbs_r <- function(N, thin) {
  mat <- matrix(nrow = N, ncol = 2)
  x <- y <- 0
  for(i in 1:N) {
    for(j in 1:thin) {
      x <- rgamma(1,3,y*y+4)
      y <- rnorm(1,1/(x+1),1/sqrt(2*(x+1)))
    }
    mat[i,] <- c(x,y)
  }
  mat
}

gibbs_r(50000,1000)

}

