/* list.h 
*/

#ifndef _LIST_H
#define _LIST_H

#define TRUE_ true
#define FALSE_ false

/* The following class defines a "list element" -- which is
 * used to keep track of one item on a list.  It is equivalent to a
 * Internal data structures kept public so that LIST operations can
 * access them directly.
 */
 
class LISTElement {
   public:
     LISTElement(void *itemPtr, int sortKey);  /* initialize a list element */

     LISTElement *next;		/* next element on list, 
				   NULL if this is the last */
     int key;		    	/* priority, for a sorted list */
     void *item; 	    	/* pointer to item on the list */
};

/* The following class defines a "list" -- a singly linked list of
 * list elements, each of which points to a single item on the list.
 *
 */
 
class LIST {
  public:
    LIST();			/* initialize the list  */
    ~LIST();			/* de-allocate the list */

    bool IsEmpty();		/* is the list empty? */
    void Insert(void *item, int sortKey);	/* Put item into list */
    void *Remove(int *keyPtr); 	  	/* Remove first item from list */

  private:
    LISTElement *first;  	/* Head of the list, NULL if list is empty */
    LISTElement *last;		/* Last element of list */
};

#endif /* _LIST_H */
