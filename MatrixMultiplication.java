package com.company;

/******************************************************************************
 *  Compilation:  javac MatrixMultiplication.java
 *  Runtime   :   Java 8 SE
 *  Execution:    java MatrixMultiplication
 *
 *  8 different ways to multiply two dense n-by-n matrices.
 *  Illustrates importance of row-major vs. column-major ordering.
 *
 *
 ******************************************************************************/

public class MatrixMultiplication {
    public static void show(double[][] a) {
        int n = a.length;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                System.out.printf("%6.4f ", a[i][j]);
            }
            System.out.println();
        }
        System.out.println();
    }


    public static void main(String[] args) {
        int n = 1000;
        long start, stop;
        double elapsed;


        // generate input
        start = System.currentTimeMillis();

        double[][] A = new double[n][n];
        double[][] B = new double[n][n];
        double[][] C;

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                A[i][j] = (float) Math.random();

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                B[i][j] = (float) Math.random();

        stop = System.currentTimeMillis();
        elapsed = (stop - start) / 1000.0;
        System.out.println("Generating input:  " + elapsed + " seconds");


        // order 7: jik optimized ala JAMA 
        C = new double[n][n];
        start = System.currentTimeMillis();
        double[] bcolj = new double[n];
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) bcolj[k] = B[k][j];
            for (int i = 0; i < n; i++) {
                double[] arowi = A[i];
                double sum = 0.0;
                for (int k = 0; k < n; k++) {
                    sum += arowi[k] * bcolj[k];
                }
                C[i][j] = sum;
            }
        }
        stop = System.currentTimeMillis();
        elapsed = (stop - start) / 1000.0;
        System.out.println("Order jik JAMA optimized:   " + elapsed + " seconds");
        if (n < 10) show(C);

        // order 8: ikj pure row
        C = new double[n][n];
        start = System.currentTimeMillis();
        for (int i = 0; i < n; i++) {
            double[] arowi = A[i];
            double[] crowi = C[i];
            for (int k = 0; k < n; k++) {
                double[] browk = B[k];
                double aik = arowi[k];
                for (int j = 0; j < n; j++) {
                    crowi[j] += aik * browk[j];
                }
            }
        }
        stop = System.currentTimeMillis();
        elapsed = (stop - start) / 1000.0;
        System.out.println("Order ikj pure row:   " + elapsed + " seconds");
        if (n < 10) show(C);

    }

}