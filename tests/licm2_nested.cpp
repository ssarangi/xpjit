int main()
{
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;

	a = 0;
	b = 1;
	c = 2;
	d = 3;
	e = 4;
	f = 5;

	int x;
	int y;
	int z;
	int m;
	int i;
	int j;
	int k;

	y = a;
	z = b;
	m = c;
	i = d;
	j = e;
	k = f;

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