#' @export
euclidean <- function() {

dist = function(X, Y) {
  nx = nrow(X)
  ny = nrow(Y)
  p = ncol(X)
  ctr = 1L
  ans = numeric(nx * ny)
  for(i in 1:nx) {
    for(j in 1:ny) {
      posX = i
      posY = j
      total = 0.0
      for(k in 1:p) {
        total = total + (X[posX] - Y[posY])^2
        posX = posX + nx
        posY = posY + ny
      }
      ans[ctr] = sqrt(total)
      ctr = ctr + 1L
    }
  }
  return(ans)
}

p0 = 40L
n1 = 8000L     # set this to 8000L for a large run
n2 = 1000L     # set this to 1000L for a large run
X = matrix(rnorm(n1 * p0), n1, p0)
Y = matrix(rnorm(n2 * p0), n2, p0)
b <- dist(X, Y)
return(b)

}
