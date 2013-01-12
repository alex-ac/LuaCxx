
int main() {
    int array[5] = {1, 2, 3, 4, 5};
    int s = 0;
    for (int x : array) {
        s += x;
    }
    return s != 15;
}

