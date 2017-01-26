/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Boris Grot, Priyank Faldu
 * *
 * *    Deadline: Wed 23 Nov (Week 10) 16:00
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
/* Do not add any more header files */

typedef enum {dm, fa} cache_map_t;
typedef enum {fifo, lru, none} cache_replacement_t;
typedef enum {instruction, data} access_t;

typedef struct {
    uint32_t address;
    access_t accesstype;
} mem_access_t;

typedef struct {
    uint32_t instruction_accesses;
    uint32_t instruction_hits;
    uint32_t data_accesses;
    uint32_t data_hits;
} result_t;

/* Cache Parameters */
uint32_t cache_size = 2048;
uint32_t block_size = 64;
cache_map_t cache_mapping = dm;
cache_replacement_t cache_replacement = none;


/* Reads a memory access from the trace file and returns
 * 1) access type (instruction or data access)
 * 2) memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1002];
    char* token = NULL;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file)!=NULL) {

        /* Get the access type */
        token = strsep(&string, " \n");
        if (strcmp(token,"I") == 0) {
            access.accesstype = instruction;
        } else if (strcmp(token,"D") == 0) {
            access.accesstype = data;
        } else {
            printf("Unkown access type\n");
            exit(-1);
        }

        /* Get the address */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtol(token, NULL, 16);

        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_blocks, uint32_t bits_offset, uint32_t bits_index, uint32_t bits_tag, result_t r) {
    /* Do Not Modify This Function */
    printf("Num_Blocks:%u\n", num_blocks);
    printf("Bits_BlockOffset:%u\n", bits_offset);
    printf("Bits_Index:%u\n", bits_index);
    printf("Bits_Tag:%u\n", bits_tag);
    if ( (r.instruction_accesses == 0) || (r.data_accesses == 0) ) {
        /*
         * Just a protection over divide by zero.
         * Ideally, it should never reach here.
         */
        return;
    }
    printf("Total_Accesses:%u\n", r.instruction_accesses + r.data_accesses);
    printf("Total_Hits:%u\n", r.instruction_hits + r.data_hits);
    printf("Total_HitRate:%2.2f%%\n", (r.instruction_hits + r.data_hits) / ((float)(r.instruction_accesses + r.data_accesses)) * 100.0);
    printf("Instruction_Accesses:%u\n", r.instruction_accesses);
    printf("Instruction_Hits:%u\n", r.instruction_hits);
    printf("Instruction_HitRate:%2.2f%%\n", r.instruction_hits / ((float)(r.instruction_accesses)) * 100.0);
    printf("Data_Accesses:%u\n", r.data_accesses);
    printf("Data_Hits:%u\n", r.data_hits);
    printf("Data_HitRate:%2.2f%%\n", r.data_hits / ((float)(r.data_accesses)) * 100.0);
}

/*
 * Global variables
 * These variables must be populated before call to print_statistics(...) is made.
 */
uint32_t num_bits_for_block_offset = 0;
uint32_t num_bits_for_index = 0;
uint32_t num_bits_for_tag = 0;
uint32_t num_blocks = 0;
result_t result;

