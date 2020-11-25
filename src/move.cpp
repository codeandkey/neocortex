/* vim: set ts=4 sw=4 noet: */

/*
 * This file is subject to the terms and conditions defined in
 * LICENSE.txt, included in this source code distribution.
 */

#include "attacks.h"
#include "bitboard.h"
#include "move.h"
#include "util.h"

#include <cassert>

using namespace neocortex;

const Move Move::null = Move();

Move::Move() : m_src(square::null), m_dst(square::null), m_ptype(piece::null) {}

Move::Move(int src, int dst, int ptype) {
	assert(square::is_valid(src));
	assert(square::is_valid(dst));

	this->m_src = src;
	this->m_dst = dst;
	this->m_ptype = ptype;
}

Move::Move(std::string uci) {
	if (uci.size() < 4 || uci.size() > 5) {
		throw util::fmterr("Invalid UCI \"%s\": moves must be 4 or 5 characters, parsed %d", uci.c_str(), uci.size());
	}

	m_src = square::from_uci(uci.substr(0, 2));
	m_dst = square::from_uci(uci.substr(2, 2));
	m_ptype = (uci.size() == 5) ? piece::type_from_uci(uci[4]) : piece::null;
}

std::string Move::to_uci() {
	std::string output;

	output += square::to_uci(m_src);
	output += square::to_uci(m_dst);

	if (piece::is_type(m_ptype)) {
		output += piece::type_to_uci(m_ptype);
	}

	return output;
}

bool Move::is_valid() {
	if (!square::is_valid(m_src)) return false;
	if (!square::is_valid(m_dst)) return false;

	return true;
}

bool Move::match_uci(std::string uci) {
	return *this == Move(uci);
}

int Move::src() {
	assert(is_valid());

	return m_src;
}

int Move::dst() {
	assert(is_valid());

	return m_dst;
}

int Move::ptype() {
	assert(is_valid());

	return m_ptype;
}

Move::operator std::string() {
	return to_uci();
}

bool Move::operator==(const Move& rhs) const {
	return (m_src == rhs.m_src && m_dst == rhs.m_dst && m_ptype == rhs.m_ptype);
}

bool Move::operator!=(const Move& rhs) const {
	return !(*this == rhs);
}

PV::PV() {
	len = 0;
}

std::string PV::to_string() {
	std::string output;

	for (int i = 0; i < len; ++i) {
		if (i) output += ' ';
		output += moves[i].to_uci();
	}

	return output;
}