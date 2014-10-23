/**\file: bgDataStructures.h
*Common data structures. (stack, list, tree, etc.)
*/
#ifndef _BG_DATASTRUCTURES_H
#define _BG_DATASTRUCTURES_H

/*
##################################################################################
Stack
##################################################################################
*/
/**
\def BGSTACK_DEPTH_MAX 
Defines maximum number of items on the stack (stack depth)
*/
#define BGSTACK_DEPTH_MAX	64


/**
\struct bgStack_typ 
Data object that represents a stack (of ptr-s)\n
*/
typedef struct bgStack_typ{
    int index;	/**<Position of the next empty location on the stack.*/
	char* data[BGSTACK_DEPTH_MAX]; /**<Array that holds the stack.*/
}bgStack_typ;



/**
Method for data object \link bgStack_typ \endlink\n
This method must be called first, prior from calling any other method related to this data object.
\param stack Ptr to data object: (\link bgStack_typ \endlink)
*/
void bgStackInit(bgStack_typ* stack);


/**
Method for data object \link bgStack_typ \endlink\n
Push item to the stack.
\param stack Ptr to data object: (\link bgStack_typ \endlink)
\param data Ptr that will be pushed / stored on the stack.
\return Same ptr that is pushed to the stack (data) or NULL if stack if full.
*/
char* bgStackPush(bgStack_typ* stack, char* data);


/**
Method for data object \link bgStack_typ \endlink\n
Pop (retrive) item from the stack.
\param stack Ptr to data object: (\link bgStack_typ \endlink)
\return Ptr that represent item from the stack (or NULL if none)
*/
char* bgStackPop(bgStack_typ* stack);


/**
Method for data object \link bgStack_typ \endlink\n
Retrieve top item (ptr) from the stack wo. removing it from the stack.
\param stack Ptr to data object: (\link bgStack_typ \endlink)
\return Ptr that represent top item from the stack (or NULL if none)
*/
char* bgStackPeek(bgStack_typ* stack);


/**
Method for data object \link bgStack_typ \endlink\n
Return number of items stored on the stack.
\param stack Ptr to data object: (\link bgStack_typ \endlink)
\return Number of items on the stack.
*/
int bgStackGetItemCount(bgStack_typ* stack);



/*
##################################################################################
Tree  of Nodes (Each nodes has lf,rt,up,dw connections - 4C(onnections) nodes)
##################################################################################
*/

/**
\struct bgTree4CNode_typ 
Data object represents a node w. 4-way connections (left, right, up, down)
*/
typedef struct bgTree4CNode_struct{
	int used;				/**<0-not used; 1-used*/
	int id;					/**<Node Id (unique identifier); 0 to (nodeCountMax - 1)*/
	int nodeCountMax;		/**<Max number of nodes (depends on the available memory)*/
	struct bgTree4CNode_struct*	up;	/**<Link (ptr) to the up node (or 0 if none)*/
	struct bgTree4CNode_struct*	dw; /**<Link (ptr) to the down node (or 0 if none)*/
	struct bgTree4CNode_struct*	rt; /**<Link (ptr) to the right node (or 0 if none)*/
	struct bgTree4CNode_struct*	lf; /**<Link (ptr) to the left node (or 0 if none)*/
	int dataType;			/**<Type of data node is holding*/
	int dataValue;			/**<Local data storage; used for simple, scalar types (int, bool)*/
	char* data;				/**<Ptr to data storage (string type, etc.)*/
}bgTree4CNode_typ;


/**
\struct bgTree4C_typ 
Data object holds a tree of 4-way nodes.
*/
typedef struct bgTree4C_typ{
	int nodeCountUsed;			/**<Number of used nodes*/
	int nodeCountMax;			/**<Max number of nodes*/
	bgTree4CNode_typ* nodes;	/**<Ptr to user supplied buffer / memory for node storage*/
}bgTree4C_typ;


/**
Method for data object \link bgTree4C_typ \endlink\n
This method must be called first, prior from calling any other method related to this data object.
\param tree Ptr to data object: (\link bgTree4C_typ \endlink)
\param treeMem Ptr to user supplied buffer / memory of nodes
\param treeMemSize Size (bytes) of user supplied buffer / memory of nodes

*/
void bgTree4CInit(bgTree4C_typ* tree, char* treeMem, int treeMemSize);

void bgTree4CNodeClr(bgTree4CNode_typ* node);

bgTree4CNode_typ* bgTree4CNodeCreate(bgTree4C_typ* tree);
void bgTree4CNodeDelete(bgTree4C_typ* tree, bgTree4CNode_typ* node);

bgTree4CNode_typ*  bgTree4CNodePreviousLevel(bgTree4CNode_typ* node1);
bgTree4CNode_typ* bgTree4CGetNextNodeRtDw(bgTree4CNode_typ* node1);
bgTree4CNode_typ* bgTree4CGetNextNodeDw(bgTree4CNode_typ* node1);
bgTree4CNode_typ*  bgTree4CGetRootNode(bgTree4C_typ* tree);
bgTree4CNode_typ* bgTree4CNodeGetNextLf(bgTree4CNode_typ* node);


void bgTree4CNodeAddDw(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2);
void bgTree4CNodeAddRt(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2);
int bgTree4CNodeAppendToDwEnd(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2);
int bgTree4CNodeAppendToRtEnd(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2);
int	bgTree4CNodeInsertRt(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2);
int	bgTree4CNodeInsertLf(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2);


void bgTree4CNodeSetDataType(bgTree4CNode_typ* node, int dataType);
void bgTree4CNodeSetDataValue(bgTree4CNode_typ* node, int dataVal);
void bgTree4CNodeSetDataPtr(bgTree4CNode_typ* node, char* data);

int bgTree4CGetCountFreeNodes(bgTree4C_typ* tree);

void bgTree4CPrint(bgTree4C_typ* tree);

#endif