/*Conversion function that takes in a string representation of a hexadecimal
address and converts it into a binary address. Based on the character in the hex
address, it is mapped to a certain string (letters are indexed by their index
in the alphabet.)
@Args hex_value - input string of hexadecimal address
@Args bin_addr - pointer to binary address array location, pass by reference.
*/
void hex_to_bin_rep(char *hex_value, char *bin_addr){
  char *bin_value = malloc(sizeof(char) * 35);
  int i = 0;
  const char hex_bin_map[16][4] = {"0000", "0001", "0010", "0011", "0100", "0101",
                     "0110", "0111", "1000", "1001", "1010", "1011",
                     "1100", "1101", "1110", "1111"};
  for(i = 0; i < strlen(hex_value); i++){
    if(strchr("0123456789", hex_value[i]) != NULL){
      int val = hex_value[i] - '0';
      strncat(bin_value, hex_bin_map[val],4);}
    else if(strchr("abcdef", hex_value[i]) != NULL){
      if(hex_value[i] ==  'a'){
        strncat(bin_value,hex_bin_map[10],4);
      }
      else if(hex_value[i] ==  'b'){
        strncat(bin_value,hex_bin_map[11],4);
      }
      else if(hex_value[i] ==  'c'){
        strncat(bin_value,hex_bin_map[12],4);
      }
      else if(hex_value[i] ==  'd'){
        strncat(bin_value,hex_bin_map[13],4);
      }
      else if(hex_value[i] ==  'e'){
        strncat(bin_value,hex_bin_map[14],4);
      }
      else if(hex_value[i] ==  'f'){
        strncat(bin_value,hex_bin_map[15],4);
      }




    }
  }
  strcpy(bin_addr, bin_value);
  free(bin_value);
}
/*Get function for direct mapped caches. It iterates through the character array
binary address. Starting at the index, tag_break, which is where the tag ends
and index begins in direct mapped caches. It iterates to the size of the index,
before the offset, and adds it to an array called bin_index. The bin_index
is then converted to decimal through strtol and returned.
@Args *bin_addr - Char array binary address
@Args tag_break - Size of tag_break in direct-mapped cache
@Args size_of_index - Size of index in direct-mapped cache
@ Return - Integer value, continuous value for index location for address in
        direct mapped cache.
*/
int get_str_index(char *bin_addr, int tag_break, int size_of_index){
  int i = 0;
  int h = 0;
  char *bin_index = malloc(sizeof(char) * size_of_index + 1);
  for(i = tag_break; i < tag_break+size_of_index; i++){
    bin_index[h] = bin_addr[i];
    h++;
  }
  bin_index[size_of_index + 1] = '\0';
  long index = strtol(bin_index, NULL, 2);
  free(bin_index);
  return index;

}
/*Conversion function, that takes in a binary address and returns a long int
of the decimal representation of the binary tag.
@Args *bin_tag - Char array of binary address, cut off to the length of the binary tag.
@ Return - long int value, continuous value for decimal representation of tag
*/
long int bin_str_to_dec(char *bin_tag){
  char* endptr;
  long int deci;
  deci = strtol(bin_tag, &endptr, 2);
  return deci;
}
/*Print function that takes in length of array (num_blocks) and the array
its iterating through and prints its contents. Only used for testing, not calculated
after final completion.
@Args length - Int of number of blocks in a cache
@Args *arr - Long int array representation of cache that holds address tags.
*/
void print_dynamic_array(int length, long int *arr){
  int i;
  printf("\n");
  for (i = 0; i < length; i++){
    printf("%ld\n",arr[i]);
  }
}
/*Function to initiate cache and allocate memory based on the parameter length
which is block size and the size of long int, which is the tag.
@Args length - Number of blocks in cache
@ Return - The allocated memoy space, cache.
*/
long int *initiate_cache(int length){
  long int *cache = malloc(length * sizeof(long int));
  return cache;
}
/*Get function that iterates through an array, in this case the cache, and returns
the amount of empty elements in the array.
@Args length - Number of blocks in cache
@Args *arr - The array being iterated through.
@ Return - The amount of empty spaces in a cache.
*/
int get_num_empty(int length, long int *arr){
  int empty = 0;
  for (int i = 0; i < length; i++){
    if(arr[i] == 0){
      empty++;

    }
  }
  return empty;
}
/* Helper function that goes through an array and shifts all elements in the array
up (or back) one index, used for replacement in fully associative caches.
@Args *arr - The long int array (cache) being iterated through and shifted.
@Args length - The amount of blocks in the cache
@Args - Starting index, the starting location of the shifting
@Return Placement_index, the final empty cell that the new tag will be placed in.
*/
int Shift(long int *arr, int length, int starting_index){
  for (starting_index; starting_index < length; starting_index++){
    arr[starting_index] = 0;
    if(starting_index == length-1 || arr[starting_index+1] == 0){
      break;
    }
    else if(arr[starting_index+1] != 0){
      arr[starting_index] = arr[starting_index+1];
    }

  }
  return starting_index;
}
/*Function that iterates through cache and processes it with the LRU replacement schene.
@Args *arr - The long int array (cache) being iterated through and shifted.
@Args length - The amount of blocks in the cache
@Args *bin_tag - Pointer to address of the binary tag, placed in cache.
@Args - *hit_status, Pass by reference that changes to 1 if a hit occurs.
*/
void LRU_Process(long int *arr, int length, long int *bin_tag, int *hit_status){
  int num_empty = get_num_empty(num_blocks,arr); //Get number of empty blocks
  int i = 0; //Iterator
  int placement_index; //Initializes int for where tag will be placed after shift
  if(num_empty == length){ //If cache is empty
    arr[0] = *bin_tag; //Place at zero index.
  }
  else if(num_empty != 0 && num_empty != length){ //If not empty and not full
    for (i = 0; i < length; i++){
      if(arr[i] != *bin_tag && arr[i+1] == 0){ //If Miss and next is epty
        arr[i+1] =  *bin_tag; //Place at next location
        break;
      }
      else if(arr[i] == *bin_tag){ //If hit
        placement_index = Shift(arr, length, i); //Shift all elements up
        arr[placement_index] = *bin_tag; //Places tag
        *hit_status = 1; //Changes hit status to one
        break;
      }
      else{};
    }
  }
  else if(num_empty == 0){ //If cache is full
    for(i = 0; i < length; i++){
      if(arr[i] != *bin_tag && i != length-1){}//If no hit and not at end
      else if(arr[i] != *bin_tag && i == length-1){//If miss and at end
        placement_index = Shift(arr,length, 0);//Start shifting from zero
        arr[placement_index] = *bin_tag; //Places tag
        break;
      }
      else if(arr[i] == *bin_tag && i != length-1){ //If hit at end
        *hit_status = 1; //Change hit status
        placement_index = Shift(arr,length,i); //Shifts all tags up
        arr[placement_index] = *bin_tag; //Places tag
        break;
      }
      else if(arr[i] == *bin_tag && i == length-1){ //If hit at end of array
        *hit_status = 1; //Just change hit status, nothing changes.
        break;
      }
    }

  }
}
/*Function that iterates through cache and processes it with the FIFO replacement schene.
@Args *arr - The long int array (cache) being iterated through and shifted.
@Args length - The amount of blocks in the cache
@Args *bin_tag - Pointer to address of the binary tag, placed in cache.
@Args - *hit_status, Pass by reference that changes to 1 if a hit occurs.
*/
void FIFO_Process(long int *arr, int length, long int *bin_tag, int*hit_status){
  int num_empty = get_num_empty(num_blocks,arr); //Number of empty blocks
  int i = 0; //Iterator
  int throw_away = 0; //Int for placement index for shift, given scheme not used just placeholder.
  if(num_empty == length){ //If empty
    arr[0] = *bin_tag; //Simply place
  }
  if(num_empty != 0 && num_empty != length){ //If not empty and not full
    for(i = 0; i < length; i++){
      if(arr[i] != *bin_tag && arr[i+1] != 0){ //If no hit and next is not empy do nothing
      }
      else if(arr[i] != *bin_tag && arr[i+1] == 0){ //If no hit and next is empty
        arr[i+1] = *bin_tag; //Place tag
        break;
      }
      else if(arr[i] == *bin_tag){ //If hit
        *hit_status = 1; //Change hit status
        break;
      }

    }
  }
  if(num_empty == 0){ //If full
    for(i = 0; i < length; i++){
      if(arr[i] != *bin_tag && i != length -1){ //If no hit and not at end, do nothing
      }
      if(arr[i] != *bin_tag && i == length-1){ //If no hit and at end, miss
        throw_away = Shift(arr, length, 0);  //Shift all elements and remove one
        arr[length - 1] = *bin_tag; //Place tag at end of array
        break;
      }
      if(arr[i] == *bin_tag && i != length -1){ //If hit and not at end
        *hit_status = 1; //Change hit status
        break;

      }
      if(arr[i] == *bin_tag && i == length-1){ //If hit and at end
        *hit_status = 1; //Change hit status
        break;
      }
    }

  }
}
/*Function that iterates through cache and processes it with the Direct mapped schene.
@Args *arr - The long int array (cache) being iterated through and shifted.
@Args *bin_tag - Pointer to address of the binary tag, placed in cache.
@Args *hit_status -  Pass by reference that changes to 1 if a hit occurs.
@Args index - The index in which the element is to be placed
*/
void DM_Process(long int *arr, long int *bin_tag, int*hit_status, int index){
  if(arr[index] == 0){ //If the cell is empty
    arr[index] = *bin_tag; //Place binary tag

  }
  else if(arr[index] == *bin_tag){ //If cell is not empty and hit
    *hit_status = 1; //Change hit status
  }
  else if(arr[index] != 0 && arr[index] != *bin_tag){ //If cell is not empty and miss
    arr[index] = *bin_tag; //Rewrite location as binary tag
  }


}
/* Add more global variables and/or functions as needed */

