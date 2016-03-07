#include "AliasMethod.h"
#include <pthread.h>
#include <unistd.h>

AliasMethod am;

void *AM(void *)
{
	int genCount[15] = {};
	for (int i = 0; i < 10000; ++i) {
		genCount[am.genNext()]++;
	}
	for (int i = 0; i < 15; ++i) {
		printf("%lu %u %d %lf\n",pthread_self(),i,genCount[i],double(genCount[i]) / genCount[0]);
	}
	for (int i = 0;i < 15;++i) {
		printf("%lu %lf %lu\n",pthread_self(), am.vecProb[i], am.vecAlias[i]);
	}
	return 0;
}


int main()
{
	uint32_t propVec[15] = {1,2,3,4,5,60,70,80,9,10,11,120,13,14,15};
	vector<uint32_t> vec(propVec,propVec+15);
	am.init(vec);


	pthread_t tid;
	pthread_create(&tid, NULL, AM, NULL);
	pthread_create(&tid, NULL, AM, NULL);
	pthread_create(&tid, NULL, AM, NULL);
	pthread_create(&tid, NULL, AM, NULL);

	for (; ;) {
		sleep(1);
	}

	return 0;
}
