#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_CHILD	5
#define HALF_CHILD	((MAX_CHILD+1)/2)

typedef int KEY;
typedef int DATA;

typedef struct node {
	int nodekind; // 内部ノードか葉か
	union {
		struct {	// 内部ノードの場合
			int	_nchildren;
			struct  node *_child[MAX_CHILD];
			KEY	_low[MAX_CHILD];
		} _internal;
		struct {	// 葉の場合
			KEY	_leaf_key;
			DATA	_your_data;
		} _leaf;
	} _u;
} NODE;

#define INTERNAL	1
#define LEAF		2

#define nchildren	_u._internal._nchildren
#define child		_u._internal._child
#define low		_u._internal._low

#define leaf_key	_u._leaf._leaf_key
#define your_data	_u._leaf._your_data

NODE	*root	= NULL;

void error(char *s)
{
	fprintf(stderr, s);
	exit(1);
}

NODE	*alloc_node(void)
{	
	NODE *p;
	if ((p = (NODE *)malloc(sizeof(NODE)))==NULL)
		error("メモリを使い果たしました\n");
	return p;
}

int keyequal(KEY a, KEY b)
{
	return a==b;
}

int keylt(KEY a, KEY b)
{
	return a<b;
}

int locate_subtree(NODE *p, KEY key)
{
	int i;
	for (i = p->nchildren - 1; i>0;i--)
		if(key >= p->low[i])
			return i;
	return 0;
}

NODE *search(KEY key)
{
	NODE *p;
	int i;

	if (root == NULL)
		return NULL;
	else {
		p = root;
		while(p->nodekind == INTERNAL){
			i = locate_subtree(p,key);
			p = p->child[i];
		}
		if (key == p->leaf_key)
			return p;
		else
			return NULL;
	}
}

NODE *insert_aux(NODE **pnode, KEY key, NODE **newnode, KEY *lowest)
{
	NODE *node;

	*newnode = NULL;
	node = *pnode;
	if (node->nodekind == LEAF){
	// 葉に到達したケース
		if (keyequal(node->leaf_key, key))
			// すでに存在するノードなので、何もしない
			return NULL;
		else {
			NODE *new_node;
			new_node = alloc_node();
			new_node->nodekind = LEAF;
			new_node->leaf_key = key;
			if (key < node->leaf_key){
				//割り当てた葉の方が小さい婆はnewとnodeの位置を入れ替え
				*pnode = new_node;
				*lowest = node->leaf_key;
				*newnode = node;
			} else {
				*lowest = key;
				*newnode = new_node;
			}
			return new_node;
		}
	} else {
		int 	pos;
		NODE	*xnode;
		KEY		xlow;
		NODE	*retv;
		int		i, j;

		pos = locate_subtree(node, key);
		retv = insert_aux(&(node->child[pos]), key, &xnode, &xlow);
		if (xnode == NULL)
			//分割されていない場合はそのままリターン
			return retv;
		// 分割が行われていた時の対処
		if (node->nchildren < MAX_CHILD){
			//追加の余地あり
			for (i=node->nchildren - 1; i > pos; i--){
				node->child[i+1] = node->child[i];
				node->low[i+1] = node->low[i];
			}
			node->child[pos+1] = xnode;
			node->low[pos+1] = xlow;
			node->nchildren++;
			return retv;
		} else {
			//追加の余地がないので、nodeを2つに分割
			NODE *new_node;
			new_node = alloc_node();
			new_node->nodekind = INTERNAL;
			// どちらのノードにも追加するかで場合わけ
			if (pos < HALF_CHILD - 1){
				// nodeに追加する場合
				for (i = HALF_CHILD -1, j = 0; i< MAX_CHILD; i++, j++){
					new_node->child[j] = node->child[i];
					new_node->low[j] = node->low[i];
				}
				for (i = HALF_CHILD - 2; i>pos; i--){
					node->child[i+1] = node->child[i];
					node->low[i+1] = node->low[i];
				}
				node->child[pos+1] = xnode;
				node->low[pos+1] = xlow;
			}else{
				// new_nodeに追加する場合
				j = MAX_CHILD - HALF_CHILD;
				for (i =MAX_CHILD-1; i>=HALF_CHILD; i--){
					if (i == pos){
						new_node->child[j] = xnode;
						new_node->low[j--] = xlow;
					}
					new_node->child[j] = node->child[i];
					new_node->low[j--] = node->low[i];
				}
				if (pos < HALF_CHILD){
					new_node->child[0]=xnode;
					new_node->low[0]=xlow;
				}
			}
			node->nchildren = HALF_CHILD;
			new_node->nchildren = MAX_CHILD + 1 - HALF_CHILD;
			*newnode = new_node;
			*lowest = new_node->low[0];
			return retv;
		}
	}
}


