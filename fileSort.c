#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

struct node{
  void * data; // can cast to either a string or int
  struct node * next; //points to the next node
  struct node * prev; // points to previous node 
  int isQSorted; // a variable that tells whether or not the node has been Quicksorted or not! 1 if quicksorted, 0 if not
};

//global variables
struct node* headNode;
int isNums = 0;
char* temp;

//prototypes
int intComparator (void* thing1, void* thing2);
int charComparator (void* thing1, void* thing2);
int insertionSort(void* toSort, int(*comparator)(void*, void*));
int quickSort(void* toSort, int(*comparator)(void*, void*));
void print();
void cleanRead(char*, int);
int loader(struct node*, char*, int);
void insertionSortHelper(void* toSort, int(*comparator)(void*, void*));
void linkedFree();

int main (int argc, char ** argv){
  //open the file and store in buffer
  int fd = open(argv[2], O_RDONLY);
  //check is the given file exists
  if(fd < 0){
    //file does not exist it is a fatal error
    printf("FATAL ERROR: File does not exist\n");
    //exit the code
    exit(1);
  }
  //buffer size
  int buffSize = 100;
  //create the buffer to store the file
  char *myBuffer = (char *)malloc(101*sizeof(char));
  //check if the pointer is null
  if(myBuffer == NULL){
    //print an error
    printf("ERROR: Not enough space on heap\n");
  }
  //set everything in the buffer to the null terminator
  memset(myBuffer, '\0', 101);
  //store the status of the read
  int readIn = 0;
  int status = 1;
  //temp pointer to hold the dynamically sized buffer
  char *temp;
  //loop until everything has been read
  while(status > 0){ 
    //read buff size number of chars and store in myBuffer
    do{
      status = read(fd, myBuffer+readIn, 100);
      readIn += status;
    }while(readIn < buffSize && status > 0);
    //check if there are more chars left
    if(status > 0){
      //increase the array size by 100
      buffSize += 100;
      //store the old values in the temp pointer
      temp = myBuffer;
      //malloc the new array to myBuffer
      myBuffer = (char *)malloc(buffSize*sizeof(char));
      //check if the pointer is null
      if(temp == NULL){
        //print an error
        printf("ERROR: Not enough space on heap\n");
      }
      //copy the old memory into the new buffer
      memcpy(myBuffer, temp, readIn);
      //free the old memory that was allocated
      free(temp);
    }
  }
  //now we want to go through the buffer and clean and load the nodes
  cleanRead(myBuffer, readIn);
  //check to see if the head node is null
  if(headNode == NULL){
    //print a warning
    printf("WARNING: File seems to be empty\n");
  }
  //check the user defined sorting function
  if(argv[1][1] == 'q' && isNums){
    //pass in the numbers function pointer
    quickSort(headNode, intComparator);
  } else if(argv[1][1] == 'q'){
    //pass in the strings function pointer 
    quickSort(headNode, charComparator);
  } else if(argv[1][1] == 'i' && isNums){
    //pass in the numbers function pointer using insertion sort
    insertionSort(headNode, intComparator);
  } else if(argv[1][1] == 'i'){
    //pass in the numbers function pointer using insertion sort
    insertionSort(headNode, charComparator);
  } else{
    //there is a fatal error
    printf("FATAL ERROR: User entered invalid sort function\n");
    //exit with a failure
    exit(1);
  }
  //print the nodes
  print();
  //free the linked list
  linkedFree();
  //free the buffer
  free(myBuffer);
  //return
  return 0;
}

//prints the linked list
void print(){
  //the traversal node
  struct node* curr = headNode;
  //int temp;
  //loop through all the nodes
  while(curr != NULL){
    if(isNums){
      //temp = *(int *)(curr -> data);
      //print the int in the node
      printf("%d\n", *(int *)(curr -> data));
    } else{
      //print the data in the node
      printf("%s\n", (char *)(curr -> data));
    }
    //increment curr
    curr = curr -> next;
  }
}

//frees the linked list
void linkedFree(){
  //the traversal node
  struct node* curr = headNode;
  struct node* temp;
  //loop through all the nodes
  while(curr != NULL){
    //set temp to curr
    temp = curr;
    //increment curr
    curr = curr -> next;
    //free the data inside temp first
    free(temp->data);
    //free the temp pointer
    free(temp);
  }
}

