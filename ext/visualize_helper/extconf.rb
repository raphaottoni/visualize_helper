require "mkmf"
extension_name = "visualize_helper"
dir_config("visualize_helper")
#CONFIG["CC"] = "gcc-5"
$CXXFLAGS += ' -fopenmp '
$CFLAGS += ' -fopenmp '
have_library('gomp', "main") 
create_makefile "visualize_helper/visualize_helper"
