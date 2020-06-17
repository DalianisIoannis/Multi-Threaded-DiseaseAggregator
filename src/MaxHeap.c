#include "../headers/MaxHeap.h"

MaxHeapPtr initMaxHeap(){
    MaxHeapPtr tree = malloc(sizeof(MaxHeap));
    if(tree==NULL){ return NULL; }
    tree->root = NULL;
    return tree;
}

void emptyMaxHeapNode(HeapNodePtr node){
    if(node==NULL){ return; }
    else{
        emptyMaxHeapNode(node->left);
        emptyMaxHeapNode(node->right);
        node->occurence = NULL;
        node->right = NULL;
        node->left  = NULL;
        free(node);
    }
}

void emptyMaxHeap(MaxHeapPtr tree){
    emptyMaxHeapNode(tree->root);
    free(tree);
}

void printMaxHeapNode(HeapNodePtr node, int space){
    int i;
    if(node==NULL){
        space += 10;
        for(i=10; i<space; i++) { printf(" "); }
        printf("NULL");
        return;
    }
    space += 10;
    printMaxHeapNode(node->right, space);
    printf("\n");
    for(i=10; i<space; i++){ printf(" "); }
    if( strcmp(node->occurence, "$$$$$$$$$$$")!=0 ){
        printf("%s, %d\n", node->occurence, node->total);
    }
    else{
        printf("NULL");
    }
    printMaxHeapNode(node->left, space);
}

// incresases total of occurences 
bool searchAllexisting(HeapNodePtr *father, char *item){
    if( (*father)==NULL ){
        return false;
    }
    else{
        if( strcmp( (*father)->occurence, item )==0 ){
            (*father)->total++;
            return true;
        }
        bool tmp = searchAllexisting( &(*father)->left, item );
        if(tmp==true){ return true; }
        return searchAllexisting( &(*father)->right, item );
    }
    return false;
}

void findParentOfInsertedNode(HeapNodePtr node, int insId, HeapNodePtr *father){
    if( node->id==(insId/2) ){
        (*father) = node;   // under this father we insert the node
    }
    if( node->left!=NULL ){
        findParentOfInsertedNode( node->left, insId, father );
    }
    if( node->right!=NULL ){
        findParentOfInsertedNode( node->right, insId, father );
    }
}

void insertNodetoMaxHeap(HeapNodePtr *root, HeapNodePtr new_node){
    HeapNodePtr parent;
    if( new_node->id==1 ){  // put in root
        (*root) = new_node;
    }
    else{
        findParentOfInsertedNode( (*root), new_node->id, &parent );
        if( (new_node->id)%2==1 ){
            parent->right = new_node;
        }
        else{
            parent->left = new_node;
        }
    }
}

void compareFatherWithChild(HeapNodePtr *father, HeapNodePtr *child){
    if( (*father)->total< (*child)->total ){
        int tmpInt;
        char *tmpStr;

        tmpInt = (*father)->total;
        (*father)->total = (*child)->total;
        (*child)->total = tmpInt;

        tmpStr = (*father)->occurence;
        (*father)->occurence = (*child)->occurence;
        (*child)->occurence = tmpStr;
    }
}

void reheapify(HeapNodePtr *node){
    if( (*node)==NULL ){ return; }
    if( (*node)->left!=NULL ){
        reheapify( &((*node)->left) );
    }
    if( (*node)->right!=NULL ){
        reheapify( &((*node)->right) );
    }
    if( (*node)->right!=NULL ){
        if( (*node)->left!=NULL ){
            if( ((*node)->left)->total > ((*node)->right)->total ){
                compareFatherWithChild( node, &((*node)->left) );   // compare current with left child
            }
            else{
                compareFatherWithChild( node, &((*node)->right) );  // compare current with right child
            }
        }
        else{
            compareFatherWithChild( node, &((*node)->right) );  // compare current with right child
        }
    }
    else{
        if( (*node)->left!=NULL ){
            compareFatherWithChild( node, &((*node)->left) );   // compare current with left child
        }
    }
}

HeapNodePtr getNodeWithId(HeapNodePtr treenode, int id, HeapNodePtr *father){
    HeapNodePtr tmp = NULL;
    if( treenode==NULL ){
        return NULL;
    }
    else{
        if( treenode->id == id ){
            return treenode;
        }
        else{
            *father = treenode;
            tmp =  getNodeWithId(treenode->right, id, father);
            if( tmp == NULL ){
                *father = treenode;
                tmp = getNodeWithId(treenode->left, id, father);
            }
            return tmp;
        }
    }
    return NULL;
}

agesRangePtr initAgeRangeStruct() {
    
    agesRangePtr node = malloc(sizeof(agesRange));
    if(node==NULL) {
        perror("Allocation of agesRangePtr\n");
        return NULL;
    }
    node->age0_20 = malloc(12);
    sprintf(node->age0_20, "%d", 0);
    node->age21_40 = malloc(12);
    sprintf(node->age21_40, "%d", 0);
    node->age41_60 = malloc(12);
    sprintf(node->age41_60, "%d", 0);
    node->age61 = malloc(12);
    sprintf(node->age61, "%d", 0);

    return node;
}
void freeagesRangePtr(agesRangePtr* node) {
    free( (*node)->age0_20 );
    free( (*node)->age21_40 );
    free( (*node)->age41_60 );
    free( (*node)->age61 );
    free( (*node) );
}

