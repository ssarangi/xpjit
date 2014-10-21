int multiply(int x, int y);

void main()
{
	int x;
	int y;
	int r;
	int r1;
	int r2;
	x = 5;
	y = 6;
	r1 = multiply(x,y);
	r2 = multiply(x,y);
	r = r1 + r2;
	return r;
}

int multiply(int x, int y)
{
	int m;
	int constant;
	constant = 10;
	m = x * y;
	m = m * constant;
	return m;	
}