//int firstmethod(int a, int b) {
//    return a + b + 4;
//}
//
//int main() {
//    int a, c, d;
//    c = 3;
//    d = 4;
//    a = firstmethod(c, d);
//    return a;
//}


int main() {
    int s, i, j;
    for (s = 0, i = 0; i < 10000; i++) {
        for (j = 0; j < 10000; j++)
            s += i*j;
    }
}