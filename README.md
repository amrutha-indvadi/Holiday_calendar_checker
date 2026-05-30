# Holiday Calendar Checker
A C application that determines the day of the week, identifies weekends, and checks whether a given date is a holiday using data stored in a CSV file.

## Features
* Accepts a user-input date (DD MM YYYY)
* Calculates the corresponding day of the week
* Identifies weekends (Saturday and Sunday)
* Reads holiday information from a CSV file
* Detects whether the entered date is a holiday
* Uses memory-mapped file processing for efficient CSV reading

## How It Works
1. The user enters a date.
2. The program calculates the day of the week.
3. It checks whether the date falls on a weekend.
4. The holiday list is loaded from a CSV file.
5. If the entered date matches a holiday, the holiday name is displayed.
