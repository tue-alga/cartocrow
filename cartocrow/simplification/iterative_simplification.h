#pragma once

#include "../core/core.h"

namespace cartocrow::simplification {

class StopCriterion {
  public:
	virtual bool stop(Number<Exact> cost, int complexity) {
		return false;
	}
};

class StopOnComplexity : public StopCriterion {
  private:
	int target;

  public:
	StopOnComplexity(int target) : target(target) {}

	virtual bool stop(Number<Exact> cost, int complexity) {
		return complexity <= target;
	}
};

class StopOnThreshold : public StopCriterion {
  private:
	Number<Exact> threshold;

  public:
	StopOnThreshold(int threshold) : threshold(threshold) {}

	virtual bool stop(Number<Exact> cost, int complexity) {
		return cost > threshold;
	}
};

class StopAfterSteps : public StopCriterion {
  private:
	int steps;

  public:
	StopOnComplexity(int steps) : target(steps) {}

	virtual bool stop(Number<Exact> cost, int complexity) {
		steps--;
		return steps < 0;
	}
};

} // namespace cartocrow::simplification