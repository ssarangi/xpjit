int main(char **argv, int argc)
{
    int a, b, c;
    a = (int)argv[0];
    b = (int)argv[1];
    c = (int)argv[2];
    a = a + b;
    b = b * 5;
    c = c * b + a;
    return c;
}