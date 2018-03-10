#ifdef __cplusplus
extern "C" {
#endif
    void ils_( int *threads, int *len,  double *a, double *b, double *x );
#ifdef __cplusplus
}
#endif

#include <math.h>  
#include <stdio.h>  
#include <stdlib.h>

/*  S E R I A L   C O D E  */

// Need the following prototype in case of a zero along the diagonal
void  dls_( int *threads, int *len, double *a, double *b, double *x );

// Code to check for zeros along the diagonal
int zerosAlongDiagonal ( int N, double *a ) {

    double ZERO;
    int i;
    int testFail;

    testFail = 0;
    for (i=0;i<N;i++) { 
        if (!testFail) { 
            testFail = fabs(*(a+i*N+i)) == ZERO;
            printf("line %d passed with %d\n", i, testFail);
            if ( testFail ) printf("failed on row %d\n", i);
    }
    }
    return testFail;
}

// Code to check for convergence
int converged( int N, double *a, double *b) {
    
    // Compute the distance between the vectors and see if the 2-Norm is
    // within tolerance

    double const TOL = 5.0e-15;
    double sum, maxb;
    int i;

    // find max in array b for tolerance scaling while computing sum
   
    maxb=*(b+0); 
    sum = 0.0; 
    for (i=0; i<N; i++) {
       maxb = fmax(maxb,fabs(*(b+i)));
       sum += (*(a+i)-*(b+i))*(*(a+i)-*(b+i));
    }
    sum = sqrt(sum);
    printf("sum = %e, scaled sum = %e\n", sum, sum/maxb);
    return (sum/maxb < TOL);    
    
}
    
void ils_( int *threads, int *len,  double *a, double *b, double *x ){

    /* in serial code, *threads not used. It is retained here so the code can be called
     * identically to the threaded methods.
     */


    int i, j, k, N, iteration;
    double sum1, sum2;
    double ZERO = 0.0;
    int ITERATION_MAX = 100000;
    double *x0;

    N = *len;

    // The iterative Jacobi method will fail if there are zeros along the diagonal of the
    // matrix.  We could reorder the matrix, but in this case if zeros are found we will switch
    // to our discrete linear solver which will do a form of Gaussian Elimination with partial
    // pivoting to solve the system. 

    if ( ! zerosAlongDiagonal( N, a ) ) {

        // Do Jacobi Iterative Method to solve Ax=b. 

        // Create an temporary to hold initial values and intermediate steps 

        x0 = malloc( N * sizeof(double) );

        // Fill the x0 vector with initial values of zero

        for (i=0;i<N;i++) *(x+i) = 0.0;
   
        // Fill the x vector with b vector just so the initial convergence test will fail

        for (i=0;i<N;i++) *(x0+i) = *(b+i);

       iteration = 0;
       while ( !converged(N,x,x0) && iteration < ITERATION_MAX ) {

          // copy last result to initial values
          
          for (i=0;i<N;i++) *(x0+i) = *(x+i);

          // start the reduction process
          
          for (i=0;i<N;i++) { 
             sum1 = 0.0;
             for (j=0;j<i-1;j++) sum1+= *(a+i*N+j)* *(x0+j); 
             sum2 = 0.0; 
             for (j=i+1;j<N;j++) sum2+= *(a+i*N+j)* *(x0+j); 
             *(x+i) = ( *(b+i) - sum1 - sum2 ) / *(a+i*N+i);
          }

          iteration++;

//          for (i=0;i<N;i++)  
//              printf( " %d  %f  %f \n", iteration, *(x+i), *(x0+i));
//          
          printf("iteration %d \n", iteration); 
          
        }
        free(x0);

     if ( iteration == ITERATION_MAX) {
       printf(" *** ITERATIVE SOLVER FAILED TO REACH CONVERGENCE AFTER  ***\n");
       printf(" *** %d ITERATIONS, SWITCHING TO DIRECT SOLVER ***\n", iteration);
       dls_( threads, len, a, b, x );
     }

    }

    else {

       dls_( threads, len, a, b, x );

    }


}


