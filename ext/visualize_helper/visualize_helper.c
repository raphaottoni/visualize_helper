#include <ruby.h>

// Hello World  without parameters
static VALUE hello_world()
{
    return rb_str_new_cstr("hello world");
}

// Find the index of the target value inside array
int findIndex(VALUE intervals, size_t size, int target) 
{
      int i=0;
       while((i<size) && (FIX2INT(rb_ary_entry(intervals,i)) != target)) i++;

       return (i<size) ? (i) : (-1);
}


// Function callback to iterate in hash
// Params: 
//        days: hash key (VALUE)
//        traj: hash value (VALUE)
//        array: [min (int) ,max (int) ,period (Qnil/int), intervals (C array), intervals_size(int), aggr (VALUE)]
static int encontra_min_max_period(VALUE days, VALUE traj, VALUE array){
 
  // Convert every variable need to C value
  VALUE intervals = rb_ary_entry(array,3);
  VALUE aggr = rb_ary_entry(array,4);
  int intervals_size = RARRAY_LEN(array);
  int aggr_size = RARRAY_LEN(aggr);
  VALUE first_interval = rb_ary_entry(intervals,0);
  VALUE last_interval = rb_ary_entry(intervals,intervals_size - 1);
  int first_interval_c = FIX2INT(first_interval);
  int last_interval_c = FIX2INT(last_interval);
  int days_c = atoi(StringValuePtr(days));
  int min_c = FIX2INT(rb_ary_entry(array,0));
  int max_c = FIX2INT(rb_ary_entry(array,1));
  int period;

  if (days_c < first_interval_c) {
    if (min_c > days_c) {
      min_c = days_c;
      //rb_ary_store(array,0,days);
    }
    period = 0;
    //rb_ary_store(array,2,INT2FIX(0));
  }else if (days_c == 0) {
    period = findIndex(intervals,intervals_size,0)+1;
    //rb_ary_store(array,2,INT2FIX(index+1));

  }else if (days_c > last_interval_c ) {
    if (max_c < days_c) {
      max_c = days_c;
      //rb_ary_store(array,1,days);
    }
    //rb_ary_store(array,2,INT2FIX(aggr_size -1));
    period = aggr_size -1;
  }else {  
    for ( int index = 0 ; index < intervals_size ; index++){
      if( index < (intervals_size - 1) ){
        int intervals_index_plus_1 =  FIX2INT(rb_ary_entry(intervals,index+1));
        if ( intervals_index_plus_1 <= 0  ){
          if (( days_c >= rb_ary_entry(intervals,index)) && (days_c < intervals_index_plus_1)) {
            period = index +1;
            //rb_ary_store(array,2,INT2FIX(i+1));
            break;
          }  
        }else if ( (days_c > rb_ary_entry(intervals,index) ) && (days_c <= intervals_index_plus_1) ) {
          period = index + 2;
          //rb_ary_store(array,2,INT2FIX(i+2));
          break;
        }   
      }  
    }
  }
  rb_ary_push(rb_ary_entry(aggr,period),rb_ary_entry(traj,0));

  return ST_CONTINUE;
}  

// Function to find min, max and the period of the HashMapValue of trajectories in Visualize app
// Return: [ min, max, period]
static VALUE min_max_period(VALUE self, VALUE min, VALUE max, VALUE hash, VALUE intervals, VALUE aggr)
{
    int  period = Qnil;

    // Create Ruby array that will be the result
    VALUE array  = rb_ary_new();

    // Push initial values of the result
    rb_ary_push(array,min);  
    rb_ary_push(array,max);  
    rb_ary_push(array,period);  
    rb_ary_push(array,intervals);  
    rb_ary_push(array,aggr);  
    
    
    // iterate on hash calling "encontra_min_max_period" for each
    rb_hash_foreach(hash,encontra_min_max_period,array);
      
    // flatten on aggr
    

    // Return results
    return aggr;
}



static VALUE test(VALUE self, VALUE param){

  int a =  INT2FIX(param);
  return FIX2INT(a);
}  

// Main function called when the gem is loaded 
void Init_visualize_helper(void) {

  // Register the VisualizeHelper module
  VALUE mVisualizeHelper = rb_define_module("VisualizeHelper");

  // Register the method Hello World without parameters 
  rb_define_singleton_method(mVisualizeHelper, "hello_world", hello_world, 0);

  // Register the method min_max_period 
  rb_define_singleton_method(mVisualizeHelper, "min_max_period", min_max_period, 5);

  // Register the method  for development testing
  rb_define_singleton_method(mVisualizeHelper, "test", test, 1 );
}
