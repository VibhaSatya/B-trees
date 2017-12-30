#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#define totalNodes 1000  //total number of nodes that can be stored in memory
//##########################################################################################################################
//FILE IMPLEMENTATION
//##########################################################################################################################
FILE* file;

typedef struct //structure to store data of each key
{
int valid; //to check if the key has been deleted
long int keyval; //col1 of dataset
char ccode[4]; //country code -col2 of dataset
char s[5]; //col3 of dataset
int x; //col4 of dataset
int y; //col5 of dataset
}data;
typedef struct
{
int pos;//position of the node in the file
int n; //number of keys currently present in the node
data *keys; //array of keys in the node
int *children; //array of children indices


int leaf; //true if leaf node
}node;

int t; //degree of btree
//Each node can have atleast t-1 keys and atmost 2t-1 keys

int mindex=-1; //position of the recently stored node 
int rootindex=-1;

//##########################################################################################################################

void printNode(data d)
{
//prints key structure at given index
printf("%ld %s %s %d %d\n",(d.keyval),(d.ccode),(d.s),(d.x),(d.y));
}

//CREATION

void create(data d)
{
//printf("\nInside create\n");
//creates a new node and inserts the first key in the node
mindex++;
node* newnode=(node*)malloc(sizeof(node));
newnode->n=1;
newnode->keys=(data*)malloc(sizeof(data)*(2*t-1)); 
newnode->keys[0].keyval=d.keyval;
strcpy(newnode->keys[0].ccode,d.ccode);
strcpy(newnode->keys[0].s,d.s);
newnode->keys[0].x=d.x;
newnode->keys[0].y=d.y;

newnode->children=(int*)malloc(sizeof(int)*(2*t));
newnode->leaf=1;
newnode->pos=mindex;
//write node into file
fseek(file,mindex*sizeof(node),0);
fwrite(newnode,sizeof(node),1,file);
rootindex=mindex;
free(newnode);
}

//##########################################################################################################################
//INSERTION

void splitChild(node* x,int i)
{

//printf("\nInside splitChild\n");
//x is the node whose child (ith child) has to be split
int index =x->children[i]; //index of ith child

//printf("\n%d\n",index);
//splitting child into y and z
node *y=(node*)malloc(sizeof(node));//child node
fseek(file,index*sizeof(node),0);
fread(y,sizeof(node),1,file);

mindex++;
node *z=(node*)malloc(sizeof(node));
z->keys=(data*)malloc(sizeof(data)*(2*t-1));
z->children=(int*)malloc(sizeof(int)*(2*t));
z->leaf=y->leaf;
z->n=t-1;
z->pos=mindex;
int j;
for(j=0;j<t-1;j++)
	z->keys[j]=y->keys[j+t];
if(y->leaf==0)
	{
	for(j=0;j<t;j++)
	  z->children[j]=y->children[j+t];
	}
y->n=t-1;
for(j=x->n;j>i;j--)
	x->children[j+1]=x->children[j];
x->children[j+1]=mindex;
for(j=x->n-1;j>i;j--)
	x->keys[j+1]=x->keys[j];	
x->keys[i]=y->keys[t];
x->n=x->n+1;
//write z into file
fseek(file,(z->pos)*sizeof(node),0);
fwrite(z,sizeof(node),1,file);
//write y into file
fseek(file,(y->pos)*sizeof(node),0);
fwrite(y,sizeof(node),1,file);
//write x into file
fseek(file,(x->pos)*sizeof(node),0);
fwrite(x,sizeof(node),1,file);

}
void insert_nonfull(node* x,data k)
{
int i=x->n-1;
//printf("\nInside insert_nonfull\n");
if(x->leaf==1)
	{
	while(i>=0 && k.keyval<(x->keys[i].keyval))
	  {
          x->keys[i+1]=x->keys[i];
	  i--;
	  }
	x->keys[i+1]=k;
	x->n=x->n+1;
	//write x into file
	fseek(file,(x->pos)*sizeof(node),0);
	fwrite(x,sizeof(node),1,file);
	return;
	}
else
	{
	while(i>=0 && k.keyval<(x->keys[i].keyval)) i--;
	i=i+1;
	//descending to the ith child
	int pos=x->children[i];
	node* ci=(node*)malloc(sizeof(node));
	//read node from file
	fseek(file,pos*sizeof(node),0);
	fread(ci,sizeof(node),1,file);
	//if the child is full
        if(ci->n==2*t-1)
	  splitChild(ci,i);
	if(k.keyval>(x->keys[i].keyval)) i++;
	insert_nonfull(ci,k);
	}

}
void insert(data k)
{
//printf("\nInside insert \n");
//read root from file
node* root=(node*)malloc(sizeof(node));
fseek(file,rootindex*sizeof(node),0);
fread(root,sizeof(node),1,file);

//if root is full
if(root->n==2*t-1)
	{
	node *s=(node*)malloc(sizeof(node));
	s->keys=(data*)malloc(sizeof(data)*(2*t-1));
	s->children=(int*)malloc(sizeof(int)*(2*t));
	s->leaf=0;
	s->n=0;
	s->children[0]=rootindex;
	mindex++;
	s->pos=mindex;
	//write s into file
	fseek(file,(s->pos)*sizeof(node),0);
	fwrite(s,sizeof(node),1,file);
	rootindex=mindex;
        splitChild(s,1); 
	insert_nonfull(s,k);       
	}
else
	insert_nonfull(root,k);

}
//##########################################################################################################################
//SEARCH

