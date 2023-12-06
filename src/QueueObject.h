#ifndef QUEUEOBJECT_H_
#define QUEUEOBJECT_H_

#include <bitset>
#include <string>

template <std::size_t PRESENT_COUNT>
class QueueObject {
public:
	QueueObject() : m_pos(0), m_presentState(), m_moves("") {
		//
	}

	QueueObject(std::size_t const& penguinPosition, std::bitset<PRESENT_COUNT> const& presentState) : m_pos(penguinPosition), m_presentState(presentState), m_moves("") {
		//
	}

	QueueObject(std::size_t const& pos, std::bitset<PRESENT_COUNT> const& presentState, std::string&& moves) : m_pos(pos), m_presentState(presentState), m_moves(moves) {
		//
	}

	inline std::size_t getPos() const noexcept(true) {
		return m_pos;
	}

	inline std::bitset<PRESENT_COUNT> getPresentState() const noexcept(true) {
		return m_presentState;
	}

	inline std::string const& getMoves() const noexcept(true) {
		return m_moves;
	}

	QueueObject moveTo(std::size_t const& newPos, std::bitset<PRESENT_COUNT> const& presentState, char direction) const {
		return QueueObject(newPos, presentState, m_moves + direction);
	}

	inline char getLastDirection() const {
		if (m_moves.size() == 0) return '?';
		return m_moves.at(m_moves.size() - 1);
	}

	template<class Archive>
	void serialize(Archive& archive) {
		archive(m_pos, m_presentState, m_moves);
	}
private:
	std::size_t m_pos;
	std::bitset<PRESENT_COUNT> m_presentState;
	std::string m_moves;
};

#endif
