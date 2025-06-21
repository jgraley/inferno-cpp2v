// Variations on a theme of *(<some array> + i)
// to test conversion to <some array>[i];

int array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
int *p;

static struct 
{
    int a2[6];
} s;

int main()
{
    int x=0;
    x += *(array+3);
    x += *(*&array + 2*2);
    p = array;
    x += *(p+1);
    s.a2[4] = 99;
    x += *(s.a2 + 4); 
    return x;
}

struct Ohnoes;

struct Recurse
{
	Ohnoes *p;
};

struct Ohnoes
{
	Recurse *p;
};

typedef int v4si __attribute__ ((vector_size (16)));

int *ptrfunc()
{
	static int i;
	return (&i)+0;
}
