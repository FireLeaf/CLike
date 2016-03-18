struct Hello 
{
	int a;
	int world;
	char int;
};

void Func(int g)
{
	int a[100] = 10;
	struct Hello a;

	for (int i = 0; i < 100; i++)
	{
		a[i] += 9;

		if (a[i] % 7 == 3 )
		{
			printf("hello");
		}
		else if (a[i] % 8 == 2)
		{
			printf("world");
		}
		else
		{
		}
	}
}
