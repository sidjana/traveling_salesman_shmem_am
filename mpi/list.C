/* list.C 

      Routines to manage a singly-linked list of "things".
      It's a priority queue, but it could be more efficient 
      implemented  as a heap.
*/

#include <stdlib.h>
#include "list.h"


/*----------------------------------------------------------------------
 * LISTElement::LISTElement
 * 	Initialize a list element, so it can be added somewhere on a list.
 *
 *	"itemPtr" is the item to be put on the list.  It can be a pointer
 *		to anything.
 *	"sortKey" is the priority of the item, if any.
 *--------------------------------------------------------------------*/

LISTElement::LISTElement(void *itemPtr, int sortKey)
{
     item = itemPtr;
     key = sortKey;
     next = NULL;	/* assume we'll put it at the end of the list */
}


/*----------------------------------------------------------------------
 * LIST::LIST
 *	Initialize a list, empty to start with.
 *	Elements can now be added to the list.
 *--------------------------------------------------------------------*/

LIST::LIST()
{ 
    first = last = NULL; 
}

/*----------------------------------------------------------------------
 * LIST::~LIST
 *	Prepare a list for deallocation.  If the list still contains any 
 *	LISTElements, de-allocate them.  However, note that we do *not*
 *	de-allocate the "items" on the list -- this module allocates
 *	and de-allocates the LISTElements to keep track of each item,
 *	but a given item may be on multiple lists, so we can't
 *	de-allocate them here.
 *--------------------------------------------------------------------*/

LIST::~LIST()
{ 
    while (Remove(NULL) != NULL)
	;	 // delete all the list elements
}


/*----------------------------------------------------------------------
 * LIST::IsEmpty
 *      Returns TRUE if the list is empty (has no items).
 *--------------------------------------------------------------------*/

bool
LIST::IsEmpty() 
{ 
    if (first == NULL)
        return TRUE_;
    else
	return FALSE_; 
}

/*----------------------------------------------------------------------
 * LIST::Insert
 *      Insert an "item" into a list, so that the list elements are
 *	sorted in increasing order by "sortKey".
 *      
 *	"item" is the thing to put on the list, it can be a pointer to 
 *		anything.
 *	"sortKey" is the priority of the item.
 *--------------------------------------------------------------------*/

void
LIST::Insert(void *item, int sortKey)
{
    LISTElement *element = new LISTElement(item, sortKey);
    LISTElement *ptr;

    if (IsEmpty()) {	/* if list is empty, put */
        first = element;
        last = element;
    } else if (sortKey < first->key) {	
		/* item goes on front of list */
	element->next = first;
	first = element;
    } else {		/* look for first element in list bigger than item */
        for (ptr = first; ptr->next != NULL; ptr = ptr->next) {
            if (sortKey < ptr->next->key) {
		element->next = ptr->next;
	        ptr->next = element;
		return;
	    }
	}
	last->next = element;		/* item goes at end of list */
	last = element;
    }
}

/*--------------------------------------------------------------------
 * LIST::Remove
 *      Remove the first "item" from the front of a list.
 * 
 * Returns:
 *	Pointer to removed item, NULL if nothing on the list.
 *	Sets *keyPtr to the priority value of the removed item
 *
 *	"keyPtr" is a pointer to the location in which to store the 
 *		priority of the removed item.
 *------------------------------------------------------------------*/

void *
LIST::Remove(int *keyPtr)
{
    LISTElement *element = first;
    void *thing;

    if (IsEmpty()) 
	return NULL;

    thing = first->item;
    if (first == last) {	/* list had one item, now has none */
        first = NULL;
	last = NULL;
    } else {
        first = element->next;
    }
    if (keyPtr != NULL)
        *keyPtr = element->key;
    delete element;
    return thing;
}