void uploadRangeStats(agesRangePtr* node, char* age, int occs) {
 
    int ageInt = atoi(age);
    int lastCount;

    if(ageInt<=20) {
        lastCount = atoi((*node)->age0_20)+occs;
        free((*node)->age0_20);
        (*node)->age0_20 = malloc(12);
        sprintf((*node)->age0_20, "%d", lastCount);
    }
    else if(ageInt<=40) {
        lastCount = atoi((*node)->age21_40)+occs;
        free((*node)->age21_40);
        (*node)->age21_40 = malloc(12);
        sprintf((*node)->age21_40, "%d", lastCount);
    }
    else if(ageInt<=60) {
        lastCount = atoi((*node)->age41_60)+occs;
        free((*node)->age41_60);
        (*node)->age41_60 = malloc(12);
        sprintf((*node)->age41_60, "%d", lastCount);
    }
    else {
        lastCount = atoi((*node)->age61)+occs;
        free((*node)->age61);
        (*node)->age61 = malloc(12);
        sprintf((*node)->age61, "%d", lastCount);
    }
 
}

char* printRange(agesRangePtr* node, int k) {

    int age0_20Int = atoi((*node)->age0_20);
    int age21_40Int = atoi((*node)->age21_40);
    int age41_60Int = atoi((*node)->age41_60);
    int age61Int = atoi((*node)->age61);
    int total = age0_20Int + age21_40Int + age41_60Int + age61Int;

    freeagesRangePtr(node);

    int arINT[4];
    arINT[0] = age0_20Int;
    arINT[1] = age21_40Int;
    arINT[2] = age41_60Int;
    arINT[3] = age61Int;

    char **arStr = malloc(4*sizeof(char*));
    arStr[0] = malloc(strlen("41-60")+1);
    strcpy(arStr[0], "0-20");

    arStr[1] = strdup("21-40");
    arStr[2] = strdup("41-60");
    
    arStr[3] = malloc(strlen("41-60")+1);
    strcpy(arStr[3], "60+");

    selectionSort(arINT, 4, arStr);

    if(k>0) {
        char* strConcat = malloc( (4*15)*sizeof(char) );
        strcpy(strConcat, arStr[3]);
        strcat(strConcat, ": ");

        char* floatStr = malloc( 12*sizeof(char) );
        
        sprintf(floatStr, "%.2f", (float)(((float)arINT[3]*(float)100)/(float)total));
        
        strcat(strConcat, floatStr);
        strcat(strConcat, "%");
        strcat(strConcat, "\n");


        int tmpI=2;
        int tmpK=k-1;
        while(tmpI>=0 && tmpK>0) {

            free(floatStr);
            
            strcat(strConcat, arStr[tmpI]);
            strcat(strConcat, ": ");

            floatStr = malloc( 12*sizeof(char) );
            sprintf(floatStr, "%.2f", (float)(((float)arINT[tmpI]*(float)100)/(float)total));
            strcat(strConcat, floatStr);
            strcat(strConcat, "%");
            strcat(strConcat, "\n");
            
            tmpK--;
            tmpI--;
        }

        free(floatStr);

        for(int i=0; i<4; i++) {
            free(arStr[i]);
        }
        free(arStr);

        return strConcat;

    }
    else {
        for(int i=0; i<4; i++) {
            free(arStr[i]);
        }
        free(arStr);

        return NULL;
    }
    

}

// replace root with last inserted
char* printKlargestItems(MaxHeapPtr tree, int k, int *id){

    // printf("I am going to print the %d largest items of Heap:\n", k);
    // printMaxHeapNode(tree->root, 0); printf("\n\n\n");

    agesRangePtr statistic = initAgeRangeStruct();

    // while( k>0 && tree->root!=NULL ){
    while( tree->root!=NULL ){
        // k--;
        HeapNodePtr father = NULL;
        HeapNodePtr tmp = getNodeWithId(tree->root, (*id)--, &father);

        // printf("Age %s appeared %d\n", tree->root->occurence, tree->root->total);
        uploadRangeStats(&statistic, tree->root->occurence, tree->root->total);

        if( tmp==NULL || strcmp(tmp->occurence, tree->root->occurence)==0 ){
            emptyMaxHeapNode(tree->root);
            tree->root = NULL;
        }
        else{
            if( father!=NULL ){
                if( father->right!=NULL ){
                    if( father->right->id==(*id)+1 ){
                        father->right = NULL;
                    }
                }
                else{
                    father->left = NULL;
                }
            }

            HeapNodePtr old_last = tmp;
            HeapNodePtr right_child_of_old_root = tree->root->right;
            HeapNodePtr left_child_of_old_root  = tree->root->left;
            
            tree->root->left = NULL;
            tree->root->right = NULL;
            HeapNodePtr tmp_root = tree->root;
            emptyMaxHeapNode(tmp_root);

            tree->root = old_last;
            tree->root->right = right_child_of_old_root;
            tree->root->left = left_child_of_old_root;
        }
        reheapify( &(tree->root) );
    }

    return printRange(&statistic, k);
    // freeagesRangePtr(&statistic);

}

bool addMaxHeapNode(MaxHeapPtr tree, char *item, int *id){

    if( searchAllexisting( &(tree->root), item )==true ){
        reheapify( &(tree->root) );  // reheapify CAN BE AFTER EVERY NODE OR IN THE END
    }
    else{
        HeapNodePtr node = malloc(sizeof(HeapNode));
        if(node==NULL){
            printf("Couldn't allocate Heap Node.\n");
            return false;
        }
        node->occurence = item;
        node->total     = 1;
        node->id        = (*id);
        node->left      = NULL;
        node->right     = NULL;
        insertNodetoMaxHeap( &(tree->root), node );
        (*id)++;
    }
    return true;
}