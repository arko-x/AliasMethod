#ifndef _ALIASMETHOD_H_
#define _ALIASMETHOD_H_

#include <stdio.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <float.h>
#include <stdint.h>

using namespace std;

class AliasMethod
{
public:
	AliasMethod(){srandom(time(NULL));};
	~AliasMethod(){};
	void init(const vector<double> &prob);
	void init(const vector<uint32_t> &prob);
	void init(const vector<int64_t> &prob);

	/**
	 * @brief     得到单个权重计算结果
	 *
	 * @returns   索引下标
	 */
	size_t genNext();


	/**
	 * @brief  精确地得到批量权重计算结果，精确、较慢，大概比genDiffBatchFast慢8倍
	 *
	 * @param batchCnt   批量数量
	 * @param v          返回值，务必传入空数
	 */
	void genDiffBatchAccurately(uint32_t batchCnt, vector<size_t> &v);


	/**
	 * @brief  快速地得到批量权重计算结果，速度快，代价是在极少情况会下失真    
	 *
	 * @param batchCnt   批量数量
	 * @param v          返回值，务必传入空数组
	 */
	void genDiffBatchFast(uint32_t batchCnt, vector<size_t> &v);
public:
	vector<double>				vecProb;
	vector<double>              vecOrigProb;
	vector<size_t>      		vecAlias;
};

void AliasMethod::init(const vector<double> &prob)
{
	vecProb.clear();
	vecProb.reserve(prob.size());
	vecAlias.clear();
	vecAlias.reserve(prob.size());
	vecOrigProb.clear();
	vecOrigProb = prob;
	

	vector<std::size_t> more;
	more.reserve(prob.size());
	vector<std::size_t> less;
	less.reserve(prob.size());

	double sum = accumulate(prob.begin(), prob.end(), 0.0);
	double average = 1.0 / prob.size();
	for (size_t i = 0; i < prob.size(); ++i) {
		double p = prob[i] / sum;
		vecProb.push_back(p);
		vecAlias.push_back(i);
		if (p > average) {
			more.push_back(i);
		} else if (p - average < FLT_EPSILON && p - average > -FLT_EPSILON) {
			vecProb[i] = 1.0;
			vecAlias[i] = i;
		} else {
			less.push_back(i);
		}
	}

	while (!less.empty() && !more.empty()) {
		size_t l = less.back();
		less.pop_back();
		size_t m = more.back();
		more.pop_back();
		vecProb[m] -= (average - vecProb[l]);
		vecProb[l] = vecProb[l] / average;
		vecAlias[l] = m;
		if (vecProb[m] > average) {
			more.push_back(m);
		} else if (vecProb[m] - average < FLT_EPSILON && vecProb[m] - average > -FLT_EPSILON) {
			vecProb[m] = 1.0;
			vecAlias[m] = m;
		} else {
			less.push_back(m);
		}
	}

	while (!less.empty()) {
		size_t l = less.back();
		less.pop_back();
		vecProb[l] = 1.0;
		vecAlias[l] = l;
	}

	while (!more.empty()) {
		size_t m = more.back();
		more.pop_back();
		vecProb[m] = 1.0;
		vecAlias[m] = m;
	}
}

void AliasMethod::init(const std::vector<uint32_t> &prob)
{
	vector<double> vec;
	vec.reserve(prob.size());
	for (size_t i = 0; i < prob.size(); i++) {
		vec.push_back(double(prob[i]));
	}
	init(vec);
}

void AliasMethod::init(const std::vector<int64_t> &prob)
{
	vector<double> vec;
	vec.reserve(prob.size());
	for (size_t i = 0; i < prob.size(); i++) {
		vec.push_back(double(prob[i]));
	}
	init(vec);
}

size_t AliasMethod::genNext()
{
	std::size_t i = random() % vecProb.size();
	if (random() / double(RAND_MAX) > vecProb[i]) {
		i = vecAlias[i];
	}
	return i;
}

void AliasMethod::genDiffBatchAccurately(uint32_t batchCnt, vector<size_t> &v)
{
	if (batchCnt == 0) return;

	batchCnt = batchCnt > vecProb.size() ? vecProb.size() : batchCnt;

	vector<double> origProb = vecOrigProb;
	for (size_t i = 0; i < batchCnt; i++) {
		AliasMethod am;
		am.init(origProb);
		size_t idx = am.genNext();
		v.push_back(idx);
		origProb[idx] = 0.0;
	}
}

void AliasMethod::genDiffBatchFast(uint32_t batchCnt, vector<size_t> &v)
{
	if (batchCnt == 0) return;

	batchCnt = batchCnt > vecProb.size() ? vecProb.size() : batchCnt;

	uint32_t loopCnt = batchCnt * 10;
	for (size_t i = 0; i < loopCnt; i++) {
		size_t idx = genNext();
		if (find(v.begin(), v.end(), idx) == v.end()) {
			v.push_back(idx);
			if (v.size() == batchCnt) break;
		}
	}

	if (v.size() < batchCnt) {
		//在这里需要记录一些日志...
		//从idx小到大补齐
		for (size_t i = 0; i < vecProb.size(); i++) {
			if (find(v.begin(), v.end(), i) == v.end()) {
				v.push_back(i);
				if (v.size() == batchCnt) break;
			}
		}
	}
}

#endif
