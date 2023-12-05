#ifndef PRESENTOVERLAY_H_
#define PRESENTOVERLAY_H_

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <vector>

template<std::size_t NUM_ROWS, std::size_t NUM_COLS>
class PresentBase {
public:
	PresentBase(std::vector<std::size_t> presentCoordinates) {
		std::sort(presentCoordinates.begin(), presentCoordinates.end());

		if (presentCoordinates.size() > 32) {
			std::cerr << "Internal Error: Can not work with > 32 presents." << std::endl;
			exit(-1);
		}

		// Mark as "not mapped"
		for (std::size_t i = 0; i < m_mapToBitset.size(); ++i) {
			m_mapToBitset[i] = std::numeric_limits<std::size_t>::max();
		}

		std::size_t indexCounter = 0;
		for (auto it = presentCoordinates.cbegin(); it != presentCoordinates.cend(); ++it) {
			if (*it >= (NUM_ROWS * NUM_COLS)) {
				std::cerr << "Invalid input, present coordinate " << *it << " is out of bounds." << std::endl;
				exit(-1);
			}
			m_mapToBitset[*it] = indexCounter;
			++indexCounter;
		}

		m_totalPresentCount = indexCounter;
	}
	~PresentBase() {
		//
	}

	inline std::size_t getBitmapIndex(std::size_t const& pos) const {
		return m_mapToBitset[pos];
	}

	inline std::size_t getTotalPresentCount() const noexcept(true) {
		return m_totalPresentCount;
	}
private:
	std::array<std::size_t, (NUM_ROWS* NUM_COLS)> m_mapToBitset;
	std::size_t m_totalPresentCount;
};

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, std::size_t BIT_COUNT>
class PresentOverlay {
public:
	PresentOverlay(PresentBase<NUM_ROWS, NUM_COLS> const& base) : m_presentBase(base) {
		m_presents.reset();
		for (std::size_t i = 0; i < m_presentBase.getTotalPresentCount(); ++i) {
			m_presents[i] = true;
		}
	}
	PresentOverlay(PresentBase<NUM_ROWS, NUM_COLS> const& base, std::bitset<BIT_COUNT> const& presents) : m_presentBase(base), m_presents(presents) {
		//
	}

	~PresentOverlay() {
		//
	}

	void collectOn(std::size_t const& pos) {
		if (pos >= (NUM_ROWS * NUM_COLS)) {
			std::cerr << "Invalid input, present coordinate " << pos << " is out of bounds." << std::endl;
			exit(-1);
		}
		std::size_t const mappedIndex = m_presentBase.getBitmapIndex(pos);
		if (mappedIndex < BIT_COUNT) {
			m_presents[mappedIndex] = false;
		}
	}

	inline std::size_t getPresentsLeft() const {
		return m_presents.count();
	}

	inline std::size_t getPresentsCollected() const {
		return m_presentBase.getTotalPresentCount() - m_presents.count();
	}

	inline std::bitset<BIT_COUNT> const& getRepresentation() const noexcept(true) {
		return m_presents;
	}

	inline PresentBase<NUM_ROWS, NUM_COLS> const& getBase() const noexcept(true) {
		return m_presentBase;
	}
private:
	PresentBase<NUM_ROWS, NUM_COLS> const m_presentBase;
	std::bitset<BIT_COUNT> m_presents;
};


#endif
