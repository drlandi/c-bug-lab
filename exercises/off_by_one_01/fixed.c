#include <stdio.h>

// Function to reverse a string
void reverseString(char *str) {
    int length = 0;
    char *end = str;
    char *start = str;

    // Calculate the length of the string
    while (*end != '\0') {
        length++;
        end++;
    }
    end--;

    // Reverse the string using two pointers
    for (int i = 0; i < length / 2; i++) {
        char temp = *start;
        *start = *end;
        *end = temp;

        start++;
        end--;
    }
}

int main() {
    char str[] = "Hello, World!";
    printf("Original string: %s\n", str);
    reverseString(str);
    printf("Reversed string: %s\n", str);

    return 0;
}
