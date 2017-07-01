# Declare micro-libs:
add_library(rcpp_Ast Ast.cpp Ast.hpp)
add_library(rcpp_AstPrinter ../empty_src.cpp AstPrinter.hpp)
add_library(rcpp_AstRule ../empty_src.cpp AstRule.hpp)
add_library(rcpp_AstTransforms AstTransforms.cpp AstTransforms.hpp)
add_library(rcpp_Cfg Cfg.cpp Cfg.hpp)
add_library(rcpp_CfgAnalysis ../empty_src.cpp CfgAnalysis.hpp)
add_library(rcpp_DebugUtil DebugUtil.cpp DebugUtil.hpp)
add_library(rcpp_HashUtil ../empty_src.cpp HashUtil.hpp)
add_library(rcpp_KnownFunctions KnownFunctions.cpp KnownFunctions.hpp)
add_library(rcpp_Macros ../empty_src.cpp Macros.hpp)
add_library(rcpp_ReachingDefinition ReachingDefinition.cpp ReachingDefinition.hpp)
add_library(rcpp_Rosa Rosa.cpp)
add_library(rcpp_TemplateUtil ../empty_src.cpp TemplateUtil.hpp)
add_library(rcpp_Type Type.cpp Type.hpp)
add_library(rcpp_TypeInference TypeInference.cpp TypeInference.hpp)
add_library(rcpp_TypeRules TypeRules.cpp TypeRules.hpp)
add_library(rcpp_VariableFacts VariableFacts.cpp VariableFacts.hpp)

# Link dependencies:
target_link_libraries(rcpp_Ast
                      rcpp_Macros
                      rcpp_Rcpp)
target_link_libraries(rcpp_AstPrinter
                      rcpp_Ast
                      rcpp_Macros
                      rcpp_TemplateUtil)
target_link_libraries(rcpp_AstRule
                      rcpp_Ast
                      rcpp_Macros)
target_link_libraries(rcpp_AstTransforms
                      rcpp_Ast
                      rcpp_AstRule
                      rcpp_Macros
                      rcpp_TemplateUtil)
target_link_libraries(rcpp_Cfg
                      rcpp_Ast
                      rcpp_AstPrinter
                      rcpp_Rcpp)
target_link_libraries(rcpp_CfgAnalysis
                      rcpp_Cfg
                      rcpp_Macros)
target_link_libraries(rcpp_DebugUtil
                      rcpp_Ast
                      rcpp_AstPrinter
                      rcpp_AstRule
                      rcpp_Cfg
                      rcpp_Macros
                      rcpp_ReachingDefinition
                      rcpp_TemplateUtil
                      rcpp_TypeInference
                      rcpp_VariableFacts)
target_link_libraries(rcpp_Macros
                      rcpp_Rcpp)
target_link_libraries(rcpp_ReachingDefinition
                      rcpp_Cfg
                      rcpp_CfgAnalysis
                      rcpp_Macros
                      rcpp_TemplateUtil
                      rcpp_VariableFacts)
target_link_libraries(rcpp_Rosa
                      rcpp_Ast
                      rcpp_AstPrinter
                      rcpp_AstRule
                      rcpp_AstTransforms
                      rcpp_Cfg
                      rcpp_CfgAnalysis
                      rcpp_DebugUtil
                      rcpp_Rcpp
                      rcpp_ReachingDefinition
                      rcpp_Type
                      rcpp_TypeInference
                      rcpp_VariableFacts)
target_link_libraries(rcpp_TemplateUtil
                      rcpp_Ast
                      rcpp_Macros
                      rcpp_Rcpp)
target_link_libraries(rcpp_Type
                      rcpp_HashUtil
                      rcpp_Macros)
target_link_libraries(rcpp_TypeInference
                      rcpp_Ast
                      rcpp_Cfg
                      rcpp_Macros
                      rcpp_ReachingDefinition
                      rcpp_TemplateUtil
                      rcpp_Type
                      rcpp_TypeRules
                      rcpp_VariableFacts)
target_link_libraries(rcpp_TypeRules
                      rcpp_Type)
target_link_libraries(rcpp_VariableFacts
                      rcpp_Ast
                      rcpp_AstPrinter
                      rcpp_AstRule
                      rcpp_Macros
                      rcpp_TemplateUtil)
