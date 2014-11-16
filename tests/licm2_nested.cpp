int main()
{
	int x;
	int y;
	int z;
	int m;
	int i;
	int j;

	y = 4;
	z = 2;
	m = 0;
	i = 0;
	j = 0;

	while (j < 50)
	{
		while (i < 100)
		{
		    x = y + z;
		    m = m + 6 * i + x * x;
		    i = i + 1;
		    m = m * j;
		}

		j = j + 1;
	}

	return 0;	
}