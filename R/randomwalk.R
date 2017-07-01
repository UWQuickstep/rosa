#' @export
randomwalk = function() {

rw2d1 = function(n = 100) {
  xpos = numeric(n)
  ypos = numeric(n)
  for(i in 2:n) {
    delta = if(runif(1) > .5) 1 else -1
    if (runif(1) > .5) {
      xpos[i] = xpos[i-1] + delta
      ypos[i] = ypos[i-1]
    }
    else {
      xpos[i] = xpos[i-1]
      ypos[i] = ypos[i-1] + delta
    }
  }
  return(list(x = xpos, y = ypos))
}

n = 1e7         # set this to 1e7 for a large run
b <- rw2d1(n)

}
