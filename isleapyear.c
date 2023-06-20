#include <stdio.h>
#include <stdlib.h>

// Function to check if a year is a leap year
int isLeapYear(int year) {
    if (year % 4 == 0) {                            // If the year is divisible by 4
        if (year % 100 != 0 || year % 400 == 0) {    // If it is not divisible by 100, or if it is divisible by 400
            return 1;                               // Then it is a leap year
        }
    }
    return 0;                                       // Not a leap year otherwise
}

int main(int argc, char *argv[]) {
    if (argc != 2) {                                // Check if the command-line argument count is correct
        printf("Usage: %s <year>\n", argv[0]);      // Print the correct usage format
        return 1;                                   // Return with an error code
    }
    
    int year = atoi(argv[1]);                       // Convert the command-line argument to an integer using atoi

    if (isLeapYear(year)) {                          // Call the isLeapYear function to check if it's a leap year
        printf("%d was a leap year\n", year);         // Print the message if it's a leap year
    } else {
        printf("%d was not a leap year\n", year);     // Print the message if it's not a leap year
    }

    return 0;                                       // Return success code
}
