/*
 * Artificial Intelligence for Humans
 * Volume 1: Fundamental Algorithms
 * C/C++ Version
 * http://www.aifh.org
 * http://www.jeffheaton.com
 *
 * Code repository:
 * https://github.com/jeffheaton/aifh

 * Copyright 2013 by Jeff Heaton
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information on Heaton Research copyrights, licenses
 * and trademarks visit:
 * http://www.heatonresearch.com/copyright
 */
#include "aifh-vol1-examples.h"

typedef struct XOR_PARAMS {
	DATA_SET *training;
	ERROR_CALC *errorCalc;
	RBF_NETWORK *network;
} XOR_PARAMS;

static DATA_SET *create_xor_training() {
	DATA_SET *result;

	result = DataCreate(4,2,1);
	DataMoveCursor(result,0);
	DataAddVar(result,0.0,0.0,0.0);
	DataAddVar(result,1.0,0.0,1.0);
	DataAddVar(result,0.0,1.0,1.0);
	DataAddVar(result,1.0,1.0,0.0);

	return result;
}

static double score_function(void *m, void *p) {
	XOR_PARAMS *params;
	RBF_NETWORK *network;
	TRAIN *train;
	double *input, *ideal,y;
	unsigned int row;
	double *memory;
	
	memory = (double*)m;
	train = (TRAIN*)p;
	params = (XOR_PARAMS *)train->params;
	network = params->network;

	memcpy(network->long_term_memory,m,sizeof(double)*network->ltm_size);

	ErrorReset(params->errorCalc);
	for(row=0;row<params->training->recordCount;row++) {
		input = DataGetInput(params->training,row);
		ideal = DataGetIdeal(params->training,row);
		RBFNetworkComputeRegression(network, input, &y);
		ErrorUpdate(params->errorCalc,&y,ideal,1);
	}
	return ErrorCalculate(params->errorCalc);
}

void ExampleRandXOR(int argIndex, int argc, char **argv) {
	XOR_PARAMS *params;
	TRAIN_GREEDY *train;
	double *x0, *input, *ideal,y;
	int size,i;

	params = (XOR_PARAMS*)calloc(1,sizeof(XOR_PARAMS));
	params->training = create_xor_training();
	params->errorCalc = ErrorCreate(TYPE_ERROR_SSE);
	params->network = RBFNetworkCreate(RBFGaussian,2,5,1);
	RBFNetworkReset(params->network);

	size = params->network->ltm_size*sizeof(double);
	x0 = (double*)calloc(size,1);
	memcpy(x0,params->network->long_term_memory,size);
	
	train = (TRAIN_GREEDY*)TrainCreateGreedyRandom(score_function,1,x0,size,params,-10,10);
	TrainRun((TRAIN*)train,1000000,0.01,1);
	TrainComplete((TRAIN*)train,params->network->long_term_memory);

	/* Perform the evaluation */
	for(i=0;i<4;i++) {
		input = DataGetInput(params->training,i);
		ideal = DataGetIdeal(params->training,i);
		RBFNetworkComputeRegression(params->network, input, &y);
		printf("[%.2f,%.2f] -> Actual: [%.2f], Ideal: [%.2f]\n",
			input[0],input[1],y,ideal[0]);
	}
	
	DataDelete(params->training);
	ErrorDelete(params->errorCalc);
	RBFNetworkDelete(params->network);
	free(params);
}