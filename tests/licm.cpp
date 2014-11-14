int main()
{
	int x;
	int y;
	int z;
	int m;
	int i;

	y = 4;
	z = 2;
	m = 0;
	i = 0;

	while (i < 100)
	{
	    x = y + z;
	    m = m + 6 * i + x * x;
	    i = i + 1;
	}

	return 0;	
}