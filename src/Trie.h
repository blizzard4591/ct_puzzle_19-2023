#ifndef TRIE_H_
#define TRIE_H_

#include <bitset>
#include <memory>

class TrieNode {
public:
	std::unique_ptr<TrieNode> zeroChild;
	std::unique_ptr<TrieNode> oneChild;
};

class Trie {
public:
	Trie();
	~Trie();

	bool hasValueOrSubsetThereof(std::bitset<32> const& value) const;
	void insertValue(std::bitset<32> const& value);
private:
	bool checkNodeHasValueOrSubsetThereof(TrieNode* node, std::bitset<32> const& value, std::size_t i) const;

	std::unique_ptr<TrieNode> m_root;
};

#endif
