int main()
{
    int a[2];
    // TypeOf(&(a[1])) is pointer not appearing anywhere else.
    // It points to int, which does exist, in the line above.
	return &(a[1]) - &(a[0]);
}
