#include "math.h"
#include "tl/array.h"

static std::random_device RandomDevice;
static std::mt19937 RandomEngine(RandomDevice());
static std::uniform_real_distribution<float> DistributionFloat(0.0f, 1.0f);

float random_float()
{
	return DistributionFloat(RandomEngine);
}

bool random_prob(float f)
{
	return (random_float() < f);
}

int random_int(int Min, int Max)
{
	std::uniform_int_distribution<int> Distribution(Min, Max);
	return Distribution(RandomEngine);
}

int random_distribution(double* pProb, double* pProb2)
{
	std::discrete_distribution<int> Distribution(pProb, pProb2);
	return Distribution(RandomEngine);
}

int to_binary(int d)
{
	array<int> b;
	int c=0;
	b.clear();
	for(int i = 0;d > 0;i ++)
	{
		b.add(d%2);
		d /= 2;
	}
	for(int i = 0;i < b.size();i ++)
	{
		c += b[i] * pow(10, i);
	}
	return c;
}
