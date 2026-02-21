#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#define NUM_AXES 3
#define POLY_ORDER 6
#define STEP_MS 200

/* Polynomial evaluation */
double evaluate_poly(const double coeff[], int order, double t_sec) {
    double result = coeff[0];
    double t_power = 1.0;
    for (int i = 1; i <= order; i++) {
        t_power *= t_sec;
        result += coeff[i] * t_power;
    }
    return result;
}

int main(void) {
    const char *input_file = "C:\\Users\\Kushi S\\Desktop\\inputfile.txt";
    const char *output_file = "C:\\Users\\Kushi S\\Desktop\\output_quaternions.txt";

    FILE *fp_in = fopen(input_file, "r");
    if (!fp_in) {
        perror("Cannot open input file");
        return 1;
    }

    FILE *fp_out = fopen(output_file, "w");
    if (!fp_out) {
        perror("Cannot open output file");
        fclose(fp_in);
        return 1;
    }

    uint64_t t_start_ms, t_end_ms;
    double omega_r;
    double vec[NUM_AXES];
    double coeff[NUM_AXES][POLY_ORDER + 1];

    if (fscanf(fp_in, "%lf", (double*)&t_start_ms) != 1) { perror("t_start"); return 1; }
    if (fscanf(fp_in, "%lf", (double*)&t_end_ms) != 1) { perror("t_end"); return 1; }
    if (fscanf(fp_in, "%lf", &omega_r) != 1) { perror("omega_r"); return 1; }

    for (int i = 0; i < NUM_AXES; i++) {
        if (fscanf(fp_in, "%lf", &vec[i]) != 1) { perror("vector value"); return 1; }
    }

    for (int i = 0; i < NUM_AXES; i++) {
        for (int j = 0; j <= POLY_ORDER; j++) {
            if (fscanf(fp_in, "%lf", &coeff[i][j]) != 1) {
                fprintf(stderr, "Error reading coefficient axis %d, order %d\n", i, j);
                return 1;
            }
        }
    }

    fclose(fp_in);

    // Write header to output file
    fprintf(fp_out, "t_ms  qx  qy  qz  qw\n");

    for (uint64_t t_ms = t_start_ms; t_ms <= t_end_ms; t_ms += STEP_MS) {
        double t_sec = (double)(t_ms - t_start_ms) / 1000.0;
        double q_ref[4];

        for (int i = 0; i < NUM_AXES; i++) {
            q_ref[i] = evaluate_poly(coeff[i], POLY_ORDER, t_sec);
        }

        double norm_sq = q_ref[0]*q_ref[0] + q_ref[1]*q_ref[1] + q_ref[2]*q_ref[2];
        if (norm_sq <= 1.0)
            q_ref[3] = sqrt(1.0 - norm_sq);
        else
            q_ref[3] = 0.0;

        double q_norm = sqrt(q_ref[0]*q_ref[0] + q_ref[1]*q_ref[1] + q_ref[2]*q_ref[2] + q_ref[3]*q_ref[3]);
        for (int i = 0; i < 4; i++)
            q_ref[i] /= q_norm;

        fprintf(fp_out, "%llu  % .12e  % .12e  % .12e  % .12e\n",
                t_ms, q_ref[0], q_ref[1], q_ref[2], q_ref[3]);
    }

    fclose(fp_out);
    printf("Reference quaternions saved to: %s\n", output_file);

    return 0;
}
