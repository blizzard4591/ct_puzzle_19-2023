#ifndef TRIE_H_
#define TRIE_H_

#include <bitset>
#include <vector>

struct TrieNode {
public:
	TrieNode() : zeroChild(-1), oneChild(-1) {
		//
	}

	std::int64_t zeroChild;
	std::int64_t oneChild;

	template<class Archive>
	void serialize(Archive& archive) {
		archive(zeroChild, oneChild);
	}
};

template <std::size_t BIT_COUNT>
class Trie {
public:
	Trie() : m_nodes() {
		//
	}
	~Trie() {
		//
	}

	bool hasValueOrSubsetThereof(std::bitset<BIT_COUNT> const& value) const {
		if (m_nodes.size() == 0) {
			return false;
		}
		if constexpr(BIT_COUNT == 0) {
			return true;
		}

		return checkNodeHasValueOrSubsetThereof(0, value, 0);
	}

	void insertValue(std::bitset<BIT_COUNT> const& value) {
		if (m_nodes.size() == 0) {
			makeNode();
		}
		if constexpr(BIT_COUNT == 0) {
			return;
		}

		std::int64_t nodeIndex = 0;
		for (std::size_t i = 0; i < BIT_COUNT; ++i) {
			auto const bit = value[i];
			if (!bit) {
				if (m_nodes[nodeIndex].zeroChild < 0) {
					m_nodes[nodeIndex].zeroChild = makeNode();
				}
				nodeIndex = m_nodes[nodeIndex].zeroChild;
			} else {
				if (m_nodes[nodeIndex].oneChild < 0) {
					m_nodes[nodeIndex].oneChild = makeNode();
				}
				nodeIndex = m_nodes[nodeIndex].oneChild;
			}
		}
	}

	template<class Archive>
	void serialize(Archive& archive) {
		archive(m_nodes);
	}
private:
	inline std::int64_t makeNode() {
		std::int64_t const result = m_nodes.size();
		m_nodes.push_back(TrieNode());
		return result;
	}

	bool checkNodeHasValueOrSubsetThereof(std::int64_t const& nodeIndex, std::bitset<BIT_COUNT> const& value, std::size_t i) const {
		if (i >= BIT_COUNT) {
			return true;
		}

		auto const bit = value[i];
		if (m_nodes[nodeIndex].zeroChild >= 0) {
			if (checkNodeHasValueOrSubsetThereof(m_nodes[nodeIndex].zeroChild, value, i + 1)) {
				return true;
			}
		}
		if (bit && (m_nodes[nodeIndex].oneChild >= 0)) {
			return checkNodeHasValueOrSubsetThereof(m_nodes[nodeIndex].oneChild, value, i + 1);
		}
		return false;
	}

	std::vector<TrieNode> m_nodes;
};

#endif