//gets rid of the extra spaces and other stuff while still storing in nodes
void cleanRead(char *buff, int buffSize){
  //current progress of the reading
  int currIndex = 0;
  //a temp node
  struct node* temp;
  //loop until we have read everything
  while(currIndex < buffSize){
    //first check if the headnode is null
    if(headNode == NULL){
      //malloc the headnode
      headNode = malloc(sizeof(struct node));
      //check if the pointer is null
      if(headNode == NULL){
        //print an error
        printf("ERROR: Not enough space on heap\n");
      }
      //load data into the node
      currIndex = loader(headNode, buff, 0);
      //make headnode's next null
      headNode -> next = NULL;
    } else{
      //malloc a temp node
      temp = malloc(sizeof(struct node));
      //check if the pointer is null
      if(temp == NULL){
        //print an error
        printf("ERROR: Not enough space on heap\n");
      }
      //load data into the node
      currIndex = loader(temp, buff, currIndex);
      //make the head node's previos node the temp
      headNode -> prev = temp;
      //make the temp's next node the head
      temp -> next = headNode;
      //reassign the head node
      headNode = temp;
    }
  }
}

//loader method into the given node
int loader(struct node * myNode, char *buff, int sIndex){
  //integer declarations
  int* idata = (int *)malloc(sizeof(int));
  //check if the pointer is null
  if(idata == NULL){
    //print an error
    printf("ERROR: Not enough space on heap\n");
  }
  int counter = sIndex;
  int localcount = 0;
  int isNeg = FALSE;
  char* temp;
  //allocate starting space into cdata
  char *cdata = (char *)malloc(11*sizeof(char));
  //check if the pointer is null
  if(cdata == NULL){
    //print an error
    printf("ERROR: Not enough space on heap\n");
  }
  //set the array to be all null terminators
  memset(cdata, '\0', 11);
  //current size of cdata
  int currSize = 10;
  //loop through the string until we hit a comma or terminator
  while(buff[counter] != ',' && buff[counter] != '\0'){
    //check to see what type of data is in the buffer
    if(isdigit(buff[counter])){
      //change the global int var to true
      isNums = TRUE;
      //add to the idata int
      *idata *= 10;
      //we don't use atoi because that method is for strings
      *idata += ((int)(buff[counter])-48);
    } else if(isalpha(buff[counter])){
      //store the character in the node array
      if(localcount == currSize){
        //make the cdata array 10 characters bigger
        currSize += 10;
        //store the old values in the temp pointer
        temp = cdata;
        //malloc the new array to myBuffer
        cdata = (char *)malloc(currSize*sizeof(char));
        //check if the pointer is null
        if(cdata == NULL){
          //print an error
          printf("ERROR: Not enough space on heap\n");
        }
        //set the random stuff to null terminators again
        memset(cdata, '\0', currSize);
        //copy the old memory into the new buffer
        memcpy(cdata, temp, localcount);
        //free the old variable
        free(temp);
        //store the buffer char
        cdata[localcount] = buff[counter];
      } else{
        //store the buffer character into the cdata character
        cdata[localcount] = buff[counter];
      }
    } else if(buff[counter] == '-'){
      //set is neg to 1 because the int is a negative number
      isNeg = TRUE;
    } else{
      //it must be a space decrement local count by 1
      localcount--;
    }
    //increment the counters
    counter++;
    localcount++;
  }
  //check to see what gets stored in the given node
  if(isNums){
    //check if the int has to be made negative
    if(isNeg){
      //multiply by negative 1
      *idata *= -1;
    }
    //store the integer in the node
    myNode -> data = idata; 
    myNode -> isQSorted = 0;
    //free the space used for cdata
    free(cdata);
  } else{
    //otherwise we store the word in the node
    myNode -> data = cdata;
    myNode -> isQSorted = 0;
    //free the space used for the int
    free(idata);
  }
  //return where you left off and ignore the comma
  return ++counter;
}

int intComparator (void* thing1, void* thing2){
  //we are working with integers
  if(*(int *)thing1 > *(int *)thing2){
    //thing1 is greater than thing2
    return 1;
  } else if(*(int *)thing1 == *(int *)thing2){
    //the two integers are equal
    return 0;
  }else{
    //thing2 is greater than thing1
    return -1;
  }
}

//returns 1 is first string comes before second string in the dictionary
int charComparator (void* thing1, void* thing2){
  //we are dealing with strings
  char * arr1 = (char *)thing1;
  char * arr2 = (char *)thing2;
  //we are working with strings or characters
  int counter = 0;
  //loop through both the strings
  while(arr1[counter] != '\0' && arr2[counter] != '\0'){
    //compare the characters
    if(arr1[counter] > arr2[counter]){
      return 1;
    } else if(arr1[counter] < arr2[counter]){
      return -1;
    }
    //increment counter
    counter++;
  }
  //both strings have the same starting chars but arr1 is shorter
  if(strlen(arr1) < strlen(arr2)){
    //arr1 is still greater
    return -1;
  } else if(strlen(arr2) < strlen(arr1)){
    //arr2 is now greater
    return 1;
  } else{
    //they are both the same thing
    return 0;
  }
}

