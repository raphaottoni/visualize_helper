#include <ruby.h>
#include <stdlib.h>
#include <string.h>


int myCompare (const void * a, const void * b ) {
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;

    return strcmp(pa,pb);
}


static VALUE sort(VALUE self, VALUE strings)
{
    int strings_size = RARRAY_LEN(strings);
    //const char *input[] = {"a","orange","apple","mobile","car"};
    const char *input[strings_size];

    for (int i = 0; i< strings_size; i++){
      VALUE string  = rb_ary_entry(strings,i);
      input[i] = StringValuePtr(string);
    }

    int stringLen = sizeof(input) / sizeof(char *);
    qsort(input, stringLen, sizeof(char *), myCompare);

    VALUE resultado = rb_ary_new();
    for (int i=0; i<stringLen; ++i) {
      rb_ary_push(resultado,rb_str_new2(input[i]));
    } 

    return resultado;  
}

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

  

  rb_ary_store(array,0,INT2FIX(min_c));
  rb_ary_store(array,1,INT2FIX(max_c));
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
      
    // Return results
    return array;
}

// Function to generate the boxes and links of each trajectory
static VALUE generate_boxes_and_links(VALUE self, VALUE min, VALUE max, VALUE aggr, VALUE boxes, VALUE links, VALUE dict, VALUE type_agroupment)
{

  VALUE seq_key2 = rb_ary_new();
  // Initial Variables
  //for(int period = 0; period < RARRAY_LEN(aggr); period++ ){
  //  VALUE seq_key = rb_ary_new();
  //  VALUE seq = rb_ary_entry(aggr,period);
  //  int seq_size = RARRAY_LEN(seq);

  //  // Translate sequences with dict
  //  if (seq_size == 0) {
  //    rb_ary_push(seq_key,rb_hash_aref(dict, rb_str_new2("M-2"))); 
  //  }else{

  //    for(int i = 0; i < seq_size; i++ ) {
  //      rb_ary_push(seq_key,rb_hash_aref(dict,rb_ary_entry(seq,i)));  
  //    }  
  //  }

  //  // agroup by unique or not
  //  if ( strcmp(StringValue(type_agroupment),"n") != 0 ) {
  //}  
  return seq_key2;
}  



// Function test that only return a int variable in C to Ruby
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

  // Register the method generate_boxes_and_links
  rb_define_singleton_method(mVisualizeHelper, "generate_boxes_and_links", generate_boxes_and_links, 7);

  // Register the method  for development testing
  rb_define_singleton_method(mVisualizeHelper, "test", test, 1 );

  // Register the method sort
  rb_define_singleton_method(mVisualizeHelper, "sort", sort, 1 );
}
