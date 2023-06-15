# Hilbert Turtle

This program generates a Hilbert Curve using the Turtle module in Python. The Hilbert Curve is a space-filling fractal curve that visits every point in a square grid in a specific order. The program provides both sequential and parallel implementations of the curve generation algorithm.

## Dependencies

- Python 3.x
- Turtle module

## Execution Instructions

1. Ensure you have a C++ compiler installed on your system.
2. Compile the source code with OpenMP support by running the following command:
g++ -fopenmp hilbert_curve.cpp -o hilbert_curve

3. Execute the compiled program by running:
./hilbert_curve


This will generate the Hilbert Curve scripts in Python.

4. Install the necessary dependencies by running the following commands:
!pip install turtle



5. Run the generated Python scripts to visualize the curves. For example:
python hilbert_turtle_seq.py

or

python hilbert_turtle_par.py


The sequential version is generated in `hilbert_turtle_seq.py`, and the parallel version is generated in `hilbert_turtle_par.py`.

## Performance Testing

The program includes a performance testing function that measures the execution time of the sequential and parallel implementations. It performs multiple tests and calculates the average time for each version.

To run the performance tests, modify the `num_tests`, `order`, and `angle` variables in the `main` function and execute the code.

```cpp
int num_tests = 10; // Number of tests to perform
int order = 10; // Hilbert Curve order
int angle = 90; // Rotation angle
test_performance(order, angle, output, num_tests);
