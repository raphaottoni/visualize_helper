#include <ruby.h>

// Hello World  without parameters
static VALUE hello_world()
{
    return rb_str_new_cstr("hello world");
}



// Function callback to iterate in hash
static int testando_hash(VALUE key, VALUE value, VALUE array){
 
  rb_ary_push(array,value);  
  

  return ST_CONTINUE;
}  

// 
static VALUE teste(VALUE self, VALUE min, VALUE max, VALUE hash)
{

    int  min_c = FIX2INT(min);
    int  max_c = FIX2INT(max);
    int period = Qnil;
    VALUE array  = rb_ary_new();

    rb_hash_foreach(hash,testando_hash,array);
      
    return array;
}


// Function to convert params into array 
static VALUE create_array(VALUE self, VALUE min, VALUE max)
{

    int  min_c = FIX2INT(min);
    int  max_c = FIX2INT(max);
    int period = Qnil;
    VALUE array  = rb_ary_new();


    VALUE str1 = INT2FIX(min_c);
    VALUE str2 = INT2FIX(max_c);

    rb_ary_push(array,str1);  
    rb_ary_push(array,str2); 

    return array;
}

// Main function called when the gem is loaded 
void Init_visualize_helper(void) {

  // Register the VisualizeHelper module
  VALUE mVisualizeHelper = rb_define_module("VisualizeHelper");

  // Register the method Hello World without parameters 
  rb_define_singleton_method(mVisualizeHelper, "hello_world", hello_world, 0);

  // Register the method  teste
  rb_define_singleton_method(mVisualizeHelper, "teste", teste, 3);

  // Register the method create_array 
  rb_define_singleton_method(mVisualizeHelper, "create_array", create_array, 2);
}
