#include <ruby.h>

// Hello World  without parameters
static VALUE hello_world()
{
    return rb_str_new_cstr("hello world");
}



// 
static VALUE teste(VALUE self, VALUE min, VALUE max)
{

    int  min_c = FIX2INT(min);
    int  max_c = FIX2INT(max);


    //VALUE str1 = rb_str_new_cstr("OLá ...");
    //VALUE str2 = rb_str_new_cstr("enfermeira!");

    VALUE str1 = INT2FIX(min_c);
    VALUE str2 = INT2FIX(max_c);
    
    VALUE array  = rb_ary_new();
    rb_ary_push(array,str1);  
    rb_ary_push(array,str2); 

    return array;
}

// Main function called when the gem is loaded 
void Init_visualize_helper(void) {

  // Register the VisualizeHelper module
  VALUE mVisualizeHelper = rb_define_module("VisualizeHelper");

  // Register the module Hello World without parameters 
  rb_define_singleton_method(mVisualizeHelper, "hello_world", hello_world, 0);

  // Register the module Hello World with parameters 
  rb_define_singleton_method(mVisualizeHelper, "teste", teste, 2);

  // Register the module Hello World without parameters 
  //rb_define_singleton_method(mVisualizeHelper, "descobre", descobre, 2);
}