NODE *insert(KEY key)
{
	if(root == NULL){
		root = alloc_node();
		root->nodekind = LEAF;
		root->leaf_key = key;
		return root;
	} else {
		NODE *retv, *new_node, *newnode;
		KEY lowest;
		retv = insert_aux(&root, key, &newnode, &lowest);
		if (newnode != NULL){
			new_node = alloc_node();
			new_node->nodekind = INTERNAL;
			new_node->nchildren = 2;
			new_node->child[0] = root;
			new_node->child[1] = newnode;
			new_node->low[1] = lowest;
			root = new_node;
		}
		return retv;
	}
}

int merge_nodes(NODE *p, int x)
{
	NODE *a, *b;
	int an, bn;
	int i;

	a = p->child[x];
	b = p->child[x+1];
	b->low[0] = p->low[x+1];
	an = a->nchildren;
	bn = b->nchildren;
	if (an + bn < MAX_CHILD){
		// aとbをマージできるので、aとbをマージする
		for (i=0;i<bn;i++){
			a->child[an+i] = b->child[i];
			a->low[an+i] = b->low[i];
		}
		a->nchildren += bn;
		free(b);
		return 1;
	} else {
		// aとbをマージできないので、aとbを再分配する
		int n, move;
		n = (an + bn) / 2;
		if(an > n) {
			move = an - n;
			// 要素を右にずらす
			for(i=bn-1;i>=0;i--){
				b->child[i+move] = b->child[i];
				b->low[i+move] = b->low[i];
			}
			// aからbに要素をmove個分移動
			for(i=0;i<move;i++){
				b->child[i] = a->child[n+i];
				b->low[i] = a->low[n+i];
			}
		} else {
			// bからaに要素を移動
			move = n - an;
			for(i=0;i<move;i++){
				a->child[an+i] = b->child[i];
				a->low[an+i] = b->low[i];
			}
			// 要素を左にずらす
			for(i=0;i<bn-move;i++){
				b->child[i] = b->child[i+move];
				b->low[i] = b->low[i+move];
			}
		}
		// aとbの要素数を更新
		a->nchildren = n;
		b->nchildren = an + bn - n;
		// pのlowを更新
		p->low[x+1] = b->low[0];
		return 0;
	}
}


#define OK			1
#define REMOVED		2
#define NEED_REORG	3

int delete_aux(NODE *node, KEY key, int *result)
{
	*result = OK;
	if(node->nodekind == LEAF){
		if(keyequal(node->leaf_key, key)){
			//  葉ノードでkeyが見つかった
			*result = REMOVED;
			free(node);
			return 1;
		} else {
			// 見つからなかった
			return 0;
		}
	} else {
		//内部ノードの場合
		int pos;
		int condition;
		int retv;
		int sub;
		int joined;
		int i;

		pos = locate_subtree(node, key);
		retv = delete_aux(node->child[pos], key, &condition);
		if(condition == OK)
			return retv;
		if(condition == NEED_REORG){
			sub = (pos==0) ? 0 : pos - 1;
			joined = merge_nodes(node, sub);
			// 統合されているのなら、sub+1の木を削除する
			if(joined){
				pos = sub+1;
			}
		}
		if (condition == REMOVED || joined){
			for (i=pos;i<node->nchildren-1;i++){
				node->child[i] = node->child[i+1];
				node->low[i] = node->low[i+1];
			}
			if (--node->nchildren < HALF_CHILD){
				*result = NEED_REORG;
			}
		}
		return retv;
	}
}


int delete(KEY key)
{
	int result;
	int retv;
	NODE *p;

	if (root == NULL)
		return 0;
	else {
		retv = delete_aux(root, key, &result);
		if(result == REMOVED)
			root = NULL;
		else if(result == NEED_REORG && root->nchildren == 1){
			p = root;
			root = root->child[0];
			free(p);
		}
		return retv;
	}
}

void printtree(NODE *p)
{
	int i;
	if(p->nodekind == LEAF){
		printf("%04x leaf val=%d\n", p, p->leaf_key);
	} else {
		printf("%04x %02d [%04x] %d[%04x] %d[%04d] %d[%04d] %d[%04d]\n",
				p, p->nchildren, p->child[0],
				p->low[1], p->child[1],
				p->low[2], p->child[2],
				p->low[3], p->child[3],
				p->low[4], p->child[4]
			);
		for (i=0;i<p->nchildren;i++)
			printtree(p->child[i]);
	}

}


int main()
{
	// static int data[] = { 10, 20, 30, 40, 50};
	static int data[] = { 13, 5, 2, 7, 6, 21, 15 };
	int i, x;
	char	str[100];

	for (i=0;i<sizeof(data)/sizeof(data[0]); i++)
		insert(data[i]);

	if (delete(5))
		printf("5を削除しました\n");
	else
		printf("5が見つかりません\n");

	printtree(root);

	return 0;
}