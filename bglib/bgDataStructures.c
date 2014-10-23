#ifndef _BG_DATASTRUCTURES_C
#define _BG_DATASTRUCTURES_C

#include "bgDataStructures.h"

#ifdef WIN32
#include <stdio.h>
#define DBGMSG printf
#else
#include "bgdebug.h"
#endif


#ifdef WIN32
static char* pTabs[30]={
"",
"    ",
"        ",
"            ",
"                ",
"                    ",
"                        ",
"                            ",
"                                ",
"                                    ",
"                                        ",
"                                            ",
"                                                ",
"                                                    ",
"                                                        ",
"                                                            ",
"                                                                ",
"                                                                    ",
};


static char* pStackDepth[30]={
" ",
"=",
"==",
"===",
"====",
"=====",
"======",
"=======",
"========",
"=========",
"==========",
"===========",
"============",
"=============",
"==============",
"===============",
"================",
"=================",
"==================",
};
#endif


/*
##################################################################################
Stack
##################################################################################
*/
void bgStackInit(bgStack_typ* stack)
{
    stack->index = 0;
}

char* bgStackPush(bgStack_typ* stack, char* data)
{
    if(stack->index >= BGSTACK_DEPTH_MAX)
		return 0;
/*
    printf("[%2d]push:%s\n", (int)data, pStackDepth[stack->index]);
*/
    stack->data[stack->index] = data;
    stack->index++;

	return data;
}

char* bgStackPop(bgStack_typ* stack)
{
    if(!stack->index)
        return 0;

    stack->index--;
/*
    printf("[%2d]pop :%s\n", (int)(stack->data[stack->index]), pStackDepth[stack->index]);
*/
    return stack->data[stack->index];
}

char* bgStackPeek(bgStack_typ* stack)
{
    if(!stack->index)
        return 0;

    return stack->data[stack->index - 1];
}

int bgStackGetItemCount(bgStack_typ* stack)
{
    return stack->index;
}



/*
##################################################################################
Tree  of Nodes (Each nodes has lf,rt,up,dw connections - 4C(onnections) nodes)
##################################################################################
*/
void bgTree4CInit(bgTree4C_typ* tree, char* pTreeMem, int treeMemSize)
{
int i;
bgTree4CNode_typ* node;

	if(!pTreeMem || (treeMemSize < 256) ){
		DBGMSG("bgTree4CInit: not enough memory!\n");
		tree->nodeCountMax = 0;
		return;
	}


	tree->nodeCountMax = treeMemSize / sizeof(bgTree4CNode_typ);
#ifdef WIN32
	DBGMSG("bgTree4CInit: max. number of nodes=%d\n", tree->nodeCountMax);
#endif
	tree->nodeCountUsed = 0;
	tree->nodes = (bgTree4CNode_typ*)pTreeMem;
	for(i=0; i<tree->nodeCountMax;i++){
        node = &tree->nodes[i];
        node->id = i;
		node->nodeCountMax = tree->nodeCountMax;
        bgTree4CNodeClr(node);
	}
}

void bgTree4CNodeClr(bgTree4CNode_typ* node)
{
	node->used = 0;	
	node->up = 0;	
	node->dw = 0;	
	node->rt = 0;	
	node->lf = 0;	
	node->data = 0;	
	node->dataType = 0;	
}

bgTree4CNode_typ* bgTree4CNodeCreate(bgTree4C_typ* tree)
{
int i;
	if(!tree->nodeCountMax)
		return NULL;


	if( tree->nodeCountUsed >= tree->nodeCountMax){
		return NULL;
	}

	for(i=0; i<tree->nodeCountMax;i++){
		if(tree->nodes[i].used == 0){
			tree->nodes[i].used = 1;
			tree->nodeCountUsed++;
			return &tree->nodes[i];
		}	
	}
	return NULL;
}

int bgTree4CGetCountFreeNodes(bgTree4C_typ* tree)
{
	return (tree->nodeCountMax -  tree->nodeCountUsed);
}

bgTree4CNode_typ*  bgTree4CGetRootNode(bgTree4C_typ* tree)
{
    return &tree->nodes[0];
}

void bgTree4CNodeDelete(bgTree4C_typ* tree, bgTree4CNode_typ* node)
{
	if(!tree->nodeCountMax)
		return;
		
	if(node->used){
#ifdef WIN32
        DBGMSG("bgTree4CNodeDelete(): node deleted, id=%d\n", node->id);
#endif
        bgTree4CNodeClr(node);
		if( tree->nodeCountUsed )
			tree->nodeCountUsed--;	
	}
}

bgTree4CNode_typ*  bgTree4CNodePreviousLevel(bgTree4CNode_typ* node1)
{
bgTree4CNode_typ* n1 = node1;
bgTree4CNode_typ* n2;
int i;

	if( !n1 )
		return 0;

	if( n1->lf)
		return n1->lf;

	if( !n1->up )
		return 0;

    for(i=0;i<node1->nodeCountMax;i++){
        n2 = n1->up;

        if( !n2 )
			return n1->lf;

		n1 = n2;
    }

	return 0;
}


void bgTree4CNodeAddDw(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2)
{
#ifdef WIN32
    if( node1->dw ){
        DBGMSG("bgTree4CNodeAddDw(): node1(id=%d) has dw(id=%d)\n ", node1->id, node1->dw->id);
    }

    if( node2->up ){
        DBGMSG("bgTree4CNodeAddDw(): node2(id=%d) has up(id=%d)\n ", node2->id, node2->up->id);
    }
#endif

    node1->dw = node2;
    node2->up = node1;
}

