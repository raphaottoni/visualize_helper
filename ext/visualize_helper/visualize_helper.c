#include <ruby.h>

// Hello World  without parameters
//static VALUE hello_world()
//{
//    return rb_str_new_cstr("hello world");
//}
//
// Hello World  with string parameter 
static VALUE hello_world2()
{
    return rb_str_new_cstr("oi");
}




// Main function called when the gem is loaded 
void Init_visualize_helper(void) {

  // Register the VisualizeHelper module
  VALUE mVisualizeHelper = rb_define_module("VisualizeHelper");

  // Register the module Hello World without parameters 
  //rb_define_singleton_method(mVisualizeHelper, "hello_world", hello_world, 0);

  // Register the module Hello World with parameters 
  rb_define_singleton_method(mVisualizeHelper, "hello_world2", hello_world2, 0);

  // Register the module Hello World without parameters 
  //rb_define_singleton_method(mVisualizeHelper, "descobre", descobre, 2);
}
