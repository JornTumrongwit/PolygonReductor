
/**
* Get the minimum cost pair to collapse in the next iteration.
*/
/*
std::vector<unsigned int> Polygon::get_min_cost_QEM() {
	std::vector<unsigned int> min_pair(2);
	unsigned int min_cost = UINT_MAX;
	unsigned int current_cost;
	for (auto const& x : edges) {
		for (auto v2 : x.second) {
			current_cost = get_cost_QEM(x.first, v2);
			if (current_cost < min_cost) {
				min_cost = current_cost;
				min_pair[0] = x.first;
				min_pair[1] = v2;
			}
		}
	}
	return min_pair;
}
*/
