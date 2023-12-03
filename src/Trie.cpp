#include "Trie.h"

Trie::Trie() : m_root() {
	//
}

Trie::~Trie() {
	//
}

bool Trie::checkNodeHasValueOrSubsetThereof(TrieNode* node, std::bitset<32> const& value, std::size_t i) const {
	if (i >= 31) {
		return true;
	}

	auto const bit = value[i];
	if (node->zeroChild) {
		if (checkNodeHasValueOrSubsetThereof(node->zeroChild.get(), value, i + 1)) {
			return true;
		}
	}
	if (bit && node->oneChild) {
		return checkNodeHasValueOrSubsetThereof(node->oneChild.get(), value, i + 1);
	}
	return false;
}

bool Trie::hasValueOrSubsetThereof(std::bitset<32> const& value) const {
	if (!m_root) {
		return false;
	}

	TrieNode* node = m_root.get();
	return checkNodeHasValueOrSubsetThereof(node, value, 0);
}

void Trie::insertValue(std::bitset<32> const& value) {
	if (!m_root) {
		m_root = std::make_unique<TrieNode>();
	}

	TrieNode* node = m_root.get();
	for (std::size_t i = 0; i < 32; ++i) {
		auto const bit = value[i];
		if (!bit) {
			if (!node->zeroChild) {
				node->zeroChild = std::make_unique<TrieNode>();
			}
			node = node->zeroChild.get();
		} else {
			if (!node->oneChild) {
				node->oneChild = std::make_unique<TrieNode>();
			}
			node = node->oneChild.get();
		}
	}
}
