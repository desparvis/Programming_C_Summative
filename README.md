# Programming C Summative Projects

# Introduction

This repository contains Low-Level Programming Projects.<br> It contains five projects which are:<br>

**Smart Traffic Control and Monitoring System**<br>
**Automated System Monitoring Shell Script**<br>
**Student Management System**<br>
**Dynamic Math and Data Processing Engine**<br>
**Multi-threaded Web Scraper**<br>

# How they work

## Smart Traffic Control and Monitoring System

Traffic signal management which includes simulation of two intersections with 2 sets of 3 LED lights.
The two intersections are adjusted to alternate well between each other.
The system give output in the Serial Monitor as it is hard to log the values in a local file using Tinkercad.

## Automated System Monitoring Shell Script

System monitoring project that uses CPU, memory, and disk information.
It uses process management commands like top, df, free.
It requires user input to function.
It logs all system activity when auto monitor is activated.

## Student Management System using Structures

Student management system that let the user manage student information.
Applies structures for easy student management.
Does memory allocation.
Applies searching, sorting, file handling, and function pointers.

## Dynamic Math and Data Processing Engine

Dynamic math and data processing engine that allows users to add, delete, and update numbers.
Display the input dataset.
Run operations on the dataset like average, sum, minimum, and maximum.
Sort or search values.
Save and load data from a file.
It applies arrays, functions, loops, memory allocation, function pointers, file I/O, sorting, and searching.

## Multi-threaded Web Scraper

Multi-threaded web scraper that uses POSIX threads. It was made to download multiple web pages at the same time.
The addresses for the web pages to be downloaded are found in the txt file.
Multi-threading is implemented where each URL runs inside its own thread using pthread_create().
The thread downloads the HTML content and saves it on a text file.

# How to run

Make sure you are on Linux, Mac, or WSL on windows.

For the C files like dynamicmath.c, you will need to have a C compiler like gcc.<br>
You will need these commands to compile the c program: gcc example.c -o example
To run the program, you will need to do: ./example

For bash scripts like monitor.sh, you just need to make sure they are executable like: chmod +x example.sh
To run the program, you will need to do: /example.sh

For the embedded systems, you will need to simulate it in Tinkercad or use Arduino. Make sure the circuit is made and copy the code and then run.

## Author

Credo Desparvis Gutabarwa