int bgTree4CNodeAppendToDwEnd(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2)
{
bgTree4CNode_typ* n1 = node1;
bgTree4CNode_typ* n2;
int i;

    for(i=0;i<node1->nodeCountMax;i++){
        n2 = n1->dw;
        if( n2 )
            n1 = n2;
        else{
            n1->dw = node2;
			node2->up = n1;
            return 0;
        }
    }

    return 1;
}

void bgTree4CNodeAddRt(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2)
{
#ifdef WIN32
    if( node1->rt ){
        DBGMSG("bgTree4CNodeAddDw(): node1(id=%d) has rt(id=%d)\n ", node1->id, node1->rt->id);
    }

    if( node2->lf ){
        DBGMSG("bgTree4CNodeAddDw(): node2(id=%d) has lf(id=%d)\n ", node2->id, node2->lf->id);
    }
#endif

    node1->rt = node2;
    node2->lf = node1;
}


int bgTree4CNodeAppendToRtEnd(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2)
{
bgTree4CNode_typ* n1 = node1;
bgTree4CNode_typ* n2;
int i;

    for(i=0;i<node1->nodeCountMax;i++){
        n2 = n1->rt;
        if( n2 )
            n1 = n2;
        else{
            n1->rt = node2;
            return 0;
        }
    }

    return 1;
}

int	bgTree4CNodeInsertRt(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2)
{
bgTree4CNode_typ* pNode;

	pNode = node1->rt;
	if( pNode ){
		pNode->lf = node2;
		node1->rt = node2;
		node2->lf = node1;
		node2->rt = pNode;
	}else{
		node1->rt = node2;
		node2->lf = node1;
	}
	return 0;
}


int	bgTree4CNodeInsertLf(bgTree4CNode_typ* node1, bgTree4CNode_typ* node2)
{
bgTree4CNode_typ* pNode;

	pNode = node1->lf;
	if( pNode ){
		pNode->rt = node2;
		node1->lf = node2;
		node2->rt = node1;
		node2->lf = pNode;
	}else{
		node1->lf = node2;
		node2->rt = node1;
	}
	return 0;
}


bgTree4CNode_typ* bgTree4CGetNextNodeRtDw(bgTree4CNode_typ* n1)
{
bgTree4CNode_typ* n2;

	if( !n1 )
		return 0;

	/*is node on the right of the current node defined?*/
	n2 = n1->rt;
    if( n2 ){
		/*right node found!*/
		return n2;
    } 
	
	/*current node has no "right" nodes*/


	/*any down nodes from the current node?*/
    n2 = n1->dw;

    if( n2 ){
		/*down node found!*/
		return n2;
    }            

	return 0;
}

bgTree4CNode_typ* bgTree4CGetNextNodeDw(bgTree4CNode_typ* n1)
{
	if( !n1 )
		return 0;

	/*any down nodes from the current node?*/
    return n1->dw;

}


void bgTree4CNodeSetDataType(bgTree4CNode_typ* node, int dataType)
{
    node->dataType = dataType;
}

void bgTree4CNodeSetDataValue(bgTree4CNode_typ* node, int dataVal)
{
    node->dataValue = dataVal;
}

void bgTree4CNodeSetDataPtr(bgTree4CNode_typ* node, char* data)
{
    node->data = data;
}

int bgTree4CNodeGetDataType(bgTree4CNode_typ* node)
{
    return node->dataType;
}

int bgTree4CNodeGetDataValue(bgTree4CNode_typ* node)
{
    return node->dataValue;
}

char* bgTree4CNodeGetDataPtr(bgTree4CNode_typ* node)
{
    return node->data;
}

void bgTree4CPrint(bgTree4C_typ* tree)
{
/*start w. root*/
bgTree4CNode_typ* n1 = &tree->nodes[0];
bgTree4CNode_typ* n2;
bgStack_typ stack;
bgStack_typ* s = &stack;

    bgStackInit(s);
    while(1){

		/*check if curent node exists*/
        if(!n1)
            return;

		/*display current node info*/			
/*
        DBGMSG("%sid=%d, type=%s, data=%s\n", pTabs[bgStackGetItemCount(s)], n1->id, xmlRpcKeywDict[n1->dataType], n1->data?n1->data:"nil");        
*/
#ifdef WIN32
		if(n1->data)
	        DBGMSG("nId=%d %s Type=%d [%s] Value=%d\n", n1->id, pTabs[bgStackGetItemCount(s)], n1->dataType, n1->data?n1->data:"?", n1->dataValue);        
		else
	        DBGMSG("nId=%d %s Type=%d Value=%d\n", n1->id, pTabs[bgStackGetItemCount(s)], n1->dataType, n1->dataValue);        
#endif

		/*first, print all nodes on the right from the current node*/

		/*is node on the right of the current node defined?*/
		n2 = n1->rt;
        if( n2 ){
			/*yes, right node defined; push current node to stack*/
            bgStackPush(s, (char*) n1);
			/*current node becomes the node from the right; noOfItemsOnTheStack == level is now increased!*/
			n1 = n2;
            continue;
        } 
		
		/*current node has no more "right" nodes*/


		/*any down nodes from the current node?*/
        n2 = n1->dw;

        if( n2 ){
			/*down node found; new current node is that node*/
            n1 = n2;
            continue;
        }            


		/*current node has no more rigth & no more down nodes;
		find a node from the stack that has a down node defined
		*/
		while(1){

			if(!bgStackGetItemCount(s))
				return;

			n2 = (bgTree4CNode_typ*)bgStackPop(s);

			if( n2 ){
				n1 = n2->dw;
				if( !n1 )
					continue;
				else
					break;
			}
		}


	}
}


#endif
