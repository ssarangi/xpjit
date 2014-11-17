int main()
{
	int x;
	int y;
	int z;
	int m;
	int i;
	int j;
	int k;

	y = 4;
	z = 2;
	m = 0;
	i = 0;
	j = 0;
	k = 1;

	while (j < 50)
	{
		while (i < 100)
		{
		    x = y + z;
		    m = m + 6 * i + x * x;
		    i = i + 1;
		    m = m * j;

		    while (k < 40)
		    {
		    	k = k + 1;
		    	m = m + j * (x + y);
		    }
		}

		j = j + 1;
	}

	j = 0;
	while (j < 50)
	{
		j = j + 1;
	}

	return 0;	
}