int main()
{
	int x;
	int y;

	x = 7;
	while(x)
	{
		x = x - 1;
		y = x * 5 + 1;
	}	
	
	x = x + 1;
	
	while(x)
		x = x + 1;
	
	x = x + 1;
	return x;
}