int main(int argc, char** argv)
{

    /*
     * Read command-line parameters and initialize:
     * cache_size, block_size, cache_mapping and cache_replacement variables
     */

    if ( argc != 4 ) { /* argc should be 4 for correct execution */
        printf("Usage: ./cache_sim [cache size: 64-8192] [cache block size: 32/64/128] [cache mapping: DM/FIFO/LRU]\n");
        exit(-1);
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */

        /* Set block and cache size in bytes */
        cache_size = atoi(argv[1]);
        block_size = atoi(argv[2]);
        assert(cache_size >= 256 && cache_size <= 8192);
        /* cache_size must be power of 2 */
        assert(!(cache_size & (cache_size-1)));
        assert(block_size >= 16 && block_size <= 256);
        /* block_size must be power of 2 */
        assert(!(block_size & (block_size-1)));
        assert(block_size <= cache_size);

        /* Set Cache Mapping */
        if (strcmp(argv[3], "DM") == 0) {
            cache_mapping = dm;
            cache_replacement = none;
        } else if (strcmp(argv[3], "FIFO") == 0) {
            cache_mapping = fa;
            cache_replacement = fifo;
        } else if (strcmp(argv[3], "LRU") == 0 ) {
            cache_mapping = fa;
            cache_replacement = lru;
        } else {
            printf("Unknown cache mapping: %s\n", argv[3]);
            exit(-1);
        }

    }

    num_blocks = cache_size/block_size; //Calculate num blocks
    num_bits_for_block_offset = log(block_size)/log(2); //Calculate num bits for offset
    if(cache_mapping == 1){ //If cache mapping is FA
      num_bits_for_index = 0; //No index, set to zero
    }
    else if(cache_mapping == 0){ //If cache mapping is DM
      num_bits_for_index = log(num_blocks)/log(2); //Calculate num bits for index
    }
    num_bits_for_tag = 32 - num_bits_for_index - num_bits_for_block_offset; //Calculate tag
    long int *cache; //Declare cache
    cache = initiate_cache(num_blocks); //Allocate memory for cache




    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file =fopen("mem_trace.txt","r");
    if (!ptr_file) {
        printf("Unable to open the trace file\n");
        exit(-1);
    }

    /* Reset the result structure */
    memset(&result, 0, sizeof(result));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task */

    /* Loop until whole trace file has been read */
    mem_access_t access;
    result.instruction_hits = 0;
    result.data_hits = 0;
    result.instruction_accesses = 0;
    result.data_accesses = 0;
    int ihits = 0; //Number of instruction hits
    int dhits = 0; //Number of data hits
    int iaccesses = 0; //Number of instruction accesses
    int daccesses = 0; //Number of data accesses
    int counter = 0; //Counter used in debugging
    while(1) {
      counter++; //Increment counter
      int hit_marker = 0; //Value for if hit occurred

        access = read_transaction(ptr_file);
        //If no transactions left, break out of loop
        if (access.address == 0)
            break;
        /* Add your code here */
        else{
          char *addressstr = malloc(sizeof(char) * 8); //Allocate memory for string hex address
          if(access.accesstype == 0){ //If instruction
            sprintf(addressstr, "%08x", access.address); //Convert uint_32t address to string
            iaccesses++; //Increment instruction accesses
          }
          if(access.accesstype == 1){ //If data
            sprintf(addressstr, "%08x", access.address); //Convert uint_32t address to string
            daccesses++; //Increment data accesses
          }
          char *bin_address = malloc(sizeof(char) * 33); //Allocate memory for string binary address
          hex_to_bin_rep(addressstr, bin_address); //Convert hex to binary
          //printf("%s\n",addressstr);
          free(addressstr); //Free hex address holder
          int dm_index = get_str_index(bin_address, num_bits_for_tag, num_bits_for_index); //Get direct map index
          bin_address[num_bits_for_tag] = '\0'; //Cut binary tag into tag
          long int bin_tag; //Initialize long int for tag
          bin_tag = bin_str_to_dec(bin_address); //Covnert binary string tag to long int tag
          //printf("%ld: %d\n",bin_tag, dm_index);
          //printf("%s\n",bin_address);
          free(bin_address); //Free binary address string
          if(cache_replacement == 0){ //if FIFO
            FIFO_Process(cache, num_blocks, &bin_tag, &hit_marker);} //Fifo scheme
          if(cache_replacement == 1){ //If LRU
            LRU_Process(cache,num_blocks, &bin_tag, &hit_marker);} //LRU scheme
          if(cache_replacement == 2){ //If direct mapped
            DM_Process(cache, &bin_tag, &hit_marker, dm_index);} //Direct mapped cache

          if(hit_marker == 1){ //If hit
            if(access.accesstype == 0){ //if instruction
              ihits++; //Increment instruction hits
            }
            if(access.accesstype == 1){ //if data
              dhits++; //Increment data hits
            }
          }
        }
      }

    result.data_hits = dhits;
    result.instruction_hits = ihits;
    result.data_accesses = daccesses;
    result.instruction_accesses = iaccesses;
    free(cache);
    /* Do not modify code below */
    /* Make sure that all the parameters are appropriately populated */
    print_statistics(num_blocks, num_bits_for_block_offset, num_bits_for_index, num_bits_for_tag, result);

    /* Close the trace file */
    fclose(ptr_file);

    return 0;
}