//helps insert the given node into the right position
void insertionSortHelper(void* toSort, int(*comparator)(void*, void*)){
  //declarations used for the sorting, sortee will be moved
  struct node * sortee = (struct node *)toSort;
  //extra nodes to do the sorting
  struct node * temp1;
  struct node * temp2;
  struct node * temp3;
  //loop through the nodes before the sortee while the sortee is less than its previous
  while(sortee -> prev != NULL && comparator(sortee->data, sortee->prev->data) == -1){
    //stores the sortee's previous value (cannot be null)
    temp1 = sortee->prev;
    //this one may be null
    temp2 = sortee->prev->prev;
    //this one also may be null
    temp3 = sortee->next;
    //sortee's new next should be the old previous
    sortee -> next = temp1;
    //sortee and the node before it are getting switched
    sortee -> prev = temp2;
    //the previous node's previous is now sortee
    temp1 -> prev = sortee;
    //the old previous node's next is now sortee's next
    temp1 -> next = temp3;
    //if temp 2 is not null, make its next the sortee
    if(temp2 != NULL){temp2 -> next = sortee;}
    //if sortee's next is not null, make its previoous temp 1
    if(temp3 != NULL){
      temp3 -> prev = temp1;
    }
  }
  //check if sortee is the new head node
  if(sortee-> prev == NULL){
    //set the headnode to be sortee
    headNode = sortee;
  }
}

/* How insertion sort basically works is that the first node is ALREADY SORTED in theory! */
int insertionSort(void* toSort, int(*comparator)(void*, void*)) {
  //declarations used for the sorting, sortee will be moved
  struct node * sortee = toSort;
  //if the sortee is null, there is nothing in the linked list
  if(sortee == NULL){
    //the list is null
    return 0;
  } else if(sortee->prev == NULL && sortee->next == NULL) {
    //its the only node in the linked list 
    return 0;
  }
  //because the headnode is technically already sorted, progress to the next node
  sortee = sortee->next;
  //compare the current value with its next value unless sortee is at the end
  while(sortee != NULL) {
    //insert the given sortee into the correct position of the linked list
    insertionSortHelper(sortee, comparator);
    //change sortee's to its next
    sortee = sortee -> next;
  }
  //we're finished
  return 0;
}