data search(node *x,long int k,int *pos,int *key)
{
int i=0;
while(i<(x->n) && (k>(x->keys[i].keyval))) i++;
if(i<(x->n) && k==(x->keys[i].keyval) && (x->keys[i].valid==1)) 
	{
	*pos=x->pos; 
	*key=i; 
	return (x->keys[i]);
	}
if(i<(x->n) && (k==(x->keys[i].keyval)) && (x->keys[i].valid==0)) 
	{
	printf("\nNOT FOUND:(\n");
	data d;
	d.keyval=-1;
	return d;
	}
else if(x->leaf==1) 
	{
	printf("\nNOT FOUND:(\n");
	data d;
	d.keyval=-1;
	return d;
	}
else
	{
	int p=x->children[i];
	fseek(file,p*sizeof(node),0);
	node* y=(node*)malloc(sizeof(node));
	fread(y,sizeof(node),1,file);
	return(search(y,k,pos,key)); //descend to ith child
	}
}

//##########################################################################################################################


void readFile(char *filename)
{

FILE *fp=fopen(filename,"r");
if(fp==NULL)
{
printf("\nError reading file\n");
return;

}

while(!feof(fp))
{
long int col1;
char *col2,*col3,*temp;
col2=(char*)malloc(sizeof(char)*4);
col3=(char*)malloc(sizeof(char)*5);
temp=(char*)malloc(sizeof(char)*10);
int col4,col5;
fscanf(fp,"%ld",&col1);
fscanf(fp,"%4s",col2);
col2=col2+1;
col2[3]='\0';
fscanf(fp,"%5s",col3);
col3=col3+1;
col3[4]='\0';
fscanf(fp,"%s",temp);
sscanf(temp,"%*[^0123456789]%d%*[^0123456789]%d",&col4,&col5);

//printf("%ld %s %s %d %d\n",col1,col2,col3,col4,col5);
data d;
d.keyval=col1;
strcpy(d.ccode,col2);
strcpy(d.s,col3);
d.x=col4;
d.y=col5;
d.valid=1;
if(mindex==-1)
create(d);
else 
{insert(d);}
}
fclose(fp);

}
void delete(long int k)
{
int pos,key;
fseek(file,rootindex*sizeof(node),0);
node* root=(node*)malloc(sizeof(node));
fread(root,sizeof(node),1,file);
data d=search(root,k,&pos,&key);
if(d.keyval==-1)
{
printf("\nKey does not exist\n"); 
return;
}
fseek(file,pos*sizeof(node),0);
node *temp;
fread(temp,sizeof(node),1,file);
temp->keys[key].valid=0;//making the deleted node invalid

//writing node back into memory after making changes
fseek(file,pos*sizeof(node),0);
fwrite(temp,sizeof(node),1,file);
printf("\n%ld DELETED\n",k);
}

int main()
{
//file to store the btree
file=fopen("btreeFile.dat","w+");
if(file==NULL)
{
printf("\nError reading file\n");
return -1;

}

struct timespec start,end;
//setting the degree of the btree
t=4;
//allocating space for btree
printf("###############################################################################");
printf("\nCONSTRUCTING THE TREE...\n\n");
 clock_gettime(CLOCK_REALTIME,&start);
readFile("smalldataset.csv");
 clock_gettime(CLOCK_REALTIME,&end);
//printf("%ld.%ld\n",start.tv_sec,start.tv_nsec);
//printf("%ld.%ld\n",end.tv_sec,end.tv_nsec);
 printf("\n%ld\n",(end.tv_sec-start.tv_sec));
int ch;
int pos,key;
while(1)
{
printf("\n###############################################################################");
printf("\nPress 1 to insert, 2 to delete, 3 to search and 4 to exit\n\n");
scanf("%d",&ch);
if(ch==1)
	{
	printf("\n\nINSERTION\n");
	data d;
	printf("\nEnter the key value: ");
	scanf("%ld",&(d.keyval));
	printf("\nEnter the country code: ");
	scanf("%s",(d.ccode));
	printf("\nEnter the status: ");
	scanf("%s",(d.s));
	printf("\nEnter x: ");
	scanf("%d",&(d.x));
	printf("\nEnter y: ");
	scanf("%d",&(d.y));
	d.valid=1;
	 clock_gettime(CLOCK_REALTIME,&start);
	insert(d);
	 clock_gettime(CLOCK_REALTIME,&end);
	 printf("\n%ld\n",(end.tv_nsec-start.tv_nsec)/1000);
	printf("\nINSERTION SUCCESSFUL\n");
	}
else if(ch==2)
	{
	printf("\n\nDELETION\n");
	long int k;
	printf("\nEnter the key value of the record to be deleted: ");
	scanf("%ld",&k);
	 clock_gettime(CLOCK_REALTIME,&start);
	 delete(k);
	 clock_gettime(CLOCK_REALTIME,&end);
 	printf("\n%ld\n",(end.tv_sec-start.tv_sec));
	}
else if(ch==3)
	{
	printf("\n\nSEARCH\n");
	long int k;
	printf("\nEnter the key value of the record: ");
	scanf("%ld",&k);
	int pos,key;
	//read the root
	node* root=(node*)malloc(sizeof(node));
	fseek(file,rootindex*sizeof(node),0);
	fread(root,sizeof(node),1,file);
	 clock_gettime(CLOCK_REALTIME,&start);
	data d=search(root,k,&pos,&key);
	 clock_gettime(CLOCK_REALTIME,&end);
 	printf("\n%ld\n",(end.tv_nsec-start.tv_nsec)/1000);
	if(d.keyval!=-1) 
	printNode(d);
	}
else
	{
 	printf("\n\nEXIT\n");break;
	}
}
fclose(file);
}
