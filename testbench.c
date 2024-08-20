
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hls_config.h"

#define REL_TOL 2
#define TRIALS 5

int main(void)
{
	printf("Running C simulation!");

	int testResult = 0;
	FILE *fdata;
	data_t output[1];
	data_t x[N];
	data_t y[N];
	data_t buffer[1];
	data_t pp_rel_err;

	fdata =fopen("golden_Reference.dat","r");

	if (fdata == NULL)
	{
		printf("Error reading goldenReference file!\n");
		exit(1);
	}

	for (int i = 0; i < TRIALS; i++)
	{
		/* Read vector */
		for (int j = 0; j < N; ++j)
		{
			fscanf(fdata, "%f\n", &x[j]);
		}
		for (int j = 0; j < N; ++j)
		{
			fscanf(fdata, "%f\n", &y[j]);
		}
		
		/* Read reference results */
		fscanf(fdata, "%f\n", &buffer[0]);
		
		/* Call hls function */
		hls_pp(&output[0], x, y);

		/* Compute error */
		pp_rel_err = 0;
		pp_rel_err = 100 * fabs((output[0] - buffer[0])/buffer[0]);
		printf("TRIAL: %d,\t Expected: %.6f\tGot: %.6f Err: %.6f%%\n", i,buffer[0],output[0], pp_rel_err);
		if (pp_rel_err > REL_TOL)
		{
			printf("PP error exceeds tolerance!\n");
			testResult += 1;
		}
	}

	fclose(fdata);

	if (testResult)
	{
		printf("*******************************************\n");
		printf("*\t \t  FAIL \n");
		printf("*******************************************\n");
	}
	else
	{
		fprintf(stdout, "*******************************************\n");
		fprintf(stdout, "*\t \t  PASS \n");
		fprintf(stdout, "*******************************************\n");
	}
	return testResult;
}
