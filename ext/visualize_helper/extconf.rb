require "mkmf"
extension_name = "visualize_helper"
dir_config("visualize_helper")
$CXXFLAGS += ' -fopenmp '
$CFLAGS += ' -fopenmp '
have_library('gomp', "main") 
create_makefile "visualize_helper/visualize_helper"
