#include <ruby.h>
#include <stdlib.h>
#include <string.h>


int myCompare (const void * a, const void * b ) {
    const char *pa = *(const char**)a;
    const char *pb = *(const char**)b;

    return strcmp(pa,pb);
}

// recive a sorted array of strings and return a sorted array with only the unique elements 
static VALUE uniq(VALUE sorted_array){

  //initialize return variable in ruby
  VALUE uniq_array = rb_ary_new();

  //initialize variables for compare 
  char* previous;
  char* current;

  for( int i = 0; i < RARRAY_LEN(sorted_array); i++){
    VALUE current_temp = rb_ary_entry(sorted_array,i);
    current =  StringValuePtr(current_temp);
    // initial case, when there is no previous
    if ( i == 0 ){
      VALUE previous_temp = rb_ary_entry(sorted_array,i);
      previous =  StringValuePtr(previous_temp);
      rb_ary_push(uniq_array,rb_ary_entry(sorted_array,i));
    } else {
      if (strcmp(previous,current) != 0 ){
        rb_ary_push(uniq_array,rb_ary_entry(sorted_array,i));
        VALUE previous_temp = rb_ary_entry(sorted_array,i);
        previous = StringValuePtr(previous_temp);
      } 
    }
  }
  
  return uniq_array;
}

//static VALUE join(VALUE self, VALUE strings){
static VALUE join(VALUE strings){

  // initial variables
  char* joined;
  int strings_size = RARRAY_LEN(strings);
  int string_size;
  VALUE string_temp;
  VALUE result;
  
  string_temp = rb_ary_entry(strings,0);
  string_size = strlen(StringValuePtr(string_temp));
  joined = (char*) malloc(strings_size + 1);
  sprintf(joined,"%s", StringValuePtr(string_temp));

  for (int i = 1 ; i < strings_size; i++) {
    
    string_temp = rb_ary_entry(strings,i);
    string_size = strlen(StringValuePtr(string_temp));
      
    joined = (char*)  realloc(joined, string_size + strlen(joined) + 1);
    sprintf(joined,"%s,%s", joined, StringValuePtr(string_temp));

  }
  result = rb_str_new2(joined);  
  free(joined);
  return  result;
}



// Receive  an array of unsorted strings with repeated strings and return a single a sorted array with unique elementes or with all
// strings = array of strings
// unique =  0 or 1  if want to call unique or not
static VALUE sort_uniq(VALUE self, VALUE strings, VALUE unique)
{
    int strings_size = RARRAY_LEN(strings);
    const char *input[strings_size];

    for (int i = 0; i< strings_size; i++){
      VALUE string  = rb_ary_entry(strings,i);
      input[i] = StringValuePtr(string);
    }

    int stringLen = sizeof(input) / sizeof(char *);
    qsort(input, stringLen, sizeof(char *), myCompare);


    // Transform the result input into a ruby array
    VALUE resultado = rb_ary_new();
    for (int i=0; i<stringLen; ++i) {
      rb_ary_push(resultado,rb_str_new2(input[i]));
    } 

    return (FIX2INT(unique) == 0 ) ? resultado : uniq(resultado);
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


//function to remove element from a array of strings
static VALUE remove_entry_from_array (VALUE strings, char* element){
    
    //initial variables
    VALUE result = rb_ary_new();
    char* current_value;
    VALUE current_value_temp;
    
    for(int i = 0; i < RARRAY_LEN(strings); i++){
        current_value_temp = rb_ary_entry(strings,i);
        current_value  = StringValuePtr(current_value_temp);
        if (strcmp(current_value, element) != 0) {
          rb_ary_push(result,rb_ary_entry(strings,i));
        }
    }

    return result;
}

// Function to generate the boxes and links of each trajectory
static VALUE generate_boxes_and_links(VALUE self, VALUE min, VALUE max, VALUE aggr, VALUE boxes, VALUE links, VALUE dict, VALUE type_agroupment)
{

  VALUE seq_key_result;
  // Initial Variables
  int length_seq_sorted;

  for(int period = 0; period < RARRAY_LEN(aggr); period++ ){
    VALUE seq_key = rb_ary_new();
    VALUE seq = rb_ary_entry(aggr,period);
    int seq_size = RARRAY_LEN(seq);

    // Translate sequences with dict
    if (seq_size == 0) {
      rb_ary_push(seq_key,rb_hash_aref(dict, rb_str_new2("M-2"))); 
    }else{

      for(int i = 0; i < seq_size; i++ ) {
        rb_ary_push(seq_key,rb_hash_aref(dict,rb_ary_entry(seq,i)));  
      }  
    }

    // agroup by unique or not
    if ( strcmp(StringValuePtr(type_agroupment),"n") != 0 ) {
      //sort with uniq
      seq_key = sort_uniq(self,seq_key,INT2FIX(1));
    }else{
      //sort without uniq
      seq_key = sort_uniq(self,seq_key,INT2FIX(0));
    }

    // if there is "no-event" and other one selected, remove the "no-event"
    length_seq_sorted  =  (strcmp(StringValuePtr(type_agroupment),"n") != 0 ) ? RARRAY_LEN(seq_key)  : RARRAY_LEN(uniq(seq_key)) ;
    if (length_seq_sorted != 1) {
      seq_key = remove_entry_from_array(seq_key,"M-3");
    }

    // Generate the key
    seq_key_result = join(seq_key);
  }  
  return seq_key_result;
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
  rb_define_singleton_method(mVisualizeHelper, "sort_uniq", sort_uniq, 2 );

  // Register the method sort
  rb_define_singleton_method(mVisualizeHelper, "join", join, 1 );
}