int quickSort(void* toSort, int(*comparator)(void*, void*)) {
  // type casting the node that was inserted to be of type struct node *, we are making pivot to be the first node of the linked list
  struct node * pivot = (struct node *)toSort; 
  // this means the node that was given does not exist and therefore cannot be sorted or is already sorted in theory
  if(pivot == NULL) { 
    //exit the call
    return 0;
  }
  // means that this is a single node linked list and is already sorted in theory!
  if(pivot->next == NULL && pivot->prev == NULL) {
    //exit the call
    return 0;
  }
  //we have hit the end of the linked list
  if(pivot->next == NULL && pivot->prev != NULL) {
    //check if the node behind pivot has been sorted
    if(pivot->prev->isQSorted == 1) {
      //the pivot node is now sorted
      pivot->isQSorted = 1;
      //exit the call
      return 0;
    }
    //TODO: Understand this else statement
    else {
      pivot->prev->isQSorted = 1;
      return 0;
    }
  }
  //we are at the start of the linked list
  if(pivot->next != NULL && pivot->prev == NULL) {
    //check if the node next to the pivot has somehow already been sorted
    if(pivot->next->isQSorted == 1) {
      //set the pivot to sorted
      pivot->isQSorted = 1;
      //exit the call
      return 0;
    }
  }
  //if the pivot's next has been sorted and the previous has also been sorted
  if(pivot->next->isQSorted == 1 && pivot->prev->isQSorted == 1) {
    //this node is surrounded by nodes that have already been sorted and in turn has already been sorted as well!
    pivot->isQSorted = 1;
    //exit the call
    return 0;
  }
  // the lcursor will always begin at the node to the right of pivot
  struct node * lcursor = pivot->next;
  // declaring and initializing the rcursor
  struct node * rcursor = pivot; 
  // this while loop will put the rcursor pointer at the end of the linked list or sub linked list it is working on!
  while(rcursor->next != NULL) {
    // the next node is sorted so you dont want to move past that
    if(rcursor->next->isQSorted == 1) { 
      break;
    } else {
      // otherwise go to the next node
      rcursor = rcursor-> next;
    }
  }
  // what the comparator will return to compare two different numbers
  int compRet;
  // declares 2 temp variables of type void* so they can be casted for use later!
  void * temp1;
  void * temp2;
  //loop until the nodes crossover
  while (lcursor->prev != rcursor && rcursor->next != lcursor) {
    //move lcursor until it finds a value greater than the pivot
    while((compRet = comparator(lcursor->data, pivot->data)) != 1 && lcursor->next != NULL) {
      //don't cross the data that has already been sorted
      if (lcursor->next->isQSorted == 1){
        break;
      } else {
        //increment the left cursor until it finds something bigger than pivot
        lcursor = lcursor->next;
      }
    }
    //move rcursor until it finds a node smaller than the pivot
    while((compRet = comparator(rcursor->data, pivot->data)) == 1 && rcursor-> prev != NULL) {
      //if we hit a point that has already been sorted, we need to stop
      if(rcursor->prev->isQSorted == 1) {
        break;
      } else {
        //increment the right cursor until it finds something smaller than the pivot
        rcursor = rcursor->prev;
      }
    }
    //both nodes are at the same location
    if(lcursor == rcursor) {
      //check to see if we are at the end of the list
      if(rcursor->next == NULL) {
        //if so, save pivot and rcursor data
        temp1 = pivot->data;
        temp2 = rcursor->data;
        //switch the two, so that pivot is in its right place
        pivot->data = temp2;
        rcursor->data = temp1;
        //Update the rcursor to show that it has been sorted
        rcursor->isQSorted = 1;
        //check if we have finished sorting the whole list
        if(pivot->next != NULL && pivot->next->isQSorted == 0) {
          //call quicksort again on the pivot
          return quickSort(pivot, comparator);
        } else {
          //we should be finished basically, but call quicksort again anyway?
          return quickSort(rcursor, comparator);
        }
      } else {
        //check if rcursor is at the end of its allowed space
        if(rcursor->next->isQSorted == 1) {
          //we need to switch data
          temp1 = pivot->data;
          temp2 = rcursor->data;
          //switch the two nodes' data
          pivot->data = temp2;
          rcursor->data = temp1;
          //update rcursor to show that it is now sorted
          rcursor->isQSorted = 1;
          //check if we're done with sorting the whole list
          if(pivot->prev != NULL && pivot->prev->isQSorted == 1 && pivot->next->isQSorted == 1) {
            return quickSort(pivot, comparator);
          }
          else if(pivot->next != NULL && pivot->next->isQSorted == 0) {
            //call the quicksort again on the pivot
            return quickSort(pivot, comparator);
          } else {
            //again, we should be finished, but I guess this works
            return quickSort(rcursor, comparator);
          }
        }
      }
    }
    //check that there is no overlap
    if(lcursor->prev != rcursor && rcursor->next != lcursor) {
      //now store the data
      temp1 = lcursor->data;
      temp2 = rcursor->data;
      //and switch the two cursors to their rightful positions
      lcursor->data = temp2;
      rcursor->data = temp1;
    }
  }
  //when this previous while loop breaks it means the lcursor and rcursor have overlapped!
  if(rcursor == pivot) { 
    // THIS MEANS we have moved rcursor all the way to the left because it could not find an element smaller than the pivot, aka pivot is first node
    pivot -> isQSorted = 1;
    //that means pivot is good and we make the next node the new pivot
    return quickSort(pivot->next,comparator);
  } else if (lcursor -> next == NULL) { 
    // THIS MEANS the largest element in the linked list is the pivot or lcursor has reached the end
    if((compRet = comparator(lcursor->data, pivot->data) != 1)) {
      //switch the data accordingly
      temp1 = lcursor->data;
      temp2 = pivot->data;
      //we have found the rightful place for the pivot
      lcursor->data = temp2;
      pivot->data = temp1;
      //set it so that lcursor is now sorted
      lcursor->isQSorted = 1;
      //call quicksort again on the new pivot
      return quickSort(pivot, comparator);
    } else {
      //largest element has not reached the end and we are somewhere in the middle
      temp1 = pivot->data;
      temp2 = rcursor->data;
      //switch data accordingly
      pivot->data = temp2;
      rcursor->data = temp1;
      
      if(pivot->prev != NULL && (compRet = comparator(pivot->next->data, pivot->data) == 1)) {
        if(pivot->prev->isQSorted == 1) {
          pivot->isQSorted = 1;
          return quickSort(pivot->next,comparator);
        }
      }
      else if((compRet = comparator(pivot->next->data,pivot->data) != 1)) {
        lcursor->isQSorted = 1;
        return quickSort(pivot, comparator);
      }
      else {
        rcursor->isQSorted = 1;
        quickSort(pivot, comparator);
        quickSort(lcursor, comparator);
      }
    }
  }
  else {
    temp1 = pivot-> data;
    temp2 = rcursor->data;
    pivot->data = temp2;
    rcursor->data = temp1; // this means rcursor now has pivots data and is now SORTED
    rcursor-> isQSorted = 1;
    quickSort(rcursor->next, comparator); // this is the second half of the linked list!
    quickSort(pivot, comparator);
  }
  return 0;
}