#include <iostream>
#include <vector>
#include <cstdlib>

int partition(std::vector<int>& a, int l, int r)
{
	int i, j, pivot, t;
	i = l - 1;
	j = r;
	pivot = a[r];

	for (;;){
		while(a[++i]<pivot)
			;
		while(i < --j && pivot < a[j])
			;
		if(i >=j)
			break;
		t = a[i]; a[i]=a[j]; a[j]=t;
	}
	t = a[i]; a[i]=a[r]; a[r]=t;
	return i;
}

void quick_sort_main(std::vector<int>& a, int l, int r)
{
	int v;
	if (l>=r)
		return;

	v = partition(a, l, r);

	quick_sort_main(a, l, v-1);
	quick_sort_main(a, v+1, r);
}

void quick_sort(std::vector<int>& a, int n)
{
	quick_sort_main(a, 0, n-1);
}

int main(int argc, char **argv){
	std::vector<int> array {1, 2, 3, 6, 9, 33, 3, 7, 10};
	quick_sort(array, 9);

	for (int i = 0; i < 9; i++){
		std::cout << array[i] << std::endl;
	}
}
