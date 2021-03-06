#include <ruby.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>

void RemoveSpaces(char* source)
{
  char* i = source;
  char* j = source;
  while(*j != 0)
  {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}


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
  int i;

  for( i = 0; i < RARRAY_LEN(sorted_array); i++){
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
static char* join(VALUE strings){

  // initial variables
  char* joined;
  int strings_size = RARRAY_LEN(strings);
  int string_size;
  int string_total_size = 0;
  VALUE string_temp;
  VALUE result;
  int i;
  


  // calculate the exact size of malloc
  for ( i = 0; i < strings_size; i++) {
      string_temp = rb_ary_entry(strings,i);
      string_total_size += strlen(StringValuePtr(string_temp));
  }


  joined = (char*) malloc(string_total_size +strings_size + 1);
  string_temp = rb_ary_entry(strings,0);
  //string_size = strlen(StringValuePtr(string_temp));
  sprintf(joined,"%s", StringValuePtr(string_temp));


  for (i = 1 ; i < strings_size; i++) {
    
    string_temp = rb_ary_entry(strings,i);
    sprintf(joined,"%s,%s", joined, StringValuePtr(string_temp));

  }
  strcat(joined,"\0");
  return  joined;
}



// Receive  an array of unsorted strings with repeated strings and return a single a sorted array with unique elementes or with all
// strings = array of strings
// unique =  0 or 1  if want to call unique or not
static VALUE sort_uniq(VALUE strings, int unique)
{
    int strings_size = RARRAY_LEN(strings);
    const char *input[strings_size];
    int i;

    for (i = 0; i< strings_size; i++){
      VALUE string  = rb_ary_entry(strings,i);
      input[i] = StringValuePtr(string);
    }

    int stringLen = sizeof(input) / sizeof(char *);
    qsort(input, stringLen, sizeof(char *), myCompare);


    // Transform the result input into a ruby array
    VALUE resultado = rb_ary_new();
    for (i=0; i<stringLen; ++i) {
      rb_ary_push(resultado,rb_str_new2(input[i]));
    } 

    return (unique == 0 ) ? resultado : uniq(resultado);
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
  int intervals_size = RARRAY_LEN(intervals);
  int aggr_size = RARRAY_LEN(aggr);
  VALUE first_interval = rb_ary_entry(intervals,0);
  VALUE last_interval = rb_ary_entry(intervals,intervals_size - 1);
  int first_interval_c = FIX2INT(first_interval);
  int last_interval_c = FIX2INT(last_interval);
  int days_c = atoi(StringValuePtr(days));
  int min_c = FIX2INT(rb_ary_entry(array,0));
  int max_c = FIX2INT(rb_ary_entry(array,1));
  int period, traj_size, index, i;


  if (days_c < first_interval_c) {
    if (min_c > days_c) {
      min_c = days_c;
    }
    period = 0;
  }else if (days_c == 0) {
    period = findIndex(intervals,intervals_size,0)+1;

  }else if (days_c > last_interval_c ) {
    if (max_c < days_c) {
      max_c = days_c;
    }
    period = aggr_size -1;
  }else {  
    for ( index = 0 ; index < intervals_size ; index++){
      if( index < (intervals_size - 1) ){
        int intervals_index_plus_1 =  FIX2INT(rb_ary_entry(intervals,index+1));
        if ( intervals_index_plus_1 <= 0  ){
          if (( days_c >= FIX2INT(rb_ary_entry(intervals,index))) && (days_c < intervals_index_plus_1)) {
            period = index +1;
            break;
          }  
        }else if ( (days_c > FIX2INT(rb_ary_entry(intervals,index)) ) && (days_c <= intervals_index_plus_1) ) {
          period = index + 2;
          break;
        }   
      }  
    }
  }

  rb_ary_store(array,0,INT2FIX(min_c));
  rb_ary_store(array,1,INT2FIX(max_c));
  traj_size = RARRAY_LEN(traj);
  for ( i =0; i < traj_size; i++){
    rb_ary_push(rb_ary_entry(aggr,period),rb_ary_entry(traj,i));
  }  

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
    int i;
    
    for( i = 0; i < RARRAY_LEN(strings); i++){
        current_value_temp = rb_ary_entry(strings,i);
        current_value  = StringValuePtr(current_value_temp);
        if (strcmp(current_value, element) != 0) {
          rb_ary_push(result,rb_ary_entry(strings,i));
        }
    }

    return result;
}

// Function to generate the boxes and links of each trajectory
static VALUE generate_boxes_and_links(VALUE self, VALUE aggr, VALUE boxes, VALUE links, VALUE dict, VALUE type_agroupment, VALUE value)
{

  char* seq_key_result;
  char* prox_key_result;
  // Initial Variables
  int length_seq_sorted;
  int length_prox_sorted;
  VALUE result_final;
  VALUE boxes_period_value;
  VALUE links_period_value;
  VALUE seq_key;
  VALUE prox_key;
  VALUE seq;
  VALUE aggr_prox;
  int seq_size;
  int aggr_prox_size;
  int prox;
  int aggr_size = RARRAY_LEN(aggr);
  char* link_key = (char*) malloc(1000);
  char* period_s;
  char* prox_s;
  int period,i;

  for(period = 0; period < aggr_size; period++ ){
     seq_key = rb_ary_new();

     if (period < aggr_size - 1) {
      prox_key = rb_ary_new();
     } 

     seq = rb_ary_entry(aggr,period);
     seq_size = (int) RARRAY_LEN(seq);

      // Translate sequences with dict
      if (seq_size == 0) {
        rb_ary_push(seq_key,rb_hash_aref(dict, rb_str_new2("M-2"))); 
      }else{
        for(i = 0; i < seq_size; i++ ) {
          rb_ary_push(seq_key,rb_hash_aref(dict,rb_ary_entry(seq,i)));  
        }  
      }

    // agroup by unique or not
    if ( strcmp(StringValuePtr(type_agroupment),"n") == 0 ) {
      //sort with uniq 
      seq_key = sort_uniq(seq_key,1);
    }else{
      //sort without uniq
      seq_key = sort_uniq(seq_key,0);
    }

    // if there is "no-event" and other one selected, remove the "no-event"
    length_seq_sorted  =  (strcmp(StringValuePtr(type_agroupment),"s") == 0 ) ? RARRAY_LEN(seq_key)  : RARRAY_LEN(uniq(seq_key)) ;
    if (length_seq_sorted != 1) {
      seq_key = remove_entry_from_array(seq_key,"M-3");
    }

    // Generate the key
    seq_key_result = join(seq_key);

    //
    boxes_period_value = rb_hash_aref(rb_ary_entry(boxes,period),rb_str_new2(seq_key_result));
    if (boxes_period_value  == Qnil) {
      rb_hash_aset(rb_ary_entry(boxes,period),rb_str_new2(seq_key_result), value);
    }else {
      rb_hash_aset(rb_ary_entry(boxes,period),rb_str_new2(seq_key_result),INT2FIX(FIX2INT(boxes_period_value) + FIX2INT(value)));
    }    

    prox = period +1;

    if (prox < aggr_size ) {
      aggr_prox = rb_ary_entry(aggr,prox); 
      aggr_prox_size = (int) RARRAY_LEN(aggr_prox);
      if ( aggr_prox_size == 0) {
        rb_ary_push(prox_key,rb_hash_aref(dict, rb_str_new2("M-2"))); 
      }else{
        for( i = 0; i < aggr_prox_size ; i++ ) {
          rb_ary_push(prox_key,rb_hash_aref(dict,rb_ary_entry(aggr_prox,i)));  
        }  
      }     

       // agroup by unique or not
      if ( strcmp(StringValuePtr(type_agroupment),"n") == 0 ) {
        //sort with uniq
        prox_key = sort_uniq(prox_key,1);
      }
      //else{
      //  //sort without uniq
      //  prox_key = prox_key;
      //}

  //    
      // if there is "no-event" and other one selected, remove the "no-event"
      length_prox_sorted  =  (strcmp(StringValuePtr(type_agroupment),"n") == 0 ) ? RARRAY_LEN(prox_key)  : RARRAY_LEN(sort_uniq(prox_key,1)) ;
      if (length_prox_sorted != 1) {
        prox_key = remove_entry_from_array(prox_key,"M-3");
      }

      // Generate the key
      prox_key_result = join(prox_key);

      // generate a key link
      period_s = ( period == 0 ) ? (char*) malloc(1)  : (char*) malloc(floor(log10(abs(period))) + 1);
      prox_s = ( prox == 0 ) ? (char*) malloc(1)  : (char*) malloc(floor(log10(abs(prox))) + 1);
      sprintf(period_s,"%d",period);
      sprintf(prox_s,"%d",prox);
      //link_key =  (char*) malloc( strlen(period_s) + strlen(seq_key_result) + strlen(prox_s) + strlen(prox_key_result) + 3);
      sprintf(link_key,"%s_%s;%s_%s", period_s,seq_key_result,prox_s,prox_key_result);
      RemoveSpaces(link_key);
      
      links_period_value = rb_hash_aref(rb_ary_entry(links,period),rb_str_new2(link_key));
      if (links_period_value  == Qnil) {
        rb_hash_aset(rb_ary_entry(links,period),rb_str_new2(link_key),value);
      }else {
        rb_hash_aset(rb_ary_entry(links,period),rb_str_new2(link_key), INT2FIX(FIX2INT(links_period_value) + FIX2INT(value)));
      }    


    }//end prox < aggr_size

  }// end for 

  free(period_s); 
  free(prox_s); 
  free(link_key); 

  return Qnil;
}  


static VALUE join_teste(VALUE self, VALUE strings){

    return rb_str_new2(join(strings));
}



// iterate over all the tracjetories and create the boxes and links needed for the dashboard
static VALUE iterate_over_trajectories(VALUE self, VALUE trajectories, VALUE min, VALUE max, VALUE hash_v, VALUE hash_t, VALUE intervals, VALUE dict, VALUE agrupamento, VALUE boxes, VALUE links) {

  VALUE trajectory;
  VALUE aggr;
  VALUE value;
  VALUE min_max_aggr;
  int i,j;

  // iterate over each trajectory
  for (i = 0; i < RARRAY_LEN(trajectories); i++ ){
  
    trajectory = rb_ary_entry(trajectories,i);

    aggr = rb_ary_new();
    for (j = 0; j < RARRAY_LEN(intervals) + 2; j++){
      rb_ary_push(aggr,rb_ary_new());
    } 
  
    value = rb_hash_aref(hash_v,trajectory);
    min_max_aggr = min_max_period(self,min,max,rb_hash_aref(hash_t,trajectory),intervals,aggr);
    aggr = rb_ary_entry(min_max_aggr,4);
    min = rb_ary_entry(min_max_aggr,0);
    max = rb_ary_entry(min_max_aggr,1);

    generate_boxes_and_links(self,aggr,boxes,links,dict,agrupamento,value);

  }  

  return Qnil;
}  


static VALUE openmp_test(VALUE self, VALUE string) {

  FILE *f = fopen("/tmp/open_mp", "w");

  int nthreads, tid;

  /* Fork a team of threads giving them their own copies of variables */
  #pragma omp parallel private(nthreads, tid)
  {
  
    /* Obtain thread number */
    tid = omp_get_thread_num();
    fprintf(f,"Hello World from thread = %d\n", tid);
  
    /* Only master thread does this */
    if (tid == 0)
      {
      nthreads = omp_get_num_threads();
      fprintf(f,"Number of threads = %d\n", nthreads);
      }
  
  }  /* All threads join master thread and disband */

  fclose(f);
  return string;
}


// Function callback to iterate in hash of boxes or links
static int update_boxes_links(VALUE key, VALUE value, VALUE parameters){
 
  VALUE boxes_links_result = rb_ary_entry(parameters,0);
  int index = FIX2INT(rb_ary_entry(parameters,1));

  VALUE boxes_links_result_index = rb_ary_entry(boxes_links_result,index);

  // if the key isnt already set on the boxes_result, create it 
  if (rb_hash_aref(boxes_links_result_index,key) == Qnil) {
    rb_hash_aset(boxes_links_result_index,key,value);
  }else{
    // if the key is already on the boxes_result, sum the results
    rb_hash_aset(boxes_links_result_index,key, FIX2INT(rb_hash_aref(boxes_links_result_index,key)) + FIX2INT(value));
  }  

  return ST_CONTINUE;
}  




// iterate over all the trajectories and create the boxes and links needed for the dashboard (Parallel)
//static VALUE iterate_over_trajectories_parallel(VALUE self, VALUE trajectories, VALUE min, VALUE max, VALUE hash_v, VALUE hash_t, VALUE intervals, VALUE dict, VALUE agrupamento, VALUE boxes, VALUE links) {
static VALUE iterate_over_trajectories_parallel() {

  //VALUE trajectory;
  VALUE aggr;
  //VALUE value;
  //VALUE min_max_aggr;
  //VALUE boxes_result = rb_ary_new(); 
  //VALUE links_result = rb_ary_new();
  int i,j;
  

  //initialize boxes_result and links_result
  //for (j = 0; j < RARRAY_LEN(intervals) + 2; j++){
  //  rb_ary_push(boxes_result,rb_hash_new());
  //  if ( j != 0) {
  //    rb_ary_push(links_result,rb_hash_new());
  //  }  
  //} 

  //#pragma omp parallel firstprivate(boxes,links,i,j)
  //#pragma omp parallel private(i,j,aggr,boxes,links)
  //{
  //  //#pragma omp for nowait
  //  #pragma omp for firstprivate(min,max) nowait
  //  for (i = 0; i < RARRAY_LEN(trajectories); i++ ){
  //  
  //    
  //    trajectory = rb_ary_entry(trajectories,i);
   
  #pragma omp parallel for private(i,aggr)
  for (i = 0; i < 500; i++ ){

      aggr = rb_ary_new();
      //for (j = 0; j < RARRAY_LEN(intervals) + 2; j++){
      //  rb_ary_push(aggr,rb_ary_new());
      //} 
    
      //value = rb_hash_aref(hash_v,trajectory);
      //min_max_aggr = min_max_period(self,min,max,rb_hash_aref(hash_t,trajectory),intervals,aggr);
      //aggr = rb_ary_entry(min_max_aggr,4);
      //min = rb_ary_entry(min_max_aggr,0);
      //max = rb_ary_entry(min_max_aggr,1);

      //generate_boxes_and_links(self,aggr,boxes,links,dict,agrupamento,value);

   // }// end pragma for  
  }
  //  // must aggregate the boxes from each thread into boxes_result
  //  #pragma omp critical
  //  {
  //    int i;
  //    VALUE parameters = rb_ary_new2(2);
  //    // iterate over the size of boxes
  //    for(i = 0; i < RARRAY_LEN(boxes); i++ ){
  //      
  //      rb_ary_store(parameters,0,boxes_result);
  //      rb_ary_store(parameters,1,INT2FIX(i));
  //      VALUE hash = rb_ary_entry(boxes,i);
  //      //iterate over each key of the hashmap
  //      rb_hash_foreach(hash,update_boxes_links,parameters);
  //    }  
  //  }  

  //  // must aggregate the links from each thread into links_result
  //  #pragma omp critical
  //  {
  //    int i;
  //    VALUE parameters = rb_ary_new2(2);
  //    for(i = 0; i < RARRAY_LEN(links); i++ ){
  //      
  //      rb_ary_store(parameters,0,links_result);
  //      rb_ary_store(parameters,1,INT2FIX(i));
  //      VALUE hash = rb_ary_entry(links,i);
  //      //iterate over each key of the hashmap
  //      rb_hash_foreach(hash,update_boxes_links,parameters);
  //    }  
  //  }  

  //}// and pragma parallel

  return Qnil;
}  





// Main function called when the gem is loaded 
void Init_visualize_helper(void) {

  // Register the VisualizeHelper module
  VALUE mVisualizeHelper = rb_define_module("VisualizeHelper");

  // Register the method min_max_period 
  rb_define_singleton_method(mVisualizeHelper, "min_max_period", min_max_period, 5);

  // Register the method generate_boxes_and_links
  rb_define_singleton_method(mVisualizeHelper, "generate_boxes_and_links", generate_boxes_and_links, 6);

  // Register the method sort
  rb_define_singleton_method(mVisualizeHelper, "sort_uniq", sort_uniq, 2 );

  // Register the method that iterates on each trajectory 
  rb_define_singleton_method(mVisualizeHelper, "iterate_over_trajectories", iterate_over_trajectories, 10);

  // Register the method sort
  rb_define_singleton_method(mVisualizeHelper, "join_teste", join_teste, 1 );

  // Register a test with openmp
  rb_define_singleton_method(mVisualizeHelper, "openmp_test", openmp_test, 1 );

  // Register the method that iterates on each trajectory using openmp
  rb_define_singleton_method(mVisualizeHelper, "iterate_over_trajectories_parallel", iterate_over_trajectories_parallel, 0);
}
