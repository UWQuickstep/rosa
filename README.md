# ROSA: R Optimization with Static Analysis
This is an intial version of the ROSA package. This package is being released primarily to gather input from the wider community and to seek collaborators to help with further development of the initial ideas published in our preliminary paper at https://arxiv.org/abs/1704.02996. Volunteers to help make this code production ready is especially welcome. We acknowledgement that there is key integration and development work that is needed to make ROSA usable in R.

## Warning: This package reflects the current state of ROSA, and it has a number of rough edges. 
The authors acknowledge that help from the core R developer community is needed to help ROSA see the light of the day, and we look forward to hearing from you.

For more information, please contact: Jignesh Patel (jignesh@cs.wisc.edu)

---
## Installation
Checkout, build and install the package.
```
git clone https://github.com/UWQuickstep/rosa.git
R CMD build rosa
sudo R CMD INSTALL rosa
```
## Usage
#### Example 1.
```R
library(rosa)

f <- function() {
  update <- function(x, y) {
    if (x > y) {
      return(x - y)
    } else {
      return(x + y)
    }
  }
  
  values <- runif(1e8)
  result <- runif(1)
  for (v in values) {
    result <- update(result, v)
  }
  return(result)
}

info <- codegen(f)

cat(info$ast)
cat(info$analysis)
cat(info$type)
cat(info$code)
print(f)

f()
```
Then the generated code (printed by `cat(info$code)`) looks like:
```cpp
// A long list of ROSA specialized functions.
...

inline double update(double x, double y, Environment env);

//' @export
// [[Rcpp::export]]
double f() {
  Function env_fn("environment", Environment::base_env());
  Environment env(env_fn());
  double result;
  double v;
  NumericVector values;
  Function runif("runif", env);

  values = runif(1e+08);
  result = rosa::scalar<double>(runif(1), env);
  for (auto &v : values) {
    result = update(result, v, env);
  }
  return result;
}

inline double update(double x, double y, Environment env) {
  if ((x)>(y)) {
    return (x)-(y);
  } else {
    return (x)+(y);
  }
}
```

#### Example 2.
```R
library(rosa)

# Also try other sample functions: euclidean, randomwalk, exps.
print(oddcount)

info <- codegen(oddcount, "c_oddcount")

cat(info$code)

c_oddcount()
```
