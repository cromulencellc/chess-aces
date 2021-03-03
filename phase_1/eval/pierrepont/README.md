# pierrepont

This challenge is a command line interface that allows a user to perform various matrix calculations similar to the open source project Octave.

To create a 2 x 3 matrix and store it into variable 'A' use the following command:

>>> A = [ 1 2 3 ; 4 5 6 ]

There are many available operations for matrices such as addition, subtraction, and sigmoid.

>>> B = [ 7 8 9 ; 10 11 12 ]
B =

    7.000 8.000 9.000
    10.000 11.000 12.000

>>> A + B

    8.000 10.000 12.000
    14.000 16.000 18.000

>>> C = [ 2 3 ; 4 5 ; 6 7 ]
C =

    2.000 3.000
    4.000 5.000
    6.000 7.000

>>> A * C

    28.000 34.000
    64.000 79.000

>>> %log(A)

    0.000 0.301 0.477
    0.602 0.699 0.778

>>> %sig(A)

    0.731 0.881 0.953
    0.982 0.993 0.998

To get the columns count of a matrix:
>>> %cols(A)

    3.000

To get the row count of a matrix:
>>> %rows(A)

    2.000

## Launching

To run standalone using stdin as input then just launch the challenge without any arguments. To run as a server specify the port either as an environment variable PORT=<value> or as an option to the command line argument -p along with the command line flag -s"

## Poller

The poller requires the numpy library and that the environment variables HOST and PORT be set to the necessary values.


## POV -- Apogee Only

The POV creates a 1 x 7 matrix and assigns it to the variable 'A'.

It then calls the function %reshape() which takes two arguments. The first is the matrix to be resized and the second is a 2 element vector with the rows and columns. For example, To reshape the matrix 'A' from the previous examples we could use this command to convert from a 2 x 3 to a 1 x 6:

>>> D = %reshape(A, [1 6])
D =

    1.000 2.000 3.000 4.000 5.000 6.000

To do this correctly the number of elements needs to be equivalent so 2 x 3 = 1 x 6. However, the vulnerable reshape function does not confirm that the new matrix has enough elements after the allocation to not overflow.

The vulnerable for() loop calculates the number of iterations based upon the matrix being resized:

for ( int i = 0; i < A->rows * A->columns; i++ ) {

However, the patch calculates the iterations based upon the destination size:

or ( int i = 0; i < (Crow * Ccol); i++ ) {
